#include "cbase.h"
#include "rd_lobby_utils.h"
#include "matchmaking/imatchframework.h"
#include "steam/steam_api.h"

#ifdef CLIENT_DLL
#include "gameui/swarm/uigamedata.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CSteamID UTIL_RD_GetCurrentLobbyID()
{
	if ( !g_pMatchFramework || !g_pMatchFramework->GetMatchSession() )
	{
		return CSteamID();
	}

	return CSteamID( g_pMatchFramework->GetMatchSession()->GetSessionSystemData()->GetUint64( "xuidReserve" ) );
}

#ifdef CLIENT_DLL
void UTIL_RD_JoinByLobbyID( CSteamID lobby )
{
	if ( !g_pMatchFramework)
	{
		return;
	}

	KeyValues *pSettings = KeyValues::FromString(
		"settings",
		" system { "
			" network LIVE "
		" } "
		" options { "
			" action joinsession "
		" } "
		);
	pSettings->SetUint64( "options/sessionid", lobby.ConvertToUint64() );
	KeyValues::AutoDelete autodelete( pSettings );

	g_pMatchFramework->MatchSession( pSettings );
}
#endif

bool UTIL_RD_IsLobbyOwner()
{
	CSteamID lobby = UTIL_RD_GetCurrentLobbyID();
	if ( !lobby.IsValid() )
	{
		return false;
	}

	CSteamID owner = SteamMatchmaking()->GetLobbyOwner( lobby );
	return owner == SteamUser()->GetSteamID();
}

KeyValues *UTIL_RD_LobbyToLegacyKeyValues( CSteamID lobby )
{
	if ( !g_pMatchFramework || !g_pMatchFramework->GetMatchNetworkMsgController() )
	{
		return NULL;
	}

	KeyValues *pKV = g_pMatchFramework->GetMatchNetworkMsgController()->UnpackGameDetailsFromSteamLobby( lobby.ConvertToUint64() );
	// the system is broken, so we need to manually fix it:
	KeyValues *pGameKey = pKV->FindKey( "game", true );
	if (pGameKey)
		pGameKey->Clear();
	int nCount = SteamMatchmaking()->GetLobbyDataCount( lobby );
	for ( int i = 0; i < nCount; i++ )
	{
		char szKey[256];
		char szValue[1024];
		if ( SteamMatchmaking()->GetLobbyDataByIndex( lobby, i, szKey, sizeof( szKey ), szValue, sizeof( szValue ) ) )
		{
			if ( StringHasPrefix( szKey, "game:" ) || StringHasPrefix( szKey, "system:" ) )
			{
				for ( char *pszKey = szKey; *pszKey; pszKey++ )
				{
					if ( *pszKey == ':' )
					{
						*pszKey = '/';
					}
				}
				pKV->SetString( szKey, szValue );
			}
		}
	}
	pKV->SetUint64( "options/sessionid", lobby.ConvertToUint64() );
	pKV->SetInt( "server/ping", UTIL_RD_PingLobby( lobby ) );
	return pKV;
}

const char *UTIL_RD_GetCurrentLobbyData( const char *pszKey, const char *pszDefaultValue )
{
	CSteamID lobby = UTIL_RD_GetCurrentLobbyID();
	if ( !lobby.IsValid() )
	{
		DevWarning( "UTIL_RD_GetCurrentLobbyData(\"%s\") called, but we're not in a lobby!\n", pszKey );
		return pszDefaultValue;
	}

	const char *pszValue = SteamMatchmaking()->GetLobbyData( lobby, pszKey );
	if ( *pszValue )
	{
		return pszValue;
	}
	return pszDefaultValue;
}

void UTIL_RD_UpdateCurrentLobbyData( const char *pszKey, const char *pszValue )
{
	CSteamID lobby = UTIL_RD_GetCurrentLobbyID();
	if ( !lobby.IsValid() )
	{
		DevWarning( "UTIL_RD_UpdateCurrentLobbyData(\"%s\", \"%s\") called, but we're not in a lobby!\n", pszKey, pszValue );
		return;
	}

	SteamMatchmaking()->SetLobbyData( lobby, pszKey, pszValue );
}

void UTIL_RD_UpdateCurrentLobbyData( const char *pszKey, int iValue )
{
	char szValue[21];
	Q_snprintf( szValue, sizeof( szValue ), "%d", iValue );
	UTIL_RD_UpdateCurrentLobbyData( pszKey, szValue );
}

void UTIL_RD_UpdateCurrentLobbyData( const char *pszKey, uint64 iValue )
{
	char szValue[17];
	Q_snprintf( szValue, sizeof( szValue ), "%llX", iValue );
	UTIL_RD_UpdateCurrentLobbyData( pszKey, szValue );
}

void UTIL_RD_RemoveCurrentLobbyData( const char *pszKey )
{
	CSteamID lobby = UTIL_RD_GetCurrentLobbyID();
	if ( !lobby.IsValid() )
	{
		DevWarning( "UTIL_RD_RemoveCurrentLobbyData(\"%s\") called, but we're not in a lobby!\n", pszKey );
		return;
	}

	SteamMatchmaking()->DeleteLobbyData( lobby, pszKey );
}

int UTIL_RD_PingLobby( CSteamID lobby )
{
	SteamNetworkPingLocation_t location;
	const char *szLocation = SteamMatchmaking()->GetLobbyData( lobby, "system:rd_lobby_location" );
	if ( SteamNetworkingUtils() && SteamNetworkingUtils()->ParsePingLocationString( szLocation, location ) )
	{
		int iPing = SteamNetworkingUtils()->EstimatePingTimeFromLocalHost( location );
		return MAX( iPing, 0 );
	}

	return 0;
}

static void DebugSpewLobby( CSteamID lobby )
{
	DevMsg( 2, "LOBBY: %llu\n", lobby.ConvertToUint64() );
	if ( developer.GetInt() >= 3 )
	{
		if ( !lobby.IsValid() )
		{
			DevMsg( 3, "Invalid lobby!\n" );
			return;
		}

		uint32 serverIP;
		uint16 serverPort;
		CSteamID serverID;
		if ( SteamMatchmaking()->GetLobbyGameServer( lobby, &serverIP, &serverPort, &serverID ) )
		{
			netadr_t serverAddr( serverIP, serverPort );
			DevMsg( 3, "server: %s (%llu)\n", serverAddr.ToString(), serverID.ConvertToUint64() );
		}
		else
		{
			DevMsg( 3, "no server!\n" );
		}

		if ( int iPing = UTIL_RD_PingLobby( lobby ) )
		{
			DevMsg( 3, "estimated ping: %dms\n", iPing );
		}

		CSteamID owner = SteamMatchmaking()->GetLobbyOwner( lobby );
		DevMsg( 3, "owner: %llu \"%s\"\n", owner.ConvertToUint64(), owner.IsValid() ? SteamFriends()->GetFriendPersonaName( owner ) : "(invalid)" );

		int iMembers = SteamMatchmaking()->GetNumLobbyMembers( lobby );
		DevMsg( 3, "members: %d\n", iMembers );
		if ( lobby == UTIL_RD_GetCurrentLobbyID() )
		{
			for ( int i = 0; i < iMembers; i++ )
			{
				CSteamID member = SteamMatchmaking()->GetLobbyMemberByIndex( lobby, i );
				DevMsg( 3, "member %d: %llu \"%s\"\n", i, member.ConvertToUint64(), SteamFriends()->GetFriendPersonaName( member ) );
			}
		}

		int iCount = SteamMatchmaking()->GetLobbyDataCount( lobby );
		for ( int i = 0; i < iCount; i++ )
		{
			char szKey[k_nMaxLobbyKeyLength];
			char szValue[1]; // only big enough to hold the null terminator
			SteamMatchmaking()->GetLobbyDataByIndex( lobby, i, szKey, sizeof( szKey ), szValue, sizeof( szValue ) );
			const char *pszValue = SteamMatchmaking()->GetLobbyData( lobby, szKey );

			DevMsg( 3, "\"%s\" \"%s\"\n", szKey, pszValue );
		}
	}
}

#ifdef CLIENT_DLL
#define LOBBY_SEARCH_DISABLED -1
#define LOBBY_SEARCH_INTERVAL 15.0f

class CReactiveDropLobbySearchSystem : CAutoGameSystemPerFrame
{
public:
	CReactiveDropLobbySearchSystem() :
		CAutoGameSystemPerFrame( "CReativeDropLobbySearchSystem" )
	{
	}

	CUtlVector<CReactiveDropLobbySearch *> m_Instances;

	virtual void Update( float frameTime )
	{
		FOR_EACH_VEC( m_Instances, i )
		{
			m_Instances[i]->Update();
		}
	}
};

static CReactiveDropLobbySearchSystem g_ReactiveDropLobbySearchSystem;

CReactiveDropLobbySearch::CReactiveDropLobbySearch( const char *pszDebugName ) :
	m_DistanceFilter( k_ELobbyDistanceFilterDefault ),
	m_pszDebugName( pszDebugName ),
	m_flNextUpdate( LOBBY_SEARCH_DISABLED )
{
}

CReactiveDropLobbySearch::~CReactiveDropLobbySearch()
{
	g_ReactiveDropLobbySearchSystem.m_Instances.FindAndRemove( this );
}

void CReactiveDropLobbySearch::Update()
{
	if ( m_flNextUpdate == LOBBY_SEARCH_DISABLED || m_flNextUpdate > Plat_FloatTime() )
	{
		return;
	}

	m_flNextUpdate = Plat_FloatTime() + LOBBY_SEARCH_INTERVAL;

	UpdateSearch();
}

static const char *LobbyComparisonName( ELobbyComparison comparison )
{
	switch ( comparison )
	{
	case k_ELobbyComparisonEqual:
		return "==";
	case k_ELobbyComparisonEqualToOrGreaterThan:
		return ">=";
	case k_ELobbyComparisonEqualToOrLessThan:
		return "<=";
	case k_ELobbyComparisonGreaterThan:
		return ">";
	case k_ELobbyComparisonLessThan:
		return "<";
	case k_ELobbyComparisonNotEqual:
		return "!=";
	}

	return "???";
}

static const char *LobbyDistanceFilterName( ELobbyDistanceFilter distance )
{
	switch ( distance )
	{
	case k_ELobbyDistanceFilterClose:
		return "close";
	case k_ELobbyDistanceFilterDefault:
		return "default";
	case k_ELobbyDistanceFilterFar:
		return "far";
	case k_ELobbyDistanceFilterWorldwide:
		return "worldwide";
	}

	return "???";
}

void CReactiveDropLobbySearch::UpdateSearch()
{
	if ( !SteamMatchmaking() )
	{
		AssertOnce( !"Missing Steam Matchmaking context" );
		return;
	}

	if ( m_LobbyMatchListResult.IsActive() )
	{
		DevWarning( 2, "%s: LobbyMatchResult has not received a value yet. Waiting until next tick.\n", m_pszDebugName );
		return;
	}

	DevMsg( 3, "%s: searching for lobbies\n", m_pszDebugName );

	FOR_EACH_VEC( m_StringFilters, i )
	{
		SteamMatchmaking()->AddRequestLobbyListStringFilter( m_StringFilters[i].m_Name, m_StringFilters[i].m_Value, m_StringFilters[i].m_Compare );
		DevMsg( 3, "%s: filter: \"%s\" %s \"%s\"\n", m_pszDebugName, m_StringFilters[i].m_Name.Get(), LobbyComparisonName( m_StringFilters[i].m_Compare ), m_StringFilters[i].m_Value.Get() );
	}
	FOR_EACH_VEC( m_NumericalFilters, i )
	{
		SteamMatchmaking()->AddRequestLobbyListNumericalFilter( m_NumericalFilters[i].m_Name, m_NumericalFilters[i].m_Value, m_NumericalFilters[i].m_Compare );
		DevMsg( 3, "%s: filter: \"%s\" %s %d\n", m_pszDebugName, m_NumericalFilters[i].m_Name.Get(), LobbyComparisonName( m_NumericalFilters[i].m_Compare ), m_NumericalFilters[i].m_Value );
	}
	SteamMatchmaking()->AddRequestLobbyListDistanceFilter( m_DistanceFilter );
	DevMsg( 3, "%s: distance filter: %s\n", m_pszDebugName, LobbyDistanceFilterName( m_DistanceFilter ) );

	SteamAPICall_t hAPICall = SteamMatchmaking()->RequestLobbyList();
	m_LobbyMatchListResult.Set( hAPICall, this, &CReactiveDropLobbySearch::LobbyMatchListResult );
}

void CReactiveDropLobbySearch::Clear()
{
	m_StringFilters.Purge();
	m_NumericalFilters.Purge();
	m_DistanceFilter = k_ELobbyDistanceFilterDefault;
}

void CReactiveDropLobbySearch::StartSearching( bool bForceNow )
{
	if ( SteamNetworkingUtils() )
	{
		SteamNetworkingUtils()->InitRelayNetworkAccess();
	}
	m_flNextUpdate = Plat_FloatTime() + ( bForceNow ? 0 : LOBBY_SEARCH_INTERVAL );
	if ( !g_ReactiveDropLobbySearchSystem.m_Instances.IsValidIndex( g_ReactiveDropLobbySearchSystem.m_Instances.Find( this ) ) )
	{
		g_ReactiveDropLobbySearchSystem.m_Instances.AddToTail( this );
	}
	if ( bForceNow )
	{
		m_LobbyMatchListResult.Cancel();
		Update();
	}
}

void CReactiveDropLobbySearch::StopSearching()
{
	m_flNextUpdate = LOBBY_SEARCH_DISABLED;
	g_ReactiveDropLobbySearchSystem.m_Instances.FindAndRemove( this );
	m_LobbyMatchListResult.Cancel();
}

void CReactiveDropLobbySearch::LobbyMatchListResult( LobbyMatchList_t *pResult, bool bIOFailure )
{
	if ( bIOFailure )
	{
		DevWarning( 2, "%s: IO failure when searching for lobbies!\n", m_pszDebugName );
		return;
	}

	CUtlVector<CSteamID> lobbies;
	lobbies.EnsureCapacity( pResult->m_nLobbiesMatching );
	for ( uint32 i = 0; i < pResult->m_nLobbiesMatching; i++ )
	{
		lobbies.AddToTail( SteamMatchmaking()->GetLobbyByIndex( i ) );
	}

	DevMsg( 2, "%s: matched %d lobbies:\n", m_pszDebugName, pResult->m_nLobbiesMatching );
	FOR_EACH_VEC( lobbies, i )
	{
		DebugSpewLobby( lobbies[i] );
	}

	FOR_EACH_VEC( m_Subscribers, i )
	{
		m_Subscribers[i]( lobbies );
	}
}

void CReactiveDropLobbySearch::Subscribe( func_t fn )
{
	m_Subscribers.AddToTail( fn );
}

void CReactiveDropLobbySearch::Unsubscribe( func_t fn )
{
	m_Subscribers.FindAndRemove( fn );
}

static CReactiveDropLobbySearch s_DebugLobbySearch( "DebugLobbySearch" );

CON_COMMAND( rd_lobby_debug_current, "" )
{
	DebugSpewLobby( UTIL_RD_GetCurrentLobbyID() );
}

CON_COMMAND( rd_lobby_debug_start_searching, "" )
{
	s_DebugLobbySearch.StartSearching( true );
}
CON_COMMAND( rd_lobby_debug_stop_searching, "" )
{
	s_DebugLobbySearch.StopSearching();
}
CON_COMMAND( rd_lobby_debug_clear, "" )
{
	s_DebugLobbySearch.Clear();
}
static bool DetermineCompare( const char *pszCompare, ELobbyComparison & compare )
{
	if ( !Q_strcmp( pszCompare, "==" ) )
	{
		compare = k_ELobbyComparisonEqual;
		return true;
	}
	if ( !Q_strcmp( pszCompare, "<=" ) )
	{
		compare = k_ELobbyComparisonEqualToOrLessThan;
		return true;
	}
	if ( !Q_strcmp( pszCompare, ">=" ) )
	{
		compare = k_ELobbyComparisonEqualToOrGreaterThan;
		return true;
	}
	if ( !Q_strcmp( pszCompare, "<" ) )
	{
		compare = k_ELobbyComparisonLessThan;
		return true;
	}
	if ( !Q_strcmp( pszCompare, ">" ) )
	{
		compare = k_ELobbyComparisonGreaterThan;
		return true;
	}
	if ( !Q_strcmp( pszCompare, "!=" ) )
	{
		compare = k_ELobbyComparisonNotEqual;
		return true;
	}
	return false;
}
CON_COMMAND( rd_lobby_debug_filter_string, "rd_lobby_debug_filter_string \"key\" \"value\" compare" )
{
	ELobbyComparison compare;
	if ( !DetermineCompare( args[3], compare ) )
	{
		Warning( "Invalid compare\n" );
		return;
	}

	s_DebugLobbySearch.m_StringFilters.AddToTail( CReactiveDropLobbySearch::StringFilter_t( args[1], args[2], compare ) );
}
CON_COMMAND( rd_lobby_debug_filter_numerical, "rd_lobby_debug_filter_numerical \"key\" value compare" )
{
	ELobbyComparison compare;
	if ( !DetermineCompare( args[3], compare ) )
	{
		Warning( "Invalid compare\n" );
		return;
	}

	s_DebugLobbySearch.m_NumericalFilters.AddToTail( CReactiveDropLobbySearch::NumericalFilter_t( args[1], atoi( args[2] ), compare ) );
}
CON_COMMAND( rd_lobby_debug_filter_distance, "1: close, 2: default, 3: far, 4: worldwide" )
{
	int iDistance = atoi( args[1] );
	switch ( iDistance )
	{
	case 1:
		s_DebugLobbySearch.m_DistanceFilter = k_ELobbyDistanceFilterClose;
		break;
	case 2:
		s_DebugLobbySearch.m_DistanceFilter = k_ELobbyDistanceFilterDefault;
		break;
	case 3:
		s_DebugLobbySearch.m_DistanceFilter = k_ELobbyDistanceFilterFar;
		break;
	case 4:
		s_DebugLobbySearch.m_DistanceFilter = k_ELobbyDistanceFilterWorldwide;
		break;
	default:
		Warning( "invalid lobby distance filter\n" );
		break;
	}
}
#endif
