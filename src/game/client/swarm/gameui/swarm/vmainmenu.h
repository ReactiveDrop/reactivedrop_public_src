//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//

#ifndef __VMAINMENU_H__
#define __VMAINMENU_H__

#include "basemodui.h"
#include "VFlyoutMenu.h"
#include "asw_hud_master.h"

namespace BaseModUI {

class QuickJoinPanel;
class QuickJoinGroupsPanel;

class MainMenu : public CBaseModFrame, public IBaseModFrameListener, public FlyoutMenuListener
{
	DECLARE_CLASS_SIMPLE( MainMenu, CBaseModFrame );

public:
	MainMenu(vgui::Panel *parent, const char *panelName);
	~MainMenu();

	void Activate();

	void UpdateVisibility();

	MESSAGE_FUNC_CHARPTR( OpenMainMenuJoinFailed, "OpenMainMenuJoinFailed", msg );
	
	//flyout menu listener
	virtual void OnNotifyChildFocus( vgui::Panel* child );
	virtual void OnFlyoutMenuClose( vgui::Panel* flyTo );
	virtual void OnFlyoutMenuCancelled();

	DECLARE_HUD_SHEET( MainMenuSheet )
		DECLARE_HUD_SHEET_UV( create_lobby ),
		DECLARE_HUD_SHEET_UV( create_lobby_hover ),
		DECLARE_HUD_SHEET_UV( create_lobby_profile_hover ),
		DECLARE_HUD_SHEET_UV( create_lobby_singleplayer_hover ),
		DECLARE_HUD_SHEET_UV( event_timer ),
		DECLARE_HUD_SHEET_UV( event_timer_above_hover ),
		DECLARE_HUD_SHEET_UV( event_timer_below_hover ),
		DECLARE_HUD_SHEET_UV( event_timer_hoiaf_timer_hover ),
		DECLARE_HUD_SHEET_UV( event_timer_hover ),
		DECLARE_HUD_SHEET_UV( event_timer_news_hover ),
		DECLARE_HUD_SHEET_UV( hoiaf_timer ),
		DECLARE_HUD_SHEET_UV( hoiaf_timer_event_timer_hover ),
		DECLARE_HUD_SHEET_UV( hoiaf_timer_hoiaf_top_10_hover ),
		DECLARE_HUD_SHEET_UV( hoiaf_timer_hover ),
		DECLARE_HUD_SHEET_UV( hoiaf_top_1 ),
		DECLARE_HUD_SHEET_UV( hoiaf_top_10 ),
		DECLARE_HUD_SHEET_UV( hoiaf_top_10_above_hover ),
		DECLARE_HUD_SHEET_UV( hoiaf_top_10_below_hover ),
		DECLARE_HUD_SHEET_UV( hoiaf_top_10_hoiaf_timer_hover ),
		DECLARE_HUD_SHEET_UV( hoiaf_top_10_hover ),
		DECLARE_HUD_SHEET_UV( hoiaf_top_10_quit_hover_1 ),
		DECLARE_HUD_SHEET_UV( hoiaf_top_10_quit_hover_2 ),
		DECLARE_HUD_SHEET_UV( hoiaf_top_10_quit_hover_3 ),
		DECLARE_HUD_SHEET_UV( hoiaf_top_1_below_hover ),
		DECLARE_HUD_SHEET_UV( hoiaf_top_1_hover ),
		DECLARE_HUD_SHEET_UV( hoiaf_top_1_quit_hover ),
		DECLARE_HUD_SHEET_UV( logo ),
		DECLARE_HUD_SHEET_UV( logo_hover ),
		DECLARE_HUD_SHEET_UV( logo_profile_hover ),
		DECLARE_HUD_SHEET_UV( logo_settings_hover ),
		DECLARE_HUD_SHEET_UV( news ),
		DECLARE_HUD_SHEET_UV( news_event_timer_hover ),
		DECLARE_HUD_SHEET_UV( news_hover ),
		DECLARE_HUD_SHEET_UV( news_update_hover ),
		DECLARE_HUD_SHEET_UV( profile ),
		DECLARE_HUD_SHEET_UV( profile_create_lobby_hover ),
		DECLARE_HUD_SHEET_UV( profile_hover ),
		DECLARE_HUD_SHEET_UV( profile_logo_hover ),
		DECLARE_HUD_SHEET_UV( profile_settings_hover ),
		DECLARE_HUD_SHEET_UV( quick_join ),
		DECLARE_HUD_SHEET_UV( quick_join_above_hover ),
		DECLARE_HUD_SHEET_UV( quick_join_below_hover ),
		DECLARE_HUD_SHEET_UV( quick_join_hover ),
		DECLARE_HUD_SHEET_UV( quick_join_singleplayer_hover ),
		DECLARE_HUD_SHEET_UV( quit ),
		DECLARE_HUD_SHEET_UV( quit_hover ),
		DECLARE_HUD_SHEET_UV( settings ),
		DECLARE_HUD_SHEET_UV( settings_hover ),
		DECLARE_HUD_SHEET_UV( settings_logo_hover ),
		DECLARE_HUD_SHEET_UV( settings_profile_hover ),
		DECLARE_HUD_SHEET_UV( singleplayer ),
		DECLARE_HUD_SHEET_UV( singleplayer_create_lobby_hover ),
		DECLARE_HUD_SHEET_UV( singleplayer_hover ),
		DECLARE_HUD_SHEET_UV( singleplayer_quick_join_hover ),
		DECLARE_HUD_SHEET_UV( ticker_left ),
		DECLARE_HUD_SHEET_UV( ticker_left_workshop_hover ),
		DECLARE_HUD_SHEET_UV( ticker_mid ),
		DECLARE_HUD_SHEET_UV( ticker_right ),
		DECLARE_HUD_SHEET_UV( ticker_right_update_hover ),
		DECLARE_HUD_SHEET_UV( top_button ),
		DECLARE_HUD_SHEET_UV( top_button_hover ),
		DECLARE_HUD_SHEET_UV( top_button_left_hover ),
		DECLARE_HUD_SHEET_UV( top_button_profile_hover ),
		DECLARE_HUD_SHEET_UV( top_button_right_hover ),
		DECLARE_HUD_SHEET_UV( update ),
		DECLARE_HUD_SHEET_UV( update_hover ),
		DECLARE_HUD_SHEET_UV( update_news_hover ),
		DECLARE_HUD_SHEET_UV( workshop ),
		DECLARE_HUD_SHEET_UV( workshop_hover ),
		DECLARE_HUD_SHEET_UV( workshop_quick_join_hover ),
	END_HUD_SHEET( MainMenuSheet );

	CUtlVector<HudSheet_t> m_HudSheets;

protected:
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void OnCommand(const char *command);
	virtual void OnKeyCodePressed(vgui::KeyCode code);
	virtual void OnThink();
	virtual void OnOpen();
	virtual void RunFrame();
	virtual void PaintBackground();

private:
	static void AcceptCommentaryRulesCallback();
	static void AcceptSplitscreenDisableCallback();
	static void AcceptVersusSoftLockCallback();
	static void AcceptQuitGameCallback();
	void SetFooterState();

	enum MainMenuQuickJoinHelpText
	{
		MMQJHT_NONE,
		MMQJHT_QUICKMATCH,
		MMQJHT_QUICKSTART,
	};

	int					m_iQuickJoinHelpText;
	BaseModHybridButton *m_pBtnSettings;
	BaseModHybridButton *m_pBtnLogo;
	BaseModHybridButton *m_pBtnLoadout;
	BaseModHybridButton *m_pBtnContracts;
	BaseModHybridButton *m_pBtnRecordings;
	BaseModHybridButton *m_pBtnSwarmopedia;
	BaseModHybridButton *m_pBtnInventory;
	BaseModHybridButton *m_pBtnQuit;
	BaseModHybridButton *m_pBtnMultiplayer;
	BaseModHybridButton *m_pBtnSingleplayer;
	QuickJoinPanel *m_pPnlQuickJoin;
	QuickJoinGroupsPanel *m_pPnlQuickJoinGroups;
};

}

#endif // __VMAINMENU_H__
