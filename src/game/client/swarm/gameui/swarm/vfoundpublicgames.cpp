//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//
#include "cbase.h"
#ifdef IS_WINDOWS_PC
#undef INVALID_HANDLE_VALUE
#include "windows.h"
#endif
#include "tier1/fmtstr.h"
#include "vgui/ILocalize.h"
#include "EngineInterface.h"
#include "VHybridButton.h"
#include "VDropDownMenu.h"
#include "VFlyoutMenu.h"
#include "vgamesettings.h"

#include "vfoundpublicgames.h"

#include "nb_header_footer.h"

#include "rd_missions_shared.h"
#include "asw_util_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;
using namespace BaseModUI;

//=============================================================================
static ConVar ui_public_lobby_filter_difficulty2( "ui_public_lobby_filter_difficulty2", "", FCVAR_ARCHIVE, "Filter type for difficulty on the public lobby display" );
static ConVar ui_public_lobby_filter_onslaught( "ui_public_lobby_filter_onslaught", "", FCVAR_ARCHIVE, "Filter type for Onslaught mode on the public lobby display");
static ConVar ui_public_lobby_filter_distance( "ui_public_lobby_filter_distance", "worldwide", FCVAR_ARCHIVE, "Filter type for distance on the public lobby display" );
static ConVar ui_public_lobby_filter_challenge( "ui_public_lobby_filter_challenge", "", FCVAR_ARCHIVE, "Filter type for challenge on the public lobby display" );
static ConVar ui_public_lobby_filter_deathmatch( "ui_public_lobby_filter_deathmatch", "", FCVAR_ARCHIVE, "Filter type for deathmatch on the public lobby display" );
ConVar ui_public_lobby_filter_campaign( "ui_public_lobby_filter_campaign", "", FCVAR_ARCHIVE, "Filter type for campaigns on the public lobby display" );
ConVar ui_public_lobby_filter_status( "ui_public_lobby_filter_status", "", FCVAR_ARCHIVE, "Filter type for game status on the public lobby display" );
ConVar ui_public_lobby_filter_dedicated_servers( "ui_public_lobby_filter_dedicated_servers", "0", FCVAR_NONE, "Filter dedicated servers from the public lobby display" );
extern ConVar rd_lobby_ping_low;
extern ConVar rd_lobby_ping_high;

static void FoundPublicGamesLobbiesFunc( const CUtlVector<CSteamID> & lobbies )
{
	FoundPublicGames *pFPG = assert_cast<FoundPublicGames *>( CBaseModPanel::GetSingleton().GetWindow( WT_FOUNDPUBLICGAMES ) );
	if ( pFPG )
	{
		pFPG->m_Lobbies = lobbies;
		pFPG->UpdateGameDetails();
	}
}

//=============================================================================
FoundPublicGames::FoundPublicGames( Panel *parent, const char *panelName ) :
	BaseClass( parent, panelName ),
	m_LobbySearch( "FoundPublicGames::m_LobbySearch" )
{
	m_drpDifficulty = NULL;
	m_drpOnslaught = NULL;
	m_drpGameStatus = NULL;
	m_drpCampaign = NULL;
	m_drpDistance = NULL;
	m_drpChallenge = NULL;
	m_drpDeathmatch = NULL;

	m_pSupportRequiredDetails = NULL;
	m_pInstallSupportBtn = NULL;

	m_numCurrentPlayers = 0;

	// increase footer tall by 10 to fit Advanced button into it
	CNB_Header_Footer *pHeaderFooter = dynamic_cast< CNB_Header_Footer* >( FindChildByName( "HeaderFooter" ) );
	pHeaderFooter->SetGradientBarPos( 80, 325 );

	m_LobbySearch.Subscribe( &FoundPublicGamesLobbiesFunc );
}

FoundPublicGames::~FoundPublicGames()
{
	m_LobbySearch.Unsubscribe( &FoundPublicGamesLobbiesFunc );
}

//=============================================================================
static void SetDropDownMenuVisible( DropDownMenu *btn, bool visible )
{
	if ( btn )
	{
		if ( FlyoutMenu *flyout = btn->GetCurrentFlyout() )
		{
			flyout->CloseMenu( NULL );
		}

		btn->SetVisible( visible );
	}
}

//=============================================================================
static void AdjustButtonY( Panel *btn, int yOffset )
{
	if ( btn )
	{
		int x, y;
		btn->GetPos( x, y );
		btn->SetPos( x, y + yOffset );
	}
}

//=============================================================================

void FoundPublicGames::UpdateFilters( bool newState )
{
}

//=============================================================================

void FoundPublicGames::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	m_drpDifficulty = dynamic_cast< DropDownMenu* >( FindChildByName( "DrpFilterDifficulty" ) );
	m_drpOnslaught = dynamic_cast< DropDownMenu* >( FindChildByName( "DrpFilterOnslaught" ) );
	m_drpGameStatus = dynamic_cast< DropDownMenu* >( FindChildByName( "DrpFilterGameStatus" ) );
	m_drpCampaign = dynamic_cast< DropDownMenu* >( FindChildByName( "DrpFilterCampaign" ) );
	m_drpDistance = dynamic_cast< DropDownMenu* >( FindChildByName( "DrpFilterDistance" ) );
	m_drpChallenge = dynamic_cast< DropDownMenu* >( FindChildByName( "DrpFilterChallenge" ) );
	m_drpDeathmatch = dynamic_cast< DropDownMenu* >( FindChildByName( "DrpFilterDeathmatch" ) );
	m_btnFilters = dynamic_cast< BaseModUI::BaseModHybridButton* >( FindChildByName( "BtnFilters" ) );

	bool bHasDifficulty = GameModeHasDifficulty( m_pDataSettings->GetString( "game/mode", "" ) );
	if ( m_drpDifficulty )
	{
		SetDropDownMenuVisible( m_drpDifficulty, bHasDifficulty );
	}

	m_pSupportRequiredDetails = dynamic_cast< vgui::Label * >( FindChildByName( "LblSupportRequiredDetails" ) );
	m_pInstallSupportBtn = dynamic_cast< BaseModUI::BaseModHybridButton * >( FindChildByName( "BtnInstallSupport" ) );

	Activate();
}

void FoundPublicGames::UpdateTitle()
{
	const char *gameMode = m_pDataSettings->GetString( "game/mode", "campaign" );

	wchar_t finalString[256] = L"";
	if ( m_numCurrentPlayers )
	{
		const wchar_t *subtitleText = g_pVGuiLocalize->Find( CFmtStr( "#L4D360UI_FoundPublicGames_Subtitle_%s", gameMode ) );
		if ( subtitleText )
		{
			wchar_t convertedString[13];
			V_snwprintf( convertedString, ARRAYSIZE( convertedString ), L"%d", m_numCurrentPlayers );
			g_pVGuiLocalize->ConstructString( finalString, sizeof( finalString ), subtitleText, 1, convertedString );
		}
	}

	//BaseClass::DrawDialogBackground( CFmtStr( "#L4D360UI_FoundPublicGames_Title_%s", gameMode ), NULL, NULL, finalString, NULL );

	m_pTitle->SetText( CFmtStr( "#L4D360UI_FoundPublicGames_Title_%s", gameMode ) );
}

//=============================================================================
void FoundPublicGames::PaintBackground()
{
	/*
	const char *gameMode = m_pDataSettings->GetString( "game/mode", "campaign" );

	wchar_t finalString[256] = L"";
	if ( m_numCurrentPlayers )
	{
		const wchar_t *subtitleText = g_pVGuiLocalize->Find( CFmtStr( "#L4D360UI_FoundPublicGames_Subtitle_%s", gameMode ) );
		if ( subtitleText )
		{
			wchar_t convertedString[13];
			V_snwprintf( convertedString, ARRAYSIZE( convertedString ), L"%d", m_numCurrentPlayers );
			g_pVGuiLocalize->ConstructString( finalString, sizeof( finalString ), subtitleText, 1, convertedString );
		}
	}

	BaseClass::DrawDialogBackground( CFmtStr( "#L4D360UI_FoundPublicGames_Title_%s", gameMode ), NULL, NULL, finalString, NULL );
	*/
}

//=============================================================================
void FoundPublicGames::StartSearching( void )
{
	m_LobbySearch.Clear();
	m_Lobbies.Purge();

	char const *szGameMode = m_pDataSettings->GetString( "game/mode", "" );
	if ( szGameMode && *szGameMode )
		m_LobbySearch.m_StringFilters.AddToTail( CReactiveDropLobbySearch::StringFilter_t( "game:mode", szGameMode ) );

	char const *szDifficulty = ui_public_lobby_filter_difficulty2.GetString();
	if ( szDifficulty && *szDifficulty && GameModeHasDifficulty( szGameMode ) )
		m_LobbySearch.m_StringFilters.AddToTail( CReactiveDropLobbySearch::StringFilter_t( "game:difficulty", szDifficulty ) );

	char const *szCampaign = ui_public_lobby_filter_campaign.GetString();
	if ( szCampaign && !Q_stricmp( szCampaign, "official" ) )
		m_LobbySearch.m_StringFilters.AddToTail( CReactiveDropLobbySearch::StringFilter_t( "game:missioninfo:official", "", k_ELobbyComparisonNotEqual ) );
	else if ( szCampaign && *szCampaign )
		m_LobbySearch.m_StringFilters.AddToTail( CReactiveDropLobbySearch::StringFilter_t( "game:missioninfo:workshop", "", k_ELobbyComparisonNotEqual ) );

	char const *szOnslaught = ui_public_lobby_filter_onslaught.GetString();
	if ( szOnslaught && *szOnslaught )
		m_LobbySearch.m_StringFilters.AddToTail( CReactiveDropLobbySearch::StringFilter_t( "game:onslaught", szOnslaught ) );

	char const *szStatus = ui_public_lobby_filter_status.GetString();
	if ( szStatus && *szStatus )
		m_LobbySearch.m_StringFilters.AddToTail( CReactiveDropLobbySearch::StringFilter_t( "game:state", szStatus ) );

	char const *szChallenge = ui_public_lobby_filter_challenge.GetString();
	if ( szChallenge && !Q_stricmp( szChallenge, "none" ) )
		m_LobbySearch.m_StringFilters.AddToTail( CReactiveDropLobbySearch::StringFilter_t( "game:challenge", "0" ) );
	else if ( szChallenge && *szChallenge )
		m_LobbySearch.m_StringFilters.AddToTail( CReactiveDropLobbySearch::StringFilter_t( "game:challenge", "0", k_ELobbyComparisonNotEqual ) );

	char const *szDeathmatch = ui_public_lobby_filter_deathmatch.GetString();
	if ( szDeathmatch && !Q_stricmp( szDeathmatch, "any" ) )
		m_LobbySearch.m_StringFilters.AddToTail( CReactiveDropLobbySearch::StringFilter_t( "game:deathmatch", "", k_ELobbyComparisonNotEqual ) );
	else if ( szDeathmatch && !Q_stricmp( szDeathmatch, "none" ) )
		m_LobbySearch.m_StringFilters.AddToTail( CReactiveDropLobbySearch::StringFilter_t( "game:challenge", "", k_ELobbyComparisonNotEqual ) );
	else if ( szDeathmatch && *szDeathmatch )
		m_LobbySearch.m_StringFilters.AddToTail( CReactiveDropLobbySearch::StringFilter_t( "game:deathmatch", szDeathmatch ) );

	if ( !Q_stricmp( ui_public_lobby_filter_distance.GetString(), "close" ) )
	{
		m_LobbySearch.m_DistanceFilter = k_ELobbyDistanceFilterClose;
	}
	else if ( !Q_stricmp( ui_public_lobby_filter_distance.GetString(), "far" ) )
	{
		m_LobbySearch.m_DistanceFilter = k_ELobbyDistanceFilterFar;
	}
	else if ( !Q_stricmp( ui_public_lobby_filter_distance.GetString(), "worldwide" ) )
	{
		m_LobbySearch.m_DistanceFilter = k_ELobbyDistanceFilterWorldwide;
	}

	m_LobbySearch.StartSearching( true );
}

//=============================================================================
bool FoundPublicGames::ShouldShowPublicGame( KeyValues *pGameDetails )
{
	DevMsg( "FoundPublicGames::ShouldShowPublicGame\n" );
	KeyValuesDumpAsDevMsg( pGameDetails );

	const char *szMode = pGameDetails->GetString( "game/mode", "campaign" );
	if ( !Q_stricmp( szMode, "campaign" ) )
	{
		char const *szCampaign = pGameDetails->GetString( "game/campaign", NULL );
		const RD_Campaign_t *pCampaign = ReactiveDropMissions::GetCampaign( szCampaign );
		bool bCampaignInstalled = pCampaign && pCampaign->Installed;

		if ( !bCampaignInstalled &&
			( !Q_stricmp( ui_public_lobby_filter_campaign.GetString(), "installedaddon" ) ||
			!Q_stricmp( ui_public_lobby_filter_campaign.GetString(), "official" ) ) )
			return false;
	}
	else if ( !Q_stricmp( szMode, "single_mission" ) )
	{
		char const *szMission = pGameDetails->GetString( "game/mission", NULL );
		const RD_Mission_t *pMission = ReactiveDropMissions::GetMission( szMission );
		bool bMissionInstalled = pMission && pMission->Installed;

		if ( !bMissionInstalled &&
			( !Q_stricmp( ui_public_lobby_filter_campaign.GetString(), "installedaddon" ) ||
			!Q_stricmp( ui_public_lobby_filter_campaign.GetString(), "official" ) ) )
			return false;
	}

	char const* szServer = pGameDetails->GetString( "options/server", "listen" );
	if ( !Q_stricmp( szServer, "dedicated" ) && ui_public_lobby_filter_dedicated_servers.GetBool() )
	{
		return false;
	}

	return true;
}

static void HandleJoinPublicGame( FoundGameListItem::Info const &fi )
{
	if ( fi.mInfoType != FoundGameListItem::FGT_PUBLICGAME )
		return;

	FoundPublicGames *pWnd = ( FoundPublicGames * ) CBaseModPanel::GetSingleton().GetWindow( WT_FOUNDPUBLICGAMES );
	if ( !pWnd )
		return;

	UTIL_RD_JoinByLobbyID( fi.mFriendXUID );
}

//=============================================================================
void FoundPublicGames::AddServersToList( void )
{
	int numItems = m_Lobbies.Count();
	for ( int i = 0; i < numItems; ++ i )
	{
		CSteamID lobby = m_Lobbies[i];
		KeyValues *pGameDetails = UTIL_RD_LobbyToLegacyKeyValues( lobby );
		if ( !pGameDetails )
			continue;
		if ( !ShouldShowPublicGame( pGameDetails ) )
			continue;

		FoundGameListItem::Info fi;

		fi.mInfoType = FoundGameListItem::FGT_PUBLICGAME;
		char const *szGameMode = pGameDetails->GetString( "game/mode", "campaign" );
		bool bGameModeChapters = GameModeIsSingleChapter( szGameMode );
		const char *szDisplayName = pGameDetails->GetString( "game/missioninfo/displaytitle" );
		if ( !szDisplayName || !szDisplayName[0] )
		{
			szDisplayName = "Unknown Mission";
		}
		Q_strncpy( fi.Name, szDisplayName, sizeof( fi.Name ) );

		fi.mIsJoinable = true;
		fi.mbInGame = true;

		fi.miPing = pGameDetails->GetInt( "server/ping", 0 );

		if ( fi.miPing == 0 )
			fi.mPing = fi.GP_NONE;
		else if ( fi.miPing < rd_lobby_ping_low.GetInt() )
			fi.mPing = fi.GP_LOW;
		else if ( fi.miPing <= rd_lobby_ping_high.GetInt() )
			fi.mPing = fi.GP_MEDIUM;
		else
			fi.mPing = fi.GP_HIGH;

		if ( !Q_stricmp( "lan", pGameDetails->GetString( "system/network", "" ) ) )
			fi.mPing = fi.GP_SYSTEMLINK;

		fi.mpGameDetails = pGameDetails->MakeCopy();
		KeyValues::AutoDelete autodelete_fi_mpGameDetails( fi.mpGameDetails );

		// Force the chapter to 1 in case there is no chapter specified
		int iChapter = fi.mpGameDetails->GetInt( "game/chapter", 0 );
		if ( !iChapter )
		{
			fi.mpGameDetails->SetInt( "game/chapter", 1 );
			fi.mpGameDetails->SetBool( "UI/nochapter", 1 );
		}

		if ( IsX360() )
		{
			// For X360 titles do not transmit campaign display title, so client needs to resolve locally
			KeyValues *pMissionInfo = NULL;
			KeyValues *pChapterInfo = NULL; //GetMapInfoRespectingAnyChapter( fi.mpGameDetails, &pMissionInfo );
			if ( pMissionInfo && pChapterInfo )
			{
				if ( bGameModeChapters )
					Q_strncpy( fi.Name, pChapterInfo->GetString( "displayname", "#L4D360UI_LevelName_Unknown" ), sizeof( fi.Name ) );
				else
					Q_strncpy( fi.Name, pMissionInfo->GetString( "displaytitle", "#L4D360UI_CampaignName_Unknown" ), sizeof( fi.Name ) );
			}
			else
			{
				Q_strncpy( fi.Name, "#L4D360UI_CampaignName_Unknown", sizeof( fi.Name ) );
				fi.mbDLC = true;
			}
		}

		fi.mFriendXUID = lobby.ConvertToUint64();

		// Check if this is actually a non-joinable game
		if ( fi.IsDLC() )
		{
			fi.mIsJoinable = false;
		}
		else if ( fi.IsDownloadable() )
		{
			fi.mIsJoinable = false;
		}
		else if ( fi.mbInGame )
		{
			char const *szHint = fi.GetNonJoinableShortHint( false );
			if ( !*szHint )
			{
				fi.mIsJoinable = true;
				fi.mpfnJoinGame = HandleJoinPublicGame;
			}
			else
			{
				fi.mIsJoinable = false;
			}
		}

		AddGameFromDetails( fi );
	}
}

bool FoundPublicGames::IsADuplicateServer( FoundGameListItem *item, FoundGameListItem::Info const &fi )
{
	// Only check UID
	FoundGameListItem::Info const &ii = item->GetFullInfo();
	if ( ii.mFriendXUID == fi.mFriendXUID )
		return true;
	else
		return false;
}

//=============================================================================
static int __cdecl FoundPublicGamesSortFunc( vgui::Panel* const *a, vgui::Panel* const *b)
{
	FoundGameListItem *fA	= dynamic_cast< FoundGameListItem* >(*a);
	FoundGameListItem *fB	= dynamic_cast< FoundGameListItem* >(*b);

	const BaseModUI::FoundGameListItem::Info &ia = fA->GetFullInfo();
	const BaseModUI::FoundGameListItem::Info &ib = fB->GetFullInfo();

	// built in first
	bool builtInA = ia.mpGameDetails->GetInt( "game/missioninfo/builtin", 0 ) != 0;
	bool builtInB = ib.mpGameDetails->GetInt( "game/missioninfo/builtin", 0 ) != 0;
	if ( builtInA != builtInB )
	{
		return ( builtInA ? -1 : 1 );
	}

	// now by swarm state
	const char *stateA = ia.mpGameDetails->GetString( "game/swarmstate", "" );
	const char *stateB = ib.mpGameDetails->GetString( "game/swarmstate", "" );
	if ( int iResult = Q_stricmp( stateA, stateB ) )
	{
		return iResult;
	}

	// now by campaign
	const char *campaignNameA = ia.mpGameDetails->GetString( "game/missioninfo/displaytitle", "" );
	const char *campaignNameB = ib.mpGameDetails->GetString( "game/missioninfo/displaytitle", "" );
	if ( int iResult = Q_stricmp( campaignNameA, campaignNameB ) )
	{
		return iResult;
	}

	// difficulty
	const char *diffA = ia.mpGameDetails->GetString( "game/difficulty", "" );
	const char *diffB = ib.mpGameDetails->GetString( "game/difficulty", "" );
	if ( int iResult = Q_stricmp( diffA, diffB ) )
	{
		return iResult;
	}

	// Sort by name in the end
	return Q_stricmp( ia.Name, ib.Name );
}

//=============================================================================
void FoundPublicGames::SortListItems()
{
	m_GplGames->SortPanelItems( FoundPublicGamesSortFunc );
}

//=============================================================================
void FoundPublicGames::OnCommand( const char *command )
{
	if( V_strcmp( command, "CreateGame" ) == 0 )
	{
		if ( !CanCreateGame() )
		{
			CBaseModPanel::GetSingleton().PlayUISound( UISOUND_INVALID );
			return;
		}

		KeyValues *pSettings = KeyValues::FromString(
			"settings",
			" system { "
			" network LIVE "
			" access friends "
			" } "
			" game { "
			" mode = "
			" campaign = "
			" mission = "
			" } "
			" options { "
			" action create "
			" } "
			);
		KeyValues::AutoDelete autodelete( pSettings );

		char const *szGameMode = "campaign";
		pSettings->SetString( "game/mode", szGameMode );
		pSettings->SetString( "game/campaign", "jacob" );
		pSettings->SetString( "game/mission", "asi-jac1-landingbay_01" );

		if ( !CUIGameData::Get()->SignedInToLive() )
		{
			pSettings->SetString( "system/network", "lan" );
			pSettings->SetString( "system/access", "public" );
		}

		if ( StringHasPrefix( szGameMode, "team" ) )
		{
			pSettings->SetString( "system/netflag", "teamlobby" );
		}
		else if ( !Q_stricmp( "custommatch", m_pDataSettings->GetString( "options/action", "" ) ) )
		{
			pSettings->SetString( "system/access", "public" );
		}

		// TCR: We need to respect the default difficulty
		pSettings->SetString( "game/difficulty", GameModeGetDefaultDifficulty( szGameMode ) );

		CBaseModPanel::GetSingleton().PlayUISound( UISOUND_ACCEPT );
		CBaseModPanel::GetSingleton().CloseAllWindows();
		CBaseModPanel::GetSingleton().OpenWindow( WT_GAMESETTINGS, NULL, true, pSettings );
	}
	else if ( !Q_stricmp( command, "StartCustomMatchSearch" ) )
	{
		if ( CheckAndDisplayErrorIfNotLoggedIn() ||
			CUIGameData::Get()->CheckAndDisplayErrorIfNotSignedInToLive( this ) )
		{
			CBaseModPanel::GetSingleton().PlayUISound( UISOUND_DENY );
			return;
		}

		KeyValues *pSettings = NULL;
		bool bDefaultSettings = true;
		if ( FoundGameListItem* gameListItem = static_cast<FoundGameListItem*>( m_GplGames->GetSelectedPanelItem() ) )
		{
			pSettings = gameListItem->GetFullInfo().mpGameDetails;
		}

		if ( !pSettings )
		{
			pSettings = new KeyValues( "settings" );
		}
		else
		{
			pSettings = pSettings->MakeCopy();
			bDefaultSettings = false;
		}

		// Take ownership of allocated/copied keyvalues
		KeyValues::AutoDelete autodelete( pSettings );
		char const *szGameMode = m_pDataSettings->GetString( "game/mode", "campaign" );

		pSettings->SetString( "system/network", "LIVE" );
		pSettings->SetString( "system/access", "public" );
		pSettings->SetString( "game/mode", szGameMode );
		pSettings->SetString( "options/action", "custommatch" );

		// TCR: We need to respect the default difficulty
		if ( bDefaultSettings && GameModeHasDifficulty( szGameMode ) )
			pSettings->SetString( "game/difficulty", GameModeGetDefaultDifficulty( szGameMode ) );

		CBaseModPanel::GetSingleton().PlayUISound( UISOUND_ACCEPT );
		CBaseModPanel::GetSingleton().CloseAllWindows();
		CBaseModPanel::GetSingleton().OpenWindow( WT_GAMESETTINGS, NULL, true, pSettings );
	}
	else if ( char const *filterDifficulty = StringAfterPrefix( command, "filter_difficulty_" ) )
	{
		ui_public_lobby_filter_difficulty2.SetValue( filterDifficulty );
		StartSearching();
	}
	else if ( char const *filterOnslaught = StringAfterPrefix( command, "filter_onslaught_" ) )
	{
		ui_public_lobby_filter_onslaught.SetValue( filterOnslaught );
		StartSearching();
	}
	else if ( char const *filterCampaign = StringAfterPrefix( command, "filter_campaign_" ) )
	{
		ui_public_lobby_filter_campaign.SetValue( filterCampaign );
		StartSearching();
	}
	else if ( char const *filterGamestatus = StringAfterPrefix( command, "filter_status_" ) )
	{
		ui_public_lobby_filter_status.SetValue( filterGamestatus );
		StartSearching();
	}
	else if ( char const *filterDistance = StringAfterPrefix( command, "filter_distance_" ) )
	{
		ui_public_lobby_filter_distance.SetValue( filterDistance );
		StartSearching();
	}
	else if ( char const *filterChallenge = StringAfterPrefix( command, "filter_challenge_" ) )
	{
		ui_public_lobby_filter_challenge.SetValue( filterChallenge );
		StartSearching();
	}
	else if ( char const *filterDeathmatch = StringAfterPrefix( command, "filter_deathmatch_" ) )
	{
		ui_public_lobby_filter_deathmatch.SetValue( filterDeathmatch );
		StartSearching();
	}
	else if ( !Q_stricmp( command, "InstallSupport" ) )
	{
		// install the add-on support package
#ifdef IS_WINDOWS_PC
		// App ID for the legacy addon data is 564
		ShellExecute ( 0, "open", "steam://install/564", NULL, 0, SW_SHOW );
#endif		
	}
	else if ( !Q_stricmp( command, "ShowAdvanced" ) )
	{
		// increase footer height to fit new drop downs
		CNB_Header_Footer *pHeaderFooter = dynamic_cast< CNB_Header_Footer* >( FindChildByName( "HeaderFooter" ) );
		pHeaderFooter->SetGradientBarPos( 80, 360 );

		SetControlVisible( "BtnAdvanced", false );
		SetControlVisible( "DrpFilterChallenge", true );
		SetControlVisible( "DrpFilterDeathmatch", true );
		SetControlVisible( "DrpFilterDistance", true );

		if ( m_drpChallenge )
		{
			m_drpChallenge->SetVisible( true );
			m_drpChallenge->NavigateTo();
		}

		FlyoutMenu::CloseActiveMenu();
	}
	else
	{
		BaseClass::OnCommand( command );
	}
}

//=============================================================================
void FoundPublicGames::Activate()
{
	BaseClass::Activate();

	if ( BaseModHybridButton *pWndCreateGame = dynamic_cast< BaseModHybridButton * >( FindChildByName( "DrpCreateGame" ) ) )
	{
		pWndCreateGame->SetVisible( CanCreateGame() );
		pWndCreateGame->SetText( CFmtStr( "#L4D360UI_FoudGames_CreateNew_%s", m_pDataSettings->GetString( "game/mode", "" ) ) );
	}

	if ( Panel *pLabelX = FindChildByName( "LblPressX" ) )
		pLabelX->SetVisible( CanCreateGame() );

	m_bShowHardcoreDifficulties = UTIL_ASW_CommanderLevelAtLeast( NULL, 15 );

	if ( m_drpDifficulty )
	{
		m_drpDifficulty->SetCurrentSelection( CFmtStr( "filter_difficulty_%s", ui_public_lobby_filter_difficulty2.GetString() ) );

		m_drpDifficulty->SetFlyoutItemEnabled( "BtnExpert", m_bShowHardcoreDifficulties );
		m_drpDifficulty->SetFlyoutItemEnabled( "BtnImba", m_bShowHardcoreDifficulties );
	}

	if ( m_drpOnslaught )
	{
		m_drpOnslaught->SetCurrentSelection( CFmtStr( "filter_onslaught_%s", ui_public_lobby_filter_onslaught.GetString() ) );
	}

	if ( m_drpGameStatus )
	{
		m_drpGameStatus->SetCurrentSelection( CFmtStr( "filter_status_%s", ui_public_lobby_filter_status.GetString() ) );
	}

	if ( m_drpCampaign )
	{
		m_drpCampaign->SetCurrentSelection( CFmtStr( "filter_campaign_%s", ui_public_lobby_filter_campaign.GetString() ) );
	}

	if ( m_drpDistance )
	{
		m_drpDistance->SetCurrentSelection( CFmtStr( "filter_distance_%s", ui_public_lobby_filter_distance.GetString() ) );
	}

	if ( m_drpChallenge )
	{
		m_drpChallenge->SetCurrentSelection( CFmtStr( "filter_challenge_%s", ui_public_lobby_filter_challenge.GetString() ) );
	}

	if ( m_drpDeathmatch )
	{
		m_drpDeathmatch->SetCurrentSelection( CFmtStr( "filter_deathmatch_%s", ui_public_lobby_filter_deathmatch.GetString() ) );
	}

#if !defined( _X360 ) && !defined( NO_STEAM )
	if ( ISteamUserStats *pSteamUserStats = SteamUserStats() )
	{
		SteamAPICall_t hSteamAPICall = pSteamUserStats->GetNumberOfCurrentPlayers();
		m_callbackNumberOfCurrentPlayers.Set( hSteamAPICall, this, &FoundPublicGames::Steam_OnNumberOfCurrentPlayers );
	}
#endif
}

bool FoundPublicGames::CanCreateGame()
{
	//char const *szGameMode = m_pDataSettings->GetString( "game/mode", NULL );
	bool bGroupServerList = !Q_stricmp( "groupserver", m_pDataSettings->GetString( "options/action", "" ) );

	//return ( szGameMode && *szGameMode && !bGroupServerList );
	return !bGroupServerList;
}

void FoundPublicGames::OnKeyCodePressed( vgui::KeyCode code )
{
	int iUserSlot = GetJoystickForCode( code );
	CBaseModPanel::GetSingleton().SetLastActiveUserId( iUserSlot );

	switch( GetBaseButtonCode( code ) )
	{
	case KEY_XBUTTON_X:
		OnCommand( "StartCustomMatchSearch" );
		break;

	default:
		BaseClass::OnKeyCodePressed( code );
		break;
	}
}

//=============================================================================
char const * FoundPublicGames::GetListHeaderText()
{
	bool bHasChapters = GameModeIsSingleChapter( m_pDataSettings->GetString( "game/mode", "campaign" ) );

	if ( bHasChapters )
		return "#L4D360UI_FoundPublicGames_Header_Survival";
	else
		return "#L4D360UI_FoundPublicGames_Header";
}

//=============================================================================
#if !defined( _X360 ) && !defined( NO_STEAM )
void FoundPublicGames::Steam_OnNumberOfCurrentPlayers( NumberOfCurrentPlayers_t *pResult, bool bError )
{
	if ( bError || !pResult->m_bSuccess )
	{
		m_numCurrentPlayers = 0;
	}
	else
	{
		m_numCurrentPlayers = pResult->m_cPlayers;
	}
}
#endif
