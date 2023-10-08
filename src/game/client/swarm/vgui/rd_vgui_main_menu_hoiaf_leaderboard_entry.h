#pragma once

#include "swarm/gameui/swarm/vhybridbutton.h"
#include "rd_hud_glow_helper.h"

class CAvatarImagePanel;
struct LeaderboardEntry_t;
struct LeaderboardScoreDetails_Points_t;

namespace BaseModUI
{
class CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry : public BaseModHybridButton
{
public:
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry, BaseModHybridButton );

	CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry( vgui::Panel *parent, const char *panelName, vgui::Panel *pActionSignalTarget = NULL, const char *pCmd = NULL );
	~CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry();

	void PerformLayout() override;
	void ApplySettings( KeyValues *pSettings ) override;
	void SetFgColor( Color color ) override;

	void SetFromEntry( const LeaderboardEntry_t &entry, const LeaderboardScoreDetails_Points_t &details );
	void ClearData();

	CSteamID m_SteamID;
	vgui::Label *m_pLblRankNumber;
	vgui::Label *m_pLblScore;
	CAvatarImagePanel *m_pAvatar;
	HUDGlowHelper_t m_GlowHover;
};

class CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry_Large : public CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry
{
public:
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry_Large, CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry );

	CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry_Large( vgui::Panel *parent, const char *panelName, vgui::Panel *pActionSignalTarget = NULL, const char *pCmd = NULL );
	~CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry_Large();

	void ApplySettings( KeyValues *pSettings ) override;
	vgui::Panel *NavigateUp() override;
};
}
