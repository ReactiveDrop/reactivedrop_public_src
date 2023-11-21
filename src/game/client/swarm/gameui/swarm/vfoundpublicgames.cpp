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
}

FoundPublicGames::~FoundPublicGames()
{
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
