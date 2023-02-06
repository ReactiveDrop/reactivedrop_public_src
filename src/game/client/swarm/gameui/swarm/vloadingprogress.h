//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//

#ifndef __VLOADINGPROGRESS_H__
#define __VLOADINGPROGRESS_H__

#include "basemodui.h"
#include "vgui/IScheme.h"
#include "const.h"
#include "loadingtippanel.h"
#include "rd_vgui_leaderboard_panel.h"

namespace BaseModUI {

class LoadingProgress : public CBaseModFrame
{
	DECLARE_CLASS_SIMPLE( LoadingProgress, CBaseModFrame );

public:
	enum LoadingType
	{
		LT_UNDEFINED = 0,
		LT_MAINMENU,
		LT_TRANSITION,
		LT_POSTER,
	};

	enum LoadingWindowType
	{
		LWT_LOADINGPLAQUE,
		LWT_BKGNDSCREEN,
	};

#define	NUM_LOADING_CHARACTERS	4

public:
	LoadingProgress( vgui::Panel *parent, const char *panelName, LoadingWindowType eLoadingType );
	~LoadingProgress();

	virtual void		Close();

	void				SetProgress( float progress );
	float				GetProgress();
	void				SetStatusText( const char *statusText );

	void				SetLoadingType( LoadingType loadingType );
	LoadingType			GetLoadingType();

	bool				ShouldShowPosterForLevel( KeyValues *pMissionInfo, KeyValues *pChapterInfo );
	void				SetPosterData( KeyValues *pMissionInfo, KeyValues *pChapterInfo, const char **pPlayerNames, unsigned int botFlags, const char *pszGameMode, const char *levelName );
	void				SetLeaderboardData( const char *pszLevelName, PublishedFileId_t nLevelAddon, const char *pszLevelDisplayName, const char *pszChallengeName, PublishedFileId_t nChallengeAddon, const char *pszChallengeDisplayName );


	bool				IsDrawingProgressBar( void ) { return m_bDrawProgress; }

protected:
	virtual void		OnThink();
	virtual void		OnCommand(const char *command);
	virtual void		ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void		PaintBackground();

private:
	void				SetupControlStates( void );
	void				SetupPoster( void );
	void				UpdateWorkingAnim();
	void				RearrangeNames( const char *pszCharacterOrder, const char **pPlayerNames );

	vgui::ProgressBar	*m_pProTotalProgress;
	vgui::ImagePanel	*m_pWorkingAnim;
	vgui::ImagePanel	*m_pBGImage;
	vgui::ImagePanel	*m_pPoster; 
	vgui::EditablePanel *m_pFooter;
	vgui::Label			*m_pLoadingText;
	LoadingType			m_LoadingType;
	LoadingWindowType	m_LoadingWindowType;

	bool				m_bFullscreenPoster;

	CCallResult<LoadingProgress, LeaderboardFindResult_t> m_LeaderboardFind;
	void LeaderboardFind( LeaderboardFindResult_t *pResult, bool bIOError );
	CCallResult<LoadingProgress, LeaderboardScoresDownloaded_t> m_LeaderboardDownloaded;
	void LeaderboardDownloaded( LeaderboardScoresDownloaded_t *pResult, bool bIOError );

	// Poster Data
	char				m_PlayerNames[NUM_LOADING_CHARACTERS][MAX_PLAYER_NAME_LENGTH];
	KeyValues			*m_pMissionInfo;
	KeyValues			*m_pChapterInfo;
	KeyValues			*m_pDefaultPosterDataKV;
	int					m_botFlags;
	bool				m_bValid;

	int					m_textureID_LoadingBar;
	int					m_textureID_LoadingBarBG;
	int					m_textureID_DefaultPosterImage;

	bool				m_bDrawBackground;
	bool				m_bDrawPoster;
	bool				m_bDrawProgress;
	bool				m_bDrawSpinner;

	float				m_flPeakProgress;

	float				m_flLastEngineTime;

	char				m_szGameMode[MAX_PATH];
	wchar_t				m_wszLeaderboardTitle[MAX_PATH];
	char				m_szLevelName[MAX_PATH];

	CLoadingTipPanel	*m_pTipPanel;

	vgui::Panel			*m_pLeaderboardBackground;
	CReactiveDrop_VGUI_Leaderboard_Panel *m_pLeaderboardPanel;
	vgui::ImagePanel	*m_pMissionPic;
	Color				m_PosterReflectionColor;
};

};

#endif // __VLOADINGPROGRESS_H__