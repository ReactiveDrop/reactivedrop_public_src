#ifndef nb_leaderboard_panel_points_h__
#define nb_leaderboard_panel_points_h__

#include <steam/steam_api.h>
#include <vgui/VGUI.h>
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/Frame.h>
#include <vgui_bitmapbutton.h>
#include "gameui/swarm/basemodframe.h"

class vgui::Label;
class CNB_Header_Footer;
class CNB_Button;
class CReactiveDrop_VGUI_Leaderboard_Panel_Points;

class CNB_Leaderboard_Panel_Points : public BaseModUI::CBaseModFrame
{
	DECLARE_CLASS_SIMPLE( CNB_Leaderboard_Panel_Points, CBaseModFrame );
public:
	CNB_Leaderboard_Panel_Points( vgui::Panel *parent, const char *name );
	virtual ~CNB_Leaderboard_Panel_Points();

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void OnCommand( const char *command );
	virtual void OnKeyCodePressed( vgui::KeyCode code );

	CNB_Header_Footer *m_pHeaderFooter;
	CNB_Button *m_pBackButton;
	CNB_Button *m_pServerList;
	CNB_Button *m_pStatsWebsite;
	CBitmapButton *m_pToggleButton;
	vgui::Label *m_pToggleLabel;
	vgui::Panel *m_pLeaderboardBackground;
	CReactiveDrop_VGUI_Leaderboard_Panel_Points *m_pLeaderboard;
	vgui::Label *m_pErrorLabel;
	vgui::Label *m_pNotFoundLabel;
	ELeaderboardDataRequest m_iCurrentLeaderboardDisplayMode;

	CCallResult<CNB_Leaderboard_Panel_Points, LeaderboardFindResult_t> m_LeaderboardFind;
	void LeaderboardFind( LeaderboardFindResult_t *pResult, bool bIOError );
	CCallResult<CNB_Leaderboard_Panel_Points, LeaderboardScoresDownloaded_t> m_LeaderboardDownload;
	void LeaderboardDownload( LeaderboardScoresDownloaded_t *pResult, bool bIOError );
};


#endif // nb_leaderboard_panel_points_h__
