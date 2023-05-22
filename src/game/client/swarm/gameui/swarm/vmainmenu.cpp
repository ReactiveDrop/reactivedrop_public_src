//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//

#include "cbase.h"
#include "VMainMenu.h"
#include "EngineInterface.h"
#include "VFooterPanel.h"
#include "VHybridButton.h"
#include "VFlyoutMenu.h"
#include "vGenericConfirmation.h"
#include "VQuickJoin.h"
#include "VQuickJoinPublic.h"
#include "basemodpanel.h"
#include "UIGameData.h"
#include "VGameSettings.h"
#include "VSteamCloudConfirmation.h"
#include "vaddonassociation.h"
#include "ConfigManager.h"
#include "rd_vgui_commander_mini_profile.h"
#include "rd_vgui_main_menu_hoiaf_leaderboard_entry.h"
#include "c_asw_steamstats.h"
#include "VSignInDialog.h"
#include "VGuiSystemModuleLoader.h"
#include "VAttractScreen.h"
#include "gamemodes.h"
#include <ctime>

#include "vgui/ILocalize.h"
#include "vgui/ISystem.h"
#include "vgui_controls/Label.h"
#include "vgui_controls/Button.h"
#include "vgui_controls/Tooltip.h"
#include "vgui_controls/ImagePanel.h"
#include "vgui_controls/Image.h"

#include "steam/isteamremotestorage.h"
#include "materialsystem/materialsystem_config.h"
#include "bitmap/psheet.h"

#include "ienginevgui.h"
#include "basepanel.h"
#include "vgui/ISurface.h"
#include "tier0/icommandline.h"
#include "fmtstr.h"
#include "cdll_client_int.h"
#include "inputsystem/iinputsystem.h"
#include "asw_util_shared.h"
#include "matchmaking/swarm/imatchext_swarm.h"
#include "rd_steam_input.h"
#include "rd_workshop.h"
#include "rd_inventory_shared.h"
#include "rd_hud_sheet.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;
using namespace BaseModUI;

//=============================================================================
static ConVar connect_lobby( "connect_lobby", "", FCVAR_HIDDEN, "Sets the lobby ID to connect to on start." );
static ConVar ui_old_options_menu( "ui_old_options_menu", "0", FCVAR_HIDDEN, "Brings up the old tabbed options dialog from Keyboard/Mouse when set to 1." );
static ConVar ui_play_online_browser( "ui_play_online_browser", "1", FCVAR_RELEASE, "Whether play online displays a browser or plain search dialog." );
ConVar rd_trending_workshop_tags( "rd_trending_workshop_tags", "Campaign,Bonus,Endless,Challenge", FCVAR_NONE, "Trending addons must have at least one of these tags to appear on the main menu." );
ConVar rd_hoiaf_leaderboard_on_main_menu( "rd_hoiaf_leaderboard_on_main_menu", "1", FCVAR_NONE, "Should we download HoIAF stats for the current season on the main menu?" );

extern ConVar mm_max_players;
ConVar rd_last_game_access( "rd_last_game_access", "public", FCVAR_ARCHIVE, "Remembers the last game access setting (public or friends) for a lobby created from the main menu." );
ConVar rd_last_game_difficulty( "rd_last_game_difficulty", "normal", FCVAR_ARCHIVE, "Remembers the last game difficulty setting (easy/normal/hard/insane/imba) for a lobby created from the main menu." );
ConVar rd_last_game_challenge( "rd_last_game_challenge", "0", FCVAR_ARCHIVE, "Remembers the last game challenge ID (0 for none) for a lobby created from the main menu." );
ConVar rd_last_game_onslaught( "rd_last_game_onslaught", "0", FCVAR_ARCHIVE, "Remembers the last game onslaught setting for a lobby created from the main menu." );
ConVar rd_last_game_hardcoreff( "rd_last_game_hardcoreff", "0", FCVAR_ARCHIVE, "Remembers the last game hardcore friendly fire setting for a lobby created from the main menu." );
ConVar rd_last_game_maxplayers( "rd_last_game_maxplayers", "4", FCVAR_ARCHIVE, "Remembers the last game max players setting for a lobby created from the main menu." );
ConVar rd_revert_convars( "rd_revert_convars", "1", FCVAR_ARCHIVE, "Resets FCVAR_REPLICATED variables to their default values when opening the main menu." );

void Demo_DisableButton( Button *pButton );
void OpenGammaDialog( VPANEL parent );

#ifdef IS_WINDOWS_PC
static const char *( *const wine_get_version )( void ) = static_cast< const char *( * )( void ) >( Plat_GetProcAddress( "ntdll.dll", "wine_get_version" ) );
#endif

//=============================================================================
MainMenu::MainMenu( Panel *parent, const char *panelName ):
	BaseClass( parent, panelName, true, true, false, false )
{
	SetProportional( true );
	SetTitle( "", false );
	SetMoveable( false );
	SetSizeable( false );
	SetPaintBackgroundEnabled( true );

	SetLowerGarnishEnabled( true );

	AddFrameListener( this );

	m_iQuickJoinHelpText = MMQJHT_NONE;
	m_iLastTimerUpdate = 0;

	SetDeleteSelfOnClose( true );

	m_pBtnSettings = new BaseModHybridButton( this, "BtnSettings", "", this, "Settings" );
	m_pBtnLogo = new BaseModHybridButton( this, "BtnLogo", "", this, "" );
	m_pTopButton[0] = new BaseModHybridButton( this, "BtnLoadout", "#rd_collection_inventory_loadout", this, "Loadout" );
	m_pTopButton[1] = new BaseModHybridButton( this, "BtnContracts", "#rd_mainmenu_contracts", this, "Contracts" );
	m_pTopButton[2] = new BaseModHybridButton( this, "BtnRecordings", "#rd_mainmenu_recordings", this, "Recordings" );
	m_pTopButton[3] = new BaseModHybridButton( this, "BtnSwarmopedia", "#rd_collection_swarmopedia", this, "Swarmopedia" );
	m_pTopButton[4] = new BaseModHybridButton( this, "BtnInventory", "#rd_mainmenu_inventory", this, "Inventory" );
	m_pBtnQuit = new BaseModHybridButton( this, "BtnQuit", "", this, "QuitGame" );
	m_pCommanderProfile = new CRD_VGUI_Commander_Mini_Profile( this, "CommanderMiniProfile" );
	m_pBtnMultiplayer = new BaseModHybridButton( this, "BtnMultiplayer", "#L4D360UI_FoudGames_CreateNew_campaign", this, "CreateGame" );
	m_pBtnSingleplayer = new BaseModHybridButton( this, "BtnSingleplayer", "#L4D360UI_MainMenu_PlaySolo", this, "SoloPlay" );
	m_pPnlQuickJoin = new QuickJoinPanel( this, "PnlQuickJoin" );
	m_pPnlQuickJoinPublic = new QuickJoinPublicPanel( this, "PnlQuickJoinPublic" );
	m_pBtnWorkshopShowcase = new BaseModHybridButton( this, "BtnWorkshopShowcase", "", this, "WorkshopShowcase" );
	m_pTopLeaderboardEntries[0] = new CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry_Large( this, "HoIAFTop1", this, "HoIAFTop1" );
	m_pTopLeaderboardEntries[1] = new CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry( this, "HoIAFTop2", this, "HoIAFTop2" );
	m_pTopLeaderboardEntries[2] = new CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry( this, "HoIAFTop3", this, "HoIAFTop3" );
	m_pTopLeaderboardEntries[3] = new CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry( this, "HoIAFTop4", this, "HoIAFTop4" );
	m_pTopLeaderboardEntries[4] = new CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry( this, "HoIAFTop5", this, "HoIAFTop5" );
	m_pTopLeaderboardEntries[5] = new CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry( this, "HoIAFTop6", this, "HoIAFTop6" );
	m_pTopLeaderboardEntries[6] = new CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry( this, "HoIAFTop7", this, "HoIAFTop7" );
	m_pTopLeaderboardEntries[7] = new CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry( this, "HoIAFTop8", this, "HoIAFTop8" );
	m_pTopLeaderboardEntries[8] = new CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry( this, "HoIAFTop9", this, "HoIAFTop9" );
	m_pTopLeaderboardEntries[9] = new CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry( this, "HoIAFTop10", this, "HoIAFTop10" );
	m_pBtnHoIAFTimer = new BaseModHybridButton( this, "BtnHoIAFTimer", "", this, "ShowHoIAF" );
	m_pBtnEventTimer[0] = new BaseModHybridButton( this, "BtnEventTimer1", "", this, "EventTimer1" );
	m_pBtnEventTimer[1] = new BaseModHybridButton( this, "BtnEventTimer2", "", this, "EventTimer2" );
	m_pBtnEventTimer[2] = new BaseModHybridButton( this, "BtnEventTimer3", "", this, "EventTimer3" );
	m_pBtnNewsShowcase = new BaseModHybridButton( this, "BtnNewsShowcase", "", this, "NewsShowcase" );
	m_pBtnUpdateNotes = new BaseModHybridButton( this, "BtnUpdateNotes", "", this, "UpdateNotes" );
}

//=============================================================================
MainMenu::~MainMenu()
{
	RemoveFrameListener( this );

	for ( int i = 0; i < NELEMS( m_iNewsImageTexture ); i++ )
	{
		if ( vgui::surface() && m_iNewsImageTexture[i] != -1 )
		{
			vgui::surface()->DestroyTextureID( m_iNewsImageTexture[i] );
		}
	}

	for ( int i = 0; i < NELEMS( m_pWorkshopTrendingPreview ); i++ )
	{
		delete m_pWorkshopTrendingPreview[i];
	}
}

//=============================================================================
void MainMenu::OnCommand( const char *command )
{
	int iUserSlot = CBaseModPanel::GetSingleton().GetLastActiveUserId();

	if ( UI_IsDebug() )
	{
		Msg("[GAMEUI] Handling main menu command %s from user%d ctrlr%d\n",
			command, iUserSlot, XBX_GetUserId( iUserSlot ) );
	}

	bool bOpeningFlyout = false;

	if ( char const *szQuickMatch = StringAfterPrefix( command, "QuickMatch_" ) )
	{
		if ( CheckAndDisplayErrorIfNotLoggedIn() ||
			CUIGameData::Get()->CheckAndDisplayErrorIfNotSignedInToLive( this ) )
			return;

		KeyValues *pSettings = KeyValues::FromString(
			"settings",
			" system { "
				" network LIVE "
			" } "
			" game { "
				" mode = "
			" } "
			" options { "
				" action quickmatch "
			" } "
			);
		KeyValues::AutoDelete autodelete( pSettings );

		pSettings->SetString( "game/mode", szQuickMatch );

		// TCR: We need to respect the default difficulty
		if ( GameModeHasDifficulty( szQuickMatch ) )
			pSettings->SetString( "game/difficulty", GameModeGetDefaultDifficulty( szQuickMatch ) );

		g_pMatchFramework->MatchSession( pSettings );
	}
	else if ( char const *szCustomMatch = StringAfterPrefix( command, "CustomMatch_" ) )
	{
		if ( CheckAndDisplayErrorIfNotLoggedIn() ||
			 CUIGameData::Get()->CheckAndDisplayErrorIfNotSignedInToLive( this ) )
			return;

		KeyValues *pSettings = KeyValues::FromString(
			"settings",
			" system { "
				" network LIVE "
			" } "
			" game { "
				" mode = "
			" } "
			" options { "
				" action custommatch "
			" } "
			);
		KeyValues::AutoDelete autodelete( pSettings );

		pSettings->SetString( "game/mode", szCustomMatch );

		CBaseModPanel::GetSingleton().OpenWindow(
			ui_play_online_browser.GetBool() ? WT_FOUNDPUBLICGAMES : WT_GAMESETTINGS,
			this, true, pSettings );
	}
	else if ( char const *szFriendsMatch = StringAfterPrefix( command, "FriendsMatch_" ) )
	{
		if ( CheckAndDisplayErrorIfNotLoggedIn() )
			return;

		if ( StringHasPrefix( szFriendsMatch, "team" ) &&
			CUIGameData::Get()->CheckAndDisplayErrorIfNotSignedInToLive( this ) )
			// Team games require to be signed in to LIVE
			return;

		KeyValues *pSettings = KeyValues::FromString(
			"settings",
			" game { "
				" mode = "
			" } "
			);
		KeyValues::AutoDelete autodelete( pSettings );

		pSettings->SetString( "game/mode", szFriendsMatch );

		if ( m_ActiveControl )
		{
			m_ActiveControl->NavigateFrom( );
		}
		CBaseModPanel::GetSingleton().OpenWindow( WT_ALLGAMESEARCHRESULTS, this, true, pSettings );
	}	
	else if ( char const *szGroupServer = StringAfterPrefix( command, "GroupServer_" ) )
	{
		if ( CheckAndDisplayErrorIfNotLoggedIn() )
			return;

		KeyValues *pSettings = KeyValues::FromString(
			"settings",
			" game { "
				// " mode = "
			" } "
			" options { "
				" action groupserver "
			" } "
			);
		KeyValues::AutoDelete autodelete( pSettings );

		if ( *szGroupServer )
			pSettings->SetString( "game/mode", szGroupServer );

		if ( m_ActiveControl )
		{
			m_ActiveControl->NavigateFrom( );
		}
		CBaseModPanel::GetSingleton().OpenWindow( WT_STEAMGROUPSERVERS, this, true, pSettings );
	}
	else if ( !Q_strcmp( command, "BtnStub" ) )
	{
		// clicking No Steam will provide some info
		GenericConfirmation* confirmation =
			static_cast<GenericConfirmation*>( CBaseModPanel::GetSingleton().OpenWindow( WT_GENERICCONFIRMATION, this, false ) );
		GenericConfirmation::Data_t data;
		data.pWindowTitle = "#rd_no_steam_service";
		data.pMessageText = "#rd_no_steam_solutions";

		if ( SteamApps() && SteamFriends() && SteamHTTP() && SteamInput() && SteamMatchmaking() && SteamMatchmakingServers() && SteamUGC() && SteamUser() && SteamUserStats() )
		{
			// The NO STEAM main menu is active, but the Steam API is available. This should never happen. Please contact https://reactivedrop.com/feedback
			data.pMessageText = "#rd_no_steam_solutions_api";
		}
		else if ( !SteamAPI_IsSteamRunning() )
		{
			// Did not detect an instance of the Steam Client. If the Steam Client is running, try selecting Steam->Exit and then restarting Steam.
			data.pMessageText = "#rd_no_steam_solutions_client";
		}
		else if ( !SteamAPI_GetSteamInstallPath() )
		{
			// Could not determine the location of the Steam Client through the registry. Try restarting Steam.
			data.pMessageText = "#rd_no_steam_solutions_path";
		}
		else if ( wine_get_version )
		{
			static wchar_t s_wszWineVersionWarning[1024];
			wchar_t wszWineVersion[128];
			V_UTF8ToUnicode( wine_get_version(), wszWineVersion, sizeof( wszWineVersion ) );
			g_pVGuiLocalize->ConstructString( s_wszWineVersionWarning, sizeof( s_wszWineVersionWarning ), g_pVGuiLocalize->Find( "#rd_no_steam_solutions_wine" ), 1, wszWineVersion );
			data.pMessageTextW = s_wszWineVersionWarning;
		}

		data.bOkButtonEnabled = true;
		confirmation->SetUsageData( data );
	}
	else if ( !Q_strcmp( command, "TrainingPlay" ) )
	{
		// Ensure that tutorial messages will be displayed.
		engine->ClientCmd_Unrestricted( "gameinstructor_enable 1; gameinstructor_reset_counts" );

		KeyValues *pSettings = KeyValues::FromString(
			"settings",
			" system { "
			" network offline "
			" } "
			" game { "
			" mode single_mission "
			" campaign jacob "
			" mission asi-jac1-landingbay_pract "
			" difficulty normal "
			" } "
		);
		KeyValues::AutoDelete autodelete( pSettings );

		g_pMatchFramework->CreateSession( pSettings );

		// Automatically start the tutorial mission, no configuration required
		if ( IMatchSession *pMatchSession = g_pMatchFramework->GetMatchSession() )
		{
			pMatchSession->Command( KeyValues::AutoDeleteInline( new KeyValues( "Start" ) ) );
		}
	}
	else if ( !Q_strcmp( command, "SoloPlay" ) )
	{
		engine->ClientCmd_Unrestricted( "asw_mission_chooser singleplayer" );
	}
	else if ( !Q_strcmp( command, "DeveloperCommentary" ) )
	{
#ifdef _X360
		if ( XBX_GetNumGameUsers() > 1 )
		{
			GenericConfirmation* confirmation = 
				static_cast< GenericConfirmation* >( CBaseModPanel::GetSingleton().OpenWindow( WT_GENERICCONFIRMATION, this, false ) );

			GenericConfirmation::Data_t data;

			data.pWindowTitle = "#L4D360UI_MainMenu_SplitscreenDisableConf";
			data.pMessageText = "#L4D360UI_Extras_Commentary_ss_Msg";

			data.bOkButtonEnabled = true;
			data.pfnOkCallback = &AcceptSplitscreenDisableCallback;
			data.bCancelButtonEnabled = true;

			confirmation->SetUsageData(data);
			return;
		}
#endif
		// Explain the rules of commentary
		GenericConfirmation* confirmation = 
			static_cast< GenericConfirmation* >( CBaseModPanel::GetSingleton().OpenWindow( WT_GENERICCONFIRMATION, this, false ) );

		GenericConfirmation::Data_t data;

		data.pWindowTitle = "#GAMEUI_CommentaryDialogTitle";
		data.pMessageText = "#L4D360UI_Commentary_Explanation";

		data.bOkButtonEnabled = true;
		data.pfnOkCallback = &AcceptCommentaryRulesCallback;
		data.bCancelButtonEnabled = true;

		confirmation->SetUsageData(data);
		NavigateFrom();
	}
	else if ( !Q_strcmp( command, "StatsAndAchievements" ) )
	{
		// If PC make sure that the Steam user is logged in
		if ( CheckAndDisplayErrorIfNotLoggedIn() )
			return;

#ifdef _X360
		// If 360 make sure that the user is not a guest
		if ( XBX_GetUserIsGuest( CBaseModPanel::GetSingleton().GetLastActiveUserId() ) )
		{
			GenericConfirmation* confirmation = 
				static_cast<GenericConfirmation*>( CBaseModPanel::GetSingleton().OpenWindow( WT_GENERICCONFIRMATION, this, false ) );
			GenericConfirmation::Data_t data;
			data.pWindowTitle = "#L4D360UI_MsgBx_AchievementsDisabled";
			data.pMessageText = "#L4D360UI_MsgBx_GuestsUnavailableToGuests";
			data.bOkButtonEnabled = true;
			confirmation->SetUsageData(data);

			return;
		}
#endif //_X360
		if ( m_ActiveControl )
		{
			m_ActiveControl->NavigateFrom( );
		}

		CBaseModPanel::GetSingleton().OpenWindow( WT_ACHIEVEMENTS, this, true );
	}
	else if ( !Q_strcmp( command, "FlmExtrasFlyoutCheck" ) )
	{
		if ( IsX360() && CUIGameData::Get()->SignedInToLive() )
			OnCommand( "FlmExtrasFlyout_Live" );
		else
			OnCommand( "FlmExtrasFlyout_Simple" );
		return;
	}
	else if ( char const *szInviteType = StringAfterPrefix( command, "InviteUI_" ) )
	{
		if ( IsX360() )
		{
			CUIGameData::Get()->OpenInviteUI( szInviteType );
		}
		else
		{
			CUIGameData::Get()->ExecuteOverlayCommand( "LobbyInvite" );
		}
	}
	else if (!Q_strcmp(command, "Game"))
	{
		if ( m_ActiveControl )
		{
			m_ActiveControl->NavigateFrom( );
		}
		CBaseModPanel::GetSingleton().OpenWindow(WT_GAMEOPTIONS, this, true );
	}
	else if (!Q_strcmp(command, "AudioVideo"))
	{
		if ( m_ActiveControl )
		{
			m_ActiveControl->NavigateFrom( );
		}
		CBaseModPanel::GetSingleton().OpenWindow(WT_AUDIOVIDEO, this, true );
	}
	else if (!Q_strcmp(command, "Controller"))
	{
		if ( m_ActiveControl )
		{
			m_ActiveControl->NavigateFrom( );
		}
		CBaseModPanel::GetSingleton().OpenWindow(WT_CONTROLLER, this, true );
	}
	else if (!Q_strcmp(command, "Storage"))
	{
#ifdef _X360
		if ( XBX_GetUserIsGuest( iUserSlot ) )
		{
			CBaseModPanel::GetSingleton().PlayUISound( UISOUND_INVALID );
			return;
		}
#endif
		if ( m_ActiveControl )
		{
			m_ActiveControl->NavigateFrom( );
		}

		// Trigger storage device selector
		CUIGameData::Get()->SelectStorageDevice( new CChangeStorageDevice( XBX_GetUserId( iUserSlot ) ) );
	}
	else if ( !Q_strcmp( command, "Workshop" ) )
	{
		OpenNewsURL( "https://steamcommunity.com/app/563560/workshop/?l=%s" );
	}
	else if ( !Q_strcmp( command, "WorkshopManage" ) )
	{
		CBaseModPanel::GetSingleton().OpenWindow( WT_WORKSHOP, this );
	}
	else if (!Q_strcmp(command, "Credits"))
	{
#ifdef _X360
		if ( XBX_GetNumGameUsers() > 1 )
		{
			GenericConfirmation* confirmation = 
				static_cast< GenericConfirmation* >( CBaseModPanel::GetSingleton().OpenWindow( WT_GENERICCONFIRMATION, this, false ) );

			GenericConfirmation::Data_t data;

			data.pWindowTitle = "#L4D360UI_MainMenu_SplitscreenDisableConf";
			data.pMessageText = "#L4D360UI_Extras_Credits_ss_Msg";

			data.bOkButtonEnabled = true;
			data.pfnOkCallback = &AcceptSplitscreenDisableCallback;
			data.bCancelButtonEnabled = true;

			confirmation->SetUsageData(data);
			return;
		}
#endif
		KeyValues *pSettings = KeyValues::FromString(
			"settings",
			" system { "
				" network offline "
			" } "
			" game { "
				" mode single_mission "
			" } "
			" options { "
				" play credits "
			" } "
			);
		KeyValues::AutoDelete autodelete( pSettings );

		g_pMatchFramework->CreateSession( pSettings );

		// Automatically start the credits session, no configuration required
		if ( IMatchSession *pMatchSession = g_pMatchFramework->GetMatchSession() )
		{
			pMatchSession->Command( KeyValues::AutoDeleteInline( new KeyValues( "Start" ) ) );
		}
	}
	else if (!Q_strcmp(command, "QuitGame"))
	{
		if ( IsPC() )
		{
			GenericConfirmation* confirmation = 
				static_cast< GenericConfirmation* >( CBaseModPanel::GetSingleton().OpenWindow( WT_GENERICCONFIRMATION, this, false ) );

			GenericConfirmation::Data_t data;

			data.pWindowTitle = "#L4D360UI_MainMenu_Quit_Confirm";
			data.pMessageText = "#L4D360UI_MainMenu_Quit_ConfirmMsg";

			data.bOkButtonEnabled = true;
			data.pfnOkCallback = &AcceptQuitGameCallback;
			data.bCancelButtonEnabled = true;

			confirmation->SetUsageData(data);

			NavigateFrom();
		}

		if ( IsX360() )
		{
			engine->ExecuteClientCmd( "demo_exit" );
		}
	}
	else if ( !Q_stricmp( command, "QuitGame_NoConfirm" ) )
	{
		if ( IsPC() )
		{
			engine->ClientCmd( "quit" );
		}
	}
	else if ( !Q_strcmp( command, "EnableSplitscreen" ) )
	{
		Msg( "Enabling splitscreen from main menu...\n" );

		CBaseModPanel::GetSingleton().CloseAllWindows();
		CAttractScreen::SetAttractMode( CAttractScreen::ATTRACT_GOSPLITSCREEN );
		CBaseModPanel::GetSingleton().OpenWindow( WT_ATTRACTSCREEN, NULL, true );
	}
	else if ( !Q_strcmp( command, "DisableSplitscreen" ) )
	{
		GenericConfirmation* confirmation = 
			static_cast< GenericConfirmation* >( CBaseModPanel::GetSingleton().OpenWindow( WT_GENERICCONFIRMATION, this, false ) );

		GenericConfirmation::Data_t data;

		data.pWindowTitle = "#L4D360UI_MainMenu_SplitscreenDisableConf";
		data.pMessageText = "#L4D360UI_MainMenu_SplitscreenDisableConfMsg";

		data.bOkButtonEnabled = true;
		data.pfnOkCallback = &AcceptSplitscreenDisableCallback;
		data.bCancelButtonEnabled = true;

		confirmation->SetUsageData(data);
	}
	else if ( !Q_strcmp( command, "DisableSplitscreen_NoConfirm" ) )
	{
		Msg( "Disabling splitscreen from main menu...\n" );

		CAttractScreen::SetAttractMode( CAttractScreen::ATTRACT_GAMESTART  );
		OnCommand( "ActivateAttractScreen" );
	}
	else if (!Q_strcmp(command, "ChangeGamers"))	// guest SIGN-IN command
	{
		CAttractScreen::SetAttractMode( CAttractScreen::ATTRACT_GUESTSIGNIN, XBX_GetUserId( iUserSlot ) );
		OnCommand( "ActivateAttractScreen" );
	}
	else if (!Q_strcmp(command, "ActivateAttractScreen"))
	{
		if ( IsX360() )
		{
			Close();
			CBaseModPanel::GetSingleton().CloseAllWindows();
			CAttractScreen::SetAttractMode( CAttractScreen::ATTRACT_GAMESTART );
			CBaseModFrame *pWnd = CBaseModPanel::GetSingleton().OpenWindow( WT_ATTRACTSCREEN, NULL, true );
			if ( pWnd )
			{
				pWnd->PostMessage( pWnd, new KeyValues( "ChangeGamers" ) );
			}
		}		
	}
	else if (!Q_strcmp(command, "Audio"))
	{
		if ( ui_old_options_menu.GetBool() )
		{
			CBaseModPanel::GetSingleton().OpenOptionsDialog( this );
		}
		else
		{
			// audio options dialog, PC only
			if ( m_ActiveControl )
			{
				m_ActiveControl->NavigateFrom( );
			}
			CBaseModPanel::GetSingleton().OpenWindow(WT_AUDIO, this, true );
		}
	}
	else if (!Q_strcmp(command, "Video"))
	{
		if ( ui_old_options_menu.GetBool() )
		{
			CBaseModPanel::GetSingleton().OpenOptionsDialog( this );
		}
		else
		{
			// video options dialog, PC only
			if ( m_ActiveControl )
			{
				m_ActiveControl->NavigateFrom( );
			}
			CBaseModPanel::GetSingleton().OpenWindow(WT_VIDEO, this, true );
		}
	}
	else if (!Q_strcmp(command, "Brightness"))
	{
		if ( ui_old_options_menu.GetBool() )
		{
			CBaseModPanel::GetSingleton().OpenOptionsDialog( this );
		}
		else
		{
			// brightness options dialog, PC only
			OpenGammaDialog( GetVParent() );
		}
	}
	else if (!Q_strcmp(command, "KeyboardMouse"))
	{
		if ( ui_old_options_menu.GetBool() )
		{
			CBaseModPanel::GetSingleton().OpenOptionsDialog( this );
		}
		else
		{
			// standalone keyboard/mouse dialog, PC only
			if ( m_ActiveControl )
			{
				m_ActiveControl->NavigateFrom( );
			}
			CBaseModPanel::GetSingleton().OpenWindow(WT_KEYBOARDMOUSE, this, true );
		}
	}
	else if ( !Q_strcmp( command, "Gamepad" ) )
	{
		if ( m_ActiveControl )
		{
			m_ActiveControl->NavigateFrom();
		}
		CBaseModPanel::GetSingleton().OpenWindow( WT_GAMEPAD, this, true );
	}
	else if( Q_stricmp( "#L4D360UI_Controller_Edit_Keys_Buttons", command ) == 0 )
	{
		FlyoutMenu::CloseActiveMenu();
		CBaseModPanel::GetSingleton().OpenKeyBindingsDialog( this );
	}
	else if (!Q_strcmp(command, "MultiplayerSettings"))
	{
		if ( ui_old_options_menu.GetBool() )
		{
			CBaseModPanel::GetSingleton().OpenOptionsDialog( this );
		}
		else
		{
			// standalone multiplayer settings dialog, PC only
			if ( m_ActiveControl )
			{
				m_ActiveControl->NavigateFrom( );
			}
			CBaseModPanel::GetSingleton().OpenWindow(WT_MULTIPLAYER, this, true );
		}
	}
	else if ( !Q_strcmp( command, "AdvancedSettings" ) )
	{
		if ( m_ActiveControl )
		{
			m_ActiveControl->NavigateFrom();
		}
		CBaseModPanel::GetSingleton().OpenWindow( WT_ADVANCEDSETTINGS, this, true );
	}
	else if (!Q_strcmp(command, "CloudSettings"))
	{
		// standalone cloud settings dialog, PC only
		if ( m_ActiveControl )
		{
			m_ActiveControl->NavigateFrom( );
		}
		CBaseModPanel::GetSingleton().OpenWindow(WT_CLOUD, this, true );
	}
	else if (!Q_strcmp(command, "SeeAll"))
	{
		if ( CheckAndDisplayErrorIfNotLoggedIn() )
			return;

		KeyValues *pSettings = KeyValues::FromString(
			"settings",
			" game { "
			// passing empty game settings to indicate no preference
			" } "
			);
		KeyValues::AutoDelete autodelete( pSettings );

		if ( m_ActiveControl )
		{
			m_ActiveControl->NavigateFrom( );
		}
		CBaseModPanel::GetSingleton().OpenWindow( WT_ALLGAMESEARCHRESULTS, this, true, pSettings );
		CBaseModPanel::GetSingleton().PlayUISound( UISOUND_ACCEPT );
	}
	else if ( !Q_strcmp( command, "OpenServerBrowser" ) )
	{
		if ( CheckAndDisplayErrorIfNotLoggedIn() )
			return;

		// on PC, bring up the server browser and switch it to the LAN tab (tab #5)
		engine->ClientCmd( "openserverbrowser" );
	}
	else if ( !Q_strcmp( command, "DemoConnect" ) )
	{
		g_pMatchFramework->GetMatchTitle()->PrepareClientForConnect( NULL );
		engine->ClientCmd( CFmtStr( "connect %s", demo_connect_string.GetString() ) );
	}
	else if (command && command[0] == '#')
	{
		// Pass it straight to the engine as a command
		engine->ClientCmd( command+1 );
	}
	else if( !Q_strcmp( command, "Addons" ) )
	{
		CBaseModPanel::GetSingleton().OpenWindow( WT_ADDONS, this, true );
	}
	else if ( !Q_strcmp( command, "IafRanks" ) )
	{
		CBaseModPanel::GetSingleton().OpenWindow( WT_IAFRANKS, this, true );
	}
	else if( !Q_strcmp( command, "CreateGame" ) )
	{
		engine->ClientCmd_Unrestricted( "asw_mission_chooser createserver" );
	}
	else if ( !V_stricmp( command, "WorkshopShowcase" ) )
	{
		uint32 iCurrentWorkshopShowcase = ( std::time( NULL ) / 60 ) % NELEMS( m_iWorkshopTrendingFileID );
		if ( m_iWorkshopTrendingFileID[iCurrentWorkshopShowcase] == k_PublishedFileIdInvalid )
		{
			OpenNewsURL( "https://steamcommunity.com/app/563560/workshop/?l=%s" );
		}
		else
		{
			OpenNewsURL( CFmtStr{ "https://steamcommunity.com/workshop/filedetails/?id=%llu&l=%%s", m_iWorkshopTrendingFileID[iCurrentWorkshopShowcase] } );
		}
	}
	else if ( const char *szHoIAFPlaceNumber = StringAfterPrefix( command, "HoIAFTop" ) )
	{
		int iPlaceNumber = V_atoi( szHoIAFPlaceNumber );
		Assert( iPlaceNumber >= 1 && iPlaceNumber <= NELEMS( m_pTopLeaderboardEntries ) );
		if ( iPlaceNumber >= 1 && iPlaceNumber <= NELEMS( m_pTopLeaderboardEntries ) )
		{
			Assert( m_pTopLeaderboardEntries[iPlaceNumber - 1]->IsVisible() );
			KeyValues::AutoDelete pSettings{ "settings" };
			pSettings->SetUint64( "steamid", m_pTopLeaderboardEntries[iPlaceNumber - 1]->m_SteamID.ConvertToUint64() );
			CBaseModPanel::GetSingleton().OpenWindow( WT_COMMANDERPROFILE, this, true, pSettings );
		}
	}
	else if ( !V_stricmp( command, "ShowHoIAF" ) )
	{
		OpenNewsURL( "https://stats.reactivedrop.com/heroes?lang=%s" );
	}
	else if ( !V_stricmp( command, "EventTimer1" ) )
	{
		OpenNewsURL( m_szEventURL[0] );
	}
	else if ( !V_stricmp( command, "EventTimer2" ) )
	{
		OpenNewsURL( m_szEventURL[1] );
	}
	else if ( !V_stricmp( command, "EventTimer3" ) )
	{
		OpenNewsURL( m_szEventURL[2] );
	}
	else if ( !V_stricmp( command, "NewsShowcase" ) )
	{
		int iNumNewsShowcases = 0;
		while ( iNumNewsShowcases < NELEMS( m_wszNewsTitle ) && m_wszNewsTitle[iNumNewsShowcases][0] != L'\0' )
			iNumNewsShowcases++;

		if ( iNumNewsShowcases )
		{
			int iCurrentMinute = ( std::time( NULL ) / 60 );
			OpenNewsURL( m_szNewsURL[iCurrentMinute % iNumNewsShowcases] );
		}
	}
	else if ( !V_stricmp( command, "UpdateNotes" ) )
	{
		char szBranchName[64]{};
		if ( SteamApps() && SteamApps()->GetCurrentBetaName( szBranchName, sizeof( szBranchName ) ) && szBranchName[0] != '\0' )
		{
			OpenNewsURL( "https://reactivedrop.com/beta-updates/" );
		}
		else
		{
			OpenNewsURL( "https://steamcommunity.com/app/563560/allnews/?l=%s" );
		}
	}
	else if ( !V_stricmp( command, "Settings" ) )
	{
		Assert( !"TODO" );
	}
	else if ( !V_stricmp( command, "Loadout" ) )
	{
		Assert( !"TODO" );
	}
	else if ( !V_stricmp( command, "Contracts" ) )
	{
		Assert( !"TODO" );
	}
	else if ( !V_stricmp( command, "Recordings" ) )
	{
		CBaseModPanel::GetSingleton().OpenWindow( WT_DEMOS, CBaseModPanel::GetSingleton().GetWindow( CBaseModPanel::GetSingleton().GetActiveWindowType() ) );
	}
	else if ( !V_stricmp( command, "Swarmopedia" ) )
	{
		Assert( !"TODO" );
	}
	else if ( !V_stricmp( command, "Inventory" ) )
	{
		Assert( !"TODO" );
	}
	else
	{
		// does this command match a flyout menu?
		BaseModUI::FlyoutMenu *flyout = dynamic_cast< FlyoutMenu* >( FindChildByName( command ) );
		if ( flyout )
		{
			bOpeningFlyout = true;

			// If so, enumerate the buttons on the menu and find the button that issues this command.
			// (No other way to determine which button got pressed; no notion of "current" button on PC.)
			for ( int iChild = 0; iChild < GetChildCount(); iChild++ )
			{
				bool bFound = false;
				GameModes *pGameModes = dynamic_cast< GameModes *>( GetChild( iChild ) );
				if ( pGameModes )
				{
					for ( int iGameMode = 0; iGameMode < pGameModes->GetNumGameInfos(); iGameMode++ )
					{
						BaseModHybridButton *pHybrid = pGameModes->GetHybridButton( iGameMode );
						if ( pHybrid && pHybrid->GetCommand() && !Q_strcmp( pHybrid->GetCommand()->GetString( "command"), command ) )
						{
							pHybrid->NavigateFrom();
							// open the menu next to the button that got clicked
							flyout->OpenMenu( pHybrid );
							flyout->SetListener( this );
							bFound = true;
							break;
						}
					}
				}

				if ( !bFound )
				{
					BaseModHybridButton *hybrid = dynamic_cast<BaseModHybridButton *>( GetChild( iChild ) );
					if ( hybrid && hybrid->GetCommand() && !Q_strcmp( hybrid->GetCommand()->GetString( "command"), command ) )
					{
						hybrid->NavigateFrom();
						// open the menu next to the button that got clicked
						flyout->OpenMenu( hybrid );
						flyout->SetListener( this );
						break;
					}
				}
			}
		}
		else
		{
			BaseClass::OnCommand( command );
		}
	}

	if( !bOpeningFlyout )
	{
		FlyoutMenu::CloseActiveMenu(); //due to unpredictability of mouse navigation over keyboard, we should just close any flyouts that may still be open anywhere.
	}
}

//=============================================================================
void MainMenu::OpenMainMenuJoinFailed( const char *msg )
{
	// This is called when accepting an invite or joining friends game fails
	CUIGameData::Get()->OpenWaitScreen( msg );
	CUIGameData::Get()->CloseWaitScreen( NULL, NULL );
}

//=============================================================================
void MainMenu::OnNotifyChildFocus( vgui::Panel* child )
{
}

void MainMenu::OnFlyoutMenuClose( vgui::Panel* flyTo )
{
	SetFooterState();
}

void MainMenu::OnFlyoutMenuCancelled()
{
}

//=============================================================================
void MainMenu::OnKeyCodePressed( KeyCode code )
{
	int userId = GetJoystickForCode( code );
	BaseModUI::CBaseModPanel::GetSingleton().SetLastActiveUserId( userId );

	switch( GetBaseButtonCode( code ) )
	{
	case KEY_XBUTTON_B:
		// Capture the B key so it doesn't play the cancel sound effect
		break;
	case KEY_XBUTTON_X:
		{
			QuickJoinPanel *pQuickJoin = dynamic_cast<QuickJoinPanel*>( FindChildByName( "PnlQuickJoin" ) );
			if ( pQuickJoin && pQuickJoin->ShouldBeVisible() )
			{
				// X shortcut only works if the panel is showing
				OnCommand( "SeeAll" );
			}

			break;
		}

	case KEY_XBUTTON_BACK:
#ifdef _X360
		if ( XBX_GetNumGameUsers() > 1 )
		{
			OnCommand( "DisableSplitscreen" );
		}
#endif
		break;

	case KEY_XBUTTON_INACTIVE_START:
#ifdef _X360
		if ( !XBX_GetPrimaryUserIsGuest() &&
			 userId != (int) XBX_GetPrimaryUserId() &&
			 userId >= 0 &&
			 CUIGameData::Get()->CanPlayer2Join() )
		{
			// Pass the index of controller which wanted to join splitscreen
			CBaseModPanel::GetSingleton().CloseAllWindows();
			CAttractScreen::SetAttractMode( CAttractScreen::ATTRACT_GOSPLITSCREEN, userId );
			CBaseModPanel::GetSingleton().OpenWindow( WT_ATTRACTSCREEN, NULL, true );
		}
#endif
		break;

	default:
		BaseClass::OnKeyCodePressed( code );
		break;
	}
}

//=============================================================================
void MainMenu::OnThink()
{
	// don't need any of this if we're not able to run
	if ( m_bIsStub )
		return;

	// need to change state of flyout if user suddenly disconnects
	// while flyout is open
	BaseModUI::FlyoutMenu *flyout = dynamic_cast< FlyoutMenu* >( FindChildByName( "FlmCampaignFlyout" ) );
	if ( flyout )
	{
		BaseModHybridButton *pButton = dynamic_cast< BaseModHybridButton* >( flyout->FindChildButtonByCommand( "QuickMatchCoOp" ) );
		if ( pButton )
		{
			if ( !CUIGameData::Get()->SignedInToLive() )
			{
				pButton->SetText( "#L4D360UI_QuickStart" );
				if ( m_iQuickJoinHelpText != MMQJHT_QUICKSTART )
				{
					pButton->SetHelpText( "#L4D360UI_QuickMatch_Offline_Tip" );
					m_iQuickJoinHelpText = MMQJHT_QUICKSTART;
				}
			}
			else
			{
				pButton->SetText( "#L4D360UI_QuickMatch" );
				if ( m_iQuickJoinHelpText != MMQJHT_QUICKMATCH )
				{
					pButton->SetHelpText( "#L4D360UI_QuickMatch_Tip" );
					m_iQuickJoinHelpText = MMQJHT_QUICKMATCH;
				}
			}
		}
	}

	if ( IsPC() )
	{
		FlyoutMenu *pFlyout = dynamic_cast< FlyoutMenu* >( FindChildByName( "FlmOptionsFlyout" ) );
		if ( pFlyout )
		{
			const MaterialSystem_Config_t &config = materials->GetCurrentConfigForVideoCard();
			pFlyout->SetControlEnabled( "BtnBrightness", !config.Windowed() );
			pFlyout->SetControlEnabled( "BtnController", g_RD_Steam_Input.GetJoystickCount() != 0 );
		}
	}

	uint32 iCurrentTime = std::time( NULL );
	uint32 iCurrentMinute = iCurrentTime / 60;
	if ( m_iLastTimerUpdate != iCurrentMinute )
	{
		wchar_t wszTimerText[1024];
		for ( int i = NELEMS( m_iEventStarts ) - 1; i >= 0; i-- )
		{
			if ( m_iEventStarts[i] > iCurrentTime || m_iEventEnds[i] < iCurrentTime )
			{
				m_pBtnEventTimer[i]->SetVisible( false );
				continue;
			}

			uint32 iTimeLeftHours = ( m_iEventEnds[i] - iCurrentTime ) / 3600;
			if ( iTimeLeftHours > 23 )
				g_pVGuiLocalize->ConstructString( wszTimerText, sizeof( wszTimerText ), g_pVGuiLocalize->Find( "#rd_event_timer_days" ), 2, m_wszEventTitle[i], UTIL_RD_CommaNumber( iTimeLeftHours / 24 ) );
			else if ( iTimeLeftHours > 0 )
				g_pVGuiLocalize->ConstructString( wszTimerText, sizeof( wszTimerText ), g_pVGuiLocalize->Find( "#rd_event_timer_hours" ), 2, m_wszEventTitle[i], UTIL_RD_CommaNumber( iTimeLeftHours ) );
			else
				g_pVGuiLocalize->ConstructString( wszTimerText, sizeof( wszTimerText ), g_pVGuiLocalize->Find( "#rd_event_timer_soon" ), 1, m_wszEventTitle[i] );

			m_pBtnEventTimer[i]->SetText( wszTimerText );
			m_pBtnEventTimer[i]->SetVisible( true );
		}

		struct tm tm;
		Plat_gmtime( iCurrentTime, &tm );

		int iSeasonNumber = ( tm.tm_year - 123 ) * 12 + 7 + tm.tm_mon;
		int iHoursRemaining = 23 - tm.tm_hour;
		int iDaysRemaining = 31 - tm.tm_mday;
		if ( tm.tm_mon == 1 )
		{
			// february
			iDaysRemaining -= 2;
			int iActualYear = tm.tm_year + 1900;
			if ( iActualYear % 4 == 0 && ( iActualYear % 100 != 0 || iActualYear % 400 == 0 ) )
				iDaysRemaining--;
		}
		else if ( tm.tm_mon == 3 || tm.tm_mon == 5 || tm.tm_mon == 8 || tm.tm_mon == 10 )
		{
			// 30-day months
			iDaysRemaining--;
		}

		if ( iDaysRemaining > 0 )
			g_pVGuiLocalize->ConstructString( wszTimerText, sizeof( wszTimerText ), g_pVGuiLocalize->Find( "#rd_hoiaf_ends_days_hours" ), 3, UTIL_RD_CommaNumber( iSeasonNumber ), UTIL_RD_CommaNumber( iDaysRemaining ), UTIL_RD_CommaNumber( iHoursRemaining ) );
		else if ( iHoursRemaining > 0 )
			g_pVGuiLocalize->ConstructString( wszTimerText, sizeof( wszTimerText ), g_pVGuiLocalize->Find( "#rd_hoiaf_ends_hours" ), 2, UTIL_RD_CommaNumber( iSeasonNumber ), UTIL_RD_CommaNumber( iHoursRemaining ) );
		else
			g_pVGuiLocalize->ConstructString( wszTimerText, sizeof( wszTimerText ), g_pVGuiLocalize->Find( "#rd_hoiaf_ends_soon" ), 1, UTIL_RD_CommaNumber( iSeasonNumber ) );

		m_pBtnHoIAFTimer->SetText( wszTimerText );
		m_pBtnHoIAFTimer->SetVisible( true );

		m_pBtnWorkshopShowcase->SetText( m_wszWorkshopTrendingTitle[iCurrentMinute % NELEMS( m_wszWorkshopTrendingTitle )] );
		m_pBtnWorkshopShowcase->SetVisible( true );

		int iNumNewsShowcases = 0;
		while ( iNumNewsShowcases < NELEMS( m_wszNewsTitle ) && m_wszNewsTitle[iNumNewsShowcases][0] != L'\0' )
			iNumNewsShowcases++;

		if ( iNumNewsShowcases == 0 )
		{
			m_pBtnNewsShowcase->SetVisible( false );
		}
		else
		{
			m_pBtnNewsShowcase->SetText( m_wszNewsTitle[iCurrentMinute % iNumNewsShowcases] );
			m_pBtnNewsShowcase->SetVisible( true );
		}

		m_iLastTimerUpdate = iCurrentMinute;
	}

	BaseClass::OnThink();
}

//=============================================================================
void MainMenu::OnOpen()
{
	extern ConVar *sv_cheats;
	if ( !sv_cheats )
	{
		sv_cheats = cvar->FindVar( "sv_cheats" );
	}

	if ( sv_cheats && !sv_cheats->GetBool() )
	{
		if ( rd_revert_convars.GetBool() )
		{
			g_pCVar->RevertFlaggedConVars( FCVAR_REPLICATED );
		}

		g_pCVar->RevertFlaggedConVars( FCVAR_CHEAT );

		if ( rd_revert_convars.GetBool() )
		{
			engine->ClientCmd_Unrestricted( "execifexists autoexec\n" );

			g_ReactiveDropWorkshop.RerunAutoExecScripts();
		}
	}

	// Apply mixer convars.
	engine->ClientCmd_Unrestricted( "_rd_mixer_init\n" );

	if ( IsPC() && connect_lobby.GetString()[0] )
	{
		// if we were launched with "+connect_lobby <lobbyid>" on the command line, join that lobby immediately
		uint64 nLobbyID = _atoi64( connect_lobby.GetString() );
		if ( nLobbyID != 0 )
		{
			KeyValues *pSettings = KeyValues::FromString(
				"settings",
				" system { "
					" network LIVE "
				" } "
				" options { "
					" action joinsession "
				" } "
				);
			pSettings->SetUint64( "options/sessionid", nLobbyID );
			KeyValues::AutoDelete autodelete( pSettings );

			g_pMatchFramework->MatchSession( pSettings );
		}
		// clear the convar so we don't try to join that lobby every time we return to the main menu
		connect_lobby.SetValue( "" );
	}

	BaseClass::OnOpen();

	SetFooterState();

#ifndef _X360
	bool bSteamCloudVisible = false;

	{
		static CGameUIConVarRef cl_cloud_settings( "cl_cloud_settings" );
		if ( cl_cloud_settings.GetInt() == -1 )
		{
#if 0
			CBaseModPanel::GetSingleton().OpenWindow( WT_STEAMCLOUDCONFIRM, this, false );
			bSteamCloudVisible = true;
#else
			// Default to allowing Steam Cloud; if the user doesn't want it, they can disable
			// it in game properties or via the menu after initial game launch.
			cl_cloud_settings.SetValue( STEAMREMOTESTORAGE_CLOUD_ALL );
#endif
		}
	}

	if ( !bSteamCloudVisible )
	{
		if ( AddonAssociation::CheckAndSeeIfShouldShow() )
		{
			CBaseModPanel::GetSingleton().OpenWindow( WT_ADDONASSOCIATION, this, false );
		}
	}
#endif
}

//=============================================================================
void MainMenu::RunFrame()
{
	BaseClass::RunFrame();
}

//=============================================================================
void MainMenu::Activate()
{
	BaseClass::Activate();

	vgui::Panel *firstPanel = FindChildByName( "BtnMultiplayer" );
	if ( firstPanel )
	{
		if ( m_ActiveControl )
		{
			m_ActiveControl->NavigateFrom( );
		}
		firstPanel->NavigateTo();
	}

	g_RD_HUD_Sheets.VidInit();

	// don't query group servers over and over; we don't need them
	if ( g_pMatchFramework && g_pMatchFramework->GetMatchSystem() && g_pMatchFramework->GetMatchSystem()->GetUserGroupsServerManager() )
		g_pMatchFramework->GetMatchSystem()->GetUserGroupsServerManager()->EnableServersUpdate( false );

	// reactivedrop: reset the value each time we go in main menu
	// for us to be able to browse lobbies with up to 32 slots
	mm_max_players.Revert();

	// we've left whatever server we were on; get rid of the stuff we borrowed
	g_ReactiveDropWorkshop.UnloadTemporaryAddons();

	static bool s_bRunOnce = true;
	if ( s_bRunOnce )
	{
		if ( SteamNetworkingUtils() )
		{
			// need to know our ping location to show pings in the lobby browser
			SteamNetworkingUtils()->InitRelayNetworkAccess();
		}

		if ( ConVarRef( "net_steamcnx_allowrelay" ).GetBool() )
		{
			// if relayed connections are enabled, use them by default instead of trying direct IPv4 UDP first
			ConVarRef( "net_steamcnx_enabled" ).SetValue( 2 );
		}

		// see if we earned any goodies since last time we played
		ReactiveDropInventory::RequestGenericPromoItems();

		// update soundcache on initial load
		engine->ClientCmd_Unrestricted( "snd_restart; update_addon_paths; mission_reload; snd_updateaudiocache; snd_restart" );

		// added support for loadout editor, by element109
		engine->ClientCmd( "execifexists loadouts" );

		s_bRunOnce = false;
	}

	KeyValues::AutoDelete pLatestUpdate{ "RDLatestUpdate" };
	if ( !UTIL_RD_LoadKeyValuesFromFile( pLatestUpdate, g_pFullFileSystem, "resource/rd_event_config.txt", "GAME" ) )
	{
		Warning( "Failed to load resource/rd_event_config.txt!\n" );
	}
	else
	{
		char szBranchName[64]{};
		if ( SteamApps() && SteamApps()->GetCurrentBetaName( szBranchName, sizeof(szBranchName) ) && szBranchName[0] != '\0' )
		{
			// we are on a branch. doesn't matter if it's beta, releasecandidate,
			// bugtest, or something else. show the beta notes message.
			m_pBtnUpdateNotes->SetText( "#rd_view_beta_update_notes" );
		}
		else
		{
			int iPatchDate = pLatestUpdate->GetInt( "patch" );
			int iPatchDay = iPatchDate % 100;
			Assert( iPatchDay >= 1 && iPatchDay <= 31 );
			iPatchDate /= 100;
			int iPatchMonth = iPatchDate % 100;
			Assert( iPatchMonth >= 1 && iPatchMonth <= 12 );
			iPatchDate /= 100;
			int iPatchYear = iPatchDate;
			Assert( iPatchYear >= 2023 && iPatchYear < 10000 );

			wchar_t wszPatchDay1[3];
			wchar_t wszPatchDay2[3];
			wchar_t wszPatchMonth1[3];
			wchar_t wszPatchMonth2[3];
			wchar_t wszPatchYear[5];
			V_snwprintf( wszPatchDay1, NELEMS( wszPatchDay1 ), L"%d", iPatchDay );
			V_snwprintf( wszPatchDay2, NELEMS( wszPatchDay2 ), L"%02d", iPatchDay );
			V_snwprintf( wszPatchMonth1, NELEMS( wszPatchMonth1 ), L"%d", iPatchMonth );
			V_snwprintf( wszPatchMonth2, NELEMS( wszPatchMonth2 ), L"%02d", iPatchMonth );
			V_snwprintf( wszPatchYear, NELEMS( wszPatchYear ), L"%d", iPatchYear );

			wchar_t wszPatchText[1024];
			g_pVGuiLocalize->ConstructString( wszPatchText, sizeof( wszPatchText ), g_pVGuiLocalize->Find( "#rd_latest_update_ymd" ), 5, wszPatchYear, wszPatchMonth2, wszPatchDay2, wszPatchMonth1, wszPatchDay1 );
			m_pBtnUpdateNotes->SetText( wszPatchText );
		}

		m_pBtnUpdateNotes->SetVisible( true );

		for ( int i = 0; i < NELEMS( m_wszNewsTitle ); i++ )
		{
			TryLocalize( pLatestUpdate->GetString( VarArgs( "major%d_title", i + 1 ) ), m_wszNewsTitle[i], sizeof( m_wszNewsTitle[i] ) );
			V_strncpy( m_szNewsURL[i], pLatestUpdate->GetString( VarArgs( "major%d_url", i + 1 ) ), sizeof( m_szNewsURL[i] ) );
			if ( m_wszNewsTitle[i][0] != L'\0' && m_iNewsImageTexture[i] == -1 )
			{
				m_iNewsImageTexture[i] = vgui::surface()->CreateNewTextureID();
				vgui::surface()->DrawSetTextureFile( m_iNewsImageTexture[i], CFmtStr{ "vgui/swarm/news_showcase_%d", i + 1 }, 1, false );
			}
		}

		for ( int i = 0; i < NELEMS( m_wszEventTitle ); i++ )
		{
			TryLocalize( pLatestUpdate->GetString( VarArgs( "event%d_title", i + 1 ) ), m_wszEventTitle[i], sizeof( m_wszEventTitle[i] ) );
			V_strncpy( m_szEventURL[i], pLatestUpdate->GetString( VarArgs( "event%d_url", i + 1 ) ), sizeof( m_szEventURL[i] ) );
			m_iEventStarts[i] = pLatestUpdate->GetInt( VarArgs( "event%d_starts", i + 1 ) );
			m_iEventEnds[i] = pLatestUpdate->GetInt( VarArgs( "event%d_ends", i + 1 ) );
		}

		// force an update
		m_iLastTimerUpdate = 0;
	}

	ISteamUserStats *pUserStats = SteamUserStats();
	if ( pUserStats && rd_hoiaf_leaderboard_on_main_menu.GetBool() )
	{
		SteamAPICall_t hCall = pUserStats->DownloadLeaderboardEntries( STEAM_LEADERBOARD_HOIAF_CURRENT_SEASON, k_ELeaderboardDataRequestGlobal, 1, 10 );
		m_HoIAFTop10Callback.Set( hCall, this, &MainMenu::OnHoIAFTop10ScoresDownloaded );
	}

	ISteamUGC *pUGC = SteamUGC();
	if ( pUGC && rd_trending_workshop_tags.GetString()[0] != '\0' )
	{
		CSplitString AllowedTags( rd_trending_workshop_tags.GetString(), "," );
		UGCQueryHandle_t hQuery = pUGC->CreateQueryAllUGCRequest( k_EUGCQuery_RankedByTrend, k_EUGCMatchingUGCType_Items_ReadyToUse, 563560, 563560 );
		SteamParamStringArray_t RequiredTags;
		RequiredTags.m_ppStrings = const_cast< const char ** >( AllowedTags.Base() );
		RequiredTags.m_nNumStrings = AllowedTags.Count();
		pUGC->AddRequiredTagGroup( hQuery, &RequiredTags );
		if ( ISteamApps *pApps = SteamApps() )
			pUGC->SetLanguage( hQuery, pApps->GetCurrentGameLanguage() );
		pUGC->SetAllowCachedResponse( hQuery, 3600 );
		SteamAPICall_t hCall = pUGC->SendQueryUGCRequest( hQuery );
		m_WorkshopTrendingItemsCallback.Set( hCall, this, &MainMenu::OnWorkshopTrendingItems );
	}
}

//=============================================================================
void MainMenu::PaintBackground()
{
	if ( m_bIsStub )
		return;

	int w, t;
	GetSize( w, t );

	vgui::surface()->DrawSetColor( 255, 255, 255, 255 );
	vgui::surface()->DrawSetTexture( g_RD_HUD_Sheets.m_nMainMenuSheetID );

	using Sheet = CRD_HUD_Sheets;

	/*
		TODO:

		DECLARE_HUD_SHEET_UV( create_lobby_profile_hover ),
		DECLARE_HUD_SHEET_UV( logo_profile_hover ),
		DECLARE_HUD_SHEET_UV( settings_profile_hover ),
		DECLARE_HUD_SHEET_UV( top_button_profile_hover ),
	*/

	int x0, y0, x1, y1, iTex;

	x0 = 0;
	y0 = 0;
	x1 = YRES( 240 );
	y1 = YRES( 24 );
	iTex = Sheet::UV_top_bar_left;
	if ( m_pBtnSettings->GetCurrentState() == BaseModHybridButton::Focus )
		iTex = Sheet::UV_top_bar_left_settings_glow;
	else if ( m_pBtnLogo->GetCurrentState() == BaseModHybridButton::Focus )
		iTex = Sheet::UV_top_bar_left_logo_glow;
	else if ( false ) // TODO
		iTex = Sheet::UV_top_bar_left_profile_glow;
	vgui::surface()->DrawTexturedSubRect( x0, y0, x1, y1, HUD_UV_COORDS( MainMenuSheet, iTex ) );

	x0 = x1;
	x1 = w - YRES( 240 );
	iTex = Sheet::UV_top_bar;
	vgui::surface()->DrawTexturedSubRect( x0, y0, x1, y1, HUD_UV_COORDS( MainMenuSheet, iTex ) );

	x0 = x1;
	x1 = w;
	iTex = Sheet::UV_top_bar_right;
	if ( m_pBtnQuit->GetCurrentState() == BaseModHybridButton::Focus )
		iTex = Sheet::UV_top_bar_right_quit_glow;
	else if ( false ) // TODO
		iTex = Sheet::UV_top_bar_right_hoiaf_glow;
	vgui::surface()->DrawTexturedSubRect( x0, y0, x1, y1, HUD_UV_COORDS( MainMenuSheet, iTex ) );

	iTex = Sheet::UV_top_bar_button_glow;
	x0 = YRES( -64 );
	x1 = YRES( 64 );
	for ( int i = 0; i < NELEMS( m_pTopButton ); i++ )
	{
		if ( m_pTopButton[i]->GetCurrentState() == BaseModHybridButton::Focus )
		{
			int x, y, wide, tall;
			m_pTopButton[i]->GetBounds( x, y, wide, tall );
			x += wide / 2;
			x0 += x;
			x1 += x;
			vgui::surface()->DrawTexturedSubRect( x0, y0, x1, y1, HUD_UV_COORDS( MainMenuSheet, iTex ) );
			break;
		}
	}

	x0 = 0;
	y0 = t - YRES( 20 );
	x1 = YRES( 150 );
	y1 = t;
	iTex = Sheet::UV_ticker_left;
	if ( m_pBtnWorkshopShowcase->GetCurrentState() == BaseModHybridButton::Focus )
		iTex = Sheet::UV_ticker_left_workshop_hover;
	vgui::surface()->DrawTexturedSubRect( x0, y0, x1, y1, HUD_UV_COORDS( MainMenuSheet, iTex ) );

	x0 = x1;
	x1 = w - YRES( 225 );
	iTex = Sheet::UV_ticker_mid;
	vgui::surface()->DrawTexturedSubRect( x0, y0, x1, y1, HUD_UV_COORDS( MainMenuSheet, iTex ) );

	x0 = x1;
	x1 = w;
	iTex = Sheet::UV_ticker_right;
	if ( m_pBtnUpdateNotes->GetCurrentState() == BaseModHybridButton::Focus )
		iTex = Sheet::UV_ticker_right_update_hover;
	vgui::surface()->DrawTexturedSubRect( x0, y0, x1, y1, HUD_UV_COORDS( MainMenuSheet, iTex ) );

	if ( m_pBtnSettings->IsVisible() )
	{
		iTex = Sheet::UV_settings;
		if ( m_pBtnSettings->GetCurrentState() == BaseModHybridButton::Focus )
			iTex = Sheet::UV_settings_hover;
		else if ( m_pBtnLogo->GetCurrentState() == BaseModHybridButton::Focus )
			iTex = Sheet::UV_settings_logo_hover;
		m_pBtnSettings->GetBounds( x0, y0, x1, y1 );
		vgui::surface()->DrawTexturedSubRect( x0, y0, x0 + x1, y0 + y1, HUD_UV_COORDS( MainMenuSheet, iTex ) );
	}
	if ( m_pBtnLogo->IsVisible() )
	{
		iTex = Sheet::UV_logo;
		if ( m_pBtnLogo->GetCurrentState() == BaseModHybridButton::Focus )
			iTex = Sheet::UV_logo_hover;
		else if ( m_pBtnSettings->GetCurrentState() == BaseModHybridButton::Focus )
			iTex = Sheet::UV_logo_settings_hover;
		m_pBtnLogo->GetBounds( x0, y0, x1, y1 );
		vgui::surface()->DrawTexturedSubRect( x0, y0, x0 + x1, y0 + y1, HUD_UV_COORDS( MainMenuSheet, iTex ) );
	}
	for ( int i = 0; i < NELEMS( m_pTopButton ); i++ )
	{
		if ( m_pTopButton[i]->IsVisible() )
		{
			iTex = Sheet::UV_top_button;
			if ( m_pTopButton[i]->GetCurrentState() == BaseModHybridButton::Focus )
				iTex = Sheet::UV_top_button_hover;
			else if ( i > 0 && m_pTopButton[i - 1]->GetCurrentState() == BaseModHybridButton::Focus )
				iTex = Sheet::UV_top_button_left_hover;
			else if ( i < NELEMS( m_pTopButton ) - 1 && m_pTopButton[i + 1]->GetCurrentState() == BaseModHybridButton::Focus )
				iTex = Sheet::UV_top_button_right_hover;
			m_pTopButton[i]->GetBounds( x0, y0, x1, y1 );
			vgui::surface()->DrawTexturedSubRect( x0, y0, x0 + x1, y0 + y1, HUD_UV_COORDS( MainMenuSheet, iTex ) );
		}
	}
	if ( m_pBtnQuit->IsVisible() )
	{
		iTex = Sheet::UV_quit;
		if ( m_pBtnQuit->GetCurrentState() == BaseModHybridButton::Focus )
			iTex = Sheet::UV_quit_hover;
		m_pBtnQuit->GetBounds( x0, y0, x1, y1 );
		vgui::surface()->DrawTexturedSubRect( x0, y0, x0 + x1, y0 + y1, HUD_UV_COORDS( MainMenuSheet, iTex ) );
	}
	if ( m_pBtnMultiplayer->IsVisible() )
	{
		iTex = Sheet::UV_create_lobby;
		if ( m_pBtnMultiplayer->GetCurrentState() == BaseModHybridButton::Focus )
			iTex = Sheet::UV_create_lobby_hover;
		if ( m_pBtnSingleplayer->GetCurrentState() == BaseModHybridButton::Focus )
			iTex = Sheet::UV_create_lobby_singleplayer_hover;
		m_pBtnMultiplayer->GetBounds( x0, y0, x1, y1 );
		vgui::surface()->DrawTexturedSubRect( x0, y0, x0 + x1, y0 + y1, HUD_UV_COORDS( MainMenuSheet, iTex ) );
	}
	if ( m_pBtnSingleplayer->IsVisible() )
	{
		iTex = Sheet::UV_singleplayer;
		if ( m_pBtnSingleplayer->GetCurrentState() == BaseModHybridButton::Focus )
			iTex = Sheet::UV_singleplayer_hover;
		else if ( m_pBtnMultiplayer->GetCurrentState() == BaseModHybridButton::Focus )
			iTex = Sheet::UV_singleplayer_create_lobby_hover;
		else if ( m_pPnlQuickJoinPublic->HasMouseover() )
			iTex = Sheet::UV_singleplayer_quick_join_hover;
		m_pBtnSingleplayer->GetBounds( x0, y0, x1, y1 );
		vgui::surface()->DrawTexturedSubRect( x0, y0, x0 + x1, y0 + y1, HUD_UV_COORDS( MainMenuSheet, iTex ) );
	}
	if ( m_pPnlQuickJoinPublic->IsVisible() )
	{
		iTex = Sheet::UV_quick_join;
		if ( m_pPnlQuickJoinPublic->HasMouseover() )
			iTex = Sheet::UV_quick_join_hover;
		else if ( m_pPnlQuickJoin->HasMouseover() )
			iTex = Sheet::UV_quick_join_below_hover;
		else if ( m_pBtnSingleplayer->GetCurrentState() == BaseModHybridButton::Focus )
			iTex = Sheet::UV_quick_join_singleplayer_hover;
		m_pPnlQuickJoinPublic->GetBounds( x0, y0, x1, y1 );
		vgui::surface()->DrawTexturedSubRect( x0, y0, x0 + x1, y0 + y1, HUD_UV_COORDS( MainMenuSheet, iTex ) );
	}
	if ( m_pPnlQuickJoin->IsVisible() )
	{
		iTex = Sheet::UV_quick_join;
		if ( m_pPnlQuickJoin->HasMouseover() )
			iTex = Sheet::UV_quick_join_hover;
		else if ( m_pPnlQuickJoinPublic->HasMouseover() )
			iTex = Sheet::UV_quick_join_above_hover;
		else if ( m_pBtnWorkshopShowcase->GetCurrentState() == BaseModHybridButton::Focus )
			iTex = Sheet::UV_quick_join_below_hover;
		m_pPnlQuickJoin->GetBounds( x0, y0, x1, y1 );
		vgui::surface()->DrawTexturedSubRect( x0, y0, x0 + x1, y0 + y1, HUD_UV_COORDS( MainMenuSheet, iTex ) );
	}
	if ( m_pBtnWorkshopShowcase->IsVisible() )
	{
		iTex = Sheet::UV_workshop;
		if ( m_pBtnWorkshopShowcase->GetCurrentState() == BaseModHybridButton::Focus )
			iTex = Sheet::UV_workshop_hover;
		else if ( m_pPnlQuickJoin->HasMouseover() )
			iTex = Sheet::UV_workshop_quick_join_hover;
		m_pBtnWorkshopShowcase->GetBounds( x0, y0, x1, y1 );
		vgui::surface()->DrawTexturedSubRect( x0, y0, x0 + x1, y0 + y1, HUD_UV_COORDS( MainMenuSheet, iTex ) );
	}
	for ( int i = 0; i < NELEMS( m_pTopLeaderboardEntries ); i++ )
	{
		if ( m_pTopLeaderboardEntries[i]->IsVisible() )
		{
			iTex = i == 0 ? Sheet::UV_hoiaf_top_1 : Sheet::UV_hoiaf_top_10;
			if ( m_pTopLeaderboardEntries[i]->GetCurrentState() == BaseModHybridButton::Focus )
				iTex = i == 0 ? Sheet::UV_hoiaf_top_1_hover : Sheet::UV_hoiaf_top_10_hover;
			else if ( m_pBtnQuit->GetCurrentState() == BaseModHybridButton::Focus )
			{
				if ( i == 0 )
					iTex = Sheet::UV_hoiaf_top_1_quit_hover;
				else if ( i == 1 )
					iTex = Sheet::UV_hoiaf_top_10_quit_hover_1;
				else if ( i == 2 )
					iTex = Sheet::UV_hoiaf_top_10_quit_hover_2;
				else if ( i == 3 )
					iTex = Sheet::UV_hoiaf_top_10_quit_hover_3;
				else if ( i == 4 )
					iTex = Sheet::UV_hoiaf_top_10_quit_hover_4;
				else if ( i == 5 )
					iTex = Sheet::UV_hoiaf_top_10_quit_hover_5;
				else if ( i == 6 )
					iTex = Sheet::UV_hoiaf_top_10_quit_hover_6;
				else if ( i == 7 )
					iTex = Sheet::UV_hoiaf_top_10_quit_hover_7;
				else if ( i == 8 )
					iTex = Sheet::UV_hoiaf_top_10_quit_hover_8;
			}
			else if ( i == 0 && m_pTopLeaderboardEntries[1]->GetCurrentState() == BaseModHybridButton::Focus )
				iTex = Sheet::UV_hoiaf_top_1_below_hover;
			else if ( i < NELEMS( m_pTopLeaderboardEntries ) - 1 && m_pTopLeaderboardEntries[i + 1]->GetCurrentState() == BaseModHybridButton::Focus )
				iTex = Sheet::UV_hoiaf_top_10_below_hover;
			else if ( i > 0 && m_pTopLeaderboardEntries[i - 1]->GetCurrentState() == BaseModHybridButton::Focus )
				iTex = Sheet::UV_hoiaf_top_10_above_hover;
			else if ( ( i == NELEMS( m_pTopLeaderboardEntries ) - 1 || !m_pTopLeaderboardEntries[i + 1]->IsVisible() ) && m_pBtnHoIAFTimer->GetCurrentState() == BaseModHybridButton::Focus )
				iTex = Sheet::UV_hoiaf_top_10_hoiaf_timer_hover;
			m_pTopLeaderboardEntries[i]->GetBounds( x0, y0, x1, y1 );
			// add 1 pixel on each side so these don't look weird with rounding errors
			vgui::surface()->DrawTexturedSubRect( x0 - 1, y0 - 1, x0 + x1 + 1, y0 + y1 + 1, HUD_UV_COORDS( MainMenuSheet, iTex ) );
		}
	}
	if ( m_pBtnHoIAFTimer->IsVisible() )
	{
		iTex = Sheet::UV_hoiaf_timer;
		if ( m_pBtnHoIAFTimer->GetCurrentState() == BaseModHybridButton::Focus )
			iTex = Sheet::UV_hoiaf_timer_hover;
		else if ( m_pTopLeaderboardEntries[NELEMS( m_pTopLeaderboardEntries ) - 1]->IsVisible() && m_pBtnEventTimer[NELEMS( m_pBtnEventTimer ) - 1]->GetCurrentState() == BaseModHybridButton::Focus )
			iTex = Sheet::UV_hoiaf_timer_event_timer_hover;
		else if ( !m_pTopLeaderboardEntries[0]->IsVisible() && m_pBtnQuit->GetCurrentState() == BaseModHybridButton::Focus )
			iTex = Sheet::UV_hoiaf_top_1_quit_hover;
		else
		{
			for ( int i = NELEMS( m_pTopLeaderboardEntries ) - 1; i >= 0; i-- )
			{
				if ( m_pTopLeaderboardEntries[i]->IsVisible() )
				{
					if ( m_pTopLeaderboardEntries[i]->GetCurrentState() == BaseModHybridButton::Focus )
						iTex = Sheet::UV_hoiaf_timer_hoiaf_top_10_hover;
					break;
				}
			}
		}
		m_pBtnHoIAFTimer->GetBounds( x0, y0, x1, y1 );
		vgui::surface()->DrawTexturedSubRect( x0, y0, x0 + x1, y0 + y1, HUD_UV_COORDS( MainMenuSheet, iTex ) );
	}
	for ( int i = 0; i < NELEMS( m_pBtnEventTimer ); i++ )
	{
		if ( m_pBtnEventTimer[i]->IsVisible() )
		{
			iTex = Sheet::UV_event_timer;
			if ( m_pBtnEventTimer[i]->GetCurrentState() == BaseModHybridButton::Focus )
				iTex = Sheet::UV_event_timer_hover;
			else if ( i == 0 && m_pBtnNewsShowcase->GetCurrentState() == BaseModHybridButton::Focus )
				iTex = Sheet::UV_event_timer_news_hover;
			else if ( i == NELEMS( m_pBtnEventTimer ) - 1 && m_pTopLeaderboardEntries[NELEMS( m_pTopLeaderboardEntries ) - 1]->IsVisible() && m_pBtnHoIAFTimer->GetCurrentState() == BaseModHybridButton::Focus )
				iTex = Sheet::UV_event_timer_hoiaf_timer_hover;
			else if ( i > 0 && m_pBtnEventTimer[i - 1]->GetCurrentState() == BaseModHybridButton::Focus )
				iTex = Sheet::UV_event_timer_below_hover;
			else if ( i < NELEMS( m_pBtnEventTimer ) - 1 && m_pBtnEventTimer[i + 1]->GetCurrentState() == BaseModHybridButton::Focus )
				iTex = Sheet::UV_event_timer_above_hover;
			m_pBtnEventTimer[i]->GetBounds( x0, y0, x1, y1 );
			vgui::surface()->DrawTexturedSubRect( x0, y0, x0 + x1, y0 + y1, HUD_UV_COORDS( MainMenuSheet, iTex ) );
		}
	}
	if ( m_pBtnNewsShowcase->IsVisible() )
	{
		iTex = Sheet::UV_news;
		if ( m_pBtnNewsShowcase->GetCurrentState() == BaseModHybridButton::Focus )
			iTex = Sheet::UV_news_hover;
		else if ( m_pBtnUpdateNotes->GetCurrentState() == BaseModHybridButton::Focus )
			iTex = Sheet::UV_news_update_hover;
		else if ( m_pBtnEventTimer[0]->GetCurrentState() == BaseModHybridButton::Focus )
			iTex = Sheet::UV_news_event_timer_hover;
		m_pBtnNewsShowcase->GetBounds( x0, y0, x1, y1 );
		vgui::surface()->DrawTexturedSubRect( x0, y0, x0 + x1, y0 + y1, HUD_UV_COORDS( MainMenuSheet, iTex ) );
	}
	if ( m_pBtnUpdateNotes->IsVisible() )
	{
		iTex = Sheet::UV_update;
		if ( m_pBtnUpdateNotes->GetCurrentState() == BaseModHybridButton::Focus )
			iTex = Sheet::UV_update_hover;
		else if ( m_pBtnNewsShowcase->GetCurrentState() == BaseModHybridButton::Focus )
			iTex = Sheet::UV_update_news_hover;
		m_pBtnUpdateNotes->GetBounds( x0, y0, x1, y1 );
		vgui::surface()->DrawTexturedSubRect( x0, y0, x0 + x1, y0 + y1, HUD_UV_COORDS( MainMenuSheet, iTex ) );
	}

	int iCurrentMinute = ( std::time( NULL ) / 60 );

	int iNumNewsShowcases = 0;
	while ( iNumNewsShowcases < NELEMS( m_wszNewsTitle ) && m_wszNewsTitle[iNumNewsShowcases][0] != L'\0' )
		iNumNewsShowcases++;

	if ( iNumNewsShowcases && m_pBtnNewsShowcase->IsVisible() )
	{
		vgui::surface()->DrawSetTexture( m_iNewsImageTexture[iCurrentMinute % iNumNewsShowcases] );
		vgui::surface()->DrawSetColor( 255, 255, 255, m_pBtnNewsShowcase->GetCurrentState() == BaseModHybridButton::Focus ? 224 : 255 );
		m_pBtnNewsShowcase->GetBounds( x0, y0, x1, y1 );
		vgui::surface()->DrawTexturedRect( x0 + YRES( 1 ), y0 + YRES( 1 ), x0 + x1 - YRES( 1 ), y0 + y1 - YRES( 1 ) );
		vgui::surface()->DrawSetColor( 255, 255, 255, 255 );
	}

	int iCurrentWorkshopShowcase = iCurrentMinute % NELEMS( m_pWorkshopTrendingPreview );
	if ( m_pBtnWorkshopShowcase->IsVisible() && m_pWorkshopTrendingPreview[iCurrentWorkshopShowcase] )
	{
		vgui::surface()->DrawSetTexture( m_pWorkshopTrendingPreview[iCurrentWorkshopShowcase]->GetID() );
		vgui::surface()->DrawSetColor( 255, 255, 255, m_pBtnWorkshopShowcase->GetCurrentState() == BaseModHybridButton::Focus ? 224 : 255 );

		int iw, it, cw, ct;
		vgui::surface()->DrawGetTextureSize( m_pWorkshopTrendingPreview[iCurrentWorkshopShowcase]->GetID(), iw, it );
		m_pWorkshopTrendingPreview[iCurrentWorkshopShowcase]->GetContentSize( cw, ct );
		m_pBtnWorkshopShowcase->GetBounds( x0, y0, x1, y1 );

		vgui::surface()->DrawTexturedSubRect( x0 + YRES( 1 ), y0 + YRES( 1 ), x0 + x1 - YRES( 1 ), y0 + y1 - YRES( 1 ), 0, 0, cw / ( float )iw, ct / ( float )it );
		vgui::surface()->DrawSetColor( 255, 255, 255, 255 );
	}
}

void MainMenu::SetFooterState()
{
	CBaseModFooterPanel *footer = BaseModUI::CBaseModPanel::GetSingleton().GetFooterPanel();
	if ( footer )
	{
		CBaseModFooterPanel::FooterButtons_t buttons = FB_ABUTTON;
#if defined( _X360 )
		if ( XBX_GetPrimaryUserIsGuest() == 0 )
		{
			buttons |= FB_XBUTTON;
		}
#endif

		footer->SetButtons( buttons, FF_MAINMENU, false );
		footer->SetButtonText( FB_ABUTTON, "#L4D360UI_Select" );
		footer->SetButtonText( FB_XBUTTON, "#L4D360UI_MainMenu_SeeAll" );
	}
}

void MainMenu::OpenNewsURL( const char *szURL )
{
	char szFormattedURL[1024];
	V_snprintf( szFormattedURL, sizeof( szFormattedURL ), szURL, SteamApps() ? SteamApps()->GetCurrentGameLanguage() : "" );
	CUIGameData::Get()->ExecuteOverlayUrl( szFormattedURL );
}

void MainMenu::LoadLayout()
{
	const char *pSettings = "Resource/UI/BaseModUI/mainmenu.res";
	if ( !g_pMatchFramework || !g_pMatchFramework->GetMatchSystem() || !g_pMatchFramework->GetMatchSystem()->GetPlayerManager() || !g_pMatchFramework->GetMatchSystem()->GetPlayerManager()->GetLocalPlayer( 0 ) || !SteamApps() || !SteamFriends() || !SteamHTTP() || !SteamInput() || !SteamMatchmaking() || !SteamMatchmakingServers() || !SteamUGC() || !SteamUser() || !SteamUserStats() )
	{
		pSettings = "Resource/UI/BaseModUI/MainMenuStub.res";
		m_bIsStub = true;

		for ( int i = GetChildCount() - 1; i >= 0; i-- )
		{
			// Clean up all UI elements we made in the constructor
			vgui::Panel *pChild = GetChild( i );
			if ( pChild && V_stricmp( pChild->GetName(), "BtnStub" ) && V_stricmp( pChild->GetName(), "BtnQuit" ) )
			{
				pChild->SetVisible( false );
			}
		}
	}

	V_strncpy( m_ResourceName, pSettings, sizeof( m_ResourceName ) );

	BaseClass::LoadLayout();
}

void MainMenu::ApplySchemeSettings( IScheme *pScheme )
{
	static bool s_bReloadLocOnce = true;
	if ( s_bReloadLocOnce )
	{
		// BenLubar: load translations again right before the main menu appears for the first time
		UTIL_RD_ReloadLocalizeFiles();
		s_bReloadLocOnce = false;
	}

	BaseClass::ApplySchemeSettings( pScheme );

	BaseModHybridButton *button = dynamic_cast< BaseModHybridButton* >( FindChildByName( "BtnPlaySolo" ) );
	if ( button )
	{
#ifdef _X360
		button->SetText( ( XBX_GetNumGameUsers() > 1 ) ? ( "#L4D360UI_MainMenu_PlaySplitscreen" ) : ( "#L4D360UI_MainMenu_PlaySolo" ) );
		button->SetHelpText( ( XBX_GetNumGameUsers() > 1 ) ? ( "#L4D360UI_MainMenu_OfflineCoOp_Tip" ) : ( "#L4D360UI_MainMenu_PlaySolo_Tip" ) );
#endif
	}

#ifdef _X360
	if ( !XBX_GetPrimaryUserIsGuest() )
	{
		wchar_t wszListText[ 128 ];
		wchar_t wszPlayerName[ 128 ];

		IPlayer *player1 = NULL;
		if ( XBX_GetNumGameUsers() > 0 )
		{
			player1 = g_pMatchFramework->GetMatchSystem()->GetPlayerManager()->GetLocalPlayer( XBX_GetUserId( 0 ) );
		}

		IPlayer *player2 = NULL;
		if ( XBX_GetNumGameUsers() > 1 )
		{
			player2 = g_pMatchFramework->GetMatchSystem()->GetPlayerManager()->GetLocalPlayer( XBX_GetUserId( 1 ) );
		}

		if ( player1 )
		{
			Label *pLblPlayer1GamerTag = dynamic_cast< Label* >( FindChildByName( "LblPlayer1GamerTag" ) );
			if ( pLblPlayer1GamerTag )
			{
				g_pVGuiLocalize->ConvertANSIToUnicode( player1->GetName(), wszPlayerName, sizeof( wszPlayerName ) );
				g_pVGuiLocalize->ConstructString( wszListText, sizeof( wszListText ), g_pVGuiLocalize->Find( "#L4D360UI_MainMenu_LocalProfilePlayer1" ), 1, wszPlayerName );

				pLblPlayer1GamerTag->SetVisible( true );
				pLblPlayer1GamerTag->SetText( wszListText );
			}
		}

		if ( player2 )
		{
			Label *pLblPlayer2GamerTag = dynamic_cast< Label* >( FindChildByName( "LblPlayer2GamerTag" ) );
			if ( pLblPlayer2GamerTag )
			{
				g_pVGuiLocalize->ConvertANSIToUnicode( player2->GetName(), wszPlayerName, sizeof( wszPlayerName ) );
				g_pVGuiLocalize->ConstructString( wszListText, sizeof( wszListText ), g_pVGuiLocalize->Find( "#L4D360UI_MainMenu_LocalProfilePlayer2" ), 1, wszPlayerName );

				pLblPlayer2GamerTag->SetVisible( true );
				pLblPlayer2GamerTag->SetText( wszListText );

				// in split screen, have player2 gamer tag instead of enable, and disable
				SetControlVisible( "LblPlayer2DisableIcon", true );
				SetControlVisible( "LblPlayer2Disable", true );
				SetControlVisible( "LblPlayer2Enable", false );
			}
		}
		else
		{
			SetControlVisible( "LblPlayer2DisableIcon", false );
			SetControlVisible( "LblPlayer2Disable", false );

			// not in split screen, no player2 gamertag, instead have enable
			SetControlVisible( "LblPlayer2GamerTag", false );
			SetControlVisible( "LblPlayer2Enable", true );
		}
	}
#endif

	if ( IsPC() )
	{
		FlyoutMenu *pFlyout = dynamic_cast< FlyoutMenu* >( FindChildByName( "FlmOptionsFlyout" ) );
		if ( pFlyout )
		{
			bool bUsesCloud = false;

#ifdef IS_WINDOWS_PC
			ISteamRemoteStorage *pRemoteStorage = SteamRemoteStorage();
#else
			ISteamRemoteStorage *pRemoteStorage =  NULL; 
			AssertMsg( false, "This branch run on a PC build without IS_WINDOWS_PC defined." );
#endif

			uint64 availableBytes, totalBytes = 0;
			if ( pRemoteStorage && pRemoteStorage->GetQuota( &totalBytes, &availableBytes ) )
			{
				if ( totalBytes > 0 )
				{
					bUsesCloud = true;
				}
			}

			pFlyout->SetControlEnabled( "BtnCloud", bUsesCloud );
		}
	}

	SetFooterState();

	vgui::Panel *firstPanel = FindChildByName( "BtnMultiplayer" );
	if ( firstPanel )
	{
		if ( m_ActiveControl )
		{
			m_ActiveControl->NavigateFrom( );
		}
		firstPanel->NavigateTo();
	}

#if defined( _X360 ) && defined( _DEMO )
	SetControlVisible( "BtnExtras", !engine->IsDemoHostedFromShell() );
	SetControlVisible( "BtnQuit", engine->IsDemoHostedFromShell() );
#endif

	// CERT CATCH ALL JUST IN CASE!
#ifdef _X360
	bool bAllUsersCorrectlySignedIn = ( XBX_GetNumGameUsers() > 0 );
	for ( int k = 0; k < ( int ) XBX_GetNumGameUsers(); ++ k )
	{
		if ( !g_pMatchFramework->GetMatchSystem()->GetPlayerManager()->GetLocalPlayer( XBX_GetUserId( k ) ) )
			bAllUsersCorrectlySignedIn = false;
	}
	if ( !bAllUsersCorrectlySignedIn )
	{
		Warning( "======= SIGNIN FAIL SIGNIN FAIL SIGNIN FAIL SIGNIN FAIL ==========\n" );
		Assert( 0 );
		CBaseModPanel::GetSingleton().CloseAllWindows( CBaseModPanel::CLOSE_POLICY_EVEN_MSGS );
		CAttractScreen::SetAttractMode( CAttractScreen::ATTRACT_GAMESTART );
		CBaseModPanel::GetSingleton().OpenWindow( WT_ATTRACTSCREEN, NULL, true );
		Warning( "======= SIGNIN RESET SIGNIN RESET SIGNIN RESET SIGNIN RESET ==========\n" );
	}
#endif

	vgui::Label *pBranchDisclaimer = dynamic_cast< vgui::Label * >( FindChildByName( "LblBranchDisclaimer" ) );
	ISteamApps *pApps = SteamApps();
	if ( pBranchDisclaimer && pApps )
	{
		char szBranch[256]{};
		if ( !pApps->GetCurrentBetaName( szBranch, sizeof( szBranch ) ) )
		{
			pBranchDisclaimer->SetVisible( false );
		}
		else
		{
			pBranchDisclaimer->SetText( VarArgs( "#rd_branch_disclaimer_%s", szBranch ) );
			pBranchDisclaimer->SetVisible( true );
		}
	}

	// force an update
	m_iLastTimerUpdate = 0;

	if ( m_iHoIAFTimerOffset >= 0 )
	{
		int x, y, discard;
		m_pBtnHoIAFTimer->GetPos( x, discard );
		m_pTopLeaderboardEntries[0]->GetPos( discard, y );

		for ( int i = 0; i < NELEMS( m_pTopLeaderboardEntries ); i++ )
		{
			if ( !m_pTopLeaderboardEntries[i]->IsVisible() )
				break;

			m_pTopLeaderboardEntries[i]->GetPos( discard, y );
			y += m_pTopLeaderboardEntries[i]->GetTall() + m_iHoIAFTimerOffset;
		}
		m_pBtnHoIAFTimer->SetPos( x, y );
	}
}

void MainMenu::AcceptCommentaryRulesCallback() 
{
	if ( MainMenu *pMainMenu = static_cast< MainMenu* >( CBaseModPanel::GetSingleton().GetWindow( WT_MAINMENU ) ) )
	{
		KeyValues *pSettings = KeyValues::FromString(
			"settings",
			" system { "
				" network offline "
			" } "
			" game { "
				" mode single_mission "
			" } "
			" options { "
				" play commentary "
			" } "
			);
		KeyValues::AutoDelete autodelete( pSettings );

		g_pMatchFramework->CreateSession( pSettings );
	}

}

void MainMenu::AcceptSplitscreenDisableCallback()
{
	if ( MainMenu *pMainMenu = static_cast< MainMenu* >( CBaseModPanel::GetSingleton().GetWindow( WT_MAINMENU ) ) )
	{
		pMainMenu->OnCommand( "DisableSplitscreen_NoConfirm" );
	}
}

void MainMenu::AcceptQuitGameCallback()
{
	if ( MainMenu *pMainMenu = static_cast< MainMenu* >( CBaseModPanel::GetSingleton().GetWindow( WT_MAINMENU ) ) )
	{
		pMainMenu->OnCommand( "QuitGame_NoConfirm" );
	}
}

void MainMenu::AcceptVersusSoftLockCallback()
{
	if ( MainMenu *pMainMenu = static_cast< MainMenu* >( CBaseModPanel::GetSingleton().GetWindow( WT_MAINMENU ) ) )
	{
		pMainMenu->OnCommand( "FlmVersusFlyout" );
	}
}

void MainMenu::OnHoIAFTop10ScoresDownloaded( LeaderboardScoresDownloaded_t *pParam, bool bIOFailure )
{
	for ( int i = 0; i < NELEMS( m_pTopLeaderboardEntries ); i++ )
	{
		m_pTopLeaderboardEntries[i]->ClearData();
		m_pTopLeaderboardEntries[i]->SetVisible( false );
	}

	if ( !bIOFailure && !m_bIsStub )
	{
		for ( int i = 0; i < pParam->m_cEntryCount; i++ )
		{
			Assert( i < NELEMS( m_pTopLeaderboardEntries ) );
			if ( i >= NELEMS( m_pTopLeaderboardEntries ) )
				break;

			LeaderboardEntry_t entry;
			LeaderboardScoreDetails_Points_t details;
			bool bOK = SteamUserStats()->GetDownloadedLeaderboardEntry( pParam->m_hSteamLeaderboardEntries, i, &entry, reinterpret_cast< int32 * >( &details ), sizeof( details ) / sizeof( int32 ) );
			Assert( bOK );
			Assert( entry.m_cDetails == sizeof( details ) / sizeof( int32 ) );

			m_pTopLeaderboardEntries[i]->SetFromEntry( entry, details );
			m_pTopLeaderboardEntries[i]->SetVisible( true );
		}
	}

	if ( m_iHoIAFTimerOffset >= 0 )
	{
		int x, y, discard;
		m_pBtnHoIAFTimer->GetPos( x, discard );
		m_pTopLeaderboardEntries[0]->GetPos( discard, y );

		for ( int i = 0; i < NELEMS( m_pTopLeaderboardEntries ); i++ )
		{
			if ( !m_pTopLeaderboardEntries[i]->IsVisible() )
				break;

			m_pTopLeaderboardEntries[i]->GetPos( discard, y );
			y += m_pTopLeaderboardEntries[i]->GetTall() + m_iHoIAFTimerOffset;
		}
		m_pBtnHoIAFTimer->SetPos( x, y );
	}
}

void MainMenu::OnWorkshopTrendingItems( SteamUGCQueryCompleted_t *pParam, bool bIOFailure )
{
	if ( bIOFailure || pParam->m_eResult != k_EResultOK )
		return;

	uint32 iCurrentWorkshopShowcase = ( std::time( NULL ) / 60 ) % NELEMS( m_iWorkshopTrendingFileID );
	for ( uint32 i = 0; i < pParam->m_unNumResultsReturned && i < NELEMS( m_iWorkshopTrendingFileID ); i++ )
	{
		SteamUGCDetails_t details;
		bool bOK = SteamUGC()->GetQueryUGCResult( pParam->m_handle, i, &details );
		Assert( bOK );
		if ( bOK )
		{
			m_iWorkshopTrendingFileID[i] = details.m_nPublishedFileId;
			V_UTF8ToUnicode( details.m_rgchTitle, m_wszWorkshopTrendingTitle[i], sizeof( m_wszWorkshopTrendingTitle[i] ) );
			m_hWorkshopTrendingPreview[i] = details.m_hPreviewFile;

			SteamAPICall_t hCall = SteamRemoteStorage()->UGCDownload( details.m_hPreviewFile, 0 );
			m_WorkshopPreviewImageCallback[i].Set( hCall, this, &MainMenu::OnWorkshopPreviewImage );

			if ( iCurrentWorkshopShowcase == i )
			{
				m_pBtnWorkshopShowcase->SetText( m_wszWorkshopTrendingTitle[i] );
			}
		}
	}
}

void MainMenu::OnWorkshopPreviewImage( RemoteStorageDownloadUGCResult_t *pParam, bool bIOFailure )
{
	if ( bIOFailure || pParam->m_eResult != k_EResultOK )
		return;

	for ( int i = 0; i < NELEMS( m_hWorkshopTrendingPreview ); i++ )
	{
		if ( m_hWorkshopTrendingPreview[i] != pParam->m_hFile )
			continue;

		CUtlBuffer buf;
		int32 nBytesRead = SteamRemoteStorage()->UGCRead( pParam->m_hFile, buf.AccessForDirectRead( pParam->m_nSizeInBytes ), pParam->m_nSizeInBytes, 0, k_EUGCRead_Close );
		Assert( nBytesRead == pParam->m_nSizeInBytes );
		buf.SeekPut( CUtlBuffer::SEEK_HEAD, nBytesRead );

		delete m_pWorkshopTrendingPreview[i]; // if we had a previous image, kill it before we put the new one in
		m_pWorkshopTrendingPreview[i] = new CReactiveDropWorkshopPreviewImage{ buf };

		return;
	}

	Assert( !"unexpected workshop preview image handle on main menu" );

	// kill the file handle anyway
	SteamRemoteStorage()->UGCRead( pParam->m_hFile, NULL, 0, 0, k_EUGCRead_Close );
}

#ifndef _X360
CON_COMMAND_F( openserverbrowser, "Opens server browser", 0 )
{
	bool isSteam = IsPC() && SteamFriends() && SteamUtils();
	if ( isSteam )
	{
		// show the server browser
		g_VModuleLoader.ActivateModule("Servers");

		// if an argument was passed, that's the tab index to show, send a message to server browser to switch to that tab
		if ( args.ArgC() > 1 )
		{
			KeyValues *pKV = new KeyValues( "ShowServerBrowserPage" );
			pKV->SetInt( "page", atoi( args[1] ) );
			g_VModuleLoader.PostMessageToAllModules( pKV );
		}

#ifdef INFESTED_DLL
		KeyValues *pSchemeKV = new KeyValues( "SetCustomScheme" );
		pSchemeKV->SetString( "SchemeName", "SwarmServerBrowserScheme" );
		g_VModuleLoader.PostMessageToAllModules( pSchemeKV );
#endif
	}
}
#endif

CON_COMMAND( rd_debug_wine_version, "" )
{
#ifdef IS_WINDOWS_PC
	if ( !wine_get_version )
	{
		Msg( "Cannot find function ntdll.dll!wine_get_version - probably not running Wine.\n" );
		return;
	}

	const char *szVersion = wine_get_version();
	Msg( "Wine Version: %s\n", szVersion );
#else
	Msg( "Not running a Windows build.\n" );
#endif
}
