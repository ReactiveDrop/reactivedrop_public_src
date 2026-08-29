#include "cbase.h"
#include "tier0/dbg.h"
#include "tier1/bitbuf.h"
#include "tier1/netadr.h"
#include "inetmessage.h"
#include "asrd_gns_message_bridge.h"
#include "asrd_gns_message_registry.h"
#include "asrd_gns_server_lifecycle.h"

#if defined( _WIN32 ) && !defined( _X360 )

#include <windows.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#if defined( GAME_DLL )
#include "../server/gameinterface.h"
#include "../server/entitylist.h"
#include "../server/player.h"
#include "../server/util.h"
extern CServerGameClients g_ServerGameClients;
extern CServerGameDLL g_ServerGameDLL;
// The game-rules translation unit owns this existing ConVar.  Keep only an
// external declaration here so the shared lifecycle module does not emit a
// second server-side definition.
extern ConVar rd_server_shutdown_when_empty;
#endif

struct ASRD_GNS_ServerConnectionContext
{
	ASRD_GNS_Connection connection;
	ASRD_GNS_ServerConnectionState state;
	int reason;

	// All command-side values are CUserCmd.command_number values. These fields
	// are scoped to the one currently supported GNS connection.
	bool has_last_accepted_command_number;
	int last_accepted_command_number;
	bool processing_move;
	int current_move_drop_number;

	// Server->client logical-update metadata. Values are captured at flush,
	// not at the first append to the pending update.
	uint32_t next_server_update_seq;
	uint32_t server_update_seq;
	int client_command_ack;
};

namespace
{
	static bool s_initialized = false;
	static uint16_t s_requestedPort = 0;
	static uint16_t s_listenPort = 0;
	static ASRD_GNS_ServerConnectionContext s_serverConnection = {};
	static ASRD_GNS_ServerConnectionContext &s_connection = s_serverConnection;

	static void ResetCompatibilityState(
		ASRD_GNS_ServerConnectionContext *context )
	{
		if ( !context )
			return;
		context->has_last_accepted_command_number = false;
		context->last_accepted_command_number = 0;
		context->processing_move = false;
		context->current_move_drop_number = 0;
		context->next_server_update_seq = 0;
		context->server_update_seq = 0;
		context->client_command_ack = 0;
	}

	// These x86 bindings are accepted only for the fixed PE identity below.
	// Task 20 records the file bytes and instruction-level evidence.
	static const DWORD kEngineTimeDateStamp = 0x5F363761;
	static const DWORD kEngineSizeOfImage = 0x006F2000;
	static const DWORD kServerGlobalRva = 0x000598480;
	static const DWORD kServerVtableRva = 0x000339C7C;
	static const DWORD kGetFreeClientRva = 0x00005B00;
	static const DWORD kConnectionStartRva = 0x000CF7D0;
	static const DWORD kClientConnectRva = 0x0017C140;
	static const DWORD kClientDisconnectRva = 0x0017CC10;
	static const DWORD kClientVtableRva = 0x0003370DC;
	static const DWORD kSetSignonStateRva = 0x0017CC70;
	// The fixed constructor writes the primary table at client and the IClient
	// handler table at client+4.  Task 20 records both tables and the state read.
	static const unsigned int kClientGetPlayerSlotSlot = 14;
	static const unsigned int kClientIsConnectedSlot = 31;
	static const unsigned int kClientIsSpawnedSlot = 32;
	static const unsigned int kClientIsActiveSlot = 33;
	static const DWORD kClientGetPlayerSlotRva = 0x001C6360;
	static const DWORD kClientIsConnectedRva = 0x00048A10;
	static const DWORD kClientIsSpawnedRva = 0x00048A20;
	static const DWORD kClientIsActiveRva = 0x00048A30;
	static const unsigned int kClientSignonStateOffset = 0xE8;

	// The bound routine returns a server-owned slot pointer or NULL in EAX.
	// Its ECX/stack contract and scan behavior are recorded in Task 20.
	typedef void *(__thiscall *GetFreeClientFn)( void *server, netadr_t *address );
	typedef void (__thiscall *ConnectionStartFn)( void *handler, void *channel );
	typedef void (__thiscall *ClientConnectFn)( void *handler, const char *name,
		int userId, void *channel, bool fakePlayer, void *conVars );
	// Public IClient ordering corroborates slot 13.  The local target's
	// stack-only variadic ABI and plain return are verified in Task 20.
	typedef void (__cdecl *ClientDisconnectFn)( void *handler,
		const char *reason, ... );
	typedef bool (__thiscall *SetSignonStateFn)( void *client, int state,
		int spawnCount );
	typedef int (__thiscall *ClientGetPlayerSlotFn)( void *clientInterface );
	typedef bool (__thiscall *ClientStateFn)( void *clientInterface );
	static const unsigned int kClientDisconnectSlot = 13;

	struct RegistrationChannelAdapter
	{
		void **vtable;
	};

	static bool __fastcall RegistrationAdapterRegisterMessage( void *self, void *,
		void *message );
	static const char * __fastcall RegistrationAdapterGetAddress( void *self, void * );
	static float __fastcall RegistrationAdapterGetLatency( void *self, void *,
		int flow );
	static float __fastcall RegistrationAdapterGetAvgLatency( void *self, void *,
		int flow );
	static float __fastcall RegistrationAdapterGetAvgLoss( void *self, void *,
		int flow );
	static void __fastcall RegistrationAdapterSetDataRate( void *self, void *,
		float rate );
	static void __fastcall RegistrationAdapterSetCompressionMode( void *self, void *,
		bool enabled );
	static void __fastcall RegistrationAdapterSetTimeout( void *self, void *,
		float seconds, bool forceExact );
	static void __fastcall RegistrationAdapterSetFileTransmissionMode( void *self,
		void *, bool backgroundMode );
	static void __fastcall RegistrationAdapterSetMaxBufferSize( void *self, void *,
		bool reliable, int bytes, bool voice );
	static bool __fastcall RegistrationAdapterSendData( void *self, void *,
		void *messageBuffer, bool reliable );
	static int __fastcall RegistrationAdapterSendDatagram( void *self, void *,
		void *messageBuffer );
	static bool __fastcall RegistrationAdapterSendNetMsg( void *self, void *,
		void *message, bool forceReliable, bool voice );
	static void __fastcall RegistrationAdapterClear( void *self, void * );
	static void __fastcall RegistrationAdapterShutdown( void *self, void *,
		const char *reason );
	static int __fastcall RegistrationAdapterGetNumBitsWritten( void *self, void *,
		bool reliable );
	static bool __fastcall RegistrationAdapterIsOverflowed( void *self, void * );
	static bool __fastcall RegistrationAdapterIsLoopback( void *self, void * );
	static bool __fastcall RegistrationAdapterCanPacket( void *self, void * );
	static int __fastcall RegistrationAdapterGetSequenceNr( void *self, void *,
		int flow );
	static bool __fastcall RegistrationAdapterTransmit( void *self, void *,
		bool onlyReliable );
	static bool __fastcall RegistrationAdapterHasPendingReliableData( void *self,
		void * );
	static bool __fastcall RegistrationAdapterIsTimedOut( void *self, void * );
	static float __fastcall RegistrationAdapterGetTimeSinceLastReceived( void *self,
		void * );
	static bool __fastcall RegistrationAdapterIsRemoteDisconnected( void *self,
		void * );
	static int __fastcall RegistrationAdapterGetDropNumber( void *self, void * );
	static void __fastcall RegistrationAdapterSetRemoteFramerate( void *self, void *,
		float frameTime, float frameTimeStdDeviation );
	static void __fastcall RegistrationAdapterSetMaxRoutablePayloadSize( void *self,
		void *, int splitSize );

	// Public INetChannel declarations corroborate the named method contracts.
	// Task 20 records this engine's indices and call sites; unused entries are NULL.
	static void *s_registrationVtable[ 75 ] = { 0 };
	static RegistrationChannelAdapter s_registrationAdapter =
		{ s_registrationVtable };
	static bool s_registrationAdapterReady = false;
	static bool s_adapterShutdownLogged = false;
	static bool s_contextBound = false;
	// Set before ConnectionStart so later bind failures can release the
	// Source-owned client before the context is fully bound.
	static bool s_contextCreated = false;
	static bool s_contextTeardownInProgress = false;
	static bool s_sourceConnectCompleted = false;
	static bool s_sourceSignonFinalized = false;
	// Phase B is one-shot for each mapped Source session.  The attempted flag
	// also covers validation failures so a later frame cannot retry them.
	static bool s_sourceSignonAttempted = false;
	static bool s_sourceSignonInProgress = false;
	static void *s_contextClient = NULL;
	// These are logical Source-session observations, not transport-handle
	// counts.  A session becomes a real player only at ClientActive and remains
	// active until the mapped session reaches RemoveConnection.
	static bool s_hadRealPlayer = false;
	static unsigned int s_activeRealPlayerCount = 0;
	static bool s_sessionHadRealPlayer = false;
	static bool s_shutdownPending = false;
	static double s_shutdownDeadline = 0.0;
	static bool s_shutdownCommitted = false;
	static bool s_gameplayContextInitialized = false;
	static void *s_gameplayBoundEntity = NULL;
	static char s_gameplayMapName[ 64 ] = { 0 };
	static bool s_clientStateBindingLogged = false;
	static int s_lastClientSlot = -1;
	static int s_lastClientConnected = -1;
	static int s_lastClientSpawned = -1;
	static int s_lastClientActive = -1;
	static unsigned int s_traceType4Count = 0;
	static unsigned int s_traceType9Count = 0;

	// Forward declaration for the Source-owned teardown callback below.  A
	// Source client can invalidate its channel while the bridge is draining a
	// batch; removing the GNS map at that exact boundary makes the remaining
	// queued messages fail closed instead of entering a client with a null
	// m_NetChannel.
	static void RemoveConnection( const char *reason );
	static void BeginShutdownGracePeriod( void );
	static void EvaluateShutdown( void );

	static void ResetShutdownState( void )
	{
		s_hadRealPlayer = false;
		s_activeRealPlayerCount = 0;
		s_sessionHadRealPlayer = false;
		s_shutdownPending = false;
		s_shutdownDeadline = 0.0;
		s_shutdownCommitted = false;
	}

	static void LogContext( const char *message )
	{
		Warning( "[ASRD-GNS-SERVER-CONTEXT] %s\n", message );
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
		if ( nt->Signature != IMAGE_NT_SIGNATURE ||
			nt->FileHeader.Machine != IMAGE_FILE_MACHINE_I386 ||
			nt->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC ||
			nt->OptionalHeader.ImageBase != 0x10000000 )
			return false;

		return nt->FileHeader.TimeDateStamp == kEngineTimeDateStamp &&
			nt->OptionalHeader.SizeOfImage == kEngineSizeOfImage;
	}

	static bool IsEngineAddress( HMODULE engineModule, const void *address, size_t bytes )
	{
		if ( !engineModule || !address )
			return false;
		const BYTE *base = (const BYTE *)engineModule;
		const BYTE *begin = base;
		const BYTE *end = base + kEngineSizeOfImage;
		const BYTE *value = (const BYTE *)address;
		return value >= begin && value <= end && bytes <= (size_t)( end - value );
	}

	struct SourceClientStateSnapshot
	{
		int slot;
		int signonState;
		bool connected;
		bool spawned;
		bool active;
	};

	static bool ReadSourceClientState( SourceClientStateSnapshot *out )
	{
		if ( !out || !s_contextBound || !s_contextClient )
			return false;

		HMODULE engineModule = GetModuleHandleA( "engine.dll" );
		if ( !IsExpectedEngineImage( engineModule ) )
			return false;

		void *clientInterface = (BYTE *)s_contextClient + 4;
		void **vtable = *(void ***)clientInterface;
		const unsigned int maxSlot = kClientIsActiveSlot;
		if ( !vtable || !IsEngineAddress( engineModule, vtable,
			sizeof( void * ) * ( maxSlot + 1 ) ) )
			return false;

		const bool targetsMatch =
			vtable[ kClientGetPlayerSlotSlot ] ==
				(void *)( engineModule ? ( (BYTE *)engineModule + kClientGetPlayerSlotRva ) : NULL ) &&
			vtable[ kClientIsConnectedSlot ] ==
				(void *)( (BYTE *)engineModule + kClientIsConnectedRva ) &&
			vtable[ kClientIsSpawnedSlot ] ==
				(void *)( (BYTE *)engineModule + kClientIsSpawnedRva ) &&
			vtable[ kClientIsActiveSlot ] ==
				(void *)( (BYTE *)engineModule + kClientIsActiveRva );
		if ( !targetsMatch )
		{
			if ( !s_clientStateBindingLogged )
			{
				LogContextf( "client state resolve rejected slotTargets=%p/%p/%p/%p",
					vtable[ kClientGetPlayerSlotSlot ],
					vtable[ kClientIsConnectedSlot ],
					vtable[ kClientIsSpawnedSlot ],
					vtable[ kClientIsActiveSlot ] );
				s_clientStateBindingLogged = true;
			}
			return false;
		}
		s_clientStateBindingLogged = true;

		ClientGetPlayerSlotFn getPlayerSlot =
			(ClientGetPlayerSlotFn)(uintptr_t)vtable[ kClientGetPlayerSlotSlot ];
		ClientStateFn isConnected =
			(ClientStateFn)(uintptr_t)vtable[ kClientIsConnectedSlot ];
		ClientStateFn isSpawned =
			(ClientStateFn)(uintptr_t)vtable[ kClientIsSpawnedSlot ];
		ClientStateFn isActive =
			(ClientStateFn)(uintptr_t)vtable[ kClientIsActiveSlot ];

		out->slot = getPlayerSlot( clientInterface );
		out->signonState = *(int *)( (BYTE *)s_contextClient + kClientSignonStateOffset );
		out->connected = isConnected( clientInterface );
		out->spawned = isSpawned( clientInterface );
		out->active = isActive( clientInterface );
		if ( out->slot != s_lastClientSlot ||
			(out->connected ? 1 : 0) != s_lastClientConnected ||
			(out->spawned ? 1 : 0) != s_lastClientSpawned ||
			(out->active ? 1 : 0) != s_lastClientActive )
		{
			LogContextf( "client state slot=%d signon=%d connected=%u spawned=%u active=%u",
				out->slot, out->signonState, out->connected ? 1U : 0U,
				out->spawned ? 1U : 0U, out->active ? 1U : 0U );
			s_lastClientSlot = out->slot;
			s_lastClientConnected = out->connected ? 1 : 0;
			s_lastClientSpawned = out->spawned ? 1 : 0;
			s_lastClientActive = out->active ? 1 : 0;
		}
		return true;
	}

#if defined( GAME_DLL )
	static bool IsMappedSourceEdict( const void *sourceEdict )
	{
		if ( !sourceEdict || !s_contextBound || !s_contextClient ||
			!gpGlobals || !gpGlobals->pEdicts )
			return false;

		SourceClientStateSnapshot state = {};
		if ( !ReadSourceClientState( &state ) || state.slot < 0 ||
			state.slot + 1 >= MAX_EDICTS )
			return false;

		return sourceEdict == (const void *)( gpGlobals->pEdicts + state.slot + 1 );
	}
#endif

#if defined( GAME_DLL )
	static bool HasGameplaySpawnPoint( void )
	{
		return gEntList.FindEntityByClassname( NULL, "info_player_start" ) != NULL ||
			gEntList.FindEntityByClassname( NULL, "info_player_coop" ) != NULL ||
			gEntList.FindEntityByClassname( NULL, "info_player_start_team1" ) != NULL;
	}

	static void EnsureSourceGameplayContext( void )
	{
		SourceClientStateSnapshot state = {};
		if ( !ReadSourceClientState( &state ) || state.slot < 0 || !state.connected )
			return;

		const char *mapName = "";
		if ( gpGlobals && gpGlobals->mapname != NULL_STRING )
			mapName = STRING( gpGlobals->mapname );
		if ( !mapName[ 0 ] )
			return;

		if ( strncmp( s_gameplayMapName, mapName, sizeof( s_gameplayMapName ) - 1 ) != 0 )
		{
			strncpy( s_gameplayMapName, mapName, sizeof( s_gameplayMapName ) - 1 );
			s_gameplayMapName[ sizeof( s_gameplayMapName ) - 1 ] = '\0';
			s_gameplayContextInitialized = false;
			s_gameplayBoundEntity = NULL;
			LogContextf( "gameplay map boundary map=%s action=reset_source_player_binding",
				mapName );
		}

		if ( !HasGameplaySpawnPoint() )
			return;
		if ( !state.spawned )
			return;

		if ( !gpGlobals || !gpGlobals->pEdicts )
			return;
		const int entityIndex = state.slot + 1;
		if ( entityIndex <= 0 || entityIndex >= MAX_EDICTS )
			return;
		edict_t *playerEdict = gpGlobals->pEdicts + entityIndex;
		CBaseEntity *entity = GetContainingEntity( playerEdict );
		const bool hasPlayer = entity && entity->IsPlayer();
		if ( !hasPlayer )
			return;

		if ( !s_gameplayContextInitialized || s_gameplayBoundEntity != (void *)entity )
		{
			LogContextf( "gameplay player context observed map=%s slot=%d entity=%p action=engine_source_init",
				mapName, state.slot, entity );
			s_gameplayContextInitialized = true;
			s_gameplayBoundEntity = entity;
		}
	}
#else
	static void EnsureSourceGameplayContext( void )
	{
	}
#endif

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
		s_registrationVtable[ 1 ] =
			(void *)(uintptr_t)&RegistrationAdapterGetAddress;
		s_registrationVtable[ 9 ] =
			(void *)(uintptr_t)&RegistrationAdapterGetLatency;
		s_registrationVtable[ 10 ] =
			(void *)(uintptr_t)&RegistrationAdapterGetAvgLatency;
		s_registrationVtable[ 11 ] =
			(void *)(uintptr_t)&RegistrationAdapterGetAvgLoss;
		s_registrationVtable[ 28 ] =
			(void *)(uintptr_t)&RegistrationAdapterSetDataRate;
		s_registrationVtable[ 29 ] =
			(void *)(uintptr_t)&RegistrationAdapterRegisterMessage;
		s_registrationVtable[ 62 ] =
			(void *)(uintptr_t)&RegistrationAdapterSetCompressionMode;
		s_registrationVtable[ 32 ] =
			(void *)(uintptr_t)&RegistrationAdapterSetTimeout;
		s_registrationVtable[ 61 ] =
			(void *)(uintptr_t)&RegistrationAdapterSetFileTransmissionMode;
		s_registrationVtable[ 64 ] =
			(void *)(uintptr_t)&RegistrationAdapterSetMaxBufferSize;
		s_registrationVtable[ 42 ] =
			(void *)(uintptr_t)&RegistrationAdapterSendData;
		s_registrationVtable[ 47 ] =
			(void *)(uintptr_t)&RegistrationAdapterSendDatagram;
		s_registrationVtable[ 41 ] =
			(void *)(uintptr_t)&RegistrationAdapterSendNetMsg;
		s_registrationVtable[ 36 ] =
			(void *)(uintptr_t)&RegistrationAdapterClear;
		s_registrationVtable[ 37 ] =
			(void *)(uintptr_t)&RegistrationAdapterShutdown;
		s_registrationVtable[ 66 ] =
			(void *)(uintptr_t)&RegistrationAdapterGetNumBitsWritten;
		s_registrationVtable[ 58 ] =
			(void *)(uintptr_t)&RegistrationAdapterIsOverflowed;
		s_registrationVtable[ 6 ] =
			(void *)(uintptr_t)&RegistrationAdapterIsLoopback;
		s_registrationVtable[ 57 ] =
			(void *)(uintptr_t)&RegistrationAdapterCanPacket;
		s_registrationVtable[ 17 ] =
			(void *)(uintptr_t)&RegistrationAdapterGetSequenceNr;
		s_registrationVtable[ 48 ] =
			(void *)(uintptr_t)&RegistrationAdapterTransmit;
		s_registrationVtable[ 60 ] =
			(void *)(uintptr_t)&RegistrationAdapterHasPendingReliableData;
		s_registrationVtable[ 59 ] =
			(void *)(uintptr_t)&RegistrationAdapterIsTimedOut;
		s_registrationVtable[ 22 ] =
			(void *)(uintptr_t)&RegistrationAdapterGetTimeSinceLastReceived;
		s_registrationVtable[ 74 ] =
			(void *)(uintptr_t)&RegistrationAdapterIsRemoteDisconnected;
		s_registrationVtable[ 51 ] =
			(void *)(uintptr_t)&RegistrationAdapterGetDropNumber;
		s_registrationVtable[ 68 ] =
			(void *)(uintptr_t)&RegistrationAdapterSetRemoteFramerate;
		s_registrationVtable[ 69 ] =
			(void *)(uintptr_t)&RegistrationAdapterSetMaxRoutablePayloadSize;
		s_registrationAdapter.vtable = s_registrationVtable;
		s_registrationAdapterReady = true;
		LogContextf( "registration adapter ready self=%p slots=%u",
			&s_registrationAdapter, 75U );
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

		return "GNS";
	}

	static float __fastcall RegistrationAdapterGetLatency( void *self, void *,
		int flow )
	{
		if ( self != &s_registrationAdapter )
		{
			LogContextf( "adapter unexpected GetLatency self=%p expected=%p flow=%d",
				self, &s_registrationAdapter, flow );
			return 0.0f;
		}

		// No wrapper latency ABI is available, so zero remains the adapter policy.
		// Task 20 records the slot contract and this limitation.
		return 0.0f;
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

		return 0.0f;
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

	}

	static void __fastcall RegistrationAdapterSetCompressionMode( void *self, void *,
		bool enabled )
	{
		if ( self != &s_registrationAdapter )
		{
			LogContextf( "adapter unexpected SetCompressionMode self=%p expected=%p enabled=%u",
				self, &s_registrationAdapter, enabled ? 1U : 0U );
			return;
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

	}

	static void __fastcall RegistrationAdapterSetFileTransmissionMode( void *self,
		void *, bool backgroundMode )
	{
		if ( self != &s_registrationAdapter )
		{
			LogContextf( "adapter unexpected SetFileTransmissionMode self=%p expected=%p background=%u",
				self, &s_registrationAdapter, backgroundMode ? 1U : 0U );
			return;
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

	}

	static bool __fastcall RegistrationAdapterSendData( void *self, void *,
		void *messageBuffer, bool reliable )
	{
		if ( self != &s_registrationAdapter )
		{
			LogContextf( "adapter unexpected SendData self=%p expected=%p buffer=%p reliable=%u",
				self, &s_registrationAdapter, messageBuffer, reliable ? 1U : 0U );
			return false;
		}

		if ( !messageBuffer )
		{
			LogContext( "adapter SendData rejected null bf_write" );
			return false;
		}

		bf_write *writer = static_cast<bf_write *>( messageBuffer );
		const int bitLength = writer->GetNumBitsWritten();
		const int payloadBytes = writer->GetNumBytesWritten();
		if ( writer->IsOverflowed() || bitLength < 0 || payloadBytes < 0 ||
			(unsigned int)bitLength > ASRD_GNS_MESSAGE_MAX_ENVELOPE_BYTES * 8U ||
			(unsigned int)payloadBytes > ASRD_GNS_MESSAGE_MAX_ENVELOPE_BYTES )
		{
			LogContextf( "adapter SendData rejected raw bf_write buffer=%p bitlen=%d bytes=%d overflow=%u",
				messageBuffer, bitLength, payloadBytes,
				writer->IsOverflowed() ? 1U : 0U );
			return false;
		}

		const ASRD_GNS_Connection connection = ASRD_GNS_ServerConnection();
		const bool sent = ASRD_GNS_MessageBridgeAppendServerRaw( connection,
			writer->GetData(), (unsigned int)bitLength, (unsigned int)payloadBytes,
			reliable );
		return sent;
	}

	static int __fastcall RegistrationAdapterSendDatagram( void *self, void *,
		void *messageBuffer )
	{
		if ( self != &s_registrationAdapter )
		{
			LogContextf( "adapter unexpected SendDatagram self=%p expected=%p buffer=%p",
				self, &s_registrationAdapter, messageBuffer );
			return 0;
		}

		const ASRD_GNS_Connection connection = ASRD_GNS_ServerConnection();
		int bitLength = 0;
		int payloadBytes = 0;
		bool payloadAppended = true;
		if ( messageBuffer )
		{
			bf_write *writer = static_cast<bf_write *>( messageBuffer );
			bitLength = writer->GetNumBitsWritten();
			payloadBytes = writer->GetNumBytesWritten();
			const int maxPayloadBits =
				ASRD_GNS_MESSAGE_MAX_ENVELOPE_BYTES * 8;
			if ( writer->IsOverflowed() || bitLength < 0 || payloadBytes < 0 ||
				bitLength > maxPayloadBits ||
				payloadBytes > ASRD_GNS_MESSAGE_MAX_ENVELOPE_BYTES ||
				bitLength > payloadBytes * 8 )
			{
				LogContextf( "adapter SendDatagram rejected raw bf_write buffer=%p bitlen=%d bytes=%d overflow=%u",
					messageBuffer, bitLength, payloadBytes,
					writer->IsOverflowed() ? 1U : 0U );
				return 0;
			}

			if ( bitLength > 0 )
			{
				// SendDatagram is one indivisible snapshot Engine block. Its
				// reliability and provenance are fixed by the confirmed boundary.
				payloadAppended = ASRD_GNS_MessageBridgeAppendServerRaw(
					connection, writer->GetData(),
					(unsigned int)bitLength, (unsigned int)payloadBytes,
					ASRD_GNS_BLOCK_RELIABILITY_UNRELIABLE,
					ASRD_GNS_PROVENANCE_SNAPSHOT );
				if ( !payloadAppended )
				{
					LogContextf( "adapter SendDatagram rejected raw bf_write buffer=%p bitlen=%d bytes=%d reason=append_raw_failed",
						messageBuffer, bitLength, payloadBytes );
					return 0;
				}
			}
		}

		const bool flushed =
			ASRD_GNS_MessageBridgeSealAndFlushServerUpdate( connection );
		return flushed ? 1 : 0;
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
		const ASRD_GNS_Connection connection = ASRD_GNS_ServerConnection();
		const bool effectiveReliable = netMessage->IsReliable() || forceReliable;
		const ASRD_GNS_BlockProvenance provenance = voice
			? ASRD_GNS_PROVENANCE_VOICE
			: ASRD_GNS_PROVENANCE_SEND_NETMSG;
		const bool sent = ASRD_GNS_MessageBridgeAppendServer( connection,
			*netMessage, effectiveReliable, voice, provenance );
		return sent;
	}

	static void __fastcall RegistrationAdapterClear( void *self, void * )
	{
		if ( self != &s_registrationAdapter )
		{
			LogContextf( "adapter unexpected Clear self=%p expected=%p",
				self, &s_registrationAdapter );
			return;
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
		if ( !s_adapterShutdownLogged )
		{
			s_adapterShutdownLogged = true;
			LogContextf( "adapter Shutdown reason=%s action=source_teardown",
				reason ? reason : "<null>" );
		}

		if ( s_connection.connection != ASRD_GNS_CONNECTION_INVALID )
		{
			const ASRD_GNS_Connection connection = s_connection.connection;
			LogContextf( "adapter Shutdown closes mapped handle=%lu action=source_teardown",
				(unsigned long)connection );
			ASRD_GNS_Close( connection );
			RemoveConnection( "source_channel_shutdown" );
		}
	}

	static int __fastcall RegistrationAdapterGetNumBitsWritten( void *self,
		void *, bool reliable )
	{
		if ( self != &s_registrationAdapter )
		{
			LogContextf( "adapter unexpected GetNumBitsWritten self=%p expected=%p reliable=%u",
				self, &s_registrationAdapter, reliable ? 1U : 0U );
			return 0;
		}
		return (int)ASRD_GNS_MessageBridgeServerPendingBits(
			ASRD_GNS_ServerConnection() );
	}

	static bool __fastcall RegistrationAdapterIsOverflowed( void *self, void * )
	{
		if ( self != &s_registrationAdapter )
		{
			LogContextf( "adapter unexpected IsOverflowed self=%p expected=%p",
				self, &s_registrationAdapter );
			return true;
		}

		return false;
	}

	static bool __fastcall RegistrationAdapterIsLoopback( void *self, void * )
	{
		if ( self != &s_registrationAdapter )
		{
			LogContextf( "adapter unexpected IsLoopback self=%p expected=%p",
				self, &s_registrationAdapter );
			return true;
		}

		return false;
	}

	static bool __fastcall RegistrationAdapterCanPacket( void *self, void * )
	{
		if ( self != &s_registrationAdapter )
		{
			LogContextf( "adapter unexpected CanPacket self=%p expected=%p",
				self, &s_registrationAdapter );
			return false;
		}

		return true;
	}

	static int __fastcall RegistrationAdapterGetSequenceNr( void *self, void *,
		int flow )
	{
		if ( self != &s_registrationAdapter )
		{
			LogContextf( "adapter unexpected GetSequenceNr self=%p expected=%p flow=%d",
				self, &s_registrationAdapter, flow );
			return 0;
		}

		// Preserve the nonzero compatibility sentinel; this adapter maintains no
		// legacy sequence or ACK state.  Task 20 records the resulting assumption.
		return 1;
	}

	static bool __fastcall RegistrationAdapterTransmit( void *self, void *,
		bool onlyReliable )
	{
		if ( self != &s_registrationAdapter )
		{
			LogContextf( "adapter unexpected Transmit self=%p expected=%p reliable=%u",
				self, &s_registrationAdapter, onlyReliable ? 1U : 0U );
			return false;
		}

		const bool flushed =
			ASRD_GNS_MessageBridgeSealAndFlushServerUpdate(
				ASRD_GNS_ServerConnection() );
		return flushed;
	}

	static bool __fastcall RegistrationAdapterHasPendingReliableData( void *self,
		void * )
	{
		if ( self != &s_registrationAdapter )
		{
			LogContextf( "adapter unexpected HasPendingReliableData self=%p expected=%p",
				self, &s_registrationAdapter );
			return true;
		}

		const bool pending = ASRD_GNS_MessageBridgeServerHasPending(
			ASRD_GNS_ServerConnection() );
		return pending;
	}

	static bool __fastcall RegistrationAdapterIsTimedOut( void *self, void * )
	{
		if ( self != &s_registrationAdapter )
		{
			LogContextf( "adapter unexpected IsTimedOut self=%p expected=%p",
				self, &s_registrationAdapter );
			return true;
		}

		return false;
	}

	static float __fastcall RegistrationAdapterGetTimeSinceLastReceived( void *self,
		void * )
	{
		if ( self != &s_registrationAdapter )
		{
			LogContextf( "adapter unexpected GetTimeSinceLastReceived self=%p expected=%p",
				self, &s_registrationAdapter );
			return 0.0f;
		}

		return 0.0f;
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

		return false;
	}

	static int __fastcall RegistrationAdapterGetDropNumber( void *self, void * )
	{
		if ( self != &s_registrationAdapter )
		{
			LogContextf( "adapter unexpected GetDropNumber self=%p expected=%p",
				self, &s_registrationAdapter );
			return 0;
		}

		const int dropped = ASRD_GNS_ServerGetCurrentMoveDrop(
			&s_serverConnection );
		return dropped;
	}

	static void __fastcall RegistrationAdapterSetRemoteFramerate( void *self, void *,
		float frameTime, float frameTimeStdDeviation )
	{
		if ( self != &s_registrationAdapter )
		{
			LogContextf( "adapter unexpected SetRemoteFramerate self=%p expected=%p frame=%f stddev=%f",
				self, &s_registrationAdapter, frameTime, frameTimeStdDeviation );
			return;
		}

	}

	static void __fastcall RegistrationAdapterSetMaxRoutablePayloadSize( void *self,
		void *, int splitSize )
	{
		if ( self != &s_registrationAdapter )
		{
			LogContextf( "adapter unexpected SetMaxRoutablePayloadSize self=%p expected=%p splitSize=%d",
				self, &s_registrationAdapter, splitSize );
			return;
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

	static bool BindSourceServerContext( void )
	{
		if ( s_contextBound )
		{
			LogContextf( "context already bound client=%p registryCount=%u sourceConnectCompleted=%u sourceSignonFinalized=%u",
				s_contextClient, ASRD_GNS_MessageRegistryCount(),
				s_sourceConnectCompleted ? 1U : 0U,
				s_sourceSignonFinalized ? 1U : 0U );
			return true;
		}
		const ASRD_GNS_Connection bindConnection = s_connection.connection;
		if ( sizeof( void * ) != 4 )
		{
			LogContext( "context bind rejected: non-x86 process" );
			return false;
		}

		HMODULE engineModule = GetModuleHandleA( "engine.dll" );
		if ( !IsExpectedEngineImage( engineModule ) )
		{
			LogContext( "context bind rejected: engine image mismatch" );
			return false;
		}

		BYTE *base = (BYTE *)engineModule;
		void *server = (void *)( base + kServerGlobalRva );
		if ( !IsEngineAddress( engineModule, server, sizeof( void * ) ) )
		{
			LogContext( "context bind rejected: server global outside image" );
			return false;
		}
		void *serverVtable = *(void **)server;
		void *expectedServerVtable = (void *)( base + kServerVtableRva );
		LogContextf( "server resolve global=%p vtable=%p expected=%p",
			server, serverVtable, expectedServerVtable );
		if ( serverVtable != expectedServerVtable )
		{
			LogContext( "context bind rejected: server vtable mismatch" );
			return false;
		}

		if ( !EnsureRegistrationAdapter() )
			return false;

		// A zero endpoint is intentional here: this is only the Source slot
		// allocator input.  GNS remains the connection identity and transport;
		// no client address is inferred or connected to from this value.
		netadr_t address;
		memset( &address, 0, sizeof( address ) );
		address.type = NA_IP;
		GetFreeClientFn getFreeClient = (GetFreeClientFn)(uintptr_t)( base + kGetFreeClientRva );
		if ( !IsEngineAddress( engineModule, (void *)getFreeClient, 1 ) )
		{
			LogContext( "context bind rejected: GetFreeClient outside image" );
			return false;
		}

		void *client = getFreeClient( server, &address );
		LogContextf( "slot result client=%p registryBefore=%u",
			client, ASRD_GNS_MessageRegistryCount() );
		if ( !client )
		{
			LogContext( "context bind failed: no free Source client slot" );
			return false;
		}

		// Constructor bytes install IClient at client+4, with ConnectionStart at
		// slot 1.  Task 20 records both writes and the target bytes.
		void *handler = (BYTE *)client + 4;
		if ( !IsEngineAddress( engineModule, *(void **)handler, sizeof( void * ) * 2 ) )
		{
			LogContextf( "context bind failed: handler vtable invalid client=%p handler=%p",
				client, handler );
			return false;
		}
		void **handlerVtable = *(void ***)handler;
		void *connectionStartAddress = handlerVtable[ 1 ];
		void *expectedConnectionStart = (void *)( base + kConnectionStartRva );
		LogContextf( "ConnectionStart resolve handler=%p vtable=%p target=%p expected=%p",
			handler, handlerVtable, connectionStartAddress, expectedConnectionStart );
		if ( connectionStartAddress != expectedConnectionStart )
		{
			LogContext( "context bind rejected: ConnectionStart target mismatch" );
			return false;
		}

		// ConnectionStart creates the Source context and may synchronously invoke
		// the registration adapter.  Publish the client before that call so any
		// later failure can tear the context down even before s_contextBound.
		s_contextClient = client;
		s_contextCreated = true;
		const unsigned int captureBefore =
			ASRD_GNS_MessageRegistryCaptureCount();
		ConnectionStartFn connectionStart =
			(ConnectionStartFn)(uintptr_t)connectionStartAddress;
		connectionStart( handler, &s_registrationAdapter );
		const unsigned int captureAfter =
			ASRD_GNS_MessageRegistryCaptureCount();
		if ( s_connection.connection != bindConnection ||
			!s_contextCreated || s_contextClient != client )
		{
			LogContext( "context bind aborted: ConnectionStart changed lifecycle state" );
			return false;
		}
		LogContextf( "ConnectionStart complete client=%p captureBefore=%u captureAfter=%u",
			client, captureBefore, captureAfter );
		if ( captureAfter <= captureBefore )
		{
			LogContext( "context bind failed: ConnectionStart captured no messages" );
			return false;
		}

		// Public IClient ordering corroborates slot 10 and five stack arguments;
		// the local target and NULL cvar-vector path are verified in Task 20.
		void **clientVtable = *(void ***)handler;
		if ( !clientVtable || !IsEngineAddress( engineModule, clientVtable,
			sizeof( void * ) * 11 ) )
		{
			LogContextf( "context bind failed: client handler vtable too short handler=%p",
				handler );
			return false;
		}
		void *clientConnectAddress = clientVtable[ 10 ];
		void *expectedClientConnect = (void *)( base + kClientConnectRva );
		LogContextf( "ClientConnect resolve handler=%p slot=10 target=%p expected=%p",
			handler, clientConnectAddress, expectedClientConnect );
		if ( clientConnectAddress != expectedClientConnect )
		{
			LogContext( "context bind rejected: ClientConnect target mismatch" );
			return false;
		}

		ClientConnectFn clientConnect =
			(ClientConnectFn)(uintptr_t)clientConnectAddress;
		LogContextf( "ClientConnect begin client=%p handler=%p registry=%u",
			client, handler, ASRD_GNS_MessageRegistryCount() );
		clientConnect( handler, "ASRD_GNS", 1, &s_registrationAdapter, false, NULL );
		LogContextf( "ClientConnect complete client=%p registry=%u",
			client, ASRD_GNS_MessageRegistryCount() );
		if ( s_connection.connection != bindConnection ||
			!s_contextCreated || s_contextClient != client )
		{
			LogContext( "context bind aborted: ClientConnect changed lifecycle state" );
			return false;
		}

		s_contextClient = client;
		s_contextBound = true;
		s_sourceConnectCompleted = true;
		s_sourceSignonFinalized = false;
		s_sourceSignonAttempted = false;
		s_sourceSignonInProgress = false;
		LogContextf( "context bind success client=%p registrationCount=%u sourceConnectCompleted=1 sourceSignonFinalized=0",
			client, ASRD_GNS_MessageRegistryCount() );
		return true;
	}

	static const char *StateName( ASRD_GNS_ServerConnectionState state )
	{
		switch ( state )
		{
		case ASRD_GNS_SERVER_PENDING: return "pending";
		case ASRD_GNS_SERVER_CONNECTED: return "connected";
		case ASRD_GNS_SERVER_FAILED: return "failed";
		case ASRD_GNS_SERVER_CLOSED: return "closed";
		default: return "disconnected";
		}
	}

	static bool HasConnection( void )
	{
		return s_connection.connection != ASRD_GNS_CONNECTION_INVALID;
	}

	static bool ShutdownWhenEmptyEnabled( void )
	{
#if defined( GAME_DLL )
		return rd_server_shutdown_when_empty.GetBool();
#else
		return false;
#endif
	}

	static void CancelShutdownGracePeriod( const char *reason )
	{
		if ( s_shutdownPending )
		{
			Warning( "[ASRD-GNS-SERVER] shutdown grace cancelled reason=%s\n",
				reason ? reason : "unspecified" );
		}
		s_shutdownPending = false;
		s_shutdownDeadline = 0.0;
	}

	static void CommitShutdown( void )
	{
		if ( s_shutdownCommitted )
			return;

		// Lock the state before touching the listener or any mapped session.  A
		// callback or queued event observed during teardown must fail closed and
		// cannot reopen the listener.
		s_shutdownCommitted = true;
		s_shutdownPending = false;
		s_shutdownDeadline = 0.0;
		Warning( "[ASRD-GNS-SERVER] shutdown grace elapsed had_real_player=%u active_real_players=%u action=commit\n",
			s_hadRealPlayer ? 1U : 0U, s_activeRealPlayerCount );

		// ServerShutdown owns the complete server-side GNS teardown: bridge
		// queues, Source mapping/context, pending/connected wrapper sessions,
		// listener, and the wrapper itself.
		ASRD_GNS_ServerShutdown();
#if defined( GAME_DLL )
		if ( engine )
		{
			// Reuse the engine's normal command/HostState shutdown path.  This is
			// intentionally not ExitProcess.
			engine->ServerCommand( "quit\n" );
			Warning( "[ASRD-GNS-SERVER] shutdown action=normal_quit_command\n" );
		}
		else
		{
			Warning( "[ASRD-GNS-SERVER] shutdown action=normal_quit_command_failed reason=no_engine\n" );
		}
#else
		// The lifecycle source is shared with the client build, where engine is
		// IVEngineClient and has no server-command API.  Shutdown is server-only.
		Warning( "[ASRD-GNS-SERVER] shutdown action=normal_quit_command_skipped reason=client_build\n" );
#endif
	}

	static void BeginShutdownGracePeriod( void )
	{
		if ( !s_initialized || s_shutdownCommitted || !s_hadRealPlayer ||
			s_activeRealPlayerCount != 0 || !ShutdownWhenEmptyEnabled() )
			return;
		if ( s_shutdownPending )
			return;

		const double now = Plat_FloatTime();
		s_shutdownPending = true;
		s_shutdownDeadline = now + 60.0;
		Warning( "[ASRD-GNS-SERVER] shutdown grace started active_real_players=0 deadline=%.3f duration=60.000\n",
			s_shutdownDeadline );
	}

	static void EvaluateShutdown( void )
	{
		if ( !s_initialized || !s_shutdownPending || s_shutdownCommitted )
			return;

		if ( !ShutdownWhenEmptyEnabled() )
		{
			CancelShutdownGracePeriod( "policy_disabled" );
			return;
		}
		if ( s_activeRealPlayerCount != 0 || !s_hadRealPlayer )
		{
			CancelShutdownGracePeriod( "real_player_present" );
			return;
		}
		if ( Plat_FloatTime() < s_shutdownDeadline )
			return;

		CommitShutdown();
	}

	static void SetState( ASRD_GNS_ServerConnectionState state, int reason )
	{
		const ASRD_GNS_ServerConnectionState previous = s_connection.state;
		s_connection.state = state;
		s_connection.reason = reason;
		if ( previous != state || state == ASRD_GNS_SERVER_FAILED ||
			 state == ASRD_GNS_SERVER_CLOSED )
		{
			Warning( "[ASRD-GNS-SERVER] state %s -> %s handle=%lu reason=%d\n",
				StateName( previous ), StateName( state ),
				(unsigned long)s_connection.connection, reason );
		}
	}

	static void RemoveConnection( const char *reason )
	{
		if ( !HasConnection() )
			return;
		const bool removedRealPlayer = s_sessionHadRealPlayer &&
			s_activeRealPlayerCount != 0;
		Warning( "[ASRD-GNS-SERVER] map remove handle=%lu reason=%s\n",
			(unsigned long)s_connection.connection, reason ? reason : "unspecified" );
		s_connection.connection = ASRD_GNS_CONNECTION_INVALID;
		s_connection.state = ASRD_GNS_SERVER_DISCONNECTED;
		s_connection.reason = 0;
		ResetCompatibilityState( &s_connection );
		s_contextBound = false;
		s_contextCreated = false;
		s_sourceConnectCompleted = false;
		s_sourceSignonFinalized = false;
		s_sourceSignonAttempted = false;
		s_sourceSignonInProgress = false;
		s_contextClient = NULL;
		s_activeRealPlayerCount = 0;
		s_sessionHadRealPlayer = false;
		s_gameplayContextInitialized = false;
		s_gameplayBoundEntity = NULL;
		s_gameplayMapName[ 0 ] = '\0';
		s_lastClientSlot = -1;
		s_lastClientConnected = -1;
		s_lastClientSpawned = -1;
		s_lastClientActive = -1;
		s_clientStateBindingLogged = false;
		s_traceType4Count = 0;
		s_traceType9Count = 0;
		if ( removedRealPlayer )
			BeginShutdownGracePeriod();
	}

	static bool TeardownSourceServerContext( const char *reason )
	{
		if ( s_contextTeardownInProgress )
			return true;
		if ( !s_contextCreated || !s_contextClient )
		{
			LogContextf( "ClientDisconnect skipped reason=no_source_context event=%s",
				reason ? reason : "<null>" );
			return false;
		}

		HMODULE engineModule = GetModuleHandleA( "engine.dll" );
		if ( !IsExpectedEngineImage( engineModule ) )
		{
			LogContextf( "ClientDisconnect skipped reason=engine_image_mismatch event=%s",
				reason ? reason : "<null>" );
			return false;
		}

		void *handler = (BYTE *)s_contextClient + 4;
		// The CBaseClient object is engine-owned heap memory, not part of the
		// engine image.  Only its vtable and resolved target are image-bound.
		if ( !handler )
		{
			LogContextf( "ClientDisconnect skipped reason=handler_invalid client=%p",
				s_contextClient );
			return false;
		}

		void **handlerVtable = *(void ***)handler;
		if ( !handlerVtable || !IsEngineAddress( engineModule, handlerVtable,
			sizeof( void * ) * ( kClientDisconnectSlot + 1 ) ) )
		{
			LogContextf( "ClientDisconnect skipped reason=handler_vtable_invalid client=%p handler=%p",
				s_contextClient, handler );
			return false;
		}

		void *disconnectAddress = handlerVtable[ kClientDisconnectSlot ];
		void *expectedDisconnect =
			(void *)( (BYTE *)engineModule + kClientDisconnectRva );
		LogContextf( "ClientDisconnect resolve client=%p handler=%p slot=%u target=%p expected=%p",
			s_contextClient, handler, kClientDisconnectSlot, disconnectAddress,
			expectedDisconnect );
		if ( !IsEngineAddress( engineModule, disconnectAddress, 1 ) )
		{
			LogContext( "ClientDisconnect skipped reason=target_outside_engine" );
			return false;
		}
		if ( disconnectAddress != expectedDisconnect )
		{
			LogContext( "ClientDisconnect skipped reason=target_mismatch" );
			return false;
		}

		ClientDisconnectFn disconnect =
			(ClientDisconnectFn)(uintptr_t)disconnectAddress;
		void *client = s_contextClient;
		s_contextTeardownInProgress = true;
		LogContextf( "ClientDisconnect begin client=%p event=%s action=source_teardown",
			client, reason ? reason : "<null>" );
		disconnect( handler, reason ? reason : "GNS remote close" );
		s_contextTeardownInProgress = false;
		LogContextf( "ClientDisconnect complete client=%p event=%s",
			client, reason ? reason : "<null>" );
		return true;
	}

	static void AbortMappedConnection( ASRD_GNS_Connection connection,
		const char *reason, int stateReason )
	{
		// A Source callback may have already cleared the mapping while the
		// caller is unwinding.  Never write a terminal state onto INVALID or
		// touch a newer session in that case.
		if ( connection == ASRD_GNS_CONNECTION_INVALID || !HasConnection() ||
			s_connection.connection != connection )
			return;

		SetState( ASRD_GNS_SERVER_FAILED, stateReason );
		s_sourceSignonFinalized = false;
		s_sourceSignonInProgress = false;
		TeardownSourceServerContext( reason );
		if ( HasConnection() && s_connection.connection == connection )
		{
			ASRD_GNS_Close( connection );
			RemoveConnection( reason );
		}
	}

	static void RejectIncoming( ASRD_GNS_Connection connection, const char *reason )
	{
		Warning( "[ASRD-GNS-SERVER] incoming reject handle=%lu reason=%s\n",
			(unsigned long)connection, reason ? reason : "unspecified" );
		if ( connection != ASRD_GNS_CONNECTION_INVALID )
			ASRD_GNS_Close( connection );
	}

	static void HandleIncoming( ASRD_GNS_Connection connection )
	{
		if ( connection == ASRD_GNS_CONNECTION_INVALID )
		{
			Warning( "[ASRD-GNS-SERVER] incoming reject reason=invalid_handle\n" );
			return;
		}
		if ( s_shutdownCommitted )
		{
			RejectIncoming( connection, "shutdown_committed" );
			return;
		}

		if ( HasConnection() )
		{
			if ( s_connection.connection == connection )
			{
				Warning( "[ASRD-GNS-SERVER] incoming duplicate handle=%lu action=ignore\n",
					(unsigned long)connection );
				return;
			}
			RejectIncoming( connection, "single_client_already_mapped" );
			return;
		}

		s_connection.connection = connection;
		s_connection.state = ASRD_GNS_SERVER_PENDING;
		s_connection.reason = 0;
		ResetCompatibilityState( &s_connection );
		Warning( "[ASRD-GNS-SERVER] map add handle=%lu state=pending\n",
			(unsigned long)connection );

		// This is deliberately called only from the engine-thread frame drain.
		if ( !ASRD_GNS_AcceptConnection( connection ) )
		{
			SetState( ASRD_GNS_SERVER_FAILED, 0 );
			ASRD_GNS_Close( connection );
			RemoveConnection( "accept_failed" );
			return;
		}

		Warning( "[ASRD-GNS-SERVER] map accept pending handle=%lu\n",
			(unsigned long)connection );

		// The accepted GNS handle now owns transport/lifecycle.  Source still
		// needs its existing client/session context and message initialization;
		// bind only that minimal capability and fail closed if the fixed engine
		// evidence does not match.
		if ( !BindSourceServerContext() )
		{
			AbortMappedConnection( connection, "source_context_bind_failed", 0 );
			return;
		}
	}

	static void HandleConnected( ASRD_GNS_Connection connection )
	{
		if ( !HasConnection() || s_connection.connection != connection )
		{
			Warning( "[ASRD-GNS-SERVER] connected reject handle=%lu reason=unmapped\n",
				(unsigned long)connection );
			RejectIncoming( connection, "connected_without_pending_map" );
			return;
		}
		if ( s_connection.state != ASRD_GNS_SERVER_PENDING )
		{
			Warning( "[ASRD-GNS-SERVER] connected ignore handle=%lu reason=state_not_pending state=%s\n",
				(unsigned long)connection, StateName( s_connection.state ) );
			return;
		}
		const int configureResult = ASRD_GNS_ConfigureLanes( connection );
		if ( configureResult != ASRD_GNS_RESULT_OK )
		{
			Warning( "[ASRD-GNS-SERVER] connected reject handle=%lu reason=lane_config_failed result=%d\n",
				(unsigned long)connection, configureResult );
			AbortMappedConnection( connection, "lane_config_failed", configureResult );
			return;
		}
		SetState( ASRD_GNS_SERVER_CONNECTED, 0 );
	}

	static void HandleTerminal( const ASRD_GNS_ConnectionEvent &event )
	{
		if ( !HasConnection() || s_connection.connection != event.connection )
		{
			Warning( "[ASRD-GNS-SERVER] terminal orphan handle=%lu state=%d reason=%d\n",
				(unsigned long)event.connection, event.state, event.reason );
			if ( event.connection != ASRD_GNS_CONNECTION_INVALID )
				ASRD_GNS_Close( event.connection );
			return;
		}

		const ASRD_GNS_ServerConnectionState state =
			event.state == ASRD_GNS_CONNECTION_EVENT_CLOSED
				? ASRD_GNS_SERVER_CLOSED : ASRD_GNS_SERVER_FAILED;
		SetState( state, event.reason );
		const char *reason =
			state == ASRD_GNS_SERVER_CLOSED ? "GNS remote close" : "GNS connection failed";
		TeardownSourceServerContext( reason );
		// ClientDisconnect normally reaches the adapter's Source teardown path,
		// which closes/removes the mapped handle synchronously.  Keep a
		// fail-closed fallback for an invalid/mismatched vtable or a context that
		// was never bound.
		if ( HasConnection() )
		{
			ASRD_GNS_Close( event.connection );
			RemoveConnection( state == ASRD_GNS_SERVER_CLOSED ? "remote_close" : "connect_failed" );
		}
	}

	static void FinalizeSourceSignon( void )
	{
		if ( !s_contextBound || !s_contextCreated || !s_contextClient ||
			!s_sourceConnectCompleted || s_sourceSignonFinalized ||
			s_sourceSignonAttempted || s_sourceSignonInProgress )
			return;

		const ASRD_GNS_Connection connection = s_connection.connection;
		s_sourceSignonAttempted = true;
		s_sourceSignonInProgress = true;

		HMODULE engineModule = GetModuleHandleA( "engine.dll" );
		if ( !IsExpectedEngineImage( engineModule ) )
		{
			LogContext( "signon finalize rejected: engine image mismatch" );
			AbortMappedConnection( connection, "source_signon_engine_image_mismatch", 0 );
			return;
		}

		void *client = s_contextClient;
		void **clientPrimaryVtable = *(void ***)client;
		void *expectedClientVtable = (void *)( (BYTE *)engineModule + kClientVtableRva );
		if ( !clientPrimaryVtable || !IsEngineAddress( engineModule,
			clientPrimaryVtable, sizeof( void * ) * 21 ) )
		{
			LogContextf( "signon finalize rejected: client primary vtable invalid client=%p",
				client );
			AbortMappedConnection( connection, "source_signon_client_vtable_invalid", 0 );
			return;
		}
		LogContextf( "signon finalize resolve client=%p vtable=%p expected=%p slot=20 target=%p expectedTarget=%p",
			client, clientPrimaryVtable, expectedClientVtable,
			clientPrimaryVtable[ 20 ],
			(void *)( (BYTE *)engineModule + kSetSignonStateRva ) );
		if ( clientPrimaryVtable != (void **)expectedClientVtable ||
			clientPrimaryVtable[ 20 ] !=
				(void *)( (BYTE *)engineModule + kSetSignonStateRva ) )
		{
			LogContext( "signon finalize rejected: primary vtable or SetSignonState target mismatch" );
			AbortMappedConnection( connection, "source_signon_target_mismatch", 0 );
			return;
		}

		SetSignonStateFn setSignonState =
			(SetSignonStateFn)(uintptr_t)clientPrimaryVtable[ 20 ];
		LogContextf( "signon finalize begin client=%p state=CONNECTED(2) spawnCount=0 registry=%u",
			client, ASRD_GNS_MessageRegistryCount() );
		const bool signonResult = setSignonState( client, 2, 0 );
		s_sourceSignonInProgress = false;
		LogContextf( "signon finalize complete client=%p result=%u registry=%u",
			client, signonResult ? 1U : 0U,
			ASRD_GNS_MessageRegistryCount() );
		if ( s_connection.connection != connection ||
			!s_contextCreated || s_contextClient != client )
		{
			LogContext( "signon finalize aborted: SetSignonState changed lifecycle state" );
			AbortMappedConnection( connection, "source_signon_lifecycle_changed", 0 );
			return;
		}
		if ( !signonResult )
		{
			LogContext( "signon finalize failed: SetSignonState returned false" );
			AbortMappedConnection( connection, "source_signon_failed", 0 );
			return;
		}
		if ( signonResult )
		{
			s_sourceSignonFinalized = true;
			LogContextf( "signon finalize success client=%p sourceSignonFinalized=1",
				client );
		}
	}
}

void ASRD_GNS_ServerRealPlayerEntered( void *sourceEdict )
{
#if defined( GAME_DLL )
	if ( !s_initialized || s_shutdownCommitted || !HasConnection() ||
		s_connection.state != ASRD_GNS_SERVER_CONNECTED ||
		!IsMappedSourceEdict( sourceEdict ) )
		return;

	if ( s_activeRealPlayerCount == 0 )
	{
		s_activeRealPlayerCount = 1;
		s_sessionHadRealPlayer = true;
		s_hadRealPlayer = true;
		Warning( "[ASRD-GNS-SERVER] real player entered handle=%lu active_real_players=1 had_real_player=1\n",
			(unsigned long)s_connection.connection );
	}
	if ( s_shutdownPending )
		CancelShutdownGracePeriod( "real_player_reentered" );
#else
	(void)sourceEdict;
#endif
}

void ASRD_GNS_ServerTraceClientMessage( int type, unsigned int bitLength )
{
	if ( type != 4 && type != 9 )
		return;

	unsigned int *count = type == 4 ? &s_traceType4Count : &s_traceType9Count;
	++( *count );
	SourceClientStateSnapshot state = {};
	if ( !ReadSourceClientState( &state ) )
	{
		if ( *count <= 3 )
			LogContextf( "client message boundary semantic=%s type=%d bitlen=%u state=unavailable count=%u",
				type == 4 ? "net_stringcmd" : "clc_move", type, bitLength, *count );
		return;
	}

	if ( *count <= 3 )
	{
		LogContextf( "client message boundary semantic=%s type=%d bitlen=%u count=%u slot=%d signon=%d connected=%u spawned=%u active=%u",
			type == 4 ? "net_stringcmd" : "clc_move", type, bitLength, *count, state.slot, state.signonState,
			state.connected ? 1U : 0U, state.spawned ? 1U : 0U,
			state.active ? 1U : 0U );
	}
}

void ASRD_GNS_ServerTraceLifecycle( const char *phase, const char *mapName,
	bool finalShutdown )
{
	SourceClientStateSnapshot state = {};
	const bool stateAvailable = ReadSourceClientState( &state );
	int playerCount = 0;
#if defined( GAME_DLL )
	if ( gpGlobals )
	{
		for ( int i = 1; i <= gpGlobals->maxClients; ++i )
		{
			if ( UTIL_PlayerByIndex( i ) != NULL )
				++playerCount;
		}
	}
#endif

	const char *currentMap = mapName;
#if defined( GAME_DLL )
	if ( ( !currentMap || !currentMap[ 0 ] ) && gpGlobals &&
		gpGlobals->mapname != NULL_STRING )
		currentMap = STRING( gpGlobals->mapname );
#endif
	if ( !currentMap )
		currentMap = "";

	LogContextf( "lifecycle phase=%s final_shutdown=%u map=%s gns_handle=%lu gns_state=%s gns_count=%u player_count=%d source_state=%s source_slot=%d signon=%d connected=%u spawned=%u active=%u",
		phase ? phase : "<null>", finalShutdown ? 1U : 0U, currentMap,
		(unsigned long)s_connection.connection, StateName( s_connection.state ),
		HasConnection() ? 1U : 0U, playerCount,
		stateAvailable ? "available" : "unavailable",
		stateAvailable ? state.slot : -1,
		stateAvailable ? state.signonState : -1,
		stateAvailable && state.connected ? 1U : 0U,
		stateAvailable && state.spawned ? 1U : 0U,
		stateAvailable && state.active ? 1U : 0U );
}

bool ASRD_GNS_ServerInit( uint16_t port )
{
	if ( s_initialized )
		return s_requestedPort == port;
	if ( port == 0 )
	{
		Warning( "[ASRD-GNS-SERVER] init rejected port=0\n" );
		return false;
	}

	// A new listener/context starts a new command namespace.  This is the
	// connection/session reset boundary used by the single-client deployment;
	// changelevel tracing alone does not reset command numbers.
	ResetShutdownState();
	ResetCompatibilityState( &s_serverConnection );

	if ( !ASRD_GNS_Initialize( 1 ) )
	{
		Warning( "[ASRD-GNS-SERVER] init failed reason=wrapper_initialize port=%u\n",
			(unsigned)port );
		return false;
	}

	Warning( "[ASRD-GNS-SERVER] listen bind requested_port=%u start_port=%u\n",
		(unsigned)port, (unsigned)port );
	for ( uint32_t candidate = port; candidate <= 65535U; ++candidate )
	{
		if ( ASRD_GNS_Listen( (uint16_t)candidate ) != ASRD_GNS_CONNECTION_INVALID )
		{
			s_initialized = true;
			s_requestedPort = port;
			s_listenPort = (uint16_t)candidate;
			Warning( "[ASRD-GNS-SERVER] listen ready requested_port=%u actual_port=%u fallback=%s\n",
				(unsigned)port, (unsigned)candidate,
				candidate == (uint32_t)port ? "no" : "yes" );
			return true;
		}
		if ( candidate == 65535U )
			break;
	}

	Warning( "[ASRD-GNS-SERVER] listen failed requested_port=%u attempted_through=65535\n",
		(unsigned)port );
	ASRD_GNS_Shutdown();
	return false;
}

void ASRD_GNS_ServerControlFrame( void )
{
	if ( !s_initialized )
		return;

	// Evaluate before RunCallbacks so an expired grace period locks shutdown
	// before a queued/new incoming event can be accepted.  This function is
	// also used by the existing hibernation wake pump; no hibernation edge is a
	// source of shutdown state.
	EvaluateShutdown();
	if ( !s_initialized )
		return;

	// RunCallbacks only queues opaque status events.  All Source/game-owned
	// lifecycle work, including AcceptConnection, occurs below on this thread.
	ASRD_GNS_RunFrame();

	ASRD_GNS_ConnectionEvent event = {};
	while ( ASRD_GNS_PollConnectionEvent( &event ) )
	{
		switch ( event.state )
		{
		case ASRD_GNS_CONNECTION_EVENT_INCOMING:
			HandleIncoming( event.connection );
			break;
		case ASRD_GNS_CONNECTION_EVENT_CONNECTED:
			HandleConnected( event.connection );
			break;
		case ASRD_GNS_CONNECTION_EVENT_FAILED:
		case ASRD_GNS_CONNECTION_EVENT_CLOSED:
			HandleTerminal( event );
			break;
		case ASRD_GNS_CONNECTION_EVENT_CONNECTING:
			// A server should receive INCOMING, but accepting this event as an
			// incoming candidate keeps the ABI tolerant of callback ordering.
			Warning( "[ASRD-GNS-SERVER] connecting event treated as incoming handle=%lu\n",
				(unsigned long)event.connection );
			HandleIncoming( event.connection );
			break;
		default:
			Warning( "[ASRD-GNS-SERVER] event ignored state=%d handle=%lu reason=%d\n",
				event.state, (unsigned long)event.connection, event.reason );
			break;
		}
	}

	// A frame spent draining callbacks can cross the deadline.  Check again
	// after the event drain, while keeping the listener alive for every frame
	// before the actual commit.
	EvaluateShutdown();
}

void ASRD_GNS_ServerWakeControlFrame( void )
{
	ASRD_GNS_ServerControlFrame();
}

void ASRD_GNS_ServerFrame( void )
{
	if ( !s_initialized )
		return;

	ASRD_GNS_ServerControlFrame();

	EnsureSourceGameplayContext();

}

bool ASRD_GNS_ServerIsInitialized( void )
{
	return s_initialized;
}

bool ASRD_GNS_ServerIsHibernating( void )
{
#if defined( GAME_DLL )
	return s_initialized && engine && engine->IsDedicatedServer() &&
		g_ServerGameDLL.m_bIsHibernating;
#else
	return false;
#endif
}

void ASRD_GNS_ServerFinalizeSourceSignon( void )
{
	if ( !s_initialized )
		return;
	FinalizeSourceSignon();

}

bool ASRD_GNS_ServerMessageBridgeReady( void )
{
	return !HasConnection() || s_sourceSignonFinalized;
}

void ASRD_GNS_ServerShutdown( void )
{
	if ( s_shutdownCommitted && !s_initialized )
		return;
	s_shutdownCommitted = true;
	s_shutdownPending = false;
	s_shutdownDeadline = 0.0;
	s_activeRealPlayerCount = 0;
	s_sessionHadRealPlayer = false;

	// Drop both queued receive work and any unsealed logical update before the
	// connection/context is destroyed.  No command or update state may survive
	// a listener/session rebuild.
	ASRD_GNS_MessageBridgeDiscardQueued( ASRD_GNS_CONNECTION_INVALID );
	if ( s_contextCreated || s_contextClient )
		TeardownSourceServerContext( "server_shutdown" );
	if ( HasConnection() )
	{
		Warning( "[ASRD-GNS-SERVER] shutdown close handle=%lu\n",
			(unsigned long)s_connection.connection );
		ASRD_GNS_Close( s_connection.connection );
		SetState( ASRD_GNS_SERVER_CLOSED, 0 );
		RemoveConnection( "server_shutdown" );
	}
	if ( s_initialized )
		ASRD_GNS_Shutdown();
	ResetCompatibilityState( &s_serverConnection );
	s_initialized = false;
	s_requestedPort = 0;
	s_listenPort = 0;
}

unsigned int ASRD_GNS_ServerConnectionCount( void )
{
	return HasConnection() ? 1U : 0U;
}

ASRD_GNS_ServerConnectionState ASRD_GNS_ServerState( void )
{
	return s_connection.state;
}

ASRD_GNS_Connection ASRD_GNS_ServerConnection( void )
{
	return s_connection.connection;
}

ASRD_GNS_ServerConnectionContext *ASRD_GNS_ServerContext( void )
{
	return &s_serverConnection;
}

ASRD_GNS_ServerConnectionContext *ASRD_GNS_ServerContextForConnection(
	ASRD_GNS_Connection connection )
{
	if ( connection == ASRD_GNS_CONNECTION_INVALID ||
		s_serverConnection.connection != connection )
		return NULL;
	return &s_serverConnection;
}

ASRD_GNS_MoveRangeClass ASRD_GNS_ServerClassifyMove(
	ASRD_GNS_ServerConnectionContext *context,
	const ASRD_GNS_MoveMetadata &move )
{
	if ( !context || context != &s_serverConnection )
		return ASRD_GNS_MOVE_STALE;
	return ASRD_GNS_ClassifyMoveRange( move,
		context->has_last_accepted_command_number,
		context->last_accepted_command_number );
}

int ASRD_GNS_ServerComputeMoveDrop(
	ASRD_GNS_ServerConnectionContext *context,
	const ASRD_GNS_MoveMetadata &move )
{
	if ( !context || context != &s_serverConnection )
		return 0;
	return ASRD_GNS_ComputeMoveDropNumber( move,
		context->has_last_accepted_command_number,
		context->last_accepted_command_number );
}

void ASRD_GNS_ServerBeginMoveProcess(
	ASRD_GNS_ServerConnectionContext *context, int droppedCommands )
{
	if ( !context || context != &s_serverConnection )
		return;
	context->processing_move = true;
	context->current_move_drop_number = droppedCommands > 0
		? droppedCommands : 0;
}

void ASRD_GNS_ServerFinishMoveProcess(
	ASRD_GNS_ServerConnectionContext *context,
	const ASRD_GNS_MoveMetadata &move, bool processSucceeded )
{
	if ( !context || context != &s_serverConnection )
		return;

	// GetDropNumber is valid only while Process() is on the stack. Clear the
	// scoped value before publishing the accepted command range.
	context->processing_move = false;
	context->current_move_drop_number = 0;
	if ( processSucceeded && move.valid && move.has_new_commands &&
		( !context->has_last_accepted_command_number ||
			move.highest_new_command_number >
				context->last_accepted_command_number ) )
	{
		context->has_last_accepted_command_number = true;
		context->last_accepted_command_number =
			move.highest_new_command_number;
	}
}

int ASRD_GNS_ServerGetCurrentMoveDrop(
	const ASRD_GNS_ServerConnectionContext *context )
{
	if ( !context || context != &s_serverConnection ||
		!context->processing_move )
		return 0;
	return context->current_move_drop_number;
}

int ASRD_GNS_ServerGetLastAcceptedCommandNumber(
	const ASRD_GNS_ServerConnectionContext *context, bool *hasValue )
{
	if ( hasValue )
		*hasValue = false;
	if ( !context || context != &s_serverConnection )
		return 0;
	if ( hasValue )
		*hasValue = context->has_last_accepted_command_number;
	return context->last_accepted_command_number;
}

bool ASRD_GNS_ServerCaptureOutgoingUpdateMetadata(
	ASRD_GNS_ServerConnectionContext *context, uint32_t *serverUpdateSeq,
	int *clientCommandAck )
{
	if ( !context || context != &s_serverConnection ||
		context->connection == ASRD_GNS_CONNECTION_INVALID ||
		!serverUpdateSeq || !clientCommandAck )
		return false;

	// This is intentionally the seal/flush point. A message appended earlier
	// in the update must observe the latest successfully accepted CLC_Move.
	context->client_command_ack =
		context->has_last_accepted_command_number
			? context->last_accepted_command_number : 0;
	context->server_update_seq = ++context->next_server_update_seq;
	*serverUpdateSeq = context->server_update_seq;
	*clientCommandAck = context->client_command_ack;
	return true;
}

void ASRD_GNS_ServerResetCompatibility(
	ASRD_GNS_ServerConnectionContext *context )
{
	if ( !context || context != &s_serverConnection )
		return;
	ResetCompatibilityState( context );
}

#else

bool ASRD_GNS_ServerInit( uint16_t )
{
	return false;
}

void ASRD_GNS_ServerFrame( void )
{
}

void ASRD_GNS_ServerControlFrame( void )
{
}

void ASRD_GNS_ServerWakeControlFrame( void )
{
}

bool ASRD_GNS_ServerIsInitialized( void )
{
	return false;
}

bool ASRD_GNS_ServerIsHibernating( void )
{
	return false;
}

void ASRD_GNS_ServerFinalizeSourceSignon( void )
{
}

void ASRD_GNS_ServerRealPlayerEntered( void * )
{
}

bool ASRD_GNS_ServerMessageBridgeReady( void )
{
	return true;
}

void ASRD_GNS_ServerShutdown( void )
{
}

unsigned int ASRD_GNS_ServerConnectionCount( void )
{
	return 0;
}

ASRD_GNS_ServerConnectionState ASRD_GNS_ServerState( void )
{
	return ASRD_GNS_SERVER_DISCONNECTED;
}

ASRD_GNS_Connection ASRD_GNS_ServerConnection( void )
{
	return ASRD_GNS_CONNECTION_INVALID;
}

ASRD_GNS_ServerConnectionContext *ASRD_GNS_ServerContext( void )
{
	return NULL;
}

ASRD_GNS_ServerConnectionContext *ASRD_GNS_ServerContextForConnection(
	ASRD_GNS_Connection )
{
	return NULL;
}

ASRD_GNS_MoveRangeClass ASRD_GNS_ServerClassifyMove(
	ASRD_GNS_ServerConnectionContext *, const ASRD_GNS_MoveMetadata & )
{
	return ASRD_GNS_MOVE_STALE;
}

int ASRD_GNS_ServerComputeMoveDrop(
	ASRD_GNS_ServerConnectionContext *, const ASRD_GNS_MoveMetadata & )
{
	return 0;
}

void ASRD_GNS_ServerBeginMoveProcess(
	ASRD_GNS_ServerConnectionContext *, int )
{
}

void ASRD_GNS_ServerFinishMoveProcess(
	ASRD_GNS_ServerConnectionContext *, const ASRD_GNS_MoveMetadata &, bool )
{
}

int ASRD_GNS_ServerGetCurrentMoveDrop(
	const ASRD_GNS_ServerConnectionContext * )
{
	return 0;
}

int ASRD_GNS_ServerGetLastAcceptedCommandNumber(
	const ASRD_GNS_ServerConnectionContext *, bool *hasValue )
{
	if ( hasValue )
		*hasValue = false;
	return 0;
}

bool ASRD_GNS_ServerCaptureOutgoingUpdateMetadata(
	ASRD_GNS_ServerConnectionContext *, uint32_t *, int * )
{
	return false;
}

void ASRD_GNS_ServerResetCompatibility(
	ASRD_GNS_ServerConnectionContext * )
{
}

#endif
