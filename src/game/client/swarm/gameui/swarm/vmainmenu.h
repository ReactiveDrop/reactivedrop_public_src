//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//

#ifndef __VMAINMENU_H__
#define __VMAINMENU_H__

#include "basemodui.h"
#include "VFlyoutMenu.h"

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

	CPanelAnimationVarAliasType( int, m_iHoIAFTimerOffset, "hoiaf_timer_offset", "-1", "proportional_int" );

protected:
	virtual void LoadLayout();
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
	bool m_bIsStub{};

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

	CCallResult<MainMenu, LeaderboardScoresDownloaded_t> m_HoIAFTop10Callback;
	CCallResult<MainMenu, SteamUGCQueryCompleted_t> m_WorkshopTrendingItemsCallback;
	CCallResult<MainMenu, RemoteStorageDownloadUGCResult_t> m_WorkshopPreviewImageCallback[5];

	void OnHoIAFTop10ScoresDownloaded( LeaderboardScoresDownloaded_t *pParam, bool bIOFailure );
	void OnWorkshopTrendingItems( SteamUGCQueryCompleted_t *pParam, bool bIOFailure );
	void OnWorkshopPreviewImage( RemoteStorageDownloadUGCResult_t *pParam, bool bIOFailure );
};

}

#endif // __VMAINMENU_H__
