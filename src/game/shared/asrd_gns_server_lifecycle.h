#pragma once

#include "asrd_gns_wrapper.h"
#include "asrd_gns_move_compat.h"

enum ASRD_GNS_ServerConnectionState
{
	ASRD_GNS_SERVER_DISCONNECTED = 0,
	ASRD_GNS_SERVER_PENDING = 1,
	ASRD_GNS_SERVER_CONNECTED = 2,
	ASRD_GNS_SERVER_FAILED = 3,
	ASRD_GNS_SERVER_CLOSED = 4,
};

// The context representation is private to the server lifecycle. The
// lifecycle owns context storage; callers may pass an opaque context pointer
// to the compatibility APIs without taking ownership of it.
struct ASRD_GNS_ServerConnectionContext;

// Initializes server lifecycle state for the requested listening port. This
// module owns connection bookkeeping and lifecycle decisions.
bool ASRD_GNS_ServerInit( uint16_t port );

// Advances pending server lifecycle work for the current frame.
void ASRD_GNS_ServerFrame( void );

// Runs only the GNS control plane.  This path is safe to call from the
// dedicated-server hibernation wake hook and deliberately does not receive or
// dispatch gameplay messages.
void ASRD_GNS_ServerControlFrame( void );
void ASRD_GNS_ServerWakeControlFrame( void );

// The wake hook uses these observations to gate its control-frame call.  The
// hibernation state is owned by the Source server DLL.
bool ASRD_GNS_ServerIsInitialized( void );
bool ASRD_GNS_ServerIsHibernating( void );

// Completes the Source signon phase after a wake-safe ClientConnect has
// returned.  It is called from the normal GameFrame path before the message
// bridge and succeeds at most once per mapped connection.
void ASRD_GNS_ServerFinalizeSourceSignon( void );

// Marks the mapped GNS-backed Source slot as a real player only after the
// engine has reported ClientActive.  The sourceEdict pointer is observed only
// for association with the current GNS session; ownership remains with Source.
void ASRD_GNS_ServerRealPlayerEntered( void *sourceEdict );

// The server bridge runs for an unmapped server or after Source signon has
// finalized; an unfinalized mapping remains isolated from MessageBridgeFrame.
bool ASRD_GNS_ServerMessageBridgeReady( void );

// Releases state owned by the server lifecycle during final shutdown.
void ASRD_GNS_ServerShutdown( void );

// Observation-only hook for client-originated messages. This must not change
// command or gameplay dispatch.
void ASRD_GNS_ServerTraceClientMessage( int type, unsigned int bitLength );

// Observation-only hook for lifecycle events. Ordinary transitions must not
// close or erase the GNS connection; finalShutdown marks final teardown.
void ASRD_GNS_ServerTraceLifecycle( const char *phase, const char *mapName,
	bool finalShutdown );

unsigned int ASRD_GNS_ServerConnectionCount( void );
ASRD_GNS_ServerConnectionState ASRD_GNS_ServerState( void );
ASRD_GNS_Connection ASRD_GNS_ServerConnection( void );

// Returns the server-owned default context, when available.
ASRD_GNS_ServerConnectionContext *ASRD_GNS_ServerContext( void );

// Returns the server-owned context associated with a connection, when
// available. Returning the pointer does not transfer ownership.
ASRD_GNS_ServerConnectionContext *ASRD_GNS_ServerContextForConnection(
	ASRD_GNS_Connection connection );

// Compatibility operations use server-owned contexts. These APIs do not
// transfer context ownership to their callers.
ASRD_GNS_MoveRangeClass ASRD_GNS_ServerClassifyMove(
	ASRD_GNS_ServerConnectionContext *context,
	const ASRD_GNS_MoveMetadata &move );
int ASRD_GNS_ServerComputeMoveDrop(
	ASRD_GNS_ServerConnectionContext *context,
	const ASRD_GNS_MoveMetadata &move );
void ASRD_GNS_ServerBeginMoveProcess(
	ASRD_GNS_ServerConnectionContext *context, int droppedCommands );
void ASRD_GNS_ServerFinishMoveProcess(
	ASRD_GNS_ServerConnectionContext *context,
	const ASRD_GNS_MoveMetadata &move, bool processSucceeded );
int ASRD_GNS_ServerGetCurrentMoveDrop(
	const ASRD_GNS_ServerConnectionContext *context );
int ASRD_GNS_ServerGetLastAcceptedCommandNumber(
	const ASRD_GNS_ServerConnectionContext *context, bool *hasValue );
bool ASRD_GNS_ServerCaptureOutgoingUpdateMetadata(
	ASRD_GNS_ServerConnectionContext *context, uint32_t *serverUpdateSeq,
	int *clientCommandAck );
void ASRD_GNS_ServerResetCompatibility(
	ASRD_GNS_ServerConnectionContext *context );
