//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//

#ifndef __VFOUNDGAMES_H__
#define __VFOUNDGAMES_H__

#include "basemodui.h"
#include "matchmaking/imatchframework.h"
#include "steam/steam_api_common.h"

struct FriendGameInfo_t;
class CNB_Header_Footer;
class CReactiveDropServerListHelper;

namespace BaseModUI {

class GenericPanelList;
class FoundGames;
class BaseModHybridButton;

class FoundGameListItem : public vgui::EditablePanel, public IBaseModFrameListener
{
	DECLARE_CLASS_SIMPLE( FoundGameListItem, vgui::EditablePanel );

public:
	enum Type_t
	{
		TYPE_UNKNOWN,
		TYPE_LOBBY,
		TYPE_SERVER,
		TYPE_LANSERVER,
		TYPE_FAVORITESERVER,
		TYPE_INSECURESERVER,
		TYPE_RANKEDSERVER,
	};

	struct Info
	{
		Type_t m_Type{ TYPE_UNKNOWN };
		CSteamID m_LobbyID; // lobby only
		servernetadr_t m_ServerIP; // server only
		CCopyableUtlVector<CSteamID> m_Friends;

		char m_szChallengeFile[64]{};
		char m_szCampaignFile[64]{}; // lobby only
		char m_szMissionFile[64]{};

		char m_szMissionImage[256]{}; // lobby only
		char m_szMissionWebsite[256]{}; // lobby only
		char m_szMissionVersion[64]{}; // lobby only
		float m_flMissionVersion{}; // lobby only

		wchar_t m_wszLobbyDisplayName[256]{}; // server only (for now)
		wchar_t m_wszChallengeDisplayName[256]{};
		wchar_t m_wszMissionDisplayName[256]{}; // lobby only
		wchar_t m_wszAuthorName[256]{}; // lobby only

		PublishedFileId_t m_iChallengeWorkshopID{ k_PublishedFileIdInvalid }; // lobby only
		PublishedFileId_t m_iMissionWorkshopID{ k_PublishedFileIdInvalid }; // lobby only
		CCopyableUtlVector<PublishedFileId_t> m_RequiredWorkshopItems; // lobby only

		bool m_bMissionBuiltIn{ false }; // comes from mission file; lobby only
		bool m_bMissionOfficial{ false }; // comes from (remote player's) game DLL; lobby only

		enum GAME_MODE { MODE_UNKNOWN, MODE_CAMPAIGN, MODE_SINGLE_MISSION } m_eGameMode{ MODE_UNKNOWN }; // lobby only
		enum GAME_STATE { STATE_BRIEFING, STATE_INGAME } m_eGameState{ STATE_INGAME };

		enum GAME_DIFFICULTY { DIFFICULTY_EASY, DIFFICULTY_NORMAL, DIFFICULTY_HARD, DIFFICULTY_INSANE, DIFFICULTY_BRUTAL } m_eGameDifficulty{ DIFFICULTY_NORMAL };
		bool m_bOnslaught{ false };
		bool m_bHardcoreFF{ false };

		enum SERVER_TYPE { SERVER_LISTEN, SERVER_DEDICATED } m_eServerType{ SERVER_LISTEN }; // SERVER_LISTEN is not used on dedicated servers (obviously)
		enum LOBBY_ACCESS { ACCESS_PUBLIC, ACCESS_FRIENDS, ACCESS_PASSWORD } m_eLobbyAccess{ ACCESS_PUBLIC }; // ACCESS_FRIENDS is lobby only; ACCESS_PASSWORD is server only
		char m_szModDir[64]{};
		char m_szNetworkVersion[32]{};
		char m_szGameBranch[32]{};
		char m_szGroupID[64]{}; // server only
		int m_iGameVersion{}; // lobby only
		int m_iMapVersion{}; // lobby only

		int m_iCurPlayers{};
		int m_iMaxPlayers{};

		SteamNetworkPingLocation_t m_PingLocation{}; // lobby only
		int m_iPingMS{};
		enum GAME_PING { PING_LOW, PING_MEDIUM, PING_HIGH, PING_SYSTEMLINK, PING_NONE } m_ePingCategory{ PING_NONE }; // PING_SYSTEMLINK means LAN server

		bool SetFromFriend( CSteamID friendID, const FriendGameInfo_t &info );
		bool SetFromLobby( CSteamID lobby );
		bool SetFromServer( CReactiveDropServerListHelper &helper, int i, FoundGameListItem::Type_t eType = FoundGameListItem::TYPE_SERVER );
		bool Merge( const Info &info );
		void SetDefaultMissionData();

		char m_szOtherTitle[64]{};

		bool IsJoinable() const;
		bool HaveMap() const;
		int CompareMapVersion() const;
		const char *IsOtherTitle() const;
		bool IsDownloadable() const;
		const char *GetNonJoinableShortHint( bool bWarnOnNoHint ) const;
		const char *GetJoinButtonHint() const;
		void SetOtherTitleFromLobby();
	};

public:
	FoundGameListItem( vgui::Panel *parent, const char *panelName );
	~FoundGameListItem();

public:
	void SetGameIndex( const Info& fi );
	const Info& GetFullInfo();

	void SetGamerTag( const wchar_t *gamerTag );

	void SetGamePing( Info::GAME_PING ping );
	void SetGameDifficulty( const char *difficultyName );
	void SetGameChallenge( const wchar_t *challengeName );
	void SetSwarmState( const char *szSwarmStateText );
	void SetGamePlayerCount( int current, int max );

	void DrawListItemLabel( vgui::Label* label, bool bSmallFont, bool bEastAligned = false );

	void SetSweep( bool sweep );
	bool IsSweep() const;

	void RunFrame() {};

	virtual void PaintBackground();
	void OnKeyCodePressed( vgui::KeyCode code );
	void OnKeyCodeTyped( vgui::KeyCode code );
	void OnMousePressed( vgui::MouseCode code );

	virtual void OnCursorEntered();
	virtual void NavigateTo( void );
	virtual void NavigateFrom( void );

	bool IsSelected( void ) { return m_bSelected; }
	void SetSelected( bool bSelected ) { m_bSelected = bSelected; }

	bool HasMouseover( void ) { return m_bHasMouseover; }
	void SetHasMouseover( bool bHasMouseover ) { m_bHasMouseover = bHasMouseover; }

	bool IsHardcoreDifficulty();

	wchar_t m_wszChallengeName[256];

protected:
	void ApplySettings( KeyValues *inResourceData );
	void ApplySchemeSettings( vgui::IScheme *pScheme );
	void PerformLayout();

protected:
	MESSAGE_FUNC( CmdJoinGame, "JoinGame" );
	void CmdViewGamercard();

private:
	Info m_FullInfo;

	Color m_OutOfFocusBgColor;
	Color m_FocusBgColor;

	GenericPanelList	*m_pListCtrlr;
	vgui::ImagePanel	*m_pImgPing;
	vgui::ImagePanel	*m_pImgPingSmall;
	vgui::Label			*m_pLblPing;
	vgui::Label			*m_pLblPlayerGamerTag;
	vgui::Label			*m_pLblDifficulty;
	vgui::Label			*m_pLblChallenge;
	vgui::Label			*m_pLblSwarmState;
	vgui::Label			*m_pLblPlayers;
	vgui::Label			*m_pLblNotJoinable;

	vgui::HFont	m_hTextFont;
	vgui::HFont	m_hTextBlurFont;

	vgui::HFont	m_hSmallTextFont;
	vgui::HFont	m_hSmallTextBlurFont;

	CPanelAnimationVar( Color, m_SelectedColor, "selected_color", "255 0 0 128" );
	bool m_sweep : 1;
	bool m_bSelected : 1;
	bool m_bHasMouseover : 1;

};

//=============================================================================

class FoundGames : public CBaseModFrame, public IBaseModFrameListener, public IMatchEventsSink
{
	DECLARE_CLASS_SIMPLE( FoundGames, CBaseModFrame );

public:
	FoundGames( vgui::Panel *parent, const char *panelName );
	~FoundGames();

	// IMatchEventsSink
public:
	virtual void OnEvent( KeyValues *pEvent );

public:
	void Activate();
	void OnCommand( const char *command );
	void OnKeyCodePressed( vgui::KeyCode code );
	void OnThink();
	void RunFrame() {};
	void PaintBackground();
	void LoadLayout();

	virtual void UpdateTitle();

#ifdef _X360
	void NavigateTo();
#endif // _X360
	
	void SetFoundDesiredText( bool bFoundGame );

	void SetDetailsPanelVisible( bool bIsVisible );

	virtual bool AddGameFromDetails( const FoundGameListItem::Info &fi, bool bOnlyMerge = false );
	void UpdateGameDetails();

	MESSAGE_FUNC_CHARPTR( OnItemSelected, "OnItemSelected", panelName );

	void ApplySchemeSettings( vgui::IScheme *pScheme );
	FoundGameListItem* GetGameItem( int index );

	virtual void OnClose();
	virtual void OnOpen();

	virtual void SetDataSettings( KeyValues *pSettings );

	void OpenPlayerFlyout( BaseModHybridButton *button, uint64 playerId, int x, int y );

	virtual void AddFakeServersToList();
	virtual void AddServersToList();
	virtual void AddFriendGamesToList( bool bMergeOnly = false );
	virtual void SortListItems();
	virtual char const * GetListHeaderText() { return NULL; }

	virtual bool IsADuplicateServer( FoundGameListItem *item, FoundGameListItem::Info const &fi );
	virtual bool MergeServerEntry( FoundGameListItem *item, FoundGameListItem::Info const &fi );
	virtual bool CanCreateGame();

	float m_flSearchStartedTime, m_flSearchEndTime;
	float m_flScheduledUpdateGameDetails;

	GenericPanelList* m_GplGames;

	float m_LastEngineSpinnerTime;
	int m_CurrentSpinnerValue;
	bool m_bShowHardcoreDifficulties;

	uint64 m_SelectedGamePlayerID;
	uint64 m_flyoutPlayerId;

	FoundGameListItem *m_pPreviousSelectedItem;

	KeyValues *m_pDataSettings;

	vgui::Label *m_pTitle;
	vgui::Label *m_pSubTitle;
	CNB_Header_Footer *m_pHeaderFooter;
};

};

#endif
