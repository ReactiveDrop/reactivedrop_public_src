#pragma once

#include <vgui_controls/EditablePanel.h>

struct LeaderboardScoreDetails_Points_t;

class CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry : public vgui::EditablePanel
{
public:
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry, vgui::EditablePanel );

	CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry( vgui::Panel *parent, const char *panelName );
	~CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry();

	void ClearData();
};

class CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry_Large : public CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry
{
public:
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry_Large, CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry );

	CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry_Large( vgui::Panel *parent, const char *panelName );
	~CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry_Large();
};
