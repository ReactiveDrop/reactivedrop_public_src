#pragma once

#include <steam/steam_api.h>
#include <vgui/VGUI.h>
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/Frame.h>

class vgui::Label;
class CNB_Header_Footer;
class CNB_Button;
class CReactiveDrop_VGUI_Leaderboard_Panel;

class CNB_Leaderboard_Panel : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CNB_Leaderboard_Panel, vgui::EditablePanel );
public:
	CNB_Leaderboard_Panel( vgui::Panel *parent, const char *name );
	virtual ~CNB_Leaderboard_Panel();
	
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void OnCommand( const char *command );

	CNB_Header_Footer *m_pHeaderFooter;
	CNB_Button	*m_pBackButton;
	vgui::Panel *m_pLeaderboardBackground;
	CReactiveDrop_VGUI_Leaderboard_Panel *m_pLeaderboard;
	vgui::Label *m_pErrorLabel;
	vgui::Label *m_pNotFoundLabel;

	CCallResult<CNB_Leaderboard_Panel, LeaderboardFindResult_t> m_LeaderboardFind;
	void LeaderboardFind( LeaderboardFindResult_t *pResult, bool bIOError );
	CCallResult<CNB_Leaderboard_Panel, LeaderboardScoresDownloaded_t> m_LeaderboardDownload;
	void LeaderboardDownload( LeaderboardScoresDownloaded_t *pResult, bool bIOError );
};
