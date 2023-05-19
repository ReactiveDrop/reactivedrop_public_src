#include "cbase.h"
#include "rd_vgui_main_menu_hoiaf_leaderboard_entry.h"
#include "c_asw_steamstats.h"
#include "asw_util_shared.h"
#include "vgui_avatarimage.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

DECLARE_BUILD_FACTORY( CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry );
DECLARE_BUILD_FACTORY( CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry_Large );

CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry::CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry( vgui::Panel *parent, const char *panelName, vgui::Panel *pActionSignalTarget, const char *pCmd ) :
	BaseClass( parent, panelName, L"", pActionSignalTarget, pCmd )
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
	m_pLblRankNumber->SetText( UTIL_RD_CommaNumber( entry.m_nGlobalRank ) );
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
