//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//

#include "cbase.h"
#include "VGameSettings.h"
#include "KeyValues.h"

#include <ctype.h>
#include <vstdlib/random.h>

#include "VDropDownMenu.h"
#include "VHybridButton.h"
#include "VFooterPanel.h"
#include "vgui/ISurface.h"
#include "EngineInterface.h"
#include "VLoadingProgress.h"
#include "VGenericConfirmation.h"
#include "nb_select_mission_panel.h"
#include "nb_select_campaign_panel.h"

#include "vgui_controls/ImagePanel.h"
#include "vgui_controls/Button.h"

#include "fmtstr.h"
#include "smartptr.h"

#include "matchmaking/swarm/imatchext_swarm.h"

#include "missionchooser/iasw_mission_chooser.h"
#include "missionchooser/iasw_mission_chooser_source.h"
#include "nb_header_footer.h"
#include "nb_button.h"

#include "rd_workshop.h"

#include "rd_challenges_shared.h"
#include "rd_challenge_selection.h"

#include "asw_util_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar ui_game_allow_create_public( "ui_game_allow_create_public", IsPC() ? "1" : "0", FCVAR_DEVELOPMENTONLY, "When set user can create public lobbies instead of matching" );
ConVar ui_game_allow_create_random( "ui_game_allow_create_random", "1", FCVAR_DEVELOPMENTONLY, "When set, creating a game will pick a random mission" );
extern ConVar mm_max_players;
extern ConVar rd_last_game_access;
extern ConVar rd_last_game_difficulty;
extern ConVar rd_last_game_challenge;
extern ConVar rd_last_game_onslaught;
extern ConVar rd_last_game_hardcoreff;
extern ConVar rd_last_game_maxplayers;

using namespace vgui;
using namespace BaseModUI;


//=============================================================================
GameSettings::GameSettings( vgui::Panel *parent, const char *panelName ):
	BaseClass( parent, panelName, true, false ),
	m_pSettings( NULL ),
	m_autodelete_pSettings( (KeyValues *)NULL ),
	m_drpDifficulty( NULL ),
	m_drpGameType( NULL ),
	m_drpGameAccess( NULL ),
	m_drpNumSlots( NULL ),
	m_drpServerType( NULL ),
	m_drpStartingMission( NULL ),
	m_bEditingSession( false ),
	m_bAllowChangeToCustomCampaign( true ),
	m_bPreventSessionModifications( false ),
	m_drpFriendlyFire( NULL ),
	m_drpOnslaught( NULL ),
	m_drpChallenge( NULL ),
	m_bWantHardcoreFF( false ),
	m_bWantOnslaught( false )
{
	m_pHeaderFooter = new CNB_Header_Footer( this, "HeaderFooter" );
	m_pHeaderFooter->SetTitle( "" );
	m_pHeaderFooter->SetHeaderEnabled( false );
	m_pHeaderFooter->SetGradientBarEnabled( true );
	m_pHeaderFooter->SetGradientBarPos( 140, 190 );
	m_pTitle = new vgui::Label( this, "Title", "" );
	SetDeleteSelfOnClose(true);
	SetProportional( true );
	SetLowerGarnishEnabled( true );
	SetCancelButtonEnabled( true );
}

//=============================================================================
GameSettings::~GameSettings()
{
}

void GameSettings::SetDataSettings( KeyValues *pSettings )
{
	IMatchSession *pIMatchSession = g_pMatchFramework->GetMatchSession();
	KeyValues *pMatchSettings = pIMatchSession ? pIMatchSession->GetSessionSettings() : NULL;
	if (pMatchSettings)
	{
		KeyValuesDumpAsDevMsg(pMatchSettings);
	}

	if ( pMatchSettings && ( !pSettings || pSettings == pMatchSettings ) )
	{
		m_pSettings = pMatchSettings;
		m_bEditingSession = true;
	}
	else
	{
		Assert( !m_pSettings );

		m_pSettings = pSettings ? pSettings->MakeCopy() : new KeyValues( "settings" );
		m_autodelete_pSettings.Assign( m_pSettings );
		m_bEditingSession = false;
	}
}

void GameSettings::UpdateSessionSettings( KeyValues *pUpdate )
{
	KeyValuesDumpAsDevMsg( pUpdate );
	if ( m_bEditingSession )
	{
		if ( m_bPreventSessionModifications )
			return;

		IMatchSession *pIMatchSession = g_pMatchFramework->GetMatchSession();
		if ( pIMatchSession )
		{
			pIMatchSession->UpdateSessionSettings( pUpdate );
		}
	}
	else
	{
		m_pSettings->MergeFrom( pUpdate );
	}
}

void GameSettings::PaintBackground()
{
	char chBufferTitle[128];
	const char *pTitle = "";

	char chBufferSubTitle[128];
	const char *pSubtitle = "#L4D360UI_GameSettings_Description";

	char const *szNetwork = m_pSettings->GetString( "system/network", "offline" );

	if ( !Q_stricmp( "offline", szNetwork ) )
	{
		pTitle = "#ASUI_GameSettings_Solo";
	}
	else
	{
		Q_snprintf( chBufferTitle, sizeof( chBufferTitle ), "#ASUI_GameSettings_MP_%s", m_pSettings->GetString( "game/mode", "campaign" ) );
		pTitle = chBufferTitle;

		char const *szAccess = m_pSettings->GetString( "system/access", "public" );
		if ( !Q_stricmp( "lan", szNetwork ) )
			szAccess = szNetwork;

		Q_snprintf( chBufferSubTitle, sizeof( chBufferSubTitle ), "#L4D360UI_Access_%s", szAccess );
		pSubtitle = chBufferSubTitle;
	}
	m_pTitle->SetText( pTitle );
/*
	BaseClass::DrawDialogBackground( pTitle, NULL, pSubtitle, NULL );
	*/
}

//=============================================================================
void GameSettings::Activate()
{
	BaseClass::Activate();

	CAutoPushPop< bool > autoPreventSessionModification( m_bPreventSessionModifications, true );

	if ( m_drpGameType )
	{
		const char *szGameMode = m_pSettings->GetString( "game/mode", "campaign" );
		if ( szGameMode )
		{
			if ( !Q_stricmp( szGameMode, "campaign" ) )
			{
				m_drpGameType->SetCurrentSelection( "#ASUI_GameType_Campaign" );
			}
			else if ( !Q_stricmp( szGameMode, "single_mission" ) )
			{
				m_drpGameType->SetCurrentSelection( "#ASUI_GameType_Single_Mission" );
			}	
		}
		UpdateSelectMissionButton();
	}

	char const *szNetwork = m_pSettings->GetString( "system/network", "offline" );

	//bool showGameAccess = !Q_stricmp( "create", m_pSettings->GetString( "options/action", "" ) ) &&
							//!IsEditingExistingLobby();
	// players below level 30 are considered new
	bool bShowNumSlots = UTIL_ASW_CommanderLevelAtLeast( NULL, 30 );

	bool showServerType = false; //!Q_stricmp( "LIVE", szNetwork );
	bool showGameAccess = !Q_stricmp( "LIVE", szNetwork );
	bool showNumSlots = showGameAccess || ( !Q_stricmp( "offline", szNetwork ) && bShowNumSlots );
	
	// On X360 we cannot allow selecting server type until the
	// session is actually created
	if ( IsX360() && showServerType )
		showServerType = IsEditingExistingLobby();

	//bool showSearchControls = IsCustomMatchSearchCriteria();

#ifdef _X360
	bool bPlayingSplitscreen = XBX_GetNumGameUsers() > 1;
#else
	bool bPlayingSplitscreen = false;
#endif

	bool showSinglePlayerControls = !Q_stricmp( "offline", szNetwork ) && !bPlayingSplitscreen;

	m_bBackButtonMeansDone = false; //( !showSearchControls && !showSinglePlayerControls && !showGameAccess );
	m_bCloseSessionOnClose = showSinglePlayerControls;

	if ( m_drpDifficulty )
	{
		m_drpDifficulty->SetCurrentSelection( CFmtStr( "#L4D360UI_Difficulty_%s",
			m_pSettings->GetString( "game/difficulty", "normal" ) ) );

		if ( FlyoutMenu *flyout = m_drpDifficulty->GetCurrentFlyout() )
		{
			flyout->CloseMenu( NULL );

			if ( !UTIL_ASW_CommanderLevelAtLeast( NULL, 30 ) )
			{
				// beginner commanders can't create a lobby with insane or brutal difficulty but can change an existing one to those skill levels.
				vgui::Panel *pBrutal = flyout->FindChildByName( "BtnImba", true );
				if ( pBrutal )
					pBrutal->SetEnabled( false );
			}
		}
	}

	if ( m_drpFriendlyFire )
	{
		if ( m_pSettings->GetInt( "game/hardcoreFF", 0 ) == 1 )
		{
			m_drpFriendlyFire->SetCurrentSelection( "#L4D360UI_HardcoreFF" );
		}
		else
		{
			m_drpFriendlyFire->SetCurrentSelection( "#L4D360UI_RegularFF" );
		}

		if ( FlyoutMenu* flyout = m_drpFriendlyFire->GetCurrentFlyout() )
			flyout->CloseMenu( NULL );
	}

	if ( m_drpOnslaught )
	{
		if ( m_pSettings->GetInt( "game/onslaught", 0 ) == 1 )
		{
			m_drpOnslaught->SetCurrentSelection( "#L4D360UI_OnslaughtEnabled" );
		}
		else
		{
			m_drpOnslaught->SetCurrentSelection( "#L4D360UI_OnslaughtDisabled" );
		}

		if ( FlyoutMenu* flyout = m_drpOnslaught->GetCurrentFlyout() )
			flyout->CloseMenu( NULL );
	}

	if ( m_drpChallenge )
	{
		const char *szChallengeName = m_pSettings->GetString( "game/challenge", "0" );
		m_drpChallenge->SetCurrentSelection( ReactiveDropChallenges::DisplayName( szChallengeName ) );

		UpdateChallenge( szChallengeName );

		if ( FlyoutMenu* flyout = m_drpChallenge->GetCurrentFlyout() )
			flyout->CloseMenu( NULL );
	}

	// If we have an active control, navigate from it since we'll be setting a new one
	if ( m_ActiveControl )
	{
		m_ActiveControl->NavigateFrom();
	}

	// select the Campaign option so controller players can move on this menu
	vgui::Panel *firstOption = FindChildByName( "DrpSelectMission", true );
	if ( firstOption )
	{
		firstOption->NavigateTo();
	}

	CNB_Button *button = dynamic_cast< CNB_Button * >( FindChildByName( "BtnStart" ) );
	if ( button )
	{
		button->SetControllerButton( KEY_XBUTTON_X );
	}

	/*
	BaseModHybridButton *button = dynamic_cast< BaseModHybridButton* >( FindChildByName( "BtnStartGame" ) );
	if( button )
	{
		button->SetVisible( showSinglePlayerControls );
		SetControlVisible( "IconForwardArrow", showSinglePlayerControls );

		if ( IsX360() && button->IsVisible() )
		{
			button->NavigateTo();
		}
	}

	button = dynamic_cast< BaseModHybridButton* > ( FindChildByName( "BtnJoinStart" ) );
	if ( button )
	{
		if ( IsX360() && button->IsVisible() )
		{
			button->NavigateTo();
		}
	}

	button = dynamic_cast< BaseModHybridButton* > ( FindChildByName( "BtnStartLobby" ) );
	if ( button )
	{
		button->SetVisible( showGameAccess );
		if ( IsX360() && button->IsVisible() )
		{
			button->NavigateTo();
		}
	}
	*/

	if ( IsPC() )
	{
		SetControlVisible( "BtnCancel", true );
	}

	if ( m_drpServerType )
	{
		m_drpServerType->SetVisible( showServerType );
		m_drpServerType->SetEnabled( showServerType );
	}
	if ( m_drpGameAccess )
	{
		m_drpGameAccess->SetVisible( showGameAccess );
		m_drpGameAccess->SetEnabled( showGameAccess );
	}
	if ( m_drpNumSlots )
	{
		m_drpNumSlots->SetVisible( showNumSlots );
		m_drpNumSlots->SetEnabled( showNumSlots );

		if ( m_pSettings->GetInt( "members/numSlots" ) <= 4 )
		{
			m_drpNumSlots->SetCurrentSelection( "#rd_ui_4_slots" );
			mm_max_players.SetValue( 4 );
		}
		else if ( m_pSettings->GetInt( "members/numSlots" ) <= 8 )
		{
			m_drpNumSlots->SetCurrentSelection( "#rd_ui_8_slots" );
			mm_max_players.SetValue( 8 );
		}
		else if ( m_pSettings->GetInt( "members/numSlots" ) <= 12 )
		{
			m_drpNumSlots->SetCurrentSelection( "#rd_ui_12_slots" );
			mm_max_players.SetValue( 12 );
		}
		else
		{
			m_drpNumSlots->SetCurrentSelection( "#rd_ui_16_slots" );
			mm_max_players.SetValue( 16 );
		}

		if ( FlyoutMenu* flyout = m_drpNumSlots->GetCurrentFlyout() )
			flyout->CloseMenu( NULL );
	}

	if ( showGameAccess )
	{
		m_pHeaderFooter->SetGradientBarPos( 140, 210 );
	}
	else if ( showNumSlots )
	{
		m_pHeaderFooter->SetGradientBarPos( 140, 185 );
	}
	else
	{
		m_pHeaderFooter->SetGradientBarPos( 140, 160 );
	}

	UpdateMissionImage();

	UpdateFooter();

	if ( m_drpServerType ) //&& m_drpServerType->IsVisible() )
	{
		char const *szDefaultServerToCreate = IsX360() ? "official" : "dedicated";
		szDefaultServerToCreate = "listen"; // force listen servers by default since we don't have dedicated servers for now
		char const *szServerType = m_pSettings->GetString( "options/server", szDefaultServerToCreate );
		char chServerType[64];
		Q_snprintf( chServerType, sizeof( chServerType ), "#L4D360UI_ServerType_%s", szServerType );
		m_drpServerType->SetCurrentSelection( chServerType );
	}
}

void GameSettings::OnThink()
{
	m_bAllowChangeToCustomCampaign = true;

	if ( m_drpGameAccess )
	{
		bool bWasEnabled = m_drpGameAccess->IsEnabled();

		//m_drpGameAccess->SetEnabled( CUIGameData::Get()->SignedInToLive() );

		if ( bWasEnabled && !m_drpGameAccess->IsEnabled() )
		{
			m_drpGameAccess->SetCurrentSelection( "GameNetwork_lan" );
			m_drpGameAccess->CloseDropDown();

			// If we are creating a team game and lost connection to LIVE then
			// we need to bail to main menu (if we have an active session, session will do it)
			if ( !g_pMatchFramework->GetMatchSession() )
			{
				char const *szGameMode = m_pSettings->GetString( "game/mode", "" );
				if ( StringHasPrefix( szGameMode, "team" ) )
				{
					g_pMatchFramework->GetEventsSubscription()->BroadcastEvent( new KeyValues(
						"OnEngineDisconnectReason", "reason", "Lost connection to LIVE" ) );
				}
			}
		}
	}

	BaseClass::OnThink();
}

void GameSettings::OnKeyCodePressed(KeyCode code)
{
	int iUserSlot = GetJoystickForCode( code );
	BaseModUI::CBaseModPanel::GetSingleton().SetLastActiveUserId( iUserSlot );

	switch( GetBaseButtonCode( code ) )
	{
	case KEY_XBUTTON_B:
		{
			CBaseModPanel::GetSingleton().PlayUISound( UISOUND_BACK );

			if ( m_hSubScreen.Get() )
			{
				m_hSubScreen->MarkForDeletion();
			}
			else if ( IsEditingExistingLobby() )
			{
				NavigateBack();
			}
			else
			{
				if ( m_bCloseSessionOnClose )
				{
					g_pMatchFramework->CloseSession();
					m_bCloseSessionOnClose = false;
				}
				m_pSettings = NULL;
				CBaseModPanel::GetSingleton().OpenFrontScreen();
			}
			break;
		}

	case KEY_XBUTTON_X:
		{
			if ( !m_hSubScreen.Get() )
			{
				OnCommand( "StartGame" );
			}
			break;
		}

	default:
		BaseClass::OnKeyCodePressed( code );
		break;
	}
}

void GameSettings::SelectNetworkAccess( char const *szNetworkType, char const *szAccessType )
{
	KeyValues *pSettings = new KeyValues( "update" );
	KeyValues::AutoDelete autodelete( pSettings );
	pSettings->SetString( "update/system/network", szNetworkType );
	pSettings->SetString( "update/system/access", szAccessType );

	rd_last_game_access.SetValue( szAccessType );

	UpdateSessionSettings( pSettings );

	/*
	if ( BaseModHybridButton* button = dynamic_cast< BaseModHybridButton* > ( FindChildByName( "BtnStartLobby" ) ) )
	{
		if ( !Q_stricmp( "public", szAccessType ) && !Q_stricmp( "LIVE", szNetworkType ) && !ui_game_allow_create_public.GetBool() )
		{
			button->SetText( "#L4D360UI_Join_At_Start" );
			button->SetHelpText( "#L4D360UI_GameSettings_Tooltip_Join_At_Start" );
		}
		else
		{
			button->SetText( "#L4D360UI_GameSettings_Create_Lobby" );
			button->SetHelpText( "#L4D360UI_GameSettings_Tooltip_Create_Lobby" );
		}
	}
	*/
}

void GameSettings::DoCustomMatch( char const *szGameState )
{
	KeyValues *pSettings = KeyValues::FromString(
		"update",
		" update { "
			" game { "
				" state = "
			" } "
			" options { "
				" action custommatch "
			" } "
		" } "
		);
	KeyValues::AutoDelete autodelete( pSettings );

	pSettings->SetString( "update/game/state", szGameState );

	const char *pszGameMode = m_pSettings->GetString( "game/mode", "" );
	if ( !GameModeIsSingleChapter( pszGameMode ) )
	{
		pSettings->SetInt( "update/game/chapter", 1 );
	}

	UpdateSessionSettings( pSettings );

	Navigate();
}

//=============================================================================
void GameSettings::OnCommand(const char *command)
{
	if( V_strcmp( command, "cmd_gametype_campaign" ) == 0 )
	{
		KeyValues *kvUpdate = new KeyValues( "update" );
		kvUpdate->SetString( "game/mode", "campaign" );
		KeyValues *kvModification = new KeyValues( "settings" );
		kvModification->AddSubKey( kvUpdate );
		UpdateSessionSettings( KeyValues::AutoDeleteInline( kvModification ) );
		UpdateSelectMissionButton();
		UpdateMissionImage();
	}
	else if( V_strcmp( command, "cmd_gametype_single_mission" ) == 0 )
	{
		KeyValues *kvUpdate = new KeyValues( "update" );
		kvUpdate->SetString( "game/mode", "single_mission" );
		KeyValues *kvModification = new KeyValues( "settings" );
		kvModification->AddSubKey( kvUpdate );
		UpdateSessionSettings( KeyValues::AutoDeleteInline( kvModification ) );
		UpdateSelectMissionButton();
		UpdateMissionImage();
	}
	else if( V_strcmp( command, "cmd_change_mission" ) == 0 )
	{
		ShowMissionSelect();
	}
	else if ( V_strcmp( command, "cmd_change_starting_mission" ) == 0 )
	{
		ShowStartingMissionSelect();
	}
	else if ( V_strcmp( command, "cmd_change_challenge" ) == 0 )
	{
		ShowChallengeSelect();
	}
	else if ( char const *szMissionSelected = StringAfterPrefix( command, "cmd_mission_selected_" ) )
	{
		KeyValues *pSettings = KeyValues::FromString(
			"update",
			" update { "
			" game { "
			" mission asi-jac1-landingbay01"
			" } "
			" } "
			);
		KeyValues::AutoDelete autodelete( pSettings );

		char stripped[MAX_PATH];
		V_StripExtension( szMissionSelected, stripped, MAX_PATH );
		pSettings->SetString( "update/game/mission", stripped );
		pSettings->SetUint64( "update/game/missioninfo/workshop", g_ReactiveDropWorkshop.FindAddonProvidingFile( VarArgs( "resource/overviews/%s.txt", stripped ) ) );

		UpdateSessionSettings( pSettings );
		UpdateMissionImage();
	}
	else if ( char const *szCampaignSelected = StringAfterPrefix( command, "cmd_campaign_selected_" ) )
	{
		KeyValues *pSettings = KeyValues::FromString(
			"update",
			" update { "
			" game { "
			" campaign jacob"
			" mission asi-jac1-landingbay01"
			" } "
			" } "
			);
		KeyValues::AutoDelete autodelete( pSettings );

		char stripped[MAX_PATH];
		V_StripExtension( szCampaignSelected, stripped, MAX_PATH );
		pSettings->SetString( "update/game/campaign", stripped );

		// set the current mission to the first real mission in the campaign
		IASW_Mission_Chooser_Source *pSource = missionchooser ? missionchooser->LocalMissionSource() : NULL;
		if ( pSource )
		{
			KeyValues *pCampaignDetails = pSource->GetCampaignDetails( szCampaignSelected );
			bool bSkippedFirst = false;
			for ( KeyValues *pMission = pCampaignDetails->GetFirstSubKey(); pMission; pMission = pMission->GetNextKey() )
			{
				if ( !Q_stricmp( pMission->GetName(), "MISSION" ) )
				{
					if ( !bSkippedFirst )
					{
						bSkippedFirst = true;
					}
					else
					{
						pSettings->SetString( "update/game/mission", pMission->GetString( "MapName", "asi-jac1-landingbay01" ) );
						pSettings->SetUint64( "update/game/missioninfo/workshop", g_ReactiveDropWorkshop.FindAddonProvidingFile( VarArgs( "resource/overviews/%s.txt", pSettings->GetString( "update/game/mission" ) ) ) );
						break;
					}
				}
			}
		}

		UpdateSessionSettings( pSettings );
		UpdateMissionImage();
	}
	else if ( char const *szChallengeSelected = StringAfterPrefix( command, "cmd_challenge_selected_" ) )
	{
		KeyValues::AutoDelete pSettings( "update" );
		pSettings->SetString( "update/game/challenge", szChallengeSelected );

		rd_last_game_challenge.SetValue( szChallengeSelected );
		UpdateSessionSettings( pSettings );

		if ( m_drpChallenge )
		{
			m_drpChallenge->SetCurrentSelection( ReactiveDropChallenges::DisplayName( szChallengeSelected ) );
		}

		UpdateChallenge( szChallengeSelected );
	}
	else if( V_strcmp( command, "JoinAny" ) == 0 )	// Join game in progress
	{
		DoCustomMatch( "game" );
	}
	else if( V_strcmp( command, "StartGame" ) == 0 )
	{
		char const *szNetwork = m_pSettings->GetString( "system/network", "offline" );
		if ( !Q_stricmp( szNetwork, "offline" ) )
		{
			if ( IsCustomMatchSearchCriteria() )
			{
				DoCustomMatch( "lobby" );
			}
			else
			{
				Navigate();
			}
		}
		else
		{
			// safety check if we aren't on live
			if( !CUIGameData::Get()->SignedInToLive() )
				SelectNetworkAccess( "lan", "public" );

			char const *szAccess = m_pSettings->GetString( "system/access", "public" );

			if ( !Q_stricmp( "LIVE", szNetwork ) &&
				!Q_stricmp( "public", szAccess ) )
			{
				if ( ui_game_allow_create_public.GetBool() )
				{
					OnCommand( "Create" );
					return;
				}

				// Instead of creating a new public lobby we're going to search
				// for any existing public lobbies that match these params!
				// This way people who take this path to a public lobby will still
				// be matched with humans (they might not end up a lobby leader).
				DoCustomMatch( "lobby" );
			}
			else
			{
				Navigate();
			}
		}
	}
	else if ( !Q_strcmp( command, "Create" ) )
	{
		Assert( !IsEditingExistingLobby() );
		g_pMatchFramework->CreateSession( m_pSettings );
	}
	else if ( char const *szNetworkType = StringAfterPrefix( command, "GameNetwork_" ) )
	{
		SelectNetworkAccess( szNetworkType, "public" );
	}
	else if ( char const *szAccessType = StringAfterPrefix( command, "GameAccess_" ) )
	{
		SelectNetworkAccess( "LIVE", szAccessType );
	}
	else if( V_strcmp( command, "Back" ) == 0 )
	{
		// Act as though 360 back button was pressed
		mm_max_players.Revert();
		OnKeyCodePressed( ButtonCodeToJoystickButtonCode( KEY_XBUTTON_B, CBaseModPanel::GetSingleton().GetLastActiveUserId() ) );
	}
	else if ( const char *szDifficultyValue = StringAfterPrefix( command, "#L4D360UI_Difficulty_" ) )
	{
		KeyValues *pSettings = KeyValues::FromString(
			"update",
			" update { "
				" game { "
					" difficulty = "
				" } "
			" } "
			);
		KeyValues::AutoDelete autodelete( pSettings );

		pSettings->SetString( "update/game/difficulty", szDifficultyValue );

		rd_last_game_difficulty.SetValue( szDifficultyValue );
		UpdateSessionSettings( pSettings );

		if( m_drpDifficulty )
		{
			if ( FlyoutMenu* pFlyout = m_drpDifficulty->GetCurrentFlyout() )
				pFlyout->SetListener( this );
		}
	}
	else if ( !Q_strcmp( command, "#L4D360UI_RegularFF" ) )
	{
		KeyValues *pSettings = KeyValues::FromString(
			"update",
			" update { "
			" game { "
			" hardcoreFF = "
			" } "
			" } "
			);
		KeyValues::AutoDelete autodelete( pSettings );

		pSettings->SetInt( "update/game/hardcoreFF", 0 );

		rd_last_game_hardcoreff.SetValue( 0 );
		UpdateSessionSettings( pSettings );

		if( m_drpFriendlyFire )
		{
			if ( m_drpFriendlyFire->IsEnabled() )
				m_bWantHardcoreFF = false;

			if ( FlyoutMenu* pFlyout = m_drpFriendlyFire->GetCurrentFlyout() )
				pFlyout->SetListener( this );
		}
	}
	else if ( !Q_strcmp( command, "#L4D360UI_HardcoreFF" ) )
	{
		KeyValues *pSettings = KeyValues::FromString(
			"update",
			" update { "
			" game { "
			" hardcoreFF = "
			" } "
			" } "
			);
		KeyValues::AutoDelete autodelete( pSettings );

		pSettings->SetInt( "update/game/hardcoreFF", 1 );

		rd_last_game_hardcoreff.SetValue( 1 );
		UpdateSessionSettings( pSettings );

		if( m_drpFriendlyFire )
		{
			if ( m_drpFriendlyFire->IsEnabled() )
				m_bWantHardcoreFF = true;

			if ( FlyoutMenu* pFlyout = m_drpFriendlyFire->GetCurrentFlyout() )
				pFlyout->SetListener( this );
		}
	}
	else if ( !Q_strcmp( command, "#L4D360UI_OnslaughtDisabled" ) )
	{
		KeyValues *pSettings = KeyValues::FromString(
			"update",
			" update { "
			" game { "
			" onslaught = "
			" } "
			" } "
			);
		KeyValues::AutoDelete autodelete( pSettings );

		pSettings->SetInt( "update/game/onslaught", 0 );

		rd_last_game_onslaught.SetValue( 0 );
		UpdateSessionSettings( pSettings );

		if( m_drpOnslaught )
		{
			if ( m_drpOnslaught->IsEnabled() )
				m_bWantOnslaught = false;

			if ( FlyoutMenu* pFlyout = m_drpOnslaught->GetCurrentFlyout() )
				pFlyout->SetListener( this );
		}
	}
	else if ( !Q_strcmp( command, "#L4D360UI_OnslaughtEnabled" ) )
	{
		KeyValues *pSettings = KeyValues::FromString(
			"update",
			" update { "
			" game { "
			" onslaught = "
			" } "
			" } "
			);
		KeyValues::AutoDelete autodelete( pSettings );

		pSettings->SetInt( "update/game/onslaught", 1 );

		rd_last_game_onslaught.SetValue( 1 );
		UpdateSessionSettings( pSettings );

		if( m_drpOnslaught )
		{
			if ( m_drpOnslaught->IsEnabled() )
				m_bWantOnslaught = true;

			if ( FlyoutMenu* pFlyout = m_drpOnslaught->GetCurrentFlyout() )
				pFlyout->SetListener( this );
		}
	}
	else if ( const char *szRoundLimitValue = StringAfterPrefix( command, "#L4D360UI_RoundLimit_" ) )
	{
		KeyValues *pSettings = new KeyValues( "update" );
		KeyValues::AutoDelete autodelete( pSettings );

		pSettings->SetInt( "update/game/maxrounds", atoi( szRoundLimitValue ) );

		UpdateSessionSettings( pSettings );
	}
	else if ( const char *szServerTypeValue = StringAfterPrefix( command, "#L4D360UI_ServerType_" ) )
	{
		KeyValues *pSettings = KeyValues::FromString(
			"update",
			" update { "
				" options { "
					" server x "
				" } "
			" } "
			);
		KeyValues::AutoDelete autodelete( pSettings );

		char chBuffer[64];
		Q_snprintf( chBuffer, sizeof( chBuffer ), "%s", szServerTypeValue );
		for ( char *pszUpper = chBuffer; *pszUpper; ++ pszUpper )
		{
			if ( isupper( *pszUpper ) )
				*pszUpper = tolower( *pszUpper );
		}

		pSettings->SetString( "update/options/server", chBuffer );

		UpdateSessionSettings( pSettings );

		if ( m_drpServerType )
		{
			if ( FlyoutMenu *pFlyout = m_drpServerType->GetCurrentFlyout() )
				pFlyout->SetListener( this );
		}
	}
	else if ( !Q_strcmp( command, "#rd_ui_4_slots" ) )
	{
		mm_max_players.SetValue( 4 );
		rd_last_game_maxplayers.SetValue( 4 );
	}
	else if ( !Q_strcmp( command, "#rd_ui_8_slots" ) )
	{
		mm_max_players.SetValue( 8 );
		rd_last_game_maxplayers.SetValue( 8 );
	}
	else if ( !Q_strcmp( command, "#rd_ui_12_slots" ) )
	{
		mm_max_players.SetValue( 12 );
		rd_last_game_maxplayers.SetValue( 12 );
	}
	else if ( !Q_strcmp( command, "#rd_ui_16_slots" ) )
	{
		mm_max_players.SetValue( 16 );
		rd_last_game_maxplayers.SetValue( 16 );
	}
	else
	{
		BaseClass::OnCommand( command );
	}
}

void GameSettings::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	char const *szNetwork = m_pSettings->GetString( "system/network", "LIVE" );
	char const *szAccess = m_pSettings->GetString( "system/access", "public" );


	KeyValues *pConditions = NULL;
	if ( !Q_stricmp( "offline", szNetwork ) )
		pConditions = new KeyValues( "", "?condition?singleplayer", 0 );

	char szPath[MAX_PATH];
	V_snprintf( szPath, sizeof( szPath ), "Resource/UI/BaseModUI/GameSettings.res" );

	LoadControlSettings( szPath, NULL, NULL, pConditions );

	if ( pConditions )
		pConditions->deleteThis();

	// required for new style
	SetPaintBackgroundEnabled( true );
	SetupAsDialogStyle();

	m_drpDifficulty = dynamic_cast< DropDownMenu* >( FindChildByName( "DrpDifficulty" ) );
	m_drpGameType = dynamic_cast< DropDownMenu* >( FindChildByName( "DrpGameType" ) );
	m_drpFriendlyFire = dynamic_cast< DropDownMenu* >( FindChildByName( "DrpFriendlyFire" ) );
	m_drpOnslaught = dynamic_cast< DropDownMenu* >( FindChildByName( "DrpOnslaught" ) );
	m_drpChallenge = dynamic_cast< DropDownMenu* >( FindChildByName( "DrpChallenge" ) );

	m_drpGameAccess = dynamic_cast< DropDownMenu* >( FindChildByName( "DrpGameAccess" ) );
	if ( m_drpGameAccess )
	{		
		if( !CUIGameData::Get()->SignedInToLive() )
		{
			m_drpGameAccess->SetCurrentSelection( "GameNetwork_lan" );
		}
		else
		{
			if ( !Q_stricmp( "lan", szNetwork ) )
				m_drpGameAccess->SetCurrentSelection( "GameNetwork_lan" );
			else if ( !Q_stricmp( "LIVE", szNetwork ) )
				m_drpGameAccess->SetCurrentSelection( CFmtStr( "GameAccess_%s", szAccess ) );
		}
	}
	m_drpServerType = dynamic_cast< DropDownMenu* >( FindChildByName( "DrpServerType" ) );
	m_drpStartingMission = dynamic_cast< DropDownMenu* >( FindChildByName( "DrpStartingMission" ) );

	m_drpNumSlots = dynamic_cast< DropDownMenu* >( FindChildByName( "DrpNumSlots" ) );

	// can now be invoked as controls exist
	Activate();
}

bool GameSettings::IsEditingExistingLobby()
{
	vgui::Panel * backPanel = GetNavBack();
	CBaseModFrame *pLobby = CBaseModPanel::GetSingleton().GetWindow( WT_GAMELOBBY );
	if ( pLobby && backPanel == pLobby )
	{
		return true;
	}

	return false;
}

bool GameSettings::IsCustomMatchSearchCriteria()
{
	if ( IsEditingExistingLobby() )
		return false;

	if ( Q_stricmp( "custommatch", m_pSettings->GetString( "options/action", "" ) ) )
		return false;

	return true;
}

void GameSettings::OnClose()
{
	BaseClass::OnClose();

	if( m_drpDifficulty )
		m_drpDifficulty->CloseDropDown();

	if( m_drpGameType )
		m_drpGameType->CloseDropDown();

	if( m_drpFriendlyFire )
		m_drpFriendlyFire->CloseDropDown();

	if( m_drpOnslaught )
		m_drpOnslaught->CloseDropDown();

	if( m_drpChallenge )
		m_drpChallenge->CloseDropDown();

	m_pSettings = NULL;	// NULL out settings in case we get some calls
	// after we are closed
	if ( m_bCloseSessionOnClose )
	{
		g_pMatchFramework->CloseSession();
	}
}

void GameSettings::Navigate()
{
	CBaseModPanel::GetSingleton().PlayUISound( UISOUND_ACCEPT );

	char const *szNetwork = m_pSettings->GetString( "system/network", "offline" );
	char const *szAccess = m_pSettings->GetString( "system/access", "public" );

	if ( IsEditingExistingLobby() )
	{
		NavigateBack();//if we were opened from the game lobby nav back to the game lobby.
	}
	else
	{
		if ( !Q_stricmp( "LIVE", szNetwork ) &&
			 !Q_stricmp( "public", szAccess ) )
		{
			g_pMatchFramework->MatchSession( m_pSettings );
			return;
		}

		if ( !Q_stricmp( "lan", szNetwork ) || (
			 !Q_stricmp( "LIVE", szNetwork ) &&
				( !Q_stricmp( "friends", szAccess ) ||
				  !Q_stricmp( "private", szAccess ) ) ) )
		{
			g_pMatchFramework->CreateSession( m_pSettings );
			return;
		}

		if ( !Q_stricmp( "offline", szNetwork ) )
		{
			IMatchSession *pIMatchSession = g_pMatchFramework->GetMatchSession();
			Assert( pIMatchSession );
			if ( pIMatchSession )
			{
				m_bCloseSessionOnClose = false;
				pIMatchSession->Command( KeyValues::AutoDeleteInline( new KeyValues( "Start" ) ) );
				return;
			}
		}

		Assert( !"unreachable" );
		NavigateBack();
	}
}

void GameSettings::OnNotifyChildFocus( vgui::Panel* child )
{
	if ( !child )
	{
		return;
	}

// 	BaseModHybridButton* button = dynamic_cast< BaseModHybridButton* >( child );
// 	if ( button )
// 	{
// 		KeyValues* command = button->GetCommand();
// 		if ( command )
// 		{
// 			const char* commandValue = command->GetString( "command", NULL );
// 			if ( char const *szChapterName = StringAfterPrefix( commandValue, "#L4D360UI_Chapter_" ) )
// 			{
// 				UpdateMissionImage( atoi( szChapterName ) );
// 			}
// 			else if ( const char *szMissionIdx = StringAfterPrefix( commandValue, "cmd_campaign_" ) )
// 			{
// 				if ( !*szMissionIdx )
// 				{
// 					if ( vgui::ImagePanel* imgLevelImage = dynamic_cast< vgui::ImagePanel* >( FindChildByName( "ImgLevelImage" ) ) )
// 					{
// 						imgLevelImage->SetImage( "maps/any" );
// 					}
// 				}
// 				else
// 				{
// 					UpdateMissionImage( 0, szMissionIdx );
// 				}
// 			}
// 			else if ( V_strcmp( commandValue, "cmd_addoncampaign" ) == 0 )
// 			{
// 				if ( vgui::ImagePanel* imgLevelImage = dynamic_cast< vgui::ImagePanel* >( FindChildByName( "ImgLevelImage" ) ) )
// 				{
// 					imgLevelImage->SetImage( "maps/unknown" );
// 				}
// 			}
// 			else if ( char const *pszCharacter = StringAfterPrefix( commandValue, "character_" ) )
// 			{
// 				const char *pszPortrait = s_characterPortraits->GetTokenI( pszCharacter );
// 				if ( pszPortrait )
// 				{
// 					vgui::ImagePanel *pPanel = dynamic_cast< vgui::ImagePanel* >( child->FindSiblingByName( "HeroPortrait" ) );
// 					if ( pPanel )
// 					{
// 						pPanel->SetVisible( true );
// 						pPanel->SetImage( pszPortrait );
// 					}
// 				}
// 			}
// 
// 		}
// 	}
}

void GameSettings::UpdateMissionImage()
{	
	vgui::ImagePanel* imgLevelImage = dynamic_cast< vgui::ImagePanel* >( FindChildByName( "ImgLevelImage" ) );
	if( !imgLevelImage )
		return;

	vgui::Label *pMissionLabel = dynamic_cast< vgui::Label* >( FindChildByName( "MissionLabel" ) );


	DropDownMenu *menu = dynamic_cast< DropDownMenu* >( FindChildByName( "DrpSelectMission", true ) );

	IASW_Mission_Chooser_Source *pSource = missionchooser ? missionchooser->LocalMissionSource() : NULL;
	if ( !pSource || !m_pSettings || !menu )
		return;

	const char *szGameType = m_pSettings->GetString( "game/mode", "campaign" );
	if ( !Q_stricmp( szGameType, "campaign" ) )
	{
		const char *szCampaign = m_pSettings->GetString( "game/campaign", NULL );
		if ( szCampaign )
		{
			KeyValues *pCampaignKeys = pSource->GetCampaignDetails( szCampaign );
			if ( pCampaignKeys )
			{
				//imgLevelImage->SetImage( pCampaignKeys->GetString( "ChooseCampaignTexture" ) );
				pMissionLabel->SetText( pCampaignKeys->GetString( "CampaignName" ) );
				menu->SetCurrentSelection( pCampaignKeys->GetString( "CampaignName" ) );
			}
		}
		if ( m_drpStartingMission )
		{
			m_drpStartingMission->SetVisible( true );

			const char *szMission = m_pSettings->GetString( "game/mission", NULL );
			if ( szMission )
			{
				KeyValues *pMissionKeys = pSource->GetMissionDetails( szMission );
				if ( pMissionKeys )
				{
					m_drpStartingMission->SetCurrentSelection( pMissionKeys->GetString( "missiontitle" ) );
					imgLevelImage->SetImage( pMissionKeys->GetString( "image" ) );
				}
			}
		}
	}
	else if ( !Q_stricmp( szGameType, "single_mission" ) )
	{
		const char *szMission = m_pSettings->GetString( "game/mission", NULL );
		if ( szMission )
		{
			KeyValues *pMissionKeys = pSource->GetMissionDetails( szMission );
			if ( pMissionKeys )
			{
				// TODO: Handle missions without an image
				imgLevelImage->SetImage( pMissionKeys->GetString( "image" ) );
				pMissionLabel->SetText( pMissionKeys->GetString( "missiontitle" ) );
				menu->SetCurrentSelection( pMissionKeys->GetString( "missiontitle" ) );
			}
		}
		if ( m_drpStartingMission )
		{
			m_drpStartingMission->SetVisible( false );
		}
	}
}

void GameSettings::UpdateFooter()
{
	CBaseModFooterPanel *footer = BaseModUI::CBaseModPanel::GetSingleton().GetFooterPanel();
	if ( footer )
	{
		bool bDoHints = false;
#if defined( _X360 )
		// only do hints in english, all other languages have lengthy hints, can't fix in time
		bDoHints = ( XBX_IsLocalized() == false );
#endif
		footer->SetButtons( FB_ABUTTON | FB_BBUTTON, FF_AB_ONLY, bDoHints );
		footer->SetButtonText( FB_ABUTTON, "#L4D360UI_Select" );

		if ( m_bBackButtonMeansDone )
		{
			// No special button needed in these menus as backing out is considered "Done"
			// There's effectively no way to "Cancel"
			footer->SetButtonText( FB_BBUTTON, "#L4D360UI_Done" );
		}
		else
		{
			footer->SetButtonText( FB_BBUTTON, "#L4D360UI_Cancel" );
		}
	}
}

void GameSettings::OnFlyoutMenuClose( vgui::Panel* flyTo )
{
	UpdateFooter();
	UpdateMissionImage();
	UpdateSelectMissionButton();
}

void GameSettings::OnFlyoutMenuCancelled()
{

}

void GameSettings::UpdateSelectMissionButton()
{
	DropDownMenu *menu = dynamic_cast< DropDownMenu* >( FindChildByName( "DrpSelectMission", true ) );
	if ( !menu )
		return;

	BaseModHybridButton *button = menu->GetButton(); //dynamic_cast< BaseModHybridButton* >( FindChildByName( "BtnSelectMission", true ) );
	if ( m_pSettings && button )
	{
		const char *szGameType = m_pSettings->GetString( "game/mode", "campaign" );
		if ( !Q_stricmp( szGameType, "campaign" ) )
		{
			button->SetText( "#ASUI_Select_Campaign" );
			button->SetHelpText( "#ASUI_Select_Campaign_tt" );
		}
		else if ( !Q_stricmp( szGameType, "single_mission" ) )
		{
			button->SetText( "#ASUI_Select_Mission" );
			button->SetHelpText( "#ASUI_Select_Mission_tt" );
		}
	}
	/*
	BaseModHybridButton *button = dynamic_cast< BaseModHybridButton* >( FindChildByName( "BtnSelectMission" ) );
	if ( m_pSettings && button )
	{
		const char *szGameType = m_pSettings->GetString( "game/mode", "campaign" );
		if ( !Q_stricmp( szGameType, "campaign" ) )
		{
			button->SetText( "#ASUI_Select_Campaign" );
			button->SetHelpText( "#ASUI_Select_Campaign_tt" );
		}
		else if ( !Q_stricmp( szGameType, "single_mission" ) )
		{
			button->SetText( "#ASUI_Select_Mission" );
			button->SetHelpText( "#ASUI_Select_Mission_tt" );
		}
	}
	*/
}

void GameSettings::ShowMissionSelect()
{
	if ( m_hSubScreen.Get() )
	{
		m_hSubScreen->MarkForDeletion();
	}

	Activate();

	if ( m_pSettings )
	{
		if ( !V_stricmp( m_pSettings->GetString( "system/network" ), "offline" ) )
		{
			engine->ClientCmd_Unrestricted( "asw_mission_chooser singleplayer\n" );
		}
		else
		{
			engine->ClientCmd_Unrestricted( "asw_mission_chooser createserver\n" );
		}
	}
}

void GameSettings::ShowStartingMissionSelect()
{
	if ( m_hSubScreen.Get() )
	{
		m_hSubScreen->MarkForDeletion();
	}

	Activate();

	if ( m_pSettings )
	{
		if ( !V_stricmp( m_pSettings->GetString( "system/network" ), "offline" ) )
		{
			engine->ClientCmd_Unrestricted( VarArgs( "asw_mission_chooser singleplayer campaign %s\n", m_pSettings->GetString( "game/campaign" ) ) );
		}
		else
		{
			engine->ClientCmd_Unrestricted( VarArgs( "asw_mission_chooser createserver campaign %s\n", m_pSettings->GetString( "game/campaign" ) ) );
		}
	}
}

void GameSettings::ShowChallengeSelect()
{
	if ( m_hSubScreen.Get() )
	{
		m_hSubScreen->MarkForDeletion();
	}

	if ( m_pSettings )
	{
		ReactiveDropChallengeSelection *pPanel = new ReactiveDropChallengeSelection( this, "ReactiveDropChallengeSelection" );
		pPanel->SetSelectedChallenge( m_pSettings->GetString( "game/challenge", "0" ) );
		pPanel->MoveToFront();
		pPanel->m_gplChallenges->RequestFocus();

		m_hSubScreen = pPanel;
	}	
}

void GameSettings::UpdateChallenge( const char *szChallengeName )
{
	bool bForceOnslaughtOn = false;
	bool bForceOnslaughtOff = false;
	bool bForceHardcoreFFOn = false;
	bool bForceHardcoreFFOff = false;

	if ( const RD_Challenge_t *pChallenge = ReactiveDropChallenges::GetSummary( szChallengeName ) )
	{
		bForceOnslaughtOn = pChallenge->ForceOnslaught && pChallenge->IsOnslaught;
		bForceOnslaughtOff = pChallenge->ForceOnslaught && !pChallenge->IsOnslaught;
		bForceHardcoreFFOn = pChallenge->ForceHardcore && pChallenge->IsHardcore;
		bForceHardcoreFFOff = pChallenge->ForceHardcore && !pChallenge->IsHardcore;

		KeyValues::AutoDelete pUpdate( "update" );
		pUpdate->SetUint64( "update/game/challengeinfo/workshop", pChallenge->WorkshopID );
		pUpdate->SetString( "update/game/challengeinfo/displaytitle", pChallenge->Title );
		UpdateSessionSettings( pUpdate );
	}

	if ( m_drpOnslaught )
	{
		if ( bForceOnslaughtOn )
		{
			m_drpOnslaught->SetEnabled( false );
			m_drpOnslaught->SetCurrentSelection( "#L4D360UI_OnslaughtEnabled" );
		}
		else if ( bForceOnslaughtOff )
		{
			m_drpOnslaught->SetEnabled( false );
			m_drpOnslaught->SetCurrentSelection( "#L4D360UI_OnslaughtDisabled" );
		}
		else if ( !m_drpOnslaught->IsEnabled() )
		{
			m_drpOnslaught->SetCurrentSelection( m_bWantOnslaught ? "#L4D360UI_OnslaughtEnabled" : "#L4D360UI_OnslaughtDisabled" );
			m_drpOnslaught->SetEnabled( true );
		}
	}
	if ( m_drpFriendlyFire )
	{
		if ( bForceHardcoreFFOn )
		{
			m_drpFriendlyFire->SetEnabled( false );
			m_drpFriendlyFire->SetCurrentSelection( "#L4D360UI_HardcoreFF" );
		}
		else if ( bForceHardcoreFFOff )
		{
			m_drpFriendlyFire->SetEnabled( false );
			m_drpFriendlyFire->SetCurrentSelection( "#L4D360UI_RegularFF" );
		}
		else if ( !m_drpFriendlyFire->IsEnabled() )
		{
			m_drpFriendlyFire->SetCurrentSelection( m_bWantHardcoreFF ? "#L4D360UI_HardcoreFF" : "#L4D360UI_RegularFF" );
			m_drpFriendlyFire->SetEnabled( true );
		}
	}
}

static void ShowGameSettings()
{
	CBaseModFrame* mainMenu = CBaseModPanel::GetSingleton().GetWindow( WT_MAINMENU );
	CBaseModPanel::GetSingleton().OpenWindow( WT_GAMESETTINGS, mainMenu );
}

ConCommand showGameSettings( "showGameSettings", ShowGameSettings );
