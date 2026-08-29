#pragma once

#include <stdint.h>

#if defined( _WIN32 )
#	if defined( ASRD_GNS_WRAPPER_EXPORTS )
#		define ASRD_GNS_WRAPPER_API __declspec( dllexport )
#	else
#		define ASRD_GNS_WRAPPER_API __declspec( dllimport )
#	endif
#else
#	define ASRD_GNS_WRAPPER_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

// This is an opaque game-side token.  The wrapper owns the actual GNS handle.
typedef uint32_t ASRD_GNS_Connection;

enum
{
	ASRD_GNS_CONNECTION_INVALID = 0,
	ASRD_GNS_CONNECTION_ACTIVE = 1,
};

// Outbound lanes are configured independently for each connection direction.
// Lane 0 is the single reliable ordering domain; both unreliable lanes share
// the session-local shared U reorder domain in the game bridge.
enum
{
	ASRD_GNS_LANE_R = 0,
	ASRD_GNS_LANE_U_REALTIME = 1,
	ASRD_GNS_LANE_U_NORMAL = 2,
	ASRD_GNS_LANE_UNKNOWN = 0xFF,
};

// These values intentionally mirror the public GNS send flags while keeping
// the game-facing ABI independent of GNS headers.
enum
{
	ASRD_GNS_SEND_UNRELIABLE = 0,
	ASRD_GNS_SEND_UNRELIABLE_NO_NAGLE = 1,
	ASRD_GNS_SEND_RELIABLE = 8,
};

// Native EResult values returned through the integer wrapper ABI.
enum
{
	ASRD_GNS_RESULT_OK = 1,
	ASRD_GNS_RESULT_INVALID_PARAM = 8,
	ASRD_GNS_RESULT_INVALID_STATE = 11,
	ASRD_GNS_RESULT_LIMIT_EXCEEDED = 25,
	ASRD_GNS_RESULT_NO_CONNECTION = 3,
};

// Connection status events are copied out of the wrapper-owned callback
// queue.  The game DLL never includes GNS headers or receives native GNS
// objects across this ABI.
enum
{
	ASRD_GNS_CONNECTION_EVENT_NONE = 0,
	ASRD_GNS_CONNECTION_EVENT_CONNECTING = 1,
	ASRD_GNS_CONNECTION_EVENT_CONNECTED = 2,
	ASRD_GNS_CONNECTION_EVENT_FAILED = 3,
	ASRD_GNS_CONNECTION_EVENT_CLOSED = 4,
	// A server-side connection is only ready for AcceptConnection after this
	// event.  The callback queues it; the game thread performs the accept.
	ASRD_GNS_CONNECTION_EVENT_INCOMING = 5,
};

typedef struct ASRD_GNS_ConnectionEvent
{
	ASRD_GNS_Connection connection;
	int state;
	int reason;
} ASRD_GNS_ConnectionEvent;

// The game-facing ABI deliberately contains no GameNetworkingSockets types or headers.
// Selects the wrapper's transport mode; this is independent of the game
// DLL/runtime identity tracked by the game-side lifecycle.
ASRD_GNS_WRAPPER_API int ASRD_GNS_Initialize( int serverTransport );
ASRD_GNS_WRAPPER_API int ASRD_GNS_Listen( uint16_t port );
ASRD_GNS_WRAPPER_API ASRD_GNS_Connection ASRD_GNS_Connect( const char *ipv4, uint16_t port );
ASRD_GNS_WRAPPER_API int ASRD_GNS_AcceptConnection( ASRD_GNS_Connection connection );
ASRD_GNS_WRAPPER_API void ASRD_GNS_RunFrame( void );
ASRD_GNS_WRAPPER_API int ASRD_GNS_PollConnectionEvent( ASRD_GNS_ConnectionEvent *event );
ASRD_GNS_WRAPPER_API int ASRD_GNS_ConfigureLanes( ASRD_GNS_Connection connection );
ASRD_GNS_WRAPPER_API int ASRD_GNS_SendLane( ASRD_GNS_Connection connection,
	const void *data, uint32_t size, uint8_t lane, int flags );
ASRD_GNS_WRAPPER_API int ASRD_GNS_Flush( ASRD_GNS_Connection connection );
// Compatibility probe API. New bridge traffic uses ASRD_GNS_SendLane.
ASRD_GNS_WRAPPER_API int ASRD_GNS_SendReliable( ASRD_GNS_Connection connection, const void *data, uint32_t size );
ASRD_GNS_WRAPPER_API int ASRD_GNS_Receive( ASRD_GNS_Connection connection, void *buffer, uint32_t capacity, uint32_t *size );
ASRD_GNS_WRAPPER_API void ASRD_GNS_Close( ASRD_GNS_Connection connection );
ASRD_GNS_WRAPPER_API void ASRD_GNS_Shutdown( void );

#ifdef __cplusplus
}
#endif

