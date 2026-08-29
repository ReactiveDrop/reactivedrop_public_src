#pragma once

#include "asrd_gns_wrapper.h"

#include <stdint.h>

class INetMessage;
struct ASRD_GNS_MoveMetadata;

// The envelope is a game-side wire format. The wrapper transports one
// complete Engine block per GNS message; it does not inspect Source payloads.
enum
{
	ASRD_GNS_MESSAGE_ENVELOPE_MAGIC = 0x4153474D, // "ASGM"
	ASRD_GNS_MESSAGE_ENVELOPE_VERSION = 4,
	ASRD_GNS_MESSAGE_RAW_STREAM_TYPE = 0xFF,
	ASRD_GNS_MESSAGE_ENVELOPE_HEADER_BYTES = 38,
	ASRD_GNS_MESSAGE_MAX_ENVELOPE_BYTES = 512 * 1024,
	ASRD_GNS_MESSAGE_MAX_PAYLOAD_BYTES = ASRD_GNS_MESSAGE_MAX_ENVELOPE_BYTES -
		ASRD_GNS_MESSAGE_ENVELOPE_HEADER_BYTES,
};

enum ASRD_GNS_BlockReliability
{
	ASRD_GNS_BLOCK_RELIABILITY_UNKNOWN = 0,
	ASRD_GNS_BLOCK_RELIABILITY_RELIABLE = 1,
	ASRD_GNS_BLOCK_RELIABILITY_UNRELIABLE = 2,
};

enum ASRD_GNS_BlockProvenance
{
	ASRD_GNS_PROVENANCE_UNKNOWN = 0,
	ASRD_GNS_PROVENANCE_SEND_NETMSG = 1,
	ASRD_GNS_PROVENANCE_SEND_DATA = 2,
	ASRD_GNS_PROVENANCE_SEND_DATAGRAM = 3,
	ASRD_GNS_PROVENANCE_VOICE = 4,
	ASRD_GNS_PROVENANCE_TEMP_ENTITIES = 5,
	ASRD_GNS_PROVENANCE_ENTITY_MESSAGE = 6,
	ASRD_GNS_PROVENANCE_SOUND = 7,
	ASRD_GNS_PROVENANCE_FX_USER_MESSAGE = 8,
	ASRD_GNS_PROVENANCE_SNAPSHOT = 9,
};

enum
{
	ASRD_GNS_ENVELOPE_FLAG_RELIABLE = 1 << 0,
	ASRD_GNS_ENVELOPE_FLAG_REALTIME = 1 << 1,
	ASRD_GNS_ENVELOPE_FLAG_VOICE = 1 << 2,
	ASRD_GNS_ENVELOPE_FLAG_RAW = 1 << 3,
	ASRD_GNS_ENVELOPE_FLAG_PACKET_CONTEXT = 1 << 4,
};

enum
{
	ASRD_GNS_DIRECTION_CLIENT_TO_SERVER = 0,
	ASRD_GNS_DIRECTION_SERVER_TO_CLIENT = 1,
};

// This is the fixed header of EnvelopeVNext. Payload bytes follow immediately
// after the 38-byte header and are bounded by the total GNS message limit.
// unreliableSeq is zero for the reliable lane and is a session-local sequence
// shared by the two unreliable lanes.
#pragma pack(push, 1)
struct ASRD_GNS_EnvelopeVNext
{
	uint32_t magic;
	uint16_t version;
	uint16_t headerBytes;
	uint64_t unreliableSeq;
	uint32_t bitLength;
	uint32_t payloadBytes;
	uint32_t serverUpdateSeq;
	int32_t clientCommandAck;
	uint8_t direction;
	uint8_t type;
	uint8_t lane;
	uint8_t provenance;
	uint16_t flags;
};
#pragma pack(pop)

// Received wrapper bytes are copied into a local queue first; queued blocks
// are dispatched only from the owning game frame.
void ASRD_GNS_MessageBridgeFrame( bool serverRole );

// Discards queued blocks and pending server blocks for a reset connection.
void ASRD_GNS_MessageBridgeDiscardQueued( ASRD_GNS_Connection connection );

// SendNetMsg supplies effective reliability from message.IsReliable() and
// forceReliable. voice is provenance only and never changes reliability.
bool ASRD_GNS_MessageBridgeSend( ASRD_GNS_Connection connection,
	INetMessage &message, bool effectiveReliable, bool voice,
	ASRD_GNS_BlockProvenance provenance,
	ASRD_GNS_MoveMetadata *moveMetadata );

// Compatibility overload for callers that do not have provenance metadata.
bool ASRD_GNS_MessageBridgeSend( ASRD_GNS_Connection connection,
	INetMessage &message, bool effectiveReliable,
	ASRD_GNS_MoveMetadata *moveMetadata );

// Server callbacks capture complete Engine blocks in source order. No callback
// payload is split or merged with another callback payload.
bool ASRD_GNS_MessageBridgeAppendServer( ASRD_GNS_Connection connection,
	INetMessage &message, bool effectiveReliable, bool voice,
	ASRD_GNS_BlockProvenance provenance );
bool ASRD_GNS_MessageBridgeAppendServer( ASRD_GNS_Connection connection,
	INetMessage &message, bool effectiveReliable );

// Known-reliability raw callback path (SendData).
bool ASRD_GNS_MessageBridgeAppendServerRaw( ASRD_GNS_Connection connection,
	const void *payload, unsigned int bitLength, unsigned int payloadBytes,
	bool effectiveReliable );

// Explicit reliability/provenance path used by SendDatagram and other raw
// callback boundaries; the confirmed snapshot boundary supplies UNRELIABLE
// and SNAPSHOT.
bool ASRD_GNS_MessageBridgeAppendServerRaw( ASRD_GNS_Connection connection,
	const void *payload, unsigned int bitLength, unsigned int payloadBytes,
	ASRD_GNS_BlockReliability reliability,
	ASRD_GNS_BlockProvenance provenance );

bool ASRD_GNS_MessageBridgeSealAndFlushServerUpdate(
	ASRD_GNS_Connection connection );
unsigned int ASRD_GNS_MessageBridgeServerPendingBits(
	ASRD_GNS_Connection connection );
bool ASRD_GNS_MessageBridgeServerHasPending(
	ASRD_GNS_Connection connection );

// Sends an already serialized client-to-server Engine block. The receiving
// side is responsible for interpreting the complete stream.
bool ASRD_GNS_MessageBridgeSendRaw( ASRD_GNS_Connection connection,
	const void *payload, unsigned int bitLength, unsigned int payloadBytes,
	bool effectiveReliable );

// Sets the receive-side unreliable reorder window ratio. Zero selects the
// automatic simulator-driven mode; non-zero values are accepted in 0.5..4.0.
// The resulting logical capacity is applied on the next normal bridge frame.
bool ASRD_GNS_MessageBridgeSetUnreliableWindowRatio( float ratio );

// Process-local bridge diagnostics.
unsigned int ASRD_GNS_MessageBridgeQueued( void );
unsigned int ASRD_GNS_MessageBridgeReceived( void );
unsigned int ASRD_GNS_MessageBridgeDelivered( void );
