#include "cbase.h"
#include "rd_vgui_main_menu_hoiaf_leaderboard_entry.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

DECLARE_BUILD_FACTORY( CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry );
DECLARE_BUILD_FACTORY( CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry_Large );

CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry::CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry( vgui::Panel *parent, const char *panelName ) :
	BaseClass( parent, panelName )
{
}

CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry::~CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry()
{
}

void CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry::ClearData()
{
}

CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry_Large::CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry_Large( vgui::Panel *parent, const char *panelName ) :
	BaseClass( parent, panelName )
{
}

CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry_Large::~CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry_Large()
{
}
