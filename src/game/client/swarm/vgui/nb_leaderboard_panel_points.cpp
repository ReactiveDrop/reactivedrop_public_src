#include "cbase.h"
#include "nb_leaderboard_panel_points.h"
#include "nb_header_footer.h"
#include "nb_button.h"
#include "rd_vgui_leaderboard_panel.h"
#include "rd_lobby_utils.h"
#include "c_asw_steamstats.h"
#include "vgui/ILocalize.h"
#include "asw_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar rd_leaderboard_by_difficulty;

DECLARE_BUILD_FACTORY( CNB_Leaderboard_Panel_Points );

CNB_Leaderboard_Panel_Points::CNB_Leaderboard_Panel_Points( vgui::Panel *parent, const char *name ) : BaseClass( parent, name )
{
	GameUI().PreventEngineHideGameUI();

	SetDeleteSelfOnClose( true );
	SetProportional( true );

	m_pHeaderFooter = new CNB_Header_Footer( this, "HeaderFooter" );
	m_pBackButton = new CNB_Button( this, "BackButton", "", this, "BackButton" );

	m_pHeaderFooter->SetTitle( "#nb_leaderboard" );

	m_pLeaderboardBackground = new vgui::Panel( this, "LeaderboardBackground" );
	m_pLeaderboard = new CReactiveDrop_VGUI_Leaderboard_Panel_Points( this, "Leaderboard" );

	m_pErrorLabel = new vgui::Label( this, "ErrorLabel", "" );
	m_pNotFoundLabel = new vgui::Label( this, "NotFoundLabel", "" );

	m_pLeaderboard->SetTitle( "#asw_iaf_heroes_title" );

	SteamAPICall_t hCall = steamapicontext->SteamUserStats()->FindLeaderboard( "RD_1PLAYERS_SEASON_POINTS" );
	m_LeaderboardFind.Set( hCall, this, &CNB_Leaderboard_Panel_Points::LeaderboardFind );
}

CNB_Leaderboard_Panel_Points::~CNB_Leaderboard_Panel_Points()
{
}

void CNB_Leaderboard_Panel_Points::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	LoadControlSettings( "resource/ui/nb_leaderboard_panel.res" );
}

void CNB_Leaderboard_Panel_Points::OnCommand( const char *command )
{
	if ( !Q_stricmp( command, "BackButton" ) )
	{
		OnKeyCodePressed( ButtonCodeToJoystickButtonCode( KEY_XBUTTON_B, BaseModUI::CBaseModPanel::GetSingleton().GetLastActiveUserId() ) );
		return;
	}
	BaseClass::OnCommand( command );
}

void CNB_Leaderboard_Panel_Points::LeaderboardFind( LeaderboardFindResult_t *pResult, bool bIOError )
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

	SteamAPICall_t hCall = steamapicontext->SteamUserStats()->DownloadLeaderboardEntries( pResult->m_hSteamLeaderboard, k_ELeaderboardDataRequestGlobal, 0, 100 );
	m_LeaderboardDownload.Set( hCall, this, &CNB_Leaderboard_Panel_Points::LeaderboardDownload );
}

void CNB_Leaderboard_Panel_Points::LeaderboardDownload( LeaderboardScoresDownloaded_t *pResult, bool bIOError )
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

	CUtlVector<RD_LeaderboardEntry_Points_t> entries;
	g_ASW_Steamstats.ReadDownloadedLeaderboard( entries, pResult->m_hSteamLeaderboardEntries, pResult->m_cEntryCount );
	m_pLeaderboard->SetEntries( entries );
	m_pLeaderboard->SetScrollable( true );
}
