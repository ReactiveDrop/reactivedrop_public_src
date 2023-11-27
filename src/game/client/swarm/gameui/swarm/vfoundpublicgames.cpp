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
#include "nb_button.h"

#include "rd_missions_shared.h"
#include "asw_util_shared.h"
#include "rd_vgui_settings.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;
using namespace BaseModUI;

//=============================================================================
extern ConVar rd_lobby_ping_low;
extern ConVar rd_lobby_ping_high;

ConVar rd_lobby_filter_distance( "rd_lobby_filter_distance", "3", FCVAR_ARCHIVE, "maximum distance for player-hosted lobbies. does not affect lobbies on dedicated servers. 0=only own region, 1=nearby regions, 2=about half of the world, 3=return all lobbies", true, k_ELobbyDistanceFilterClose, true, k_ELobbyDistanceFilterWorldwide );

//=============================================================================
FoundPublicGames::FoundPublicGames( Panel *parent, const char *panelName ) :
	BaseClass( parent, panelName )
{
	m_numCurrentPlayers = 0;

	// increase footer tall by 10 to fit Advanced button into it
	CNB_Header_Footer *pHeaderFooter = dynamic_cast< CNB_Header_Footer* >( FindChildByName( "HeaderFooter" ) );
	pHeaderFooter->SetGradientBarPos( 80, 325 );

	m_pOptionNameMode = new CRD_VGUI_Option( this, "OptionNameMode", "#rd_lobby_option_name_mode_title" );
	m_pOptionNameMode->AddOption( 0, "#rd_lobby_option_name_mode_mission", "#rd_lobby_option_name_mode_mission_hint" );
	m_pOptionNameMode->AddOption( 1, "#rd_lobby_option_name_mode_lobby", "#rd_lobby_option_name_mode_lobby_hint" );
	m_pOptionNameMode->AddOption( 2, "#rd_lobby_option_name_mode_both", "#rd_lobby_option_name_mode_both_hint" );
	m_pOptionNameMode->LinkToConVar( "rd_lobby_name_mode", false );
	m_pOptionNameMode->AddActionSignalTarget( this );
	m_pOptionDifficultyMin = new CRD_VGUI_Option( this, "OptionDifficultyMin", "#rd_lobby_option_difficulty_min_title" );
	m_pOptionDifficultyMin->AddOption( 1, "#asw_difficulty_easy", "#rd_lobby_option_difficulty_min_hint" );
	m_pOptionDifficultyMin->AddOption( 2, "#asw_difficulty_normal", "#rd_lobby_option_difficulty_min_hint" );
	m_pOptionDifficultyMin->AddOption( 3, "#asw_difficulty_hard", "#rd_lobby_option_difficulty_min_hint" );
	m_pOptionDifficultyMin->AddOption( 4, "#asw_difficulty_insane", "#rd_lobby_option_difficulty_min_hint" );
	m_pOptionDifficultyMin->AddOption( 5, "#asw_difficulty_imba", "#rd_lobby_option_difficulty_min_hint" );
	m_pOptionDifficultyMin->LinkToConVar( "rd_lobby_filter_difficulty_min", false );
	m_pOptionDifficultyMin->AddActionSignalTarget( this );
	m_pOptionDifficultyMax = new CRD_VGUI_Option( this, "OptionDifficultyMax", "#rd_lobby_option_difficulty_max_title" );
	m_pOptionDifficultyMax->AddOption( 1, "#asw_difficulty_easy", "#rd_lobby_option_difficulty_max_hint" );
	m_pOptionDifficultyMax->AddOption( 2, "#asw_difficulty_normal", "#rd_lobby_option_difficulty_max_hint" );
	m_pOptionDifficultyMax->AddOption( 3, "#asw_difficulty_hard", "#rd_lobby_option_difficulty_max_hint" );
	m_pOptionDifficultyMax->AddOption( 4, "#asw_difficulty_insane", "#rd_lobby_option_difficulty_max_hint" );
	m_pOptionDifficultyMax->AddOption( 5, "#asw_difficulty_imba", "#rd_lobby_option_difficulty_max_hint" );
	m_pOptionDifficultyMax->LinkToConVar( "rd_lobby_filter_difficulty_max", false );
	m_pOptionDifficultyMax->AddActionSignalTarget( this );
	m_pOptionOnslaught = new CRD_VGUI_Option( this, "OptionOnslaught", "#rd_lobby_option_onslaught_title" );
	m_pOptionOnslaught->AddOption( -1, "#rd_lobby_option_onslaught_any", "#rd_lobby_option_onslaught_any_hint" );
	m_pOptionOnslaught->AddOption( 0, "#rd_lobby_option_onslaught_forbidden", "#rd_lobby_option_onslaught_forbidden_hint" );
	m_pOptionOnslaught->AddOption( 1, "#rd_lobby_option_onslaught_required", "#rd_lobby_option_onslaught_required_hint" );
	m_pOptionOnslaught->LinkToConVar( "rd_lobby_filter_onslaught", false );
	m_pOptionOnslaught->AddActionSignalTarget( this );
	m_pOptionHardcoreFF = new CRD_VGUI_Option( this, "OptionHardcoreFF", "#rd_lobby_option_hardcoreff_title" );
	m_pOptionHardcoreFF->AddOption( -1, "#rd_lobby_option_hardcoreff_any", "#rd_lobby_option_hardcoreff_any_hint" );
	m_pOptionHardcoreFF->AddOption( 0, "#rd_lobby_option_hardcoreff_forbidden", "#rd_lobby_option_hardcoreff_forbidden_hint" );
	m_pOptionHardcoreFF->AddOption( 1, "#rd_lobby_option_hardcoreff_required", "#rd_lobby_option_hardcoreff_required_hint" );
	m_pOptionHardcoreFF->LinkToConVar( "rd_lobby_filter_hardcoreff", false );
	m_pOptionHardcoreFF->AddActionSignalTarget( this );
	m_pOptionDedicated = new CRD_VGUI_Option( this, "OptionDedicated", "#rd_lobby_option_dedicated_title" );
	m_pOptionDedicated->AddOption( -1, "#rd_lobby_option_dedicated_any", "#rd_lobby_option_dedicated_any_hint" );
	m_pOptionDedicated->AddOption( 0, "#rd_lobby_option_dedicated_forbidden", "#rd_lobby_option_dedicated_forbidden_hint" );
	m_pOptionDedicated->AddOption( 1, "#rd_lobby_option_dedicated_required", "#rd_lobby_option_dedicated_required_hint" );
	m_pOptionDedicated->LinkToConVar( "rd_lobby_filter_dedicated", false );
	m_pOptionDedicated->AddActionSignalTarget( this );
	m_pOptionInstalled = new CRD_VGUI_Option( this, "OptionInstalled", "#rd_lobby_option_installed_title" );
	m_pOptionInstalled->AddOption( -1, "#rd_lobby_option_installed_any", "#rd_lobby_option_installed_any_hint" );
	m_pOptionInstalled->AddOption( 0, "#rd_lobby_option_installed_forbidden", "#rd_lobby_option_installed_forbidden_hint" );
	m_pOptionInstalled->AddOption( 1, "#rd_lobby_option_installed_required", "#rd_lobby_option_installed_required_hint" );
	m_pOptionInstalled->LinkToConVar( "rd_lobby_filter_installed", false );
	m_pOptionInstalled->AddActionSignalTarget( this );
	m_pOptionChallenge = new CRD_VGUI_Option( this, "OptionChallenge", "#rd_lobby_option_challenge_title" );
	m_pOptionChallenge->AddOption( -1, "#rd_lobby_option_challenge_any", "#rd_lobby_option_challenge_any_hint" );
	m_pOptionChallenge->AddOption( 0, "#rd_lobby_option_challenge_forbidden", "#rd_lobby_option_challenge_forbidden_hint" );
	m_pOptionChallenge->AddOption( 1, "#rd_lobby_option_challenge_required", "#rd_lobby_option_challenge_required_hint" );
	m_pOptionChallenge->LinkToConVar( "rd_lobby_filter_challenge", false );
	m_pOptionChallenge->AddActionSignalTarget( this );
	m_pOptionDistance = new CRD_VGUI_Option( this, "OptionDistance", "#rd_lobby_option_distance_title" );
	m_pOptionDistance->AddOption( k_ELobbyDistanceFilterClose, "#L4D360UI_FoundPublicGames_Filter_Distance_Close", "#rd_lobby_option_distance_close_hint" );
	m_pOptionDistance->AddOption( k_ELobbyDistanceFilterDefault, "#L4D360UI_FoundPublicGames_Filter_Distance_Default", "#rd_lobby_option_distance_default_hint" );
	m_pOptionDistance->AddOption( k_ELobbyDistanceFilterFar, "#L4D360UI_FoundPublicGames_Filter_Distance_Far", "#rd_lobby_option_distance_far_hint" );
	m_pOptionDistance->AddOption( k_ELobbyDistanceFilterWorldwide, "#L4D360UI_FoundPublicGames_Filter_Distance_Worldwide", "#rd_lobby_option_distance_worldwide_hint" );
	m_pOptionDistance->LinkToConVar( "rd_lobby_filter_distance", false );
	m_pOptionDistance->AddActionSignalTarget( this );
	m_pOptionAlwaysFriends = new CRD_VGUI_Option( this, "OptionAlwaysFriends", "#rd_lobby_option_always_friends_title", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pOptionAlwaysFriends->SetDefaultHint( "#rd_lobby_option_always_friends_hint" );
	m_pOptionAlwaysFriends->LinkToConVar( "rd_lobby_filter_always_friends", false );
	m_pOptionAlwaysFriends->AddActionSignalTarget( this );
	m_pBtnFilters = new CNB_Button( this, "BtnFilters", "#rd_lobby_options_button_active", this, "ToggleFilters" );
	m_pBtnFilters->SetControllerButton( KEY_XBUTTON_Y );
}

FoundPublicGames::~FoundPublicGames()
{
	CRD_VGUI_Option::WriteConfig( false );
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

void FoundPublicGames::UpdateDifficultyMinMaxLock()
{
	// We don't want players to set minimum difficulty above maximum difficulty because that rules out every lobby.
	int iMin = m_pOptionDifficultyMin->GetCurrentOption();
	int iMax = m_pOptionDifficultyMax->GetCurrentOption();

	for ( int i = 1; i <= 5; i++ )
	{
		m_pOptionDifficultyMin->SetOptionFlags( i, iMax >= i ? 0 : CRD_VGUI_Option::FLAG_DISABLED );
		m_pOptionDifficultyMax->SetOptionFlags( i, iMin <= i ? 0 : CRD_VGUI_Option::FLAG_DISABLED );
	}
}

void FoundPublicGames::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

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

	m_pTitle->SetText( CFmtStr( "#L4D360UI_FoundPublicGames_Title_%s", gameMode ) );
	m_pSubTitle->SetText( finalString );
}

void FoundPublicGames::AddServersToList()
{
	AddFriendGamesToList();
	AddDedicatedServersToList();
	AddPublicGamesToList();
}

void FoundPublicGames::AddPublicGamesToList()
{
	bool bUsingDistanceFilter = rd_lobby_filter_distance.GetInt() != k_ELobbyDistanceFilterWorldwide;
	g_ReactiveDropServerList.m_PublicLobbies.WantUpdatedLobbyList();
	if ( bUsingDistanceFilter )
	{
		g_ReactiveDropServerList.m_PublicDistanceLobbies.m_DistanceFilter = ELobbyDistanceFilter( rd_lobby_filter_distance.GetInt() );
		g_ReactiveDropServerList.m_PublicDistanceLobbies.WantUpdatedLobbyList();

		FOR_EACH_VEC( g_ReactiveDropServerList.m_PublicDistanceLobbies.m_MatchingLobbies, i )
		{
			FoundGameListItem::Info info;
			if ( info.SetFromLobby( g_ReactiveDropServerList.m_PublicDistanceLobbies.m_MatchingLobbies[i] ) )
				AddGameFromDetails( info );
		}
	}

	FOR_EACH_VEC( g_ReactiveDropServerList.m_PublicLobbies.m_MatchingLobbies, i )
	{
		FoundGameListItem::Info info;
		if ( info.SetFromLobby( g_ReactiveDropServerList.m_PublicLobbies.m_MatchingLobbies[i] ) )
			AddGameFromDetails( info, bUsingDistanceFilter );
	}
}

void FoundPublicGames::AddDedicatedServersToList()
{
	if ( !g_ReactiveDropServerList.m_InternetServers.Count() )
	{
		// While we're doing our first unfiltered query, show the filtered server list we retrieved on the main menu.
		FOR_EACH_VEC( g_ReactiveDropServerList.m_PublicServers, i )
		{
			FoundGameListItem::Info info;
			if ( info.SetFromServer( g_ReactiveDropServerList.m_PublicServers, i ) )
				AddGameFromDetails( info );
		}
	}

	g_ReactiveDropServerList.m_InternetServers.WantUpdatedServerList();

	FOR_EACH_VEC( g_ReactiveDropServerList.m_InternetServers, i )
	{
		FoundGameListItem::Info info;
		if ( info.SetFromServer( g_ReactiveDropServerList.m_InternetServers, i ) )
			AddGameFromDetails( info );
	}

	g_ReactiveDropServerList.m_LANServers.WantUpdatedServerList();

	FOR_EACH_VEC( g_ReactiveDropServerList.m_LANServers, i )
	{
		FoundGameListItem::Info info;
		if ( info.SetFromServer( g_ReactiveDropServerList.m_LANServers, i, FoundGameListItem::TYPE_LANSERVER ) )
			AddGameFromDetails( info );
	}

	g_ReactiveDropServerList.m_FavoriteServers.WantUpdatedServerList();

	FOR_EACH_VEC( g_ReactiveDropServerList.m_FavoriteServers, i )
	{
		FoundGameListItem::Info info;
		if ( info.SetFromServer( g_ReactiveDropServerList.m_FavoriteServers, i, FoundGameListItem::TYPE_FAVORITESERVER ) )
			AddGameFromDetails( info );
	}
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
	else if ( !V_strcmp( command, "ToggleFilters" ) )
	{
		m_bFiltersVisible = !m_bFiltersVisible;
		if ( m_bFiltersVisible )
		{
			m_pBtnFilters->SetText( "#rd_lobby_options_button_active" );

			OnItemSelected( "" );

			NavigateToChild( m_pOptionNameMode );
		}
		else
		{
			int nFiltersActive = 0;
			if ( m_pOptionDifficultyMin->GetCurrentOption() != 1 || m_pOptionDifficultyMax->GetCurrentOption() != 5 )
				nFiltersActive++;
			if ( m_pOptionOnslaught->GetCurrentOption() != -1 )
				nFiltersActive++;
			if ( m_pOptionHardcoreFF->GetCurrentOption() != -1 )
				nFiltersActive++;
			if ( m_pOptionDedicated->GetCurrentOption() != -1 )
				nFiltersActive++;
			if ( m_pOptionInstalled->GetCurrentOption() != -1 )
				nFiltersActive++;
			if ( m_pOptionChallenge->GetCurrentOption() != -1 )
				nFiltersActive++;
			if ( m_pOptionDistance->GetCurrentOption() != k_ELobbyDistanceFilterWorldwide )
				nFiltersActive++;

			wchar_t wszText[256];
			g_pVGuiLocalize->ConstructString( wszText, sizeof( wszText ), g_pVGuiLocalize->Find( "#rd_lobby_options_button_inactive" ), 1, UTIL_RD_CommaNumber( nFiltersActive ) );

			m_pBtnFilters->SetText( wszText, true );

			UpdateGameDetails();

			NavigateToChild( m_GplGames );
		}
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

#if !defined( _X360 ) && !defined( NO_STEAM )
	if ( ISteamUserStats *pSteamUserStats = SteamUserStats() )
	{
		SteamAPICall_t hSteamAPICall = pSteamUserStats->GetNumberOfCurrentPlayers();
		m_callbackNumberOfCurrentPlayers.Set( hSteamAPICall, this, &FoundPublicGames::Steam_OnNumberOfCurrentPlayers );
	}
#endif

	m_bFiltersVisible = true;
	OnCommand( "ToggleFilters" );
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
		OnCommand( "CreateGame" );
		break;

	case KEY_XBUTTON_Y:
		OnCommand( "ToggleFilters" );
		break;

	default:
		BaseClass::OnKeyCodePressed( code );
		break;
	}
}

void FoundPublicGames::OnItemSelected( const char *panelName )
{
	SetControlVisible( "PnlFiltersBackground", m_bFiltersVisible );
	m_pOptionNameMode->SetVisible( m_bFiltersVisible );
	m_pOptionDifficultyMin->SetVisible( m_bFiltersVisible );
	m_pOptionDifficultyMax->SetVisible( m_bFiltersVisible );
	m_pOptionOnslaught->SetVisible( m_bFiltersVisible );
	m_pOptionHardcoreFF->SetVisible( m_bFiltersVisible );
	m_pOptionDedicated->SetVisible( m_bFiltersVisible );
	m_pOptionInstalled->SetVisible( m_bFiltersVisible );
	m_pOptionChallenge->SetVisible( m_bFiltersVisible );
	m_pOptionAlwaysFriends->SetVisible( m_bFiltersVisible );
	m_pOptionDistance->SetVisible( m_bFiltersVisible );
	UpdateDifficultyMinMaxLock();

	m_GplGames->SetEnabled( !m_bFiltersVisible );
	SetDetailsPanelVisible( !m_bFiltersVisible );

	if ( m_bFiltersVisible )
	{
		if ( m_pPreviousSelectedItem )
		{
			for ( int i = 0; i < m_GplGames->GetPanelItemCount(); i++ )
			{
				FoundGameListItem *pItem = dynamic_cast< FoundGameListItem * >( m_GplGames->GetPanelItem( i ) );

				if ( pItem )
				{
					pItem->SetSelected( false );
				}
			}
			m_pPreviousSelectedItem = NULL;
		}
	}
	else
	{
		BaseClass::OnItemSelected( panelName );
	}
}

void FoundPublicGames::OnCurrentOptionChanged( vgui::Panel *panel )
{
	UpdateGameDetails();
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
