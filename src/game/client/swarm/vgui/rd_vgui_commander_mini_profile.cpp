#include "cbase.h"
#include "rd_vgui_commander_mini_profile.h"
#include "c_asw_steamstats.h"
#include "rd_hud_sheet.h"
#include "vgui/ISurface.h"
#include "gameui/swarm/vmainmenu.h"
#include "rd_vgui_main_menu_top_bar.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

DECLARE_BUILD_FACTORY( CRD_VGUI_Commander_Mini_Profile );

CRD_VGUI_Commander_Mini_Profile::CRD_VGUI_Commander_Mini_Profile( vgui::Panel *parent, const char *panelName ) :
	BaseClass( parent, panelName )
{
	g_RD_HUD_Sheets.VidInit();
}

CRD_VGUI_Commander_Mini_Profile::~CRD_VGUI_Commander_Mini_Profile()
{
}

void CRD_VGUI_Commander_Mini_Profile::NavigateTo()
{
	BaseClass::NavigateTo();

	if ( m_bIsButton )
		RequestFocus( 0 );
}

void CRD_VGUI_Commander_Mini_Profile::OnCursorEntered()
{
	BaseClass::OnCursorEntered();

	if ( m_bIsButton )
		RequestFocus( 0 );
}

void CRD_VGUI_Commander_Mini_Profile::PaintBackground()
{
	vgui::surface()->DrawSetColor( 255, 255, 255, 255 );
	vgui::surface()->DrawSetTexture( g_RD_HUD_Sheets.m_nCommanderProfileSheetID );

	int x0, y0, x1, y1, iTex;

	x0 = y0 = 0;
	GetSize( x1, y1 );
	iTex = CRD_HUD_Sheets::UV_profile;
	CRD_VGUI_Main_Menu_Top_Bar *pTopBar = assert_cast< CRD_VGUI_Main_Menu_Top_Bar * >( FindSiblingByName( "TopBar" ) );
	BaseModUI::MainMenu *pMainMenu = dynamic_cast< BaseModUI::MainMenu * >( GetParent() );
	if ( m_bIsButton && HasFocus() )
		iTex = CRD_HUD_Sheets::UV_profile_hover;
	else if ( pMainMenu && pMainMenu->m_pBtnMultiplayer->GetCurrentState() == BaseModUI::BaseModHybridButton::Focus )
		iTex = CRD_HUD_Sheets::UV_profile_create_lobby_hover;
	else if ( pTopBar && pTopBar->m_pBtnLogo->GetCurrentState() == BaseModUI::BaseModHybridButton::Focus )
		iTex = CRD_HUD_Sheets::UV_profile_logo_hover;
	else if ( pTopBar && pTopBar->m_pBtnSettings->GetCurrentState() == BaseModUI::BaseModHybridButton::Focus )
		iTex = CRD_HUD_Sheets::UV_profile_settings_hover;

	vgui::surface()->DrawTexturedSubRect( x0, y0, x1, y1, HUD_UV_COORDS( CommanderProfileSheet, iTex ) );
}

void CRD_VGUI_Commander_Mini_Profile::SetHoIAFData( const LeaderboardEntry_t &entry, const LeaderboardScoreDetails_Points_t &details )
{
}

void CRD_VGUI_Commander_Mini_Profile::ClearHoIAFData()
{
}
