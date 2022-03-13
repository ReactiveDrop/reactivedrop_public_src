#include "cbase.h"
#include "VFoundGroupGames_IAFRanks.h"
#include "EngineInterface.h"

#include "fmtstr.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;
using namespace BaseModUI;

extern ConVar ui_foundgames_spinner_time;

#define IPv4(a, b, c, d) ((uint32(a)<<24) | (uint32(b)<<16) | (uint32(c)<<8) | uint32(d))
static const uint32 s_ParticipatingServers[] =
{
	IPv4(32, 97, 131, 159),
	IPv4(85, 214, 164, 101),
	IPv4(109, 255, 126, 44),
	IPv4(82, 64, 91, 191),
	IPv4(176, 67, 13, 11),
};
#undef IPv4

static void JoinIAFRanksServerGame( const FoundGameListItem::Info & fi )
{
	DebuggerBreak(); // TODO
}

FoundGroupGamesIAFRanks::FoundGroupGamesIAFRanks( Panel *parent, const char *panelName ) : BaseClass( parent, panelName )
{
	m_hServerListRequest = NULL;
	m_bRefreshFinished = false;
}

FoundGroupGamesIAFRanks::~FoundGroupGamesIAFRanks()
{
	if ( m_hServerListRequest )
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

	m_KeyValuesCleanup.Purge();

	int nServerCount = steamapicontext->SteamMatchmakingServers()->GetServerCount( m_hServerListRequest );
	for ( int i = 0; i < nServerCount; i++ )
	{
		gameserveritem_t *pServer = steamapicontext->SteamMatchmakingServers()->GetServerDetails( m_hServerListRequest, i );
		uint32 iServerAddr = pServer->m_NetAdr.GetIP();

		bool bFound = false;
		for ( int j = 0; j < NELEMS( s_ParticipatingServers ); j++ )
		{
			if ( iServerAddr == s_ParticipatingServers[j] )
			{
				bFound = true;
				break;
			}
		}

		if ( !bFound )
		{
			continue;
		}

		FoundGameListItem::Info info;
		info.mInfoType = FoundGameListItem::FGT_SERVER;
		Q_strncpy( info.Name, pServer->GetName(), sizeof( info.Name ) );
		info.mIsJoinable = true;
		info.mbDLC = false;
		info.mbInGame = false;
		info.mPing = FoundGameListItem::Info::GP_HIGH;
		info.mpGameDetails = new KeyValues( "FoundGame" );
		// TODO: what does matchmaking.dll put in here?
		m_KeyValuesCleanup[m_KeyValuesCleanup.AddToTail()].Assign( info.mpGameDetails );
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
