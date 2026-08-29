#pragma once

#include "asrd_gns_wrapper.h"

enum ASRD_GNS_ClientConnectionState
{
	ASRD_GNS_CLIENT_DISCONNECTED = 0,
	ASRD_GNS_CLIENT_PENDING = 1,
	ASRD_GNS_CLIENT_CONNECTED = 2,
	ASRD_GNS_CLIENT_FAILED = 3,
	ASRD_GNS_CLIENT_CLOSED = 4,
};

// Handles a client connection request at the engine-owned boundary.  A false
// return leaves the request blocked; the caller decides whether to provide a
// replacement.
bool ASRD_GNS_ClientConnectIntent( void *clientState, const char *endpoint,
	const char *secondary );

// Requests a disconnect from the engine thread.  The request waits for the GNS
// connection to be established, then closes it before the normal disconnect
// command is queued.
void ASRD_GNS_ClientRequestDisconnect( void );
void ASRD_GNS_ClientScheduleDisconnect( float delaySeconds );

// Associates engine-owned client state with the existing client registration
// path.  This does not create a transport channel.
bool ASRD_GNS_ClientBindSourceContext( void *clientState );

// Makes the registration adapter available for client message dispatch.  Once
// the GNS connection is locally ready, it remains installed until normal
// shutdown or detachment.
bool ASRD_GNS_ClientBindSourceChannelForDispatch( void );
void ASRD_GNS_ClientUnbindSourceChannelForDispatch( void );
void ASRD_GNS_ClientPromoteSourceChannelForLifecycle( void );

// Brackets one logical server update for client message dispatch.  A failed
// dispatch aborts the current compatibility session before the next update.
bool ASRD_GNS_ClientPacketStart( uint32_t serverUpdateSeq,
	int clientCommandAck );
bool ASRD_GNS_ClientPacketEnd( void );
void ASRD_GNS_ClientAbortCompatibilitySession( const char *reason );

// Runs on the engine thread.  Wrapper callbacks enqueue status events; this
// function drains them and updates client lifecycle state.
void ASRD_GNS_ClientFrame( void );
void ASRD_GNS_ClientShutdown( void );
bool ASRD_GNS_ClientBridgeReady( void );

ASRD_GNS_ClientConnectionState ASRD_GNS_ClientState( void );
ASRD_GNS_Connection ASRD_GNS_ClientConnection( void );
