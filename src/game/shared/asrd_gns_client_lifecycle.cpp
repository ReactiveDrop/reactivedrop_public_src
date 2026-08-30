#include "cbase.h"
#include "tier0/dbg.h"
#include "tier1/netadr.h"
#include "inetmessage.h"
#include "protocol.h"
#include "asrd_gns_client_lifecycle.h"
#include "asrd_gns_message_bridge.h"
#include "asrd_gns_message_registry.h"
#include "asrd_gns_move_compat.h"

#if defined( _WIN32 ) && !defined( _X360 )

#include <windows.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace
{
	static const DWORD kClientStateMagic = 0x41524743; // "ARGC"
	static const DWORD kClientStateVersion = 1;
	static const char kClientStatePrefix[] = "ASRD_GNS_CLIENT_LIFECYCLE";
	static const size_t kEndpointCapacity = 64;
	static const uint16_t kDefaultClientConnectPort = 27015;
	// engine.dll vtable evidence is recorded in
	// src/game/gns_wrapper/disassembly_evidence/engine_bindings.md: slots 4 and 5
	// target the packet-boundary methods with two and zero stack arguments.
	static const unsigned int kClientPacketStartSlot = 4;
	static const unsigned int kClientPacketEndSlot = 5;
	static const DWORD kClientPacketStartRva = 0x000A5EC0;
	static const DWORD kClientPacketEndRva = 0x000A5EE0;

	// Fixed engine bindings are validated against the local engine.dll image;
	// exact bytes and decoded instructions are recorded in
	// src/game/gns_wrapper/disassembly_evidence/engine_bindings.md.
	static const DWORD kEngineTimeDateStamp = 0x5F363761;
	static const DWORD kEngineSizeOfImage = 0x006F2000;
	// The expected live client-state table is at this RVA; slot 1 targets the
	// verified registration method.
	static const DWORD kClientStateVtableRva = 0x00329B44;
	static const DWORD kConnectionStartRva = 0x000D57E0;
	// SetSignonState reads its channel pointer from this byte offset in the
	// verified client-state object layout. The adapter is bound only to that slot.
	static const size_t kClientStateNetChannelOffset = 0x10;
	// The verified SetSignonState entry reads the previous signon state from
	// this offset. It distinguishes an in-place changelevel from disconnect.
	static const size_t kClientStateSignonStateOffset = 0x68;
	// Slot 15 targets the verified signon method. Its `ret 0xC` requires the
	// explicit third stack argument after state and count.
	static const unsigned int kClientSetSignonStateSlot = 15;
	static const DWORD kClientSetSignonStateRva = 0x001091A0;

	// The engine targets use ECX for the object pointer; their stack cleanup is
	// verified in src/game/gns_wrapper/disassembly_evidence/engine_bindings.md.
	typedef void (__thiscall *ClientConnectionStartFn)( void *clientState, void *channel );
	typedef bool (__thiscall *ClientSetSignonStateFn)( void *clientState,
		int state, int count, int reserved );
	typedef void (__thiscall *ClientPacketStartFn)( void *clientState,
		int incomingSequence, int outgoingAcknowledged );
	typedef void (__thiscall *ClientPacketEndFn)( void *clientState );

	struct RegistrationChannelAdapter
	{
		void **vtable;
	};

	// This table covers the highest retained adapter slot. Only the explicitly
	// installed entries are callable; all remaining entries fail closed as NULL.
	static void *s_registrationVtable[ 75 ] = { 0 };
	static RegistrationChannelAdapter s_registrationAdapter =
		{ s_registrationVtable };
	static bool s_registrationAdapterReady = false;
	static bool s_sourceContextBound = false;
	static void *s_sourceContext = NULL;
	static void *s_dispatchPreviousNetChannel = NULL;
	static bool s_dispatchNetChannelBound = false;
	static bool s_sourceChannelPersistent = false;
	static bool s_clientShutdownInProgress = false;
	enum SourceLifecyclePhase
	{
		SOURCE_LIFECYCLE_RUNNING = 0,
		SOURCE_LIFECYCLE_CHANGELEVEL,
	};
	static SourceLifecyclePhase s_sourceLifecyclePhase =
		SOURCE_LIFECYCLE_RUNNING;
	static bool s_sourceReattachPending = false;
	static bool s_disconnectRequested = false;
	static float s_disconnectAfterSeconds = -1.0f;
	static double s_disconnectDeadline = 0.0;
	static bool s_disconnectTimerArmed = false;
	static bool s_compatibilityFatal = false;
	// These flags belong to one GNS connection generation.  They are deliberately
	// separate from compatibility/packet bookkeeping so a new takeover cannot
	// inherit a previously primed Source signon state.
	static bool s_challengePrimed = false;
	static bool s_connectedPrimed = false;
	static bool s_localConnectedPrime = false;
	static int s_localConnectedPrimeState = -1;
	static bool s_localConnectedPrimeSignonSuppressed = false;
	static bool s_localConnectedPrimeSendFailed = false;
	static bool s_connectedPrimeSetConVarSeen = false;
	static bool s_packetStarted = false;
	static uint32_t s_currentServerUpdateSeq = 0;
	static int s_currentClientCommandAck = 0;
	static bool s_hasLastDeliveredClientCommandAck = false;
	static int s_lastDeliveredClientCommandAck = 0;
	static bool s_has_last_outgoing_command_number = false;
	static int s_last_outgoing_command_number = 0;
	static netadr_t s_registrationRemoteAddress;
	static bool s_adapterAddressLogged = false;
	static bool s_adapterLoopbackLogged = false;
	static bool s_adapterNullLogged = false;
	static bool s_adapterResetLogged = false;
	static bool s_adapterClearLogged = false;
	static bool s_adapterRemoteFramerateLogged = false;
	static bool s_adapterTimingOutLogged = false;
	static bool s_adapterTimeoutLogged = false;
	static bool s_adapterReceiveTimeLogged = false;
	static bool s_adapterChokedLogged = false;
	static bool s_adapterCanPacketLogged = false;
	static bool s_adapterOverflowLogged = false;
	static bool s_adapterTimedOutLogged = false;
	static bool s_adapterPendingReliableLogged = false;
	static bool s_adapterSequenceLogged = false;
	static bool s_adapterTransmitLogged = false;
	static bool s_adapterRemoteDisconnectedLogged = false;
	static bool s_adapterAvgLatencyLogged = false;
	static bool s_adapterAvgLossLogged = false;
	static bool s_adapterAvgChokeLogged = false;
	static bool s_adapterAvgPacketsLogged = false;
	static bool s_adapterRemoteFramerateInfoLogged = false;
	static bool s_adapterDataRateLogged = false;
	static bool s_adapterInterpolationLogged = false;
	static bool s_adapterMessageStatsLogged = false;
	static bool s_adapterMaxBufferLogged = false;

	static void ResetCompatibilityState( void )
	{
		s_compatibilityFatal = false;
		s_packetStarted = false;
		s_currentServerUpdateSeq = 0;
		s_currentClientCommandAck = 0;
		s_hasLastDeliveredClientCommandAck = false;
		s_lastDeliveredClientCommandAck = 0;
		s_has_last_outgoing_command_number = false;
		s_last_outgoing_command_number = 0;
	}

	static void ResetConnectionGenerationState( void )
	{
		s_challengePrimed = false;
		s_connectedPrimed = false;
		s_localConnectedPrime = false;
		s_localConnectedPrimeState = -1;
		s_localConnectedPrimeSignonSuppressed = false;
		s_localConnectedPrimeSendFailed = false;
		s_connectedPrimeSetConVarSeen = false;
	}

	// Engine callers pass the channel object in ECX. The fastcall shim absorbs
	// the unused EDX position while preserving each observed stack argument.
	static bool __fastcall RegistrationAdapterRegisterMessage( void *self, void *,
		void *message );
	static const char * __fastcall RegistrationAdapterGetAddress( void *self, void * );
	static const netadr_t & __fastcall RegistrationAdapterGetRemoteAddress( void *self,
		void * );
	static bool __fastcall RegistrationAdapterIsLoopback( void *self, void * );
	static bool __fastcall RegistrationAdapterIsNull( void *self, void * );
	static void __fastcall RegistrationAdapterReset( void *self, void * );
	static void __fastcall RegistrationAdapterClear( void *self, void * );
	static void __fastcall RegistrationAdapterShutdown( void *self, void *,
		const char *reason );
	static void __fastcall RegistrationAdapterSetRemoteFramerate( void *self, void *,
		float frameTime, float frameTimeStdDeviation );
	static bool __fastcall RegistrationAdapterIsTimingOut( void *self, void * );
	static float __fastcall RegistrationAdapterGetTimeoutSeconds( void *self, void * );
	static float __fastcall RegistrationAdapterGetTimeSinceLastReceived( void *self,
		void * );
	static void __fastcall RegistrationAdapterSetChoked( void *self, void * );
	static int __fastcall RegistrationAdapterSendDatagram( void *self, void *, void *data );
	static bool __fastcall RegistrationAdapterCanPacket( void *self, void * );
	static bool __fastcall RegistrationAdapterIsOverflowed( void *self, void * );
	static bool __fastcall RegistrationAdapterIsTimedOut( void *self, void * );
	static bool __fastcall RegistrationAdapterHasPendingReliableData( void *self,
		void * );
	static int __fastcall RegistrationAdapterGetSequenceNr( void *self, void *, int flow );
	static bool __fastcall RegistrationAdapterTransmit( void *self, void *,
		bool onlyReliable );
	static bool __fastcall RegistrationAdapterIsRemoteDisconnected( void *self,
		void * );
	static bool __fastcall RegistrationAdapterSendNetMsg( void *self, void *,
		void *message, bool forceReliable, bool voice );
	static float __fastcall RegistrationAdapterGetAvgLatency( void *self, void *,
		int flow );
	static float __fastcall RegistrationAdapterGetAvgLoss( void *self, void *,
		int flow );
	static float __fastcall RegistrationAdapterGetAvgChoke( void *self, void *,
		int flow );
	static float __fastcall RegistrationAdapterGetAvgPackets( void *self, void *,
		int flow );
	static void __fastcall RegistrationAdapterGetRemoteFramerate( void *self, void *,
		float *frameTime, float *frameTimeStdDeviation );
	static void __fastcall RegistrationAdapterSetDataRate( void *self, void *,
		float rate );
	static void __fastcall RegistrationAdapterSetInterpolationAmount( void *self,
		void *, float amount );
	static void __fastcall RegistrationAdapterUpdateMessageStats( void *self, void *,
		int messageGroup, int bits );
	static void __fastcall RegistrationAdapterSetTimeout( void *self, void *,
		float seconds, bool forceExact );
	static void __fastcall RegistrationAdapterSetMaxBufferSize( void *self, void *,
		bool reliable, int bytes, bool voice );

#pragma pack(push, 1)
	struct SharedClientState
	{
		DWORD magic;
		DWORD version;
		LONG state;
		DWORD connection;
		LONG reason;
		LONG intentCount;
		LONG takeoverCount;
		LONG eventCount;
		LONG wrapperInitialized;
		char endpoint[ kEndpointCapacity ];
	};
#pragma pack(pop)

	static HANDLE s_mapping = NULL;
	static HANDLE s_mutex = NULL;
	static SharedClientState *s_state = NULL;

	// Invalid connect intents must be rejectable without opening or mutating the
	// shared lifecycle mapping, so a healthy generation remains untouched. The
	// shared fields are fixed-width Win32 values; CompareExchange with identical
	// comparand/exchange values provides an atomic no-op peek without taking the
	// lifecycle mutex or initializing the mapping.
	static bool HasExistingClientGeneration( void )
	{
		if ( s_sourceContextBound || s_sourceContext != NULL ||
			s_sourceChannelPersistent || s_dispatchNetChannelBound ||
			s_challengePrimed || s_connectedPrimed || s_packetStarted )
			return true;
		if ( !s_state )
			return false;

		const LONG sharedWrapperInitialized = InterlockedCompareExchange(
			&s_state->wrapperInitialized, 0, 0 );
		// SharedClientState::connection is a DWORD on the fixed x86 ABI, so a
		// 32-bit LONG interlocked read is width-compatible and does not change
		// the stored bits.
		const LONG sharedConnection = InterlockedCompareExchange(
			(volatile LONG *)&s_state->connection, 0, 0 );
		return sharedWrapperInitialized != 0 ||
			(DWORD)sharedConnection != (DWORD)ASRD_GNS_CONNECTION_INVALID;
	}

	static const char *StateName( ASRD_GNS_ClientConnectionState state )
	{
		switch ( state )
		{
		case ASRD_GNS_CLIENT_PENDING: return "pending";
		case ASRD_GNS_CLIENT_CONNECTED: return "connected";
		case ASRD_GNS_CLIENT_FAILED: return "failed";
		case ASRD_GNS_CLIENT_CLOSED: return "closed";
		default: return "disconnected";
		}
	}

	static void LogClient( const char *message )
	{
		Warning( "[ASRD-GNS-CLIENT] %s\n", message );
	}

	static void LogClientf( const char *format, ... )
	{
		char buffer[ 512 ];
		va_list args;
		va_start( args, format );
		_vsnprintf( buffer, sizeof( buffer ) - 1, format, args );
		buffer[ sizeof( buffer ) - 1 ] = '\0';
		va_end( args );
		LogClient( buffer );
	}

	static bool MakeObjectName( const char *prefix, char *name, size_t capacity )
	{
		if ( !prefix || !name || capacity == 0 )
			return false;
		const int written = _snprintf( name, capacity, "Local\\%s_%08lX",
			prefix, (unsigned long)GetCurrentProcessId() );
		return written > 0 && (size_t)written < capacity;
	}

	static bool OpenClientState( void )
	{
		if ( s_state && s_mutex )
			return true;

		char mappingName[ 96 ];
		char mutexName[ 96 ];
		if ( !MakeObjectName( kClientStatePrefix, mappingName, sizeof( mappingName ) ) ||
			!MakeObjectName( "ASRD_GNS_CLIENT_LIFECYCLE_LOCK", mutexName, sizeof( mutexName ) ) )
			return false;

		if ( !s_mapping )
		{
			s_mapping = CreateFileMappingA( INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
				0, (DWORD)sizeof( SharedClientState ), mappingName );
			if ( !s_mapping )
				return false;
		}
		if ( !s_state )
		{
			s_state = (SharedClientState *)MapViewOfFile( s_mapping, FILE_MAP_ALL_ACCESS, 0, 0,
				sizeof( SharedClientState ) );
		}
		if ( !s_state )
			return false;
		if ( !s_mutex )
			s_mutex = CreateMutexA( NULL, FALSE, mutexName );
		return s_mutex != NULL;
	}

	static bool LockClientState( void )
	{
		if ( !OpenClientState() )
			return false;
		const DWORD waitResult = WaitForSingleObject( s_mutex, 10000 );
		if ( waitResult != WAIT_OBJECT_0 && waitResult != WAIT_ABANDONED )
			return false;
		if ( s_state->magic != kClientStateMagic || s_state->version != kClientStateVersion )
		{
			memset( s_state, 0, sizeof( *s_state ) );
			s_state->magic = kClientStateMagic;
			s_state->version = kClientStateVersion;
		}
		return true;
	}

	static void UnlockClientState( void )
	{
		if ( s_mutex )
			ReleaseMutex( s_mutex );
	}

	static bool IsIPv4( const char *address, unsigned int octets[ 4 ] )
	{
		if ( !address || !address[ 0 ] || !octets )
			return false;

		const char *cursor = address;
		for ( int i = 0; i < 4; ++i )
		{
			char *end = NULL;
			const unsigned long value = strtoul( cursor, &end, 10 );
			if ( end == cursor || value > 255 )
				return false;
			octets[ i ] = (unsigned int)value;
			if ( i < 3 )
			{
				if ( *end != '.' )
					return false;
				cursor = end + 1;
			}
			else if ( *end != '\0' )
			{
				return false;
			}
		}
		return true;
	}

	static bool ParseEndpoint( const char *endpoint, char *ipv4, size_t ipv4Capacity,
		uint16_t *port )
	{
		if ( !endpoint || !ipv4 || !port || ipv4Capacity == 0 )
			return false;

		const char *colon = strrchr( endpoint, ':' );
		if ( colon && ( colon == endpoint || strchr( endpoint, ':' ) != colon ) )
			return false;
		const size_t addressLength = colon ? (size_t)( colon - endpoint ) : strlen( endpoint );
		if ( addressLength == 0 || addressLength >= ipv4Capacity )
			return false;
		memcpy( ipv4, endpoint, addressLength );
		ipv4[ addressLength ] = '\0';

		unsigned int octets[ 4 ] = {};
		if ( !IsIPv4( ipv4, octets ) )
			return false;
		// The client must never initiate a loopback or unspecified connection.
		// Keep the check here as well as in the wrapper so the decision is logged
		// at the connect-intent boundary.
		if ( octets[ 0 ] == 0 || octets[ 0 ] == 127 || octets[ 0 ] >= 224 )
			return false;

		if ( colon )
		{
			char *portEnd = NULL;
			const unsigned long parsedPort = strtoul( colon + 1, &portEnd, 10 );
			if ( portEnd == colon + 1 || *portEnd != '\0' || parsedPort == 0 || parsedPort > 65535 )
				return false;
			*port = (uint16_t)parsedPort;
		}
		else
		{
			*port = kDefaultClientConnectPort;
		}
		return true;
	}

	static void SetState( ASRD_GNS_ClientConnectionState state, ASRD_GNS_Connection connection,
		int reason, bool clearConnection )
	{
		if ( !LockClientState() )
		{
			LogClient( "state update skipped: shared state unavailable" );
			return;
		}
		const ASRD_GNS_ClientConnectionState previous =
			(ASRD_GNS_ClientConnectionState)s_state->state;
		s_state->state = (LONG)state;
		s_state->reason = reason;
		if ( connection != ASRD_GNS_CONNECTION_INVALID )
			s_state->connection = connection;
		else if ( clearConnection )
			s_state->connection = ASRD_GNS_CONNECTION_INVALID;
		UnlockClientState();

		if ( previous != state || state == ASRD_GNS_CLIENT_FAILED || state == ASRD_GNS_CLIENT_CLOSED )
		{
			LogClientf( "lifecycle state=%s reason=%d handle=%lu",
				StateName( state ), reason, (unsigned long)connection );
		}
	}

	static ASRD_GNS_Connection GetConnection( bool *wrapperInitialized )
	{
		if ( wrapperInitialized )
			*wrapperInitialized = false;
		if ( !LockClientState() )
			return ASRD_GNS_CONNECTION_INVALID;
		const ASRD_GNS_Connection connection = (ASRD_GNS_Connection)s_state->connection;
		if ( wrapperInitialized )
			*wrapperInitialized = s_state->wrapperInitialized != 0;
		UnlockClientState();
		return connection;
	}

	static void LogContext( const char *message )
	{
		Warning( "[ASRD-GNS-CLIENT-CONTEXT] %s\n", message );
	}

	static void LogContextf( const char *format, ... )
	{
		char buffer[ 512 ];
		va_list args;
		va_start( args, format );
		_vsnprintf( buffer, sizeof( buffer ) - 1, format, args );
		buffer[ sizeof( buffer ) - 1 ] = '\0';
		va_end( args );
		LogContext( buffer );
	}

	static bool IsExpectedEngineImage( HMODULE engineModule )
	{
		if ( !engineModule )
			return false;

		const BYTE *base = (const BYTE *)engineModule;
		const IMAGE_DOS_HEADER *dos = (const IMAGE_DOS_HEADER *)base;
		if ( dos->e_magic != IMAGE_DOS_SIGNATURE || dos->e_lfanew <= 0 ||
			dos->e_lfanew > 0x100000 )
			return false;

		const IMAGE_NT_HEADERS32 *nt =
			(const IMAGE_NT_HEADERS32 *)( base + dos->e_lfanew );
		return nt->Signature == IMAGE_NT_SIGNATURE &&
			nt->FileHeader.Machine == IMAGE_FILE_MACHINE_I386 &&
			nt->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC &&
			nt->OptionalHeader.ImageBase == 0x10000000 &&
			nt->FileHeader.TimeDateStamp == kEngineTimeDateStamp &&
			nt->OptionalHeader.SizeOfImage == kEngineSizeOfImage;
	}

	static bool IsEngineAddress( HMODULE engineModule, const void *address, size_t bytes )
	{
		if ( !engineModule || !address )
			return false;
		const BYTE *base = (const BYTE *)engineModule;
		const BYTE *value = (const BYTE *)address;
		const BYTE *end = base + kEngineSizeOfImage;
		return value >= base && value <= end && bytes <= (size_t)( end - value );
	}

	static bool IsExpectedSourceContext( void )
	{
		if ( sizeof( void * ) != 4 || !s_sourceContextBound || !s_sourceContext )
			return false;

		HMODULE engineModule = GetModuleHandleA( "engine.dll" );
		if ( !IsExpectedEngineImage( engineModule ) )
			return false;
		return *(void ***)s_sourceContext ==
			(void *)( (BYTE *)engineModule + kClientStateVtableRva );
	}

	static bool ReadSourceSignonState( int *state )
	{
		if ( !state || !IsExpectedSourceContext() )
			return false;
		*state = *(int *)( (BYTE *)s_sourceContext +
			kClientStateSignonStateOffset );
		return *state >= SIGNONSTATE_NONE &&
			*state <= SIGNONSTATE_CHANGELEVEL;
	}

	static bool SourceAdapterSlotMatches( void )
	{
		if ( !IsExpectedSourceContext() || !s_registrationAdapterReady ||
			s_registrationAdapter.vtable != s_registrationVtable )
			return false;
		void **netChannelSlot = (void **)( (BYTE *)s_sourceContext +
			kClientStateNetChannelOffset );
		return *netChannelSlot == &s_registrationAdapter;
	}

	static bool RestoreChangelevelSourceAdapter( void )
	{
		if ( !IsExpectedSourceContext() || !s_registrationAdapterReady ||
			s_registrationAdapter.vtable != s_registrationVtable )
			return false;

		void **netChannelSlot = (void **)( (BYTE *)s_sourceContext +
			kClientStateNetChannelOffset );
		if ( *netChannelSlot != &s_registrationAdapter )
		{
			LogContextf( "changelevel adapter restore slot=%p previous=%p adapter=%p handle=%lu",
				netChannelSlot, *netChannelSlot, &s_registrationAdapter,
				(unsigned long)ASRD_GNS_ClientConnection() );
			*netChannelSlot = &s_registrationAdapter;
		}
		s_sourceChannelPersistent = true;
		s_dispatchPreviousNetChannel = NULL;
		s_dispatchNetChannelBound = false;
		s_sourceReattachPending = false;
		return true;
	}

	static bool RefreshSourceLifecyclePhase( void )
	{
		int signonState = SIGNONSTATE_NONE;
		if ( !ReadSourceSignonState( &signonState ) )
			return true;

		if ( signonState == SIGNONSTATE_CHANGELEVEL )
		{
			if ( s_sourceLifecyclePhase != SOURCE_LIFECYCLE_CHANGELEVEL )
			{
				s_sourceLifecyclePhase = SOURCE_LIFECYCLE_CHANGELEVEL;
				LogContextf( "source lifecycle phase=changelevel handle=%lu adapter=%p context=%p",
					(unsigned long)ASRD_GNS_ClientConnection(),
					&s_registrationAdapter, s_sourceContext );
			}
			if ( s_sourceReattachPending || !SourceAdapterSlotMatches() )
				return RestoreChangelevelSourceAdapter();
			return true;
		}

		if ( s_sourceLifecyclePhase != SOURCE_LIFECYCLE_CHANGELEVEL )
			return true;

		if ( signonState == SIGNONSTATE_NONE )
		{
			// Source may temporarily drop its upper channel while rebinding the
			// same server. Keep GNS alive but leave dispatch gated until a same-
			// endpoint intent or signon progression restores this adapter.
			s_sourceReattachPending = true;
			return true;
		}

		if ( s_sourceReattachPending || !SourceAdapterSlotMatches() )
		{
			if ( !RestoreChangelevelSourceAdapter() )
				return false;
		}
		if ( signonState >= SIGNONSTATE_CONNECTED )
		{
			s_sourceLifecyclePhase = SOURCE_LIFECYCLE_RUNNING;
			LogContextf( "source lifecycle phase=running signon=%d handle=%lu adapter=%p context=%p",
				signonState, (unsigned long)ASRD_GNS_ClientConnection(),
				&s_registrationAdapter, s_sourceContext );
		}
		return true;
	}

	static bool ResolveClientSetSignonState( ClientSetSignonStateFn *out )
	{
		if ( out )
			*out = NULL;
		if ( !out || sizeof( void * ) != 4 || !s_sourceContextBound ||
			!s_sourceContext || !s_registrationAdapterReady ||
			s_registrationAdapter.vtable != s_registrationVtable )
			return false;

		HMODULE engineModule = GetModuleHandleA( "engine.dll" );
		if ( !IsExpectedEngineImage( engineModule ) )
		{
			LogContext( "SetSignonState rejected: engine image mismatch" );
			return false;
		}

		BYTE *base = (BYTE *)engineModule;
		void **clientVtable = *(void ***)s_sourceContext;
		void *expectedVtable = (void *)( base + kClientStateVtableRva );
		if ( !clientVtable || clientVtable != (void **)expectedVtable ||
			!IsEngineAddress( engineModule, clientVtable,
				sizeof( void * ) * ( kClientSetSignonStateSlot + 1 ) ) )
		{
			LogContextf( "SetSignonState rejected: client context vtable invalid state=%p vtable=%p expected=%p",
				s_sourceContext, clientVtable, expectedVtable );
			return false;
		}

		void *address = clientVtable[ kClientSetSignonStateSlot ];
		void *expectedAddress = (void *)( base + kClientSetSignonStateRva );
		LogContextf( "SetSignonState resolve state=%p vtable=%p slot=%u target=%p expected=%p",
			s_sourceContext, clientVtable, kClientSetSignonStateSlot,
			address, expectedAddress );
		if ( address != expectedAddress ||
			!IsEngineAddress( engineModule, address, 1 ) )
		{
			LogContext( "SetSignonState rejected: most-derived target mismatch" );
			return false;
		}

		*out = (ClientSetSignonStateFn)(uintptr_t)address;
		return true;
	}

	static bool InvokeClientSetSignonState( int state, int count )
	{
		ClientSetSignonStateFn setSignonState = NULL;
		if ( !ResolveClientSetSignonState( &setSignonState ) )
			return false;

		LogContextf( "SetSignonState begin state=%d count=%d reserved=0",
			state, count );
		const bool result = setSignonState( s_sourceContext, state, count, 0 );
		LogContextf( "SetSignonState complete state=%d count=%d result=%u",
			state, count, result ? 1U : 0U );
		return result;
	}

	static bool PrimeSourceChallengeState( void )
	{
		if ( s_challengePrimed )
			return true;
		if ( !s_sourceContextBound || !s_sourceContext ||
			!s_registrationAdapterReady )
		{
			LogContext( "challenge prime rejected: Source context or adapter invalid" );
			return false;
		}

		if ( !InvokeClientSetSignonState( SIGNONSTATE_CHALLENGE, -1 ) )
		{
			LogContext( "challenge prime failed state=CHALLENGE(1)" );
			return false;
		}
		s_challengePrimed = true;
		LogContext( "challenge prime complete state=CHALLENGE(1) count=-1" );
		return true;
	}

	static bool PrimeSourceConnectedState( void )
	{
		if ( s_connectedPrimed )
			return true;
		if ( !s_challengePrimed || !s_sourceContextBound || !s_sourceContext ||
			!s_registrationAdapterReady ||
			s_registrationAdapter.vtable != s_registrationVtable ||
			ASRD_GNS_ClientConnection() == ASRD_GNS_CONNECTION_INVALID )
		{
			LogContext( "connected prime rejected: Source context, adapter, challenge, or GNS connection invalid" );
			return false;
		}

		if ( !ASRD_GNS_ClientBindSourceChannelForDispatch() )
		{
			LogContext( "connected prime rejected: adapter channel bind failed" );
			return false;
		}

		s_localConnectedPrime = true;
		s_localConnectedPrimeState = SIGNONSTATE_CONNECTED;
		s_localConnectedPrimeSignonSuppressed = false;
		s_localConnectedPrimeSendFailed = false;

		const bool signonResult =
			InvokeClientSetSignonState( SIGNONSTATE_CONNECTED, -1 );
		const bool signonSuppressed = s_localConnectedPrimeSignonSuppressed;
		const bool normalSendFailed = s_localConnectedPrimeSendFailed;
		if ( !signonResult || !signonSuppressed || normalSendFailed )
		{
			s_localConnectedPrime = false;
			s_localConnectedPrimeState = -1;
			s_localConnectedPrimeSignonSuppressed = false;
			s_localConnectedPrimeSendFailed = false;
			ASRD_GNS_ClientUnbindSourceChannelForDispatch();
			LogContextf( "connected prime failed result=%u signon_suppressed=%u normal_send_failed=%u set_convar_seen=%u",
				signonResult ? 1U : 0U, signonSuppressed ? 1U : 0U,
				normalSendFailed ? 1U : 0U,
				s_connectedPrimeSetConVarSeen ? 1U : 0U );
			return false;
		}

		s_localConnectedPrime = false;
		s_localConnectedPrimeState = -1;
		s_localConnectedPrimeSignonSuppressed = false;
		s_localConnectedPrimeSendFailed = false;
		// CONNECTED makes the normal Source frame pump dereference
		// m_NetChannel.  Keep the GNS adapter installed for this generation;
		// restoring NULL here would make the next engine frame crash even though
		// the Source signon state is already CONNECTED.
		ASRD_GNS_ClientPromoteSourceChannelForLifecycle();
		if ( !s_sourceChannelPersistent )
		{
			ASRD_GNS_ClientUnbindSourceChannelForDispatch();
			LogContext( "connected prime failed: adapter lifecycle promotion failed" );
			return false;
		}

		s_connectedPrimed = true;
		LogContextf( "connected prime complete state=CONNECTED(2) count=-1 signon_suppressed=%u set_convar_seen=%u",
			signonSuppressed ? 1U : 0U,
			s_connectedPrimeSetConVarSeen ? 1U : 0U );
		return true;
	}

	static void AbortClientGeneration( const char *reason )
	{
		const ASRD_GNS_Connection connection = ASRD_GNS_ClientConnection();
		LogClientf( "connection generation abort reason=%s handle=%lu action=discard_shutdown_detach_no_legacy_fallback",
			reason ? reason : "unspecified", (unsigned long)connection );
		ASRD_GNS_MessageBridgeDiscardQueued( connection );
		ASRD_GNS_ClientShutdown();
		SetState( ASRD_GNS_CLIENT_FAILED, ASRD_GNS_CONNECTION_INVALID, 0, true );
	}

	static bool EnsureRegistrationAdapter( void )
	{
		if ( s_registrationAdapterReady )
			return true;
		if ( sizeof( void * ) != 4 )
		{
			LogContext( "registration adapter rejected: non-x86 process" );
			return false;
		}

		memset( s_registrationVtable, 0, sizeof( s_registrationVtable ) );
		// Retained adapter slot assignments are cross-referenced to the engine
		// call-site disassembly in
		// src/game/gns_wrapper/disassembly_evidence/engine_bindings.md.
		s_registrationVtable[ 1 ] =
			(void *)(uintptr_t)&RegistrationAdapterGetAddress;
		s_registrationVtable[ 6 ] =
			(void *)(uintptr_t)&RegistrationAdapterIsLoopback;
		s_registrationVtable[ 29 ] =
			(void *)(uintptr_t)&RegistrationAdapterRegisterMessage;
		s_registrationVtable[ 35 ] =
			(void *)(uintptr_t)&RegistrationAdapterReset;
		s_registrationVtable[ 36 ] =
			(void *)(uintptr_t)&RegistrationAdapterClear;
		s_registrationVtable[ 37 ] =
			(void *)(uintptr_t)&RegistrationAdapterShutdown;
		s_registrationVtable[ 49 ] =
			(void *)(uintptr_t)&RegistrationAdapterGetRemoteAddress;
		s_registrationVtable[ 65 ] =
			(void *)(uintptr_t)&RegistrationAdapterIsNull;
		s_registrationVtable[ 68 ] =
			(void *)(uintptr_t)&RegistrationAdapterSetRemoteFramerate;
		s_registrationVtable[ 7 ] =
			(void *)(uintptr_t)&RegistrationAdapterIsTimingOut;
		s_registrationVtable[ 22 ] =
			(void *)(uintptr_t)&RegistrationAdapterGetTimeSinceLastReceived;
		s_registrationVtable[ 26 ] =
			(void *)(uintptr_t)&RegistrationAdapterGetTimeoutSeconds;
		s_registrationVtable[ 46 ] =
			(void *)(uintptr_t)&RegistrationAdapterSetChoked;
		s_registrationVtable[ 47 ] =
			(void *)(uintptr_t)&RegistrationAdapterSendDatagram;
		s_registrationVtable[ 57 ] =
			(void *)(uintptr_t)&RegistrationAdapterCanPacket;
		s_registrationVtable[ 58 ] =
			(void *)(uintptr_t)&RegistrationAdapterIsOverflowed;
		s_registrationVtable[ 59 ] =
			(void *)(uintptr_t)&RegistrationAdapterIsTimedOut;
		s_registrationVtable[ 60 ] =
			(void *)(uintptr_t)&RegistrationAdapterHasPendingReliableData;
		s_registrationVtable[ 17 ] =
			(void *)(uintptr_t)&RegistrationAdapterGetSequenceNr;
		s_registrationVtable[ 48 ] =
			(void *)(uintptr_t)&RegistrationAdapterTransmit;
		s_registrationVtable[ 74 ] =
			(void *)(uintptr_t)&RegistrationAdapterIsRemoteDisconnected;
		s_registrationVtable[ 41 ] =
			(void *)(uintptr_t)&RegistrationAdapterSendNetMsg;
		s_registrationVtable[ 10 ] =
			(void *)(uintptr_t)&RegistrationAdapterGetAvgLatency;
		s_registrationVtable[ 11 ] =
			(void *)(uintptr_t)&RegistrationAdapterGetAvgLoss;
		s_registrationVtable[ 12 ] =
			(void *)(uintptr_t)&RegistrationAdapterGetAvgChoke;
		s_registrationVtable[ 14 ] =
			(void *)(uintptr_t)&RegistrationAdapterGetAvgPackets;
		s_registrationVtable[ 25 ] =
			(void *)(uintptr_t)&RegistrationAdapterGetRemoteFramerate;
		s_registrationVtable[ 28 ] =
			(void *)(uintptr_t)&RegistrationAdapterSetDataRate;
		s_registrationVtable[ 67 ] =
			(void *)(uintptr_t)&RegistrationAdapterSetInterpolationAmount;
		s_registrationVtable[ 56 ] =
			(void *)(uintptr_t)&RegistrationAdapterUpdateMessageStats;
		s_registrationVtable[ 32 ] =
			(void *)(uintptr_t)&RegistrationAdapterSetTimeout;
		s_registrationVtable[ 64 ] =
			(void *)(uintptr_t)&RegistrationAdapterSetMaxBufferSize;
		s_registrationAdapter.vtable = s_registrationVtable;
		s_registrationAdapterReady = true;
		LogContextf( "registration adapter ready=%u self=%p addressSlot=1 loopbackSlot=6 registerSlot=29 resetSlot=35 clearSlot=36 shutdownSlot=37 remoteAddressSlot=49 nullSlot=65 remoteFramerateSlot=68",
			1U, &s_registrationAdapter );
		return true;
	}

	static const char * __fastcall RegistrationAdapterGetAddress( void *self, void * )
	{
		if ( self != &s_registrationAdapter )
		{
			LogContextf( "adapter unexpected GetAddress self=%p expected=%p",
				self, &s_registrationAdapter );
			return "GNS";
		}
		if ( !s_adapterAddressLogged )
		{
			s_adapterAddressLogged = true;
			LogContextf( "adapter GetAddress served value=%s",
				s_registrationRemoteAddress.ToString() );
		}
		return s_registrationRemoteAddress.ToString();
	}

	static const netadr_t & __fastcall RegistrationAdapterGetRemoteAddress( void *self,
		void * )
	{
		if ( self != &s_registrationAdapter )
			LogContextf( "adapter unexpected GetRemoteAddress self=%p expected=%p",
				self, &s_registrationAdapter );
		if ( !s_adapterAddressLogged )
		{
			s_adapterAddressLogged = true;
			LogContextf( "adapter GetRemoteAddress served value=%s",
				s_registrationRemoteAddress.ToString() );
		}
		return s_registrationRemoteAddress;
	}

	static bool __fastcall RegistrationAdapterIsLoopback( void *self, void * )
	{
		if ( self != &s_registrationAdapter )
		{
			LogContextf( "adapter unexpected IsLoopback self=%p expected=%p",
				self, &s_registrationAdapter );
			return true;
		}
		if ( !s_adapterLoopbackLogged )
		{
			s_adapterLoopbackLogged = true;
			LogContext( "adapter IsLoopback served value=0" );
		}
		return false;
	}

	static bool __fastcall RegistrationAdapterIsNull( void *self, void * )
	{
		if ( self != &s_registrationAdapter )
		{
			LogContextf( "adapter unexpected IsNull self=%p expected=%p",
				self, &s_registrationAdapter );
			return true;
		}
		if ( !s_adapterNullLogged )
		{
			s_adapterNullLogged = true;
			LogContext( "adapter IsNull served value=0" );
		}
		return false;
	}

	static void __fastcall RegistrationAdapterReset( void *self, void * )
	{
		if ( self != &s_registrationAdapter )
		{
			LogContextf( "adapter unexpected Reset self=%p expected=%p",
				self, &s_registrationAdapter );
			return;
		}
		if ( !s_adapterResetLogged )
		{
			s_adapterResetLogged = true;
			LogContext( "adapter Reset served action=noop" );
		}
	}

	static void __fastcall RegistrationAdapterClear( void *self, void * )
	{
		if ( self != &s_registrationAdapter )
		{
			LogContextf( "adapter unexpected Clear self=%p expected=%p",
				self, &s_registrationAdapter );
			return;
		}
		if ( !s_adapterClearLogged )
		{
			s_adapterClearLogged = true;
			LogContext( "adapter Clear served action=noop" );
		}
	}

	static void __fastcall RegistrationAdapterShutdown( void *self, void *,
		const char *reason )
	{
		if ( self != &s_registrationAdapter )
		{
			LogContextf( "adapter unexpected Shutdown self=%p expected=%p reason=%s",
				self, &s_registrationAdapter, reason ? reason : "<null>" );
			return;
		}

		if ( s_clientShutdownInProgress )
		{
			LogContextf( "adapter Shutdown reentrant reason=%s action=ignore",
				reason ? reason : "<null>" );
			return;
		}

		int signonState = SIGNONSTATE_NONE;
		const bool hasSignonState = ReadSourceSignonState( &signonState );
		if ( ( hasSignonState && signonState == SIGNONSTATE_CHANGELEVEL ) ||
			s_sourceLifecyclePhase == SOURCE_LIFECYCLE_CHANGELEVEL )
		{
			s_sourceLifecyclePhase = SOURCE_LIFECYCLE_CHANGELEVEL;
			s_sourceReattachPending = true;
			LogContextf( "adapter Shutdown deferred reason=%s signon=%d handle=%lu action=preserve_changelevel_transport",
				reason ? reason : "<null>", hasSignonState ? signonState : -1,
				(unsigned long)ASRD_GNS_ClientConnection() );
			return;
		}

		const ASRD_GNS_Connection connection = ASRD_GNS_ClientConnection();
		LogContextf( "adapter Shutdown begin reason=%s handle=%lu persistent=%u action=detach_then_gns_close",
			reason ? reason : "<null>", (unsigned long)connection,
			s_sourceChannelPersistent ? 1U : 0U );

		// The lifecycle disconnect path clears the channel slot after this callback
		// returns. Detach the persistent adapter first so nested cleanup cannot
		// call through a stale lifecycle binding.
		if ( s_sourceContext && s_sourceChannelPersistent )
		{
			void **netChannelSlot = (void **)( (BYTE *)s_sourceContext +
				kClientStateNetChannelOffset );
			if ( *netChannelSlot == &s_registrationAdapter )
				*netChannelSlot = NULL;
			s_sourceChannelPersistent = false;
			LogContextf( "adapter Shutdown detached persistent slot=%p",
				netChannelSlot );
		}

		// Reuse the existing lifecycle close path after detaching the Source
		// pointer.  Its guard prevents duplicate cleanup if Source invokes this
		// slot again while the upper-layer disconnect is unwinding.
		ASRD_GNS_ClientShutdown();
		LogContextf( "adapter Shutdown complete reason=%s handle=%lu",
			reason ? reason : "<null>", (unsigned long)connection );
	}

	static void __fastcall RegistrationAdapterSetRemoteFramerate( void *self, void *,
		float frameTime, float frameTimeStdDeviation )
	{
		if ( self != &s_registrationAdapter )
		{
			LogContextf( "adapter unexpected SetRemoteFramerate self=%p expected=%p",
				self, &s_registrationAdapter );
			return;
		}
		if ( !s_adapterRemoteFramerateLogged )
		{
			s_adapterRemoteFramerateLogged = true;
			LogContextf( "adapter SetRemoteFramerate frame=%f deviation=%f action=noop",
				frameTime, frameTimeStdDeviation );
		}
	}

	static bool __fastcall RegistrationAdapterIsTimingOut( void *self, void * )
	{
		if ( self != &s_registrationAdapter )
			LogContextf( "adapter unexpected IsTimingOut self=%p expected=%p",
				self, &s_registrationAdapter );
		if ( !s_adapterTimingOutLogged )
		{
			s_adapterTimingOutLogged = true;
			LogContext( "adapter IsTimingOut served value=0" );
		}
		return false;
	}

	static float __fastcall RegistrationAdapterGetTimeoutSeconds( void *self, void * )
	{
		if ( self != &s_registrationAdapter )
			LogContextf( "adapter unexpected GetTimeoutSeconds self=%p expected=%p",
				self, &s_registrationAdapter );
		if ( !s_adapterTimeoutLogged )
		{
			s_adapterTimeoutLogged = true;
			LogContext( "adapter GetTimeoutSeconds served value=300.0" );
		}
		return 300.0f;
	}

	static float __fastcall RegistrationAdapterGetTimeSinceLastReceived( void *self,
		void * )
	{
		if ( self != &s_registrationAdapter )
			LogContextf( "adapter unexpected GetTimeSinceLastReceived self=%p expected=%p",
				self, &s_registrationAdapter );
		if ( !s_adapterReceiveTimeLogged )
		{
			s_adapterReceiveTimeLogged = true;
			LogContext( "adapter GetTimeSinceLastReceived served value=0" );
		}
		return 0.0f;
	}

	static void __fastcall RegistrationAdapterSetChoked( void *self, void * )
	{
		if ( self != &s_registrationAdapter )
			LogContextf( "adapter unexpected SetChoked self=%p expected=%p",
				self, &s_registrationAdapter );
		if ( !s_adapterChokedLogged )
		{
			s_adapterChokedLogged = true;
			LogContext( "adapter SetChoked action=noop" );
		}
	}

	static int __fastcall RegistrationAdapterSendDatagram( void *self, void *, void *data )
	{
		if ( self != &s_registrationAdapter )
		{
			LogContextf( "adapter unexpected SendDatagram self=%p expected=%p data=%p",
				self, &s_registrationAdapter, data );
			return 0;
		}

		// Source assigns the int return value of SendDatagram(NULL) to its
		// engine-owned lastoutgoingcommand. Keep that ABI and return the highest
		// successfully serialized CUserCmd.command_number, rather than a boolean
		// transport status. The GNS wrapper has no legacy packet to send here.
		const int returnValue = s_has_last_outgoing_command_number
			? s_last_outgoing_command_number : 0;
		return returnValue;
	}

	static bool __fastcall RegistrationAdapterCanPacket( void *self, void * )
	{
		if ( self != &s_registrationAdapter )
			LogContextf( "adapter unexpected CanPacket self=%p expected=%p",
				self, &s_registrationAdapter );
		if ( !s_adapterCanPacketLogged )
		{
			s_adapterCanPacketLogged = true;
			LogContext( "adapter CanPacket served value=1" );
		}
		return true;
	}

	static bool __fastcall RegistrationAdapterIsOverflowed( void *self, void * )
	{
		if ( self != &s_registrationAdapter )
			LogContextf( "adapter unexpected IsOverflowed self=%p expected=%p",
				self, &s_registrationAdapter );
		if ( !s_adapterOverflowLogged )
		{
			s_adapterOverflowLogged = true;
			LogContext( "adapter IsOverflowed served value=0" );
		}
		return false;
	}

	static bool __fastcall RegistrationAdapterIsTimedOut( void *self, void * )
	{
		if ( self != &s_registrationAdapter )
			LogContextf( "adapter unexpected IsTimedOut self=%p expected=%p",
				self, &s_registrationAdapter );
		if ( !s_adapterTimedOutLogged )
		{
			s_adapterTimedOutLogged = true;
			LogContext( "adapter IsTimedOut served value=0" );
		}
		return false;
	}

	static bool __fastcall RegistrationAdapterHasPendingReliableData( void *self,
		void * )
	{
		if ( self != &s_registrationAdapter )
			LogContextf( "adapter unexpected HasPendingReliableData self=%p expected=%p",
				self, &s_registrationAdapter );
		if ( !s_adapterPendingReliableLogged )
		{
			s_adapterPendingReliableLogged = true;
			LogContext( "adapter HasPendingReliableData served value=0" );
		}
		return false;
	}

	static int __fastcall RegistrationAdapterGetSequenceNr( void *self, void *, int flow )
	{
		if ( self != &s_registrationAdapter )
			LogContextf( "adapter unexpected GetSequenceNr self=%p expected=%p flow=%d",
				self, &s_registrationAdapter, flow );
		if ( !s_adapterSequenceLogged )
		{
			s_adapterSequenceLogged = true;
			LogContextf( "adapter GetSequenceNr flow=%d served value=1 action=skip_legacy_oob",
				flow );
		}
		return 1;
	}

	static bool __fastcall RegistrationAdapterTransmit( void *self, void *,
		bool onlyReliable )
	{
		if ( self != &s_registrationAdapter )
			LogContextf( "adapter unexpected Transmit self=%p expected=%p reliable=%u",
				self, &s_registrationAdapter, onlyReliable ? 1U : 0U );
		const ASRD_GNS_Connection connection = ASRD_GNS_ClientConnection();
		const int flushResult = ASRD_GNS_Flush( connection );
		if ( !s_adapterTransmitLogged )
		{
			s_adapterTransmitLogged = true;
			LogContextf( "adapter Transmit reliable=%u action=gns_flush handle=%lu result=%d",
				onlyReliable ? 1U : 0U, (unsigned long)connection,
				flushResult );
		}
		return flushResult == ASRD_GNS_RESULT_OK;
	}

	static bool __fastcall RegistrationAdapterIsRemoteDisconnected( void *self,
		void * )
	{
		if ( self != &s_registrationAdapter )
		{
			LogContextf( "adapter unexpected IsRemoteDisconnected self=%p expected=%p",
				self, &s_registrationAdapter );
			return true;
		}
		if ( !s_adapterRemoteDisconnectedLogged )
		{
			s_adapterRemoteDisconnectedLogged = true;
			LogContext( "adapter IsRemoteDisconnected served value=0" );
		}
		return false;
	}

	static bool __fastcall RegistrationAdapterSendNetMsg( void *self, void *,
		void *message, bool forceReliable, bool voice )
	{
		if ( self != &s_registrationAdapter || !message )
		{
			LogContextf( "adapter unexpected SendNetMsg self=%p expected=%p message=%p reliable=%u voice=%u",
				self, &s_registrationAdapter, message,
				forceReliable ? 1U : 0U, voice ? 1U : 0U );
			return false;
		}

		INetMessage *netMessage = static_cast<INetMessage *>( message );
		const int type = netMessage->GetType();
		// GNS has already supplied the transport-connected fact.  Consume only
		// Source's redundant legacy CONNECTED handshake locally.  The other
		// semantic messages emitted by SetSignonState, including SetConVar, must
		// remain visible to the normal GNS message bridge.
		if ( s_localConnectedPrime &&
			s_localConnectedPrimeState == SIGNONSTATE_CONNECTED &&
			type == net_SignonState )
		{
			if ( s_localConnectedPrimeSignonSuppressed )
			{
				s_localConnectedPrimeSendFailed = true;
				LogContext( "adapter SendNetMsg type=net_SignonState state=CONNECTED action=duplicate_rejected" );
				return false;
			}
			s_localConnectedPrimeSignonSuppressed = true;
			LogContext( "adapter SendNetMsg type=net_SignonState state=CONNECTED action=local_consume_no_network" );
			return true;
		}
		if ( s_localConnectedPrime && type == net_SetConVar )
		{
			s_connectedPrimeSetConVarSeen = true;
			const char *contents = netMessage->ToString();
			LogContextf( "adapter SendNetMsg type=net_SetConVar action=forward_gns contents=%s",
				contents ? contents : "<null>" );
		}
		const ASRD_GNS_Connection connection = ASRD_GNS_ClientConnection();
		const bool effectiveReliable = netMessage->IsReliable() || forceReliable;
		const ASRD_GNS_BlockProvenance provenance = voice
			? ASRD_GNS_PROVENANCE_VOICE
			: ASRD_GNS_PROVENANCE_SEND_NETMSG;
		LogContextf( "adapter SendNetMsg callback=SendNetMsg handle=%lu type=%d IsReliable=%u forceReliable=%u voice=%u effectiveReliable=%u provenance=%u",
			(unsigned long)connection, type, netMessage->IsReliable() ? 1U : 0U,
			forceReliable ? 1U : 0U, voice ? 1U : 0U,
			effectiveReliable ? 1U : 0U, (unsigned)provenance );
		ASRD_GNS_MoveMetadata moveMetadata = {};
		const bool sent = ASRD_GNS_MessageBridgeSend( connection, *netMessage,
			effectiveReliable, voice, provenance, &moveMetadata );
		if ( s_localConnectedPrime && !sent )
			s_localConnectedPrimeSendFailed = true;
		if ( sent && type == clc_Move && moveMetadata.valid )
		{
			if ( moveMetadata.has_new_commands )
			{
				s_has_last_outgoing_command_number = true;
				s_last_outgoing_command_number =
					moveMetadata.highest_new_command_number;
			}
		}
		return sent;
	}

	static float __fastcall RegistrationAdapterGetAvgLatency( void *self, void *,
		int flow )
	{
		if ( self != &s_registrationAdapter )
		{
			LogContextf( "adapter unexpected GetAvgLatency self=%p expected=%p flow=%d",
				self, &s_registrationAdapter, flow );
			return 0.0f;
		}
		if ( !s_adapterAvgLatencyLogged )
		{
			s_adapterAvgLatencyLogged = true;
			LogContextf( "adapter GetAvgLatency flow=%d served value=0.0",
				flow );
		}
		return 0.0f;
	}

	static float __fastcall RegistrationAdapterGetAvgLoss( void *self, void *,
		int flow )
	{
		if ( self != &s_registrationAdapter )
		{
			LogContextf( "adapter unexpected GetAvgLoss self=%p expected=%p flow=%d",
				self, &s_registrationAdapter, flow );
			return 0.0f;
		}
		if ( !s_adapterAvgLossLogged )
		{
			s_adapterAvgLossLogged = true;
			LogContextf( "adapter GetAvgLoss flow=%d served value=0.0",
				flow );
		}
		return 0.0f;
	}

	static float __fastcall RegistrationAdapterGetAvgChoke( void *self, void *,
		int flow )
	{
		if ( self != &s_registrationAdapter )
		{
			LogContextf( "adapter unexpected GetAvgChoke self=%p expected=%p flow=%d",
				self, &s_registrationAdapter, flow );
			return 0.0f;
		}
		if ( !s_adapterAvgChokeLogged )
		{
			s_adapterAvgChokeLogged = true;
			LogContextf( "adapter GetAvgChoke flow=%d served value=0.0",
				flow );
		}
		return 0.0f;
	}

	static float __fastcall RegistrationAdapterGetAvgPackets( void *self, void *,
		int flow )
	{
		if ( self != &s_registrationAdapter )
		{
			LogContextf( "adapter unexpected GetAvgPackets self=%p expected=%p flow=%d",
				self, &s_registrationAdapter, flow );
			return 0.0f;
		}
		if ( !s_adapterAvgPacketsLogged )
		{
			s_adapterAvgPacketsLogged = true;
			LogContextf( "adapter GetAvgPackets flow=%d served value=0.0",
				flow );
		}
		return 0.0f;
	}

	static void __fastcall RegistrationAdapterGetRemoteFramerate( void *self,
		void *, float *frameTime, float *frameTimeStdDeviation )
	{
		if ( self != &s_registrationAdapter )
		{
			LogContextf( "adapter unexpected GetRemoteFramerate self=%p expected=%p",
				self, &s_registrationAdapter );
			return;
		}
		if ( frameTime )
			*frameTime = 0.0f;
		if ( frameTimeStdDeviation )
			*frameTimeStdDeviation = 0.0f;
		if ( !s_adapterRemoteFramerateInfoLogged )
		{
			s_adapterRemoteFramerateInfoLogged = true;
			LogContext( "adapter GetRemoteFramerate served value=0 action=gns_transport" );
		}
	}

	static void __fastcall RegistrationAdapterSetDataRate( void *self, void *,
		float rate )
	{
		if ( self != &s_registrationAdapter )
		{
			LogContextf( "adapter unexpected SetDataRate self=%p expected=%p rate=%f",
				self, &s_registrationAdapter, rate );
			return;
		}
		if ( !s_adapterDataRateLogged )
		{
			s_adapterDataRateLogged = true;
			LogContextf( "adapter SetDataRate rate=%f action=noop", rate );
		}
	}

	static void __fastcall RegistrationAdapterSetInterpolationAmount( void *self,
		void *, float amount )
	{
		if ( self != &s_registrationAdapter )
		{
			LogContextf( "adapter unexpected SetInterpolationAmount self=%p expected=%p amount=%f",
				self, &s_registrationAdapter, amount );
			return;
		}
		if ( !s_adapterInterpolationLogged )
		{
			s_adapterInterpolationLogged = true;
			LogContextf( "adapter SetInterpolationAmount amount=%f action=noop",
				amount );
		}
	}

	static void __fastcall RegistrationAdapterUpdateMessageStats( void *self,
		void *, int messageGroup, int bits )
	{
		if ( self != &s_registrationAdapter )
		{
			LogContextf( "adapter unexpected UpdateMessageStats self=%p expected=%p group=%d bits=%d",
				self, &s_registrationAdapter, messageGroup, bits );
			return;
		}
		if ( !s_adapterMessageStatsLogged )
		{
			s_adapterMessageStatsLogged = true;
			LogContextf( "adapter UpdateMessageStats group=%d bits=%d action=noop",
				messageGroup, bits );
		}
	}

	static void __fastcall RegistrationAdapterSetTimeout( void *self, void *,
		float seconds, bool forceExact )
	{
		if ( self != &s_registrationAdapter )
		{
			LogContextf( "adapter unexpected SetTimeout self=%p expected=%p seconds=%f force=%u",
				self, &s_registrationAdapter, seconds, forceExact ? 1U : 0U );
			return;
		}

		if ( !s_adapterTimeoutLogged )
		{
			s_adapterTimeoutLogged = true;
			LogContextf( "adapter SetTimeout seconds=%f force=%u action=noop",
				seconds, forceExact ? 1U : 0U );
		}
	}

	static void __fastcall RegistrationAdapterSetMaxBufferSize( void *self, void *,
		bool reliable, int bytes, bool voice )
	{
		if ( self != &s_registrationAdapter )
		{
			LogContextf( "adapter unexpected SetMaxBufferSize self=%p expected=%p reliable=%u bytes=%d voice=%u",
				self, &s_registrationAdapter, reliable ? 1U : 0U, bytes,
				voice ? 1U : 0U );
			return;
		}

		if ( !s_adapterMaxBufferLogged )
		{
			s_adapterMaxBufferLogged = true;
			LogContextf( "adapter SetMaxBufferSize reliable=%u bytes=%d voice=%u action=noop",
				reliable ? 1U : 0U, bytes, voice ? 1U : 0U );
		}
	}

	static bool __fastcall RegistrationAdapterRegisterMessage( void *self, void *,
		void *message )
	{
		if ( self != &s_registrationAdapter || !message )
		{
			LogContextf( "adapter unexpected call self=%p expected=%p message=%p",
				self, &s_registrationAdapter, message );
			return false;
		}

		const unsigned int beforeCapture = ASRD_GNS_MessageRegistryCaptureCount();
		ASRD_GNS_MessageRegistryCapture( message, self );
		const unsigned int afterCapture = ASRD_GNS_MessageRegistryCaptureCount();
		if ( afterCapture == beforeCapture )
		{
			LogContextf( "adapter registration failed message=%p registryCount=%u",
				message, ASRD_GNS_MessageRegistryCount() );
			return false;
		}
		return true;
	}
}

void ASRD_GNS_ClientRequestDisconnect( void )
{
	s_disconnectRequested = true;
	LogClient( "disconnect intent requested action=defer_until_engine_frame" );
}

void ASRD_GNS_ClientScheduleDisconnect( float delaySeconds )
{
	if ( delaySeconds < 0.0f )
		delaySeconds = 0.0f;
	s_disconnectAfterSeconds = delaySeconds;
	s_disconnectTimerArmed = false;
	s_disconnectDeadline = 0.0;
	LogClientf( "disconnect intent scheduled delay=%.3f action=wait_for_connected",
		delaySeconds );
}

bool ASRD_GNS_ClientBindSourceContext( void *clientState )
{
	if ( s_sourceContextBound )
	{
		if ( s_sourceContext != clientState )
		{
			LogContextf( "context bind rejected: already bound state=%p requested=%p",
				s_sourceContext, clientState );
			return false;
		}
		LogContextf( "context already bound state=%p registryCount=%u",
			s_sourceContext, ASRD_GNS_MessageRegistryCount() );
		return true;
	}
	if ( sizeof( void * ) != 4 || !clientState )
	{
		LogContextf( "context bind rejected: invalid client state=%p", clientState );
		return false;
	}

	HMODULE engineModule = GetModuleHandleA( "engine.dll" );
	if ( !IsExpectedEngineImage( engineModule ) )
	{
		LogContext( "context bind rejected: engine image mismatch" );
		return false;
	}

	BYTE *base = (BYTE *)engineModule;
	void **clientVtable = *(void ***)clientState;
	void *expectedVtable = (void *)( base + kClientStateVtableRva );
	LogContextf( "client resolve state=%p vtable=%p expected=%p",
		clientState, clientVtable, expectedVtable );
	if ( clientVtable != expectedVtable )
	{
		LogContext( "context bind rejected: client state vtable mismatch" );
		return false;
	}

	if ( !EnsureRegistrationAdapter() )
		return false;

	void **netChannelSlot = (void **)( (BYTE *)clientState +
		kClientStateNetChannelOffset );
	LogContextf( "client context netchannel slot=%p offset=0x%X current=%p action=deferred_dispatch_binding",
		netChannelSlot, (unsigned)kClientStateNetChannelOffset, *netChannelSlot );

	void *connectionStartAddress = clientVtable[ 1 ];
	void *expectedConnectionStart = (void *)( base + kConnectionStartRva );
	LogContextf( "ConnectionStart resolve state=%p target=%p expected=%p",
		clientState, connectionStartAddress, expectedConnectionStart );
	if ( connectionStartAddress != expectedConnectionStart ||
		!IsEngineAddress( engineModule, connectionStartAddress, 1 ) )
	{
		LogContext( "context bind rejected: ConnectionStart target mismatch" );
		return false;
	}

	const unsigned int captureBefore = ASRD_GNS_MessageRegistryCaptureCount();
	ClientConnectionStartFn connectionStart =
		(ClientConnectionStartFn)(uintptr_t)connectionStartAddress;
	connectionStart( clientState, &s_registrationAdapter );
	const unsigned int captureAfter = ASRD_GNS_MessageRegistryCaptureCount();
	LogContextf( "ConnectionStart complete state=%p captureBefore=%u captureAfter=%u",
		clientState, captureBefore, captureAfter );
	if ( captureAfter <= captureBefore )
	{
		LogContext( "context bind failed: ConnectionStart registered no messages" );
		return false;
	}
	const unsigned int registrationCount = ASRD_GNS_MessageRegistryCount();

	s_sourceContext = clientState;
	s_sourceContextBound = true;
	ResetConnectionGenerationState();
	ResetCompatibilityState();
	LogContextf( "context bind success state=%p registrationCount=%u generation=reset",
		clientState, registrationCount );
	return true;
}

bool ASRD_GNS_ClientBindSourceChannelForDispatch( void )
{
	if ( !s_sourceContextBound || !s_sourceContext ||
		s_sourceChannelPersistent || s_dispatchNetChannelBound )
	{
		if ( s_sourceChannelPersistent )
			return true;
		return s_dispatchNetChannelBound;
	}

	void **netChannelSlot = (void **)( (BYTE *)s_sourceContext +
		kClientStateNetChannelOffset );
	s_dispatchPreviousNetChannel = *netChannelSlot;
	if ( s_dispatchPreviousNetChannel &&
		s_dispatchPreviousNetChannel != &s_registrationAdapter )
	{
		LogContextf( "dispatch binding replacing existing engine pointer=%p slot=%p",
			s_dispatchPreviousNetChannel, netChannelSlot );
	}
	*netChannelSlot = &s_registrationAdapter;
	s_dispatchNetChannelBound = true;
	LogContextf( "dispatch netchannel bound slot=%p adapter=%p action=temporary",
		netChannelSlot, &s_registrationAdapter );
	return true;
}

void ASRD_GNS_ClientUnbindSourceChannelForDispatch( void )
{
	if ( s_sourceChannelPersistent || !s_dispatchNetChannelBound || !s_sourceContext )
		return;

	void **netChannelSlot = (void **)( (BYTE *)s_sourceContext +
		kClientStateNetChannelOffset );
	if ( *netChannelSlot == &s_registrationAdapter )
		*netChannelSlot = s_dispatchPreviousNetChannel;
	LogContextf( "dispatch netchannel unbound slot=%p restored=%p",
		netChannelSlot, s_dispatchPreviousNetChannel );
	s_dispatchPreviousNetChannel = NULL;
	s_dispatchNetChannelBound = false;
}

void ASRD_GNS_ClientPromoteSourceChannelForLifecycle( void )
{
	if ( s_sourceChannelPersistent || !s_sourceContext )
		return;
	if ( !s_dispatchNetChannelBound &&
		!ASRD_GNS_ClientBindSourceChannelForDispatch() )
		return;

	s_sourceChannelPersistent = true;
	LogContextf( "dispatch netchannel promoted slot=%p adapter=%p action=lifecycle_owner",
		(void *)( (BYTE *)s_sourceContext + kClientStateNetChannelOffset ),
		&s_registrationAdapter );
	// Keep the adapter installed for the connected Source lifecycle. GNS now
	// owns this lifecycle; no legacy channel is restored.
	s_dispatchPreviousNetChannel = NULL;
	s_dispatchNetChannelBound = false;
}

bool ASRD_GNS_ClientPacketStart( uint32_t serverUpdateSeq,
	int clientCommandAck )
{
	if ( s_compatibilityFatal || !s_connectedPrimed || s_packetStarted ||
		!s_sourceContextBound || !s_sourceContext || serverUpdateSeq == 0 ||
		serverUpdateSeq > 0x7FFFFFFFU )
		return false;

	HMODULE engineModule = GetModuleHandleA( "engine.dll" );
	if ( !IsExpectedEngineImage( engineModule ) )
		return false;

	void **clientVtable = *(void ***)s_sourceContext;
	if ( !clientVtable || !IsEngineAddress( engineModule, clientVtable,
		sizeof( void * ) * ( kClientPacketEndSlot + 1 ) ) )
		return false;
	void *packetStartAddress = clientVtable[ kClientPacketStartSlot ];
	void *expectedPacketStart = (void *)( (BYTE *)engineModule + kClientPacketStartRva );
	if ( packetStartAddress != expectedPacketStart ||
		!IsEngineAddress( engineModule, packetStartAddress, 1 ) )
	{
		LogContextf( "PacketStart rejected slot=%u target=%p expected=%p",
			kClientPacketStartSlot, packetStartAddress, expectedPacketStart );
		return false;
	}

	ClientPacketStartFn packetStart =
		(ClientPacketStartFn)(uintptr_t)packetStartAddress;
	int deliveredClientCommandAck = clientCommandAck;
	if ( s_hasLastDeliveredClientCommandAck &&
		deliveredClientCommandAck < s_lastDeliveredClientCommandAck )
	{
		// Reliable and unreliable GNS lanes are ordered independently. An older
		// block can therefore arrive after a newer update and must not move the
		// Source packet ACK baseline backwards. PacketEnd converts this absolute
		// value into an acknowledged-command delta; a regression would accumulate
		// a negative delta in prediction state.
		LogContextf( "PacketStart command ack clamped update=%u received=%d delivered=%d reason=cross_lane_regression",
			(unsigned)serverUpdateSeq, clientCommandAck,
			s_lastDeliveredClientCommandAck );
		deliveredClientCommandAck = s_lastDeliveredClientCommandAck;
	}
	packetStart( s_sourceContext, (int)serverUpdateSeq,
		deliveredClientCommandAck );
	s_currentServerUpdateSeq = serverUpdateSeq;
	s_currentClientCommandAck = deliveredClientCommandAck;
	s_packetStarted = true;
	return true;
}

bool ASRD_GNS_ClientPacketEnd( void )
{
	if ( !s_connectedPrimed || !s_packetStarted ||
		!s_sourceContextBound || !s_sourceContext )
		return false;

	HMODULE engineModule = GetModuleHandleA( "engine.dll" );
	if ( !IsExpectedEngineImage( engineModule ) )
		return false;
	void **clientVtable = *(void ***)s_sourceContext;
	if ( !clientVtable || !IsEngineAddress( engineModule, clientVtable,
		sizeof( void * ) * ( kClientPacketEndSlot + 1 ) ) )
		return false;
	void *packetEndAddress = clientVtable[ kClientPacketEndSlot ];
	void *expectedPacketEnd = (void *)( (BYTE *)engineModule + kClientPacketEndRva );
	if ( packetEndAddress != expectedPacketEnd ||
		!IsEngineAddress( engineModule, packetEndAddress, 1 ) )
	{
		LogContextf( "PacketEnd rejected slot=%u target=%p expected=%p",
			kClientPacketEndSlot, packetEndAddress, expectedPacketEnd );
		return false;
	}

	ClientPacketEndFn packetEnd =
		(ClientPacketEndFn)(uintptr_t)packetEndAddress;
	packetEnd( s_sourceContext );
	s_lastDeliveredClientCommandAck = s_currentClientCommandAck;
	s_hasLastDeliveredClientCommandAck = true;
	s_packetStarted = false;
	s_currentServerUpdateSeq = 0;
	s_currentClientCommandAck = 0;
	return true;
}

void ASRD_GNS_ClientAbortCompatibilitySession( const char *reason )
{
	if ( !s_compatibilityFatal )
		LogContextf( "compatibility abort reason=%s action=disconnect_reset_no_next_update",
			reason ? reason : "unspecified" );
	s_compatibilityFatal = true;
	s_packetStarted = false;
	s_currentServerUpdateSeq = 0;
	s_currentClientCommandAck = 0;
	const ASRD_GNS_Connection connection = ASRD_GNS_ClientConnection();
	ASRD_GNS_MessageBridgeDiscardQueued( connection );
	// PacketEnd is intentionally not called on this path. The connection is
	// closed and the Source adapter detached, so no later envelope can reuse
	// the PacketStart state that was just abandoned.
	ASRD_GNS_ClientShutdown();
}

bool ASRD_GNS_ClientConnectIntent( void *clientState, const char *endpoint,
	const char *secondary )
{
	(void)secondary;
	char ipv4[ 64 ];
	uint16_t port = 0;
	if ( !ParseEndpoint( endpoint, ipv4, sizeof( ipv4 ), &port ) )
	{
		const bool existingGeneration = HasExistingClientGeneration();
		if ( !existingGeneration )
			SetState( ASRD_GNS_CLIENT_FAILED, ASRD_GNS_CONNECTION_INVALID, 0, true );
		LogClientf( "connect intent parse=failed legacy=blocked endpoint_length=%u action=%s",
			endpoint ? (unsigned)strlen( endpoint ) : 0U,
			existingGeneration ? "preserve_generation" : "failed" );
		return false;
	}

	LogClientf( "connect intent parse=ok legacy=blocked target=non_loopback_ipv4 port=%u",
		(unsigned)port );
	netadr_t candidateRemoteAddress;
	char normalizedEndpoint[ kEndpointCapacity + 8 ];
	Q_snprintf( normalizedEndpoint, sizeof( normalizedEndpoint ), "%s:%u", ipv4,
		(unsigned)port );
	candidateRemoteAddress.SetFromString( normalizedEndpoint, false );
	if ( !candidateRemoteAddress.IsValid() )
	{
		const bool existingGeneration = HasExistingClientGeneration();
		if ( !existingGeneration )
			SetState( ASRD_GNS_CLIENT_FAILED, ASRD_GNS_CONNECTION_INVALID, 0, true );
		LogClientf( "connect intent takeover=failed reason=remote_address_adapter legacy=blocked action=%s",
			existingGeneration ? "preserve_generation" : "failed" );
		return false;
	}

	if ( !LockClientState() )
	{
		LogClient( "connect intent blocked: shared state unavailable" );
		return false;
	}
	InterlockedIncrement( &s_state->intentCount );
	UnlockClientState();

	bool existingWrapperInitialized = false;
	const ASRD_GNS_Connection existingConnection =
		GetConnection( &existingWrapperInitialized );
	const bool existingGeneration = HasExistingClientGeneration() ||
		existingWrapperInitialized || existingConnection != ASRD_GNS_CONNECTION_INVALID;
	const bool sameEndpoint = existingConnection != ASRD_GNS_CONNECTION_INVALID &&
		existingWrapperInitialized && candidateRemoteAddress == s_registrationRemoteAddress;
	if ( sameEndpoint && s_sourceContextBound && s_sourceContext == clientState &&
		ASRD_GNS_ClientState() == ASRD_GNS_CLIENT_CONNECTED )
	{
		int signonState = SIGNONSTATE_NONE;
		(void)ReadSourceSignonState( &signonState );
		if ( s_sourceLifecyclePhase == SOURCE_LIFECYCLE_CHANGELEVEL ||
			signonState == SIGNONSTATE_CHANGELEVEL )
		{
			s_sourceLifecyclePhase = SOURCE_LIFECYCLE_CHANGELEVEL;
			if ( !SourceAdapterSlotMatches() && !RestoreChangelevelSourceAdapter() )
			{
				LogClient( "connect intent reuse failed reason=source_adapter_restore" );
				return false;
			}
			LogClientf( "connect intent reuse=connected handle=%lu action=preserve_changelevel_transport",
				(unsigned long)existingConnection );
			return true;
		}
	}

	// A connect intent starts a new Source/GNS generation.  Retire any stale
	// adapter or wrapper state before resetting the generation-local primes.
	if ( existingGeneration )
	{
		LogClient( "connect intent found previous generation action=shutdown_before_rebind" );
		ASRD_GNS_ClientShutdown();
	}
	s_registrationRemoteAddress = candidateRemoteAddress;
	ResetConnectionGenerationState();
	if ( !ASRD_GNS_ClientBindSourceContext( clientState ) )
	{
		AbortClientGeneration( "source_context" );
		LogClient( "connect intent takeover=failed reason=source_context legacy=blocked" );
		return false;
	}
	if ( !PrimeSourceChallengeState() )
	{
		AbortClientGeneration( "challenge_prime" );
		LogClient( "connect intent takeover=failed reason=challenge_prime legacy=blocked" );
		return false;
	}
	if ( !ASRD_GNS_Initialize( 0 ) )
	{
		AbortClientGeneration( "wrapper_initialize" );
		LogClient( "connect intent takeover=failed reason=wrapper_initialize legacy=blocked" );
		return false;
	}

	if ( LockClientState() )
	{
		s_state->wrapperInitialized = 1;
		memset( s_state->endpoint, 0, sizeof( s_state->endpoint ) );
		strncpy( s_state->endpoint, endpoint, sizeof( s_state->endpoint ) - 1 );
		UnlockClientState();
	}

	const ASRD_GNS_Connection connection = ASRD_GNS_Connect( ipv4, port );
	if ( connection == ASRD_GNS_CONNECTION_INVALID )
	{
		AbortClientGeneration( "wrapper_connect" );
		LogClient( "connect intent takeover=failed reason=wrapper_connect legacy=blocked" );
		return false;
	}

	if ( LockClientState() )
	{
		s_state->connection = connection;
		s_state->state = ASRD_GNS_CLIENT_PENDING;
		InterlockedIncrement( &s_state->takeoverCount );
		UnlockClientState();
	}
	LogClientf( "connect intent takeover=pending legacy=blocked handle=%lu",
		(unsigned long)connection );
	return true;
}

void ASRD_GNS_ClientFrame( void )
{
	if ( !OpenClientState() )
		return;
	if ( !RefreshSourceLifecyclePhase() )
		return;

	// This is the engine-thread boundary.  The wrapper's callback only writes
	// its event queue; no Source object is touched by the callback itself.
	ASRD_GNS_RunFrame();

	ASRD_GNS_ConnectionEvent event = {};
	while ( ASRD_GNS_PollConnectionEvent( &event ) )
	{
		if ( LockClientState() )
		{
			InterlockedIncrement( &s_state->eventCount );
			UnlockClientState();
		}
		const ASRD_GNS_Connection mappedEventConnection =
			ASRD_GNS_ClientConnection();
		if ( event.connection != ASRD_GNS_CONNECTION_INVALID &&
			mappedEventConnection != ASRD_GNS_CONNECTION_INVALID &&
			event.connection != mappedEventConnection )
		{
			LogClientf( "connection event ignored state=%d reason=%d handle=%lu mapped_handle=%lu action=stale_generation",
				event.state, event.reason, (unsigned long)event.connection,
				(unsigned long)mappedEventConnection );
			continue;
		}

		switch ( event.state )
		{
		case ASRD_GNS_CONNECTION_EVENT_CONNECTING:
			SetState( ASRD_GNS_CLIENT_PENDING, event.connection, event.reason, false );
			break;
		case ASRD_GNS_CONNECTION_EVENT_CONNECTED:
		{
			const ASRD_GNS_Connection mappedConnection =
				ASRD_GNS_ClientConnection();
			SetState( ASRD_GNS_CLIENT_CONNECTED, event.connection, event.reason, false );
			const bool sourceReady = s_sourceContextBound && s_sourceContext &&
				s_registrationAdapterReady &&
				s_registrationAdapter.vtable == s_registrationVtable;
			if ( event.connection == ASRD_GNS_CONNECTION_INVALID ||
				mappedConnection != event.connection || !sourceReady ||
				ASRD_GNS_ConfigureLanes( event.connection ) != ASRD_GNS_RESULT_OK ||
				!PrimeSourceConnectedState() )
			{
				AbortClientGeneration( "connected_prime" );
				return;
			}
			break;
		}
		case ASRD_GNS_CONNECTION_EVENT_FAILED:
		{
			const ASRD_GNS_ClientConnectionState finalState = ASRD_GNS_CLIENT_FAILED;
			const int reason = event.reason;
			ASRD_GNS_MessageBridgeDiscardQueued( event.connection );
			ASRD_GNS_ClientShutdown();
			SetState( finalState, ASRD_GNS_CONNECTION_INVALID, reason, true );
			break;
		}
		case ASRD_GNS_CONNECTION_EVENT_CLOSED:
		{
			const ASRD_GNS_ClientConnectionState finalState = ASRD_GNS_CLIENT_CLOSED;
			const int reason = event.reason;
			ASRD_GNS_MessageBridgeDiscardQueued( event.connection );
			ASRD_GNS_ClientShutdown();
			SetState( finalState, ASRD_GNS_CONNECTION_INVALID, reason, true );
			break;
		}
		default:
			LogClientf( "connection event ignored state=%d reason=%d", event.state, event.reason );
			break;
		}
	}

	const ASRD_GNS_ClientConnectionState state = ASRD_GNS_ClientState();
	if ( state == ASRD_GNS_CLIENT_CONNECTED &&
		s_disconnectAfterSeconds >= 0.0f && !s_disconnectTimerArmed )
	{
		s_disconnectDeadline = Plat_FloatTime() + s_disconnectAfterSeconds;
		s_disconnectTimerArmed = true;
		LogClientf( "disconnect timer armed delay=%.3f action=engine_thread",
			s_disconnectAfterSeconds );
	}
	if ( s_disconnectTimerArmed && Plat_FloatTime() >= s_disconnectDeadline )
	{
		s_disconnectRequested = true;
		s_disconnectTimerArmed = false;
		s_disconnectAfterSeconds = -1.0f;
		LogClient( "disconnect timer elapsed action=request" );
	}

	if ( s_disconnectRequested )
	{
		const ASRD_GNS_Connection connection = GetConnection( NULL );
		if ( connection == ASRD_GNS_CONNECTION_INVALID )
		{
			LogClient( "disconnect intent ignored reason=no_mapped_connection" );
			s_disconnectRequested = false;
		}
		else
		{
			LogClientf( "disconnect intent close handle=%lu action=gns_then_source_upper_teardown",
				(unsigned long)connection );
			ASRD_GNS_ClientShutdown();
			s_disconnectRequested = false;
#if defined( CLIENT_DLL )
			if ( engine )
			{
				engine->ClientCmd( "disconnect\n" );
				LogClient( "disconnect intent queued Source command=disconnect" );
			}
#endif
		}
	}
}

void ASRD_GNS_ClientShutdown( void )
{
	if ( s_clientShutdownInProgress )
	{
		LogClient( "shutdown reentrant action=ignore" );
		return;
	}
	s_clientShutdownInProgress = true;
	LogClientf( "shutdown begin source_context=%p persistent=%u handle=%lu",
		s_sourceContext, s_sourceChannelPersistent ? 1U : 0U,
		(unsigned long)ASRD_GNS_ClientConnection() );

	if ( s_sourceContext && s_sourceChannelPersistent )
	{
		void **netChannelSlot = (void **)( (BYTE *)s_sourceContext +
			kClientStateNetChannelOffset );
		if ( *netChannelSlot == &s_registrationAdapter )
			*netChannelSlot = NULL;
		s_sourceChannelPersistent = false;
	}
	ASRD_GNS_ClientUnbindSourceChannelForDispatch();
	bool wrapperInitialized = false;
	const ASRD_GNS_Connection connection = GetConnection( &wrapperInitialized );
	ASRD_GNS_MessageBridgeDiscardQueued( connection );
	if ( connection != ASRD_GNS_CONNECTION_INVALID )
	{
		ASRD_GNS_Close( connection );
		SetState( ASRD_GNS_CLIENT_CLOSED, ASRD_GNS_CONNECTION_INVALID, 0, true );
	}
	if ( wrapperInitialized )
	{
		ASRD_GNS_Shutdown();
		if ( LockClientState() )
		{
			s_state->wrapperInitialized = 0;
			UnlockClientState();
		}
	}
	s_sourceContextBound = false;
	s_sourceContext = NULL;
	s_dispatchPreviousNetChannel = NULL;
	s_dispatchNetChannelBound = false;
	s_sourceLifecyclePhase = SOURCE_LIFECYCLE_RUNNING;
	s_sourceReattachPending = false;
	s_disconnectRequested = false;
	s_disconnectAfterSeconds = -1.0f;
	s_disconnectDeadline = 0.0;
	s_disconnectTimerArmed = false;
	ResetCompatibilityState();
	ResetConnectionGenerationState();
	s_clientShutdownInProgress = false;
	LogClient( "shutdown complete action=source_adapter_detached" );
}

bool ASRD_GNS_ClientBridgeReady( void )
{
	return s_sourceContextBound && s_sourceContext && s_connectedPrimed &&
		!s_compatibilityFatal && SourceAdapterSlotMatches() &&
		ASRD_GNS_ClientState() == ASRD_GNS_CLIENT_CONNECTED;
}

ASRD_GNS_ClientConnectionState ASRD_GNS_ClientState( void )
{
	if ( !LockClientState() )
		return ASRD_GNS_CLIENT_DISCONNECTED;
	const ASRD_GNS_ClientConnectionState state =
		(ASRD_GNS_ClientConnectionState)s_state->state;
	UnlockClientState();
	return state;
}

ASRD_GNS_Connection ASRD_GNS_ClientConnection( void )
{
	return GetConnection( NULL );
}

#else

bool ASRD_GNS_ClientBindSourceContext( void * )
{
	return false;
}

bool ASRD_GNS_ClientBindSourceChannelForDispatch( void )
{
	return false;
}

void ASRD_GNS_ClientUnbindSourceChannelForDispatch( void )
{
}

void ASRD_GNS_ClientPromoteSourceChannelForLifecycle( void )
{
}

bool ASRD_GNS_ClientPacketStart( uint32_t, int )
{
	return false;
}

bool ASRD_GNS_ClientPacketEnd( void )
{
	return false;
}

void ASRD_GNS_ClientAbortCompatibilitySession( const char * )
{
}

bool ASRD_GNS_ClientConnectIntent( void *, const char *, const char * )
{
	return false;
}

void ASRD_GNS_ClientRequestDisconnect( void )
{
}

void ASRD_GNS_ClientScheduleDisconnect( float )
{
}

void ASRD_GNS_ClientFrame( void )
{
}

void ASRD_GNS_ClientShutdown( void )
{
}

bool ASRD_GNS_ClientBridgeReady( void )
{
	return false;
}

ASRD_GNS_ClientConnectionState ASRD_GNS_ClientState( void )
{
	return ASRD_GNS_CLIENT_DISCONNECTED;
}

ASRD_GNS_Connection ASRD_GNS_ClientConnection( void )
{
	return ASRD_GNS_CONNECTION_INVALID;
}

#endif
