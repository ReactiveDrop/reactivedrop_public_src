//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//

#include "VFoundGroupGames.h"
#include "EngineInterface.h"

#include "fmtstr.h"
#include "rd_lobby_utils.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;
using namespace BaseModUI;

const char *COM_GetModDirectory();

extern ConVar rd_lobby_ping_low;
extern ConVar rd_lobby_ping_high;

//=============================================================================
FoundGroupGames::FoundGroupGames( Panel *parent, const char *panelName ) : BaseClass( parent, panelName )
{
}

//=============================================================================
void FoundGroupGames::PaintBackground()
{
	char const *szGameMode = m_pDataSettings->GetString( "game/mode" );

	BaseClass::DrawDialogBackground( CFmtStr( "#L4D360UI_FoundGroupGames_Title%s%s", szGameMode[0] ? "_" : "", szGameMode ), NULL,
		"#L4D360UI_FoundGroupGames_Subtitle", NULL, NULL );
}

void FoundGroupGames::OnEvent( KeyValues *pEvent )
{
	char const *szName = pEvent->GetName();

	if ( !Q_stricmp( "OnMatchServerMgrUpdate", szName ) )
	{
		char const *szUpdate = pEvent->GetString( "update", "" );
		if ( !Q_stricmp( "searchstarted", szUpdate ) )
		{
			extern ConVar ui_foundgames_spinner_time;
			m_flSearchStartedTime = Plat_FloatTime();
			m_flSearchEndTime = m_flSearchStartedTime + ui_foundgames_spinner_time.GetFloat();
			OnThink();
		}
		else if ( !Q_stricmp( "searchfinished", szUpdate ) )
		{
			m_flSearchStartedTime = 0.0f;
			UpdateGameDetails();
		}
		else if ( !Q_stricmp( "server", szUpdate ) )
		{
			// Friend's game details have been updated
			// Don't update every individual server, calls a full sort, etc
			// just wait for the searchfinished
			//UpdateGameDetails();
		}
	}
}

//=============================================================================
void FoundGroupGames::AddServersToList( void )
{
	ISteamFriends *pFriends = SteamFriends();
	Assert( pFriends );
	if ( !pFriends )
		return;

	CUtlVector<CSteamID> GroupIDs;
	GroupIDs.SetCount( pFriends->GetClanCount() );
	FOR_EACH_VEC( GroupIDs, i )
	{
		GroupIDs[i] = pFriends->GetClanByIndex( i );
	}

	g_ReactiveDropServerList.m_InternetServers.WantUpdatedServerList();

	FOR_EACH_VEC( g_ReactiveDropServerList.m_InternetServers, i )
	{
		FoundGameListItem::Info info;
		if ( info.SetFromServer( g_ReactiveDropServerList.m_InternetServers, i ) )
		{
			CSteamID group = V_atoui64( info.m_szGroupID );
			if ( !group.BClanAccount() )
				continue;

			if ( GroupIDs.IsValidIndex( GroupIDs.Find( group ) ) )
			{
				AddGameFromDetails( info );
			}
		}
	}

	// now add lobbies that match the servers we just found to the list
	g_ReactiveDropServerList.m_PublicLobbies.WantUpdatedLobbyList();

	FOR_EACH_VEC( g_ReactiveDropServerList.m_PublicLobbies.m_MatchingLobbies, i )
	{
		FoundGameListItem::Info info;
		if ( info.SetFromLobby( g_ReactiveDropServerList.m_PublicLobbies.m_MatchingLobbies[i] ) )
			AddGameFromDetails( info, true );
	}

	AddFriendGamesToList( true );
}
