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

using namespace BaseModUI;

DECLARE_BUILD_FACTORY( CNB_Leaderboard_Panel_Points );

#define HUMAN_BUTTON_ICON "vgui/briefing/human_icon"

CNB_Leaderboard_Panel_Points::CNB_Leaderboard_Panel_Points( vgui::Panel *parent, const char *name ) : BaseClass( parent, name )
{
	GameUI().PreventEngineHideGameUI();

	SetDeleteSelfOnClose( true );
	SetProportional( true );

	m_pHeaderFooter = new CNB_Header_Footer( this, "HeaderFooter" );
	m_pBackButton = new CNB_Button( this, "BackButton", "", this, "BackButton" );
	m_pBackButton->SetControllerButton( KEY_XBUTTON_B );
	m_pServerList = new CNB_Button( this, "ServerList", "", this, "ServerList" );
	m_pServerList->SetControllerButton( KEY_XBUTTON_X );
	m_pStatsWebsite = new CNB_Button( this, "StatsWebsite", "", this, "StatsWebsite" );

	m_pToggleButton = new CBitmapButton(this, "ToggleButton", "");
	m_pToggleButton->AddActionSignalTarget(this);
	m_pToggleButton->SetCommand("ToggleButton");

	m_pHeaderFooter->SetTitle( "#nb_leaderboard" );

	m_pLeaderboardBackground = new vgui::Panel( this, "LeaderboardBackground" );
	m_pLeaderboard = new CReactiveDrop_VGUI_Leaderboard_Panel_Points( this, "Leaderboard" );

	m_pErrorLabel = new vgui::Label( this, "ErrorLabel", "" );
	m_pNotFoundLabel = new vgui::Label( this, "NotFoundLabel", "" );

	m_pLeaderboard->SetTitle( "#asw_iaf_heroes_title" );

	m_iCurrentLeaderboardDisplayMode = k_ELeaderboardDataRequestGlobal;

	SteamAPICall_t hCall = SteamUserStats()->FindLeaderboard( "RD_1PLAYERS_SEASON_POINTS" );
	m_LeaderboardFind.Set( hCall, this, &CNB_Leaderboard_Panel_Points::LeaderboardFind );
}

CNB_Leaderboard_Panel_Points::~CNB_Leaderboard_Panel_Points()
{
	GameUI().AllowEngineHideGameUI();
}

void CNB_Leaderboard_Panel_Points::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	LoadControlSettings( "resource/ui/nb_leaderboard_panel.res" );

	m_pServerList->SetVisible( true );
	m_pStatsWebsite->SetVisible( true );

	color32 white;
	white.r = 255;
	white.g = 255;
	white.b = 255;
	white.a = 255;

	color32 grey;
	grey.r = 190;
	grey.g = 190;
	grey.b = 190;
	grey.a = 255;

	m_pToggleButton->SetImage( CBitmapButton::BUTTON_ENABLED, HUMAN_BUTTON_ICON, grey );
	m_pToggleButton->SetImage( CBitmapButton::BUTTON_DISABLED, HUMAN_BUTTON_ICON, grey );
	m_pToggleButton->SetImage( CBitmapButton::BUTTON_PRESSED, HUMAN_BUTTON_ICON, white );
	m_pToggleButton->SetImage( CBitmapButton::BUTTON_ENABLED_MOUSE_OVER, HUMAN_BUTTON_ICON, white );
	m_pToggleButton->SetVisible( true );
}

void CNB_Leaderboard_Panel_Points::OnCommand( const char *command )
{
	if ( !Q_stricmp( command, "BackButton" ) )
	{
		OnKeyCodePressed( ButtonCodeToJoystickButtonCode( KEY_XBUTTON_B, CBaseModPanel::GetSingleton().GetLastActiveUserId() ) );
		return;
	}
	if ( !Q_stricmp( command, "ServerList" ) )
	{
		OnKeyCodePressed( ButtonCodeToJoystickButtonCode( KEY_XBUTTON_B, CBaseModPanel::GetSingleton().GetLastActiveUserId() ) );
		CBaseModPanel::GetSingleton().OpenWindow( WT_IAFRANKSSERVERS, this, true );
		return;
	}
	if ( !V_stricmp( command, "StatsWebsite" ) )
	{
		if ( SteamApps() )
		{
			char statsWeb[256];
			V_snprintf( statsWeb, sizeof( statsWeb ), "https://stats.reactivedrop.com/heroes?lang=%s&utm_source=mainmenu",
				SteamApps()->GetCurrentGameLanguage() );
			BaseModUI::CUIGameData::Get()->ExecuteOverlayUrl( statsWeb );
		}
		return;
	}
	if ( !Q_stricmp( command, "ToggleButton" ) )
	{
		m_iCurrentLeaderboardDisplayMode--;
		if ( m_iCurrentLeaderboardDisplayMode < 0 )
			m_iCurrentLeaderboardDisplayMode = k_ELeaderboardDataRequestFriends;

		m_pLeaderboard->ClearEntries();
		SteamAPICall_t hCall = SteamUserStats()->FindLeaderboard( "RD_1PLAYERS_SEASON_POINTS" );
		m_LeaderboardFind.Set( hCall, this, &CNB_Leaderboard_Panel_Points::LeaderboardFind );

		return;
	}
	BaseClass::OnCommand( command );
}

void CNB_Leaderboard_Panel_Points::OnKeyCodePressed( vgui::KeyCode code )
{
	int joystick = GetJoystickForCode( code );
	int userId = CBaseModPanel::GetSingleton().GetLastActiveUserId();
	if ( joystick != userId || joystick < 0 )
	{
		return;
	}

	switch ( GetBaseButtonCode( code ) )
	{
	case KEY_XBUTTON_UP:
	case KEY_XSTICK1_UP:
	case KEY_XSTICK2_UP:
		m_pLeaderboard->m_gplLeaderboard->OnMouseWheeled( 10 );
		break;

	case KEY_XBUTTON_DOWN:
	case KEY_XSTICK1_DOWN:
	case KEY_XSTICK2_DOWN:
		m_pLeaderboard->m_gplLeaderboard->OnMouseWheeled( -10 );
		break;

	case KEY_XBUTTON_X:
		OnCommand( "ServerList" );
		break;

	default:
		BaseClass::OnKeyCodePressed( code );
		break;
	}
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

	SteamAPICall_t hCall;
	switch ( ELeaderboardDataRequest( m_iCurrentLeaderboardDisplayMode ) )
	{
	case k_ELeaderboardDataRequestFriends:
		hCall = SteamUserStats()->DownloadLeaderboardEntries( pResult->m_hSteamLeaderboard, k_ELeaderboardDataRequestFriends, 0, 0 );
		break;
	case k_ELeaderboardDataRequestGlobalAroundUser:
		hCall = SteamUserStats()->DownloadLeaderboardEntries( pResult->m_hSteamLeaderboard, k_ELeaderboardDataRequestGlobalAroundUser, -50, 50 );
		break;
	default:
		hCall = SteamUserStats()->DownloadLeaderboardEntries( pResult->m_hSteamLeaderboard, k_ELeaderboardDataRequestGlobal, 0, 100 );
		break;
	}

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
