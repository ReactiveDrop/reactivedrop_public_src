#include "cbase.h"
#include "rd_vgui_main_menu_hoiaf_leaderboard_entry.h"
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include "c_asw_steamstats.h"
#include "asw_util_shared.h"
#include "vgui_avatarimage.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace BaseModUI;

DECLARE_BUILD_FACTORY( CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry );
DECLARE_BUILD_FACTORY( CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry_Large );

CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry::CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry( vgui::Panel *parent, const char *panelName, vgui::Panel *pActionSignalTarget, const char *pCmd ) :
	BaseClass( parent, panelName, "", pActionSignalTarget, pCmd )
{
	m_pAvatar = new CAvatarImagePanel( this, "PnlAvatar" );
	m_pAvatar->SetMouseInputEnabled( false );
	m_pLblRankNumber = new vgui::Label( this, "LblRankNumber", L"" );
	m_pLblRankNumber->SetMouseInputEnabled( false );
	m_pLblScore = new vgui::Label( this, "LblScore", L"" );
	m_pLblScore->SetMouseInputEnabled( false );
}

CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry::~CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry()
{
}

void CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry::PerformLayout()
{
	m_pAvatar->m_bLegacyPadding = false;
	m_pAvatar->SetPos( YRES( 1 ), YRES( 1 ) );
	m_pAvatar->SetSize( GetTall() - YRES( 2 ), GetTall() - YRES( 1 ) );
	m_pLblRankNumber->SetContentAlignment( vgui::Label::a_southeast );
	m_pLblRankNumber->SetPos( GetWide() - YRES( 50 ), 0);
	m_pLblRankNumber->SetSize( YRES( 50 ), GetTall() );
	m_pLblScore->SetContentAlignment( vgui::Label::a_northeast );
	m_pLblScore->SetPos( GetWide() - GetTall() * 3 - YRES( 1 ), 0 );
	m_pLblScore->SetSize( GetTall() * 3, GetTall() );

	BaseClass::PerformLayout();
}

void CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry::ApplySettings( KeyValues *pSettings )
{
	BaseClass::ApplySettings( pSettings );

	vgui::IScheme *pScheme = vgui::scheme()->GetIScheme( GetScheme() );
	Assert( pScheme );
	if ( pScheme )
	{
		m_pLblRankNumber->SetFont( pScheme->GetFont( "DefaultVerySmall", true ) );
		m_hTextFont = pScheme->GetFont( "DefaultMedium", true );
		m_hTextBlurFont = pScheme->GetFont( "DefaultMediumBlur", true );
		m_nTextFontHeight = vgui::surface()->GetFontTall( m_hTextFont );
		SetFont( m_hTextFont );
	}

	m_textInsetX = GetTall();
	SetTextInset( m_textInsetX * 2, m_textInsetY * 2 );

	m_nStyle = BUTTON_REACTIVEDROPMAINMENUHOIAF;
}

void CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry::SetFgColor( Color color )
{
	BaseClass::SetFgColor( color );
	m_pLblRankNumber->SetFgColor( color );
	m_pLblScore->SetFgColor( color );
}

void CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry::SetFromEntry( const LeaderboardEntry_t &entry, const LeaderboardScoreDetails_Points_t &details )
{
	m_SteamID = entry.m_steamIDUser;

	ISteamFriends *pFriends = SteamFriends();
	Assert( pFriends );
	if ( pFriends )
	{
		wchar_t wszName[k_cwchPersonaNameMax];
		V_UTF8ToUnicode( pFriends->GetFriendPersonaName( m_SteamID ), wszName, sizeof( wszName ) );
		SetText( wszName );
	}

	m_pAvatar->SetAvatarBySteamID( &m_SteamID );
	wchar_t wszRankNumber[32];
	g_pVGuiLocalize->ConstructString( wszRankNumber, sizeof( wszRankNumber ), g_pVGuiLocalize->Find( "#rd_main_menu_hoiaf_place_number" ), 1, UTIL_RD_CommaNumber( entry.m_nGlobalRank ) );
	m_pLblRankNumber->SetText( wszRankNumber );
	m_pLblScore->SetText( UTIL_RD_CommaNumber( entry.m_nScore ) );
}

void CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry::ClearData()
{
	m_SteamID.Clear();
	SetText( L"" );
	m_pAvatar->SetAvatarBySteamID( NULL );
	m_pLblRankNumber->SetText( L"" );
	m_pLblScore->SetText( L"" );
}

CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry_Large::CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry_Large( vgui::Panel *parent, const char *panelName, vgui::Panel *pActionSignalTarget, const char *pCmd ) :
	BaseClass( parent, panelName, pActionSignalTarget, pCmd )
{
}

CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry_Large::~CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry_Large()
{
}

void CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry_Large::ApplySettings( KeyValues *pSettings )
{
	BaseClass::ApplySettings( pSettings );

	vgui::IScheme *pScheme = vgui::scheme()->GetIScheme( GetScheme() );
	Assert( pScheme );
	if ( pScheme )
	{
		m_pLblScore->SetFont( pScheme->GetFont( "DefaultMedium", true ) );
		m_pLblRankNumber->SetFont( pScheme->GetFont( "DefaultSmall", true ) );
		m_hTextFont = pScheme->GetFont( "DefaultBold", true );
		m_hTextBlurFont = pScheme->GetFont( "DefaultBoldBlur", true );
		m_nTextFontHeight = vgui::surface()->GetFontTall( m_hTextFont );
		SetFont( m_hTextFont );
	}
}
