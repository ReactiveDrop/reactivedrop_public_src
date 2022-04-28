#include "cbase.h"
#include "VFoundGroupGames_IAFRanks.h"
#include "EngineInterface.h"
#include "filesystem.h"

#include "fmtstr.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;
using namespace BaseModUI;

extern ConVar ui_foundgames_spinner_time;

static void JoinIAFRanksServerGame( const FoundGameListItem::Info & fi )
{
	engine->ClientCmd_Unrestricted( CFmtStr( "connect %s", fi.mpGameDetails->GetString( "server/adronline" ) ) );
}

FoundGroupGamesIAFRanks::FoundGroupGamesIAFRanks( Panel *parent, const char *panelName ) : BaseClass( parent, panelName )
{
	m_hServerListRequest = NULL;
	m_bRefreshFinished = false;
}

FoundGroupGamesIAFRanks::~FoundGroupGamesIAFRanks()
{
	if ( m_hServerListRequest && steamapicontext->SteamMatchmakingServers() )
	{
		steamapicontext->SteamMatchmakingServers()->CancelQuery( m_hServerListRequest );
		steamapicontext->SteamMatchmakingServers()->ReleaseRequest( m_hServerListRequest );
	}
}

void FoundGroupGamesIAFRanks::PaintBackground()
{
	char const *szGameMode = m_pDataSettings->GetString( "game/mode" );

	// TODO: different title?
	BaseClass::DrawDialogBackground( CFmtStr( "#L4D360UI_FoundGroupGames_Title%s%s", szGameMode[0] ? "_" : "", szGameMode ), NULL,
		"#L4D360UI_FoundGroupGames_Subtitle", NULL, NULL );
}

void FoundGroupGamesIAFRanks::OnEvent( KeyValues *pEvent )
{
	// we don't use these events; we do it ourselves
}

void FoundGroupGamesIAFRanks::StartSearching( void )
{
	if ( m_hServerListRequest )
	{
		steamapicontext->SteamMatchmakingServers()->CancelQuery( m_hServerListRequest );
		steamapicontext->SteamMatchmakingServers()->ReleaseRequest( m_hServerListRequest );
	}

	MatchMakingKeyValuePair_t filterTag;
	Q_strncpy( filterTag.m_szKey, "gametagsand", sizeof( filterTag.m_szKey ) );
	Q_strncpy( filterTag.m_szValue, "HoIAF", sizeof( filterTag.m_szValue ) );
	MatchMakingKeyValuePair_t *filters[] = { &filterTag };

	m_hServerListRequest = steamapicontext->SteamMatchmakingServers()->RequestInternetServerList( 563560, filters, NELEMS( filters ), this );
	m_bRefreshFinished = false;
	m_flSearchStartedTime = Plat_FloatTime();
	m_flSearchEndTime = m_flSearchStartedTime + ui_foundgames_spinner_time.GetFloat();
}

void FoundGroupGamesIAFRanks::AddServersToList( void )
{
	if ( !m_bRefreshFinished )
	{
		return;
	}

	static CUtlVector<uint32_t> s_ParticipatingServers;
	if ( s_ParticipatingServers.Count() == 0 )
	{
		CUtlBuffer buf;
		if ( g_pFullFileSystem->ReadFile( "resource/hoiaf_participating_servers.bin", "MOD", buf ) )
		{
			s_ParticipatingServers.EnsureCount( buf.Size() / 4 );
			V_memcpy( s_ParticipatingServers.Base(), buf.Base(), s_ParticipatingServers.Count() * 4 );
		}
	}

	int nServerCount = steamapicontext->SteamMatchmakingServers()->GetServerCount( m_hServerListRequest );
	for ( int i = 0; i < nServerCount; i++ )
	{
		gameserveritem_t *pServer = steamapicontext->SteamMatchmakingServers()->GetServerDetails( m_hServerListRequest, i );
		uint32 iServerAddr = pServer->m_NetAdr.GetIP();

		if ( s_ParticipatingServers.Find( iServerAddr ) == -1 )
		{
			continue;
		}

		FoundGameListItem::Info info;
		info.mInfoType = FoundGameListItem::FGT_SERVER;
		Q_strncpy( info.Name, pServer->GetName(), sizeof( info.Name ) );
		info.mIsJoinable = true;
		info.mbDLC = false;
		info.mbInGame = true;
		info.mPing = FoundGameListItem::Info::GP_HIGH;
		info.mpGameDetails = new KeyValues( "settings" );
		info.mpGameDetails->SetString( "system/network", "LIVE" );
		info.mpGameDetails->SetString( "system/access", "public" );
		info.mpGameDetails->SetString( "server/name", pServer->GetName() );
		info.mpGameDetails->SetString( "server/server", "dedicated" );
		info.mpGameDetails->SetString( "server/adronline", pServer->m_NetAdr.GetConnectionAddressString() );
		// not setting server/adrlocal
		info.mpGameDetails->SetInt( "server/ping", pServer->m_nPing );
		info.mpGameDetails->SetString( "server/connectstring", pServer->m_NetAdr.GetConnectionAddressString() );
		info.mpGameDetails->SetInt( "members/numSlots", pServer->m_nMaxPlayers );
		info.mpGameDetails->SetInt( "members/numPlayers", pServer->m_nPlayers );
		info.mpGameDetails->SetString( "game/mission", pServer->m_szMap );
		info.mpGameDetails->SetString( "game/state", "game" );
		info.mpGameDetails->SetString( "game/dir", pServer->m_szGameDir );
		info.mpGameDetails->SetString( "game/mode", "campaign" );
		info.mpGameDetails->SetString( "game/swarmstate", V_strstr( pServer->m_szGameTags, "Briefing," ) != NULL ? "briefing" : "ingame" );
		if ( V_strstr( pServer->m_szGameTags, "Easy," ) != NULL )
			info.mpGameDetails->SetString( "game/difficulty", "easy" );
		else if ( V_strstr( pServer->m_szGameTags, "Hard," ) != NULL )
			info.mpGameDetails->SetString( "game/difficulty", "hard" );
		else if ( V_strstr( pServer->m_szGameTags, "Insane," ) != NULL )
			info.mpGameDetails->SetString( "game/difficulty", "insane" );
		else if ( V_strstr( pServer->m_szGameTags, "Imba," ) != NULL )
			info.mpGameDetails->SetString( "game/difficulty", "imba" );
		else
			info.mpGameDetails->SetString( "game/difficulty", "normal" );
		info.mpGameDetails->SetBool( "game/hardcoreFF", V_strstr( pServer->m_szGameTags, "HardcoreFF," ) != NULL );
		info.mpGameDetails->SetBool( "game/onslaught", V_strstr( pServer->m_szGameTags, "Onslaught," ) != NULL );
		info.mpGameDetails->SetInt( "game/dlcrequired", 0 );
		if ( KeyValues *pMapInfo = g_pMatchExtSwarm->GetMapInfoByBspName( info.mpGameDetails, pServer->m_szMap ) )
		{
			info.mpGameDetails->SetInt( "game/missioninfo/version", pMapInfo->GetInt( "version" ) );
			info.mpGameDetails->SetString( "game/missioninfo/displaytitle", pMapInfo->GetString( "displaytitle" ) );
			info.mpGameDetails->SetString( "game/missioninfo/author", pMapInfo->GetString( "author" ) );
			info.mpGameDetails->SetString( "game/missioninfo/website", pMapInfo->GetString( "website" ) );
			info.mpGameDetails->SetBool( "game/missioninfo/builtin", pMapInfo->GetBool( "builtin" ) );
			info.mpGameDetails->SetString( "game/missioninfo/image", pMapInfo->GetString( "image" ) );
		}
		info.mFriendXUID = pServer->m_steamID.ConvertToUint64();
		info.miPing = pServer->m_nPing;
		info.mpfnJoinGame = &JoinIAFRanksServerGame;
		AddGameFromDetails( info );
	}
}

void FoundGroupGamesIAFRanks::ServerResponded( HServerListRequest hRequest, int iServer )
{
	// ignore; we only care about complete refresh
}

void FoundGroupGamesIAFRanks::ServerFailedToRespond( HServerListRequest hRequest, int iServer )
{
	// ignore; we only care about complete refresh
}

void FoundGroupGamesIAFRanks::RefreshComplete( HServerListRequest hRequest, EMatchMakingServerResponse response )
{
	m_flSearchStartedTime = 0.0f;
	m_bRefreshFinished = true;
	UpdateGameDetails();
}
