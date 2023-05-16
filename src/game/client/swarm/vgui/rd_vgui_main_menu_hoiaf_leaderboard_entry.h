#pragma once

#include "swarm/gameui/swarm/vhybridbutton.h"

class CAvatarImagePanel;
struct LeaderboardEntry_t;
struct LeaderboardScoreDetails_Points_t;

class CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry : public BaseModUI::BaseModHybridButton
{
public:
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry, BaseModUI::BaseModHybridButton );

	CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry( vgui::Panel *parent, const char *panelName, vgui::Panel *pActionSignalTarget = NULL, const char *pCmd = NULL );
	~CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry();

	void SetFromEntry( const LeaderboardEntry_t &entry, const LeaderboardScoreDetails_Points_t &details );
	void ClearData();

	vgui::Label *m_pLblRankNumber;
	vgui::Label *m_pLblScore;
	CAvatarImagePanel *m_pAvatar;
};

class CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry_Large : public CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry
{
public:
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry_Large, CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry );

	CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry_Large( vgui::Panel *parent, const char *panelName, vgui::Panel *pActionSignalTarget = NULL, const char *pCmd = NULL );
	~CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry_Large();
};
