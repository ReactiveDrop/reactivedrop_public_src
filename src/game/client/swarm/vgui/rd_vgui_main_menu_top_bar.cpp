#include "cbase.h"
#include "rd_vgui_main_menu_top_bar.h"
#include "tabbedgriddetails.h"
#include "rd_hud_sheet.h"
#include "vgui/ISurface.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

DECLARE_BUILD_FACTORY( CRD_VGUI_Main_Menu_Top_Bar );

using namespace BaseModUI;

CRD_VGUI_Main_Menu_Top_Bar::CRD_VGUI_Main_Menu_Top_Bar( vgui::Panel *parent, const char *panelName ) :
	BaseClass( parent, panelName )
{
	m_pBtnSettings = new BaseModHybridButton( this, "BtnSettings", "", this, "Settings" );
	m_pBtnLogo = new BaseModHybridButton( this, "BtnLogo", "", this, "MainMenu" );
	m_pTopButton[0] = new BaseModHybridButton( this, "BtnLoadout", "#rd_collection_inventory_loadout", this, "Loadout" );
	m_pTopButton[1] = new BaseModHybridButton( this, "BtnContracts", "#rd_mainmenu_contracts", this, "Contracts" );
	m_pTopButton[2] = new BaseModHybridButton( this, "BtnRecordings", "#rd_mainmenu_recordings", this, "Recordings" );
	m_pTopButton[3] = new BaseModHybridButton( this, "BtnSwarmopedia", "#rd_collection_swarmopedia", this, "Swarmopedia" );
	m_pTopButton[4] = new BaseModHybridButton( this, "BtnWorkshop", "#rd_mainmenu_workshop", this, "Workshop" );
	m_pTopButton[5] = new BaseModHybridButton( this, "BtnInventory", "#rd_mainmenu_inventory", this, "Inventory" );
	m_pBtnQuit = new BaseModHybridButton( this, "BtnQuit", "", this, "QuitGame" );

	m_bLeftGlow = false;
	m_bRightGlow = false;

	g_RD_HUD_Sheets.VidInit();
}

CRD_VGUI_Main_Menu_Top_Bar::~CRD_VGUI_Main_Menu_Top_Bar()
{
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
		BaseModPanel.OpenWindow( WT_SETTINGS, BaseModPanel.GetWindow( BaseModPanel.GetActiveWindowType() ), false );
	}
	else if ( !V_stricmp( command, "MainMenu" ) )
	{
		DismissMainMenuScreens();
		BaseModPanel.OpenFrontScreen();
	}
	else if ( !V_stricmp( command, "Loadout" ) )
	{
		DismissMainMenuScreens();
		BaseModPanel.OpenWindow( WT_LOADOUTS, BaseModPanel.GetWindow( BaseModPanel.GetActiveWindowType() ) );
	}
	else if ( !V_stricmp( command, "Contracts" ) )
	{
		DismissMainMenuScreens();
		BaseModPanel.OpenWindow( WT_CONTRACTS, BaseModPanel.GetWindow( BaseModPanel.GetActiveWindowType() ) );
	}
	else if ( !V_stricmp( command, "Recordings" ) )
	{
		DismissMainMenuScreens();
		BaseModPanel.OpenWindow( WT_DEMOS, BaseModPanel.GetWindow( BaseModPanel.GetActiveWindowType() ) );
	}
	else if ( !V_stricmp( command, "Swarmopedia" ) )
	{
		DismissMainMenuScreens();
		LaunchSwarmopediaFrame();
	}
	else if ( !V_stricmp( command, "Workshop" ) )
	{
		DismissMainMenuScreens();
		BaseModPanel.OpenWindow( WT_WORKSHOP, BaseModPanel.GetWindow( BaseModPanel.GetActiveWindowType() ) );
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
	vgui::surface()->DrawSetColor( 255, 255, 255, 255 );
	vgui::surface()->DrawSetTexture( g_RD_HUD_Sheets.m_nMainMenuSheetID );

	int x0, y0, x1, y1, iTex;

	x0 = 0;
	y0 = 0;
	x1 = YRES( 240 );
	y1 = YRES( 24 );
	iTex = CRD_HUD_Sheets::UV_top_bar_left;
	if ( m_pBtnSettings->GetCurrentState() == BaseModHybridButton::Focus )
		iTex = CRD_HUD_Sheets::UV_top_bar_left_settings_glow;
	else if ( m_pBtnLogo->GetCurrentState() == BaseModHybridButton::Focus )
		iTex = CRD_HUD_Sheets::UV_top_bar_left_logo_glow;
	else if ( m_bLeftGlow )
		iTex = CRD_HUD_Sheets::UV_top_bar_left_profile_glow;
	vgui::surface()->DrawTexturedSubRect( x0, y0, x1, y1, HUD_UV_COORDS( MainMenuSheet, iTex ) );

	x0 = x1;
	x1 = GetWide() - YRES(240);
	iTex = CRD_HUD_Sheets::UV_top_bar;
	vgui::surface()->DrawTexturedSubRect( x0, y0, x1, y1, HUD_UV_COORDS( MainMenuSheet, iTex ) );

	x0 = x1;
	x1 = GetWide();
	iTex = CRD_HUD_Sheets::UV_top_bar_right;
	if ( m_pBtnQuit->GetCurrentState() == BaseModHybridButton::Focus )
		iTex = CRD_HUD_Sheets::UV_top_bar_right_quit_glow;
	else if ( m_bRightGlow )
		iTex = CRD_HUD_Sheets::UV_top_bar_right_hoiaf_glow;
	vgui::surface()->DrawTexturedSubRect( x0, y0, x1, y1, HUD_UV_COORDS( MainMenuSheet, iTex ) );

	iTex = CRD_HUD_Sheets::UV_top_bar_button_glow;
	x0 = YRES( -64 );
	x1 = YRES( 64 );
	for ( int i = 0; i < NELEMS( m_pTopButton ); i++ )
	{
		if ( m_pTopButton[i]->GetCurrentState() == BaseModHybridButton::Focus )
		{
			int x, y, wide, tall;
			m_pTopButton[i]->GetBounds( x, y, wide, tall );
			x += wide / 2;
			x0 += x;
			x1 += x;
			vgui::surface()->DrawTexturedSubRect( x0, y0, x1, y1, HUD_UV_COORDS( MainMenuSheet, iTex ) );
			break;
		}
	}

	if ( m_pBtnSettings->IsVisible() )
	{
		iTex = CRD_HUD_Sheets::UV_settings;
		if ( m_pBtnSettings->GetCurrentState() == BaseModHybridButton::Focus )
			iTex = CRD_HUD_Sheets::UV_settings_hover;
		else if ( m_pBtnLogo->GetCurrentState() == BaseModHybridButton::Focus )
			iTex = CRD_HUD_Sheets::UV_settings_logo_hover;
		else if ( m_bLeftGlow )
			iTex = CRD_HUD_Sheets::UV_settings_profile_hover;
		m_pBtnSettings->GetBounds( x0, y0, x1, y1 );
		vgui::surface()->DrawTexturedSubRect( x0, y0, x0 + x1, y0 + y1, HUD_UV_COORDS( MainMenuSheet, iTex ) );
	}
	if ( m_pBtnLogo->IsVisible() )
	{
		iTex = CRD_HUD_Sheets::UV_logo;
		if ( m_pBtnLogo->GetCurrentState() == BaseModHybridButton::Focus )
			iTex = CRD_HUD_Sheets::UV_logo_hover;
		else if ( m_pBtnSettings->GetCurrentState() == BaseModHybridButton::Focus )
			iTex = CRD_HUD_Sheets::UV_logo_settings_hover;
		else if ( m_bLeftGlow )
			iTex = CRD_HUD_Sheets::UV_logo_profile_hover;
		m_pBtnLogo->GetBounds( x0, y0, x1, y1 );
		vgui::surface()->DrawTexturedSubRect( x0, y0, x0 + x1, y0 + y1, HUD_UV_COORDS( MainMenuSheet, iTex ) );
	}
	for ( int i = 0; i < NELEMS( m_pTopButton ); i++ )
	{
		if ( m_pTopButton[i]->IsVisible() )
		{
			iTex = CRD_HUD_Sheets::UV_top_button;
			if ( m_pTopButton[i]->GetCurrentState() == BaseModHybridButton::Focus )
				iTex = CRD_HUD_Sheets::UV_top_button_hover;
			else if ( i > 0 && m_pTopButton[i - 1]->GetCurrentState() == BaseModHybridButton::Focus )
				iTex = CRD_HUD_Sheets::UV_top_button_left_hover;
			else if ( i < NELEMS( m_pTopButton ) - 1 && m_pTopButton[i + 1]->GetCurrentState() == BaseModHybridButton::Focus )
				iTex = CRD_HUD_Sheets::UV_top_button_right_hover;
			else if ( i == 0 && m_bLeftGlow )
				iTex = CRD_HUD_Sheets::UV_top_button_profile_hover;
			m_pTopButton[i]->GetBounds( x0, y0, x1, y1 );
			vgui::surface()->DrawTexturedSubRect( x0, y0, x0 + x1, y0 + y1, HUD_UV_COORDS( MainMenuSheet, iTex ) );
		}
	}
	if ( m_pBtnQuit->IsVisible() )
	{
		iTex = CRD_HUD_Sheets::UV_quit;
		if ( m_pBtnQuit->GetCurrentState() == BaseModHybridButton::Focus )
			iTex = CRD_HUD_Sheets::UV_quit_hover;
		m_pBtnQuit->GetBounds( x0, y0, x1, y1 );
		vgui::surface()->DrawTexturedSubRect( x0, y0, x0 + x1, y0 + y1, HUD_UV_COORDS( MainMenuSheet, iTex ) );
	}
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

extern vgui::DHANDLE<TabbedGridDetails> g_hCollectionFrame;
void CRD_VGUI_Main_Menu_Top_Bar::DismissMainMenuScreens()
{
	if ( g_hCollectionFrame )
	{
		g_hCollectionFrame->SetVisible( false );
		g_hCollectionFrame->MarkForDeletion();
		g_hCollectionFrame = NULL;
	}
}
