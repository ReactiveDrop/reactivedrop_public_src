//====== Copyright © 1996-2003, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#include "cbase.h"
#include "gameinterface.h"
#include "mapentities.h"
#include "asw_shareddefs.h"
#include "fmtstr.h"
#include "missionchooser/iasw_mission_chooser.h"
#include "missionchooser/iasw_mission_chooser_source.h"
#include "matchmaking/swarm/imatchext_swarm.h"
#include "asw_spawn_manager.h"
#include "rd_lobby_utils.h"
#include "matchmaking/imatchframework.h"
#include "rd_missions_shared.h"

extern ConVar sv_force_transmit_ents;
extern ConVar mm_max_players;
ConVar rd_adjust_sv_maxrate( "rd_adjust_sv_maxrate", "1", FCVAR_ARCHIVE, "If 1 for 8 slot listen server sv_maxrate will be set to 60000" );

// -------------------------------------------------------------------------------------------- //
// Mod-specific CServerGameClients implementation.
// -------------------------------------------------------------------------------------------- //

void CServerGameClients::GetPlayerLimits( int& minplayers, int& maxplayers, int &defaultMaxPlayers ) const
{
	minplayers = 1; 
	defaultMaxPlayers = 8;
	maxplayers = ASW_MAX_PLAYERS;

	ConVarRef tv_enable( "tv_enable", true );
	if ( tv_enable.IsValid() && tv_enable.GetBool() )
	{
		minplayers++;
		defaultMaxPlayers++;
	}

}

// -------------------------------------------------------------------------------------------- //
// Mod-specific CServerGameDLL implementation.
// -------------------------------------------------------------------------------------------- //

void CServerGameDLL::LevelInit_ParseAllEntities( const char *pMapEntities )
{
	// precache even if not in the level, for onslaught mode
	for ( int i = 0; i < NELEMS( g_Aliens ); i++ )
	{
		UTIL_PrecacheOther( g_Aliens[i].m_pszAlienClass );
		CBaseEntity::PrecacheScriptSound( g_Aliens[i].m_szHordeSound );
	}
}

bool g_bOfflineGame = false;

extern const char *COM_GetModDirectory( void );

//-----------------------------------------------------------------------------
// Purpose: Called to apply lobby settings to a dedicated server
//-----------------------------------------------------------------------------
void CServerGameDLL::ApplyGameSettings( KeyValues *pKV )
{
	if ( !pKV )
		return;

	// Fix the game settings request when a generic request for
	// map launch comes in (it might be nested under reservation
	// processing)
	bool bAlreadyLoadingMap = false;
	const char *pGameDir = COM_GetModDirectory();
	if ( !Q_stricmp( pKV->GetName(), "::ExecGameTypeCfg" ) )
	{
		if ( !engine )
			return;

		char const *szNewMap = pKV->GetString( "map/mapname", "" );
		if ( !szNewMap || !*szNewMap )
			return;

		KeyValues *pLaunchOptions = engine->GetLaunchOptions();
		if ( !pLaunchOptions )
			return;

		if ( FindLaunchOptionByValue( pLaunchOptions, "changelevel" ) ||
			FindLaunchOptionByValue( pLaunchOptions, "changelevel2" ) )
			return;

		if ( FindLaunchOptionByValue( pLaunchOptions, "map_background" ) )
		{

			return;
		}

		bAlreadyLoadingMap = true;

		if ( FindLaunchOptionByValue( pLaunchOptions, "reserved" ) )
		{
			// We are already reserved, but we still need to let the engine
			// baseserver know how many human slots to allocate
			pKV->SetInt( "members/numSlots", g_bOfflineGame ? 1 : mm_max_players.GetInt() );
			return;
		}

		pKV->SetName( pGameDir );
	}

	if ( Q_stricmp( pKV->GetName(), pGameDir ) || bAlreadyLoadingMap )
		return;

	//g_bOfflineGame = pKV->GetString( "map/offline", NULL ) != NULL;
	g_bOfflineGame = !Q_stricmp( pKV->GetString( "system/network", "LIVE" ), "offline" );

	//Msg( "GameInterface reservation payload:\n" );
	//KeyValuesDumpAsDevMsg( pKV );

	// Vitaliy: Disable cheats as part of reservation in case they were enabled (unless we are on Steam Beta)
	if ( sv_force_transmit_ents.IsFlagSet( FCVAR_DEVELOPMENTONLY ) &&	// any convar with FCVAR_DEVELOPMENTONLY flag will be sufficient here
		sv_cheats && sv_cheats->GetBool() )
	{
		sv_cheats->SetValue( 0 );
	}

	static ConVarRef asw_skill( "asw_skill", true );
	const char *szDifficulty = pKV->GetString( "game/difficulty", "normal" );
	if ( !Q_stricmp( szDifficulty, "easy" ) )
	{
		asw_skill.SetValue( 1 );
	}
	else if ( !Q_stricmp( szDifficulty, "normal" ) )
	{
		asw_skill.SetValue( 2 );
	}
	else if ( !Q_stricmp( szDifficulty, "hard" ) )
	{
		asw_skill.SetValue( 3 );
	}
	else if ( !Q_stricmp( szDifficulty, "insane" ) )
	{
		asw_skill.SetValue( 4 );
	}
	else if ( !Q_stricmp( szDifficulty, "imba" ) )
	{
		asw_skill.SetValue( 5 );
	}

	extern ConVar asw_sentry_friendly_fire_scale;
	extern ConVar asw_marine_ff_absorption;
	int nHardcoreFF = pKV->GetInt( "game/hardcoreFF", 0 );
	if ( nHardcoreFF == 1 )
	{
		asw_sentry_friendly_fire_scale.SetValue( 1.0f );
		asw_marine_ff_absorption.SetValue( 0 );
	}
	else
	{
		asw_sentry_friendly_fire_scale.SetValue( 0.0f );
		asw_marine_ff_absorption.SetValue( 1 );
	}

	extern ConVar asw_horde_override;
	extern ConVar asw_wanderer_override;
	int nOnslaught = pKV->GetInt( "game/onslaught", 0 );
	if ( nOnslaught == 1 )
	{
		asw_horde_override.SetValue( 1 );
		asw_wanderer_override.SetValue( 1 );
	}
	else
	{
		asw_horde_override.SetValue( 0 );
		asw_wanderer_override.SetValue( 0 );
	}

	char const *szMapCommand = pKV->GetString( "map/mapcommand", "map" );

	const char *szMode = pKV->GetString( "game/mode", "campaign" );

	char const *szGameMode = pKV->GetString( "game/mode", "" );
	if ( szGameMode && *szGameMode )
	{
		extern ConVar mp_gamemode;
		mp_gamemode.SetValue( szGameMode );
	}

	KeyValues *pKeyValuesForChallenge = NULL;
	if (g_bOfflineGame && g_pMatchFramework && g_pMatchFramework->GetMatchSession() && g_pMatchFramework->GetMatchSession()->GetSessionSettings())
	{
		pKeyValuesForChallenge = g_pMatchFramework->GetMatchSession()->GetSessionSettings();
	}

	extern ConVar rd_challenge;

	if (pKeyValuesForChallenge)
	{
		char const *szChallenge = pKeyValuesForChallenge->GetString("game/challenge", "");
		if ( szChallenge && *szChallenge )
		{
			rd_challenge.SetValue(szChallenge);
		}
	}
	else if ( UTIL_RD_GetCurrentLobbyID().IsValid() )
	{
		rd_challenge.SetValue( UTIL_RD_GetCurrentLobbyData( "game:challenge", "0" ) );
	}
	else
	{
		rd_challenge.SetValue( "0" );
	}

	if ( rd_adjust_sv_maxrate.GetBool() )
	{
		int numSlots = pKV->GetInt( "members/numSlots", 32 );
		if ( numSlots > 4 )
		{
			ConVarRef sv_maxrate( "sv_maxrate" );
			if ( sv_maxrate.IsValid() )
			{
				if ( sv_maxrate.GetInt() < 60000 && sv_maxrate.GetInt() != 0 )
				{
					sv_maxrate.SetValue( 60000 );	// reactivedrop: for 8 player servers increase sv_maxrate by 2 times(30k is default)
				}
			}
		}
		ConVarRef net_splitpacket_maxrate( "net_splitpacket_maxrate" );
		if ( net_splitpacket_maxrate.IsValid() )
		{
			if ( net_splitpacket_maxrate.GetInt() < 40000 )
			{
				net_splitpacket_maxrate.SetValue( 40000 );	// reactivedrop: increase net_splitpacket_maxrate(15k is default) to combat stutter when there is a lot of aliens
			}
		}
	}

	bool bCampaignGame = !Q_stricmp( szMode, "campaign" );
	if ( bCampaignGame || !Q_stricmp( szMode, "single_mission" ) )
	{
		const char *szCampaignName = pKV->GetString( "game/campaign", NULL );
		if ( bCampaignGame && !szCampaignName )
			return;

		IASW_Mission_Chooser_Source* pSource = missionchooser ? missionchooser->LocalMissionSource() : NULL;
		if ( !pSource )
			return;

		char szSaveFilename[ MAX_PATH ];
		szSaveFilename[ 0 ] = 0;
		const char *szStartingMission = pKV->GetString( "game/mission", NULL );

		extern ConVar asw_default_campaign;
		if ( bCampaignGame && szStartingMission && !Q_stricmp( szCampaignName, "jacob" ) && !Q_stricmp( pKV->GetString( "game/state" ), "lobby" ) && Q_stricmp( asw_default_campaign.GetString(), "jacob" ) )
		{
			// BenLubar: waking up a dedicated server has been giving us Jacob's Rest as the campaign but keeping the default campaign's first mission. This is a hack to detect and fix this situation.
			if ( const RD_Campaign_t *pJacob = ReactiveDropMissions::GetCampaign( "jacob" ) )
			{
				if ( pJacob->GetMissionByMapName( szStartingMission ) )
				{
					Warning( "Found mission %s in Jacob's Rest, but we're waking up from hibernation and Jacob's Rest isn't the default campaign!\n", szStartingMission );
				}
				else
				{
					DevMsg( 2, "Mission %s started as Jacob's Rest while server waking up from hibernation. Attempting to fix.\n", szStartingMission );
				}
			}
			else
			{
				Warning( "Could not load Jacob's Rest campaign details. Continuing anyway.\n" );
			}

			if ( const RD_Campaign_t *pDefault = ReactiveDropMissions::GetCampaign( asw_default_campaign.GetString() ) )
			{
				if ( pDefault->GetMissionByMapName( szStartingMission ) )
				{
					Msg( "Fixing campaign in hibernation wakeup: setting to %s for mission %s.\n", asw_default_campaign.GetString(), szStartingMission );
					szCampaignName = asw_default_campaign.GetString();
				}
				else
				{
					DevWarning( 2, "Mission %s not in default campaign %s, but we're waking up from hibernation!\n", szStartingMission, asw_default_campaign.GetString() );
				}
			}
			else
			{
				Warning( "Could not load default campaign details. Continuing anyway.\n" );
			}
		}

		if ( !pSource->ASW_Campaign_CreateNewSaveGame( &szSaveFilename[0], sizeof( szSaveFilename ), szCampaignName, !g_bOfflineGame, szStartingMission ) )
		{
			Msg( "Unable to create new save game.\n" );
			return;
		}

		if ( engine->IsDedicatedServer() )
		{
			mm_max_players.SetValue( gpGlobals->maxClients );
		}

		engine->ServerCommand( CFmtStr( "%s %s %s %s reserved\n",
			szMapCommand,
			szStartingMission ? szStartingMission : "lobby",
			szMode,
			szSaveFilename ) );
	}
	else
	{
		Warning( "ApplyGameSettings: Unknown game mode!\n" );
	}
}