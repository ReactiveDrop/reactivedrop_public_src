#include "cbase.h"
#include "nb_leaderboard_panel.h"
#include "nb_header_footer.h"
#include "nb_button.h"
#include "rd_vgui_leaderboard_panel.h"
#include "rd_lobby_utils.h"
#include "c_asw_steamstats.h"
#include "vgui/ILocalize.h"
#include "asw_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CNB_Leaderboard_Panel::CNB_Leaderboard_Panel( vgui::Panel *parent, const char *name ) : BaseClass( parent, name )
{
	m_pHeaderFooter = new CNB_Header_Footer( this, "HeaderFooter" );
	m_pBackButton = new CNB_Button( this, "BackButton", "", this, "BackButton" );

	m_pHeaderFooter->SetTitle( "#nb_leaderboard" );

	m_pLeaderboardBackground = new vgui::Panel( this, "LeaderboardBackground" );
	m_pLeaderboard = new CReactiveDrop_VGUI_Leaderboard_Panel( this, "Leaderboard" );

	m_pErrorLabel = new vgui::Label( this, "ErrorLabel", "" );
	m_pNotFoundLabel = new vgui::Label( this, "NotFoundLabel", "" );

	const char *szMap = IGameSystem::MapName();
	const char *szMapID = UTIL_RD_GetCurrentLobbyData( "game:missioninfo:workshop", "0" );
	PublishedFileId_t nMapID = strtoull( szMapID, NULL, 16 );
	const char *szChallenge = UTIL_RD_GetCurrentLobbyData( "game:challenge", "0" );
	const char *szChallengeID = UTIL_RD_GetCurrentLobbyData( "game:challengeinfo:workshop", "0" );
	PublishedFileId_t nChallengeID = strtoull( szChallengeID, NULL, 16 );

	if ( FStrEq( szChallenge, "0" ) )
	{
		m_pLeaderboard->SetTitle( UTIL_RD_GetCurrentLobbyData( "game:missioninfo:displaytitle" ) );
	}
	else
	{
		wchar_t wszTitle[256];
		wchar_t wszMission[256];
		const wchar_t *pwszMission = NULL;
		const char *szMission = UTIL_RD_GetCurrentLobbyData( "game:missioninfo:displaytitle" );
		if ( *szMission == '#' )
		{
			pwszMission = g_pVGuiLocalize->Find( szMission );
		}
		if ( !pwszMission )
		{
			g_pVGuiLocalize->ConvertANSIToUnicode( szMission, wszMission, sizeof( wszMission ) );
			pwszMission = wszMission;
		}
		wchar_t wszChallenge[256];
		const wchar_t *pwszChallenge = NULL;
		const char *szChallengeTitle = UTIL_RD_GetCurrentLobbyData( "game:challengeinfo:displaytitle" );
		if ( *szChallengeTitle == '#' )
		{
			pwszChallenge = g_pVGuiLocalize->Find( szChallengeTitle );
		}
		if ( !pwszChallenge )
		{
			g_pVGuiLocalize->ConvertANSIToUnicode( szChallengeTitle, wszChallenge, sizeof( wszChallenge ) );
			pwszChallenge = wszChallenge;
		}
		g_pVGuiLocalize->ConstructString( wszTitle, sizeof( wszTitle ), L"%s1: %s2", 2, pwszMission, pwszChallenge );
		m_pLeaderboard->SetTitle( wszTitle );
	}

	char szLeaderboardName[k_cchLeaderboardNameMax]{};
	g_ASW_Steamstats.SpeedRunLeaderboardName( szLeaderboardName, sizeof( szLeaderboardName ), szMap, nMapID, szChallenge, nChallengeID );
	if ( !g_ASW_Steamstats.IsLBWhitelisted( szLeaderboardName ) )
	{
		g_ASW_Steamstats.SpeedRunLeaderboardName( szLeaderboardName, sizeof( szLeaderboardName ), szMap, nMapID, "0", k_PublishedFileIdInvalid );
		m_pLeaderboard->SetTitle( UTIL_RD_GetCurrentLobbyData( "game:missioninfo:displaytitle" ) );
	}

	SteamAPICall_t hCall = SteamUserStats()->FindLeaderboard( szLeaderboardName );
	m_LeaderboardFind.Set( hCall, this, &CNB_Leaderboard_Panel::LeaderboardFind );
}

CNB_Leaderboard_Panel::~CNB_Leaderboard_Panel()
{
}

void CNB_Leaderboard_Panel::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	LoadControlSettings( "resource/ui/nb_leaderboard_panel.res" );
}

void CNB_Leaderboard_Panel::OnCommand( const char *command )
{
	if ( !Q_stricmp( command, "BackButton" ) )
	{
		MarkForDeletion();
		return;
	}
	BaseClass::OnCommand( command );
}

void CNB_Leaderboard_Panel::LeaderboardFind( LeaderboardFindResult_t *pResult, bool bIOError )
{
	if ( bIOError )
	{
		m_pErrorLabel->SetVisible( true );
		return;
	}

	if ( !pResult->m_bLeaderboardFound )
	{
		m_pNotFoundLabel->SetVisible( true );
		return;
	}

	m_pLeaderboard->SetDisplayType( SteamUserStats()->GetLeaderboardDisplayType( pResult->m_hSteamLeaderboard ) );

	SteamAPICall_t hCall = SteamUserStats()->DownloadLeaderboardEntries( pResult->m_hSteamLeaderboard, k_ELeaderboardDataRequestFriends, 0, 0 );
	m_LeaderboardDownload.Set( hCall, this, &CNB_Leaderboard_Panel::LeaderboardDownload );
}

void CNB_Leaderboard_Panel::LeaderboardDownload( LeaderboardScoresDownloaded_t *pResult, bool bIOError )
{
	if ( bIOError )
	{
		m_pErrorLabel->SetVisible( true );
		return;
	}

	if ( pResult->m_cEntryCount == 0 )
	{
		m_pNotFoundLabel->SetVisible( true );
		return;
	}

	CUtlVector<RD_LeaderboardEntry_t> entries;
	g_ASW_Steamstats.ReadDownloadedLeaderboard( entries, pResult->m_hSteamLeaderboardEntries, pResult->m_cEntryCount );
	m_pLeaderboard->SetEntries( entries );
	m_pLeaderboard->SetScrollable( true );
}
