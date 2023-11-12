#include "cbase.h"
#include "rd_vgui_main_menu_top_bar.h"
#include "tabbedgriddetails.h"
#include "rd_hud_sheet.h"
#include "vgui/ISurface.h"
#include "gameui/swarm/vmainmenu.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

DECLARE_BUILD_FACTORY( CRD_VGUI_Main_Menu_Top_Bar );

using namespace BaseModUI;

static int s_nMainMenuTopBarCount = 0;

CRD_VGUI_Main_Menu_Top_Bar::CRD_VGUI_Main_Menu_Top_Bar( vgui::Panel *parent, const char *panelName ) :
	BaseClass( parent, panelName )
{
	m_pBtnSettings = new BaseModHybridButton( this, "BtnSettings", "", this, "Settings" );
	m_pBtnLogo = new BaseModHybridButton( this, "BtnLogo", "", this, "MainMenu" );
	m_pTopButton[BTN_LOADOUTS] = new BaseModHybridButton( this, "BtnLoadout", "#rd_collection_inventory_loadout", this, "Loadout" );
#ifdef RD_7A_QUESTS
	m_pTopButton[BTN_CONTRACTS] = new BaseModHybridButton( this, "BtnContracts", "#rd_mainmenu_contracts", this, "Contracts" );
#endif
	m_pTopButton[BTN_RECORDINGS] = new BaseModHybridButton( this, "BtnRecordings", "#rd_mainmenu_recordings", this, "Recordings" );
	m_pTopButton[BTN_SWARMOPEDIA] = new BaseModHybridButton( this, "BtnSwarmopedia", "#rd_collection_swarmopedia", this, "Swarmopedia" );
	m_pTopButton[BTN_WORKSHOP] = new BaseModHybridButton(this, "BtnWorkshop", "#rd_mainmenu_workshop", this, "Workshop");
	m_pTopButton[BTN_INVENTORY] = new BaseModHybridButton( this, "BtnInventory", "#rd_mainmenu_inventory", this, "Inventory" );
	m_pBtnQuit = new BaseModHybridButton( this, "BtnQuit", "", this, "QuitGame" );

	m_iLeftGlow = 0;
	m_iRightGlow = 0;

	g_RD_HUD_Sheets.VidInit();

	if ( !s_nMainMenuTopBarCount && SteamUtils() )
	{
		SteamUtils()->SetOverlayNotificationInset( 0, YRES( 24 ) );
	}
	s_nMainMenuTopBarCount++;
}

CRD_VGUI_Main_Menu_Top_Bar::~CRD_VGUI_Main_Menu_Top_Bar()
{
	Assert( s_nMainMenuTopBarCount > 0 );
	s_nMainMenuTopBarCount--;
	if ( !s_nMainMenuTopBarCount && SteamUtils() )
	{
		SteamUtils()->SetOverlayNotificationInset( 0, 0 );
	}
}

void CRD_VGUI_Main_Menu_Top_Bar::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	LoadControlSettings( "resource/ui/basemodui/mainmenutop.res" );

	m_pBtnSettings->SetNavDown( this );
	m_pBtnLogo->SetNavDown( this );
	for ( int i = 0; i < NELEMS( m_pTopButton ); i++ )
		m_pTopButton[i]->SetNavDown( this );
	m_pBtnQuit->SetNavDown( this );

	SetNavUp( m_pBtnLogo );

	// We may have loaded localization files since we first initialized.
	// Set the text again to grab updates if we have them.
	m_pTopButton[BTN_LOADOUTS]->SetText( "#rd_collection_inventory_loadout" );
#ifdef RD_7A_QUESTS
	m_pTopButton[BTN_CONTRACTS]->SetText( "#rd_mainmenu_contracts" );
#endif
	m_pTopButton[BTN_RECORDINGS]->SetText( "#rd_mainmenu_recordings" );
	m_pTopButton[BTN_SWARMOPEDIA]->SetText( "#rd_collection_swarmopedia" );
	m_pTopButton[BTN_WORKSHOP]->SetText( "#rd_mainmenu_workshop" );
	m_pTopButton[BTN_INVENTORY]->SetText( "#rd_mainmenu_inventory" );

#ifndef RD_7A_LOADOUTS
	m_pTopButton[BTN_LOADOUTS]->SetVisible( false );
#endif
}

extern void LaunchCollectionsFrame();
extern void LaunchSwarmopediaFrame();
void CRD_VGUI_Main_Menu_Top_Bar::OnCommand( const char *command )
{
	CBaseModPanel &BaseModPanel = CBaseModPanel::GetSingleton();
	if ( !V_stricmp( command, "QuitGame" ) )
	{
		DismissMainMenuScreens();

		CBaseModFrame *pMainMenu = BaseModPanel.GetWindow( WT_MAINMENU );
		Assert( pMainMenu );
		if ( pMainMenu )
		{
			pMainMenu->OnCommand( command );
		}
	}
	else if ( !V_stricmp( command, "Settings" ) )
	{
		DismissMainMenuScreens();
		BaseModPanel.OpenWindow( WT_SETTINGS, BaseModPanel.GetWindow( WT_MAINMENU ) );
	}
	else if ( !V_stricmp( command, "MainMenu" ) )
	{
		DismissMainMenuScreens();
		BaseModPanel.OpenFrontScreen();

		MainMenu *pMainMenu = assert_cast< MainMenu * >( BaseModPanel.GetWindow( WT_MAINMENU ) );
		pMainMenu->m_iInactiveHideMainMenu = 65535; // start fully slid in
	}
	else if ( !V_stricmp( command, "Loadout" ) )
	{
		DismissMainMenuScreens();
		BaseModPanel.OpenWindow( WT_LOADOUTS, BaseModPanel.GetWindow( WT_MAINMENU ) );
	}
	else if ( !V_stricmp( command, "Contracts" ) )
	{
		DismissMainMenuScreens();
		BaseModPanel.OpenWindow( WT_CONTRACTS, BaseModPanel.GetWindow( WT_MAINMENU ) );
	}
	else if ( !V_stricmp( command, "Recordings" ) )
	{
		DismissMainMenuScreens();
		BaseModPanel.OpenWindow( WT_DEMOS, BaseModPanel.GetWindow( WT_MAINMENU ) );
	}
	else if ( !V_stricmp( command, "Swarmopedia" ) )
	{
		DismissMainMenuScreens();
		LaunchSwarmopediaFrame();
	}
	else if ( !V_stricmp( command, "Workshop" ) )
	{
		DismissMainMenuScreens();
		BaseModPanel.OpenWindow( WT_WORKSHOP, BaseModPanel.GetWindow( WT_MAINMENU ) );
	}
	else if ( !V_stricmp( command, "Inventory" ) )
	{
		DismissMainMenuScreens();
		LaunchCollectionsFrame();
	}
	else
	{
		BaseClass::OnCommand( command );
	}
}

void CRD_VGUI_Main_Menu_Top_Bar::PaintBackground()
{
	int w = GetWide();
	int t = YRES( 24 );

	int p1 = YRES( 240 );
	int p2 = w - YRES( 240 );
	HUD_SHEET_DRAW_RECT( 0, 0, p1, t, MainMenuSheet, UV_top_bar_left );
	HUD_SHEET_DRAW_RECT( p1, 0, p2, t, MainMenuSheet, UV_top_bar );
	HUD_SHEET_DRAW_RECT( p2, 0, w, t, MainMenuSheet, UV_top_bar_right );

	BaseModHybridButton *pActiveButton = m_hActiveButton;

	if ( pActiveButton == m_pBtnSettings )
		HUD_SHEET_DRAW_RECT_ALPHA( 0, 0, p1, t, MainMenuAdditive, UV_top_bar_left_settings_glow, 255 );
	if ( pActiveButton == m_pBtnLogo )
		HUD_SHEET_DRAW_RECT_ALPHA( 0, 0, p1, t, MainMenuAdditive, UV_top_bar_left_logo_glow, 255 );
	if ( pActiveButton == m_pBtnQuit )
		HUD_SHEET_DRAW_RECT_ALPHA( p2, 0, w, t, MainMenuAdditive, UV_top_bar_right_quit_glow, 255 );

	HUD_SHEET_DRAW_RECT_ALPHA( 0, 0, p1, t, MainMenuAdditive, UV_top_bar_left_profile_glow, m_iLeftGlow );
	HUD_SHEET_DRAW_RECT_ALPHA( p2, 0, w, t, MainMenuAdditive, UV_top_bar_right_hoiaf_glow, m_iRightGlow );

	HUD_SHEET_DRAW_RECT_ALPHA( 0, 0, p1, t, MainMenuAdditive, UV_top_bar_left_settings_glow, m_GlowSettings.Update( m_pBtnSettings->GetCurrentState( true ) == BaseModHybridButton::Focus ) );
	HUD_SHEET_DRAW_RECT_ALPHA( 0, 0, p1, t, MainMenuAdditive, UV_top_bar_left_logo_glow, m_GlowLogo.Update( m_pBtnLogo->GetCurrentState( true ) == BaseModHybridButton::Focus ) );
	HUD_SHEET_DRAW_RECT_ALPHA( p2, 0, w, t, MainMenuAdditive, UV_top_bar_right_quit_glow, m_GlowQuit.Update( m_pBtnQuit->GetCurrentState( true ) == BaseModHybridButton::Focus ) );

	for ( int i = 0; i < NELEMS( m_pTopButton ); i++ )
	{
		int x, y, wide, tall;
		m_pTopButton[i]->GetBounds( x, y, wide, tall );
		x += wide / 2;
		int x0 = x - YRES( 64 );
		int x1 = x + YRES( 64 );

		if ( pActiveButton == m_pTopButton[i] )
			HUD_SHEET_DRAW_RECT_ALPHA( x0, 0, x1, t, MainMenuAdditive, UV_top_bar_button_glow, 255 );

		HUD_SHEET_DRAW_RECT_ALPHA( x0, 0, x1, t, MainMenuAdditive, UV_top_bar_button_glow, m_GlowTopButton[i].Update( m_pTopButton[i]->GetCurrentState( true ) == BaseModHybridButton::Focus ) );
	}

	HUD_SHEET_DRAW_PANEL( m_pBtnSettings, MainMenuSheet, UV_settings );
	HUD_SHEET_DRAW_PANEL( m_pBtnLogo, MainMenuSheet, UV_logo );
	for ( int i = 0; i < NELEMS( m_pTopButton ); i++ )
	{
		HUD_SHEET_DRAW_PANEL( m_pTopButton[i], MainMenuSheet, UV_top_button );
	}
	HUD_SHEET_DRAW_PANEL( m_pBtnQuit, MainMenuSheet, UV_quit );

	if ( pActiveButton == m_pBtnSettings )
		HUD_SHEET_DRAW_PANEL_ALPHA( m_pBtnSettings, MainMenuAdditive, UV_settings_hover, 255 );
	if ( pActiveButton == m_pBtnLogo )
		HUD_SHEET_DRAW_PANEL_ALPHA( m_pBtnSettings, MainMenuAdditive, UV_settings_logo_hover, 255 );

	if ( pActiveButton == m_pBtnLogo )
		HUD_SHEET_DRAW_PANEL_ALPHA( m_pBtnLogo, MainMenuAdditive, UV_logo_hover, 255 );
	if ( pActiveButton == m_pBtnSettings )
		HUD_SHEET_DRAW_PANEL_ALPHA( m_pBtnLogo, MainMenuAdditive, UV_logo_settings_hover, 255 );

	HUD_SHEET_DRAW_PANEL_ALPHA( m_pBtnSettings, MainMenuAdditive, UV_settings_profile_hover, m_iLeftGlow );
	HUD_SHEET_DRAW_PANEL_ALPHA( m_pBtnLogo, MainMenuAdditive, UV_logo_profile_hover, m_iLeftGlow );

	HUD_SHEET_DRAW_PANEL_ALPHA( m_pBtnSettings, MainMenuAdditive, UV_settings_hover, m_GlowSettings.Get() );
	HUD_SHEET_DRAW_PANEL_ALPHA( m_pBtnSettings, MainMenuAdditive, UV_settings_logo_hover, m_GlowLogo.Get() );

	HUD_SHEET_DRAW_PANEL_ALPHA( m_pBtnLogo, MainMenuAdditive, UV_logo_settings_hover, m_GlowSettings.Get() );
	HUD_SHEET_DRAW_PANEL_ALPHA( m_pBtnLogo, MainMenuAdditive, UV_logo_hover, m_GlowLogo.Get() );

	for ( int i = 0; i < NELEMS( m_pTopButton ); i++ )
	{
		HUD_SHEET_DRAW_PANEL_ALPHA( m_pTopButton[i], MainMenuAdditive, UV_top_button_hover, m_GlowTopButton[i].Get() );
		if ( i == 0 )
			HUD_SHEET_DRAW_PANEL_ALPHA( m_pTopButton[i], MainMenuAdditive, UV_top_button_profile_hover, m_iLeftGlow );
		if ( pActiveButton == m_pTopButton[i] )
			HUD_SHEET_DRAW_PANEL_ALPHA( m_pTopButton[i], MainMenuAdditive, UV_top_button_hover, 255 );
		if ( i > 0 )
			HUD_SHEET_DRAW_PANEL_ALPHA( m_pTopButton[i], MainMenuAdditive, UV_top_button_left_hover, m_GlowTopButton[i - 1].Get() );
		if ( i < NELEMS( m_pTopButton ) - 1 )
			HUD_SHEET_DRAW_PANEL_ALPHA( m_pTopButton[i], MainMenuAdditive, UV_top_button_right_hover, m_GlowTopButton[i + 1].Get() );
		if ( i > 0 && pActiveButton == m_pTopButton[i - 1] )
			HUD_SHEET_DRAW_PANEL_ALPHA( m_pTopButton[i], MainMenuAdditive, UV_top_button_left_hover, 255 );
		if ( i < NELEMS( m_pTopButton ) - 1 && pActiveButton == m_pTopButton[i + 1] )
			HUD_SHEET_DRAW_PANEL_ALPHA( m_pTopButton[i], MainMenuAdditive, UV_top_button_right_hover, 255 );
	}

	HUD_SHEET_DRAW_PANEL_ALPHA( m_pBtnQuit, MainMenuAdditive, UV_quit_hover, m_GlowQuit.Get() );
	if ( pActiveButton == m_pBtnQuit )
		HUD_SHEET_DRAW_PANEL_ALPHA( m_pBtnQuit, MainMenuAdditive, UV_quit_hover, 255 );
}

void CRD_VGUI_Main_Menu_Top_Bar::NavigateTo()
{
	// navigate twice so we end up on a button
	if ( GetLastNavDirection() == ND_DOWN && NavigateDown() )
	{
		return;
	}

	if ( GetLastNavDirection() == ND_UP && NavigateUp() )
	{
		return;
	}

	BaseClass::NavigateTo();
}

void CRD_VGUI_Main_Menu_Top_Bar::DismissMainMenuScreens()
{
	CBaseModPanel::GetSingleton().CloseAllWindows();
	CBaseModPanel::GetSingleton().OpenFrontScreen();
}
