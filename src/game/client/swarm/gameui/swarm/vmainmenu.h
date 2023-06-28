//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//

#ifndef __VMAINMENU_H__
#define __VMAINMENU_H__

#include "basemodui.h"
#include "VFlyoutMenu.h"
#include "rd_hud_glow_helper.h"

class CRD_VGUI_Commander_Mini_Profile;
class CRD_VGUI_Main_Menu_Top_Bar;
class CRD_VGUI_Stock_Ticker_Helper;
class CReactiveDropWorkshopPreviewImage;

namespace BaseModUI {

class CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry;
class CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry_Large;
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

	CPanelAnimationVarAliasType( int, m_iHoIAFTimerOffset, "hoiaf_timer_offset", "-1", "proportional_int" );

	void LoadLayout() override;
	void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	void OnCommand( const char *command ) override;
	void OnKeyCodePressed( vgui::KeyCode code ) override;
	void OnThink() override;
	void OnOpen() override;
	void RunFrame() override;
	void PaintBackground() override;

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
	CRD_VGUI_Main_Menu_Top_Bar *m_pTopBar;
	CRD_VGUI_Stock_Ticker_Helper *m_pStockTickerHelper;
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
	vgui::Label *m_pBranchDisclaimer{};
	bool m_bIsStub{};
	bool m_bIsLegacy{};
	bool m_bGrabPanelLocations{};
	bool m_bHavePanelLocations{};

	HUDGlowHelper_t m_GlowCreateLobby;
	HUDGlowHelper_t m_GlowSingleplayer;
	HUDGlowHelper_t m_GlowQuickJoinPublic;
	HUDGlowHelper_t m_GlowQuickJoinFriends;
	HUDGlowHelper_t m_GlowWorkshopShowcase;
	HUDGlowHelper_t m_GlowHoIAFTimer;
	HUDGlowHelper_t m_GlowEventTimer[3];
	HUDGlowHelper_t m_GlowNewsShowcase;
	HUDGlowHelper_t m_GlowUpdateNotes;

	int m_iNewsImageTexture[5]{ -1, -1, -1, -1, -1 };
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

	int m_iTargetYTopBar;
	int m_iTargetXCommanderProfile;
	int m_iTargetXCreateLobby;
	int m_iTargetXSingleplayer;
	int m_iTargetXQuickJoinPublic;
	int m_iTargetXQuickJoinFriends;
	int m_iTargetXWorkshopShowcase;
	int m_iTargetXHoIAFLeaderboard[10];
	int m_iTargetXHoIAFTimer;
	int m_iTargetXEventTimer[3];
	int m_iTargetXNewsShowcase;
	int m_iTargetXUpdateNotes;
	int m_iTargetYStockTicker;
	uint16 m_iInactiveHideMainMenu{};
	HUDGlowHelper_t m_InactiveHideQuickJoinPublic;
	HUDGlowHelper_t m_InactiveHideQuickJoinFriends;
	float m_flLastActiveTime;

	CCallResult<MainMenu, LeaderboardScoresDownloaded_t> m_HoIAFTop10Callback;
	CCallResult<MainMenu, SteamUGCQueryCompleted_t> m_WorkshopTrendingItemsCallback;
	CCallResult<MainMenu, RemoteStorageDownloadUGCResult_t> m_WorkshopPreviewImageCallback[5];

	void OnHoIAFTop10ScoresDownloaded( LeaderboardScoresDownloaded_t *pParam, bool bIOFailure );
	void OnWorkshopTrendingItems( SteamUGCQueryCompleted_t *pParam, bool bIOFailure );
	void OnWorkshopPreviewImage( RemoteStorageDownloadUGCResult_t *pParam, bool bIOFailure );
};

}

#endif // __VMAINMENU_H__
