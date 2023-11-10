#include "cbase.h"
#include "rd_lobby_utils.h"
#include "matchmaking/imatchframework.h"
#include "steam/steam_api.h"

#ifdef CLIENT_DLL
#include "gameui/swarm/uigamedata.h"
#include "filesystem.h"
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
	if ( !lobby.IsValid() || !SteamMatchmaking() || !SteamUser() )
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

void RD_Lobby_Scoreboard_Entry_t::CountryCodeTexCoords( float &s0, float &t0, float &s1, float &t1 )
{
	Assert( CountryCode[0] >= 'A' && CountryCode[0] <= 'Z' );
	Assert( CountryCode[1] >= 'A' && CountryCode[1] <= 'Z' );
	Assert( CountryCode[2] == '\0' );

	// country code texture is laid out with 32 flags per row in AA AB AC AD... order.
	// each flag is 16x11 and the full texture is 512x256.
	// inset each side of the bounding box by half a texel to avoid color bleed.
	int index = ( CountryCode[0] - 'A' ) + ( CountryCode[1] - 'A' ) * 26;
	s0 = ( index % 32 ) * ( 16.0f / 512.0f ) + ( 0.5f / 512.0f );
	t0 = ( index / 32 ) * ( 11.0f / 256.0f ) + ( 0.5f / 256.0f );
	s1 = s0 + ( 15.0f / 512.0f );
	t1 = t0 + ( 10.0f / 256.0f );
}

void UTIL_RD_ReadLobbyScoreboard( CSteamID lobby, CUtlVector<RD_Lobby_Scoreboard_Entry_t> &scoreboard )
{
	const char *szScoreboard = SteamMatchmaking()->GetLobbyData( lobby, "system:rd_players" );
	if ( !szScoreboard || *szScoreboard == '\0' )
		return;

	CSplitString Players{ szScoreboard, "," };
	FOR_EACH_VEC( Players, i )
	{
		CSplitString PlayerInfo{ Players[i], "|" };
		Assert( PlayerInfo.Count() == 4 );
		if ( PlayerInfo.Count() != 4 )
			continue;

		int index = scoreboard.AddToTail();
		char szName[k_cchPersonaNameMax];
		V_hextobinary( PlayerInfo[0], V_strlen( PlayerInfo[0] ), reinterpret_cast< byte * >( szName ), sizeof( szName ) );
		V_UTF8ToUnicode( szName, scoreboard[index].Name, sizeof( scoreboard[index].Name ) );
		scoreboard[index].Score = V_atoi( PlayerInfo[1] );
		scoreboard[index].Connected = V_atof( PlayerInfo[2] );
		if ( V_strlen( PlayerInfo[3] ) == 2 )
		{
			scoreboard[index].CountryCode[0] = PlayerInfo[3][0];
			scoreboard[index].CountryCode[1] = PlayerInfo[3][1];
		}
		else
		{
			scoreboard[index].CountryCode[0] = 'X';
			scoreboard[index].CountryCode[1] = 'X';
		}
		scoreboard[index].CountryCode[2] = '\0';
	}
}

static void DebugSpewLobby( CSteamID lobby )
{
	ISteamMatchmaking *pMatchmaking = SteamMatchmaking();
	Assert( pMatchmaking );
	if ( !pMatchmaking )
		return;

	ISteamFriends *pFriends = SteamFriends();
	Assert( pFriends );
	if ( !pFriends )
		return;

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
		if ( pMatchmaking->GetLobbyGameServer( lobby, &serverIP, &serverPort, &serverID ) )
		{
			netadr_t serverAddr( serverIP, serverPort );
			DevMsg( 3, "server: %s (%llu)\n", serverAddr.ToString(), serverID.ConvertToUint64() );
		}
		else
		{
			DevMsg( 3, "no server info (expected)\n" );
		}

		if ( int iPing = UTIL_RD_PingLobby( lobby ) )
		{
			DevMsg( 3, "estimated ping: %dms\n", iPing );
		}

		int iMembers = pMatchmaking->GetNumLobbyMembers( lobby );
		DevMsg( 3, "members: %d\n", iMembers );
		if ( lobby == UTIL_RD_GetCurrentLobbyID() )
		{
			CSteamID owner = pMatchmaking->GetLobbyOwner( lobby );
			DevMsg( 3, "owner: %llu \"%s\"\n", owner.ConvertToUint64(), owner.IsValid() ? pFriends->GetFriendPersonaName( owner ) : "(invalid)" );

			for ( int i = 0; i < iMembers; i++ )
			{
				CSteamID member = pMatchmaking->GetLobbyMemberByIndex( lobby, i );
				DevMsg( 3, "member %d: %llu \"%s\"\n", i, member.ConvertToUint64(), pFriends->GetFriendPersonaName( member ) );
			}
		}

		CUtlVector<RD_Lobby_Scoreboard_Entry_t> scoreboard;
		UTIL_RD_ReadLobbyScoreboard( lobby, scoreboard );
		FOR_EACH_VEC( scoreboard, i )
		{
			char szName[k_cchPersonaNameMax];
			V_UnicodeToUTF8( scoreboard[i].Name, szName, sizeof( szName ) );
			DevMsg( 3, "player %d: \"%s\" | score: %d | connected for %d:%05.3f\n", i, szName, scoreboard[i].Score, int( scoreboard[i].Connected / 60 ), fmodf( scoreboard[i].Connected, 60.0f ) );
		}

		int iCount = pMatchmaking->GetLobbyDataCount( lobby );
		for ( int i = 0; i < iCount; i++ )
		{
			char szKey[k_nMaxLobbyKeyLength];
			char szValue[1]; // only big enough to hold the null terminator
			pMatchmaking->GetLobbyDataByIndex( lobby, i, szKey, sizeof( szKey ), szValue, sizeof( szValue ) );
			const char *pszValue = pMatchmaking->GetLobbyData( lobby, szKey );

			DevMsg( 3, "\"%s\" \"%s\"\n", szKey, pszValue );
		}
	}
}

#ifdef CLIENT_DLL
#define LOBBY_SEARCH_INTERVAL 30.0f

ConVar rd_lobby_search_debug( "rd_lobby_search_debug", "0" );

CReactiveDropLobbySearch::CReactiveDropLobbySearch( const char *pszDebugName ) :
	m_DistanceFilter{ k_ELobbyDistanceFilterWorldwide },
	m_pszDebugName{ pszDebugName },
	m_flNextUpdate{}
{
}

CReactiveDropLobbySearch::~CReactiveDropLobbySearch()
{
}

void CReactiveDropLobbySearch::WantUpdatedLobbyList( bool bForce )
{
	if ( !bForce && m_flNextUpdate > Plat_FloatTime() )
		return;

	// we need relay network access initialized to estimate lobby pings
	if ( ISteamNetworkingUtils *pUtils = SteamNetworkingUtils() )
		pUtils->InitRelayNetworkAccess();

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
	ISteamMatchmaking *pMatchmaking = SteamMatchmaking();
	if ( !pMatchmaking )
	{
		AssertOnce( !"Missing Steam Matchmaking context" );
		return;
	}

	if ( m_LobbyMatchListResult.IsActive() )
	{
		if ( rd_lobby_search_debug.GetBool() )
			DevWarning( 2, "%s: LobbyMatchResult has not received a value yet. Waiting until next tick.\n", m_pszDebugName );
		return;
	}

	if ( rd_lobby_search_debug.GetBool() )
		DevMsg( 3, "%s: searching for lobbies\n", m_pszDebugName );

	FOR_EACH_VEC( m_StringFilters, i )
	{
		pMatchmaking->AddRequestLobbyListStringFilter( m_StringFilters[i].m_Name, m_StringFilters[i].m_Value, m_StringFilters[i].m_Compare );
		if ( rd_lobby_search_debug.GetBool() )
			DevMsg( 3, "%s: filter: \"%s\" %s \"%s\"\n", m_pszDebugName, m_StringFilters[i].m_Name.Get(), LobbyComparisonName( m_StringFilters[i].m_Compare ), m_StringFilters[i].m_Value.Get() );
	}
	FOR_EACH_VEC( m_NumericalFilters, i )
	{
		pMatchmaking->AddRequestLobbyListNumericalFilter( m_NumericalFilters[i].m_Name, m_NumericalFilters[i].m_Value, m_NumericalFilters[i].m_Compare );
		if ( rd_lobby_search_debug.GetBool() )
			DevMsg( 3, "%s: filter: \"%s\" %s %d\n", m_pszDebugName, m_NumericalFilters[i].m_Name.Get(), LobbyComparisonName( m_NumericalFilters[i].m_Compare ), m_NumericalFilters[i].m_Value );
	}
	pMatchmaking->AddRequestLobbyListDistanceFilter( m_DistanceFilter );
	if ( rd_lobby_search_debug.GetBool() )
		DevMsg( 3, "%s: distance filter: %s\n", m_pszDebugName, LobbyDistanceFilterName( m_DistanceFilter ) );

	SteamAPICall_t hAPICall = pMatchmaking->RequestLobbyList();
	m_LobbyMatchListResult.Set( hAPICall, this, &CReactiveDropLobbySearch::LobbyMatchListResult );
}

void CReactiveDropLobbySearch::Clear()
{
	m_StringFilters.Purge();
	m_NumericalFilters.Purge();
	m_DistanceFilter = k_ELobbyDistanceFilterWorldwide;
}

void CReactiveDropLobbySearch::LobbyMatchListResult( LobbyMatchList_t *pResult, bool bIOFailure )
{
	if ( bIOFailure )
	{
		if ( rd_lobby_search_debug.GetBool() )
			DevWarning( 2, "%s: IO failure when searching for lobbies!\n", m_pszDebugName );
		m_MatchingLobbies.Purge();
		return;
	}

	ISteamMatchmaking *pMatchmaking = SteamMatchmaking();
	Assert( pMatchmaking );

	m_MatchingLobbies.SetCount( pResult->m_nLobbiesMatching );
	for ( uint32 i = 0; i < pResult->m_nLobbiesMatching; i++ )
	{
		m_MatchingLobbies[i] = pMatchmaking->GetLobbyByIndex( i );
	}

	if ( rd_lobby_search_debug.GetBool() )
	{
		DevMsg( 2, "%s: matched %d lobbies:\n", m_pszDebugName, pResult->m_nLobbiesMatching );
		FOR_EACH_VEC( m_MatchingLobbies, i )
		{
			DebugSpewLobby( m_MatchingLobbies[i] );
		}
	}
}

static CReactiveDropLobbySearch s_DebugLobbySearch( "DebugLobbySearch" );

CON_COMMAND( rd_lobby_debug_current, "" )
{
	DebugSpewLobby( UTIL_RD_GetCurrentLobbyID() );
}

CON_COMMAND( rd_lobby_debug_search, "" )
{
	rd_lobby_search_debug.SetValue( true );
	s_DebugLobbySearch.WantUpdatedLobbyList( true );
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

#ifdef DBGFLAG_ASSERT
// on debug builds, make sure that the participating servers file is up-to-date.
// we don't do this on release builds because it'd just create a bunch of network traffic for no gain.
static class CCheckHoIAFParticipatingServersFile : public CAutoGameSystem
{
public:
	CCheckHoIAFParticipatingServersFile() : CAutoGameSystem( "CCheckHoIAFParticipatingServersFile" )
	{
	}

	void PostInit() override
	{
		ISteamHTTP *pHTTP = SteamHTTP();
		Assert( pHTTP );
		if ( !pHTTP )
			return;

		HTTPRequestHandle hRequest = pHTTP->CreateHTTPRequest( k_EHTTPMethodGET, "https://stats.reactivedrop.com/hoiaf_participating_servers.bin" );
		Assert( hRequest != INVALID_HTTPREQUEST_HANDLE );
		SteamAPICall_t hCall{ k_uAPICallInvalid };
		bool bOK = pHTTP->SendHTTPRequest( hRequest, &hCall );
		Assert( bOK );

		m_HTTPRequestCompleted.Set( hCall, this, &CCheckHoIAFParticipatingServersFile::OnHTTPRequestCompleted );
	}

	void OnHTTPRequestCompleted( HTTPRequestCompleted_t *pParam, bool bIOFailure )
	{
		Assert( !bIOFailure && pParam->m_bRequestSuccessful );
		if ( bIOFailure || !pParam->m_bRequestSuccessful )
			return;

		Assert( pParam->m_eStatusCode == k_EHTTPStatusCode200OK );

		CUtlBuffer localBuf;
		bool bOK = g_pFullFileSystem->ReadFile( "resource/hoiaf_participating_servers.bin", "MOD", localBuf );
		Assert( bOK );

		Assert( localBuf.TellMaxPut() == int( pParam->m_unBodySize ) );

		CUtlBuffer remoteBuf{ 0, int( pParam->m_unBodySize ), 0 };
		bOK = SteamHTTP()->GetHTTPResponseBodyData( pParam->m_hRequest, ( uint8_t * )remoteBuf.Base(), pParam->m_unBodySize );
		Assert( bOK );

		Assert( !V_memcmp( localBuf.Base(), remoteBuf.Base(), MIN( localBuf.TellMaxPut(), int( pParam->m_unBodySize ) ) ) );
		SteamHTTP()->ReleaseHTTPRequest( pParam->m_hRequest );
	}

	CCallResult<CCheckHoIAFParticipatingServersFile, HTTPRequestCompleted_t> m_HTTPRequestCompleted;
} s_CheckHoIAFParticipatingServersFile;
#endif

CReactiveDropServerListHelper::CReactiveDropServerListHelper( const char *szDebugName )
{
	m_pszDebugName = szDebugName;
}

CReactiveDropServerListHelper::~CReactiveDropServerListHelper()
{
	ISteamMatchmakingServers *pServers = SteamMatchmakingServers();
	if ( pServers && m_hServerListRequest )
	{
		pServers->ReleaseRequest( m_hServerListRequest );
		m_hServerListRequest = NULL;
	}
	if ( pServers && m_hServerListRequestNext )
	{
		pServers->ReleaseRequest( m_hServerListRequestNext );
		m_hServerListRequestNext = NULL;
	}
}

void CReactiveDropServerListHelper::WantUpdatedServerList()
{
	if ( m_hServerListRequestNext || m_flSoonestServerListRequest > Plat_FloatTime() )
		return;

	ISteamMatchmakingServers *pServers = SteamMatchmakingServers();
	Assert( pServers );
	if ( !pServers )
		return;

	static const AppId_t iAppID = SteamUtils()->GetAppID();

	switch ( m_eMode )
	{
	case MODE_INTERNET:
		m_hServerListRequestNext = pServers->RequestInternetServerList( iAppID, m_Filters.Base(), m_Filters.Count(), this );
		break;
	case MODE_LAN:
		m_hServerListRequestNext = pServers->RequestLANServerList( iAppID, this );
		break;
	case MODE_FRIENDS:
		m_hServerListRequestNext = pServers->RequestFriendsServerList( iAppID, m_Filters.Base(), m_Filters.Count(), this );
		break;
	case MODE_FAVORITES:
		m_hServerListRequestNext = pServers->RequestFavoritesServerList( iAppID, m_Filters.Base(), m_Filters.Count(), this );
		break;
	case MODE_HISTORY:
		m_hServerListRequestNext = pServers->RequestHistoryServerList( iAppID, m_Filters.Base(), m_Filters.Count(), this );
		break;
	case MODE_SPECTATOR:
		m_hServerListRequestNext = pServers->RequestSpectatorServerList( iAppID, m_Filters.Base(), m_Filters.Count(), this );
		break;
	default:
		Assert( 0 );
		break;
	}

	m_flSoonestServerListRequest = Plat_FloatTime() + LOBBY_SEARCH_INTERVAL;
}

int CReactiveDropServerListHelper::Count() const
{
	ISteamMatchmakingServers *pServers = SteamMatchmakingServers();
	Assert( pServers );
	if ( !pServers || !m_hServerListRequest )
		return 0;

	return pServers->GetServerCount( m_hServerListRequest );
}

gameserveritem_t *CReactiveDropServerListHelper::GetDetails( int iServer ) const
{
	ISteamMatchmakingServers *pServers = SteamMatchmakingServers();
	Assert( pServers );
	if ( !pServers || !m_hServerListRequest )
		return NULL;

	gameserveritem_t *pDetails = pServers->GetServerDetails( m_hServerListRequest, iServer );
	Assert( pDetails );

	// favorite servers aren't filtering (bug in Steam API)
	Assert( SteamUtils() && pDetails->m_nAppID == SteamUtils()->GetAppID() );
	if ( SteamUtils() && pDetails->m_nAppID != SteamUtils()->GetAppID() )
		return NULL;

	return pDetails;
}

bool CReactiveDropServerListHelper::IsHoIAFServer( int iServer ) const
{
	return IsHoIAFServer( GetDetails( iServer ) );
}

bool CReactiveDropServerListHelper::IsHoIAFServer( gameserveritem_t *pDetails ) const
{
	if ( !pDetails )
		return false;

	CSplitString tags( pDetails->m_szGameTags, "," );
	FOR_EACH_VEC( tags, i )
	{
		if ( !V_strcmp( tags[i], "HoIAF" ) )
		{
			return g_ReactiveDropServerList.IsHoIAFServerIP( pDetails->m_NetAdr.GetIP() );
		}
	}

	return false;
}

bool CReactiveDropServerListHelper::IsVACSecure( int iServer ) const
{
	gameserveritem_t *pDetails = GetDetails( iServer );
	return pDetails && pDetails->m_bSecure;
}

bool CReactiveDropServerListHelper::HasPassword( int iServer ) const
{
	gameserveritem_t *pDetails = GetDetails( iServer );
	return pDetails && pDetails->m_bPassword;
}

int CReactiveDropServerListHelper::GetPing( int iServer ) const
{
	gameserveritem_t *pDetails = GetDetails( iServer );
	return pDetails ? pDetails->m_nPing : 0;
}

const char *CReactiveDropServerListHelper::GetName( int iServer ) const
{
	gameserveritem_t *pDetails = GetDetails( iServer );
	return pDetails ? pDetails->GetName() : "";
}

CSteamID CReactiveDropServerListHelper::GetSteamID( int iServer ) const
{
	gameserveritem_t *pDetails = GetDetails( iServer );
	return pDetails ? pDetails->m_steamID : k_steamIDNil;
}

void CReactiveDropServerListHelper::ServerResponded( HServerListRequest hRequest, int iServer )
{
	Assert( m_hServerListRequestNext == hRequest );
}
void CReactiveDropServerListHelper::ServerFailedToRespond( HServerListRequest hRequest, int iServer )
{
	Assert( m_hServerListRequestNext == hRequest );
}
void CReactiveDropServerListHelper::RefreshComplete( HServerListRequest hRequest, EMatchMakingServerResponse response )
{
	Assert( m_hServerListRequestNext == hRequest );

	if ( m_hServerListRequest )
		SteamMatchmakingServers()->ReleaseRequest( m_hServerListRequest );

	m_hServerListRequest = m_hServerListRequestNext;
	m_hServerListRequestNext = NULL;
}

CReactiveDropServerList g_ReactiveDropServerList;

CReactiveDropServerList::CReactiveDropServerList() :
	m_PublicLobbies{ "g_ReactiveDropServerList.m_PublicLobbies" },
	m_PublicServers{ "g_ReactiveDropServerList.m_PublicServers" },
	m_InternetServers{ "g_ReactiveDropServerList.m_InternetServers" },
	m_FavoriteServers{ "g_ReactiveDropServerList.m_FavoriteServers" },
	m_LANServers{ "g_ReactiveDropServerList.m_LANServers" }
{
	m_PublicServers.m_eMode = CReactiveDropServerListHelper::MODE_INTERNET;
	m_PublicServers.m_Filters.AddToTail( new MatchMakingKeyValuePair_t{ "and", "6" } );
	m_PublicServers.m_Filters.AddToTail( new MatchMakingKeyValuePair_t{ "secure", "" } );
	m_PublicServers.m_Filters.AddToTail( new MatchMakingKeyValuePair_t{ "or", "4" } );
	m_PublicServers.m_Filters.AddToTail( new MatchMakingKeyValuePair_t{ "and", "2" } );
	m_PublicServers.m_Filters.AddToTail( new MatchMakingKeyValuePair_t{ "notfull", "" } );
	m_PublicServers.m_Filters.AddToTail( new MatchMakingKeyValuePair_t{ "hasplayers", "" } );
	m_PublicServers.m_Filters.AddToTail( new MatchMakingKeyValuePair_t{ "gametagsand", "HoIAF" } );

	m_InternetServers.m_eMode = CReactiveDropServerListHelper::MODE_INTERNET;
	// if we don't specify a filter at all, the API starts querying ten thousand random servers for any game on Steam,
	// which isn't likely to give us useful results and it also takes several minutes to finish each refresh.
	// since we don't want to filter servers on this list, we're allowing servers with at least one player or empty slot;
	// this will allow servers with at least 1 max players, which is fine because you can't even network in source engine
	// with max players set below 2.
	m_InternetServers.m_Filters.AddToTail( new MatchMakingKeyValuePair_t{ "or", "2" } );
	m_InternetServers.m_Filters.AddToTail( new MatchMakingKeyValuePair_t{ "notfull", "" } );
	m_InternetServers.m_Filters.AddToTail( new MatchMakingKeyValuePair_t{ "hasplayers", "" } );

	m_FavoriteServers.m_eMode = CReactiveDropServerListHelper::MODE_FAVORITES;

	m_LANServers.m_eMode = CReactiveDropServerListHelper::MODE_LAN;
}

bool CReactiveDropServerList::IsHoIAFServerIP( uint32_t ip )
{
	if ( m_ParticipatingServers.Count() == 0 )
	{
		CUtlBuffer buf;
		if ( g_pFullFileSystem->ReadFile( "resource/hoiaf_participating_servers.bin", "MOD", buf ) )
		{
			m_ParticipatingServers.SetCount( buf.Size() / 4 );
			V_memcpy( m_ParticipatingServers.Base(), buf.Base(), m_ParticipatingServers.Count() * 4 );
		}
	}

	return m_ParticipatingServers.IsValidIndex( m_ParticipatingServers.Find( ip ) );
}
#endif
