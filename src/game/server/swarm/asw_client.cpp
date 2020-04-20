//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
/*

===== asw_client.cpp ========================================================

  Infested client/server game specific stuff

*/

#include "cbase.h"
#include "asw_player.h"
#include "asw_gamerules.h"
#include "gamerules.h"
#include "teamplay_gamerules.h"
#include "EntityList.h"
#include "physics.h"
#include "game.h"
#include "player_resource.h"
#include "engine/IEngineSound.h"

#include "tier0/vprof.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

void Host_Say( edict_t *pEdict, bool teamonly );

extern bool			g_fGameOver;

// this cvar can be used for automatically restart servers when they are empty
// because even empty server, when being not restarted for several hours
// starts to have performance issues like warping drones and stutters on adrenaline usage
ConVar rd_server_shutdown_after_num_secs( "rd_server_shutdown_after_num_secs", "3600", FCVAR_NONE, "Server will shutdown after being online for this number of seconds." );
ConVar rd_server_shutdown_wait_for_players_num_secs( "rd_server_shutdown_wait_for_players_num_secs", "90", FCVAR_DEVELOPMENTONLY, "Number of seconds to wait for players to load the map before considering server empty" );

/*
===========
ClientPutInServer

called each time a player is spawned into the game
============
*/
void ClientPutInServer( edict_t *pEdict, const char *playername )
{
	// Allocate a CBasePlayer for pev, and call spawn
	CASW_Player *pPlayer = CASW_Player::CreatePlayer( "player", pEdict );
	pPlayer->SetPlayerName( playername );
}


void ClientActive( edict_t *pEdict, bool bLoadGame )
{
	CASW_Player *pPlayer = dynamic_cast< CASW_Player* >( CBaseEntity::Instance( pEdict ) );
	Assert( pPlayer );

	if ( !pPlayer )
	{
		return;
	}

	pPlayer->InitialSpawn();

	if ( !bLoadGame )
	{
		pPlayer->Spawn();
	}
}


void ClientFullyConnect( edict_t *pEntity )
{

}


/*
===============
const char *GetGameDescription()

Returns the descriptive name of this .dll.  E.g., Half-Life, or Team Fortress 2
===============
*/
const char *GetGameDescription()
{
	//DevMsg( "Current time = %f\n", Plat_FloatTime() );
	if ( rd_server_shutdown_after_num_secs.GetInt() > 0 && 
		 engine && engine->IsDedicatedServer() &&
		 Plat_FloatTime() > rd_server_shutdown_after_num_secs.GetInt() )
	{
		int iPlayers = 0;
		for ( int i = 1; i <= gpGlobals->maxClients; i++ )
		{
			CASW_Player* pOtherPlayer = dynamic_cast<CASW_Player*>( UTIL_PlayerByIndex( i ) );

			if ( pOtherPlayer && pOtherPlayer->IsConnected() )
			{
				iPlayers++;
			}
		}
		//DevMsg( "Num players = %i :", iPlayers );

		static float fFirstTimePlayersAreZero = 0;
		const float PLAYER_CHECK_TIMEOUT = rd_server_shutdown_wait_for_players_num_secs.GetFloat();

		if ( iPlayers <= 0 )
		{
			if ( fFirstTimePlayersAreZero == 0)
			{
				//DevMsg( "Not restarting, starting the coundown\n", fFirstTimePlayersAreZero + PLAYER_CHECK_TIMEOUT - Plat_FloatTime() );
				fFirstTimePlayersAreZero = Plat_FloatTime();
			}
			else if ( fFirstTimePlayersAreZero + PLAYER_CHECK_TIMEOUT < Plat_FloatTime() )
			{
				exit( 0 ); // reactivedrop: Isn't the best solution but works. Issuing "quit" doesn't work here. 
			}
			else
			{
				//DevMsg( "Not restarting, waiting %f\n", fFirstTimePlayersAreZero + PLAYER_CHECK_TIMEOUT - Plat_FloatTime() );
			}
		}
		else 
		{
			fFirstTimePlayersAreZero = 0;
			//DevMsg( "\n" );
		}
	}

	if ( g_pGameRules ) // this function may be called before the world has spawned, and the game rules initialized
		return g_pGameRules->GetGameDescription();
	else
	{
		return "Alien Swarm: Reactive Drop";
	}
}

//-----------------------------------------------------------------------------
// Purpose: Given a player and optional name returns the entity of that 
//			classname that the player is nearest facing
//			
// Input  :
// Output :
//-----------------------------------------------------------------------------
CBaseEntity* FindEntity( edict_t *pEdict, char *classname)
{
	// If no name was given set bits based on the picked
	if (FStrEq(classname,"")) 
	{
		CBasePlayer *pPlayer = static_cast<CBasePlayer*>(GetContainingEntity(pEdict));
		if ( pPlayer )
		{
			return pPlayer->FindPickerEntityClass( classname );
		}
	}
	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Precache game-specific models & sounds
//-----------------------------------------------------------------------------
PRECACHE_REGISTER_BEGIN( GLOBAL, ClientGamePrecache )
	PRECACHE( MODEL, "models/player.mdl");
	PRECACHE( MODEL, "models/gibs/agibs.mdl" );
	PRECACHE( MODEL, "models/gibs/hgibs.mdl" );
	PRECACHE( MODEL, "models/gibs/hgibs_spine.mdl" );
	PRECACHE( MODEL, "models/gibs/hgibs_scapula.mdl" );
	
	PRECACHE( GAMESOUND, "FX_AntlionImpact.ShellImpact" );
	PRECACHE( GAMESOUND, "Missile.ShotDown" );
	
	PRECACHE( GAMESOUND, "Geiger.BeepHigh" );
	PRECACHE( GAMESOUND, "Geiger.BeepLow" );

	PRECACHE( KV_DEP_FILE, "resource/ParticleEmitters.txt" )
PRECACHE_REGISTER_END()

void ClientGamePrecache( void )
{
}


// called by ClientKill and DeadThink
void respawn( CBaseEntity *pEdict, bool fCopyCorpse )
{
	if (gpGlobals->coop || gpGlobals->deathmatch)
	{
		if ( fCopyCorpse )
		{
			// make a copy of the dead body for appearances sake
			((CASW_Player *)pEdict)->CreateCorpse();
		}

		// respawn player
		pEdict->Spawn();
	}
	else
	{       // restart the entire server
		engine->ServerCommand("reload\n");
	}
}

void GameStartFrame( void )
{
	VPROF("GameStartFrame()");
	if ( g_fGameOver )
		return;

	gpGlobals->teamplay = (teamplay.GetInt() != 0);
}


//=========================================================
// instantiate the proper game rules object
//=========================================================
void InstallGameRules()
{
	CreateGameRulesObject( "CAlienSwarm" );
}

