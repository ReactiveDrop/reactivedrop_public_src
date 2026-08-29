#include "cbase.h"
#include "tier0/dbg.h"
#include "tier1/bitbuf.h"
#include "inetmessage.h"
#include "inetchannel.h"
#include "protocol.h"
#include "asrd_gns_message_bridge.h"
#include "asrd_gns_move_compat.h"
#include "asrd_gns_client_lifecycle.h"
#include "asrd_gns_server_lifecycle.h"
#include "asrd_gns_message_registry.h"

#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <atomic>
#include <deque>
#include <limits>
#include <mutex>
#include <new>
#include <vector>

namespace
{
	static const unsigned int kPhysicalRingCapacity = 128;
	static const unsigned int kDefaultLogicalCapacity = 32;
	static const unsigned int kMinimumLogicalCapacity = 16;
	static const unsigned int kMaximumLogicalCapacity = 128;
	static const unsigned int kLogicalCapacityStep = 16;
	static const unsigned int kSimulatorCount = 8;
	static const unsigned int kRollingOutcomeCapacity = 2000;
	// Automatic selection compares one exact sequence interval for every
	// candidate and for D_current.  The warm-up is deliberately outside the
	// formal sample set, so differing retirement speeds cannot change the
	// eligible sequence range seen by a simulator.
	static const unsigned int kMeasurementWarmupCount = 200;
	static const unsigned int kMeasurementEligibleCount =
		kRollingOutcomeCapacity;
	static const unsigned int kMeasurementTotalCount =
		kMeasurementWarmupCount + kMeasurementEligibleCount;
	static const double kReorderDeadlineSeconds = 0.030;
	static const double kWindowSelectionIntervalSeconds = 10.0;
	// This is the existing bridge delivery-rate requirement. It is kept as a
	// named policy constant so automatic selection cannot derive a target from
	// the candidate samples themselves.
	static const double kMinimumDeliveryRate = 0.98;
	static const double kDeliveryRateTolerance = 0.005;
	static const double kLatencyComparisonThresholdSeconds = 0.005;
	static const float kMinimumWindowRatio = 0.5f;
	static const float kMaximumWindowRatio = 4.0f;
	static const unsigned int kResidenceHistogramBinCount = 256;
	static const double kResidenceHistogramBinWidthSeconds = 0.001;
	static const uint16_t kNoResidenceHistogramBin = 0xffff;
	static const uint32_t kNoResidenceMicros = 0xffffffff;
	static const unsigned int kCandidateCapacities[ kSimulatorCount ] =
		{ 16, 32, 48, 64, 80, 96, 112, 128 };
	static const uint64_t kSequenceModulus =
		std::numeric_limits<uint64_t>::max();
	static const uint64_t kSequenceHalfRange = kSequenceModulus / 2;

	typedef ASRD_GNS_EnvelopeVNext MessageEnvelopeHeader;
	COMPILE_TIME_ASSERT( sizeof( MessageEnvelopeHeader ) ==
		ASRD_GNS_MESSAGE_ENVELOPE_HEADER_BYTES );

	struct QueuedEnvelope
	{
		ASRD_GNS_Connection connection;
		std::vector<unsigned char> bytes;
		unsigned int envelopeBytes;
		double firstArrivalTime;

		QueuedEnvelope()
			: connection( ASRD_GNS_CONNECTION_INVALID ), envelopeBytes( 0 ),
				firstArrivalTime( 0.0 )
		{
		}
	};

	static std::deque<QueuedEnvelope *> s_queue;
	static unsigned int s_received = 0;
	static unsigned int s_delivered = 0;
	static bool s_lastMapped = false;
	static ASRD_GNS_Connection s_lastHandle = ASRD_GNS_CONNECTION_INVALID;
	static ASRD_GNS_Connection s_failClosedServerConnection =
		ASRD_GNS_CONNECTION_INVALID;

	static std::atomic<uint64_t> s_nextSourceOrder( 1 );
	static std::mutex s_submissionMutex;

	static bool IsValidSequence( uint64_t sequence )
	{
		return sequence != 0;
	}

	static uint64_t SequenceDistance( uint64_t head, uint64_t sequence )
	{
		// Wire U sequences use the nonzero uint64 space: UINT64_MAX is followed
		// by 1, while zero remains reserved for the reliable lane. UINT64_MAX is
		// also the invalid-distance sentinel; a valid forward distance is at most
		// UINT64_MAX - 1.
		if ( !IsValidSequence( head ) || !IsValidSequence( sequence ) )
			return kSequenceModulus;
		return sequence >= head ? sequence - head
			: ( kSequenceModulus - head ) + sequence;
	}

	static bool SequenceIsBefore( uint64_t sequence, uint64_t boundary )
	{
		// Distances beyond the half-range are stale (and cover the ambiguous
		// half-space conservatively); invalid zero values are stale as well.
		if ( !IsValidSequence( sequence ) || !IsValidSequence( boundary ) )
			return true;
		const uint64_t distance = SequenceDistance( boundary, sequence );
		return distance != 0 && distance > kSequenceHalfRange;
	}

	static uint64_t AdvanceSequenceBy( uint64_t sequence, uint64_t count )
	{
		if ( !IsValidSequence( sequence ) )
			return 0;

		// Work in zero-based sequence coordinates without overflowing the
		// uint64 addition. The modulus is UINT64_MAX, not 2^64.
		const uint64_t offset = count % kSequenceModulus;
		const uint64_t zeroBased = sequence - 1;
		const uint64_t untilWrap = kSequenceModulus - zeroBased;
		const uint64_t advanced = offset >= untilWrap
			? offset - untilWrap : zeroBased + offset;
		return advanced + 1;
	}

	static uint64_t RetreatSequenceBy( uint64_t sequence, uint64_t count )
	{
		if ( !IsValidSequence( sequence ) )
			return 0;

		const uint64_t offset = count % kSequenceModulus;
		const uint64_t zeroBased = sequence - 1;
		const uint64_t retreated = offset > zeroBased
			? kSequenceModulus - ( offset - zeroBased )
			: zeroBased - offset;
		return retreated + 1;
	}

	static uint64_t AdvanceSequence( uint64_t sequence )
	{
		return AdvanceSequenceBy( sequence, 1 );
	}

	static bool HasExceededReorderDeadline( double currentTime,
		double firstArrivalTime )
	{
		// Plat_FloatTime is monotonic for the process. A negative/NaN age is
		// naturally rejected by the comparison, while an exact deadline is
		// eligible for force progress.
		return currentTime - firstArrivalTime >= kReorderDeadlineSeconds;
	}

	static uint16_t ResidenceHistogramBin( double residenceSeconds )
	{
		if ( !( residenceSeconds > 0.0 ) )
			return 0;
		const double scaled = residenceSeconds /
			kResidenceHistogramBinWidthSeconds;
		if ( scaled >= (double)kResidenceHistogramBinCount )
			return (uint16_t)( kResidenceHistogramBinCount - 1 );
		return (uint16_t)scaled;
	}

	static unsigned int NormalizeLogicalCapacity( unsigned int capacity )
	{
		if ( capacity < kMinimumLogicalCapacity )
			capacity = kMinimumLogicalCapacity;
		if ( capacity > kMaximumLogicalCapacity )
			capacity = kMaximumLogicalCapacity;
		capacity = ( ( capacity + kLogicalCapacityStep / 2 ) /
			kLogicalCapacityStep ) * kLogicalCapacityStep;
		if ( capacity < kMinimumLogicalCapacity )
			capacity = kMinimumLogicalCapacity;
		if ( capacity > kMaximumLogicalCapacity )
			capacity = kMaximumLogicalCapacity;
		return capacity;
	}

	static bool NormalizeWindowRatio( float ratio, unsigned int *capacity )
	{
		if ( !capacity || ratio != ratio )
			return false;
		if ( ratio == 0.0f )
		{
			*capacity = kDefaultLogicalCapacity;
			return true;
		}
		if ( ratio < kMinimumWindowRatio || ratio > kMaximumWindowRatio )
			return false;
		const double desired = (double)ratio * 32.0 + 0.5;
		*capacity = NormalizeLogicalCapacity( (unsigned int)desired );
		return true;
	}

	class UnreliableReorderRing
	{
	public:
		struct Slot
		{
			QueuedEnvelope *packet;
			uint64_t sequence;
			unsigned int packetBytes;
			double firstArrivalTime;

			Slot()
				: packet( NULL ), sequence( 0 ), packetBytes( 0 ),
					firstArrivalTime( 0.0 )
			{
			}
		};

		UnreliableReorderRing()
			: hasHead( false ), headSeq( 0 ), headIndex( 0 ),
				logicalCapacity( kDefaultLogicalCapacity ), pendingCount( 0 )
			{
			}

		void Reset( void )
		{
			for ( unsigned int i = 0; i < kPhysicalRingCapacity; ++i )
			{
				if ( slots[ i ].packet )
					delete slots[ i ].packet;
				slots[ i ] = Slot();
			}
			hasHead = false;
			headSeq = 0;
			headIndex = 0;
			pendingCount = 0;
			logicalCapacity = kDefaultLogicalCapacity;
		}

		bool HasHead( void ) const
		{
			return hasHead;
		}

		bool HasHeadPacket( void ) const
		{
			return hasHead && slots[ headIndex ].packet != NULL;
		}

		void Initialize( uint64_t firstSequence )
		{
			if ( hasHead || !IsValidSequence( firstSequence ) )
				return;
			hasHead = true;
			headSeq = firstSequence;
			headIndex = 0;
		}

		bool FastForwardEmpty( uint64_t count )
		{
			if ( !hasHead || pendingCount != 0 )
				return false;
			headSeq = AdvanceSequenceBy( headSeq, count );
			headIndex = ( headIndex +
				(unsigned int)( count % kPhysicalRingCapacity ) ) %
				kPhysicalRingCapacity;
			return true;
		}

		bool IsStale( uint64_t sequence ) const
		{
			return hasHead && SequenceIsBefore( sequence, headSeq );
		}

		bool IsOutsideWindow( uint64_t sequence ) const
		{
			return hasHead &&
				SequenceDistance( headSeq, sequence ) >= logicalCapacity;
		}

		unsigned int IndexFor( uint64_t sequence ) const
		{
			const unsigned int offset = (unsigned int)SequenceDistance(
				headSeq, sequence );
			return ( headIndex + offset ) % kPhysicalRingCapacity;
		}

		bool IsOccupied( uint64_t sequence ) const
		{
			return hasHead && slots[ IndexFor( sequence ) ].packet != NULL;
		}

		bool Store( uint64_t sequence, QueuedEnvelope *packet,
			unsigned int packetBytes, double firstArrivalTime )
		{
			if ( !packet || !hasHead || IsStale( sequence ) ||
				IsOutsideWindow( sequence ) )
				return false;
			Slot &slot = slots[ IndexFor( sequence ) ];
			if ( slot.packet )
				return false;
			slot.packet = packet;
			slot.sequence = sequence;
			slot.packetBytes = packetBytes;
			slot.firstArrivalTime = firstArrivalTime;
			++pendingCount;
			return true;
		}

		QueuedEnvelope *PopHead( uint64_t *retiredSequence,
			bool *hadPacket )
		{
			if ( retiredSequence )
				*retiredSequence = 0;
			if ( hadPacket )
				*hadPacket = false;
			if ( !hasHead )
				return NULL;

			Slot &slot = slots[ headIndex ];
			QueuedEnvelope *packet = slot.packet;
			if ( retiredSequence )
				*retiredSequence = headSeq;
			if ( hadPacket )
				*hadPacket = packet != NULL;
			if ( packet && pendingCount > 0 )
				--pendingCount;
			slot.packet = NULL;
			slot.sequence = 0;
			slot.packetBytes = 0;
			slot.firstArrivalTime = 0.0;
			headSeq = AdvanceSequence( headSeq );
			headIndex = ( headIndex + 1 ) % kPhysicalRingCapacity;
			return packet;
		}

		unsigned int PendingSpan( void ) const
		{
			if ( !hasHead )
				return 0;
			unsigned int span = 0;
			for ( unsigned int offset = 0; offset < kPhysicalRingCapacity;
				++offset )
			{
				const unsigned int index = ( headIndex + offset ) %
					kPhysicalRingCapacity;
				if ( slots[ index ].packet )
					span = offset + 1;
			}
			return span;
		}

		unsigned int PendingCount( void ) const
		{
			return pendingCount;
		}

		unsigned int LogicalCapacity( void ) const
		{
			return logicalCapacity;
		}

		void SetLogicalCapacity( unsigned int capacity )
		{
			logicalCapacity = NormalizeLogicalCapacity( capacity );
		}

		uint64_t HeadSequence( void ) const
		{
			return headSeq;
		}

		bool FindFarthestTimedOut( double currentTime,
			uint64_t *sequence ) const
		{
			if ( !sequence || !hasHead )
				return false;
			for ( unsigned int offset = logicalCapacity; offset > 0; --offset )
			{
				const unsigned int actualOffset = offset - 1;
				const unsigned int index = ( headIndex + actualOffset ) %
					kPhysicalRingCapacity;
				const Slot &slot = slots[ index ];
				if ( slot.packet && HasExceededReorderDeadline( currentTime,
					slot.firstArrivalTime ) )
				{
					*sequence = AdvanceSequenceBy( headSeq, actualOffset );
					return true;
				}
			}
			return false;
		}

		// Physical storage is deliberately fixed. Packet pointers are owned by
		// their slots until PopHead or Reset transfers/releases them.
		Slot slots[ 128 ];
		bool hasHead;
		uint64_t headSeq;
		unsigned int headIndex;
		unsigned int logicalCapacity;
		unsigned int pendingCount;
	};

	class UnreliableWindowSimulator
	{
	public:
		UnreliableWindowSimulator()
			: hasHead( false ), headSeq( 0 ), headIndex( 0 ),
				logicalCapacity( kDefaultLogicalCapacity ), outcomeHead( 0 ),
				outcomeCount( 0 ), deliveredCount( 0 ),
				residenceSampleCount( 0 ), residenceSumSeconds( 0.0 ),
				retiredCount( 0 ), measurementActive( false ),
				measurementComplete( false ), measurementStartSequence( 0 ),
				measurementEndSequence( 0 ), measurementOutcomeCount( 0 ),
				measurementDeliveredCount( 0 ),
				measurementResidenceSampleCount( 0 ),
				measurementResidenceSumSeconds( 0.0 )
		{
			Reset( kDefaultLogicalCapacity );
		}

		void Reset( unsigned int capacity )
		{
			hasHead = false;
			headSeq = 0;
			headIndex = 0;
			logicalCapacity = NormalizeLogicalCapacity( capacity );
			outcomeHead = 0;
			outcomeCount = 0;
			deliveredCount = 0;
			residenceSampleCount = 0;
			residenceSumSeconds = 0.0;
			retiredCount = 0;
			measurementActive = false;
			measurementComplete = false;
			measurementStartSequence = 0;
			measurementEndSequence = 0;
			measurementOutcomeCount = 0;
			measurementDeliveredCount = 0;
			measurementResidenceSampleCount = 0;
			measurementResidenceSumSeconds = 0.0;
			memset( occupied, 0, sizeof( occupied ) );
			memset( firstArrivalTime, 0, sizeof( firstArrivalTime ) );
			memset( sequence, 0, sizeof( sequence ) );
			memset( outcomes, 0, sizeof( outcomes ) );
			for ( unsigned int i = 0; i < kRollingOutcomeCapacity; ++i )
			{
				residenceBins[ i ] = kNoResidenceHistogramBin;
				residenceMicros[ i ] = kNoResidenceMicros;
			}
			memset( residenceHistogram, 0, sizeof( residenceHistogram ) );
			memset( measurementResidenceHistogram, 0,
				sizeof( measurementResidenceHistogram ) );
		}

		void Initialize( uint64_t firstSequence )
		{
			if ( hasHead || !IsValidSequence( firstSequence ) )
				return;
			hasHead = true;
			headSeq = firstSequence;
			headIndex = 0;
		}

		bool HasHead( void ) const
		{
			return hasHead;
		}

		uint64_t HeadSequence( void ) const
		{
			return headSeq;
		}

		// Start a new common measurement generation without touching the ring.
		// The caller chooses a sequence at or after every simulator's current
		// head.  Thus each simulator can retire at a different wall-clock time,
		// but formal metrics still cover exactly the same sequence interval.
		void BeginMeasurement( uint64_t firstSequence )
		{
			measurementActive = IsValidSequence( firstSequence );
			measurementComplete = false;
			measurementStartSequence = firstSequence;
			measurementEndSequence = measurementActive
				? AdvanceSequenceBy( firstSequence,
					kMeasurementTotalCount - 1 ) : 0;
			measurementOutcomeCount = 0;
			measurementDeliveredCount = 0;
			measurementResidenceSampleCount = 0;
			measurementResidenceSumSeconds = 0.0;
			memset( measurementResidenceHistogram, 0,
				sizeof( measurementResidenceHistogram ) );
		}

		bool MeasurementComplete( void ) const
		{
			return measurementActive && measurementComplete &&
				measurementOutcomeCount == kMeasurementEligibleCount;
		}

		void Observe( uint64_t incomingSequence, double arrivalTime )
		{
			if ( !IsValidSequence( incomingSequence ) )
				return;
			if ( !hasHead )
				Initialize( 1 );
			if ( SequenceIsBefore( incomingSequence, headSeq ) )
				return;

			const uint64_t distance = SequenceDistance( headSeq,
				incomingSequence );
			if ( distance >= logicalCapacity )
			{
				const uint64_t advanceCount = distance - logicalCapacity + 1;
				uint64_t physicalRetireCount = advanceCount;
				if ( physicalRetireCount > kPhysicalRingCapacity )
					physicalRetireCount = kPhysicalRingCapacity;
				for ( uint64_t i = 0; i < physicalRetireCount; ++i )
					PopHead( arrivalTime );
				if ( advanceCount > physicalRetireCount )
					FastForwardEmpty( advanceCount - physicalRetireCount );
			}

			const unsigned int index = IndexFor( incomingSequence );
			if ( occupied[ index ] )
				return;
			occupied[ index ] = true;
			sequence[ index ] = incomingSequence;
			firstArrivalTime[ index ] = arrivalTime;
			DrainAvailable( arrivalTime );
		}

		void CheckTimeout( double currentTime )
		{
			// Timeout checks are made only by the completed receive epoch. The
			// caller has already drained ASRD_GNS_Receive to zero before invoking
			// this method; drain again to make the simulator's force-progress
			// boundary identical to the real ring.
			DrainAvailable( currentTime );
			int farthestOffset = -1;
			for ( unsigned int offset = logicalCapacity; offset > 0; --offset )
			{
				const unsigned int actualOffset = offset - 1;
				const unsigned int index = ( headIndex + actualOffset ) %
					kPhysicalRingCapacity;
				if ( occupied[ index ] && HasExceededReorderDeadline( currentTime,
					firstArrivalTime[ index ] ) )
				{
					farthestOffset = (int)actualOffset;
					break;
				}
			}
			if ( farthestOffset >= 0 )
			{
				for ( int offset = 0; offset <= farthestOffset; ++offset )
					PopHead( currentTime );
			}
			DrainAvailable( currentTime );
		}

		void SetLogicalCapacity( unsigned int capacity, double currentTime )
		{
			capacity = NormalizeLogicalCapacity( capacity );
			while ( PendingSpan() > capacity )
				PopHead( currentTime );
			logicalCapacity = capacity;
			DrainAvailable( currentTime );
		}

		unsigned int SampleCount( void ) const
		{
			return measurementOutcomeCount;
		}

		double DeliveryRate( void ) const
		{
			return measurementOutcomeCount > 0
				? (double)measurementDeliveredCount /
					(double)measurementOutcomeCount : 0.0;
		}

		double P95ResidenceMilliseconds( void ) const
		{
			if ( measurementResidenceSampleCount == 0 )
				return 0.0;
			const unsigned int rank =
				( measurementResidenceSampleCount * 95 + 99 ) / 100;
			unsigned int accumulated = 0;
			for ( unsigned int i = 0; i < kResidenceHistogramBinCount; ++i )
			{
				accumulated += measurementResidenceHistogram[ i ];
				if ( accumulated >= rank )
					return ( (double)i + 1.0 ) *
						( kResidenceHistogramBinWidthSeconds * 1000.0 );
			}
			return (double)kResidenceHistogramBinCount *
				( kResidenceHistogramBinWidthSeconds * 1000.0 );
		}

		double AverageResidenceMilliseconds( void ) const
		{
			return measurementResidenceSampleCount > 0
				? ( measurementResidenceSumSeconds * 1000.0 ) /
					(double)measurementResidenceSampleCount : 0.0;
		}

	private:
		unsigned int IndexFor( uint64_t incomingSequence ) const
		{
			return ( headIndex + (unsigned int)SequenceDistance( headSeq,
				incomingSequence ) ) % kPhysicalRingCapacity;
		}

		unsigned int PendingSpan( void ) const
		{
			if ( !hasHead )
				return 0;
			unsigned int span = 0;
			for ( unsigned int offset = 0; offset < kPhysicalRingCapacity;
				++offset )
			{
				const unsigned int index = ( headIndex + offset ) %
					kPhysicalRingCapacity;
				if ( occupied[ index ] )
					span = offset + 1;
			}
			return span;
		}

		void RecordMeasurementOutcome( uint64_t retiredSequence,
			bool delivered, double residenceSeconds )
		{
			if ( !measurementActive || measurementComplete ||
				!IsValidSequence( retiredSequence ) )
				return;
			const uint64_t offset = SequenceDistance( measurementStartSequence,
				retiredSequence );
			if ( offset < kMeasurementWarmupCount ||
				offset >= kMeasurementTotalCount ||
				measurementOutcomeCount >= kMeasurementEligibleCount )
				return;

			++measurementOutcomeCount;
			if ( !delivered )
				return;
			++measurementDeliveredCount;
			const uint16_t bin = ResidenceHistogramBin( residenceSeconds );
			const double nonnegativeResidence = residenceSeconds > 0.0
				? residenceSeconds : 0.0;
			const double scaledMicros = nonnegativeResidence * 1000000.0;
			const uint32_t micros = scaledMicros >=
				(double)kNoResidenceMicros - 1.0
				? kNoResidenceMicros - 1 : (uint32_t)scaledMicros;
			++measurementResidenceHistogram[ bin ];
			++measurementResidenceSampleCount;
			measurementResidenceSumSeconds += (double)micros / 1000000.0;
		}

		void RecordMeasurementMissing( uint64_t firstSequence,
			uint64_t count )
		{
			if ( !measurementActive || measurementComplete || count == 0 )
				return;

			uint64_t offset = SequenceDistance( measurementStartSequence,
				firstSequence );
			uint64_t available = count;
			if ( offset >= kMeasurementTotalCount )
			{
				if ( !SequenceIsBefore( firstSequence,
					measurementStartSequence ) )
					return;
				const uint64_t toStart = SequenceDistance( firstSequence,
					measurementStartSequence );
				if ( available <= toStart )
					return;
				available -= toStart;
				offset = 0;
			}
			if ( offset >= kMeasurementTotalCount )
				return;
			const uint64_t toEnd = kMeasurementTotalCount - offset;
			if ( available > toEnd )
				available = toEnd;

			uint64_t formalStart = offset;
			if ( formalStart < kMeasurementWarmupCount )
			{
				const uint64_t warmupSkip = kMeasurementWarmupCount -
					formalStart;
				if ( available <= warmupSkip )
					return;
				available -= warmupSkip;
				formalStart = kMeasurementWarmupCount;
			}
			const uint64_t formalCapacity = kMeasurementTotalCount -
				formalStart;
			if ( available > formalCapacity )
				available = formalCapacity;
			const uint64_t remaining = kMeasurementEligibleCount -
				measurementOutcomeCount;
			if ( available > remaining )
				available = remaining;
			measurementOutcomeCount += (unsigned int)available;
		}

		void UpdateMeasurementCompletion( void )
		{
			// Head advancement is monotonic within a generation. Comparing the
			// forward distance from the generation start also handles a large
			// bounded fast-forward and the UINT64_MAX -> 1 wrap; the half-range
			// ordering helper alone would be ambiguous after a large gap.
			if ( measurementActive && hasHead &&
				SequenceDistance( measurementStartSequence, headSeq ) >=
					kMeasurementTotalCount )
				measurementComplete = true;
		}

		void RecordOutcome( bool delivered, double residenceSeconds )
		{
			if ( outcomeCount == kRollingOutcomeCapacity )
			{
				if ( outcomes[ outcomeHead ] )
					--deliveredCount;
				const uint16_t oldBin = residenceBins[ outcomeHead ];
				if ( oldBin != kNoResidenceHistogramBin )
				{
					if ( residenceHistogram[ oldBin ] > 0 )
						--residenceHistogram[ oldBin ];
					if ( residenceSampleCount > 0 )
						--residenceSampleCount;
					if ( residenceMicros[ outcomeHead ] != kNoResidenceMicros )
					{
						residenceSumSeconds -= (double)residenceMicros[
							outcomeHead ] / 1000000.0;
						if ( residenceSumSeconds < 0.0 )
							residenceSumSeconds = 0.0;
					}
				}
				outcomeHead = ( outcomeHead + 1 ) %
					kRollingOutcomeCapacity;
				--outcomeCount;
			}
			const unsigned int index = ( outcomeHead + outcomeCount ) %
				kRollingOutcomeCapacity;
			outcomes[ index ] = delivered ? 1 : 0;
			residenceBins[ index ] = kNoResidenceHistogramBin;
			residenceMicros[ index ] = kNoResidenceMicros;
			if ( delivered )
			{
				++deliveredCount;
				const uint16_t bin = ResidenceHistogramBin( residenceSeconds );
				const double nonnegativeResidence = residenceSeconds > 0.0
					? residenceSeconds : 0.0;
				const double scaledMicros = nonnegativeResidence * 1000000.0;
				const uint32_t micros = scaledMicros >=
					(double)kNoResidenceMicros - 1.0
					? kNoResidenceMicros - 1
					: (uint32_t)scaledMicros;
				residenceBins[ index ] = bin;
				residenceMicros[ index ] = micros;
				++residenceHistogram[ bin ];
				++residenceSampleCount;
				residenceSumSeconds += (double)micros / 1000000.0;
			}
			++outcomeCount;
			++retiredCount;
		}

		void PopHead( double currentTime )
		{
			if ( !hasHead )
				return;
			const uint64_t retiredSequence = headSeq;
			const bool delivered = occupied[ headIndex ];
			const double residenceSeconds = delivered
				? currentTime - firstArrivalTime[ headIndex ] : 0.0;
			occupied[ headIndex ] = false;
			sequence[ headIndex ] = 0;
			firstArrivalTime[ headIndex ] = 0.0;
			RecordOutcome( delivered, residenceSeconds );
			RecordMeasurementOutcome( retiredSequence, delivered,
				residenceSeconds );
			headSeq = AdvanceSequence( headSeq );
			headIndex = ( headIndex + 1 ) % kPhysicalRingCapacity;
			UpdateMeasurementCompletion();
		}

		void FastForwardEmpty( uint64_t count )
		{
			if ( !hasHead || count == 0 )
				return;
			RecordMissing( count );
			RecordMeasurementMissing( headSeq, count );
			headSeq = AdvanceSequenceBy( headSeq, count );
			headIndex = ( headIndex +
				(unsigned int)( count % kPhysicalRingCapacity ) ) %
				kPhysicalRingCapacity;
			UpdateMeasurementCompletion();
		}

		void RecordMissing( uint64_t count )
		{
			if ( count == 0 )
				return;
			if ( count >= kRollingOutcomeCapacity )
			{
				memset( outcomes, 0, sizeof( outcomes ) );
				outcomeHead = 0;
				outcomeCount = kRollingOutcomeCapacity;
				deliveredCount = 0;
				residenceSampleCount = 0;
				residenceSumSeconds = 0.0;
				memset( residenceHistogram, 0, sizeof( residenceHistogram ) );
				for ( unsigned int i = 0; i < kRollingOutcomeCapacity; ++i )
				{
					residenceBins[ i ] = kNoResidenceHistogramBin;
					residenceMicros[ i ] = kNoResidenceMicros;
				}
				retiredCount += count;
				return;
			}
			for ( uint64_t i = 0; i < count; ++i )
				RecordOutcome( false, 0.0 );
		}

		void DrainAvailable( double currentTime )
		{
			while ( hasHead && occupied[ headIndex ] )
				PopHead( currentTime );
		}

		bool occupied[ 128 ];
		double firstArrivalTime[ 128 ];
		uint64_t sequence[ 128 ];
		uint8_t outcomes[ 2000 ];
		uint16_t residenceBins[ 2000 ];
		uint32_t residenceMicros[ 2000 ];
		unsigned int residenceHistogram[ 256 ];
		bool hasHead;
		uint64_t headSeq;
		unsigned int headIndex;
		unsigned int logicalCapacity;
		unsigned int outcomeHead;
		unsigned int outcomeCount;
		unsigned int deliveredCount;
		unsigned int residenceSampleCount;
		double residenceSumSeconds;
		uint64_t retiredCount;
		// The rolling arrays above remain bounded diagnostics. The fields below
		// are the formal, sequence-aligned generation used by window selection;
		// they are reset together for every candidate and D_current snapshot.
		bool measurementActive;
		bool measurementComplete;
		uint64_t measurementStartSequence;
		uint64_t measurementEndSequence;
		unsigned int measurementOutcomeCount;
		unsigned int measurementDeliveredCount;
		unsigned int measurementResidenceSampleCount;
		double measurementResidenceSumSeconds;
		unsigned int measurementResidenceHistogram[ 256 ];
	};

	struct SessionReceiveState
	{
		ASRD_GNS_Connection connection;
		uint64_t receiveEpoch;
		uint64_t receivedUnreliableSequences;
		uint64_t acceptedUnreliableBlocks;
		uint64_t acceptedReliableBlocks;
		uint64_t staleDrops;
		uint64_t packetStartUnreliableBlocks;
		uint64_t packetStartReliableBlocks;
		uint64_t packetEndUnreliableBlocks;
		uint64_t packetEndReliableBlocks;
		UnreliableReorderRing reorderRing;
		UnreliableWindowSimulator simulators[ kSimulatorCount ];
		// This simulator is deliberately separate from every candidate. It
		// measures D_current for the window actually in use under the same
		// packet/epoch stream and sequence-aligned measurement generation.
		UnreliableWindowSimulator currentSimulator;
		uint64_t measurementStartSequence;
		uint64_t measurementEndSequence;
		uint64_t measurementGeneration;
		double lastWindowSelectionTime;
		bool hasWindowSelectionTime;

		SessionReceiveState()
			: connection( ASRD_GNS_CONNECTION_INVALID ), receiveEpoch( 0 ),
				receivedUnreliableSequences( 0 ), acceptedUnreliableBlocks( 0 ),
				acceptedReliableBlocks( 0 ),
				staleDrops( 0 ), packetStartUnreliableBlocks( 0 ),
				packetStartReliableBlocks( 0 ), packetEndUnreliableBlocks( 0 ),
				packetEndReliableBlocks( 0 ), measurementStartSequence( 0 ),
				measurementEndSequence( 0 ), measurementGeneration( 0 ),
				lastWindowSelectionTime( 0.0 ), hasWindowSelectionTime( false )
		{
			Reset( ASRD_GNS_CONNECTION_INVALID );
		}

		void Reset( ASRD_GNS_Connection newConnection )
		{
			connection = newConnection;
			receiveEpoch = 0;
			receivedUnreliableSequences = 0;
			acceptedUnreliableBlocks = 0;
			acceptedReliableBlocks = 0;
			staleDrops = 0;
			packetStartUnreliableBlocks = 0;
			packetStartReliableBlocks = 0;
			packetEndUnreliableBlocks = 0;
			packetEndReliableBlocks = 0;
			measurementStartSequence = 0;
			measurementEndSequence = 0;
			measurementGeneration = 0;
			lastWindowSelectionTime = 0.0;
			hasWindowSelectionTime = false;

			reorderRing.Reset();
			for ( unsigned int i = 0; i < kSimulatorCount; ++i )
				simulators[ i ].Reset( kCandidateCapacities[ i ] );
			currentSimulator.Reset( kDefaultLogicalCapacity );
			if ( newConnection != ASRD_GNS_CONNECTION_INVALID )
			{
				// The sender's first U sequence is always 1. Keep a valid
				// receive session anchored there so an initial seq=2 waits for
				// seq=1 instead of redefining the stream head.
				reorderRing.Initialize( 1 );
				for ( unsigned int i = 0; i < kSimulatorCount; ++i )
					simulators[ i ].Initialize( 1 );
				currentSimulator.Initialize( 1 );
				BeginMeasurement( 1 );
			}
		}

		void BeginMeasurement( uint64_t firstSequence )
		{
			measurementStartSequence = firstSequence;
			measurementEndSequence = IsValidSequence( firstSequence )
				? AdvanceSequenceBy( firstSequence,
					kMeasurementTotalCount - 1 ) : 0;
			++measurementGeneration;
			for ( unsigned int i = 0; i < kSimulatorCount; ++i )
				simulators[ i ].BeginMeasurement( firstSequence );
			currentSimulator.BeginMeasurement( firstSequence );
		}

		bool MeasurementComplete( void ) const
		{
			for ( unsigned int i = 0; i < kSimulatorCount; ++i )
			{
				if ( !simulators[ i ].MeasurementComplete() )
					return false;
			}
			return currentSimulator.MeasurementComplete();
		}

		void BeginNextMeasurement( void )
		{
			if ( connection == ASRD_GNS_CONNECTION_INVALID )
				return;
			uint64_t firstSequence = 0;
			for ( unsigned int i = 0; i < kSimulatorCount; ++i )
			{
				if ( !simulators[ i ].HasHead() )
					continue;
				if ( !IsValidSequence( firstSequence ) ||
					SequenceIsBefore( firstSequence,
						simulators[ i ].HeadSequence() ) )
					firstSequence = simulators[ i ].HeadSequence();
			}
			if ( currentSimulator.HasHead() &&
				( !IsValidSequence( firstSequence ) ||
				SequenceIsBefore( firstSequence,
					currentSimulator.HeadSequence() ) ) )
				firstSequence = currentSimulator.HeadSequence();
			if ( IsValidSequence( firstSequence ) )
				BeginMeasurement( firstSequence );
		}
	};

	static SessionReceiveState s_receiveState;
	static float s_requestedWindowRatio = 0.0f;
	static bool s_windowConfigDirty = true;
	static void LogBridge( const char *format, ... );

	struct SessionSendState
	{
		ASRD_GNS_Connection connection;
		bool inUse;
		uint64_t nextUnreliableSeq;

		SessionSendState()
			: connection( ASRD_GNS_CONNECTION_INVALID ), inUse( false ),
				nextUnreliableSeq( 1 )
		{
		}
	};

	static SessionSendState s_sendSessions[ 8 ];

	static bool NextUnreliableSequence( ASRD_GNS_Connection connection,
		uint64_t *sequence )
	{
		if ( !sequence || connection == ASRD_GNS_CONNECTION_INVALID )
			return false;

		SessionSendState *freeState = NULL;
		for ( unsigned int i = 0; i < ARRAYSIZE( s_sendSessions ); ++i )
		{
			if ( s_sendSessions[ i ].inUse &&
				s_sendSessions[ i ].connection == connection )
			{
				freeState = &s_sendSessions[ i ];
				break;
			}
			if ( !s_sendSessions[ i ].inUse && !freeState )
				freeState = &s_sendSessions[ i ];
		}
		if ( !freeState )
			return false;
		if ( !freeState->inUse )
		{
			freeState->inUse = true;
			freeState->connection = connection;
			freeState->nextUnreliableSeq = 1;
		}
		if ( !IsValidSequence( freeState->nextUnreliableSeq ) )
			return false;
		*sequence = freeState->nextUnreliableSeq;
		freeState->nextUnreliableSeq = AdvanceSequence(
			freeState->nextUnreliableSeq );
		return true;
	}

	static void ResetSendSession( ASRD_GNS_Connection connection )
	{
		for ( unsigned int i = 0; i < ARRAYSIZE( s_sendSessions ); ++i )
		{
			if ( connection == ASRD_GNS_CONNECTION_INVALID ||
				s_sendSessions[ i ].connection == connection )
				s_sendSessions[ i ] = SessionSendState();
		}
	}

	static void ResetReceiveState( ASRD_GNS_Connection connection )
	{
		s_receiveState.Reset( connection );
		s_windowConfigDirty = true;
	}

	static void LogPacketLifecycleSummary( ASRD_GNS_Connection connection )
	{
		if ( s_receiveState.acceptedUnreliableBlocks == 0 &&
			s_receiveState.acceptedReliableBlocks == 0 &&
			s_receiveState.staleDrops == 0 &&
			s_receiveState.packetStartUnreliableBlocks == 0 &&
			s_receiveState.packetStartReliableBlocks == 0 &&
			s_receiveState.packetEndUnreliableBlocks == 0 &&
			s_receiveState.packetEndReliableBlocks == 0 )
			return;

		LogBridge( "event=packet_lifecycle result=session_summary role=client "
			"handle=%lu accepted_unreliable=%llu accepted_reliable=%llu "
			"packet_start_ok_unreliable=%llu packet_start_ok_reliable=%llu "
			"packet_end_ok_unreliable=%llu packet_end_ok_reliable=%llu "
			"stale_drops=%llu",
			(unsigned long)connection,
			(unsigned long long)s_receiveState.acceptedUnreliableBlocks,
			(unsigned long long)s_receiveState.acceptedReliableBlocks,
			(unsigned long long)s_receiveState.packetStartUnreliableBlocks,
			(unsigned long long)s_receiveState.packetStartReliableBlocks,
			(unsigned long long)s_receiveState.packetEndUnreliableBlocks,
			(unsigned long long)s_receiveState.packetEndReliableBlocks,
			(unsigned long long)s_receiveState.staleDrops );
	}

	static void RecordAcceptedBlock( uint8_t lane )
	{
		if ( lane == ASRD_GNS_LANE_R )
			++s_receiveState.acceptedReliableBlocks;
		else
			++s_receiveState.acceptedUnreliableBlocks;
	}

	static void RecordPacketStartSuccess( uint8_t lane )
	{
		if ( lane == ASRD_GNS_LANE_R )
			++s_receiveState.packetStartReliableBlocks;
		else
			++s_receiveState.packetStartUnreliableBlocks;
	}

	static void RecordPacketEndSuccess( uint8_t lane )
	{
		if ( lane == ASRD_GNS_LANE_R )
			++s_receiveState.packetEndReliableBlocks;
		else
			++s_receiveState.packetEndUnreliableBlocks;
	}

	struct SerializedEngineBlock
	{
		std::vector<unsigned char> payload;
		unsigned int bitLength;
		unsigned int payloadBytes;
		uint8_t type;
		ASRD_GNS_BlockReliability reliability;
		uint8_t lane;
		uint8_t provenance;
		uint16_t flags;
		uint64_t unreliableSeq;

		SerializedEngineBlock()
			: bitLength( 0 ), payloadBytes( 0 ), type( ASRD_GNS_MESSAGE_RAW_STREAM_TYPE ),
				reliability( ASRD_GNS_BLOCK_RELIABILITY_UNKNOWN ),
			lane( ASRD_GNS_LANE_UNKNOWN ), provenance( ASRD_GNS_PROVENANCE_UNKNOWN ),
			flags( 0 ), unreliableSeq( 0 )
		{
		}
	};

	struct PendingBlock
	{
		SerializedEngineBlock block;
		uint64_t sourceOrder;
	};

	struct PendingServerUpdate
	{
		bool open;
		ASRD_GNS_Connection connection;

		std::vector<PendingBlock> blocks;

		PendingServerUpdate()
			: open( false ), connection( ASRD_GNS_CONNECTION_INVALID )
		{
		}
	};

	static PendingServerUpdate s_pendingServerUpdate;

	static const char *RoleName( bool serverRole )
	{
		return serverRole ? "server" : "client";
	}

	static void LogBridge( const char *format, ... )
	{
		char buffer[ 768 ];
		va_list args;
		va_start( args, format );
		_vsnprintf( buffer, sizeof( buffer ) - 1, format, args );
		buffer[ sizeof( buffer ) - 1 ] = '\0';
		va_end( args );
		Warning( "[ASRD-GNS-BRIDGE] %s\n", buffer );
	}

	static ASRD_GNS_Connection MappedConnection( bool serverRole, bool *mapped )
	{
		if ( mapped )
			*mapped = false;

		if ( serverRole )
		{
			if ( ASRD_GNS_ServerState() != ASRD_GNS_SERVER_CONNECTED )
				return ASRD_GNS_CONNECTION_INVALID;
			const ASRD_GNS_Connection connection = ASRD_GNS_ServerConnection();
			if ( mapped )
				*mapped = connection != ASRD_GNS_CONNECTION_INVALID;
			return connection;
		}

		if ( ASRD_GNS_ClientState() != ASRD_GNS_CLIENT_CONNECTED )
			return ASRD_GNS_CONNECTION_INVALID;
		const ASRD_GNS_Connection connection = ASRD_GNS_ClientConnection();
		if ( mapped )
			*mapped = connection != ASRD_GNS_CONNECTION_INVALID;
		return connection;
	}

	static unsigned int PayloadBytesForBits( unsigned int bitLength )
	{
		return bitLength / 8U + ( bitLength % 8U != 0 ? 1U : 0U );
	}

	static bool NextCounter( std::atomic<uint64_t> &counter, uint64_t *value )
	{
		if ( !value )
			return false;

		uint64_t current = counter.load( std::memory_order_relaxed );
		for ( ;; )
		{
			if ( current == 0 || current ==
				std::numeric_limits<uint64_t>::max() )
				return false;
			if ( counter.compare_exchange_weak( current, current + 1,
				std::memory_order_relaxed, std::memory_order_relaxed ) )
			{
				*value = current;
				return true;
			}
		}
	}

	static bool IsRealtimeProvenanceMatch(
		ASRD_GNS_BlockProvenance provenance )
	{
		return provenance == ASRD_GNS_PROVENANCE_SNAPSHOT;
	}

	static bool IsRealtimeTypeWhitelistMatch( uint8_t type,
		ASRD_GNS_BlockProvenance provenance )
	{
		if ( type == ASRD_GNS_MESSAGE_RAW_STREAM_TYPE )
			return false;

		switch ( type )
		{
		case clc_Move:
			return provenance == ASRD_GNS_PROVENANCE_SEND_NETMSG;
		case svc_TempEntities:
			return provenance == ASRD_GNS_PROVENANCE_SEND_NETMSG ||
				provenance == ASRD_GNS_PROVENANCE_TEMP_ENTITIES;
		case svc_EntityMessage:
			return provenance == ASRD_GNS_PROVENANCE_ENTITY_MESSAGE;
		case svc_Sounds:
			return provenance == ASRD_GNS_PROVENANCE_SOUND;
		case svc_UserMessage:
			return provenance == ASRD_GNS_PROVENANCE_FX_USER_MESSAGE;
		default:
			return false;
		}
	}

	static ASRD_GNS_BlockProvenance InferMessageProvenance( uint8_t type,
		ASRD_GNS_BlockProvenance supplied )
	{
		if ( supplied != ASRD_GNS_PROVENANCE_SEND_NETMSG )
			return supplied;

		// Only the callback provenance is authoritative. In particular, do not
		// infer EntityMessage or Sounds realtime status from the wire type: those
		// classifications require their fixed callsite/filter evidence.
		(void)type;
		return supplied;
	}

	static bool BuildEnvelope( const SerializedEngineBlock &block,
		uint8_t direction, uint32_t serverUpdateSeq, int clientCommandAck,
		std::vector<unsigned char> *out )
	{
		const bool reliable = block.lane == ASRD_GNS_LANE_R;
		if ( !out || block.lane > ASRD_GNS_LANE_U_NORMAL ||
			( reliable ? block.unreliableSeq != 0 : block.unreliableSeq == 0 ) ||
			( block.payloadBytes > 0 && block.payload.empty() ) ||
			(size_t)block.payloadBytes != block.payload.size() ||
			block.payloadBytes > ASRD_GNS_MESSAGE_MAX_PAYLOAD_BYTES ||
			block.bitLength > block.payloadBytes * 8U ||
			block.payloadBytes != PayloadBytesForBits( block.bitLength ) ||
			direction > ASRD_GNS_DIRECTION_SERVER_TO_CLIENT )
			return false;

		const size_t totalBytes = sizeof( MessageEnvelopeHeader ) +
			(size_t)block.payloadBytes;
		if ( totalBytes > ASRD_GNS_MESSAGE_MAX_ENVELOPE_BYTES )
			return false;

		out->resize( totalBytes );
		MessageEnvelopeHeader header = {};
		header.magic = ASRD_GNS_MESSAGE_ENVELOPE_MAGIC;
		header.version = ASRD_GNS_MESSAGE_ENVELOPE_VERSION;
		header.headerBytes = (uint16_t)sizeof( MessageEnvelopeHeader );
		header.unreliableSeq = block.unreliableSeq;
		header.bitLength = block.bitLength;
		header.payloadBytes = block.payloadBytes;
		header.serverUpdateSeq = serverUpdateSeq;
		header.clientCommandAck = (int32_t)clientCommandAck;
		header.direction = direction;
		header.type = block.type;
		header.lane = block.lane;
		header.provenance = block.provenance;
		header.flags = block.flags;
		if ( direction == ASRD_GNS_DIRECTION_SERVER_TO_CLIENT )
			header.flags |= ASRD_GNS_ENVELOPE_FLAG_PACKET_CONTEXT;

		memcpy( &(*out)[ 0 ], &header, sizeof( header ) );
		if ( block.payloadBytes > 0 )
			memcpy( &(*out)[ sizeof( header ) ], &block.payload[ 0 ],
				block.payloadBytes );
		return true;
	}

	static bool ClassifyBlock( SerializedEngineBlock *block )
	{
		if ( !block )
			return false;
		if ( block->reliability == ASRD_GNS_BLOCK_RELIABILITY_UNKNOWN )
		{
			block->lane = ASRD_GNS_LANE_UNKNOWN;
			return false;
		}

		const bool reliable = block->reliability ==
			ASRD_GNS_BLOCK_RELIABILITY_RELIABLE;
		const ASRD_GNS_BlockProvenance provenance =
			(ASRD_GNS_BlockProvenance)block->provenance;
		const bool realtime = !reliable &&
			( IsRealtimeProvenanceMatch( provenance ) ||
				IsRealtimeTypeWhitelistMatch( block->type, provenance ) );
		block->lane = reliable ? ASRD_GNS_LANE_R
			: realtime ? ASRD_GNS_LANE_U_REALTIME
			: ASRD_GNS_LANE_U_NORMAL;
		block->flags = 0;
		if ( reliable )
			block->flags |= ASRD_GNS_ENVELOPE_FLAG_RELIABLE;
		if ( realtime )
			block->flags |= ASRD_GNS_ENVELOPE_FLAG_REALTIME;
		if ( block->provenance == ASRD_GNS_PROVENANCE_VOICE )
			block->flags |= ASRD_GNS_ENVELOPE_FLAG_VOICE;
		if ( block->type == ASRD_GNS_MESSAGE_RAW_STREAM_TYPE )
			block->flags |= ASRD_GNS_ENVELOPE_FLAG_RAW;
		return true;
	}

	static bool CaptureBlock( const void *payload, unsigned int bitLength,
		unsigned int payloadBytes, uint8_t type,
		ASRD_GNS_BlockReliability reliability,
		ASRD_GNS_BlockProvenance provenance, SerializedEngineBlock *out )
	{
		if ( !out || ( payloadBytes > 0 && !payload ) ||
			payloadBytes > ASRD_GNS_MESSAGE_MAX_PAYLOAD_BYTES ||
			bitLength > payloadBytes * 8U ||
			payloadBytes != PayloadBytesForBits( bitLength ) ||
			( type != ASRD_GNS_MESSAGE_RAW_STREAM_TYPE && type > 63 ) )
			return false;

		out->payload.assign( payloadBytes, 0 );
		if ( payloadBytes > 0 )
			memcpy( &out->payload[ 0 ], payload, payloadBytes );
		out->bitLength = bitLength;
		out->payloadBytes = payloadBytes;
		out->type = type;
		out->reliability = reliability;
		out->provenance = (uint8_t)InferMessageProvenance( type,
			provenance );
		out->unreliableSeq = 0;
		if ( reliability == ASRD_GNS_BLOCK_RELIABILITY_UNKNOWN )
		{
			out->lane = ASRD_GNS_LANE_UNKNOWN;
			out->flags = type == ASRD_GNS_MESSAGE_RAW_STREAM_TYPE
				? ASRD_GNS_ENVELOPE_FLAG_RAW : 0;
			return true;
		}
		return ClassifyBlock( out );
	}

	static bool SubmitBlock( ASRD_GNS_Connection connection,
		SerializedEngineBlock *block, uint8_t direction,
		uint32_t serverUpdateSeq, int clientCommandAck )
	{
		if ( !block )
			return false;
		if ( ( block->lane == ASRD_GNS_LANE_U_REALTIME ||
			block->lane == ASRD_GNS_LANE_U_NORMAL ) &&
			block->unreliableSeq == 0 &&
			!NextUnreliableSequence( connection, &block->unreliableSeq ) )
		{
			LogBridge( "event=submit result=error reason=unreliable_sequence_exhausted handle=%lu lane=%u",
				(unsigned long)connection, (unsigned)block->lane );
			return false;
		}
		if ( block->lane == ASRD_GNS_LANE_R )
			block->unreliableSeq = 0;

		std::vector<unsigned char> envelope;
		if ( !BuildEnvelope( *block, direction, serverUpdateSeq,
			clientCommandAck, &envelope ) )
		{
			LogBridge( "event=submit result=error reason=envelope_invalid direction=%u type=%u sourceBitLength=%u payloadBytes=%u envelopeBytes=%u lane=%u unreliable_seq=%llu EResult=%d",
				(unsigned)direction, (unsigned)block->type, block->bitLength,
				block->payloadBytes,
				(unsigned)( sizeof( MessageEnvelopeHeader ) + block->payloadBytes ),
				(unsigned)block->lane, (unsigned long long)block->unreliableSeq,
				ASRD_GNS_RESULT_INVALID_PARAM );
			return false;
		}

		const int sendFlags = block->lane == ASRD_GNS_LANE_R
			? ASRD_GNS_SEND_RELIABLE
			: block->lane == ASRD_GNS_LANE_U_REALTIME
				? ASRD_GNS_SEND_UNRELIABLE_NO_NAGLE
				: ASRD_GNS_SEND_UNRELIABLE;
		const int result = ASRD_GNS_SendLane( connection, &envelope[ 0 ],
			(unsigned int)envelope.size(), block->lane, sendFlags );
		if ( result != ASRD_GNS_RESULT_OK )
		{
			LogBridge( "event=submit result=error direction=%u type=%u sourceBitLength=%u payloadBytes=%u envelopeBytes=%u lane=%u unreliable_seq=%llu EResult=%d",
				(unsigned)direction, (unsigned)block->type, block->bitLength,
				block->payloadBytes, (unsigned)envelope.size(), (unsigned)block->lane,
				(unsigned long long)block->unreliableSeq, result );
		}
		return result == ASRD_GNS_RESULT_OK;
	}

	static void ResetPendingServerUpdate( void )
	{
		s_pendingServerUpdate.open = false;
		s_pendingServerUpdate.connection = ASRD_GNS_CONNECTION_INVALID;
		s_pendingServerUpdate.blocks.clear();
	}

	static bool EnsurePendingServerUpdate( ASRD_GNS_Connection connection )
	{
		if ( connection == ASRD_GNS_CONNECTION_INVALID )
			return false;

		if ( s_pendingServerUpdate.open &&
			s_pendingServerUpdate.connection != connection )
		{
			LogBridge( "event=server_update result=error reason=connection_changed old_handle=%lu new_handle=%lu action=discard_pending",
				(unsigned long)s_pendingServerUpdate.connection,
				(unsigned long)connection );
			ResetPendingServerUpdate();
		}

		if ( !s_pendingServerUpdate.open )
		{
			s_pendingServerUpdate.connection = connection;
			s_pendingServerUpdate.open = true;
		}
		return true;
	}

	static bool AppendPendingServerBlock( ASRD_GNS_Connection connection,
		const void *payload, unsigned int bitLength, unsigned int payloadBytes,
		uint8_t type, ASRD_GNS_BlockReliability reliability,
		ASRD_GNS_BlockProvenance provenance )
	{
		if ( connection == ASRD_GNS_CONNECTION_INVALID ||
			( payloadBytes > 0 && !payload ) )
			return false;
		if ( bitLength == 0 )
			return true;
		if ( !EnsurePendingServerUpdate( connection ) )
			return false;

		PendingBlock pending = {};
		if ( !CaptureBlock( payload, bitLength, payloadBytes, type,
			reliability, provenance, &pending.block ) )
		{
			LogBridge( "event=server_update result=error reason=capture_failed handle=%lu type=%u bitLength=%u payloadBytes=%u",
				(unsigned long)connection, (unsigned)type, bitLength,
				payloadBytes );
			ResetPendingServerUpdate();
			return false;
		}
		if ( !NextCounter( s_nextSourceOrder, &pending.sourceOrder ) )
		{
			LogBridge( "event=server_update result=error reason=source_order_overflow handle=%lu unreliable_seq=%llu",
				(unsigned long)connection,
				(unsigned long long)pending.block.unreliableSeq );
			ResetPendingServerUpdate();
			return false;
		}
		s_pendingServerUpdate.blocks.push_back( pending );
		return true;
	}

	static bool ParseEnvelope( const std::vector<unsigned char> &bytes,
		MessageEnvelopeHeader *header, const unsigned char **payload )
	{
		if ( !header || !payload || bytes.size() < sizeof( MessageEnvelopeHeader ) )
			return false;

		memcpy( header, &bytes[ 0 ], sizeof( *header ) );
		if ( header->magic != ASRD_GNS_MESSAGE_ENVELOPE_MAGIC ||
			header->version != ASRD_GNS_MESSAGE_ENVELOPE_VERSION ||
			header->headerBytes != sizeof( MessageEnvelopeHeader ) ||
			header->payloadBytes > ASRD_GNS_MESSAGE_MAX_PAYLOAD_BYTES ||
			header->bitLength > header->payloadBytes * 8U ||
			header->payloadBytes != PayloadBytesForBits( header->bitLength ) ||
			header->direction > ASRD_GNS_DIRECTION_SERVER_TO_CLIENT ||
			header->lane > ASRD_GNS_LANE_U_NORMAL ||
			( header->type != ASRD_GNS_MESSAGE_RAW_STREAM_TYPE &&
				header->type > 63 ) ||
			( header->lane == ASRD_GNS_LANE_R
				? header->unreliableSeq != 0 : header->unreliableSeq == 0 ) ||
			( header->flags & ASRD_GNS_ENVELOPE_FLAG_RELIABLE ) !=
				( header->lane == ASRD_GNS_LANE_R
					? ASRD_GNS_ENVELOPE_FLAG_RELIABLE : 0 ) ||
			( header->flags & ASRD_GNS_ENVELOPE_FLAG_REALTIME ) !=
				( header->lane == ASRD_GNS_LANE_U_REALTIME
					? ASRD_GNS_ENVELOPE_FLAG_REALTIME : 0 ) ||
			( header->flags & ASRD_GNS_ENVELOPE_FLAG_RAW ) !=
				( header->type == ASRD_GNS_MESSAGE_RAW_STREAM_TYPE
					? ASRD_GNS_ENVELOPE_FLAG_RAW : 0 ) ||
			( header->flags & ASRD_GNS_ENVELOPE_FLAG_VOICE ) !=
				( header->provenance == ASRD_GNS_PROVENANCE_VOICE
					? ASRD_GNS_ENVELOPE_FLAG_VOICE : 0 ) ||
			( header->flags & ASRD_GNS_ENVELOPE_FLAG_PACKET_CONTEXT ) !=
				( header->direction == ASRD_GNS_DIRECTION_SERVER_TO_CLIENT
					? ASRD_GNS_ENVELOPE_FLAG_PACKET_CONTEXT : 0 ) ||
			sizeof( *header ) + (size_t)header->payloadBytes != bytes.size() )
			return false;

		*payload = header->payloadBytes > 0
			? &bytes[ sizeof( *header ) ] : NULL;
		return true;
	}

	static bool DispatchSourceMessage( bool serverRole,
		ASRD_GNS_Connection connection, int type, bf_read &reader,
		int availableBits, bool *processed )
	{
		if ( processed )
			*processed = false;

		// A control opcode has no project-local message object. NOP is padding;
		// all other control messages remain fail-closed because their legacy
		// channel side effects are not part of the GNS compatibility layer.
		if ( type <= net_File )
		{
			if ( type == net_NOP )
				return true;
			LogBridge( "event=dispatch result=error role=%s handle=%lu reason=unsupported_control type=%d",
				RoleName( serverRole ), (unsigned long)connection, type );
			return false;
		}

		ASRD_GNS_MessageRegistration registration = {};
		if ( !ASRD_GNS_MessageRegistryLookup( type, &registration ) ||
			!registration.message || !registration.channel )
		{
			LogBridge( "event=dispatch result=error role=%s handle=%lu reason=registry_miss type=%d read_bits=%d registry_count=%u",
				RoleName( serverRole ), (unsigned long)connection, type,
				reader.GetNumBitsRead(), ASRD_GNS_MessageRegistryCount() );
			return false;
		}

		ASRD_GNS_MoveMetadata move = {};
		ASRD_GNS_ServerConnectionContext *serverContext = NULL;
		bool isMove = false;
		if ( serverRole && type == clc_Move )
		{
			isMove = true;
			serverContext = ASRD_GNS_ServerContextForConnection( connection );
			if ( !serverContext || !ASRD_GNS_ParseCLCMoveBody( &reader,
				availableBits, &move ) )
			{
				LogBridge( "event=dispatch result=error role=server handle=%lu reason=clc_move_metadata_parse_failed available_bits=%d",
					(unsigned long)connection, availableBits );
				return false;
			}

			// A move that arrives after another unreliable block is not filtered
			// here. It is an ordinary network loss/order outcome; Source command
			// progression and the scoped drop/ACK compatibility path handle it.
		}

		INetMessage *message = static_cast<INetMessage *>( registration.message );
		INetChannel *channel = static_cast<INetChannel *>( registration.channel );
		message->SetNetChannel( channel );
		const int messageStartBit = reader.GetNumBitsRead();

		// CLC_Move::ReadFromBuffer stores its input reader in m_DataIn for the
		// later ProcessUsercmds call.  Give that call a reader whose absolute end
		// is the declared CLC_Move payload end; passing the outer stream directly
		// would let a malformed command decoder see the next Source message.
		bf_read boundedMoveReader;
		bf_read *messageReader = &reader;
		const int messageEndBit = isMove
			? messageStartBit + move.consumed_bits
			: messageStartBit;
		if ( isMove )
		{
			boundedMoveReader.StartReading( reader.GetBasePointer(),
				reader.TotalBytesAvailable(), messageStartBit, messageEndBit );
			messageReader = &boundedMoveReader;
		}

		const bool readResult = message->ReadFromBuffer( *messageReader );
		if ( !readResult || messageReader->IsOverflowed() ||
			( isMove && messageReader->GetNumBitsRead() != messageEndBit ) )
		{
			LogBridge( "event=dispatch result=error role=%s handle=%lu reason=read_failed type=%d start_bits=%d end_bits=%d overflow=%u",
				RoleName( serverRole ), (unsigned long)connection, type,
				messageStartBit, messageReader->GetNumBitsRead(),
				messageReader->IsOverflowed() ? 1U : 0U );
			return false;
		}
		if ( isMove && ( !reader.SeekRelative( move.consumed_bits ) ||
			reader.IsOverflowed() ) )
		{
			LogBridge( "event=dispatch result=error role=%s handle=%lu reason=clc_move_outer_reader_advance_failed consumed_bits=%d",
				RoleName( serverRole ), (unsigned long)connection,
				move.consumed_bits );
			return false;
		}

		int droppedCommands = 0;
		if ( isMove )
			droppedCommands = ASRD_GNS_ServerComputeMoveDrop(
				serverContext, move );
		if ( isMove )
			ASRD_GNS_ServerBeginMoveProcess( serverContext, droppedCommands );

		if ( serverRole && ( type == 4 || type == clc_Move ) )
			ASRD_GNS_ServerTraceClientMessage( type,
				(unsigned int)reader.GetNumBitsLeft() );
		const bool processResult = message->Process();
		if ( isMove )
			ASRD_GNS_ServerFinishMoveProcess( serverContext, move,
				processResult );
		if ( !processResult )
		{
			LogBridge( "event=dispatch result=error role=%s handle=%lu reason=process_failed type=%d read_bits=%d",
				RoleName( serverRole ), (unsigned long)connection, type,
				reader.GetNumBitsRead() );
			return false;
		}

		if ( processed )
			*processed = true;
		return true;
	}

	static bool DispatchRawStream( bool serverRole, ASRD_GNS_Connection connection,
		const unsigned char *payload, unsigned int payloadBytes, unsigned int bitLength,
		unsigned int *delivered )
	{
		if ( delivered )
			*delivered = 0;
		if ( !payload && payloadBytes > 0 )
			return false;

		bf_read reader( payload, (int)payloadBytes, (int)bitLength );
		unsigned int dispatched = 0;
		while ( reader.GetNumBitsLeft() >= 6 )
		{
			const int type = (int)reader.ReadUBitLong( 6 );
			if ( reader.IsOverflowed() )
				break;

			bool processed = false;
			if ( !DispatchSourceMessage( serverRole, connection, type, reader,
				reader.GetNumBitsLeft(), &processed ) )
				return false;
			if ( processed )
				++dispatched;
		}

		if ( reader.IsOverflowed() )
		{
			LogBridge( "event=raw_dispatch result=error role=%s handle=%lu reason=stream_overflow read_bits=%d bitlen=%u dispatched=%u",
				RoleName( serverRole ), (unsigned long)connection,
				reader.GetNumBitsRead(), bitLength, dispatched );
			return false;
		}

		if ( delivered )
			*delivered = dispatched;
		return true;
	}

	static bool PopAndDispatchRingHead( bool serverRole,
		unsigned int *deliveredThisFrame );
	static bool DrainRingAvailable( bool serverRole,
		unsigned int *deliveredThisFrame );
	static bool FastForwardRingForIncoming( bool serverRole,
		uint64_t incomingSequence, unsigned int *deliveredThisFrame );

	static bool EnqueueEnvelope( ASRD_GNS_Connection connection,
		const unsigned char *data, unsigned int size, bool serverRole,
		unsigned int *deliveredThisFrame )
	{
		if ( connection == ASRD_GNS_CONNECTION_INVALID || !data ||
			size < sizeof( MessageEnvelopeHeader ) ||
			size > ASRD_GNS_MESSAGE_MAX_ENVELOPE_BYTES )
			return false;

		QueuedEnvelope *item = new ( std::nothrow ) QueuedEnvelope();
		if ( !item )
			return false;
		item->connection = connection;
		item->envelopeBytes = size;
		item->bytes.assign( data, data + size );
		item->firstArrivalTime = Plat_FloatTime();

		MessageEnvelopeHeader header = {};
		const unsigned char *payload = NULL;
		if ( !ParseEnvelope( item->bytes, &header, &payload ) )
		{
			delete item;
			return false;
		}

		if ( header.lane == ASRD_GNS_LANE_R )
		{
			s_queue.push_back( item );
			return true;
		}

		// Simulators observe every valid unreliable sequence before the real ring
		// can classify it as stale, duplicate, or an overflow victim.
		++s_receiveState.receivedUnreliableSequences;
		for ( unsigned int i = 0; i < kSimulatorCount; ++i )
			s_receiveState.simulators[ i ].Observe( header.unreliableSeq,
				item->firstArrivalTime );
		s_receiveState.currentSimulator.Observe( header.unreliableSeq,
			item->firstArrivalTime );

		if ( !s_receiveState.reorderRing.HasHead() )
			s_receiveState.reorderRing.Initialize( 1 );
		if ( s_receiveState.reorderRing.IsStale( header.unreliableSeq ) )
		{
			if ( !serverRole )
				++s_receiveState.staleDrops;
			LogBridge( "event=receive result=drop_stale role=%s handle=%lu lane=%u unreliable_seq=%llu head_seq=%llu",
				RoleName( serverRole ), (unsigned long)connection,
				(unsigned)header.lane,
				(unsigned long long)header.unreliableSeq,
				(unsigned long long)s_receiveState.reorderRing.HeadSequence() );
			delete item;
			return true;
		}
		if ( !FastForwardRingForIncoming( serverRole,
			header.unreliableSeq, deliveredThisFrame ) )
		{
			delete item;
			return false;
		}

		if ( s_receiveState.reorderRing.IsOccupied( header.unreliableSeq ) )
		{
			LogBridge( "event=receive result=drop_duplicate role=%s handle=%lu lane=%u unreliable_seq=%llu",
				RoleName( serverRole ), (unsigned long)connection,
				(unsigned)header.lane,
				(unsigned long long)header.unreliableSeq );
			delete item;
			return true;
		}
		if ( !s_receiveState.reorderRing.Store( header.unreliableSeq, item,
			size, item->firstArrivalTime ) )
		{
			delete item;
			return false;
		}
		return DrainRingAvailable( serverRole, deliveredThisFrame );
	}

	static bool DispatchEnvelope( bool serverRole, const QueuedEnvelope *item,
		bool *delivered )
	{
		if ( delivered )
			*delivered = false;
		if ( !item )
			return false;
		MessageEnvelopeHeader header = {};
		const unsigned char *payload = NULL;
		if ( !ParseEnvelope( item->bytes, &header, &payload ) )
		{
			LogBridge( "event=dispatch result=error reason=invalid_envelope handle=%lu bytes=%u",
				(unsigned long)item->connection, item->envelopeBytes );
			return false;
		}
		const uint8_t expectedDirection = serverRole
			? ASRD_GNS_DIRECTION_CLIENT_TO_SERVER
			: ASRD_GNS_DIRECTION_SERVER_TO_CLIENT;
		if ( header.direction != expectedDirection ||
			( expectedDirection == ASRD_GNS_DIRECTION_SERVER_TO_CLIENT
				? header.serverUpdateSeq == 0
				: header.serverUpdateSeq != 0 ) )
		{
			LogBridge( "event=dispatch result=error reason=direction_or_context_mismatch role=%s handle=%lu direction=%u expected=%u update=%u",
				RoleName( serverRole ), (unsigned long)item->connection,
				(unsigned)header.direction, (unsigned)expectedDirection,
				(unsigned)header.serverUpdateSeq );
			return false;
		}

		bool mapped = false;
		const ASRD_GNS_Connection mappedConnection = MappedConnection( serverRole, &mapped );
		if ( !mapped || mappedConnection != item->connection )
		{
			LogBridge( "event=dispatch result=error reason=connection_map_miss role=%s handle=%lu mapped=%lu type=%d bitlen=%u",
				RoleName( serverRole ), (unsigned long)item->connection,
				(unsigned long)mappedConnection, (unsigned)header.type,
				(unsigned)header.bitLength );
			return false;
		}

		// The reorder ring is the only stale boundary for U traffic. Reliable
		// traffic is dispatched from its own FIFO and never changes that boundary.
		// Accepted is recorded only after ring admission and before lifecycle calls.
		if ( !serverRole )
			RecordAcceptedBlock( header.lane );

		bool clientPacketStarted = false;
		if ( !serverRole )
		{
			if ( !ASRD_GNS_ClientPacketStart( header.serverUpdateSeq,
				header.clientCommandAck ) )
			{
			LogBridge( "event=dispatch result=error role=client handle=%lu reason=packet_start_failed server_update_seq=%u ack=%d",
					(unsigned long)item->connection, (unsigned)header.serverUpdateSeq,
					header.clientCommandAck );
				return false;
			}
			RecordPacketStartSuccess( header.lane );
			clientPacketStarted = true;
		}

		if ( header.type == ASRD_GNS_MESSAGE_RAW_STREAM_TYPE )
		{
			unsigned int deliveredMessages = 0;
		const bool result = DispatchRawStream( serverRole, item->connection,
			payload, header.payloadBytes, header.bitLength,
			&deliveredMessages );
			if ( result )
				s_delivered += deliveredMessages;
			if ( result && clientPacketStarted )
			{
				if ( !ASRD_GNS_ClientPacketEnd() )
					return false;
				RecordPacketEndSuccess( header.lane );
			}
			if ( result && deliveredMessages > 0 && delivered )
				*delivered = true;
			return result;
		}

		// CLC_ClientInfo is the first server-side message that can reject the
		// GNS context as HLTV/replay.  Log only the current wire fields needed to
		// distinguish a bad payload from Source-side context state; do not alter
		// the message object or its processing path.
		if ( serverRole && header.type == 8 && payload && header.bitLength >= 71 )
		{
			bf_read infoReader( payload, (int)header.payloadBytes,
				(int)header.bitLength );
			const int wireType = (int)infoReader.ReadUBitLong( 6 );
			const int serverCount = infoReader.ReadLong();
			const int sendTableCrc = infoReader.ReadLong();
			const int isHltv = infoReader.ReadOneBit();
			LogBridge( "event=client_info role=server handle=%lu wire_type=%d server_count=%d sendtable_crc=%d is_hltv=%d read_bits=%d overflow=%u",
				(unsigned long)item->connection, wireType, serverCount,
				sendTableCrc, isHltv, infoReader.GetNumBitsRead(),
				infoReader.IsOverflowed() ? 1U : 0U );
		}

		bf_read reader( payload, (int)header.payloadBytes, (int)header.bitLength );
		if ( reader.GetNumBitsLeft() < 6 )
		{
			LogBridge( "event=dispatch result=error reason=missing_type role=%s handle=%lu type=%d bitlen=%u",
				RoleName( serverRole ), (unsigned long)item->connection,
				(unsigned)header.type, (unsigned)header.bitLength );
			return false;
		}
		const int wireType = (int)reader.ReadUBitLong( 6 );
		if ( reader.IsOverflowed() || wireType != header.type )
		{
			LogBridge( "event=dispatch result=error reason=type_mismatch role=%s handle=%lu envelope_type=%d wire_type=%d bitlen=%u overflow=%u",
				RoleName( serverRole ), (unsigned long)item->connection,
				(unsigned)header.type, wireType, (unsigned)header.bitLength,
				reader.IsOverflowed() ? 1U : 0U );
			return false;
		}
		bool processed = false;
		const bool result = DispatchSourceMessage( serverRole, item->connection,
			wireType, reader, reader.GetNumBitsLeft(), &processed );
		if ( result && processed )
			++s_delivered;
		if ( result && processed && delivered )
			*delivered = true;
		if ( result && clientPacketStarted )
		{
			if ( !ASRD_GNS_ClientPacketEnd() )
				return false;
			RecordPacketEndSuccess( header.lane );
		}
		return result;
	}

	static bool PopAndDispatchRingHead( bool serverRole,
		unsigned int *deliveredThisFrame )
	{
		bool hadPacket = false;
		QueuedEnvelope *item = s_receiveState.reorderRing.PopHead(
			NULL, &hadPacket );
		if ( !hadPacket )
			return true;

		bool delivered = false;
		const bool result = DispatchEnvelope( serverRole, item, &delivered );
		delete item;
		if ( !result )
			return false;
		if ( delivered && deliveredThisFrame )
			++*deliveredThisFrame;
		return true;
	}

	static bool FastForwardRingForIncoming( bool serverRole,
		uint64_t incomingSequence, unsigned int *deliveredThisFrame )
	{
		if ( !s_receiveState.reorderRing.HasHead() )
			return false;

		const uint64_t distance = SequenceDistance(
			s_receiveState.reorderRing.HeadSequence(), incomingSequence );
		const unsigned int logicalCapacity =
			s_receiveState.reorderRing.LogicalCapacity();
		if ( distance < logicalCapacity )
			return true;

		// Keep the incoming sequence at the final slot of the logical window.
		// Retire at most the fixed physical ring, then jump the remaining empty
		// gap in one modular operation. This bounds both packet work and logs.
		const uint64_t advanceCount = distance - logicalCapacity + 1;
		const uint64_t targetHead = RetreatSequenceBy( incomingSequence,
			logicalCapacity - 1 );
		uint64_t physicalRetireCount = advanceCount;
		if ( physicalRetireCount > kPhysicalRingCapacity )
			physicalRetireCount = kPhysicalRingCapacity;
		for ( uint64_t i = 0; i < physicalRetireCount; ++i )
		{
			if ( !PopAndDispatchRingHead( serverRole, deliveredThisFrame ) )
				return false;
		}

		const uint64_t skippedCount = advanceCount - physicalRetireCount;
		if ( skippedCount > 0 &&
			!s_receiveState.reorderRing.FastForwardEmpty( skippedCount ) )
			return false;
		if ( s_receiveState.reorderRing.HeadSequence() != targetHead )
			return false;
		LogBridge( "event=receive result=fast_forward role=%s handle=%lu incoming_seq=%llu distance=%llu advance=%llu retired_slots=%u skipped=%llu",
			RoleName( serverRole ),
			(unsigned long)s_receiveState.connection,
			(unsigned long long)incomingSequence,
			(unsigned long long)distance,
			(unsigned long long)advanceCount,
			(unsigned)physicalRetireCount,
			(unsigned long long)skippedCount );
		return true;
	}

	static bool DrainRingAvailable( bool serverRole,
		unsigned int *deliveredThisFrame )
	{
		while ( s_receiveState.reorderRing.HasHeadPacket() )
		{
			if ( !PopAndDispatchRingHead( serverRole, deliveredThisFrame ) )
				return false;
		}
		return true;
	}

	static bool ForcePopRingThrough( bool serverRole, uint64_t targetSequence,
		unsigned int *deliveredThisFrame )
	{
		while ( s_receiveState.reorderRing.HasHead() )
		{
			const bool isTarget = s_receiveState.reorderRing.HeadSequence() ==
				targetSequence;
			if ( !PopAndDispatchRingHead( serverRole, deliveredThisFrame ) )
				return false;
			if ( isTarget )
				return true;
		}
		return false;
	}

	static bool ApplyLogicalCapacity( bool serverRole,
		unsigned int logicalCapacity, unsigned int *deliveredThisFrame )
	{
		logicalCapacity = NormalizeLogicalCapacity( logicalCapacity );
		while ( s_receiveState.reorderRing.PendingSpan() > logicalCapacity )
		{
			if ( !PopAndDispatchRingHead( serverRole, deliveredThisFrame ) )
				return false;
		}
		s_receiveState.reorderRing.SetLogicalCapacity( logicalCapacity );
		// A shrink can expose a packet immediately at the new head after the
		// span-retirement loop. Drain it before returning so the new capacity is
		// effective for this same processing opportunity.
		if ( !DrainRingAvailable( serverRole, deliveredThisFrame ) )
			return false;

		// Keep the separately measured current-window simulator aligned with the
		// real ring whenever a manual or automatic capacity change is applied.
		s_receiveState.currentSimulator.SetLogicalCapacity( logicalCapacity,
			Plat_FloatTime() );
		return true;
	}

	static void LogWindowChange( unsigned int oldCapacity,
		unsigned int newCapacity, unsigned int sampleCount )
	{
		LogBridge( "event=unreliable_window_change old_capacity=%u new_capacity=%u sample_count=%u measurement_generation=%llu measurement_start_seq=%llu measurement_end_seq=%llu current_p95_ms=%.2f current_avg_ms=%.2f current_delivery=%.4f p95_ms_16=%.2f p95_ms_32=%.2f p95_ms_48=%.2f p95_ms_64=%.2f p95_ms_80=%.2f p95_ms_96=%.2f p95_ms_112=%.2f p95_ms_128=%.2f delivery_rate_16=%.4f delivery_rate_32=%.4f delivery_rate_48=%.4f delivery_rate_64=%.4f delivery_rate_80=%.4f delivery_rate_96=%.4f delivery_rate_112=%.4f delivery_rate_128=%.4f",
			oldCapacity, newCapacity, sampleCount,
			(unsigned long long)s_receiveState.measurementGeneration,
			(unsigned long long)s_receiveState.measurementStartSequence,
			(unsigned long long)s_receiveState.measurementEndSequence,
			s_receiveState.currentSimulator.P95ResidenceMilliseconds(),
			s_receiveState.currentSimulator.AverageResidenceMilliseconds(),
			s_receiveState.currentSimulator.DeliveryRate(),
			s_receiveState.simulators[ 0 ].P95ResidenceMilliseconds(),
			s_receiveState.simulators[ 1 ].P95ResidenceMilliseconds(),
			s_receiveState.simulators[ 2 ].P95ResidenceMilliseconds(),
			s_receiveState.simulators[ 3 ].P95ResidenceMilliseconds(),
			s_receiveState.simulators[ 4 ].P95ResidenceMilliseconds(),
			s_receiveState.simulators[ 5 ].P95ResidenceMilliseconds(),
			s_receiveState.simulators[ 6 ].P95ResidenceMilliseconds(),
			s_receiveState.simulators[ 7 ].P95ResidenceMilliseconds(),
			s_receiveState.simulators[ 0 ].DeliveryRate(),
			s_receiveState.simulators[ 1 ].DeliveryRate(),
			s_receiveState.simulators[ 2 ].DeliveryRate(),
			s_receiveState.simulators[ 3 ].DeliveryRate(),
			s_receiveState.simulators[ 4 ].DeliveryRate(),
			s_receiveState.simulators[ 5 ].DeliveryRate(),
			s_receiveState.simulators[ 6 ].DeliveryRate(),
			s_receiveState.simulators[ 7 ].DeliveryRate() );
	}

	static bool ApplyRequestedWindow( bool serverRole,
		unsigned int *deliveredThisFrame )
	{
		// Auto mode owns capacity after its one transition application. Keeping
		// the current window here avoids restoring the session-default 32 every
		// frame after automatic selection has changed it.
		if ( s_requestedWindowRatio == 0.0f )
		{
			if ( s_windowConfigDirty )
				s_windowConfigDirty = false;
			return true;
		}

		unsigned int requestedCapacity = kDefaultLogicalCapacity;
		if ( !NormalizeWindowRatio( s_requestedWindowRatio,
			&requestedCapacity ) )
		{
			LogBridge( "event=unreliable_window_config result=error ratio=%.3f",
				s_requestedWindowRatio );
			return false;
		}

		if ( !s_windowConfigDirty &&
			s_receiveState.reorderRing.LogicalCapacity() == requestedCapacity )
			return true;
		if ( s_receiveState.reorderRing.LogicalCapacity() == requestedCapacity )
		{
			s_windowConfigDirty = false;
			return true;
		}

		const unsigned int oldCapacity =
			s_receiveState.reorderRing.LogicalCapacity();
		if ( !ApplyLogicalCapacity( serverRole, requestedCapacity,
			deliveredThisFrame ) )
			return false;
		s_windowConfigDirty = false;
		LogWindowChange( oldCapacity, requestedCapacity,
			s_receiveState.currentSimulator.SampleCount() );
		s_receiveState.BeginNextMeasurement();
		return true;
	}

	static double AbsoluteDifference( double first, double second )
	{
		return first >= second ? first - second : second - first;
	}

	static bool CandidateMetricsBetter(
		const UnreliableWindowSimulator &candidate,
		const UnreliableWindowSimulator &best )
	{
		const double candidateP95 = candidate.P95ResidenceMilliseconds();
		const double bestP95 = best.P95ResidenceMilliseconds();
		const double p95Difference = AbsoluteDifference( candidateP95, bestP95 );
		if ( p95Difference > kLatencyComparisonThresholdSeconds * 1000.0 )
			return candidateP95 < bestP95;

		const double candidateAverage =
			candidate.AverageResidenceMilliseconds();
		const double bestAverage = best.AverageResidenceMilliseconds();
		const double averageDifference = AbsoluteDifference( candidateAverage,
			bestAverage );
		if ( averageDifference > kLatencyComparisonThresholdSeconds * 1000.0 )
			return candidateAverage < bestAverage;

		const double candidateDelivery = candidate.DeliveryRate();
		const double bestDelivery = best.DeliveryRate();
		if ( AbsoluteDifference( candidateDelivery, bestDelivery ) >
			kDeliveryRateTolerance )
			return candidateDelivery > bestDelivery;
		return false;
	}

	static bool CandidatePreferredToCurrent(
		const UnreliableWindowSimulator &candidate,
		const UnreliableWindowSimulator &current )
	{
		const double candidateP95 = candidate.P95ResidenceMilliseconds();
		const double currentP95 = current.P95ResidenceMilliseconds();
		if ( AbsoluteDifference( candidateP95, currentP95 ) >
			kLatencyComparisonThresholdSeconds * 1000.0 )
			return candidateP95 < currentP95;

		const double candidateAverage =
			candidate.AverageResidenceMilliseconds();
		const double currentAverage = current.AverageResidenceMilliseconds();
		if ( AbsoluteDifference( candidateAverage, currentAverage ) >
			kLatencyComparisonThresholdSeconds * 1000.0 )
			return candidateAverage < currentAverage;

		const double candidateDelivery = candidate.DeliveryRate();
		const double currentDelivery = current.DeliveryRate();
		if ( AbsoluteDifference( candidateDelivery, currentDelivery ) >
			kDeliveryRateTolerance )
			return candidateDelivery > currentDelivery;

		// All metrics are within their hysteresis bands. Keep the active window
		// to avoid oscillation.
		return false;
	}

	static bool MaybeSelectAutomaticWindow( bool serverRole,
		unsigned int *deliveredThisFrame )
	{
		if ( s_requestedWindowRatio != 0.0f )
			return true;

		// A generation is complete only after every simulator has retired the
		// same warm-up plus eligible sequence interval.  This prevents a fast
		// window's counter from being compared with a slower window's earlier or
		// shorter interval.
		if ( !s_receiveState.MeasurementComplete() )
			return true;

		const unsigned int currentCapacity =
			s_receiveState.reorderRing.LogicalCapacity();
		const double now = Plat_FloatTime();
		const bool normal = !s_receiveState.hasWindowSelectionTime ||
			now - s_receiveState.lastWindowSelectionTime >=
			kWindowSelectionIntervalSeconds;
		if ( !normal )
			return true;

		double maximumCandidateDelivery = 0.0;
		for ( unsigned int i = 0; i < kSimulatorCount; ++i )
		{
			const double deliveryRate =
				s_receiveState.simulators[ i ].DeliveryRate();
			if ( deliveryRate > maximumCandidateDelivery )
				maximumCandidateDelivery = deliveryRate;
		}

		// Apply the explicit minimum-quality failure rule before reading any
		// latency or D_current metric. The 0.5% qualification band below is
		// retained for auditability, but cannot admit a candidate when the
		// maximum itself is below D_min: that failure rule has precedence.
		if ( maximumCandidateDelivery < kMinimumDeliveryRate )
		{
			const unsigned int fallbackCapacity = kDefaultLogicalCapacity;
			if ( currentCapacity != fallbackCapacity &&
				!ApplyLogicalCapacity( serverRole, fallbackCapacity,
					deliveredThisFrame ) )
				return false;
			s_receiveState.lastWindowSelectionTime = now;
			s_receiveState.hasWindowSelectionTime = true;
			LogBridge( "event=unreliable_window_selection result=quality_failure max_candidate_delivery=%.4f minimum_delivery=%.4f fallback_capacity=%u",
				maximumCandidateDelivery, kMinimumDeliveryRate,
				fallbackCapacity );
			s_receiveState.BeginNextMeasurement();
			return true;
		}

		const double minimumCandidateDelivery = kMinimumDeliveryRate -
			kDeliveryRateTolerance;
		int bestIndex = -1;
		// The policy defines hysteresis bands but no additional global tie-break.
		// Iterate the fixed ascending candidate table so near-tie selection remains
		// deterministic without inventing a preference for a window size.
		for ( unsigned int i = 0; i < kSimulatorCount; ++i )
		{
			const UnreliableWindowSimulator &candidate =
				s_receiveState.simulators[ i ];
			if ( candidate.DeliveryRate() < minimumCandidateDelivery )
				continue;
			if ( bestIndex < 0 || CandidateMetricsBetter( candidate,
				s_receiveState.simulators[ bestIndex ] ) )
				bestIndex = (int)i;
		}
		if ( bestIndex < 0 )
			return true;
		s_receiveState.lastWindowSelectionTime = now;
		s_receiveState.hasWindowSelectionTime = true;

		// D_current is read only after D_best has been formed solely from
		// qualified candidate metrics. It is never used to rank candidates.
		const UnreliableWindowSimulator &bestCandidate =
			s_receiveState.simulators[ bestIndex ];
		const bool candidateWins = CandidatePreferredToCurrent( bestCandidate,
			s_receiveState.currentSimulator );
		const unsigned int selectedCapacity = candidateWins
			? kCandidateCapacities[ bestIndex ] : currentCapacity;
		if ( selectedCapacity == currentCapacity )
		{
			s_receiveState.BeginNextMeasurement();
			return true;
		}
		if ( !ApplyLogicalCapacity( serverRole, selectedCapacity,
			deliveredThisFrame ) )
			return false;
		LogWindowChange( currentCapacity, selectedCapacity,
			s_receiveState.currentSimulator.SampleCount() );
		s_receiveState.BeginNextMeasurement();
		return true;
	}

	static bool EndReceiveEpoch( bool serverRole, uint64_t completedReceiveEpoch,
		double currentTime, unsigned int *deliveredThisFrame )
	{
		// The epoch is a safety boundary only: timeout age is wall-clock based.
		// Rejecting a stale completion protects against a future split receive
		// loop without making epoch count part of the timeout policy.
		if ( completedReceiveEpoch != s_receiveState.receiveEpoch )
			return false;

		// No timeout decision is made until ASRD_GNS_Receive has returned zero for
		// this epoch. First drain everything already contiguous.
		if ( !DrainRingAvailable( serverRole, deliveredThisFrame ) )
			return false;
		for ( unsigned int i = 0; i < kSimulatorCount; ++i )
			s_receiveState.simulators[ i ].CheckTimeout( currentTime );
		s_receiveState.currentSimulator.CheckTimeout( currentTime );

		uint64_t targetSequence = 0;
		if ( s_receiveState.reorderRing.FindFarthestTimedOut( currentTime,
			&targetSequence ) )
		{
			if ( !ForcePopRingThrough( serverRole, targetSequence,
				deliveredThisFrame ) )
				return false;
			if ( !DrainRingAvailable( serverRole, deliveredThisFrame ) )
				return false;
		}
		return MaybeSelectAutomaticWindow( serverRole, deliveredThisFrame );
	}

	static bool SerializeSourceMessage( INetMessage &message,
		unsigned char *payload, unsigned int *bitLength,
		unsigned int *payloadBytes, ASRD_GNS_MoveMetadata *moveMetadata )
	{
		if ( !payload || !bitLength || !payloadBytes )
			return false;
		if ( moveMetadata )
			memset( moveMetadata, 0, sizeof( *moveMetadata ) );

		bf_write writer( payload, ASRD_GNS_MESSAGE_MAX_ENVELOPE_BYTES );
		writer.SetAssertOnOverflow( false );
		const bool writeResult = message.WriteToBuffer( writer );
		const int writtenBits = writer.GetNumBitsWritten();
		const int writtenBytes = writer.GetNumBytesWritten();
		if ( !writeResult || writer.IsOverflowed() || writtenBits < 0 ||
			writtenBytes < 0 ||
			(unsigned int)writtenBits > ASRD_GNS_MESSAGE_MAX_ENVELOPE_BYTES * 8U ||
			(unsigned int)writtenBytes > ASRD_GNS_MESSAGE_MAX_ENVELOPE_BYTES )
			return false;

		*bitLength = (unsigned int)writtenBits;
		*payloadBytes = (unsigned int)writtenBytes;

		if ( moveMetadata && message.GetType() == clc_Move )
		{
			if ( *bitLength < 6 )
				return false;
			bf_read reader( payload, writtenBytes, writtenBits );
			const int wireType = (int)reader.ReadUBitLong( 6 );
			if ( reader.IsOverflowed() || wireType != clc_Move ||
				!ASRD_GNS_ParseCLCMoveBody( &reader, writtenBits - 6,
					moveMetadata ) )
				return false;
		}
		return true;
	}

	static unsigned int DiscardQueuedForConnection(
		ASRD_GNS_Connection connection )
	{
		unsigned int discarded = 0;
		std::deque<QueuedEnvelope *>::iterator it = s_queue.begin();
		while ( it != s_queue.end() )
		{
			if ( connection == ASRD_GNS_CONNECTION_INVALID ||
				( *it )->connection == connection )
			{
				delete *it;
				it = s_queue.erase( it );
				++discarded;
			}
			else
				++it;
		}
		return discarded;
	}

	static unsigned int FailClosedServerSession(
		ASRD_GNS_Connection connection )
	{
		if ( connection == ASRD_GNS_CONNECTION_INVALID )
			return 0;

		const unsigned int discarded = DiscardQueuedForConnection(
			ASRD_GNS_CONNECTION_INVALID );
		{
			std::lock_guard<std::mutex> lock( s_submissionMutex );
			ResetPendingServerUpdate();
		}
		ResetReceiveState( ASRD_GNS_CONNECTION_INVALID );
		ResetSendSession( ASRD_GNS_CONNECTION_INVALID );
		ASRD_GNS_ServerResetCompatibility(
			ASRD_GNS_ServerContextForConnection( connection ) );
		ASRD_GNS_Close( connection );

		// Close is consumed by the server lifecycle on its next frame. Keep the
		// bridge fail-closed in the meantime so an INVALID receive reset cannot
		// be immediately remapped to the still-reporting connection.
		s_failClosedServerConnection = connection;
		s_lastMapped = false;
		s_lastHandle = ASRD_GNS_CONNECTION_INVALID;
		return discarded;
	}
}

bool ASRD_GNS_MessageBridgeSend( ASRD_GNS_Connection connection,
	INetMessage &message, bool forceReliable,
	ASRD_GNS_MoveMetadata *moveMetadata )
{
	return ASRD_GNS_MessageBridgeSend( connection, message, forceReliable,
		false, ASRD_GNS_PROVENANCE_SEND_NETMSG, moveMetadata );
}

bool ASRD_GNS_MessageBridgeSetUnreliableWindowRatio( float ratio )
{
	unsigned int capacity = 0;
	if ( ratio != 0.0f && ( ratio < 0.5f || ratio > 4.0f ) )
	{
		LogBridge( "event=unreliable_window_config result=error ratio=%.3f valid_range=0.0_or_0.5_to_4.0",
			ratio );
		return false;
	}
	if ( !NormalizeWindowRatio( ratio, &capacity ) )
		return false;

	const bool modeChanged = ( s_requestedWindowRatio == 0.0f ) !=
		( ratio == 0.0f );
	s_requestedWindowRatio = ratio == 0.0f ? 0.0f : ratio;
	s_windowConfigDirty = true;
	if ( modeChanged )
	{
		s_receiveState.lastWindowSelectionTime = 0.0;
		s_receiveState.hasWindowSelectionTime = false;
		// A manual/automatic mode transition changes the measurement contract
		// even when the normalized capacity happens to stay the same. Drop the
		// completed or partial generation so auto mode cannot select from stale
		// samples gathered under the previous mode.
		s_receiveState.BeginNextMeasurement();
	}
	LogBridge( "event=unreliable_window_config result=accepted ratio=%.3f mode=%s requested_capacity=%u",
		ratio, ratio == 0.0f ? "auto" : "manual", capacity );
	return true;
}

bool ASRD_GNS_MessageBridgeSend( ASRD_GNS_Connection connection,
	INetMessage &message, bool effectiveReliable, bool voice,
	ASRD_GNS_BlockProvenance provenance,
	ASRD_GNS_MoveMetadata *moveMetadata )
{
	if ( connection == ASRD_GNS_CONNECTION_INVALID )
	{
		LogBridge( "event=send result=error reason=invalid_handle type=%d", message.GetType() );
		return false;
	}
	const int messageType = message.GetType();
	if ( messageType < 0 || messageType > 63 )
	{
		LogBridge( "event=send result=error reason=invalid_type handle=%lu type=%d",
			(unsigned long)connection, messageType );
		return false;
	}

	std::vector<unsigned char> payload( ASRD_GNS_MESSAGE_MAX_ENVELOPE_BYTES );
	unsigned int bitLength = 0;
	unsigned int payloadBytes = 0;
	if ( !SerializeSourceMessage( message, &payload[ 0 ], &bitLength,
		&payloadBytes, moveMetadata ) )
	{
		LogBridge( "event=send result=error reason=serialize_failed handle=%lu type=%d",
			(unsigned long)connection, messageType );
		return false;
	}

	SerializedEngineBlock block;
	{
		std::lock_guard<std::mutex> lock( s_submissionMutex );
		if ( !CaptureBlock( &payload[ 0 ], bitLength, payloadBytes,
			(uint8_t)messageType,
			effectiveReliable ? ASRD_GNS_BLOCK_RELIABILITY_RELIABLE
				: ASRD_GNS_BLOCK_RELIABILITY_UNRELIABLE,
			voice ? ASRD_GNS_PROVENANCE_VOICE : provenance, &block ) )
		{
			LogBridge( "event=send result=error reason=capture_failed handle=%lu type=%d bitlen=%u bytes=%u reliable=%u voice=%u",
				(unsigned long)connection, messageType, bitLength, payloadBytes,
				effectiveReliable ? 1U : 0U, voice ? 1U : 0U );
			return false;
		}

		const bool sent = SubmitBlock( connection, &block,
			ASRD_GNS_DIRECTION_CLIENT_TO_SERVER, 0, 0 );
		if ( !sent )
			LogBridge( "event=send result=error reason=transport_failed handle=%lu type=%d bitlen=%u bytes=%u reliable=%u voice=%u lane=%u unreliable_seq=%llu",
				(unsigned long)connection, messageType, bitLength, payloadBytes,
				effectiveReliable ? 1U : 0U, voice ? 1U : 0U,
				(unsigned)block.lane, (unsigned long long)block.unreliableSeq );
		return sent;
	}
}

bool ASRD_GNS_MessageBridgeAppendServer( ASRD_GNS_Connection connection,
	INetMessage &message, bool effectiveReliable )
{
	return ASRD_GNS_MessageBridgeAppendServer( connection, message,
		effectiveReliable, false, ASRD_GNS_PROVENANCE_SEND_NETMSG );
}

bool ASRD_GNS_MessageBridgeAppendServer( ASRD_GNS_Connection connection,
	INetMessage &message, bool effectiveReliable, bool voice,
	ASRD_GNS_BlockProvenance provenance )
{
	if ( connection == ASRD_GNS_CONNECTION_INVALID )
		return false;
	const int messageType = message.GetType();
	if ( messageType < 0 || messageType > 63 )
	{
		LogBridge( "event=server_append result=error reason=invalid_type handle=%lu type=%d",
			(unsigned long)connection, messageType );
		return false;
	}

	std::vector<unsigned char> payload( ASRD_GNS_MESSAGE_MAX_ENVELOPE_BYTES );
	unsigned int bitLength = 0;
	unsigned int payloadBytes = 0;
	if ( !SerializeSourceMessage( message, &payload[ 0 ], &bitLength,
		&payloadBytes, NULL ) )
	{
		LogBridge( "event=server_append result=error reason=serialize_failed handle=%lu type=%d",
			(unsigned long)connection, messageType );
		return false;
	}

	std::lock_guard<std::mutex> lock( s_submissionMutex );
	const bool appended = AppendPendingServerBlock( connection, &payload[ 0 ],
		bitLength, payloadBytes, (uint8_t)messageType,
		effectiveReliable ? ASRD_GNS_BLOCK_RELIABILITY_RELIABLE
			: ASRD_GNS_BLOCK_RELIABILITY_UNRELIABLE,
		voice ? ASRD_GNS_PROVENANCE_VOICE : provenance );
	if ( !appended )
		LogBridge( "event=server_append result=error handle=%lu type=%d bitlen=%u bytes=%u reliable=%u voice=%u",
			(unsigned long)connection, messageType, bitLength, payloadBytes,
			effectiveReliable ? 1U : 0U, voice ? 1U : 0U );
	return appended;
}

bool ASRD_GNS_MessageBridgeAppendServerRaw( ASRD_GNS_Connection connection,
	const void *payload, unsigned int bitLength, unsigned int payloadBytes,
	bool effectiveReliable )
{
	return ASRD_GNS_MessageBridgeAppendServerRaw( connection, payload,
		bitLength, payloadBytes,
		effectiveReliable ? ASRD_GNS_BLOCK_RELIABILITY_RELIABLE
			: ASRD_GNS_BLOCK_RELIABILITY_UNRELIABLE,
		ASRD_GNS_PROVENANCE_SEND_DATA );
	}

bool ASRD_GNS_MessageBridgeAppendServerRaw( ASRD_GNS_Connection connection,
	const void *payload, unsigned int bitLength, unsigned int payloadBytes,
	ASRD_GNS_BlockReliability reliability,
	ASRD_GNS_BlockProvenance provenance )
{
	std::lock_guard<std::mutex> lock( s_submissionMutex );
	const bool appended = AppendPendingServerBlock( connection, payload,
		bitLength, payloadBytes, ASRD_GNS_MESSAGE_RAW_STREAM_TYPE,
		reliability, provenance );
	if ( !appended )
		LogBridge( "event=server_append_raw result=error handle=%lu bitlen=%u bytes=%u reliability=%u provenance=%u",
			(unsigned long)connection, bitLength, payloadBytes,
			(unsigned)reliability, (unsigned)provenance );
	return appended;
}

bool ASRD_GNS_MessageBridgeSealAndFlushServerUpdate(
	ASRD_GNS_Connection connection )
{
	if ( connection == ASRD_GNS_CONNECTION_INVALID )
		return false;

	std::lock_guard<std::mutex> lock( s_submissionMutex );
	if ( !s_pendingServerUpdate.open ||
		s_pendingServerUpdate.connection != connection )
		return true;
	if ( s_pendingServerUpdate.blocks.empty() )
	{
		ResetPendingServerUpdate();
		return true;
	}

	uint32_t serverUpdateSeq = 0;
	int clientCommandAck = 0;
	ASRD_GNS_ServerConnectionContext *context =
		ASRD_GNS_ServerContextForConnection( connection );
	if ( !context || !ASRD_GNS_ServerCaptureOutgoingUpdateMetadata(
		context, &serverUpdateSeq, &clientCommandAck ) )
	{
		LogBridge( "event=server_update result=error reason=metadata_capture_failed handle=%lu",
			(unsigned long)connection );
		ResetPendingServerUpdate();
		return false;
	}

	for ( std::vector<PendingBlock>::iterator it =
		s_pendingServerUpdate.blocks.begin();
		it != s_pendingServerUpdate.blocks.end(); ++it )
	{
		if ( !SubmitBlock( connection, &it->block,
			ASRD_GNS_DIRECTION_SERVER_TO_CLIENT, serverUpdateSeq,
			clientCommandAck ) )
		{
			LogBridge( "event=server_update result=error reason=block_submit_failed handle=%lu sourceOrder=%llu unreliable_seq=%llu lane=%u update=%u ack=%d action=discard_pending",
				(unsigned long)connection,
				(unsigned long long)it->sourceOrder,
				(unsigned long long)it->block.unreliableSeq, (unsigned)it->block.lane,
				(unsigned)serverUpdateSeq, clientCommandAck );
			ResetPendingServerUpdate();
			return false;
		}
	}

		// GNS flushes the whole connection. It is issued once after every block
		// in this logical update has been enqueued; it is not a lane-ordering
		// primitive.
	{
		const int flushResult = ASRD_GNS_Flush( connection );
		if ( flushResult != ASRD_GNS_RESULT_OK )
		{
			LogBridge( "event=server_update_flush result=error handle=%lu EResult=%d update=%u ack=%d",
				(unsigned long)connection, flushResult,
				(unsigned)serverUpdateSeq, clientCommandAck );
			ResetPendingServerUpdate();
			return false;
		}
	}

	ResetPendingServerUpdate();
	return true;
}

unsigned int ASRD_GNS_MessageBridgeServerPendingBits(
	ASRD_GNS_Connection connection )
{
	std::lock_guard<std::mutex> lock( s_submissionMutex );
	if ( !s_pendingServerUpdate.open ||
		s_pendingServerUpdate.connection != connection )
		return 0;

	uint64_t totalBits = 0;
	for ( std::vector<PendingBlock>::const_iterator it =
		s_pendingServerUpdate.blocks.begin();
		it != s_pendingServerUpdate.blocks.end(); ++it )
	{
		totalBits += it->block.bitLength;
		if ( totalBits >= std::numeric_limits<unsigned int>::max() )
			return std::numeric_limits<unsigned int>::max();
	}
	return (unsigned int)totalBits;
}

bool ASRD_GNS_MessageBridgeServerHasPending(
	ASRD_GNS_Connection connection )
{
	return ASRD_GNS_MessageBridgeServerPendingBits( connection ) != 0;
}

bool ASRD_GNS_MessageBridgeSendRaw( ASRD_GNS_Connection connection,
	const void *payload, unsigned int bitLength, unsigned int payloadBytes,
	bool effectiveReliable )
{
	if ( connection == ASRD_GNS_CONNECTION_INVALID )
	{
		LogBridge( "event=send_raw result=error reason=invalid_handle handle=%lu bitlen=%u bytes=%u",
			(unsigned long)connection, bitLength, payloadBytes );
		return false;
	}

	std::lock_guard<std::mutex> lock( s_submissionMutex );
	SerializedEngineBlock block;
	if ( !CaptureBlock( payload, bitLength, payloadBytes,
		ASRD_GNS_MESSAGE_RAW_STREAM_TYPE,
		effectiveReliable ? ASRD_GNS_BLOCK_RELIABILITY_RELIABLE
			: ASRD_GNS_BLOCK_RELIABILITY_UNRELIABLE,
		ASRD_GNS_PROVENANCE_SEND_DATA, &block ) )
	{
		LogBridge( "event=send_raw result=error reason=capture_failed handle=%lu bitlen=%u bytes=%u reliable=%u",
			(unsigned long)connection, bitLength, payloadBytes,
			effectiveReliable ? 1U : 0U );
		return false;
	}

	const bool sent = SubmitBlock( connection, &block,
		ASRD_GNS_DIRECTION_CLIENT_TO_SERVER, 0, 0 );
	if ( !sent )
		LogBridge( "event=send_raw result=error reason=transport_failed handle=%lu bitlen=%u bytes=%u reliable=%u lane=%u unreliable_seq=%llu",
			(unsigned long)connection, bitLength, payloadBytes,
			effectiveReliable ? 1U : 0U, (unsigned)block.lane,
			(unsigned long long)block.unreliableSeq );
	return sent;
}

void ASRD_GNS_MessageBridgeFrame( bool serverRole )
{
	// A GNS connection event is not enough to expose the Source message path:
	// the local CClientState CONNECTED prime must have completed first.  Return
	// before mapping, receiving, enqueueing, draining, or PacketStart/End so
	// the pre-prime queue remains untouched.
	if ( !serverRole && !ASRD_GNS_ClientBridgeReady() )
		return;

	bool mapped = false;
	const ASRD_GNS_Connection connection = MappedConnection( serverRole, &mapped );
	if ( !mapped )
	{
		s_failClosedServerConnection = ASRD_GNS_CONNECTION_INVALID;
		if ( s_lastMapped )
		{
			const unsigned int discarded = DiscardQueuedForConnection( s_lastHandle );
			{
				std::lock_guard<std::mutex> lock( s_submissionMutex );
				if ( s_pendingServerUpdate.open &&
					s_pendingServerUpdate.connection == s_lastHandle )
					ResetPendingServerUpdate();
			}
			if ( serverRole )
				ASRD_GNS_ServerResetCompatibility(
					ASRD_GNS_ServerContextForConnection( s_lastHandle ) );
			LogPacketLifecycleSummary( s_lastHandle );
			LogBridge( "event=map result=lost role=%s previous_handle=%lu queue=%u discarded=%u",
				RoleName( serverRole ), (unsigned long)s_lastHandle,
				(unsigned int)s_queue.size(), discarded );
			ResetSendSession( s_lastHandle );
		}
		ResetReceiveState( ASRD_GNS_CONNECTION_INVALID );
		ResetSendSession( ASRD_GNS_CONNECTION_INVALID );
		s_lastMapped = false;
		s_lastHandle = ASRD_GNS_CONNECTION_INVALID;
		return;
	}
	if ( serverRole && s_failClosedServerConnection !=
		ASRD_GNS_CONNECTION_INVALID )
	{
		if ( s_failClosedServerConnection == connection )
		{
			LogBridge( "event=map result=blocked role=server handle=%lu reason=fail_closed_session_pending_lifecycle_removal",
				(unsigned long)connection );
			return;
		}
		s_failClosedServerConnection = ASRD_GNS_CONNECTION_INVALID;
	}
	if ( !s_lastMapped || s_lastHandle != connection )
	{
		if ( s_lastMapped && s_lastHandle != connection )
		{
			DiscardQueuedForConnection( s_lastHandle );
			ResetSendSession( s_lastHandle );
			LogPacketLifecycleSummary( s_lastHandle );
		}
		LogBridge( "event=map result=ready role=%s handle=%lu registry_count=%u",
			RoleName( serverRole ), (unsigned long)connection,
			ASRD_GNS_MessageRegistryCount() );
		ResetReceiveState( connection );
		s_lastMapped = true;
		s_lastHandle = connection;
	}

	unsigned int deliveredThisFrame = 0;
	bool temporaryClientChannel = false;
	if ( !serverRole )
	{
		// U packets can become contiguous while the receive queue is being
		// drained, so the Source dispatch channel must be available before the
		// first ReceiveMessagesOnConnection call in this epoch.
		temporaryClientChannel = ASRD_GNS_ClientBindSourceChannelForDispatch();
		if ( !temporaryClientChannel )
		{
			LogBridge( "event=frame result=error role=client reason=source_channel_bind_failed" );
			return;
		}
	}

	if ( !ApplyRequestedWindow( serverRole, &deliveredThisFrame ) )
	{
		if ( temporaryClientChannel )
			ASRD_GNS_ClientUnbindSourceChannelForDispatch();
		if ( serverRole )
			FailClosedServerSession( connection );
		else
			ASRD_GNS_ClientAbortCompatibilitySession(
				"unreliable_window_apply_failed" );
		return;
	}

	// Receive epochs are local bookkeeping only. They delimit the point after
	// which a wall-clock timeout check is safe; they do not contribute to age.
	++s_receiveState.receiveEpoch;
	const uint64_t receiveEpoch = s_receiveState.receiveEpoch;

	static unsigned char receiveBuffer[ ASRD_GNS_MESSAGE_MAX_ENVELOPE_BYTES ];
	bool receiveFatal = false;
	const char *receiveFatalReason = NULL;
	for ( ;; )
	{
		uint32_t size = 0;
		const int receiveResult = ASRD_GNS_Receive( connection, receiveBuffer,
			(uint32_t)sizeof( receiveBuffer ), &size );
		if ( receiveResult == 0 )
			break;
		if ( receiveResult < 0 )
		{
			LogBridge( "event=receive result=error role=%s handle=%lu code=%d",
				RoleName( serverRole ), (unsigned long)connection, receiveResult );
			receiveFatal = true;
			receiveFatalReason = "receive_failed";
			break;
		}
		++s_received;
		if ( !EnqueueEnvelope( connection, receiveBuffer, size,
			serverRole, &deliveredThisFrame ) )
		{
			LogBridge( "event=enqueue result=error role=%s handle=%lu bytes=%u queue=%u",
				RoleName( serverRole ), (unsigned long)connection, (unsigned)size,
				(unsigned int)s_queue.size() );
			receiveFatal = true;
			receiveFatalReason = "enqueue_failed";
			break;
		}
	}
	if ( receiveFatal )
	{
		if ( temporaryClientChannel )
			ASRD_GNS_ClientUnbindSourceChannelForDispatch();
		if ( serverRole )
			FailClosedServerSession( connection );
		else
		{
			ASRD_GNS_MessageBridgeDiscardQueued( connection );
			ASRD_GNS_ClientAbortCompatibilitySession( receiveFatalReason );
		}
		LogBridge( "event=frame result=abort role=%s handle=%lu reason=%s action=session_fatal",
			RoleName( serverRole ), (unsigned long)connection,
			receiveFatalReason ? receiveFatalReason : "receive_failed" );
		return;
	}
	bool dispatchFailure = false;
	while ( !s_queue.empty() )
	{
		QueuedEnvelope *item = s_queue.front();
		s_queue.pop_front();
		bool itemDelivered = false;
		if ( DispatchEnvelope( serverRole, item, &itemDelivered ) )
		{
			if ( itemDelivered )
				++deliveredThisFrame;
		}
		else
		{
			dispatchFailure = true;
			delete item;
			break;
		}
		delete item;
	}
	// Capture one timestamp only after the GNS receive queue returned zero and
	// the reliable FIFO has been dispatched. All simulators and the real ring
	// use this exact timeout-check time, so reliable dispatch time counts toward
	// the wall-clock residence deadline.
	const double receiveDrainTime = Plat_FloatTime();
	if ( !dispatchFailure && !EndReceiveEpoch( serverRole, receiveEpoch,
		receiveDrainTime, &deliveredThisFrame ) )
		dispatchFailure = true;
	if ( dispatchFailure )
	{
		unsigned int discarded = 0;
		if ( serverRole )
			discarded = FailClosedServerSession( connection );
		else
		{
			discarded = DiscardQueuedForConnection( connection );
			ResetReceiveState( ASRD_GNS_CONNECTION_INVALID );
			ASRD_GNS_ClientAbortCompatibilitySession(
				"logical_update_dispatch_failed" );
		}
		LogBridge( "event=frame result=abort role=%s handle=%lu action=compatibility_reset discarded=%u",
			RoleName( serverRole ), (unsigned long)connection, discarded );
	}
	if ( !serverRole && !dispatchFailure && deliveredThisFrame > 0 )
		ASRD_GNS_ClientPromoteSourceChannelForLifecycle();
	if ( temporaryClientChannel )
		ASRD_GNS_ClientUnbindSourceChannelForDispatch();

}

void ASRD_GNS_MessageBridgeDiscardQueued( ASRD_GNS_Connection connection )
{
	if ( connection == ASRD_GNS_CONNECTION_INVALID ||
		connection == s_failClosedServerConnection )
		s_failClosedServerConnection = ASRD_GNS_CONNECTION_INVALID;
	const unsigned int discarded = DiscardQueuedForConnection( connection );
	unsigned int discardedRing = 0;
	{
		std::lock_guard<std::mutex> lock( s_submissionMutex );
		if ( s_pendingServerUpdate.open &&
			( connection == ASRD_GNS_CONNECTION_INVALID ||
				s_pendingServerUpdate.connection == connection ) )
			ResetPendingServerUpdate();
	}
	if ( connection == ASRD_GNS_CONNECTION_INVALID ||
		s_receiveState.connection == connection )
	{
		discardedRing = s_receiveState.reorderRing.PendingCount();
		LogPacketLifecycleSummary( s_receiveState.connection );
		ResetReceiveState( ASRD_GNS_CONNECTION_INVALID );
	}
	ResetSendSession( connection );
	if ( discarded > 0 || discardedRing > 0 )
		LogBridge( "event=queue action=discard handle=%lu count=%u",
			(unsigned long)connection, discarded + discardedRing );
}

unsigned int ASRD_GNS_MessageBridgeQueued( void )
{
	return (unsigned int)s_queue.size() +
		s_receiveState.reorderRing.PendingCount();
}

unsigned int ASRD_GNS_MessageBridgeReceived( void )
{
	return s_received;
}

unsigned int ASRD_GNS_MessageBridgeDelivered( void )
{
	return s_delivered;
}
