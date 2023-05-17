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

class CRD_VGUI_Commander_Mini_Profile;
class CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry;
class CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry_Large;
class CReactiveDropWorkshopPreviewImage;

namespace BaseModUI {

class QuickJoinPanel;
class QuickJoinPublicPanel;

class MainMenu : public CBaseModFrame, public IBaseModFrameListener, public FlyoutMenuListener
{
	DECLARE_CLASS_SIMPLE( MainMenu, CBaseModFrame );

public:
	MainMenu( vgui::Panel *parent, const char *panelName );
	~MainMenu();

	void Activate();

	void UpdateVisibility();

	MESSAGE_FUNC_CHARPTR( OpenMainMenuJoinFailed, "OpenMainMenuJoinFailed", msg );

	//flyout menu listener
	virtual void OnNotifyChildFocus( vgui::Panel *child );
	virtual void OnFlyoutMenuClose( vgui::Panel *flyTo );
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
		DECLARE_HUD_SHEET_UV( hoiaf_top_10_quit_hover_4 ),
		DECLARE_HUD_SHEET_UV( hoiaf_top_10_quit_hover_5 ),
		DECLARE_HUD_SHEET_UV( hoiaf_top_10_quit_hover_6 ),
		DECLARE_HUD_SHEET_UV( hoiaf_top_10_quit_hover_7 ),
		DECLARE_HUD_SHEET_UV( hoiaf_top_10_quit_hover_8 ),
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
		DECLARE_HUD_SHEET_UV( top_bar ),
		DECLARE_HUD_SHEET_UV( top_bar_button_glow ),
		DECLARE_HUD_SHEET_UV( top_bar_left ),
		DECLARE_HUD_SHEET_UV( top_bar_left_logo_glow ),
		DECLARE_HUD_SHEET_UV( top_bar_left_profile_glow ),
		DECLARE_HUD_SHEET_UV( top_bar_left_settings_glow ),
		DECLARE_HUD_SHEET_UV( top_bar_right ),
		DECLARE_HUD_SHEET_UV( top_bar_right_hoiaf_glow ),
		DECLARE_HUD_SHEET_UV( top_bar_right_quit_glow ),
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

	CPanelAnimationVarAliasType( int, m_iHoIAFTimerOffset, "hoiaf_timer_offset", "-1", "proportional_int" );

protected:
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void OnCommand( const char *command );
	virtual void OnKeyCodePressed( vgui::KeyCode code );
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
	void OpenNewsURL( const char *szURL );

	enum MainMenuQuickJoinHelpText
	{
		MMQJHT_NONE,
		MMQJHT_QUICKMATCH,
		MMQJHT_QUICKSTART,
	};

	uint32 m_iLastTimerUpdate;
	int m_iQuickJoinHelpText;
	BaseModHybridButton *m_pBtnSettings;
	BaseModHybridButton *m_pBtnLogo;
	BaseModHybridButton *m_pTopButton[5];
	BaseModHybridButton *m_pBtnQuit;
	CRD_VGUI_Commander_Mini_Profile *m_pCommanderProfile;
	BaseModHybridButton *m_pBtnMultiplayer;
	BaseModHybridButton *m_pBtnSingleplayer;
	QuickJoinPanel *m_pPnlQuickJoin;
	QuickJoinPublicPanel *m_pPnlQuickJoinPublic;
	BaseModHybridButton *m_pBtnWorkshopShowcase;
	CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry *m_pTopLeaderboardEntries[10];
	BaseModHybridButton *m_pBtnHoIAFTimer;
	BaseModHybridButton *m_pBtnEventTimer[3];
	BaseModHybridButton *m_pBtnNewsShowcase;
	BaseModHybridButton *m_pBtnUpdateNotes;

	wchar_t m_wszNewsTitle[5][256]{};
	char m_szNewsURL[5][256]{};
	wchar_t m_wszEventTitle[3][256]{};
	char m_szEventURL[3][256]{};
	uint32 m_iEventStarts[3]{};
	uint32 m_iEventEnds[3]{};

	PublishedFileId_t m_iWorkshopTrendingFileID[5]{ k_PublishedFileIdInvalid, k_PublishedFileIdInvalid, k_PublishedFileIdInvalid, k_PublishedFileIdInvalid, k_PublishedFileIdInvalid };
	wchar_t m_wszWorkshopTrendingTitle[5][k_cchPublishedDocumentTitleMax]{};
	UGCHandle_t m_hWorkshopTrendingPreview[5]{ k_UGCHandleInvalid, k_UGCHandleInvalid, k_UGCHandleInvalid, k_UGCHandleInvalid, k_UGCHandleInvalid };
	CReactiveDropWorkshopPreviewImage *m_pWorkshopTrendingPreview[5]{};

	CCallResult<MainMenu, LeaderboardScoresDownloaded_t> m_HoIAFTop10Callback;
	CCallResult<MainMenu, LeaderboardScoresDownloaded_t> m_HoIAFSelfCallback;
	CCallResult<MainMenu, SteamUGCQueryCompleted_t> m_WorkshopTrendingItemsCallback;
	CCallResult<MainMenu, RemoteStorageDownloadUGCResult_t> m_WorkshopPreviewImageCallback[5];

	void OnHoIAFTop10ScoresDownloaded( LeaderboardScoresDownloaded_t *pParam, bool bIOFailure );
	void OnHoIAFSelfScoreDownloaded( LeaderboardScoresDownloaded_t *pParam, bool bIOFailure );
	void OnWorkshopTrendingItems( SteamUGCQueryCompleted_t *pParam, bool bIOFailure );
	void OnWorkshopPreviewImage( RemoteStorageDownloadUGCResult_t *pParam, bool bIOFailure );
};

}

#endif // __VMAINMENU_H__
