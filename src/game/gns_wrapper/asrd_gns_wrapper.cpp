#include "asrd_gns_wrapper.h"

#include <winsock2.h>
#include <windows.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <atomic>

// This translation unit is the game-side inclusion point for the native
// GameNetworkingSockets interfaces.  The library is linked statically into
// this DLL.
#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>

namespace
{
	static bool s_initialized = false;
	// This flag describes the wrapper's active transport mode only.  It is not
	// the game DLL/runtime identity, which is tracked by the game-side role
	// state and the explicit dedicated-server query.
	static bool s_transportServerMode = false;
	static ISteamNetworkingSockets *s_sockets = NULL;
	static HSteamListenSocket s_listenSocket = k_HSteamListenSocket_Invalid;
	static const unsigned int kMaxConnections = 8;
	// Each in-use slot binds one opaque public token to one native GNS handle.
	// accepted, connected, and closing record observed lifecycle transitions;
	// the token remains stable until the slot is retired or the table is reset.
	struct ConnectionSlot
	{
		ASRD_GNS_Connection token;
		HSteamNetConnection native;
		bool inUse;
		bool accepted;
		bool connected;
		bool closing;
		bool lanesConfigured;
	};
	static ConnectionSlot s_connections[ kMaxConnections ] = {};
	static std::atomic<uint64_t> s_laneSendAttempts[ 3 ];
	static std::atomic<uint64_t> s_laneSendBytes[ 3 ];
	static std::atomic<uint64_t> s_laneSendFailures[ 3 ];
	static ASRD_GNS_Connection s_nextConnectionToken = 2;
	// The vendored static library is built without
	// STEAMNETWORKINGSOCKETS_OPENSOURCE, so its IP_AllowWithoutAuth default is
	// disabled.  GNS also defaults the initial handshake timeout to 10 seconds.
	// The ASRD transport is intentionally direct-IP and usable without a Steam
	// identity, so make both settings explicit instead of depending on library
	// build-flavor defaults.
	static const int kGnsInitialConnectionTimeoutMs = 60000;
	static const int kGnsAllowWithoutAuth = 2;
	static const int kGnsConnectionOptionCount = 2;

	static void SetGnsConnectionOptions( SteamNetworkingConfigValue_t *options )
	{
		options[ 0 ].SetInt32(
			k_ESteamNetworkingConfig_TimeoutInitial,
			kGnsInitialConnectionTimeoutMs );
		options[ 1 ].SetInt32(
			k_ESteamNetworkingConfig_IP_AllowWithoutAuth,
			kGnsAllowWithoutAuth );
	}

	// ACTIVE resolves through s_activeConnectionToken and is not stored in a
	// connection slot.  It is retained as a compatibility alias for the
	// existing smoke probe; new client/server code uses the per-connection token.
	static ASRD_GNS_Connection s_activeConnectionToken = ASRD_GNS_CONNECTION_INVALID;
	static INIT_ONCE s_eventQueueInitOnce = INIT_ONCE_STATIC_INIT;
	static CRITICAL_SECTION s_eventQueueLock;
	static ASRD_GNS_ConnectionEvent s_eventQueue[ 32 ];
	static unsigned int s_eventHead = 0;
	static unsigned int s_eventTail = 0;
	static INIT_ONCE s_connectionTableInitOnce = INIT_ONCE_STATIC_INIT;
	static CRITICAL_SECTION s_connectionTableLock;
	static void Log( const char *format, ... );

	static BOOL CALLBACK InitializeEventQueue( PINIT_ONCE, PVOID, PVOID * )
	{
		InitializeCriticalSection( &s_eventQueueLock );
		return TRUE;
	}

	static void EnsureEventQueueInitialized( void )
	{
		InitOnceExecuteOnce( &s_eventQueueInitOnce, InitializeEventQueue, NULL, NULL );
	}

	static BOOL CALLBACK InitializeConnectionTable( PINIT_ONCE, PVOID, PVOID * )
	{
		InitializeCriticalSection( &s_connectionTableLock );
		return TRUE;
	}

	static void EnsureConnectionTableInitialized( void )
	{
		InitOnceExecuteOnce( &s_connectionTableInitOnce, InitializeConnectionTable, NULL, NULL );
	}

	static bool QueueConnectionEvent( ASRD_GNS_ConnectionEvent event )
	{
		EnsureEventQueueInitialized();
		bool queued = false;
		EnterCriticalSection( &s_eventQueueLock );
		const unsigned int nextTail = ( s_eventTail + 1 ) % ARRAYSIZE( s_eventQueue );
		if ( nextTail != s_eventHead )
		{
			s_eventQueue[ s_eventTail ] = event;
			s_eventTail = nextTail;
			queued = true;
		}
		else
		{
			Log( "[ASRD-GNS] connection event queue full; dropping state=%d reason=%d\n",
				event.state, event.reason );
		}
		LeaveCriticalSection( &s_eventQueueLock );
		return queued;
	}

	static void Log( const char *format, ... )
	{
		char buffer[ 1024 ];
		va_list args;
		va_start( args, format );
		_vsnprintf_s( buffer, sizeof( buffer ), _TRUNCATE, format, args );
		va_end( args );

		OutputDebugStringA( buffer );
		fputs( buffer, stdout );
		fflush( stdout );
	}

	static int FindSlotByTokenLocked( ASRD_GNS_Connection token )
	{
		for ( unsigned int i = 0; i < kMaxConnections; ++i )
		{
			if ( s_connections[ i ].inUse && s_connections[ i ].token == token )
				return (int)i;
		}
		return -1;
	}

	static int FindSlotByNativeLocked( HSteamNetConnection native )
	{
		for ( unsigned int i = 0; i < kMaxConnections; ++i )
		{
			if ( s_connections[ i ].inUse && s_connections[ i ].native == native )
				return (int)i;
		}
		return -1;
	}

	static ASRD_GNS_Connection AllocateConnectionToken( HSteamNetConnection native )
	{
		if ( native == k_HSteamNetConnection_Invalid )
			return ASRD_GNS_CONNECTION_INVALID;

		EnsureConnectionTableInitialized();
		EnterCriticalSection( &s_connectionTableLock );
		const int existing = FindSlotByNativeLocked( native );
		if ( existing >= 0 )
		{
			const ASRD_GNS_Connection token = s_connections[ existing ].token;
			LeaveCriticalSection( &s_connectionTableLock );
			return token;
		}

		int freeSlot = -1;
		for ( unsigned int i = 0; i < kMaxConnections; ++i )
		{
			if ( !s_connections[ i ].inUse )
			{
				freeSlot = (int)i;
				break;
			}
		}
		if ( freeSlot < 0 )
		{
			LeaveCriticalSection( &s_connectionTableLock );
			Log( "[ASRD-GNS] connection table full; native=%lu\n", (unsigned long)native );
			return ASRD_GNS_CONNECTION_INVALID;
		}

		ASRD_GNS_Connection token = s_nextConnectionToken++;
		if ( token == ASRD_GNS_CONNECTION_INVALID || token == ASRD_GNS_CONNECTION_ACTIVE )
			token = s_nextConnectionToken++;
		ConnectionSlot &slot = s_connections[ freeSlot ];
		slot.token = token;
		slot.native = native;
		slot.inUse = true;
		slot.accepted = false;
		slot.connected = false;
		slot.closing = false;
		slot.lanesConfigured = false;
		LeaveCriticalSection( &s_connectionTableLock );
		return token;
	}

	static HSteamNetConnection ResolveConnection( ASRD_GNS_Connection connection,
		ASRD_GNS_Connection *resolvedToken = NULL )
	{
		if ( resolvedToken )
			*resolvedToken = ASRD_GNS_CONNECTION_INVALID;
		EnsureConnectionTableInitialized();
		EnterCriticalSection( &s_connectionTableLock );
		ASRD_GNS_Connection token = connection;
		if ( token == ASRD_GNS_CONNECTION_ACTIVE )
			token = s_activeConnectionToken;
		const int index = FindSlotByTokenLocked( token );
		const HSteamNetConnection native = index >= 0
			? s_connections[ index ].native : k_HSteamNetConnection_Invalid;
		if ( index >= 0 && resolvedToken )
			*resolvedToken = token;
		LeaveCriticalSection( &s_connectionTableLock );
		return native;
	}

	static bool GetConnectionSlotState( ASRD_GNS_Connection connection,
		bool *closing )
	{
		if ( closing )
			*closing = false;
		if ( connection == ASRD_GNS_CONNECTION_INVALID )
			return false;

		EnsureConnectionTableInitialized();
		EnterCriticalSection( &s_connectionTableLock );
		ASRD_GNS_Connection token = connection;
		if ( token == ASRD_GNS_CONNECTION_ACTIVE )
			token = s_activeConnectionToken;
		const int index = FindSlotByTokenLocked( token );
		const bool found = index >= 0;
		if ( found && closing )
			*closing = s_connections[ index ].closing;
		LeaveCriticalSection( &s_connectionTableLock );
		return found;
	}

	static ASRD_GNS_Connection FindOrAllocateToken( HSteamNetConnection native )
	{
		return AllocateConnectionToken( native );
	}

	static bool MarkConnectionState( HSteamNetConnection native, bool accepted,
		bool connected, ASRD_GNS_Connection *token )
	{
		if ( token )
			*token = ASRD_GNS_CONNECTION_INVALID;
		EnsureConnectionTableInitialized();
		EnterCriticalSection( &s_connectionTableLock );
		const int index = FindSlotByNativeLocked( native );
		if ( index < 0 )
		{
			LeaveCriticalSection( &s_connectionTableLock );
			return false;
		}
		ConnectionSlot &slot = s_connections[ index ];
		if ( slot.closing )
		{
			LeaveCriticalSection( &s_connectionTableLock );
			return false;
		}
		if ( accepted )
			slot.accepted = true;
		if ( connected )
			slot.connected = true;
		if ( token )
			*token = slot.token;
		LeaveCriticalSection( &s_connectionTableLock );
		return true;
	}

	static bool MarkConnectionClosing( HSteamNetConnection native,
		ASRD_GNS_Connection *token, bool *connected )
	{
		if ( token )
			*token = ASRD_GNS_CONNECTION_INVALID;
		if ( connected )
			*connected = false;
		EnsureConnectionTableInitialized();
		EnterCriticalSection( &s_connectionTableLock );
		const int index = FindSlotByNativeLocked( native );
		if ( index < 0 || s_connections[ index ].closing )
		{
			LeaveCriticalSection( &s_connectionTableLock );
			return false;
		}
		ConnectionSlot &slot = s_connections[ index ];
		if ( token )
			*token = slot.token;
		if ( connected )
			*connected = slot.connected;
		slot.closing = true;
		LeaveCriticalSection( &s_connectionTableLock );
		return true;
	}

	static void RetireConnectionToken( ASRD_GNS_Connection token )
	{
		if ( token == ASRD_GNS_CONNECTION_INVALID || token == ASRD_GNS_CONNECTION_ACTIVE )
			return;
		EnsureConnectionTableInitialized();
		EnterCriticalSection( &s_connectionTableLock );
		const int index = FindSlotByTokenLocked( token );
		if ( index >= 0 )
		{
			if ( s_activeConnectionToken == token )
				s_activeConnectionToken = ASRD_GNS_CONNECTION_INVALID;
			memset( &s_connections[ index ], 0, sizeof( s_connections[ index ] ) );
		}
		LeaveCriticalSection( &s_connectionTableLock );
	}

	static void SetActiveConnectionToken( ASRD_GNS_Connection token )
	{
		EnsureConnectionTableInitialized();
		EnterCriticalSection( &s_connectionTableLock );
		s_activeConnectionToken = token;
		LeaveCriticalSection( &s_connectionTableLock );
	}

	static bool CloseAndRetireConnection( ASRD_GNS_Connection connection )
	{
		if ( connection == ASRD_GNS_CONNECTION_INVALID )
			return false;

		HSteamNetConnection native = k_HSteamNetConnection_Invalid;
		ASRD_GNS_Connection token = ASRD_GNS_CONNECTION_INVALID;
		bool queueTerminal = false;
		bool connected = false;
		EnsureConnectionTableInitialized();
		EnterCriticalSection( &s_connectionTableLock );
		token = connection == ASRD_GNS_CONNECTION_ACTIVE
			? s_activeConnectionToken : connection;
		const int index = FindSlotByTokenLocked( token );
		if ( index < 0 )
		{
			if ( s_activeConnectionToken == token )
				s_activeConnectionToken = ASRD_GNS_CONNECTION_INVALID;
			LeaveCriticalSection( &s_connectionTableLock );
			return false;
		}
		native = s_connections[ index ].native;
		queueTerminal = !s_connections[ index ].closing;
		connected = s_connections[ index ].connected;
		s_connections[ index ].closing = true;
		if ( s_activeConnectionToken == token )
			s_activeConnectionToken = ASRD_GNS_CONNECTION_INVALID;
		LeaveCriticalSection( &s_connectionTableLock );

		if ( s_sockets && native != k_HSteamNetConnection_Invalid )
			s_sockets->CloseConnection( native, 0, "game wrapper close", false );
		if ( queueTerminal )
		{
			const ASRD_GNS_ConnectionEvent event = { token,
				connected ? ASRD_GNS_CONNECTION_EVENT_CLOSED
						  : ASRD_GNS_CONNECTION_EVENT_FAILED, 0 };
			QueueConnectionEvent( event );
		}
		RetireConnectionToken( token );
		return true;
	}

	static void OnConnectionStatusChanged( SteamNetConnectionStatusChangedCallback_t *info )
	{
		if ( !info || !s_sockets )
			return;

		if ( info->m_info.m_eState == k_ESteamNetworkingConnectionState_Connecting )
		{
			const ASRD_GNS_Connection token = FindOrAllocateToken( info->m_hConn );
			if ( token == ASRD_GNS_CONNECTION_INVALID )
				return;
			bool closing = false;
			if ( !GetConnectionSlotState( token, &closing ) || closing )
				return;

			const bool incoming = s_transportServerMode &&
				info->m_info.m_hListenSocket != k_HSteamListenSocket_Invalid;
			ASRD_GNS_ConnectionEvent event = { token,
				incoming ? ASRD_GNS_CONNECTION_EVENT_INCOMING
						: ASRD_GNS_CONNECTION_EVENT_CONNECTING, 0 };
			if ( QueueConnectionEvent( event ) )
			{
				Log( "[ASRD-GNS] %s queued token=%lu\n",
					incoming ? "server incoming" : "client connecting",
					(unsigned long)token );
			}
			return;
		}

		if ( info->m_info.m_eState == k_ESteamNetworkingConnectionState_Connected )
		{
			ASRD_GNS_Connection token = ASRD_GNS_CONNECTION_INVALID;
			if ( !MarkConnectionState( info->m_hConn, false, true, &token ) )
				return;
			ASRD_GNS_ConnectionEvent event = { token,
				ASRD_GNS_CONNECTION_EVENT_CONNECTED, 0 };
			if ( QueueConnectionEvent( event ) )
				Log( "[ASRD-GNS] connected queued role=%s token=%lu\n",
					 s_transportServerMode ? "server" : "client", (unsigned long)token );
			return;
		}

		if ( info->m_info.m_eState == k_ESteamNetworkingConnectionState_ClosedByPeer ||
				 info->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally )
		{
			ASRD_GNS_Connection token = ASRD_GNS_CONNECTION_INVALID;
			bool connected = false;
			if ( !MarkConnectionClosing( info->m_hConn, &token, &connected ) )
				return;
			const int eventState = connected
				? ASRD_GNS_CONNECTION_EVENT_CLOSED
				: ASRD_GNS_CONNECTION_EVENT_FAILED;
			ASRD_GNS_ConnectionEvent event = { token,
				eventState, info->m_info.m_eEndReason };
			if ( QueueConnectionEvent( event ) )
				Log( "[ASRD-GNS] terminal queued state=%d reason=%d event=%d token=%lu\n",
					(int)info->m_info.m_eState, info->m_info.m_eEndReason, eventState,
					(unsigned long)token );
			else
				CloseAndRetireConnection( token );
		}
	}
}

extern "C" ASRD_GNS_WRAPPER_API int ASRD_GNS_Initialize( int serverTransport )
{
	if ( s_initialized )
		return s_transportServerMode == ( serverTransport != 0 ) ? 1 : 0;

	SteamNetworkingErrMsg errorMessage = {};
	if ( !GameNetworkingSockets_Init( NULL, errorMessage ) )
	{
		Log( "[ASRD-GNS] initialize failed: %s\n", errorMessage );
		return 0;
	}

	s_transportServerMode = serverTransport != 0;
	EnsureConnectionTableInitialized();
	s_sockets = SteamNetworkingSockets();
	ISteamNetworkingUtils *utils = SteamNetworkingUtils();
	if ( !s_sockets || !utils || !utils->SetGlobalCallback_SteamNetConnectionStatusChanged( OnConnectionStatusChanged ) )
	{
		Log( "[ASRD-GNS] initialize failed: missing standalone interfaces\n" );
		GameNetworkingSockets_Kill();
		s_sockets = NULL;
		return 0;
	}

	s_initialized = true;
	Log( "[ASRD-GNS] initialized transport=%s\n",
		s_transportServerMode ? "server" : "client" );
	return 1;
}

extern "C" ASRD_GNS_WRAPPER_API int ASRD_GNS_Listen( uint16_t port )
{
	if ( !s_initialized || !s_transportServerMode || !s_sockets )
		return ASRD_GNS_CONNECTION_INVALID;

	if ( s_listenSocket != k_HSteamListenSocket_Invalid )
		return ASRD_GNS_CONNECTION_ACTIVE;

	SteamNetworkingIPAddr address;
	address.Clear();
	address.SetIPv4( 0, port );
	SteamNetworkingConfigValue_t options[ kGnsConnectionOptionCount ];
	SetGnsConnectionOptions( options );
	s_listenSocket = s_sockets->CreateListenSocketIP(
		address, kGnsConnectionOptionCount, options );
	if ( s_listenSocket == k_HSteamListenSocket_Invalid )
	{
		Log( "[ASRD-GNS] server listen failed port=%u\n", (unsigned)port );
		return ASRD_GNS_CONNECTION_INVALID;
	}

	Log( "[ASRD-GNS] server ready listen=0.0.0.0:%u timeout_initial_ms=%d allow_without_auth=%d\n",
		(unsigned)port, kGnsInitialConnectionTimeoutMs, kGnsAllowWithoutAuth );
	return ASRD_GNS_CONNECTION_ACTIVE;
}

extern "C" ASRD_GNS_WRAPPER_API ASRD_GNS_Connection ASRD_GNS_Connect( const char *ipv4, uint16_t port )
{
	if ( !s_initialized || s_transportServerMode || !s_sockets )
		return ASRD_GNS_CONNECTION_INVALID;

	CloseAndRetireConnection( ASRD_GNS_CONNECTION_ACTIVE );

	const char *target = ( ipv4 && ipv4[0] ) ? ipv4 : NULL;
	if ( !target )
	{
		Log( "[ASRD-GNS] client rejected missing target\n" );
		return ASRD_GNS_CONNECTION_INVALID;
	}
	const unsigned long packedAddress = inet_addr( target );
	if ( packedAddress == INADDR_NONE && strcmp( target, "255.255.255.255" ) != 0 )
	{
		Log( "[ASRD-GNS] client rejected non-IPv4 target=%s\n", target );
		return ASRD_GNS_CONNECTION_INVALID;
	}
	const unsigned long hostAddress = ntohl( packedAddress );
	if ( hostAddress == 0 || ( hostAddress >> 24 ) == 127 || ( hostAddress >> 24 ) >= 224 )
	{
		Log( "[ASRD-GNS] client rejected loopback/unspecified target=%s\n", target );
		return ASRD_GNS_CONNECTION_INVALID;
	}

	SteamNetworkingIPAddr address;
	address.Clear();
	address.SetIPv4( hostAddress, port );
	SteamNetworkingConfigValue_t options[ kGnsConnectionOptionCount ];
	SetGnsConnectionOptions( options );
	const HSteamNetConnection native = s_sockets->ConnectByIPAddress(
		address, kGnsConnectionOptionCount, options );
	if ( native == k_HSteamNetConnection_Invalid )
	{
		Log( "[ASRD-GNS] client connect failed target=%s:%u\n", target, (unsigned)port );
		return ASRD_GNS_CONNECTION_INVALID;
	}

	const ASRD_GNS_Connection token = AllocateConnectionToken( native );
	if ( token == ASRD_GNS_CONNECTION_INVALID )
	{
		s_sockets->CloseConnection( native, 0, "connection table full", false );
		return ASRD_GNS_CONNECTION_INVALID;
	}
	SetActiveConnectionToken( token );
	Log( "[ASRD-GNS] client connecting target=%s:%u timeout_initial_ms=%d allow_without_auth=%d token=%lu\n",
		target, (unsigned)port, kGnsInitialConnectionTimeoutMs,
		kGnsAllowWithoutAuth, (unsigned long)token );
	return token;
}

extern "C" ASRD_GNS_WRAPPER_API int ASRD_GNS_AcceptConnection( ASRD_GNS_Connection connection )
{
	if ( !s_initialized || !s_transportServerMode || !s_sockets )
		return 0;

	ASRD_GNS_Connection resolvedConnection = ASRD_GNS_CONNECTION_INVALID;
	const HSteamNetConnection native = ResolveConnection( connection, &resolvedConnection );
	if ( native == k_HSteamNetConnection_Invalid )
	{
		Log( "[ASRD-GNS] server accept rejected unknown token=%lu\n",
			(unsigned long)connection );
		return 0;
	}

	const EResult result = s_sockets->AcceptConnection( native );
	if ( result != k_EResultOK )
	{
		Log( "[ASRD-GNS] server accept failed token=%lu result=%d\n",
			(unsigned long)connection, (int)result );
		return 0;
	}

	if ( !MarkConnectionState( native, true, false, &resolvedConnection ) )
	{
		CloseAndRetireConnection( resolvedConnection );
		return 0;
	}
	SetActiveConnectionToken( resolvedConnection );
	Log( "[ASRD-GNS] server accept succeeded token=%lu\n",
		(unsigned long)resolvedConnection );
	return 1;
}

extern "C" ASRD_GNS_WRAPPER_API void ASRD_GNS_RunFrame( void )
{
	if ( s_initialized && s_sockets )
		s_sockets->RunCallbacks();
}

extern "C" ASRD_GNS_WRAPPER_API int ASRD_GNS_PollConnectionEvent( ASRD_GNS_ConnectionEvent *event )
{
	if ( !event )
		return 0;

	EnsureEventQueueInitialized();
	for ( ;; )
	{
		EnterCriticalSection( &s_eventQueueLock );
		if ( s_eventHead == s_eventTail )
		{
			LeaveCriticalSection( &s_eventQueueLock );
			return 0;
		}

		const ASRD_GNS_ConnectionEvent queuedEvent = s_eventQueue[ s_eventHead ];
		s_eventHead = ( s_eventHead + 1 ) % ARRAYSIZE( s_eventQueue );
		LeaveCriticalSection( &s_eventQueueLock );

		const bool terminal = queuedEvent.state == ASRD_GNS_CONNECTION_EVENT_FAILED ||
			queuedEvent.state == ASRD_GNS_CONNECTION_EVENT_CLOSED;
		if ( !terminal )
		{
			bool closing = false;
			if ( !GetConnectionSlotState( queuedEvent.connection, &closing ) || closing )
				continue;
		}

		// The server/client terminal consumers call ASRD_GNS_Close after handling
		// the event. Keep the token mapped until then so a remote terminal still
		// has a resolvable native handle; local close already retired its token.
		*event = queuedEvent;
		return 1;
	}
}

extern "C" ASRD_GNS_WRAPPER_API int ASRD_GNS_ConfigureLanes(
	ASRD_GNS_Connection connection )
{
	if ( connection == ASRD_GNS_CONNECTION_INVALID ||
		!s_initialized || !s_sockets )
		return (int)k_EResultNoConnection;

	const HSteamNetConnection nativeConnection = ResolveConnection( connection );
	if ( !s_sockets || nativeConnection == k_HSteamNetConnection_Invalid )
		return (int)k_EResultNoConnection;

	EnsureConnectionTableInitialized();
	EnterCriticalSection( &s_connectionTableLock );
	const int slotIndex = FindSlotByNativeLocked( nativeConnection );
	const bool alreadyConfigured = slotIndex >= 0 &&
		s_connections[ slotIndex ].lanesConfigured;
	LeaveCriticalSection( &s_connectionTableLock );
	if ( alreadyConfigured )
		return (int)k_EResultOK;

	// All lanes have the same priority. Weights only bias resource allocation
	// among these equal-priority lanes; they do not alter reliability, ordering,
	// or the bridge's cross-lane receive sequence policy.
	const int lanePriorities[ 3 ] = { 0, 0, 0 };
	// Lane order is R, U realtime, U normal. The initial 8:4:1 proposal is
	// represented as U realtime:R:U normal and remains a runtime tuning value.
	const uint16 laneWeights[ 3 ] = { 4, 8, 1 };
	const EResult result = s_sockets->ConfigureConnectionLanes( nativeConnection,
		3, lanePriorities, laneWeights );
	Log( "[ASRD-GNS] ConfigureConnectionLanes token=%lu lanes=3 priority={%d,%d,%d} weight={%u,%u,%u} scheduling=equal_priority_weighted result=%d\n",
		(unsigned long)connection, lanePriorities[ 0 ], lanePriorities[ 1 ],
		lanePriorities[ 2 ], (unsigned)laneWeights[ 0 ],
		(unsigned)laneWeights[ 1 ], (unsigned)laneWeights[ 2 ], (int)result );
	if ( result == k_EResultOK )
	{
		EnterCriticalSection( &s_connectionTableLock );
		const int currentIndex = FindSlotByNativeLocked( nativeConnection );
		if ( currentIndex >= 0 )
			s_connections[ currentIndex ].lanesConfigured = true;
		LeaveCriticalSection( &s_connectionTableLock );
	}

	return (int)result;
}

// Send and receive are hot paths.  They return native EResult values to the
// game-side bridge so every failed block records the exact GNS result.
extern "C" ASRD_GNS_WRAPPER_API int ASRD_GNS_SendLane(
	ASRD_GNS_Connection connection, const void *data, uint32_t size,
	uint8_t lane, int flags )
{
	const bool validLaneFlags =
		( lane == ASRD_GNS_LANE_R &&
			flags == ASRD_GNS_SEND_RELIABLE ) ||
		( lane == ASRD_GNS_LANE_U_REALTIME &&
			flags == ASRD_GNS_SEND_UNRELIABLE_NO_NAGLE ) ||
		( lane == ASRD_GNS_LANE_U_NORMAL &&
			flags == ASRD_GNS_SEND_UNRELIABLE );
	if ( !validLaneFlags || ( size > 0 && !data ) )
		return (int)k_EResultInvalidParam;

	const HSteamNetConnection nativeConnection = ResolveConnection( connection );
	if ( !s_initialized || !s_sockets ||
		nativeConnection == k_HSteamNetConnection_Invalid )
		return (int)k_EResultNoConnection;

	const int configureResult = ASRD_GNS_ConfigureLanes( connection );
	if ( configureResult != (int)k_EResultOK )
		return configureResult;

	ISteamNetworkingUtils *utils = SteamNetworkingUtils();
	if ( !utils )
		return (int)k_EResultFail;
	SteamNetworkingMessage_t *message = utils->AllocateMessage( (int)size );
	if ( !message || ( size > 0 && !message->m_pData ) )
	{
		if ( message )
			message->Release();
		return (int)k_EResultFail;
	}
	if ( size > 0 )
		memcpy( message->m_pData, data, (size_t)size );
	message->m_conn = nativeConnection;
	message->m_cbSize = (int)size;
	message->m_nFlags = flags;
	message->m_idxLane = (uint16)lane;

	SteamNetworkingMessage_t *messages[ 1 ] = { message };
	int64 messageResult = 0;
	s_sockets->SendMessages( 1, messages, &messageResult );
	const int result = messageResult < 0
		? (int)( -messageResult ) : (int)k_EResultOK;
	s_laneSendAttempts[ lane ].fetch_add( 1 );
	s_laneSendBytes[ lane ].fetch_add( size );
	if ( result != (int)k_EResultOK )
		s_laneSendFailures[ lane ].fetch_add( 1 );
	return result;
}

extern "C" ASRD_GNS_WRAPPER_API int ASRD_GNS_Flush(
	ASRD_GNS_Connection connection )
{
	const HSteamNetConnection nativeConnection = ResolveConnection( connection );
	if ( !s_initialized || !s_sockets ||
		nativeConnection == k_HSteamNetConnection_Invalid )
		return (int)k_EResultNoConnection;

	const EResult result = s_sockets->FlushMessagesOnConnection( nativeConnection );
	if ( result != k_EResultOK )
	{
		Log( "[ASRD-GNS] FlushMessagesOnConnection token=%lu result=%d\n",
			(unsigned long)connection, (int)result );
	}
	return (int)result;
}

extern "C" ASRD_GNS_WRAPPER_API int ASRD_GNS_SendReliable(
	ASRD_GNS_Connection connection, const void *data, uint32_t size )
{
	return ASRD_GNS_SendLane( connection, data, size, ASRD_GNS_LANE_R,
		k_nSteamNetworkingSend_Reliable ) == (int)k_EResultOK ? 1 : 0;
}

extern "C" ASRD_GNS_WRAPPER_API int ASRD_GNS_Receive( ASRD_GNS_Connection connection, void *buffer, uint32_t capacity, uint32_t *size )
{
	if ( size )
		*size = 0;

	const HSteamNetConnection nativeConnection = ResolveConnection( connection );
	if ( !s_sockets || nativeConnection == k_HSteamNetConnection_Invalid )
		return -1;


	SteamNetworkingMessage_t *message = NULL;
	const int count = s_sockets->ReceiveMessagesOnConnection( nativeConnection, &message, 1 );
	if ( count <= 0 || !message )
		return count < 0 ? -1 : 0;

	const bool fits = message->m_cbSize >= 0 && (uint32_t)message->m_cbSize <= capacity &&
		( message->m_cbSize == 0 || buffer != NULL );
	if ( fits )
	{
		if ( message->m_cbSize > 0 )
			memcpy( buffer, message->m_pData, (size_t)message->m_cbSize );
		if ( size )
			*size = (uint32_t)message->m_cbSize;
	}

	message->Release();
	return fits ? 1 : -1;
}

extern "C" ASRD_GNS_WRAPPER_API void ASRD_GNS_Close( ASRD_GNS_Connection connection )
{
	if ( CloseAndRetireConnection( connection ) )
		Log( "[ASRD-GNS] close requested token=%lu\n", (unsigned long)connection );
}

extern "C" ASRD_GNS_WRAPPER_API void ASRD_GNS_Shutdown( void )
{
	if ( !s_initialized )
		return;

	if ( s_sockets )
	{
		HSteamNetConnection nativeConnections[ kMaxConnections ] = {};
		unsigned int nativeCount = 0;
		EnsureConnectionTableInitialized();
		EnterCriticalSection( &s_connectionTableLock );
		for ( unsigned int i = 0; i < kMaxConnections; ++i )
		{
			if ( s_connections[ i ].inUse )
				nativeConnections[ nativeCount++ ] = s_connections[ i ].native;
		}
		LeaveCriticalSection( &s_connectionTableLock );
		for ( unsigned int i = 0; i < nativeCount; ++i )
			s_sockets->CloseConnection( nativeConnections[ i ], 0,
				"wrapper shutdown", false );
	}
	if ( s_sockets && s_listenSocket != k_HSteamListenSocket_Invalid )
		s_sockets->CloseListenSocket( s_listenSocket );

	GameNetworkingSockets_Kill();
	EnsureEventQueueInitialized();
	EnterCriticalSection( &s_eventQueueLock );
	memset( s_eventQueue, 0, sizeof( s_eventQueue ) );
	s_eventHead = 0;
	s_eventTail = 0;
	LeaveCriticalSection( &s_eventQueueLock );
	EnsureConnectionTableInitialized();
	EnterCriticalSection( &s_connectionTableLock );
	memset( s_connections, 0, sizeof( s_connections ) );
	s_activeConnectionToken = ASRD_GNS_CONNECTION_INVALID;
	LeaveCriticalSection( &s_connectionTableLock );
	s_listenSocket = k_HSteamListenSocket_Invalid;
	s_sockets = NULL;
	s_initialized = false;
	s_transportServerMode = false;
	for ( unsigned int lane = 0; lane < 3; ++lane )
	{
		s_laneSendAttempts[ lane ].store( 0 );
		s_laneSendBytes[ lane ].store( 0 );
		s_laneSendFailures[ lane ].store( 0 );
	}
	Log( "[ASRD-GNS] shutdown\n" );
}
