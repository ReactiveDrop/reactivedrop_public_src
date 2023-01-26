#include "cbase.h"
#include "ammodef.h"

#ifdef CLIENT_DLL
	#include "c_asw_marine.h"
	#include "c_asw_pickup.h"
	#include "c_asw_marine_resource.h"
	#include "c_asw_flare_projectile.h"
	#include "c_asw_weapon.h"
	#include "c_asw_game_resource.h"
	#include "c_asw_player.h"
	#include "asw_shareddefs.h"
	#include "c_asw_campaign_save.h"
	#include "c_asw_ammo.h"
	#include "voice_status.h"
	#include "gamestringpool.h"
	#include "c_user_message_register.h"
	#define CASW_Equip_Req C_ASW_Equip_Req
	#include "asw_equip_req.h"
	#define CASW_Weapon C_ASW_Weapon
	#define CAI_BaseNPC C_AI_BaseNPC
	#define CASW_Flare_Projectile C_ASW_Flare_Projectile
	#define CASW_Campaign_Save C_ASW_Campaign_Save
	#define CASW_Ammo C_ASW_Ammo
	#include "c_asw_steamstats.h"
	#include "rd_rich_presence.h"
	#include "c_world.h"
#else
	#include "asw_marine_resource.h"
	#include "player.h"
	#include "game.h"
	#include "gamerules.h"
	#include "teamplay_gamerules.h"
	#include "asw_player.h"
	#include "voice_gamemgr.h"
	#include "globalstate.h"
	#include "ai_basenpc.h"
	#include "asw_game_resource.h"
		
	#include "asw_marine.h"
	#include "asw_spawner.h"
	#include "asw_pickup.h"
	#include "asw_flare_projectile.h"
	#include "asw_weapon.h"
	#include "asw_ammo.h"
	#include "asw_mission_manager.h"
	#include "asw_marine_speech.h"
	#include "asw_gamestats.h"
	#include "ai_networkmanager.h"
	#include "ai_initutils.h"
	#include "ai_network.h"
	#include "ai_navigator.h"
	#include "ai_node.h"
	#include "asw_campaign_save.h"
	#include "asw_egg.h"
	#include "asw_alien_goo_shared.h"
	#include "asw_parasite.h"
	#include "asw_harvester.h"
	#include "asw_drone_advanced.h"
	#include "asw_shieldbug.h"
	#include "asw_parasite.h"
	#include "asw_medals.h"
	#include "asw_mine.h"
	#include "asw_burning.h"
	#include "asw_triggers.h"
	#include "asw_use_area.h"
	#include "asw_grenade_vindicator.h"
	#include "asw_sentry_top.h"
	#include "asw_sentry_base.h"
	#include "asw_radiation_volume.h"
	#include "missionchooser/iasw_mission_chooser_source.h"
	#include "asw_objective.h"
	#include "asw_debrief_stats.h"
	#include "props.h"
	#include "vgui/ILocalize.h"
	#include "inetchannelinfo.h"
	#include "asw_util_shared.h"
	#include "filesystem.h"
	#include "asw_intro_control.h"
	#include "asw_tutorial_spawning.h"
	#include "asw_equip_req.h"
	#include "asw_map_scores.h"
	#include "world.h"
	#include "asw_bloodhound.h"
	#include "asw_fire.h"
	#include "engine/IEngineSound.h"
	#include "asw_pickup_weapon.h"
	#include "asw_fail_advice.h"
	#include "asw_spawn_manager.h"
	#include "asw_path_utils.h"
	#include "EntityFlame.h"
	#include "asw_buffgrenade_projectile.h"
	#include "asw_achievements.h"
	#include "asw_director.h"
	#include "team.h"
	#include "asw_pickup_equipment.h"
	#include "Sprite.h"
	#include "highres_timer.h"
	#include "env_tonemap_controller.h"
	#include "asw_marine_hint.h"
	#include "eventqueue.h"
	#include "ai_dynamiclink.h"
	#include "asw_spawn_selection.h"
	#include "asw_door.h"
	#include "ScriptGameEventListener.h"
	#include "cdll_int.h"
#endif
#include "fmtstr.h"
#include "game_timescale_shared.h"
#include "asw_gamerules.h"
#include "asw_equipment_list.h"
#include "asw_marine_profile.h"
#include "asw_weapon_parse.h"
#include "asw_weapon_ammo_bag_shared.h"
#include "takedamageinfo.h"
#include "asw_holdout_mode.h"
#include "asw_deathmatch_mode.h"
#include "asw_powerup_shared.h"
#include "missionchooser/iasw_mission_chooser.h"
#include "missionchooser/iasw_random_missions.h"
#include "missionchooser/iasw_map_builder.h"
#include "rd_challenges_shared.h"
#include "rd_missions_shared.h"
#include "rd_workshop.h"
#include "rd_lobby_utils.h"
#include "matchmaking/imatchframework.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar old_radius_damage;

#define ASW_LAUNCHING_STEP 0.25f			// time between each stage of launching
#define ASW_DEFAULT_COMMANDER_FACE "briefing/face_pilot"

#ifndef CLIENT_DLL
	extern ConVar asw_debug_alien_damage;
	extern ConVar asw_medal_lifesaver_dist;
	extern ConVar asw_medal_lifesaver_kills;
	extern ConVar asw_tutorial_save_stage;
	extern ConVar asw_horde;
	extern ConVar asw_director_controls_spawns;
	extern ConVar asw_medal_barrel_kills;
	extern ConVar rd_killingspree_time_limit;
	extern ConVar rd_quake_sounds;
	extern ConVar rd_server_marine_backpacks;
	ConVar asw_objective_slowdown_time( "asw_objective_slowdown_time", "1.8", FCVAR_CHEAT, "Length of time that the slowdown effect lasts." );
	ConVar asw_marine_explosion_protection("asw_marine_explosion_protection", "0.5", FCVAR_CHEAT, "Reduction of explosion radius against marines");
	ConVar asw_door_explosion_boost("asw_door_explosion_boost", "2.0", FCVAR_CHEAT, "Sets damage scale for grenades vs doors");
	// Alien modifiers for difficulty level
	// Base mission difficulty is:
	//    Easy   = 3
	//    Normal = 5
	//    Hard   = 7
	//    Insane = 10
	//  With campaign map modifers affecting this number (-2 to +2)

	// Alien health and damage modifiers are applied for every step away from
	// the base mission difficulty level of 5.

	// 20% extra health for every difficulty above 5
	ConVar asw_difficulty_alien_health_step("asw_difficulty_alien_health_step", "0.2", FCVAR_CHEAT, "How much alien health is changed per mission difficulty level");
	// 20% extra damage for every difficulty above 5
	ConVar asw_difficulty_alien_damage_step("asw_difficulty_alien_damage_step", "0.2", FCVAR_CHEAT, "How much alien damage is changed per mission difficulty level");
	ConVar asw_ww_chatter_interval_min("asw_ww_chatter_interval_min", "200", 0, "Min time between wildcat and wolfe conversation");
	ConVar asw_ww_chatter_interval_max("asw_ww_chatter_interval_max", "260", 0, "Max time between wildcat and wolfe conversation");
	ConVar asw_compliment_chatter_interval_min("asw_compliment_chatter_interval_min", "180", 0, "Min time between kill compliments");
	ConVar asw_compliment_chatter_interval_max("asw_compliment_chatter_interval_max", "240", 0, "Max time between kill compliments");	
	ConVar asw_default_campaign("asw_default_campaign", "jacob", FCVAR_ARCHIVE, "Default campaign used when dedicated server restarts");
	ConVar rd_max_marines("rd_max_marines", "-1", FCVAR_NONE, "Sets how many marines can be selected"); 
	ConVar asw_campaign_wounding("asw_campaign_wounding", "0", FCVAR_NONE, "Whether marines are wounded in the roster if a mission is completed with the marine having taken significant damage");
	ConVar asw_drop_powerups("asw_drop_powerups", "0", FCVAR_CHEAT, "Do aliens drop powerups?");
	ConVar asw_adjust_difficulty_by_number_of_marines( "asw_adjust_difficulty_by_number_of_marines", "1", FCVAR_CHEAT, "If enabled, difficulty will be reduced when there are only 3 or 2 marines." );
	ConVar rd_increase_difficulty_by_number_of_marines( "rd_increase_difficulty_by_number_of_marines", "0", FCVAR_CHEAT, "If enabled, difficulty will be increased when there are more than 4 marines." );	
	ConVar sv_vote_kick_ban_duration("sv_vote_kick_ban_duration", "5", 0, "How long should a kick vote ban someone from the server? (in minutes)");
	ConVar sv_timeout_when_fully_connected( "sv_timeout_when_fully_connected", "30", FCVAR_NONE, "Once fully connected, player will be kicked if he doesn't send a network message within this interval." );
	ConVar mm_swarm_state( "mm_swarm_state", "ingame", FCVAR_DEVELOPMENTONLY );
	ConVar rd_request_experience("rd_request_experience", "1", FCVAR_DEVELOPMENTONLY, "For dev, if 1 RequestExperience function is called. Used for standard coop");
	ConVar rd_reassign_marines("rd_reassign_marines", "1", FCVAR_NONE, "For dev, if 1 ReassignMarines function will be called");
	ConVar rd_ready_mark_override("rd_ready_mark_override", "0", FCVAR_NONE, "If set to 1 all players will be auto ready, the green ready mark will be set to checked state");
	ConVar rd_server_shutdown_when_empty( "rd_server_shutdown_when_empty", "0", FCVAR_NONE, "Server will shutdown after last player left." );
	ConVar rd_auto_kick_low_level_player( "rd_auto_kick_low_level_player", "0", FCVAR_CHEAT, "Server auto kicks players below level 30 from challenges which have this cvar set to 1. This cvar is meant for players who use dedicated server browser to join games, since Public Games window already restricts filters to max Hard difficulty and challenge being disabled" );
	ConVar rd_auto_kick_low_level_player_if_brutal_or_challenge( "rd_auto_kick_low_level_player_if_brutal_or_challenge", "0", FCVAR_CHEAT, "Server auto kicks players below level 30 if difficulty is brutal or a challenge is active" );
	ConVar rd_auto_kick_low_level_player_if_brutal_or_asbi( "rd_auto_kick_low_level_player_if_brutal_or_asbi", "0", FCVAR_NONE, "Server auto kicks players below level 30 if difficulty is brutal or official asbi is active" );
	ConVar rd_auto_kick_high_ping_player( "rd_auto_kick_high_ping_player", "0", FCVAR_CHEAT, "Server auto kick players with pings higher than this cvar." );
	ConVar rd_clearhouse_on_mission_complete( "rd_clearhouse_on_mission_complete", "0", FCVAR_NONE, "If 1 all NPCs will be removed from map on round end" );
	ConVar rd_sentry_block_aliens( "rd_sentry_block_aliens", "1", FCVAR_CHEAT, "If 0 sentries don't collide with aliens" );
	ConVar rd_auto_fast_restart( "rd_auto_fast_restart", "0", FCVAR_NONE, "Set to 1 to restart mission on fail automatically" );
	static void RDAAutoMissionFailedInstantRestartChanged( IConVar *pConVar, const char *szOldValue, float flOldValue )
	{
		Warning( "rda_auto_mission_failed_instant_restart is deprecated and may be removed in a future version - use rd_auto_fast_restart instead.\n" );
		rd_auto_fast_restart.SetValue( ConVarRef( pConVar ).GetString() );
	}
	ConVar rda_auto_mission_failed_instant_restart( "rda_auto_mission_failed_instant_restart", "0", FCVAR_HIDDEN, "", &RDAAutoMissionFailedInstantRestartChanged );
	ConVar rd_adjust_mod_dont_load_vertices("rd_adjust_mod_dont_load_vertices", "1", FCVAR_NONE, "Automatically disables loading of vertex data.", true, 0, true, 1);
	ConVar rd_dedicated_high_resolution_timer_ms( "rd_dedicated_high_resolution_timer_ms", "0.01", FCVAR_NONE, "Acquire timer with specified resolution in ms" );
	ConVar rd_radial_damage_no_falloff_distance( "rd_radial_damage_no_falloff_distance", "16", FCVAR_CHEAT, "Distance from an explosion where damage starts to decrease based on distance.", true, 0, false, 0 );
	ConVar rda_marine_allow_strafe("rda_marine_allow_strafe", "0", FCVAR_CHEAT, "Allow marines to use strafe command");
	ConVar rd_mapcycle_deathmatch( "rd_mapcycle_deathmatch", "1", FCVAR_ARCHIVE, "Automatically select the next Deathmatch mission." );
	ConVar rd_mapcycle_endless( "rd_mapcycle_endless", "0", FCVAR_ARCHIVE, "Automatically select the next Endless mission." );
	ConVar rd_mapcycle_bonus( "rd_mapcycle_bonus", "2", FCVAR_ARCHIVE, "Automatically select the next Bonus mission." );
	ConVar rd_mapcycle_ignore( "rd_mapcycle_ignore", "", FCVAR_ARCHIVE, "Comma-separated list of map filenames (no .bsp) that cannot be selected by map cycle." );

	// allow updateing the high res timer realtime
	inline void HighResTimerChangeCallback( IConVar* pConVar, const char* pOldString, float flOldValue )
	{
		// on change apply timer resolution immediately
		highres_timer_set( rd_dedicated_high_resolution_timer_ms.GetFloat() );
	}

	static void EnforceWeaponClassRestriction( IConVar *pConVar = NULL, const char *pOldValue = NULL, float flOldValue = 0.0f )
	{
		// don't do anything if the new value is the same as the old value
		if ( !V_strcmp( ConVarRef( pConVar ).GetString(), pOldValue ) )
			return;

		ConVarRef rd_weapons_regular_class_unrestricted( "rd_weapons_regular_class_unrestricted" );
		ConVarRef rd_weapons_extra_class_unrestricted( "rd_weapons_extra_class_unrestricted" );
		if ( rd_weapons_regular_class_unrestricted.GetInt() == -2 && rd_weapons_extra_class_unrestricted.GetInt() == -2 )
			return;

		if ( ASWGameRules() && ASWGameResource() && ASWEquipmentList() && ASWGameRules()->GetGameState() == ASW_GS_BRIEFING )
		{
			for ( int i = 0; i < ASW_MAX_MARINE_RESOURCES; i++ )
			{
				CASW_Marine_Resource *pMR = ASWGameResource()->GetMarineResource( i );
				if ( !pMR )
					continue;

				CASW_Player *pPlayer = pMR->GetCommander();
				CASW_Marine_Profile *pProfile = pMR->GetProfile();
				if ( !pPlayer || !pProfile )
					continue;

				for ( int j = 0; j < ASW_MAX_EQUIP_SLOTS; j++ )
				{
					const char *szWeaponClass = pProfile->m_DefaultWeaponsInSlots[ j ];
					int nWeaponIndex = ASWEquipmentList()->GetIndexForSlot( j, szWeaponClass );
					engine->ClientCommand( pPlayer->edict(), "cl_loadout %d %d %d", pProfile->m_ProfileIndex, j, nWeaponIndex );
				}
			}
		}
	}

	// reactivedrop: this callback function is called when rd_weapons_<slot>_allowed cvar is
	// changed. It checks whether marine has not allowed weapons selected and replaces them
	// with an allowed alternative. Such as replace all weapons with rifles for Rifle Mod
	static void EnforceWeaponSelectionRules( IConVar *pConVar = NULL, const char *pOldValue = NULL, float flOldValue = 0.0f )
	{
		if ( ASWGameRules() && ASWGameResource() )
		{
			for ( int i = 0; i < ASW_MAX_MARINE_RESOURCES; i++ )
			{
				CASW_Marine_Resource *pMR = ASWGameResource()->GetMarineResource( i );
				if ( !pMR )
				{
					continue;
				}

				for ( int j = 0; j < ASW_MAX_EQUIP_SLOTS; j++ )
				{
					pMR->m_iWeaponsInSlots.Set( j, ASWGameRules()->ApplyWeaponSelectionRules( j, pMR->m_iWeaponsInSlots.Get( j ) ) );
				}
			}
		}
	}

	// reactivedrop: this callback function is called when rd_player_bots_allowed cvar is
	// changed. If the value is 0 it will remove all bots from the briefing
	static void DeselectMarineBots( IConVar *pConVar = NULL, const char *pOldValue = NULL, float flOldValue = 0.0f )
	{
		ConVarRef rd_player_bots_allowed( "rd_player_bots_allowed" );
		if ( rd_player_bots_allowed.GetBool() )
			return;

		if ( ASWGameRules() && ASWGameResource() )
		{
			for ( int i = 0; i < ASW_MAX_MARINE_RESOURCES; i++ )
			{
				CASW_Marine_Resource *pMR = ASWGameResource()->GetMarineResource( i );
				if ( !pMR )
				{
					continue;
				}

				CASW_Player *pPlayer = pMR->GetCommander();
				if ( !pPlayer )
				{
					continue;
				}

				engine->ClientCommand( pPlayer->edict(), "cl_dselectm 0;cl_dselectm 1;cl_dselectm 2;cl_dselectm 3;cl_dselectm 4;cl_dselectm 5;cl_dselectm 6;cl_dselectm 7;" );
			}
		}
	}
#endif

static void UpdateMatchmakingTagsCallback( IConVar *pConVar, const char *pOldValue, float flOldValue )
{
#ifdef GAME_DLL
	// don't allow changing challenge convars from their desired values
	if ( pConVar && ASWGameRules() && ASWGameRules()->m_SavedConvars_Challenge.Defined( pConVar->GetName() ) )
	{
		const char *pszDesiredValue = STRING( ASWGameRules()->m_SavedConvars_Challenge[pConVar->GetName()] );
		if ( Q_strcmp( ConVarRef( pConVar->GetName() ).GetString(), pszDesiredValue ) )
		{
			pConVar->SetValue( pszDesiredValue );
			return;
		}
	}

	// update sv_tags to force an update of the matchmaking tags
	static ConVarRef sv_tags( "sv_tags" );

	if ( sv_tags.IsValid() )
	{
		char buffer[1024];
		Q_snprintf( buffer, sizeof( buffer ), "%s", sv_tags.GetString() );
		sv_tags.SetValue( buffer );
	}
#else
	g_ReactiveDropWorkshop.CheckForRequiredAddons();

	C_AlienSwarm *pAlienSwarm = ASWGameRules();
	if ( !pAlienSwarm || !UTIL_RD_IsLobbyOwner() )
	{
		return;
	}

	KeyValues *pSettings = g_pMatchFramework->GetMatchSession()->GetSessionSettings();

	// The matchmaking library knows if we're on a dedicated server but does not automatically add that information to the Steam lobby.
	if ( V_strcmp( pSettings->GetString( "server/server" ), pSettings->GetString( "options/server" ) ) )
	{
		KeyValues::AutoDelete pUpdate( "update" );
		pUpdate->SetString( "update/options/server", pSettings->GetString( "server/server" ) );
		g_pMatchFramework->GetMatchSession()->UpdateSessionSettings( pUpdate );
	}

	// mm_max_players gets updated after it's read for the lobby, so we need to update the slot count here as well.
	SteamMatchmaking()->SetLobbyMemberLimit( UTIL_RD_GetCurrentLobbyID(), gpGlobals->maxClients );
	UTIL_RD_UpdateCurrentLobbyData( "members:numSlots", gpGlobals->maxClients );

	PublishedFileId_t missionAddonID = pAlienSwarm->m_iMissionWorkshopID.Get();
	if ( missionAddonID == k_PublishedFileIdInvalid )
	{
		UTIL_RD_RemoveCurrentLobbyData( "game:missioninfo:workshop" );
	}
	else
	{
		UTIL_RD_UpdateCurrentLobbyData( "game:missioninfo:workshop", missionAddonID );
	}

	if ( g_ASW_Steamstats.IsOfficialCampaign() )
	{
		UTIL_RD_UpdateCurrentLobbyData( "game:missioninfo:official", "1" );
	}
	else
	{
		UTIL_RD_RemoveCurrentLobbyData( "game:missioninfo:official" );
	}

	UTIL_RD_UpdateCurrentLobbyData( "game:missioninfo:map_name", engine->GetLevelName() );

	UTIL_RD_UpdateCurrentLobbyData( "system:game_version", engine->GetProductVersionString() );
	UTIL_RD_UpdateCurrentLobbyData( "system:map_version", GetClientWorldEntity()->m_nMapVersion );
	if ( ISteamApps *pSteamApps = SteamApps() )
	{
		UTIL_RD_UpdateCurrentLobbyData( "system:game_build", pSteamApps->GetAppBuildId() );
		char szBranch[256]{};
		pSteamApps->GetCurrentBetaName( szBranch, sizeof( szBranch ) );
		UTIL_RD_UpdateCurrentLobbyData( "system:game_branch", szBranch );

		KeyValues::AutoDelete pUpdate( "update" );
		pUpdate->SetString( "update/system/game_version", engine->GetProductVersionString() );
		pUpdate->SetInt( "update/system/map_version", GetClientWorldEntity()->m_nMapVersion );
		pUpdate->SetInt( "update/system/game_build", pSteamApps->GetAppBuildId() );
		pUpdate->SetString( "update/system/game_branch", szBranch );
		g_pMatchFramework->GetMatchSession()->UpdateSessionSettings( pUpdate );
	}
	else
	{
		UTIL_RD_RemoveCurrentLobbyData( "system:game_build" );
		UTIL_RD_RemoveCurrentLobbyData( "system:game_branch" );
	}

	if ( C_ASW_Deathmatch_Mode *pDeathmatch = ASWDeathmatchMode() )
	{
		switch ( pDeathmatch->GetGameMode() )
		{
		case GAMEMODE_DEATHMATCH:
			UTIL_RD_UpdateCurrentLobbyData( "game:deathmatch", "deathmatch" );
			break;
		case GAMEMODE_INSTAGIB:
			UTIL_RD_UpdateCurrentLobbyData( "game:deathmatch", "instagib" );
			break;
		case GAMEMODE_GUNGAME:
			UTIL_RD_UpdateCurrentLobbyData( "game:deathmatch", "gungame" );
			break;
		case GAMEMODE_TEAMDEATHMATCH:
			UTIL_RD_UpdateCurrentLobbyData( "game:deathmatch", "teamdeathmatch" );
			break;
		default:
			AssertOnce( !"Unhandled deathmatch mode" );
			UTIL_RD_UpdateCurrentLobbyData( "game:deathmatch", CFmtStr( "unknown_%d", pDeathmatch->GetGameMode() ) );
			break;
		}
	}
	else
	{
		UTIL_RD_RemoveCurrentLobbyData( "game:deathmatch" );
	}

	extern ConVar rd_challenge;

	if ( !V_strcmp( rd_challenge.GetString(), "0" ) )
	{
		UTIL_RD_UpdateCurrentLobbyData( "game:challenge", "0" );
		UTIL_RD_RemoveCurrentLobbyData( "game:challengeinfo:workshop" );
		UTIL_RD_RemoveCurrentLobbyData( "game:challengeinfo:displaytitle" );
	}
	else
	{
		UTIL_RD_UpdateCurrentLobbyData( "game:challenge", rd_challenge.GetString() );
		PublishedFileId_t challengeAddonID = ReactiveDropChallenges::WorkshopID( rd_challenge.GetString() );
		if ( challengeAddonID == k_PublishedFileIdInvalid )
		{
			UTIL_RD_RemoveCurrentLobbyData( "game:challengeinfo:workshop" );
		}
		else
		{
			UTIL_RD_UpdateCurrentLobbyData( "game:challengeinfo:workshop", challengeAddonID );
		}
		UTIL_RD_UpdateCurrentLobbyData( "game:challengeinfo:displaytitle", ReactiveDropChallenges::DisplayName( rd_challenge.GetString() ) );
	}
	UTIL_RD_UpdateCurrentLobbyData( "game:onslaught", pAlienSwarm->IsOnslaught() ? "1" : "0" );
	UTIL_RD_UpdateCurrentLobbyData( "game:hardcoreFF", pAlienSwarm->IsHardcoreFF() ? "1" : "0" );

	CUtlVector<PublishedFileId_t> RequiredAddons;
	g_ReactiveDropWorkshop.GetRequiredAddons( RequiredAddons );

	char szRequiredAddons[1024]{};
	FOR_EACH_VEC( RequiredAddons, i )
	{
		if ( i )
			V_snprintf( szRequiredAddons, sizeof( szRequiredAddons ), "%s,%llX", szRequiredAddons, RequiredAddons[i] );
		else
			V_snprintf( szRequiredAddons, sizeof( szRequiredAddons ), "%llX", RequiredAddons[i] );
	}

	UTIL_RD_UpdateCurrentLobbyData( "game:required_workshop_items", szRequiredAddons );
#endif
}

// moved here for deathmatch
ConVar asw_marine_names("asw_marine_names", "1", FCVAR_CHEAT | FCVAR_REPLICATED, "Whether to show the marine name");
ConVar asw_world_healthbars("asw_world_healthbars", "1", FCVAR_CHEAT | FCVAR_REPLICATED, "Shows health bars in the game world");
ConVar asw_world_usingbars("asw_world_usingbars", "1", FCVAR_CHEAT | FCVAR_REPLICATED, "Shows using bars in the game world");
ConVar rd_show_arrow_to_marine("rd_show_arrow_to_marine", "1", FCVAR_CHEAT | FCVAR_REPLICATED, "If 1 then arrows show where marines are"); 
//
// also for deathmatch
ConVar rd_show_others_laser_pointer("rd_show_others_laser_pointer", "1", FCVAR_CHEAT | FCVAR_REPLICATED, "Allows to show laser pointers of other marines, not my marine");
ConVar rd_paint_marine_blips("rd_paint_marine_blips", "1", FCVAR_CHEAT | FCVAR_REPLICATED, "If 1 marines' position and facing is shown on minimap");
ConVar rd_paint_scanner_blips("rd_paint_scanner_blips", "1", FCVAR_CHEAT | FCVAR_REPLICATED, "If 1 scanner blips are shown on minimap");
ConVar rd_deathmatch_loadout_allowed("rd_deathmatch_loadout_allowed", "0", FCVAR_REPLICATED, "If set to 1 players can choose weapons they spawn with");
ConVar rd_respawn_time( "rd_respawn_time", "0.2",  FCVAR_REPLICATED, "Number of seconds after you can respawn in Deathmatch", true, 0.2f, false, 1000.0f);
ConVar rd_ground_shooting( "rd_ground_shooting", "0",  FCVAR_CHEAT | FCVAR_REPLICATED, "1 enable ground shooting, 0 disabled" );
ConVar asw_cam_marine_pitch( "asw_cam_marine_pitch", "60", FCVAR_CHEAT | FCVAR_REPLICATED, "Marine Camera: pitch." );
ConVar asw_cam_marine_dist( "asw_cam_marine_dist", "412", FCVAR_CHEAT | FCVAR_REPLICATED, "Marine Camera: Distance from marine." );
ConVar rd_allow_afk( "rd_allow_afk", "1", FCVAR_REPLICATED, "If set to 0 players cannot use asw_afk command or Esc - Take a Break" );
// for deathmatch

ConVar asw_vote_duration("asw_vote_duration", "30", FCVAR_REPLICATED, "Time allowed to vote on a map/campaign/saved game change.");
#ifdef CLIENT_DLL
ConVar asw_marine_death_cam("asw_marine_death_cam", "1", FCVAR_ARCHIVE | FCVAR_DEMO, "Use death cam");
#else
static void UpdateGameRulesDeathCamSlowdown( IConVar *var, const char *pOldValue, float flOldValue );
ConVar asw_marine_death_cam_slowdown( "asw_marine_death_cam_slowdown", "1", FCVAR_ARCHIVE, "Slow down time when a marine dies", &UpdateGameRulesDeathCamSlowdown );
static void UpdateGameRulesDeathCamSlowdown( IConVar *var, const char *pOldValue, float flOldValue )
{
	CAlienSwarm *pASW = ASWGameRules();
	if ( pASW )
	{
		pASW->m_bDeathCamSlowdown = asw_marine_death_cam_slowdown.GetBool();
	}
}
#endif
ConVar asw_marine_death_cam_time_interp("asw_marine_death_cam_time_interp", "0.5", FCVAR_CHEAT | FCVAR_REPLICATED, "Time to blend into the death cam");
ConVar asw_marine_death_cam_time_interp_out("asw_marine_death_cam_time_interp_out", "0.75", FCVAR_CHEAT | FCVAR_REPLICATED, "Time to blend out of the death cam");
ConVar asw_marine_death_cam_time("asw_marine_death_cam_time", "0.4", FCVAR_CHEAT | FCVAR_REPLICATED, "Time to do the slowdown death cam");
ConVar asw_marine_death_cam_hold("asw_marine_death_cam_time_hold", "1.75", FCVAR_CHEAT | FCVAR_REPLICATED, "Time to hold on the dying marine at time ramps back up");
ConVar asw_marine_death_cam_time_local_hold("asw_marine_death_cam_time_local_hold", "5.0", FCVAR_CHEAT | FCVAR_REPLICATED, "Time to hold on the dying marine at time ramps back up if they died");
ConVar asw_marine_death_cam_time_scale("asw_marine_death_cam_time_scale", "0.035", FCVAR_CHEAT | FCVAR_REPLICATED, "Time scale during death cam");
ConVar asw_campaign_death("asw_campaign_death", "0", FCVAR_REPLICATED, "Whether marines are killed in the roster if a mission is completed with the marine dead");
ConVar asw_stim_time_scale("asw_stim_time_scale", "0.35", FCVAR_REPLICATED | FCVAR_CHEAT, "Time scale during stimpack slomo");
ConVar asw_time_scale_delay("asw_time_scale_delay", "0.15", FCVAR_REPLICATED | FCVAR_CHEAT, "Delay before timescale changes to give a chance for the client to comply and predict.");
ConVar asw_ignore_need_two_player_requirement("asw_ignore_need_two_player_requirement", "1", FCVAR_REPLICATED, "If set to 1, ignores the mission setting that states two players are needed to start the mission.");
ConVar mp_gamemode( "mp_gamemode", "campaign", FCVAR_REPLICATED | FCVAR_DEVELOPMENTONLY, "Current game mode for matchmaking.dll." );
ConVar sv_gametypes( "sv_gametypes", "campaign,bonus_mission,endless,deathmatch", FCVAR_REPLICATED, "Game modes that can be selected on this server." );
// this cvar is tricky if we want to have lobbies with different number of slots(e.g. 4 or 8)
// when client creates a lobby it sets the number of slots for lobby
// when client searches for lobbies and this cvar is set to 3 then all 4+ player lobbies will be unjoinable, the join button will be greyed out
// when client connects to a dedicated server which has lobbies enabled, the new lobby with this number of slots will be created, no matter what maxplayers value server has, so the client desides how big the lobby will be
ConVar mm_max_players( "mm_max_players", "32", FCVAR_REPLICATED, "Max players for matchmaking system" );
ConVar asw_sentry_friendly_fire_scale( "asw_sentry_friendly_fire_scale", "0", FCVAR_REPLICATED, "Damage scale for sentry gun friendly fire", UpdateMatchmakingTagsCallback );
ConVar asw_marine_ff_absorption( "asw_marine_ff_absorption", "1", FCVAR_REPLICATED, "Friendly fire absorption style (0=none 1=ramp up 2=ramp down)", UpdateMatchmakingTagsCallback );
ConVar asw_horde_override( "asw_horde_override", "0", FCVAR_REPLICATED, "Forces hordes to spawn", UpdateMatchmakingTagsCallback );
ConVar asw_wanderer_override( "asw_wanderer_override", "0", FCVAR_REPLICATED, "Forces wanderers to spawn", UpdateMatchmakingTagsCallback );
ConVar rd_challenge( "rd_challenge", "0", FCVAR_REPLICATED | FCVAR_DEMO, "Activates a challenge by ID", UpdateMatchmakingTagsCallback );
ConVar rd_techreq( "rd_techreq", "1", FCVAR_CHEAT | FCVAR_REPLICATED, "If 0 tech will be not required to start a mission. Mission will not restart if tech dies. 1 is default" );
ConVar rd_hackall( "rd_hackall", "0", FCVAR_CHEAT | FCVAR_REPLICATED, "If 1 all marines can hack doors and computers" );
ConVar rd_weapons_show_hidden( "rd_weapons_show_hidden", "0", FCVAR_CHEAT | FCVAR_REPLICATED, "If 1 will show the hidden weapons and extra items at briefing" );
ConVar rd_weapons_regular_class_unrestricted( "rd_weapons_regular_class_unrestricted", "-1", FCVAR_CHEAT | FCVAR_REPLICATED, "Space separated array of unrestricted class weapon IDs: 1 3 16. See asw_list_equipment. -2 value will un-restrict all regular weapons"
#ifdef GAME_DLL
	, EnforceWeaponClassRestriction );
#else
	);
#endif
ConVar rd_weapons_extra_class_unrestricted( "rd_weapons_extra_class_unrestricted", "-1", FCVAR_CHEAT | FCVAR_REPLICATED, "Space separated array of unrestricted class weapon IDs: 11 17. See asw_list_equipment. -2 value will un-restrict all extra weapons"
#ifdef GAME_DLL
	, EnforceWeaponClassRestriction );
#else
	);
#endif

ConVar rd_weapons_regular_allowed( "rd_weapons_regular_allowed", "-1", FCVAR_CHEAT | FCVAR_REPLICATED, "Space separated array of allowed weapon IDs: 0 6 9 15. See asw_list_equipment" );
ConVar rd_weapons_extra_allowed( "rd_weapons_extra_allowed", "-1", FCVAR_CHEAT | FCVAR_REPLICATED, "Space separated array of allowed extra items IDs: 1 2 7 12. See asw_list_equipment" );
ConVar rd_weapons_regular_allowed_inverted( "rd_weapons_regular_allowed_inverted", "0", FCVAR_CHEAT | FCVAR_REPLICATED, "If 1 inverts the allowed weapons for rd_weapons_regular_allowed" );
ConVar rd_weapons_extra_allowed_inverted( "rd_weapons_extra_allowed_inverted", "0", FCVAR_CHEAT | FCVAR_REPLICATED, "If 1 inverts the allowed weapons for rd_weapons_extra_allowed");
ConVar rd_weapon_requirement_override( "rd_weapon_requirement_override", "0", FCVAR_CHEAT | FCVAR_REPLICATED, "0 = asw_equip_req wins, 1 = rd_weapons_<slot>_allowed wins"
#ifdef GAME_DLL
	, EnforceWeaponSelectionRules );
#else
	);
#endif
ConVar rd_carnage_scale( "rd_carnage_scale", "1", FCVAR_CHEAT | FCVAR_REPLICATED, "the factor used to scale the amount of aliens in each drone spawner" );
ConVar rd_heavy_scale( "rd_heavy_scale", "1.0", FCVAR_CHEAT | FCVAR_REPLICATED, "the factor used to scale aliens' health" );
ConVar rd_alien_speed_scale( "rd_alien_speed_scale", "1.0", FCVAR_CHEAT | FCVAR_REPLICATED, "the factor used to scale aliens' speed" );
ConVar rd_refill_secondary( "rd_refill_secondary", "0", FCVAR_CHEAT | FCVAR_REPLICATED, "0 = false, 1 = true. If true the secondary ammo is picked up too" );
ConVar rd_allow_revive( "rd_allow_revive", "0", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar rd_revive_duration( "rd_revive_duration", "2.0", FCVAR_CHEAT | FCVAR_REPLICATED, "How long it takes to revive an incapacitated marine" );
ConVar rd_revive_health( "rd_revive_health", "10", FCVAR_CHEAT | FCVAR_REPLICATED, "How much health a revived marine gets" );
ConVar rd_hp_regen( "rd_hp_regen", "0", FCVAR_CHEAT | FCVAR_REPLICATED, "0 disable marines' health regeneration" );
ConVar rd_add_bots( "rd_add_bots", "0", FCVAR_CHEAT | FCVAR_REPLICATED, "1 add bots to fill free slots, 0 don't add" );
ConVar rd_ammo_bonus( "rd_ammo_bonus", "0", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar asw_bonus_charges( "asw_bonus_charges", "0", FCVAR_CHEAT | FCVAR_REPLICATED, "Bonus ammo given to starting equipment" );
ConVar rd_infinite_spawners( "rd_infinite_spawners", "0", FCVAR_CHEAT | FCVAR_REPLICATED, "If 1 all spawners will be set to infinitely spawn aliens" );
ConVar rd_hud_hide_clips( "rd_hud_hide_clips", "0", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar rd_biomass_ignite_from_explosions( "rd_biomass_ignite_from_explosions", "0", FCVAR_CHEAT | FCVAR_REPLICATED, "If 1, biomass will ignite from blast damage" );
ConVar rd_spawn_medkits( "rd_spawn_medkits", "0", FCVAR_CHEAT | FCVAR_REPLICATED, "Will spawn a med kit from 31st killed alien is set to 31" );
ConVar rd_spawn_ammo( "rd_spawn_ammo", "0", FCVAR_CHEAT | FCVAR_REPLICATED, "Will spawn an ammo box from 51st killed alien if set to 51" );
ConVar rd_alien_num_min_players( "rd_alien_num_min_players", "4", FCVAR_REPLICATED | FCVAR_CHEAT, "with this many or fewer players, alien number scale will use the minimum value" );
ConVar rd_alien_num_max_players( "rd_alien_num_max_players", "8", FCVAR_REPLICATED | FCVAR_CHEAT, "with this many or more players, alien number scale will use the maximum value" );
ConVar rd_alien_num_min_scale( "rd_alien_num_min_scale", "1", FCVAR_REPLICATED | FCVAR_CHEAT, "the minimum alien number scale for co-op" );
ConVar rd_alien_num_max_scale( "rd_alien_num_max_scale", "2", FCVAR_REPLICATED | FCVAR_CHEAT, "the maximum alien number scale for co-op" );
// this cvar is used by client only, but, to support it in challenges we need to make it replicated and set it from server
ConVar rd_ray_trace_distance( "rd_ray_trace_distance", "3000", FCVAR_REPLICATED | FCVAR_CHEAT, "Increase this parameter for huge maps for grenade launcher to properly aim to far away distances" );
ConVar rd_leaderboard_enabled( "rd_leaderboard_enabled", "1", FCVAR_REPLICATED, "If 0 player leaderboard scores will not be updated on mission complete. Use this for modded servers." );
ConVar rd_aim_marines( "rd_aim_marines", "0", FCVAR_CHEAT | FCVAR_REPLICATED, "If 1 marines can aim at marines that are on different Z level" );

ConVar rd_points_delay( "rd_points_delay", "1.5", FCVAR_REPLICATED, "Number of seconds after the score changes before it starts decaying.", true, 0, false, 0 );
ConVar rd_points_delay_max( "rd_points_delay_max", "5", FCVAR_REPLICATED, "Maximum number of seconds that the score can remain still without decaying.", true, 0, false, 0 );
ConVar rd_points_decay( "rd_points_decay", "0.97", FCVAR_REPLICATED, "Amount that score change decays by per tick.", true, 0, true, 0.999 );
ConVar rd_points_decay_tick( "rd_points_decay_tick", "0.01", FCVAR_REPLICATED, "Number of seconds between score decay ticks.", true, 0, false, 0 );

#ifdef CLIENT_DLL
ConVar rd_skip_all_dialogue( "rd_skip_all_dialogue", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Tell the server not to send audio from asw_voiceover_dialogue." );
#endif

// ASW Weapons
// Rifle (5 clips, 98 per)
ConVar	sk_plr_dmg_asw_r( "sk_plr_dmg_asw_r", "7", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	sk_npc_dmg_asw_r( "sk_npc_dmg_asw_r", "7", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	sk_max_asw_r( "sk_max_asw_r", "490", FCVAR_REPLICATED | FCVAR_CHEAT );
// Rifle Grenade (4 def 8 max)
ConVar	sk_plr_dmg_asw_r_g( "sk_plr_dmg_asw_r_g", "100", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	sk_npc_dmg_asw_r_g( "sk_npc_dmg_asw_r_g", "100", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	sk_max_asw_r_g( "sk_max_asw_r_g", "8", FCVAR_REPLICATED | FCVAR_CHEAT );
// Autogun (1 clip max on this)
ConVar	sk_plr_dmg_asw_ag( "sk_plr_dmg_asw_ag", "9", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	sk_npc_dmg_asw_ag( "sk_npc_dmg_asw_ag", "9", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	sk_max_asw_ag( "sk_max_asw_ag", "250", FCVAR_REPLICATED | FCVAR_CHEAT );
// Shotgun (12 clips, 4 per)
ConVar	sk_plr_dmg_asw_sg( "sk_plr_dmg_asw_sg", "10", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	sk_npc_dmg_asw_sg( "sk_npc_dmg_asw_sg", "10", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	sk_max_asw_sg( "sk_max_asw_sg", "48", FCVAR_REPLICATED | FCVAR_CHEAT );
// Shotgun (secondary)
ConVar	sk_plr_dmg_asw_sg_g( "sk_plr_dmg_asw_sg_g", "0", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	sk_npc_dmg_asw_sg_g( "sk_npc_dmg_asw_sg_g", "0", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	sk_max_asw_sg_g( "sk_max_asw_sg_g", "0", FCVAR_REPLICATED | FCVAR_CHEAT );
// Vindicator (5 clips, 14 per)
ConVar	sk_plr_dmg_asw_asg( "sk_plr_dmg_asw_asg", "10", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	sk_npc_dmg_asw_asg( "sk_npc_dmg_asw_asg", "10", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	sk_max_asw_asg( "sk_max_asw_asg", "70", FCVAR_REPLICATED | FCVAR_CHEAT );
// Flamer (5 clips, 80 per)
ConVar	sk_plr_dmg_asw_f( "sk_plr_dmg_asw_f", "5", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	sk_npc_dmg_asw_f( "sk_npc_dmg_asw_f", "5", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	sk_max_asw_f( "sk_max_asw_f", "400", FCVAR_REPLICATED | FCVAR_CHEAT );
// Pistol (10 clips, 32 per)
ConVar	sk_plr_dmg_asw_p( "sk_plr_dmg_asw_p", "22", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	sk_npc_dmg_asw_p( "sk_npc_dmg_asw_p", "22", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	sk_max_asw_p( "sk_max_asw_p", "240", FCVAR_REPLICATED | FCVAR_CHEAT );
// Mining laser
ConVar	sk_plr_dmg_asw_ml( "sk_plr_dmg_asw_ml", "50", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	sk_npc_dmg_asw_ml( "sk_npc_dmg_asw_ml", "50", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	sk_max_asw_ml( "sk_max_asw_ml", "250", FCVAR_REPLICATED | FCVAR_CHEAT );
// Tesla CANNON
ConVar	sk_plr_dmg_asw_tg( "sk_plr_dmg_asw_tg", "50", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	sk_npc_dmg_asw_tg( "sk_npc_dmg_asw_tg", "50", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	sk_max_asw_tg( "sk_max_asw_tg", "260", FCVAR_REPLICATED | FCVAR_CHEAT );
// Chainsaw
ConVar	sk_plr_dmg_asw_cs( "sk_plr_dmg_asw_cs", "8", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	sk_npc_dmg_asw_cs( "sk_npc_dmg_asw_cs", "8", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	sk_max_asw_cs( "sk_max_asw_cs", "1111", FCVAR_REPLICATED | FCVAR_CHEAT );
// Railgun
ConVar	sk_plr_dmg_asw_rg( "sk_plr_dmg_asw_rg", "45", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	sk_npc_dmg_asw_rg( "sk_npc_dmg_asw_rg", "45", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	sk_max_asw_rg( "sk_max_asw_rg", "72", FCVAR_REPLICATED | FCVAR_CHEAT );
// Flares
ConVar	sk_plr_dmg_asw_flares( "sk_plr_dmg_asw_flares", "1", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	sk_npc_dmg_asw_flares( "sk_npc_dmg_asw_flares", "1", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	sk_max_asw_flares( "sk_max_asw_flares", "0", FCVAR_REPLICATED | FCVAR_CHEAT );
// Medkit
ConVar	sk_plr_dmg_asw_medkit( "sk_plr_dmg_asw_medkit", "1", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	sk_npc_dmg_asw_medkit( "sk_npc_dmg_asw_medkit", "1", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	sk_max_asw_medkit( "sk_max_asw_medkit", "0", FCVAR_REPLICATED | FCVAR_CHEAT );
// Med Satchel
ConVar	sk_plr_dmg_asw_medsat( "sk_plr_dmg_asw_medsat", "1", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	sk_npc_dmg_asw_medsat( "sk_npc_dmg_asw_medsat", "1", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	sk_max_asw_medsat( "sk_max_asw_medsat", "0", FCVAR_REPLICATED | FCVAR_CHEAT );
// Med Satchel self heal secondary fire
ConVar	sk_plr_dmg_asw_medself( "sk_plr_dmg_asw_medself", "1", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	sk_npc_dmg_asw_medself( "sk_npc_dmg_asw_medself", "1", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	sk_max_asw_medself( "sk_max_asw_medself", "0", FCVAR_REPLICATED | FCVAR_CHEAT );
// Med Stim
ConVar	sk_plr_dmg_asw_stim( "sk_plr_dmg_asw_stim", "1", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	sk_npc_dmg_asw_stim( "sk_npc_dmg_asw_stim", "1", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	sk_max_asw_stim( "sk_max_asw_stim", "0", FCVAR_REPLICATED | FCVAR_CHEAT );
// Welder
ConVar	sk_plr_dmg_asw_welder( "sk_plr_dmg_asw_welder", "0", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	sk_npc_dmg_asw_welder( "sk_npc_dmg_asw_welder", "0", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	sk_max_asw_welder( "sk_max_asw_welder", "0", FCVAR_REPLICATED | FCVAR_CHEAT );
// Extinguisher
ConVar	sk_plr_dmg_asw_ext( "sk_plr_dmg_asw_ext", "1", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	sk_npc_dmg_asw_ext( "sk_npc_dmg_asw_ext", "1", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	sk_max_asw_ext( "sk_max_asw_ext", "0", FCVAR_REPLICATED | FCVAR_CHEAT );
// Mines
ConVar	sk_plr_dmg_asw_mines( "sk_plr_dmg_asw_mines", "1", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	sk_npc_dmg_asw_mines( "sk_npc_dmg_asw_mines", "1", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	sk_max_asw_mines( "sk_max_asw_mines", "0", FCVAR_REPLICATED | FCVAR_CHEAT );
// PDW (6 clips, 80 per)
ConVar	sk_plr_dmg_asw_pdw( "sk_plr_dmg_asw_pdw", "4", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	sk_npc_dmg_asw_pdw( "sk_npc_dmg_asw_pdw", "4", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	sk_max_asw_pdw( "sk_max_asw_pdw", "480", FCVAR_REPLICATED | FCVAR_CHEAT );
// Hand Grenades
ConVar	sk_npc_dmg_asw_hg( "sk_npc_dmg_asw_hg", "0", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	sk_max_asw_hg( "sk_max_asw_hg", "0", FCVAR_REPLICATED | FCVAR_CHEAT );
// Grenade launcher
ConVar	sk_npc_dmg_asw_gl( "sk_npc_dmg_asw_gl", "0", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	sk_max_asw_gl( "sk_max_asw_gl", "18", FCVAR_REPLICATED | FCVAR_CHEAT );
// Sniper Rifle
ConVar	sk_npc_dmg_asw_sniper( "sk_npc_dmg_asw_sniper", "0", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	sk_max_asw_sniper( "sk_max_asw_sniper", "60", FCVAR_REPLICATED | FCVAR_CHEAT );

// Bulldog (9 clips, 7 per)
ConVar	sk_plr_dmg_asw_deagle( "sk_plr_dmg_asw_deagle", "75", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	sk_npc_dmg_asw_deagle( "sk_npc_dmg_asw_deagle", "75", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	sk_max_asw_deagle( "sk_max_asw_deagle", "63", FCVAR_REPLICATED | FCVAR_CHEAT );

// Devastator Automated Heavy Shotgun (1 clip, 70 per)
ConVar	sk_plr_dmg_asw_devastator( "sk_plr_dmg_asw_devastator", "10", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	sk_npc_dmg_asw_devastator( "sk_npc_dmg_asw_devastator", "10", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	sk_max_asw_devastator( "sk_max_asw_devastator", "70", FCVAR_REPLICATED | FCVAR_CHEAT );

// 50 Cal Machine Gun
ConVar	sk_plr_dmg_asw_50calmg( "sk_plr_dmg_asw_50calmg", "0", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	sk_npc_dmg_asw_50calmg( "sk_npc_dmg_asw_50calmg", "0", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	sk_max_asw_50calmg( "sk_max_asw_50calmg", "0", FCVAR_REPLICATED | FCVAR_CHEAT );

// Gas_Grenades
ConVar	sk_plr_dmg_asw_gas_grenades( "sk_plr_dmg_asw_gas_grenades", "0", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	sk_npc_dmg_asw_gas_grenades( "sk_npc_dmg_asw_gas_grenades", "0", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	sk_max_asw_gas_grenades( "sk_max_asw_gas_grenades", "0", FCVAR_REPLICATED | FCVAR_CHEAT );

// Heavy Rifle
ConVar	sk_plr_dmg_asw_hr( "sk_plr_dmg_asw_hr", "7", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	sk_npc_dmg_asw_hr( "sk_npc_dmg_asw_hr", "7", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	sk_max_asw_hr( "sk_max_asw_hr", "490", FCVAR_REPLICATED | FCVAR_CHEAT );

// Heavy Rifle (secondary)
ConVar	sk_plr_dmg_asw_hr_g( "sk_plr_dmg_asw_hr_g", "1", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	sk_npc_dmg_asw_hr_g( "sk_npc_dmg_asw_hr_g", "1", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	sk_max_asw_hr_g( "sk_max_asw_hr_g", "5", FCVAR_REPLICATED | FCVAR_CHEAT );

// Med Rifle (7 clips, 72 per)
ConVar	sk_plr_dmg_asw_medrifle( "sk_plr_dmg_asw_medrifle", "7", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	sk_npc_dmg_asw_medrifle( "sk_npc_dmg_asw_medrifle", "7", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	sk_max_asw_medrifle( "sk_max_asw_medrifle", "504", FCVAR_REPLICATED | FCVAR_CHEAT );

// AR2 (6 clips, 30 per)
ConVar	sk_plr_dmg_ar2( "sk_plr_dmg_ar2", "8", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	sk_npc_dmg_ar2( "sk_npc_dmg_ar2", "8", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	sk_max_ar2( "sk_max_ar2", "180", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar	sk_max_ar2_altfire( "sk_max_ar2_altfire", "3", FCVAR_REPLICATED | FCVAR_CHEAT );

ConVar sk_asw_parasite_infest_dmg_easy( "sk_asw_parasite_infest_dmg_easy", "175", FCVAR_REPLICATED | FCVAR_CHEAT, "Total damage from parasite infestation" );
ConVar sk_asw_parasite_infest_dmg_normal( "sk_asw_parasite_infest_dmg_normal", "225", FCVAR_REPLICATED | FCVAR_CHEAT, "Total damage from parasite infestation" );
ConVar sk_asw_parasite_infest_dmg_hard( "sk_asw_parasite_infest_dmg_hard", "270", FCVAR_REPLICATED | FCVAR_CHEAT, "Total damage from parasite infestation" );
ConVar sk_asw_parasite_infest_dmg_insane( "sk_asw_parasite_infest_dmg_insane", "280", FCVAR_REPLICATED | FCVAR_CHEAT, "Total damage from parasite infestation" );
ConVar sk_asw_parasite_infest_dmg_brutal( "sk_asw_parasite_infest_dmg_brutal", "280", FCVAR_REPLICATED | FCVAR_CHEAT, "Total damage from parasite infestation" );

// reactivedrop: adding these weapon damage overrides for PvP 
ConVar	rd_shotgun_dmg_base("rd_shotgun_dmg_base",	"0", FCVAR_REPLICATED | FCVAR_CHEAT, "Base damage of shotgun", true, 0, false, 0);
ConVar	rd_rifle_dmg_base("rd_rifle_dmg_base", "0", FCVAR_REPLICATED | FCVAR_CHEAT, "Base damage of rifle", true, 0, false, 0);
ConVar	rd_prifle_dmg_base("rd_prifle_dmg_base", "0", FCVAR_REPLICATED | FCVAR_CHEAT, "Base damage of prototype rifle", true, 0, false, 0);
ConVar	rd_autogun_dmg_base("rd_autogun_dmg_base", "0", FCVAR_REPLICATED | FCVAR_CHEAT, "Base damage of shotgun", true, 0, false, 0);
ConVar	rd_vindicator_dmg_base("rd_vindicator_dmg_base", "0", FCVAR_REPLICATED | FCVAR_CHEAT, "Base damage of vindicator", true, 0, false, 0);
ConVar	rd_pistol_dmg_base("rd_pistol_dmg_base", "0", FCVAR_REPLICATED | FCVAR_CHEAT, "Base damage of twin pistols", true, 0, false, 0);
ConVar	rd_railgun_dmg_base("rd_railgun_dmg_base", "0", FCVAR_REPLICATED | FCVAR_CHEAT, "Base damage of railgun", true, 0, false, 0);
ConVar	rd_pdw_dmg_base("rd_pdw_dmg_base", "0", FCVAR_REPLICATED | FCVAR_CHEAT, "Base damage of PDW", true, 0, false, 0);
ConVar	rd_flamer_dmg_base("rd_flamer_dmg_base", "0", FCVAR_REPLICATED | FCVAR_CHEAT, "Base damage of flamer", true, 0, false, 0);
ConVar	rd_minigun_dmg_base("rd_minigun_dmg_base", "0", FCVAR_REPLICATED | FCVAR_CHEAT, "Base damage of minigun", true, 0, false, 0);
ConVar	rd_sniper_dmg_base("rd_sniper_dmg_base", "0", FCVAR_REPLICATED | FCVAR_CHEAT, "Base damage of sniper rifle", true, 0, false, 0);
ConVar	rd_chainsaw_dmg_base("rd_chainsaw_dmg_base", "0", FCVAR_REPLICATED | FCVAR_CHEAT, "Base damage of chainsaw", true, 0, false, 0);
ConVar	rd_grenade_launcher_dmg_base("rd_grenade_launcher_dmg_base", "0", FCVAR_REPLICATED | FCVAR_CHEAT, "Base damage of grenade launcher", true, 0, false, 0);
ConVar	rd_mininglaser_dmg_base("rd_mininglaser_dmg_base", "0", FCVAR_REPLICATED | FCVAR_CHEAT, "Base damage of mining laser", true, 0, false, 0);
ConVar	rd_deagle_dmg_base("rd_deagle_dmg_base", "0", FCVAR_REPLICATED | FCVAR_CHEAT, "Base damage of deagle", true, 0, false, 0);
ConVar	rd_devastator_dmg_base("rd_devastator_dmg_base", "0", FCVAR_REPLICATED | FCVAR_CHEAT, "Base damage of devastator", true, 0, false, 0);
ConVar	rd_combat_rifle_dmg_base("rd_combat_rifle_dmg_base", "0", FCVAR_REPLICATED | FCVAR_CHEAT, "Base damage of combat rifle", true, 0, false, 0);
ConVar	rd_heavy_rifle_dmg_base("rd_heavy_rifle_dmg_base", "0", FCVAR_REPLICATED | FCVAR_CHEAT, "Base damage of heavy rifle", true, 0, false, 0);
ConVar	rd_medrifle_dmg_base("rd_medrifle_dmg_base", "0", FCVAR_REPLICATED | FCVAR_CHEAT, "Base damage of medical rifle", true, 0, false, 0);
ConVar	rd_grenades_dmg_base( "rd_grenades_dmg_base", "0", FCVAR_REPLICATED | FCVAR_CHEAT, "Base damage of hand grenades", true, 0, false, 0);
ConVar	rd_ar2_dmg_base( "rd_ar2_dmg_base", "0", FCVAR_REPLICATED | FCVAR_CHEAT, "Base damage of AR2", true, 0, false, 0 );

ConVar asw_flare_autoaim_radius("asw_flare_autoaim_radius", "250", FCVAR_REPLICATED | FCVAR_CHEAT, "Radius of autoaim effect from flares");
ConVar asw_vote_kick_fraction("asw_vote_kick_fraction", "0.6", FCVAR_REPLICATED, "Fraction of players needed to activate a kick vote");
ConVar asw_vote_leader_fraction("asw_vote_leader_fraction", "0.6", FCVAR_REPLICATED, "Fraction of players needed to activate a leader vote");
ConVar asw_vote_map_fraction("asw_vote_map_fraction", "0.6", FCVAR_REPLICATED, "Fraction of players needed to activate a map vote");
ConVar asw_marine_collision("asw_marine_collision", "0", FCVAR_REPLICATED, "Whether marines collide with each other or not, in a multiplayer game");
ConVar asw_skill( "asw_skill","2", FCVAR_REPLICATED, "Game skill level (1-5).", true, 1, true, 5 );
ConVar asw_money( "asw_money", "0", FCVAR_REPLICATED, "Can players collect money?" );
ConVar asw_client_build_maps( "asw_client_build_maps", "0", FCVAR_REPLICATED, "Whether clients compile random maps rather than getting sent them" );
ConVar rd_deathmatch_ending_time( "rd_deathmatch_ending_time", "10.0",  FCVAR_REPLICATED, "Period of time after which the round ends after somebody has won" );
ConVar rd_jumpjet_knockdown_marines( "rd_jumpjet_knockdown_marines", "0",  FCVAR_CHEAT | FCVAR_REPLICATED, "If 1 Jump Jet knock down marines" );
ConVar rd_default_weapon( "rd_default_weapon", "0",  FCVAR_CHEAT | FCVAR_REPLICATED, "Index of the weapon that is given to marine after he spawns" );
ConVar asw_player_avoidance( "asw_player_avoidance", "1", FCVAR_CHEAT | FCVAR_REPLICATED, "Enable/Disable player avoidance." );
#ifdef GAME_DLL
static void UpdateGameRulesOverrideAllowRotateCamera( IConVar *var, const char *pOldValue, float flOldValue );
ConVar rd_override_allow_rotate_camera( "rd_override_allow_rotate_camera", "-1", FCVAR_ARCHIVE, "-1(default)-uses asw_gamerules setting, 0-disable rotation, 1-enable rotation", true, -1, true, 1, &UpdateGameRulesOverrideAllowRotateCamera );
static void UpdateGameRulesOverrideAllowRotateCamera( IConVar *var, const char *pOldValue, float flOldValue )
{
	CAlienSwarm *pASW = ASWGameRules();
	if ( pASW )
	{
		pASW->m_iOverrideAllowRotateCamera = rd_override_allow_rotate_camera.GetInt();
	}
}
#endif
ConVar rd_player_bots_allowed( "rd_player_bots_allowed", "1", FCVAR_CHEAT | FCVAR_REPLICATED, "If 0 will prevent players from adding bots"
#ifdef GAME_DLL
	, DeselectMarineBots );
#else
	);
#endif
#ifdef GAME_DLL
ConVar rd_slowmo( "rd_slowmo", "1", FCVAR_NONE, "If 0 env_slomo will be deleted from map on round start(if present)" );
#endif
ConVar rd_queen_hud_suppress_time( "rd_queen_hud_suppress_time", "-1.0", FCVAR_CHEAT | FCVAR_REPLICATED, "Hides the Swarm Queen's health HUD if not damaged for this long (-1 to always show)" );
ConVar rd_anniversary_week_debug( "rd_anniversary_week_debug", "-1", FCVAR_CHEAT | FCVAR_REPLICATED, "Set to 1 to force anniversary week logic; 0 to force off" );
extern ConVar asw_stats_verbose;

#define ADD_STAT( field, amount ) \
			if ( asw_stats_verbose.GetBool() ) \
			{ \
				DevMsg( "marine %d (%s %d+%d)\n", ASWGameResource()->GetMarineResourceIndex( pMR ), #field, pMR->field, amount ); \
			} \
			pMR->field += amount;

REGISTER_GAMERULES_CLASS( CAlienSwarm );

BEGIN_NETWORK_TABLE_NOBASE( CAlienSwarm, DT_ASWGameRules )
	#ifdef CLIENT_DLL
		RecvPropInt(RECVINFO(m_iGameState)),
		RecvPropBool(RECVINFO(m_bMissionSuccess)),
		RecvPropBool(RECVINFO(m_bMissionFailed)),
		RecvPropInt(RECVINFO(m_nFailAdvice)),
		RecvPropInt(RECVINFO(m_iMissionDifficulty) ),
		RecvPropInt(RECVINFO(m_iSkillLevel) ),
		RecvPropInt(RECVINFO(m_iCurrentVoteYes) ),
		RecvPropInt(RECVINFO(m_iCurrentVoteNo) ),
		RecvPropInt(RECVINFO(m_iCurrentVoteType) ),
		RecvPropString(RECVINFO(m_szCurrentVoteDescription) ),
		RecvPropString(RECVINFO(m_szCurrentVoteMapName) ),
		RecvPropString(RECVINFO(m_szCurrentVoteCampaignName) ),
		RecvPropFloat(RECVINFO(m_fVoteEndTime)),
		RecvPropFloat(RECVINFO(m_fBriefingStartedTime) ),
		RecvPropBool(RECVINFO(m_bMissionRequiresTech)),
		RecvPropBool(RECVINFO(m_bCheated)),
		RecvPropEHandle(RECVINFO(m_hStartStimPlayer)),
		RecvPropFloat(RECVINFO(m_flStimEndTime)),
		RecvPropFloat(RECVINFO(m_flStimStartTime)),
		RecvPropFloat(RECVINFO(m_flRestartingMissionTime)),
		RecvPropFloat(RECVINFO(m_fPreventStimMusicTime)),
		RecvPropBool(RECVINFO(m_bForceStylinCam)),
		RecvPropBool(RECVINFO(m_bShowCommanderFace)),
		RecvPropEHandle(RECVINFO(m_hCurrentStylinCam)),
		RecvPropFloat(RECVINFO(m_fMarineDeathTime)),
		RecvPropVector(RECVINFO(m_vMarineDeathPos)),
		RecvPropInt(RECVINFO(m_nMarineForDeathCam)),
		RecvPropFloat(RECVINFO(m_fMissionStartedTime)),
		RecvPropInt(RECVINFO(m_iMissionWorkshopID)),
		RecvPropBool(RECVINFO(m_bDeathCamSlowdown)),
		RecvPropInt(RECVINFO(m_iOverrideAllowRotateCamera)),
		RecvPropString(RECVINFO(m_szApproximatePingLocation)),
		RecvPropString(RECVINFO(m_szBriefingVideo)),
		RecvPropEHandle(RECVINFO(m_hBriefingCamera)),
		RecvPropString( RECVINFO( m_szDeathmatchWinnerName ) ),
		RecvPropString( RECVINFO( m_szCycleNextMap ) ),
	#else
		SendPropInt(SENDINFO(m_iGameState), 8, SPROP_UNSIGNED ),
		SendPropBool(SENDINFO(m_bMissionSuccess)),
		SendPropBool(SENDINFO(m_bMissionFailed)),
		SendPropInt(SENDINFO(m_nFailAdvice)),
		SendPropInt(SENDINFO(m_iMissionDifficulty) ),
		SendPropInt(SENDINFO(m_iSkillLevel) ),
		SendPropInt(SENDINFO(m_iCurrentVoteYes) ),
		SendPropInt(SENDINFO(m_iCurrentVoteNo) ),
		SendPropInt(SENDINFO(m_iCurrentVoteType) ),
		SendPropString(SENDINFO(m_szCurrentVoteDescription) ),
		SendPropString(SENDINFO(m_szCurrentVoteMapName) ),
		SendPropString(SENDINFO(m_szCurrentVoteCampaignName) ),
		SendPropFloat(SENDINFO(m_fVoteEndTime) ),
		SendPropFloat(SENDINFO(m_fBriefingStartedTime) ),
		SendPropBool(SENDINFO(m_bMissionRequiresTech)),
		SendPropBool(SENDINFO(m_bCheated)),
		SendPropEHandle(SENDINFO(m_hStartStimPlayer)),
		SendPropFloat(SENDINFO(m_flStimEndTime), 0, SPROP_NOSCALE),
		SendPropFloat(SENDINFO(m_flStimStartTime), 0, SPROP_NOSCALE),
		SendPropFloat(SENDINFO(m_flRestartingMissionTime), 0, SPROP_NOSCALE),
		SendPropFloat(SENDINFO(m_fPreventStimMusicTime), 0, SPROP_NOSCALE),
		SendPropBool(SENDINFO(m_bForceStylinCam)),
		SendPropBool(SENDINFO(m_bShowCommanderFace)),
		SendPropEHandle(SENDINFO(m_hCurrentStylinCam)),
		SendPropFloat(SENDINFO(m_fMarineDeathTime), 0, SPROP_NOSCALE),
		SendPropVector(SENDINFO(m_vMarineDeathPos)),
		SendPropInt(SENDINFO(m_nMarineForDeathCam), 8),
		SendPropFloat(SENDINFO(m_fMissionStartedTime)),
		SendPropInt(SENDINFO(m_iMissionWorkshopID), 64, SPROP_UNSIGNED),
		SendPropBool(SENDINFO(m_bDeathCamSlowdown)),
		SendPropInt(SENDINFO(m_iOverrideAllowRotateCamera)),
		SendPropString(SENDINFO(m_szApproximatePingLocation)),
		SendPropString(SENDINFO(m_szBriefingVideo)),
		SendPropEHandle(SENDINFO(m_hBriefingCamera)),
		SendPropString( SENDINFO( m_szDeathmatchWinnerName ) ),
		SendPropString( SENDINFO( m_szCycleNextMap ) ),
	#endif
END_NETWORK_TABLE()


BEGIN_DATADESC( CAlienSwarmProxy )
	DEFINE_KEYFIELD( m_iSpeedrunTime, FIELD_INTEGER, "speedruntime" ),
	DEFINE_KEYFIELD( m_iJumpJetType,  FIELD_INTEGER, "jumpjettype" ),
	DEFINE_KEYFIELD( m_bDisallowCameraRotation, FIELD_BOOLEAN, "disallowcamerarotation" ),
#ifdef GAME_DLL
	DEFINE_INPUTFUNC( FIELD_INTEGER, "SetTutorialStage", InputSetTutorialStage ),
	DEFINE_INPUTFUNC( FIELD_INTEGER, "AddPoints", InputAddPoints ),
	DEFINE_INPUTFUNC( FIELD_INTEGER, "ModifyDifficulty", InputModifyDifficulty ),
	DEFINE_INPUTFUNC( FIELD_EHANDLE, "MarineFinishedMission", InputMarineFinishedMission ),
	DEFINE_OUTPUT( m_OnDifficulty, "OnDifficulty" ),
	DEFINE_OUTPUT( m_OnOnslaught, "OnOnslaught" ),
	DEFINE_OUTPUT( m_OnFriendlyFire, "OnFriendlyFire" ),
	DEFINE_OUTPUT( m_OnChallenge, "OnChallenge" ),
	DEFINE_OUTPUT( m_TotalPoints, "TotalPoints" ),
	DEFINE_OUTPUT( m_MissionDifficulty, "MissionDifficulty" ),
#endif
END_DATADESC()

LINK_ENTITY_TO_CLASS( asw_gamerules, CAlienSwarmProxy );
IMPLEMENT_NETWORKCLASS_ALIASED( AlienSwarmProxy, DT_AlienSwarmProxy )

static CAlienSwarmProxy *g_pSwarmProxy = NULL;

CAlienSwarmProxy::CAlienSwarmProxy()
{
	m_iSpeedrunTime = 0;
	m_iJumpJetType = 0;
	m_bDisallowCameraRotation = false;

	g_pSwarmProxy = this;
}

CAlienSwarmProxy::~CAlienSwarmProxy()
{
	if ( g_pSwarmProxy == this )
	{
		g_pSwarmProxy = NULL;
	}
}

#ifdef CLIENT_DLL
	void CAlienSwarmProxy::OnDataChanged( DataUpdateType_t updateType )
	{
		BaseClass::OnDataChanged( updateType );
		if ( ASWGameRules() )
		{
			ASWGameRules()->OnDataChanged( updateType );
		}
	}

	void RecvProxy_ASWGameRules( const RecvProp *pProp, void **pOut, void *pData, int objectID )
	{
		CAlienSwarm *pRules = ASWGameRules();
		Assert( pRules );
		*pOut = pRules;
	}

	BEGIN_RECV_TABLE( CAlienSwarmProxy, DT_AlienSwarmProxy )
		RecvPropDataTable( "asw_gamerules_data", 0, 0, &REFERENCE_RECV_TABLE( DT_ASWGameRules ), RecvProxy_ASWGameRules ),
		RecvPropBool( RECVINFO( m_bDisallowCameraRotation ) ),		
	END_RECV_TABLE()
#else
	void* SendProxy_ASWGameRules( const SendProp *pProp, const void *pStructBase, const void *pData, CSendProxyRecipients *pRecipients, int objectID )
	{
		CAlienSwarm *pRules = ASWGameRules();
		Assert( pRules );
		pRecipients->SetAllRecipients();
		return pRules;
	}

	BEGIN_SEND_TABLE( CAlienSwarmProxy, DT_AlienSwarmProxy )
		SendPropDataTable( "asw_gamerules_data", 0, &REFERENCE_SEND_TABLE( DT_ASWGameRules ), SendProxy_ASWGameRules ),
		SendPropBool( SENDINFO( m_bDisallowCameraRotation ) ),
	END_SEND_TABLE()

bool CAlienSwarmProxy::KeyValue( const char *szKeyName, const char *szValue )
{
	if ( FStrEq( szKeyName, "briefingmovie" ) )
	{
		CAlienSwarm *pGameRules = ASWGameRules();
		Assert( pGameRules );
		if ( pGameRules && szValue[0] != '\0' )
		{
			V_snprintf( pGameRules->m_szBriefingVideo.GetForModify(), sizeof( pGameRules->m_szBriefingVideo ), "media/%s.bik", szValue );
		}

		return true;
	}

	return BaseClass::KeyValue( szKeyName, szValue );
}

void CAlienSwarmProxy::InputSetTutorialStage( inputdata_t & inputdata )
{
	CAlienSwarm *pGameRules = ASWGameRules();
	Assert( pGameRules );
	if ( !pGameRules || !pGameRules->IsTutorialMap() )
	{
		Warning( "Cannot SetTutorialStage on non-tutorial map.\n" );
		return;
	}

	CASW_Marine_Resource *pMR = ASWGameResource()->GetMarineResource( 0 );

	asw_tutorial_save_stage.SetValue( inputdata.value.Int() );
	pGameRules->SetGameState( ASW_GS_BRIEFING );
	pGameRules->m_iMarinesSpawned = 0;
	pGameRules->StartTutorial( pMR->GetCommander() );
}

void CAlienSwarmProxy::InputAddPoints( inputdata_t & inputdata )
{
	CASW_Game_Resource *pGameResource = ASWGameResource();
	Assert( pGameResource );
	if ( !pGameResource )
	{
		Warning( "Cannot AddPoints: cannot find game resource\n" );
		return;
	}

	if ( inputdata.value.Int() < 0 )
	{
		Warning( "Cannot AddPoints with negative value. (%d)\n", inputdata.value.Int() );
		return;
	}

	int iMaxScore = inputdata.value.Int();

	for ( int i = 0; i < pGameResource->GetMaxMarineResources(); i++ )
	{
		CASW_Marine_Resource *pMR = pGameResource->GetMarineResource( i );
		if ( !pMR )
			continue;

		if ( pMR->m_iScore < 0 )
		{
			Warning( "Cannot AddPoints on this map: overview not tagged with 'points' or mission not yet started.\n" );
			return;
		}

		int64_t iTotal = int64_t( pMR->m_iScore ) + int64_t( inputdata.value.Int() );
		if ( iTotal < 0 || iTotal > INT32_MAX )
		{
			Warning( "AddPoints would overflow by %d - clamping.\n", int( iTotal - INT32_MAX ) );
			iTotal = INT32_MAX;
		}

		pMR->m_TimelineScore.RecordValue( iTotal - pMR->m_iScore );
		pMR->m_iScore = iTotal;

		iMaxScore = MAX( iMaxScore, pMR->m_iScore );
	}

	m_TotalPoints.Set( iMaxScore, inputdata.pActivator, inputdata.pCaller );

	CBroadcastRecipientFilter filter;
	UserMessageBegin( filter, "ShowObjectives" );
	WRITE_FLOAT( 30.0f );
	MessageEnd();
}

void CAlienSwarmProxy::InputModifyDifficulty( inputdata_t & inputdata )
{
	CAlienSwarm *pGameRules = ASWGameRules();
	Assert( pGameRules );
	if ( !pGameRules || pGameRules->GetGameState() != ASW_GS_INGAME )
	{
		Warning( "Cannot ModifyDifficulty when mission is not active.\n" );
		return;
	}

	int iOldDifficulty = pGameRules->m_iMissionDifficulty;

	// limit difficulty to between 2 (classic minimum) and 10 million (going a bit above this makes shieldbug health overflow).
	int iNewDifficulty = clamp( iOldDifficulty + inputdata.value.Int(), 2, 10000000 );

	DevMsg( "Mapper modified difficulty from %d to %d\n", iOldDifficulty, iNewDifficulty );
	pGameRules->m_iMissionDifficulty = iNewDifficulty;
	m_MissionDifficulty.Set( iNewDifficulty, inputdata.pActivator, inputdata.pCaller );
}

void CAlienSwarmProxy::InputMarineFinishedMission( inputdata_t & inputdata )
{
	CBaseEntity *pEnt = inputdata.value.Entity();
	CASW_Marine *pMarine = CASW_Marine::AsMarine( pEnt );
	if ( !pMarine )
	{
		Warning( "Cannot MarineFinishedMission on something that is not a marine (%s)\n", pEnt ? pEnt->GetDebugName() : "<<NULL>>" );
		return;
	}

	CAlienSwarm *pGameRules = ASWGameRules();
	Assert( pGameRules );
	if ( !pGameRules || pGameRules->GetGameState() != ASW_GS_INGAME )
	{
		Warning( "Cannot MarineFinishedMission when mission is not active.\n" );
		return;
	}

	CASW_Marine_Resource *pMR = pMarine->GetMarineResource();
	if ( pMR )
	{
		pMR->m_flFinishedMissionTime = gpGlobals->curtime - pGameRules->m_fMissionStartedTime;
	}
}

void CAlienSwarmProxy::OnMissionStart()
{
	CASW_Game_Resource *pGameResource = ASWGameResource();
	if ( const RD_Mission_t *pMission = ReactiveDropMissions::GetMission( STRING( gpGlobals->mapname ) ) )
	{
		if ( pMission->HasTag( "points" ) )
		{
			m_TotalPoints.Set( 0, this, this );

			for ( int i = 0; i < pGameResource->GetMaxMarineResources(); i++ )
			{
				CASW_Marine_Resource *pMR = pGameResource->GetMarineResource( i );
				if ( !pMR )
					continue;

				pMR->m_iScore = 0;
			}
		}
	}

	CAlienSwarm *pGameRules = ASWGameRules();
	m_OnDifficulty.Set( pGameRules->GetSkillLevel(), this, this );
	m_OnOnslaught.Set( pGameRules->IsOnslaught() ? 1 : 0, this, this );
	m_OnFriendlyFire.Set( pGameRules->IsHardcoreFF() ? 1 : 0, this, this );
	m_OnChallenge.Set( AllocPooledString( rd_challenge.GetString() ), this, this );
	m_MissionDifficulty.Set( pGameRules->GetMissionDifficulty(), this, this );
}
#endif

class CStylinCamProxy : public CBaseEntity
{
public:
	DECLARE_CLASS( CStylinCamProxy, CBaseEntity );

	CStylinCamProxy();

	virtual void Precache();

#ifdef CLIENT_DLL
	DECLARE_CLIENTCLASS();
#else
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

	virtual int ShouldTransmit( const CCheckTransmitInfo *pInfo );
	virtual bool KeyValue( const char *szKeyName, const char *szValue );

	void InputShowStylinCam( inputdata_t &inputdata );
	void InputHideStylinCam( inputdata_t &inputdata );
	void InputShowCommanderFace( inputdata_t &inputdata );
	void InputHideCommanderFace( inputdata_t &inputdata );
#endif

	CNetworkString( m_szCommanderFace, 255 );
};

CStylinCamProxy::CStylinCamProxy()
{
	V_strcpy( m_szCommanderFace.GetForModify(), ASW_DEFAULT_COMMANDER_FACE );
}

void CStylinCamProxy::Precache()
{
	BaseClass::Precache();

	char szMaterialName[255];
	V_strcpy( szMaterialName, "vgui/" );
	V_strncat( szMaterialName, m_szCommanderFace.Get(), 255 );
	PrecacheMaterial( szMaterialName );
}

#ifdef CLIENT_DLL
IMPLEMENT_CLIENTCLASS_DT( CStylinCamProxy, DT_ASW_StylinCamProxy, CStylinCamProxy )
	RecvPropString( RECVINFO( m_szCommanderFace ) ),
END_RECV_TABLE()
#else
LINK_ENTITY_TO_CLASS( asw_stylincam, CStylinCamProxy );

BEGIN_DATADESC( CStylinCamProxy )
	DEFINE_INPUTFUNC( FIELD_VOID, "ShowStylinCam", InputShowStylinCam ),
	DEFINE_INPUTFUNC( FIELD_VOID, "HideStylinCam", InputHideStylinCam ),
	DEFINE_INPUTFUNC( FIELD_VOID, "ShowCommanderFace", InputShowCommanderFace ),
	DEFINE_INPUTFUNC( FIELD_VOID, "HideCommanderFace", InputHideCommanderFace ),
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CStylinCamProxy, DT_ASW_StylinCamProxy )
	SendPropString( SENDINFO( m_szCommanderFace ) ),
END_SEND_TABLE()

int CStylinCamProxy::ShouldTransmit( const CCheckTransmitInfo *pInfo )
{
	return FL_EDICT_ALWAYS;
}

bool CStylinCamProxy::KeyValue( const char* szKeyName, const char* szValue )
{
	if ( FStrEq( szKeyName, "CommanderFace" ) )
	{
		if ( !*szValue )
		{
			szValue = ASW_DEFAULT_COMMANDER_FACE;
		}

		V_strncpy( m_szCommanderFace.GetForModify(), szValue, 255 );

		return true;
	}

	return BaseClass::KeyValue( szKeyName, szValue );
}

void CStylinCamProxy::InputShowStylinCam( inputdata_t &inputdata )
{
	if ( !ASWGameRules() )
		return;

	ASWGameRules()->m_bForceStylinCam = true;
}

void CStylinCamProxy::InputHideStylinCam( inputdata_t &inputdata )
{
	if ( !ASWGameRules() )
		return;

	ASWGameRules()->m_bForceStylinCam = false;
}

void CStylinCamProxy::InputShowCommanderFace( inputdata_t &inputdata )
{
	if ( !ASWGameRules() )
		return;

	ASWGameRules()->m_hCurrentStylinCam = this;
	ASWGameRules()->m_bShowCommanderFace = true;
}

void CStylinCamProxy::InputHideCommanderFace( inputdata_t &inputdata )
{
	if ( !ASWGameRules() )
		return;

	ASWGameRules()->m_bShowCommanderFace = false;
}
#endif

const char *CAlienSwarm::GetCommanderFace()
{
	CStylinCamProxy *pStylinCam = assert_cast<CStylinCamProxy *>( m_hCurrentStylinCam.Get() );
	if ( !pStylinCam )
	{
		return ASW_DEFAULT_COMMANDER_FACE;
	}

	return pStylinCam->m_szCommanderFace.Get();
}


// shared ammo definition
// JAY: Trying to make a more physical bullet response
#define BULLET_MASS_GRAINS_TO_LB(grains)	(0.002285*(grains)/16.0f)
#define BULLET_MASS_GRAINS_TO_KG(grains)	lbs2kg(BULLET_MASS_GRAINS_TO_LB(grains))

// exaggerate all of the forces, but use real numbers to keep them consistent
#define BULLET_IMPULSE_EXAGGERATION			3.5
// convert a velocity in ft/sec and a mass in grains to an impulse in kg in/s
#define BULLET_IMPULSE(grains, ftpersec)	((ftpersec)*12*BULLET_MASS_GRAINS_TO_KG(grains)*BULLET_IMPULSE_EXAGGERATION)

CAmmoDef *GetAmmoDef()
{
	static CAmmoDef def;
	static bool bInitted = false;

	if ( !bInitted )
	{
		bInitted = true;

		// HL2 based ammo types
		def.AddAmmoType( "AR2", DMG_BULLET, TRACER_LINE_AND_WHIZ, "sk_plr_dmg_ar2", "sk_npc_dmg_ar2", "sk_max_ar2", BULLET_IMPULSE( 200, 1225 ), 0 );
		def.AddAmmoType( "AR2G", DMG_DISSOLVE, TRACER_NONE, NULL, NULL, "sk_max_ar2_altfire", 0, 0 );
		def.AddAmmoType( "StriderMinigun", DMG_BULLET, TRACER_LINE, 5, 5, 15, 1.0 * 750 * 12, AMMO_FORCE_DROP_IF_CARRIED ); // hit like a 1.0kg weight at 750 ft/s
		def.AddAmmoType( "StriderMinigunDirect", DMG_BULLET, TRACER_LINE, 2, 2, 15, 1.0 * 750 * 12, AMMO_FORCE_DROP_IF_CARRIED ); // hit like a 1.0kg weight at 750 ft/s
		def.AddAmmoType( "CombineCannon", DMG_BULLET, TRACER_LINE, 3, 40, 0, 1.5 * 750 * 12, 0 ); // hit like a 1.5kg weight at 750 ft/s
		def.AddAmmoType( "HelicopterGun", DMG_BULLET, TRACER_LINE_AND_WHIZ, 3, 6, 225, BULLET_IMPULSE( 400, 1225 ), AMMO_FORCE_DROP_IF_CARRIED | AMMO_INTERPRET_PLRDAMAGE_AS_DAMAGE_TO_PLAYER );

		// asw ammo
		//				name				damagetype					tracertype				player dmg					npc damage					carry					physics force impulse		flags
		// rifle  DMG_BULLET
		def.AddAmmoType("ASW_R",			DMG_BULLET,					TRACER_LINE,	"sk_plr_dmg_asw_r",			"sk_npc_dmg_asw_r",			"sk_max_asw_r",			BULLET_IMPULSE(200, 1225),	0 );
		// rifle grenades
		def.AddAmmoType("ASW_R_G",			DMG_BLAST,					TRACER_NONE,	"sk_plr_dmg_asw_r_g",			"sk_npc_dmg_asw_r_g",			"sk_max_asw_r_g",			0,	0 );
		// autogun
		def.AddAmmoType("ASW_AG",			DMG_BULLET,					TRACER_LINE_AND_WHIZ,	"sk_plr_dmg_asw_ag",		"sk_npc_dmg_asw_ag",		"sk_max_asw_ag",		BULLET_IMPULSE(200, 1225),	0 );
		// shotgun
		def.AddAmmoType("ASW_SG",			DMG_BULLET | DMG_BUCKSHOT,	TRACER_LINE_AND_WHIZ,	"sk_plr_dmg_asw_sg",		"sk_npc_dmg_asw_sg",		"sk_max_asw_sg",		BULLET_IMPULSE(200, 1225),	0 );
		// shotgun (secondary)
		def.AddAmmoType("ASW_SG_G",			DMG_BULLET | DMG_BUCKSHOT,	TRACER_LINE_AND_WHIZ,	"sk_plr_dmg_asw_sg_g",		"sk_npc_dmg_asw_sg_g",		"sk_max_asw_sg_g",		BULLET_IMPULSE(200, 1225),	0 );
		// assault shotgun
		def.AddAmmoType("ASW_ASG",			DMG_BULLET | DMG_BUCKSHOT,	TRACER_LINE_AND_WHIZ,	"sk_plr_dmg_asw_asg",		"sk_npc_dmg_asw_asg",		"sk_max_asw_asg",		BULLET_IMPULSE(200, 1225),	0 );
		// flamer
		def.AddAmmoType("ASW_F",			DMG_BURN,					TRACER_LINE_AND_WHIZ,	"sk_plr_dmg_asw_f",			"sk_npc_dmg_asw_f",			"sk_max_asw_f",			BULLET_IMPULSE(200, 1225),	0 );
		// pistol
		def.AddAmmoType("ASW_P",			DMG_BULLET,					TRACER_LINE_AND_WHIZ,	"sk_plr_dmg_asw_p",			"sk_npc_dmg_asw_p",			"sk_max_asw_p",			BULLET_IMPULSE(200, 1225),	0 );
		// mining laser
		def.AddAmmoType("ASW_ML",			DMG_ENERGYBEAM,				TRACER_LINE_AND_WHIZ,	"sk_plr_dmg_asw_ml",		"sk_npc_dmg_asw_ml",		"sk_max_asw_ml",		BULLET_IMPULSE(200, 1225),	0 );
		// tesla gun - happy LJ?
		def.AddAmmoType("ASW_TG",			DMG_SHOCK,				TRACER_LINE_AND_WHIZ,	"sk_plr_dmg_asw_tg",		"sk_npc_dmg_asw_tg",		"sk_max_asw_tg",		BULLET_IMPULSE(200, 1225),	0 );
		// railgun
		def.AddAmmoType("ASW_RG",			DMG_SONIC,					TRACER_LINE_AND_WHIZ,	"sk_plr_dmg_asw_rg",		"sk_npc_dmg_asw_rg",		"sk_max_asw_rg",		BULLET_IMPULSE(200, 1225),	0 );
		// chainsaw
		def.AddAmmoType("ASW_CS",			DMG_SLASH,					TRACER_NONE,			"sk_plr_dmg_asw_cs",		"sk_npc_dmg_asw_cs",		"sk_max_asw_cs",		BULLET_IMPULSE(200, 1225),	0 );
		// flares
		def.AddAmmoType("ASW_FLARES",		DMG_SONIC,					TRACER_LINE_AND_WHIZ,	"sk_plr_dmg_asw_flares",	"sk_npc_dmg_asw_flares",	"sk_max_asw_flares",	BULLET_IMPULSE(200, 1225),	0 );
		// medkit
		def.AddAmmoType("ASW_MEDKIT",		DMG_SONIC,					TRACER_LINE_AND_WHIZ,	"sk_plr_dmg_asw_medkit",	"sk_npc_dmg_asw_medkit",	"sk_max_asw_medkit",	BULLET_IMPULSE(200, 1225),	0 );
		// med satchel
		def.AddAmmoType("ASW_MEDSAT",		DMG_SONIC,					TRACER_LINE_AND_WHIZ,	"sk_plr_dmg_asw_medsat",	"sk_npc_dmg_asw_medsat",	"sk_max_asw_medsat",	BULLET_IMPULSE(200, 1225),	0 );
		// med satchel self heal
		def.AddAmmoType("ASW_MEDSELF",		DMG_SONIC,					TRACER_LINE_AND_WHIZ,	"sk_plr_dmg_asw_medself",	"sk_npc_dmg_asw_medself",	"sk_max_asw_medself",	BULLET_IMPULSE(200, 1225),	0 );
		// stim
		def.AddAmmoType("ASW_STIM",		DMG_SONIC,						TRACER_LINE_AND_WHIZ,	"sk_plr_dmg_asw_stim",	"sk_npc_dmg_asw_stim",	"sk_max_asw_stim",	BULLET_IMPULSE(200, 1225),	0 );
		// welder
		def.AddAmmoType("ASW_WELDER",		DMG_SONIC,					TRACER_LINE_AND_WHIZ,	"sk_plr_dmg_asw_welder",	"sk_npc_dmg_asw_welder",	"sk_max_asw_welder",	BULLET_IMPULSE(200, 1225),	0 );
		// fire extinguisher
		def.AddAmmoType("ASW_EXT",			DMG_SONIC,					TRACER_NONE,	"sk_plr_dmg_asw_ext",			"sk_npc_dmg_asw_ext",			"sk_max_asw_ext",		BULLET_IMPULSE(200, 1225),	0 );
		// mines
		def.AddAmmoType("ASW_MINES",		DMG_BURN,					TRACER_NONE,	"sk_plr_dmg_asw_mines",			"sk_npc_dmg_asw_mines",			"sk_max_asw_mines",		BULLET_IMPULSE(200, 1225),	0 );
		// vindicator grenades
		def.AddAmmoType("ASW_ASG_G",		DMG_BURN,					TRACER_NONE,	"sk_plr_dmg_asw_r_g",			"sk_npc_dmg_asw_r_g",			"sk_max_asw_r_g",			0,	0 );
		// PDW
		def.AddAmmoType("ASW_PDW",			DMG_BULLET,					TRACER_LINE_AND_WHIZ,	"sk_plr_dmg_asw_pdw",			"sk_npc_dmg_asw_pdw",			"sk_max_asw_pdw",		BULLET_IMPULSE(200, 1225),	0 );
		// Hand Grenades
		def.AddAmmoType("ASW_HG",			DMG_BLAST,					TRACER_NONE,	"sk_npc_dmg_asw_hg",			"sk_npc_dmg_asw_hg",			"sk_max_asw_hg",		BULLET_IMPULSE(200, 1225),	0 );
		// Grenade launcher
		def.AddAmmoType("ASW_GL",			DMG_BLAST,					TRACER_NONE,	"sk_npc_dmg_asw_gl",			"sk_npc_dmg_asw_gl",			"sk_max_asw_gl",		BULLET_IMPULSE(200, 1225),	0 );
		// Sniper Rifle
		def.AddAmmoType("ASW_SNIPER",		DMG_BULLET,					TRACER_LINE_AND_WHIZ,	"sk_npc_dmg_asw_sniper",		"sk_npc_dmg_asw_sniper",			"sk_max_asw_sniper",		BULLET_IMPULSE(200, 1225),	0 );
		// desert eagle
		def.AddAmmoType("ASW_DEAGLE",		DMG_BULLET,					TRACER_LINE_AND_WHIZ,	"sk_plr_dmg_asw_deagle",		"sk_npc_dmg_asw_deagle",			"sk_max_asw_deagle",		BULLET_IMPULSE(200, 1225),	0 );
		// devastator (automated heavy shotgun)
		def.AddAmmoType( "ASW_DEVASTATOR",	DMG_BULLET | DMG_BUCKSHOT,	TRACER_LINE_AND_WHIZ,	"sk_plr_dmg_asw_devastator",	"sk_npc_dmg_asw_devastator",		"sk_max_asw_devastator",	BULLET_IMPULSE(200, 1225), 0);
		// 
		def.AddAmmoType( "ASW_50CALMG",		DMG_BULLET,					TRACER_LINE_AND_WHIZ,	"sk_plr_dmg_asw_50calmg",		"sk_npc_dmg_asw_50calmg",			"sk_max_asw_50calmg",		BULLET_IMPULSE(200, 1225),	0 );
		// gas_grenades
		def.AddAmmoType( "ASW_GAS_GRENADES",DMG_NERVEGAS,					TRACER_LINE_AND_WHIZ,	"sk_plr_dmg_asw_gas_grenades", "sk_npc_dmg_asw_gas_grenades",		"sk_max_asw_gas_grenades",	BULLET_IMPULSE( 200, 1225 ), 0 );
		// heavy rifle
		def.AddAmmoType( "ASW_HR",			DMG_BULLET,					TRACER_LINE,	"sk_plr_dmg_asw_hr",			"sk_npc_dmg_asw_hr",			"sk_max_asw_hr",			BULLET_IMPULSE(200, 1225),	0 );
		// heavy rifle secondary
		def.AddAmmoType( "ASW_HR_G",		DMG_SONIC,					TRACER_LINE_AND_WHIZ,	"sk_plr_dmg_asw_hr_g",	"sk_npc_dmg_asw_hr_g",	"sk_max_asw_hr_g",	BULLET_IMPULSE(200, 1225),	0 );
		// medrifle
		def.AddAmmoType("ASW_MEDRIFLE",		DMG_BULLET,					TRACER_LINE,	"sk_plr_dmg_asw_medrifle",			"sk_npc_dmg_asw_medrifle",			"sk_max_asw_medrifle",			BULLET_IMPULSE(200, 1225),	0 );
	}

	return &def;
}


#ifdef CLIENT_DLL

CAlienSwarm::CAlienSwarm()
{
	Msg("C_AlienSwarm created\n");

	if (ASWEquipmentList())
		ASWEquipmentList()->LoadTextures();

	m_nOldMarineForDeathCam = -1;
	m_fMarineDeathCamRealtime = 0.0f;
	m_nMarineDeathCamStep = 0;
	m_hMarineDeathRagdoll = NULL;
	m_fDeathCamYawAngleOffset = 0.0f;
	m_iPreviousGameState = 200;
	m_iPreviousMissionWorkshopID = 1; // impossible workshop ID

	engine->SetPitchScale( 1.0f );

	CVoiceStatus *pVoiceMgr = GetClientVoiceMgr();
	if ( pVoiceMgr )
	{
		pVoiceMgr->SetHeadLabelsDisabled( true );
	}
}

CAlienSwarm::~CAlienSwarm()
{
	RevertSavedConvars();

	Msg("C_AlienSwarm deleted\n");

	GameTimescale()->SetDesiredTimescale( 1.0f );
	engine->SetPitchScale( 1.0f );
}

float CAlienSwarm::GetMarineDeathCamInterp( bool bIgnoreCvar )
{
	if ( !m_bDeathCamSlowdown || ( !asw_marine_death_cam.GetBool() && !bIgnoreCvar && !ASWDeathmatchMode() ) || m_nMarineForDeathCam == -1 || m_fMarineDeathTime <= 0.0f )
		return 0.0f;

	bool bNewStep = false;

	if ( m_nOldMarineForDeathCam != m_nMarineForDeathCam )
	{
		m_nOldMarineForDeathCam = m_nMarineForDeathCam;
		m_hMarineDeathRagdoll = NULL;
		m_nMarineDeathCamStep = 0;
	}

	if ( m_nMarineDeathCamStep == 0 )
	{
		if ( gpGlobals->curtime > m_fMarineDeathTime + asw_time_scale_delay.GetFloat() )
		{
			m_nMarineDeathCamStep++;
			bNewStep = true;
		}
	}
	else if ( m_nMarineDeathCamStep == 1 )
	{
		if ( gpGlobals->curtime > m_fMarineDeathTime + asw_time_scale_delay.GetFloat() + asw_marine_death_cam_time.GetFloat() )
		{
			m_nMarineDeathCamStep++;
			bNewStep = true;
		}
	}

	if ( bNewStep )
	{
		m_fMarineDeathCamRealtime = gpGlobals->realtime;
	}

	C_ASW_Marine *pLocalMarine = C_ASW_Marine::GetViewMarine();
	bool bLocal = ( !pLocalMarine || !pLocalMarine->IsAlive() );

	float flHoldTime = ( bLocal ? asw_marine_death_cam_time_local_hold.GetFloat() : asw_marine_death_cam_hold.GetFloat() );

	if ( m_nMarineDeathCamStep == 1 )
	{
		return clamp( ( gpGlobals->realtime - m_fMarineDeathCamRealtime ) / asw_marine_death_cam_time_interp.GetFloat(), 0.001f, 1.0f );
	}
	else if ( m_nMarineDeathCamStep == 2 )
	{
		if ( gpGlobals->realtime > m_fMarineDeathCamRealtime + flHoldTime + asw_marine_death_cam_time_interp_out.GetFloat() )
		{
			m_nMarineDeathCamStep = 3;
		}

		return clamp( 1.0f - ( ( gpGlobals->realtime - ( m_fMarineDeathCamRealtime + flHoldTime ) ) / asw_marine_death_cam_time_interp_out.GetFloat() ), 0.001f, 1.0f );
	}
	else if ( m_nMarineDeathCamStep == 3 )
	{
		return 0.0f;
	}

	return 0.0f;
}

void CAlienSwarm::OnDataChanged( DataUpdateType_t updateType )
{
	bool bGameStateChanged = m_iPreviousGameState != GetGameState();
	if ( bGameStateChanged )
	{
		m_iPreviousGameState = GetGameState();

		IGameEvent * event = gameeventmanager->CreateEvent( "swarm_state_changed" );
		if ( event )
		{
			event->SetInt( "state", m_iPreviousGameState );
			gameeventmanager->FireEventClientSide( event );
		}

		if ( GetGameState() == ASW_GS_INGAME )
		{
			g_ReactiveDropWorkshop.OnMissionStart();
		}

		g_ReactiveDropWorkshop.CheckForRequiredAddons();

		g_RD_Rich_Presence.UpdatePresence();
	}
	if ( bGameStateChanged || m_iPreviousMissionWorkshopID != m_iMissionWorkshopID || updateType == DATA_UPDATE_CREATED )
	{
		m_iPreviousMissionWorkshopID = m_iMissionWorkshopID;

		UpdateMatchmakingTagsCallback( NULL, NULL, 0 );
	}
	if ( UTIL_RD_IsLobbyOwner() )
	{
		UTIL_RD_UpdateCurrentLobbyData( "system:rd_lobby_location", m_szApproximatePingLocation );
	}
}

#else

extern bool g_bAIDisabledByUser;
extern ConVar asw_springcol;
ConVar asw_blip_speech_chance( "asw_blip_speech_chance", "0.8", FCVAR_CHEAT, "Chance the tech marines will shout about movement on their scanner after a period of no activity" );
ConVar asw_instant_restart( "asw_instant_restart", "1", FCVAR_NONE, "Whether the game should use the instant restart (if not, it'll do a full reload of the map)." );
ConVar asw_instant_restart_debug( "asw_instant_restart_debug", "0", FCVAR_NONE, "Write a lot of developer messages to the console during an instant restart." );

const char * GenerateNewSaveGameName()
{
	static char szNewSaveName[256];	
	// count up save names until we find one that doesn't exist
	for (int i=1;i<10000;i++)
	{
		Q_snprintf(szNewSaveName, sizeof(szNewSaveName), "save/save%d.campaignsave", i);
		if (!filesystem->FileExists(szNewSaveName))
		{
			Q_snprintf(szNewSaveName, sizeof(szNewSaveName), "save%d.campaignsave", i);
			return szNewSaveName;
		}
	}

	return NULL;
}

const char* CAlienSwarm::GetGameDescription( void )
{ 
	return m_szGameDescription; 
}

CAlienSwarm::CAlienSwarm() : m_ActorSpeakingUntil( DefLessFunc( string_t ) )
{
	m_bShuttingDown = false;

	// fixes a memory leak on dedicated server where model vertex data
	// is not freed on map transition and remains locked, leading to increased
	// memory usage and cache trashing over time
	if ( engine->IsDedicatedServer() && rd_adjust_mod_dont_load_vertices.GetBool() )
	{
		ConVarRef mod_dont_load_vertices( "mod_dont_load_vertices" );
		mod_dont_load_vertices.SetValue( 1 );
	}

	V_strncpy( m_szGameDescription, "Alien Swarm: Reactive Drop", sizeof( m_szGameDescription ) );

	// create the profile list for the server
	//  clients do this is in c_asw_player.cpp
	MarineProfileList();

	ASWEquipmentList();

	// set which entities should stay around when we restart the mission
	m_MapResetFilter.AddKeepEntity( "worldspawn" );
	m_MapResetFilter.AddKeepEntity( "soundent" );
	m_MapResetFilter.AddKeepEntity( "asw_gamerules" );
	m_MapResetFilter.AddKeepEntity( "player" );
	m_MapResetFilter.AddKeepEntity( "asw_player" );
	m_MapResetFilter.AddKeepEntity( "player_manager" );
	m_MapResetFilter.AddKeepEntity( "predicted_viewmodel" );
	m_MapResetFilter.AddKeepEntity( "sdk_team_manager" );
	m_MapResetFilter.AddKeepEntity( "scene_manager" );
	m_MapResetFilter.AddKeepEntity( "event_queue_saveload_proxy" );
	m_MapResetFilter.AddKeepEntity( "ai_network" );
	m_MapResetFilter.AddKeepEntity( "ai_hint" );
	m_MapResetFilter.AddKeepEntity( "info_node" );
	m_MapResetFilter.AddKeepEntity( "info_hint" );
	m_MapResetFilter.AddKeepEntity( "info_node_hint" );
	m_MapResetFilter.AddKeepEntity( "info_node_air" );
	m_MapResetFilter.AddKeepEntity( "info_node_air_hint" );
	m_MapResetFilter.AddKeepEntity( "info_node_climb" );
	m_MapResetFilter.AddKeepEntity( "info_marine_hint" );
	m_MapResetFilter.AddKeepEntity( "info_node_marine_hint" );

	// riflemod: keep health regen entity all the time
	m_MapResetFilter.AddKeepEntity( "asw_health_regen" );

	FullReset();
}

void CAlienSwarm::FullReset()
{
	m_bStartedIntro = 0;
	m_iNumGrubs = 0;	// counts how many grubs have been spawned
	m_fVoteEndTime = 0.0f;
	m_flStimEndTime = 0.0f;
	m_flStimStartTime = 0.0f;
	m_fPreventStimMusicTime = 0.0f;
	m_bForceStylinCam = false;

	m_fMarineDeathTime = 0.0f;
	m_vMarineDeathPos = vec3_origin;
	m_vMarineDeathPosDeathmatch = vec3_origin;
	m_nMarineForDeathCam = -1;
	m_nMarineForDeathCamDeathmatch = -1;

	m_bMarineInvuln = false;

	SetGameState( ASW_GS_NONE );
	m_bMissionSuccess = false;
	m_bMissionFailed = false;
	m_fReserveMarinesEndTime = 0;

	m_nFailAdvice = ASW_FAIL_ADVICE_DEFAULT;

	m_fNextChatterTime = 0.0f;
	m_fNextIncomingChatterTime = 0.0f;
	m_fLastNoAmmoChatterTime = 0;
	m_fLastFireTime = 0.0f;
	m_fNextWWKillConv = random->RandomInt(asw_ww_chatter_interval_min.GetInt(), asw_ww_chatter_interval_max.GetInt());
	m_fNextCompliment = random->RandomInt(asw_compliment_chatter_interval_min.GetInt(), asw_compliment_chatter_interval_max.GetInt());
	m_bSargeAndJaeger = false;
	m_bWolfeAndWildcat = false;

	m_iMissionRestartCount = 0;
	m_bDoneCrashShieldbugConv = false;
	m_bShouldStartMission = false;

	m_fLastBlipSpeechTime = -200.0f;

	m_iSkillLevel = asw_skill.GetInt();
	OnSkillLevelChanged( m_iSkillLevel );

	m_pMissionManager = (CASW_Mission_Manager*) CreateEntityByName( "asw_mission_manager" );
	DispatchSpawn( m_pMissionManager );

	m_fVoteEndTime = 0;
	V_memset( m_szCurrentVoteDescription.GetForModify(), 0, sizeof( m_szCurrentVoteDescription ) );
	V_memset( m_szCurrentVoteMapName.GetForModify(), 0, sizeof( m_szCurrentVoteMapName ) );
	V_memset( m_szCurrentVoteCampaignName.GetForModify(), 0, sizeof( m_szCurrentVoteCampaignName ) );
	V_memset( m_szCycleNextMap.GetForModify(), 0, sizeof( m_szCycleNextMap ) );

	m_szCurrentVoteName[0] = '\0';
	m_iCurrentVoteYes = 0;
	m_iCurrentVoteNo = 0;
	m_iCurrentVoteType = (int) ASW_VOTE_NONE;

	m_hDebriefStats = NULL;

	m_fMissionStartedTime = 0;
	m_fBriefingStartedTime = 0;

	m_iForceReadyType = ASW_FR_NONE;
	m_fForceReadyTime = 0;
	m_iForceReadyCount = 0;
	m_iDeathmatchFinishCount = 0;
	m_fLaunchOutroMapTime = 0;
	m_bMissionRequiresTech = false;
	m_hEquipReq = NULL;
	m_fRemoveAliensTime = 0;

	m_fDeathmatchFinishTime = 0.0f;
	V_memset( m_szDeathmatchWinnerName.GetForModify(), 0, sizeof( m_szDeathmatchWinnerName ) );

	m_fNextLaunchingStep = 0;
	m_iMarinesSpawned = 0;
	m_pSpawningSpot = NULL;

	m_bIsTutorial = false;

	m_bCheckAllPlayersLeft = false;

	m_pMapBuilder = NULL;

	m_fLastPowerupDropTime = 0;
	m_flTechFailureRestartTime = 0.0f;

	m_bSentLeaderboardReady = false;

	m_bChallengeActiveThisCampaign = false;
	m_bChallengeActiveThisMission = false;

	m_iMissionWorkshopID = g_ReactiveDropWorkshop.FindAddonProvidingFile( CFmtStr( "resource/overviews/%s.txt", STRING( gpGlobals->mapname ) ) );
	EnableChallenge( rd_challenge.GetString() );

	m_bDeathCamSlowdown = asw_marine_death_cam_slowdown.GetBool();
	m_iOverrideAllowRotateCamera = rd_override_allow_rotate_camera.GetInt();

	V_memset( m_szApproximatePingLocation.GetForModify(), 0, sizeof( m_szApproximatePingLocation ) );
	m_bObtainedPingLocation = false;

	m_hBriefingCamera = NULL;
	m_bHadBriefingCamera = false;
	switch ( RandomInt( 0, 3 ) )
	{
	case 0:
		V_strncpy( m_szBriefingVideo.GetForModify(), "media/BGFX_01.bik", sizeof( m_szBriefingVideo ) );
		break;
	case 1:
		V_strncpy( m_szBriefingVideo.GetForModify(), "media/BGFX_02.bik", sizeof( m_szBriefingVideo ) );
		break;
	default:
	case 2:
		V_strncpy( m_szBriefingVideo.GetForModify(), "media/BGFX_03.bik", sizeof( m_szBriefingVideo ) );
		break;
	case 3:
		V_strncpy( m_szBriefingVideo.GetForModify(), "media/BG_04_FX.bik", sizeof( m_szBriefingVideo ) );
		break;
	}

	m_ActorSpeakingUntil.Purge();

	if ( !sv_cheats )
	{
		sv_cheats = cvar->FindVar( "sv_cheats" );
	}

	if ( sv_cheats && !sv_cheats->GetBool() )
	{
		if ( CAI_BaseNPC::m_nDebugBits & bits_debugDisableAI )
		{
			CAI_BaseNPC::m_nDebugBits &= ~bits_debugDisableAI;
			DevMsg( "AI Enabled.\n" );
			g_bAIDisabledByUser = false;
		}
	}
}

CAlienSwarm::~CAlienSwarm()
{
	RevertSavedConvars();
}

ConVar asw_reserve_marine_time("asw_reserve_marine_time", "30.0f", 0, "Number of seconds marines are reserved for at briefing start");

void CAlienSwarm::Precache( void )
{
	UTIL_PrecacheOther( "asw_marine" );

	PrecacheEffect( "ASWSpeech" );
	PrecacheEffect( "ASWBloodImpact" );
	PrecacheEffect( "DroneGib" );
	PrecacheEffect( "ParasiteGib" );
	PrecacheEffect( "ASWWelderSparks" );
	PrecacheEffect( "DroneBleed" );
	PrecacheEffect( "DroneGib" );
	PrecacheEffect( "HarvesterGib" );
	PrecacheEffect( "GrubGib" );
	PrecacheEffect( "ParasiteGib" );
	PrecacheEffect( "HarvesiteGib" );
	PrecacheEffect( "QueenSpitBurst" );
	PrecacheEffect( "EggGibs" );
	PrecacheEffect( "ElectroStun" );
	PrecacheEffect( "PierceSpark" );
	PrecacheEffect( "ExtinguisherCloud" );
	PrecacheEffect( "ASWTracer" );
	PrecacheEffect( "ASWUTracer" );
	PrecacheEffect( "ASWUTracerRG" );
	PrecacheEffect( "ASWUTraceless" );
	PrecacheEffect( "ASWUTracerUnattached" );
	PrecacheEffect( "aswwatersplash" );
	PrecacheEffect( "aswstunexplo" );
	PrecacheEffect( "ASWExplodeMap" );
	PrecacheEffect( "ASWAcidBurn" );
	PrecacheEffect( "ASWFireBurst" );
	PrecacheEffect( "aswcorpse" );
	PrecacheEffect( "QueenDie" );
	PrecacheEffect( "ASWWheelDust" );
	PrecacheEffect( "RailgunBeam" );
	PrecacheEffect( "ASWUTracerDual" );
	PrecacheEffect( "ASWUTracerDualLeft" );
	PrecacheEffect( "ASWUTracerDualRight" );

	BaseClass::Precache();
}

// spawns the marines needed for the tutorial and starts the mission
void CAlienSwarm::StartTutorial( CASW_Player *pPlayer )
{
	if ( !ASWGameResource() || !pPlayer )
		return;

	// disable onslaught and set skill level to normal
	asw_horde_override.SetValue( false );
	asw_wanderer_override.SetValue( false );
	asw_skill.SetValue( 2 );

	RosterDeselectAll( pPlayer );

	CASW_Marine_Resource *pMR;
	if ( CASW_TutorialStartPoint::GetTutorialSaveStage() == 0 )
	{
		RosterSelect( pPlayer, 0 ); // sarge

		pMR = ASWGameResource()->GetMarineResource( 0 );
		if ( pMR )
		{
			pMR->m_iWeaponsInSlots.Set( 0, -1 );
			pMR->m_iWeaponsInSlots.Set( 1, -1 );
			pMR->m_iWeaponsInSlots.Set( 2, -1 );
		}
	}
	else if ( CASW_TutorialStartPoint::GetTutorialSaveStage() == 1 )
	{
		RosterSelect( pPlayer, 6 ); // bastille

		pMR = ASWGameResource()->GetMarineResource( 0 );
		if ( pMR )
		{
			pMR->m_iWeaponsInSlots.Set( 0, -1 );
			pMR->m_iWeaponsInSlots.Set( 1, -1 );
			pMR->m_iWeaponsInSlots.Set( 2, -1 );
		}
	}
	else if ( CASW_TutorialStartPoint::GetTutorialSaveStage() == 2 )
	{
		RosterSelect( pPlayer, 3 ); // crash

		pMR = ASWGameResource()->GetMarineResource( 0 );
		if ( pMR )
		{
			pMR->m_iWeaponsInSlots.Set( 0, ASWEquipmentList()->GetIndexForSlot( 0, "asw_weapon_prifle" ) );
			pMR->m_iWeaponsInSlots.Set( 1, -1 );
			pMR->m_iWeaponsInSlots.Set( 2, -1 );
		}
	}
	else
	{
		RosterSelect( pPlayer, 3 ); // crash
		RosterSelect( pPlayer, 0 ); // sarge
		RosterSelect( pPlayer, 6 ); // bastille

		pMR = ASWGameResource()->GetMarineResource( 0 );
		if ( pMR )
		{
			pMR->m_iWeaponsInSlots.Set( 0, ASWEquipmentList()->GetIndexForSlot( 0, "asw_weapon_prifle" ) );
			pMR->m_iWeaponsInSlots.Set( 1, ASWEquipmentList()->GetIndexForSlot( 1, "asw_weapon_ammo_satchel" ) );
			pMR->m_iWeaponsInSlots.Set( 2, ASWEquipmentList()->GetIndexForSlot( 2, "asw_weapon_welder" ) );
		}

		pMR = ASWGameResource()->GetMarineResource( 1 );
		if ( pMR )
		{
			pMR->m_iWeaponsInSlots.Set( 0, ASWEquipmentList()->GetIndexForSlot( 0, "asw_weapon_vindicator" ) );
			pMR->m_iWeaponsInSlots.Set( 1, ASWEquipmentList()->GetIndexForSlot( 1, "asw_weapon_sentry" ) );
			pMR->m_iWeaponsInSlots.Set( 2, ASWEquipmentList()->GetIndexForSlot( 2, "asw_weapon_medkit" ) );
		}

		pMR = ASWGameResource()->GetMarineResource( 2 );
		if ( pMR )
		{
			pMR->m_iWeaponsInSlots.Set( 0, ASWEquipmentList()->GetIndexForSlot( 0, "asw_weapon_flamer" ) );
			pMR->m_iWeaponsInSlots.Set( 1, ASWEquipmentList()->GetIndexForSlot( 1, "asw_weapon_heal_grenade" ) );
			pMR->m_iWeaponsInSlots.Set( 2, ASWEquipmentList()->GetIndexForSlot( 2, "asw_weapon_flares" ) );
		}
	}

	StartMission();	// spawns marines and causes game state to go into ASW_GS_INGAME
}

void CAlienSwarm::ReserveMarines()
{
	// flag marines as reserved if a commander was using them last mission
	CASW_Game_Resource *pGameResource = ASWGameResource();
	if (!pGameResource || !GetCampaignSave())
		return;

	// don't do reserving in singleplayer
	if ( ASWGameResource() && ASWGameResource()->IsOfflineGame() )
		return;

	for (int i=0;i<ASW_NUM_MARINE_PROFILES;i++)
	{

		// if no-one was using this marine, skip it
		if ( ( Q_strlen( STRING( GetCampaignSave()->m_LastCommanders[i] ) ) <= 1 )
			|| !GetCampaignSave()->IsMarineAlive(i) )
			continue;
		Msg("reserving marine %d for %s\n", i, STRING(GetCampaignSave()->m_LastCommanders[i]));

		// someone was using it, so flag the marine as reserved
		if ( !pGameResource->IsRosterSelected( i ) )
			pGameResource->SetRosterSelected( i, 2 );
	}

	m_fReserveMarinesEndTime = gpGlobals->curtime + asw_reserve_marine_time.GetFloat();
}

void CAlienSwarm::UnreserveMarines()
{
	// flag marines as reserved if a commander was using them last mission
	CASW_Game_Resource *pGameResource = ASWGameResource();
	if (!pGameResource)
		return;

	for (int i=0;i<ASW_NUM_MARINE_PROFILES;i++)
	{
		// undo reserving of this marine
		if (pGameResource->IsRosterReserved(i))
			pGameResource->SetRosterSelected(i, 0);		
	}

	m_fReserveMarinesEndTime = 0;
}

void CAlienSwarm::AutoselectMarines(CASW_Player *pPlayer)
{
	if (!ASWGameResource() || !GetCampaignSave() || !pPlayer)
		return;

	char buffer[256];
	Q_snprintf(buffer, sizeof(buffer), "%s%s", pPlayer->GetPlayerName(), pPlayer->GetASWNetworkID());
	//Msg("checking autoselect for: %s\n", buffer);
	for ( int m = 0; m < ASWGameResource()->GetMaxMarineResources(); m++ )
	{
		for (int i=0;i<ASW_NUM_MARINE_PROFILES;i++)
		{
			if (!ASWGameResource()->IsRosterSelected(i))
			{
				if ( GetCampaignSave()->m_LastMarineResourceSlot[ i ] != m )
					continue;

				//Msg("checking %d: %s\n", i,STRING(GetCampaignSave()->m_LastCommanders[i]) );
				if (!Q_strcmp(buffer, STRING(GetCampaignSave()->m_LastCommanders[i])))
				{
					//Msg("this is a match, attempting autoselect\n");
					// check marine isn't wounded first
					bool bWounded = false;
					if (GetCampaignSave())
					{
						if (GetCampaignSave()->IsMarineWounded(i))
							bWounded = true;
					}
					//if (!bWounded)
					RosterSelect(pPlayer, i);
				}
			}
		}
	}
}

void CAlienSwarm::AllowBriefing()
{
	DevMsg( "Cheat allowing return to briefing\n" );
	SetGameState( ASW_GS_BRIEFING );
	mm_swarm_state.SetValue( "briefing" );
}


void CAlienSwarm::PlayerSpawn( CBasePlayer *pPlayer )
{
	BaseClass::PlayerSpawn(pPlayer);

	CASW_Player *pASWPlayer = ToASW_Player( pPlayer );

	if ( pASWPlayer->IsAnyBot() )
	{
		return;
	}

	// assign leader if there isn't one already
	if ( ASWGameResource() && ASWGameResource()->GetLeader() == NULL && pASWPlayer && pASWPlayer->CanBeLeader() )
	{
		ASWGameResource()->SetLeader( pASWPlayer );
	}

	if ( !pASWPlayer->IsSpectatorOnly() )
	{
		if ( ShouldQuickStart() )
		{
			StartTutorial( pASWPlayer );
		}
		else if ( IsTutorialMap() || engine->IsCreatingReslist() || engine->IsCreatingXboxReslist() )
		{
			StartTutorial( pASWPlayer );
		}
		else
		{
			AutoselectMarines( pASWPlayer );
		}
	}

	if ( !pASWPlayer->IsAnyBot() )
	{
		// BenLubar: Send saved replicated convars to the client so that it can reset them if the player disconnects during the mission.
		CSingleUserRecipientFilter filter( pPlayer );
		filter.MakeReliable();
		for ( int i = 0; i < m_SavedConvars.GetNumStrings(); i++ )
		{
			const char *pszCVarName = m_SavedConvars.String( i );
			if ( ConVarRef( pszCVarName ).IsFlagSet( FCVAR_REPLICATED ) )
			{
				UserMessageBegin( filter, "SavedConvar" );
				WRITE_STRING( pszCVarName );
				WRITE_STRING( STRING( m_SavedConvars[i] ) );
				MessageEnd();
			}
		}

		// ask Steam for our XP amounts
		// sometimes caused crashes for a mod
		if ( rd_request_experience.GetBool() )
			pASWPlayer->RequestExperience();
	}
}

bool CAlienSwarm::ClientConnected( edict_t *pEntity, const char *pszName, const char *pszAddress, char *reject, int maxrejectlen )
{	
#ifndef CLIENT_DLL
	// request a high resolution timer from the os
	if ( engine->IsDedicatedServer() )
	{
		// we have a client, apply the new timer resolution
		highres_timer_set( rd_dedicated_high_resolution_timer_ms.GetFloat() );
	}
#endif

	GetVoiceGameMgr()->ClientConnected( pEntity );

	CASW_Player *pPlayer = ToASW_Player(CBaseEntity::Instance( pEntity ));
	//Msg("ClientConnected, entindex is %d\n", pPlayer ? pPlayer->entindex() : -1);

	if (ASWGameResource())
	{
		int index = ENTINDEX(pEntity) - 1;
		if (index >= 0 && index < ASW_MAX_READY_PLAYERS)
		{
			ASWGameResource()->m_bPlayerReady.Set( index, false );
		}

		// if we have no leader
		if (ASWGameResource()->GetLeader() == NULL)
		{
			if (pPlayer)
				ASWGameResource()->SetLeader(pPlayer);
			//else
				//Msg("Failed to cast connected player\n");
		}
	}	

	return BaseClass::ClientConnected(pEntity, pszName, pszAddress, reject, maxrejectlen);
}

void CAlienSwarm::ClientDisconnected( edict_t *pClient )
{
	//Msg("CAlienSwarm::ClientDisconnected %d\n", pClient);
	if ( pClient )
	{
		CASW_Player *pPlayer = ToASW_Player(CBaseEntity::Instance( pClient ) );
		if ( pPlayer )
		{
			if ( ASWGameResource() )
			{
				for ( int i = 0; i < ASWGameResource()->GetMaxMarineResources(); i++ )
				{
					CASW_Marine_Resource *pMR = ASWGameResource()->GetMarineResource( i );
					if ( !pMR )
						continue;
					
					if ( pMR->GetCommander() == pPlayer )
					{
						pMR->SetInhabited( false );
					}
				}
			}
			//Msg("  This is an ASW_Player %d\n", pPlayer->entindex());
			RosterDeselectAll(pPlayer);

			// if they're leader, pick another leader
			if ( ASWGameResource() && ASWGameResource()->GetLeader() == pPlayer )
			{
				//Msg("  This is a leader disconnecting\n");
				ASWGameResource()->SetLeader(NULL);
				CASW_Game_Resource::s_bLeaderGivenDifficultySuggestion = false;

				int iPlayerEntIndex = pPlayer->entindex();
				
				CASW_Player *pBestPlayer = NULL;
				for ( int i = 0; i < ASW_MAX_READY_PLAYERS; i++ )
				{
					if ( i + 1 == iPlayerEntIndex )
						continue;

					// found a connected player?
					CASW_Player *pOtherPlayer = ToASW_Player( UTIL_PlayerByIndex( i + 1 ) );
					// if they're not connected, skip them
					if ( !pOtherPlayer || !pOtherPlayer->IsConnected() || !pOtherPlayer->CanBeLeader() )
						continue;

					if ( !pBestPlayer || pBestPlayer->m_bRequestedSpectator )
						pBestPlayer = pOtherPlayer;
				}

				if ( pBestPlayer )
				{
					ASWGameResource()->SetLeader( pBestPlayer );
				}
			}

			SetMaxMarines(pPlayer);

			// this function calls LeaveMarines which causes bugs for deathmatch
			// reassign marines owned by this player to someone else
			if ( rd_reassign_marines.GetBool() )
				ReassignMarines(pPlayer);

			SetLeaderVote(pPlayer, -1);
			SetKickVote(pPlayer, -1);
			RemoveVote(pPlayer);	// removes map vote

			// remove next campaign map vote
			if (GetCampaignSave())
				GetCampaignSave()->PlayerDisconnected(pPlayer);

			//UTIL_ClientPrintAll(ASW_HUD_PRINTTALKANDCONSOLE, "#asw_player_disco", pPlayer->GetPlayerName());
		}
	}
	BaseClass::ClientDisconnected(pClient);
	m_bCheckAllPlayersLeft = true;
}

bool CAlienSwarm::ShouldTimeoutClient( int nUserID, float flTimeSinceLastReceived )
{
	CASW_Player *pPlayer = static_cast<CASW_Player*>( UTIL_PlayerByUserId( nUserID ) );
	if ( !pPlayer || !pPlayer->IsConnected() || !pPlayer->HasFullyJoined() )
		return false;

	return ( sv_timeout_when_fully_connected.GetFloat() > 0.0f && flTimeSinceLastReceived > sv_timeout_when_fully_connected.GetFloat() );
}

static void PrintMaxMarinesCapError( CASW_Player *pPlayer, bool cappedByChallenge, int maxAllowedMarines )
{
	char buffer[16];
	Q_snprintf( buffer, sizeof( buffer ), "%i", maxAllowedMarines );
	if ( cappedByChallenge )
	{
		ClientPrint( pPlayer, HUD_PRINTTALK, "#rd_chat_marine_cap_reached_challenge", buffer );
	}
	else
	{
		ClientPrint( pPlayer, HUD_PRINTTALK, "#rd_chat_marine_cap_reached_server", buffer );
	}
}

bool CAlienSwarm::RosterSelect( CASW_Player *pPlayer, int RosterIndex, int nPreferredSlot )
{
	// for deathmatch allow marine selection any time 
	// if coop and game state is not BRIEFING, then return
	if ( !ASWGameResource() )
		return false;

	CASW_Game_Resource &rGameResource = *ASWGameResource();

	// BenLubar(deathmatch-improvements): used with deathmatch mode bot 
	// spawning to make it work (-2 is "spawn as me", -1 is "spawn in any slot")
	bool bForceInhabited = false;
	if ( nPreferredSlot == -2 )
	{
		nPreferredSlot = -1;
		bForceInhabited = true;
	}

	DevMsg("CAlienSwarm::RosterSelect( %d, %d) \n", RosterIndex, nPreferredSlot);

	if ( !ASWDeathmatchMode() && GetGameState() != ASW_GS_BRIEFING )
		return false;

	if ( RosterIndex < 0 || RosterIndex >= ASW_NUM_MARINE_PROFILES )
	{
		return false;
	}

	if ( rGameResource.m_iNumMarinesSelected >= rGameResource.m_iMaxMarines )		// too many selected?
	{
		if ( nPreferredSlot == -1 )
		{
			PrintMaxMarinesCapError( pPlayer, HaveSavedConvar( ConVarRef( "rd_max_marines" ) ), rGameResource.m_iMaxMarines );
			return false;
		}
		
		CASW_Marine_Resource *pExisting = rGameResource.GetMarineResource( nPreferredSlot );		// if we're not swapping out for another then abort
		// if there is no this check then players can use console command cl_selectm 5 5 to select more than rd_max_marines
		if ( !pExisting )
		{
			PrintMaxMarinesCapError( pPlayer, HaveSavedConvar( ConVarRef( "rd_max_marines" ) ), rGameResource.m_iMaxMarines );
			return false;
		}

		if ( pExisting && pExisting->GetCommander() != pPlayer )
		{
			PrintMaxMarinesCapError( pPlayer, HaveSavedConvar( ConVarRef( "rd_max_marines" ) ), rGameResource.m_iMaxMarines );
			return false;
		}
	}

	// one marine each?
	if (!IsTutorialMap() && (!rd_player_bots_allowed.GetBool() || ASWGameResource()->m_bOneMarineEach) && ASWGameResource()->GetNumMarines(pPlayer) > 0)
	{
		if ( nPreferredSlot == -1 )
		{
			if ( !rd_player_bots_allowed.GetBool() )
				ClientPrint( pPlayer, HUD_PRINTTALK, "#rd_no_bots_allowed" );

			return false;
		}

		CASW_Marine_Resource *pExisting = ASWGameResource()->GetMarineResource( nPreferredSlot );		// if we're not swapping out for another then abort
		if ( pExisting && pExisting->GetCommander() != pPlayer )
		{
			return false;
		}
		else if ( !rd_player_bots_allowed.GetBool() && !pExisting )
		{
			ClientPrint( pPlayer, HUD_PRINTTALK, "#rd_no_bots_allowed" );
			return false;
		}
	}

	// don't select a dead man
	// reactivedrop: just comment this because we don't use it
//	bool bDead = false;	
// 	if (ASWGameResource()->IsCampaignGame())
// 	{
// 		CASW_Campaign_Save *pSave = ASWGameResource()->GetCampaignSave();
// 		if (pSave)
// 		{
// 			if (!pSave->IsMarineAlive(RosterIndex))
// 			{
// 				bDead = true;
// 				return false;
// 			}
// 		}
// 	}

	// always allow to select every marine for deathmatch
	// also allow for coop if not already selected
	if (ASWDeathmatchMode() || !ASWGameResource()->IsRosterSelected(RosterIndex))
	{
		bool bCanSelect = true;
		// check we're allowed to take this marine, if he's reserved
		// it is unused in deathmatch
		if ( !ASWDeathmatchMode() )
		{
			if (ASWGameResource()->IsRosterReserved(RosterIndex) && GetCampaignSave() && RosterIndex>=0 && RosterIndex <ASW_NUM_MARINE_PROFILES)
			{
				if (pPlayer)
				{
					char buffer[256];
					Q_snprintf(buffer, sizeof(buffer), "%s%s", pPlayer->GetPlayerName(), pPlayer->GetASWNetworkID());
					if (Q_strcmp(buffer, STRING(GetCampaignSave()->m_LastCommanders[RosterIndex])))
					{
						Q_snprintf( buffer, sizeof( buffer ), "%i", int( m_fReserveMarinesEndTime - gpGlobals->curtime ) );
						ClientPrint( pPlayer, HUD_PRINTTALK, "#rd_chat_marine_reserved", buffer );
						bCanSelect = false;
					}
				}
			}
			// check this marine isn't already selected by someone else
			for (int i=0;i<ASWGameResource()->GetMaxMarineResources();i++)
			{
				CASW_Marine_Resource *pMR = ASWGameResource()->GetMarineResource(i);
				if (pMR && pMR->GetProfileIndex() == RosterIndex)
				{
					bCanSelect = false;
					break;
				}
			}
		}

		if (bCanSelect)
		{					
			CASW_Marine_Resource* m = (CASW_Marine_Resource*)CreateEntityByName("asw_marine_resource");
			m->SetCommander(pPlayer);
			m->SetProfileIndex(RosterIndex);
			if ( bForceInhabited )
			{
				m->SetInhabited( true );
				m->ChangeTeam( pPlayer->GetTeamNumber() );
			}
			else if ( ASWDeathmatchMode() && ASWDeathmatchMode()->IsTeamDeathmatchEnabled() )
			{
				m->ChangeTeam( ASWDeathmatchMode()->GetSmallestTeamNumber() );
			}
			if ( ASWEquipmentList() )
			{
				for ( int iWpnSlot = 0; iWpnSlot < ASW_MAX_EQUIP_SLOTS; ++ iWpnSlot )
				{
					const char *szWeaponClass = m->GetProfile()->m_DefaultWeaponsInSlots[ iWpnSlot ];
					int nWeaponIndex = ASWEquipmentList()->GetIndexForSlot( iWpnSlot, szWeaponClass );
					if ( nWeaponIndex < 0 )		// if there's a bad weapon here, then fall back to one of the starting weapons
					{
						if ( iWpnSlot == 2 )
						{
							nWeaponIndex = ASWEquipmentList()->GetIndexForSlot( iWpnSlot, "asw_weapon_medkit" );
						}
						else
						{
							nWeaponIndex = ASWEquipmentList()->GetIndexForSlot( iWpnSlot, "asw_weapon_rifle" );
						}
					}
					nWeaponIndex = ApplyWeaponSelectionRules( iWpnSlot, nWeaponIndex );

					if ( nWeaponIndex >= 0 )
					{
						m->m_iWeaponsInSlots.Set( iWpnSlot, nWeaponIndex );

						// store also in initial array to disallow marines spawn 
						// with picked up items 
						if ( ASWDeathmatchMode() )
						{
							m->m_iInitialWeaponsInSlots[iWpnSlot] = nWeaponIndex;
						}
					}
					else
					{
						Warning( "Bad default weapon for %s in slot %d\n", m->GetProfile()->GetShortName(), iWpnSlot );
					}
				}
			}
			m->Spawn();	// asw needed?
			if ( !ASWGameResource()->AddMarineResource( m, nPreferredSlot ) )
			{
				UTIL_Remove( m );
				return false;
			}

			ASWGameResource()->SetRosterSelected(RosterIndex, 1);			// select the marine

			return true;
		}
	}
	Warning("Something failed, returning false \n");
	return false;
}

void CAlienSwarm::RosterDeselect( CASW_Player *pPlayer, int RosterIndex)
{
	if (!ASWGameResource())
		return;

	if ( !ASWDeathmatchMode() )
	{
		// only allow roster deselection during briefing
		if (GetGameState() != ASW_GS_BRIEFING)
			return;

		if (!ASWGameResource()->IsRosterSelected(RosterIndex))		// if not already selected
			return;
	}

	// check if this marine is selected by this player
	for (int i=0;i<ASWGameResource()->GetMaxMarineResources();i++)
	{
		CASW_Marine_Resource* pMR = ASWGameResource()->GetMarineResource(i);
		if (pMR && pMR->GetCommander() == pPlayer && pMR->GetProfileIndex() == RosterIndex)
		{
			// if controls marine kill the marine first before choosing new
			if ( ASWDeathmatchMode() && pPlayer->GetNPC() && pPlayer->GetNPC()->GetHealth() > 0 ) 
			{
				pPlayer->GetNPC()->Suicide();
			}
			ASWGameResource()->SetRosterSelected(RosterIndex, 0);
			ASWGameResource()->DeleteMarineResource(pMR);
			return;
		}
	}
}

void CAlienSwarm::RosterDeselectAll( CASW_Player *pPlayer )
{
	if (!ASWGameResource() || !pPlayer)
		return;

	//Msg("  RosterDeselectAll\n");
	// check if this marine is selected by this player
	int m = ASWGameResource()->GetMaxMarineResources();
	for (int i=m-1;i>=0;i--)
	{
		CASW_Marine_Resource* pMR = ASWGameResource()->GetMarineResource(i);
		if (pMR && pMR->GetCommander() == pPlayer)
		{
			// commented to allow marine resource deleting during game
			if (GetGameState() == ASW_GS_BRIEFING || ASWDeathmatchMode() )
			{
				//Msg("Roster deselecting %d\n", pMR->GetProfileIndex());
				ASWGameResource()->SetRosterSelected(pMR->GetProfileIndex(), 0);
				if ( ( ASWDeathmatchMode() || IsTutorialMap() ) && pMR->GetMarineEntity() )
				{
					UTIL_Remove(pMR->GetMarineEntity());
				}
				ASWGameResource()->DeleteMarineResource(pMR);
			}
		}
	}
}

void CAlienSwarm::ReassignMarines(CASW_Player *pPlayer)
{
	if (!ASWGameResource() || !pPlayer)
		return;

	CASW_Player *pNewCommander = NULL;
	int numMarinesByNewCommander = 0;
	// firstly try to find a player who has the biggest number of bots assigned
	// or a player who has at least one marine assigned
	for ( int k = 1; k <= gpGlobals->maxClients; ++k )
	{
		CASW_Player *pTmpPlayer = ToASW_Player( UTIL_PlayerByIndex( k ) );
		if ( pTmpPlayer == pPlayer || !pTmpPlayer || !pTmpPlayer->IsConnected() || !pTmpPlayer->GetNPC() )
			continue;

		int numMarines = 0;
		for ( int i = 0; i < ASWGameResource()->GetMaxMarineResources(); ++i )
		{
			CASW_Marine_Resource* pMR = ASWGameResource()->GetMarineResource( i );
			
			if ( pMR && pMR->GetCommander() == pTmpPlayer )
			{
				++numMarines;
			}
		}
		if ( numMarines > numMarinesByNewCommander )
		{
			pNewCommander = pTmpPlayer;
			numMarinesByNewCommander = numMarines;
		}
	}
	// if nobody has any marines(most likely singe player)
	// assign bots to the first found valid player
	if ( !pNewCommander )
	{
		for ( int k = 1; k <= gpGlobals->maxClients; ++k )
		{
			pNewCommander = ToASW_Player( UTIL_PlayerByIndex( k ) );
			if ( pNewCommander && !pNewCommander->IsAnyBot() )
				break;
		}
	}

	// make sure he's not inhabiting any of them
	pPlayer->LeaveMarines();
	int m = ASWGameResource()->GetMaxMarineResources();
	for (int i=m-1;i>=0;i--)
	{
		CASW_Marine_Resource* pMR = ASWGameResource()->GetMarineResource(i);
		if (pMR && pMR->GetCommander() == pPlayer)
		{
			// sets the marine's commander to the other player we found, or no-one
			pMR->SetCommander( pNewCommander );
			CASW_Marine *pMarine = pMR->GetMarineEntity();
			if ( pMarine && pNewCommander )
			{
				pMarine->SetCommander( pNewCommander );
				pMarine->OrdersFromPlayer( pNewCommander, ASW_ORDER_FOLLOW, pNewCommander->GetNPC(), false );
			}
		}
	}
}

void CAlienSwarm::SetMaxMarines( CASW_Player *pException )
{
	// allow this method working during game for deathmatch
	if ( !ASWDeathmatchMode() && GetGameState() != ASW_GS_BRIEFING )
		return;

	CASW_Game_Resource *pGameResource = ASWGameResource();

	if ( !pGameResource )
		return;

	if ( ASWDeathmatchMode() )
	{
		pGameResource->SetMaxMarines( ASW_MAX_MARINE_RESOURCES, false );
	}
	else if ( rd_max_marines.GetInt() > 0 )
	{
		pGameResource->SetMaxMarines( rd_max_marines.GetInt(), false );
	}
	else if ( gpGlobals->maxClients == 1 )
	{
		if ( pGameResource->IsOfflineGame() )
			pGameResource->SetMaxMarines( mm_max_players.GetInt(), false );
		else
			pGameResource->SetMaxMarines( 4, false );
	}
	else
	{
		pGameResource->SetMaxMarines( gpGlobals->maxClients, false );
	}
}

// 1 marine each 'fair rules' have been turned on, make sure no players have 2 selected
void CAlienSwarm::EnforceFairMarineRules()
{
	CASW_Game_Resource *pGameResource = ASWGameResource();
	if (!pGameResource)
		return;
	for ( int i = 1; i <= gpGlobals->maxClients; i++ )	
	{
		CASW_Player* pOtherPlayer = ToASW_Player(UTIL_PlayerByIndex(i));

		if ( pOtherPlayer && pOtherPlayer->IsConnected() )
		{
			int k = 0;
			while (pGameResource->GetNumMarines(pOtherPlayer) > 1 && k < 255)
			{
				pGameResource->RemoveAMarineFor(pOtherPlayer);
				k++;
			}
		}
	}
}

// if players have more marines selected than the current game mode allows
// deselect marines
void CAlienSwarm::EnforceMaxMarines()
{
	CASW_Game_Resource *pGameResource = ASWGameResource();
	if (!pGameResource)
		return;
	// deselect excessive marines
	int nNumSelectedMarines = 0;
	int nMaxMarines = pGameResource->m_iMaxMarines;
	CUtlVector<CASW_Marine_Resource*> marineResourcesToDelete;
	// going in descending order because DeleteMarineResource() shuffles end array elements
	for ( int i = 0; i < ASW_MAX_MARINE_RESOURCES; ++i )
	{
		CASW_Marine_Resource* pMR = ASWGameResource()->GetMarineResource(i);
		for (int RosterIndex = 0; pMR && RosterIndex < ASW_NUM_MARINE_PROFILES; ++RosterIndex)
		{
			if ( pMR->GetProfileIndex() == RosterIndex )
			{
				++nNumSelectedMarines;
				DevMsg("Found marine resource with index %i, nNumSelectedMarines=%i\n", RosterIndex, nNumSelectedMarines);
				if (nNumSelectedMarines > nMaxMarines)
				{
					// we don't delete elements here because 
					// DeleteMarineResource() shuffles array elements
					DevMsg("Marking marine %i for deselection\n", RosterIndex);
					marineResourcesToDelete.AddToTail(pMR);
				}
				break;
			}
		}
	}
	// the actual deletion
	FOR_EACH_VEC(marineResourcesToDelete, it)
	{
		ASWGameResource()->SetRosterSelected(marineResourcesToDelete[it]->GetProfileIndex(), 0);
		ASWGameResource()->DeleteMarineResource(marineResourcesToDelete[it]);
	}
}

void CAlienSwarm::ReviveDeadMarines()
{
	if (GetCampaignSave())
	{
		for (int i=0;i<ASW_NUM_MARINE_PROFILES;i++)
		{
			GetCampaignSave()->ReviveMarine(i);
		}
		//GetCampaignSave()->SaveGameToFile();
		if (ASWGameResource())
			ASWGameResource()->UpdateMarineSkills(GetCampaignSave());
		UTIL_ClientPrintAll( ASW_HUD_PRINTTALKANDCONSOLE, "#asw_marines_revived" );
	}	
}

void CAlienSwarm::LoadoutSelect( CASW_Player *pPlayer, int iRosterIndex, int iInvSlot, int iEquipIndex)
{
	if (!ASWGameResource())
		return;

	if ( iInvSlot < 0 || iInvSlot >= ASW_MAX_EQUIP_SLOTS )
		return;

	// find the appropriate marine resource
	int iMarineIndex=-1;
	for (int i=0;i<ASWGameResource()->GetMaxMarineResources();i++)
	{
		CASW_Marine_Resource *pMR = ASWGameResource()->GetMarineResource(i);
		if (!pMR)
			continue;

		if (pMR->GetProfileIndex() == iRosterIndex && pMR->GetCommander() == pPlayer)
		{
			iMarineIndex = i;
			break;
		}
	}
	if (iMarineIndex == -1)
		return;

	CASW_Marine_Resource* pMarineResource = ASWGameResource()->GetMarineResource(iMarineIndex);
	if (!pMarineResource)
		return;

	// reactivedrop: check whether this weapon is allowed, if not, returns an ID of alternative
	iEquipIndex = ApplyWeaponSelectionRules( iInvSlot, iEquipIndex );

	// Figure out what item the marine is trying to equip
	CASW_EquipItem *pNewItem = ASWEquipmentList()->GetItemForSlot( iInvSlot, iEquipIndex );
	if ( !pNewItem || ( !pNewItem->m_bSelectableInBriefing && !rd_weapons_show_hidden.GetBool() ) )
		return;

	// Figure out if the marine is already carrying an item in the slot
	CASW_EquipItem *pOldItem = ASWEquipmentList()->GetItemForSlot( iInvSlot, pMarineResource->m_iWeaponsInSlots.Get( iInvSlot ) );
	// Can swap the old item for new one?
	if ( !MarineCanSelectInLobby( pMarineResource,
		pNewItem ? STRING(pNewItem->m_EquipClass) : NULL,
		pOldItem ? STRING(pOldItem->m_EquipClass) : NULL ) )
		return;

	pMarineResource->m_iWeaponsInSlots.Set( iInvSlot, iEquipIndex );

	if ( ASWDeathmatchMode() )
	{
		pMarineResource->m_iInitialWeaponsInSlots[iInvSlot] = iEquipIndex;
	}

	if ( ASWHoldoutMode() )
	{
		ASWHoldoutMode()->LoadoutSelect( pMarineResource, iEquipIndex, iInvSlot );
	}
}

// a player wants to start the mission
//  flag it so we can trigger the start in our next think
void CAlienSwarm::RequestStartMission(CASW_Player *pPlayer)
{
	// check we actually have some marines selected before starting
	CASW_Game_Resource *pGameResource = ASWGameResource();
	if (!pGameResource)
		return;
	int m = pGameResource->GetMaxMarineResources();	
	bool bCanStart = false;
	bool bTech = false;
	for (int i=0;i<m;i++)
	{
		if (pGameResource->GetMarineResource(i))
		{
			bCanStart = true;

			// check for a tech
			if (pGameResource->GetMarineResource(i)->GetProfile() && pGameResource->GetMarineResource(i)->GetProfile()->CanHack())
				bTech = true;
		}
	}
	if (!bCanStart)
		return;
	if ( MissionRequiresTech() && !bTech)
		return;
	if (m_hEquipReq.Get() && !m_hEquipReq->AreRequirementsMet())
		return;
	
	if (ASWGameResource()->AreAllOtherPlayersReady(pPlayer->entindex()))
	{
		m_bShouldStartMission = true;
	}
}

void CAlienSwarm::StartMission()
{	
	if (m_iGameState != ASW_GS_BRIEFING)
		return;

	SetForceReady(ASW_FR_NONE);
	
	// check we actually have some marines selected before starting
	CASW_Game_Resource *pGameResource = ASWGameResource();
	if ( !pGameResource )
		return;

	int iMaxMarineResources = pGameResource->GetMaxMarineResources();	
	bool bCanStart = false;	
	if ( ASWDeathmatchMode() )
		bCanStart = true;		// allow start without marines selected
	bool bTech = false;
	bool bMedic = false;
	for ( int i = 0; i < iMaxMarineResources; i++ )
	{
		CASW_Marine_Resource *pMR = pGameResource->GetMarineResource( i );
		if ( pMR )
		{
			bCanStart = true;

			// check for a tech
			if ( pMR->GetProfile() && pMR->GetProfile()->CanHack() )
			{
				bTech = true;
			}

			// check for a medic
			if ( pMR->GetProfile() && pMR->GetProfile()->CanUseFirstAid() )
			{
				bMedic = true;
			}

			pMR->m_TimelineFriendlyFire.ClearAndStart();
			pMR->m_TimelineKillsTotal.ClearAndStart();
			pMR->m_TimelineHealth.ClearAndStart();
			pMR->m_TimelineAmmo.ClearAndStart();
			pMR->m_TimelinePosX.ClearAndStart();
			pMR->m_TimelinePosY.ClearAndStart();
		}
	}
	if (!bCanStart)
		return;
	if ( MissionRequiresTech() && !bTech )
		return;
	if (m_hEquipReq.Get() && !m_hEquipReq->AreRequirementsMet())
		return;

	// Calling OnSkillLevelChanged() here to properly update m_iMissionDifficulty and thus aliens' health, damage and speed
	OnSkillLevelChanged( m_iSkillLevel );
	ApplyChallenge();

	// store our current leader (so we can keep the same leader after a map load)
	pGameResource->RememberLeaderID();

	if ( rd_add_bots.GetBool() )
	{
		// riflemod: add bots for missing slots 
		// find out the actual number of marines, if the number is less than 4 then
		// add marines for leader player 
		
		/* // skip this code, stupidly add 4 bots to leader 
		int num_selected_marines = 0;
		for (int i = 0; i < ASW_MAX_MARINE_RESOURCES; ++i)
		{
			CASW_Marine_Resource* pMR = ASWGameResource()->GetMarineResource(i);
			if (pMR)
				++num_selected_marines;
		}
		int num_bots_to_add = 4 - num_selected_marines;
		//*/
		CASW_Player *pLeader = ASWGameResource() ? ASWGameResource()->GetLeader() : NULL;
		int nPreferredSlot = -1; // we don't care which slot will it take 
		for (int i = 0; i < 4; ++i)
		{
			if ( RosterSelect( pLeader, i, nPreferredSlot ) )
			{
				// 0 is Sarge, select fire mines(11) for him
				if (0 == i)
					LoadoutSelect(pLeader, i, 2, 11);	// Sarge has asw_weapon_mines
				if (1 == i)
					LoadoutSelect(pLeader, i, 2,  6);   // Wildcat has asw_weapon_hornet_barrage
				if (2 == i)
					LoadoutSelect(pLeader, i, 2,  7);   // Faith has asw_weapon_freeze_grenades
				if (3 == i)
					LoadoutSelect(pLeader, i, 2, 10);   // Tech has asw_weapon_electrified_armor
			}
		}
		// end of riflemod code
	}

	// activate the level's ambient sounds
	StartAllAmbientSounds();

	// carnage mode?
	float flCarnage = rd_carnage_scale.GetFloat();

	if ( rd_alien_num_min_players.GetFloat() == rd_alien_num_max_players.GetFloat() )
	{
		// avoid division by 0
		flCarnage *= ASWGameResource()->m_iNumMarinesSelected > rd_alien_num_min_players.GetInt() ? rd_alien_num_max_scale.GetFloat() : rd_alien_num_min_scale.GetFloat();
	}
	else if ( ASWGameResource()->m_iNumMarinesSelected > rd_alien_num_min_players.GetInt() )
	{
		flCarnage *= Lerp( clamp( ( ASWGameResource()->m_iNumMarinesSelected - rd_alien_num_min_players.GetFloat() ) / ( rd_alien_num_max_players.GetFloat() - rd_alien_num_min_players.GetFloat() ), 0, 1 ), rd_alien_num_min_scale.GetFloat(), rd_alien_num_max_scale.GetFloat() );
	}
	if ( flCarnage > 1.0001 )
		ASW_ApplyCarnage_f( flCarnage );

	if ( rd_infinite_spawners.GetBool() )
		ASW_ApplyInfiniteSpawners_f();

	if ( !rd_slowmo.GetBool() )
	{
		int iCount = 0;
		CBaseEntity *ent = NULL;
		while ((ent = gEntList.NextEnt(ent)) != NULL)
		{
			if ( ent->GetClassname() != NULL && FStrEq( "env_slomo", ent->GetClassname() ) )
			{
				UTIL_Remove(ent);
				iCount++;
			}
		}
	}

	// increase num retries
	if ( CASW_Campaign_Save *pSave = GetCampaignSave() )
	{
		pSave->IncreaseRetries();
		pSave->UpdateLastCommanders();
		pSave->SaveGameToFile();
	}

	m_Medals.OnStartMission();

	if ( ASWDirector() )
	{
		ASWDirector()->OnMissionStarted();
	}

	Msg("==STARTMISSION==\n");

	// reset spawn point pointer for deathmatch
	CASW_Player::spawn_point = NULL;

	SetGameState(ASW_GS_LAUNCHING);
	mm_swarm_state.SetValue( "ingame" );
	m_fNextLaunchingStep = gpGlobals->curtime + ASW_LAUNCHING_STEP;

	// reset fail advice
	ASWFailAdvice()->OnMissionStart();

	if ( !bMedic )
	{
		ASWFailAdvice()->OnNoMedicStart();
	}

	if ( ASWHoldoutMode() )
	{
		ASWHoldoutMode()->OnMissionStart();
	}

	if ( ASWDeathmatchMode() )
	{
		ASWDeathmatchMode()->OnMissionStart();
	}

	g_ReactiveDropWorkshop.OnMissionStart();

	// reset various chatter timers
	CASW_Drone_Advanced::s_fNextTooCloseChatterTime = 0;
	CASW_Egg::s_fNextSpottedChatterTime = 0;
	CASW_Marine::s_fNextMadFiringChatter = 0;
	CASW_Marine::s_fNextIdleChatterTime = 0;
	CASW_Parasite::s_fNextSpottedChatterTime = 0;
	CASW_Parasite::s_fLastHarvesiteAttackSound = 0;	
	CASW_Shieldbug::s_fNextSpottedChatterTime = 0;
	CASW_Alien_Goo::s_fNextSpottedChatterTime = 0;
	CASW_Harvester::s_fNextSpawnSoundTime = 0;
	CASW_Harvester::s_fNextPainSoundTime = 0;
// 	CASW_Spawner::s_iFailedUberSpawns = 0;
// 	CASW_Spawner::s_iUberDronesSpawned = 0;
// 	CASW_Spawner::s_iNormalDronesSpawned = 0;
	m_fMissionStartedTime = gpGlobals->curtime;

	// check if certain marines are here for conversation triggering
	bool bSarge, bJaeger, bWildcat, bWolfe;
	bSarge = bJaeger = bWildcat = bWolfe = false;
	for ( int i = 0;i < iMaxMarineResources; i++ )
	{
		if (pGameResource->GetMarineResource(i) && pGameResource->GetMarineResource(i)->GetProfile())
		{
			if (pGameResource->GetMarineResource(i)->GetProfile()->m_VoiceType == ASW_VOICE_SARGE)
				bSarge = true;
			if (pGameResource->GetMarineResource(i)->GetProfile()->m_VoiceType == ASW_VOICE_JAEGER)
				bJaeger = true;
			if (pGameResource->GetMarineResource(i)->GetProfile()->m_VoiceType == ASW_VOICE_WILDCAT)
				bWildcat = true;
			if (pGameResource->GetMarineResource(i)->GetProfile()->m_VoiceType == ASW_VOICE_WOLFE)
				bWolfe = true;
		}
	}

	m_bSargeAndJaeger = bSarge && bJaeger;
	m_bWolfeAndWildcat = bWildcat && bWolfe;

	CASW_GameStats.Event_MissionStarted();

	// count eggs in map
	ASWGameResource()->m_iStartingEggsInMap = 0;
	CBaseEntity* pEntity = NULL;
	while ((pEntity = gEntList.FindEntityByClassname( pEntity, "asw_egg" )) != NULL)
	{
		ASWGameResource()->m_iStartingEggsInMap++;
	}

	AddBonusChargesToPickups();
	if( g_pScriptVM )
	{
		HSCRIPT hMissionStartFunc = g_pScriptVM->LookupFunction( "OnMissionStart" );
		if ( hMissionStartFunc )
		{
			ScriptStatus_t nStatus = g_pScriptVM->Call( hMissionStartFunc, NULL, false, NULL );
			if ( nStatus != SCRIPT_DONE )
			{
				DevWarning( "OnMissionStart VScript function did not finish!\n" );
			}
			g_pScriptVM->ReleaseFunction( hMissionStartFunc );
		}

		if ( g_pScriptVM->ValueExists( "g_ModeScript" ) )
		{
			ScriptVariant_t hModeScript;
			if ( g_pScriptVM->GetValue( "g_ModeScript", &hModeScript ) )
			{
				if ( HSCRIPT hFunction = g_pScriptVM->LookupFunction( "OnMissionStart", hModeScript ) )
				{
					ScriptStatus_t nStatus = g_pScriptVM->Call( hFunction, hModeScript, false, NULL );
					if ( nStatus != SCRIPT_DONE )
					{
						DevWarning( "OnMissionStart VScript function did not finish!\n" );
					}

					g_pScriptVM->ReleaseFunction( hFunction );
				}
				g_pScriptVM->ReleaseValue( hModeScript );
			}
		}
	}
	if ( g_pSwarmProxy )
	{
		g_pSwarmProxy->OnMissionStart();
	}
}

void CAlienSwarm::UpdateLaunching()
{
	if (!ASWGameResource())
		return;

	if (gpGlobals->curtime < m_fNextLaunchingStep)
		return;

	int iNumMarines = ASWGameResource()->GetNumMarines(NULL);

	if (m_iMarinesSpawned >=iNumMarines || !SpawnNextMarine())
	{
		// we've spawned all we can, finish up and go to ingame state
		
		// any players with no marines should be set to spectating one
		for ( int i = 1; i <= gpGlobals->maxClients; i++ )
		{
			CASW_Player* pOtherPlayer = ToASW_Player(UTIL_PlayerByIndex(i));
			
			if ( pOtherPlayer && pOtherPlayer->IsConnected() && ASWGameResource())
			{
				if (ASWGameResource()->GetNumMarines(pOtherPlayer) == 0)
					pOtherPlayer->SpectateNextMarine();
			}
		}	

		// notify all our alien spawners that the mission has started
		CBaseEntity* pEntity = NULL;
		while ((pEntity = gEntList.FindEntityByClassname( pEntity, "asw_spawner" )) != NULL)
		{
			CASW_Spawner* spawner = dynamic_cast<CASW_Spawner*>(pEntity);
			spawner->MissionStart();
		}

		SetGameState(ASW_GS_INGAME);
		mm_swarm_state.SetValue( "ingame" );
		DevMsg( "Setting game state to ingame\n" );

		// Re-compute FOV in case we were in first person on the lobby screen.
		for ( int i = 1; i <= gpGlobals->maxClients; i++ )
		{
			CBasePlayer *pPlayer = UTIL_PlayerByIndex( i );
			if ( pPlayer )
			{
				ClientSettingsChanged( pPlayer );
			}
		}

		// Alert gamestats of spawning
		CASW_GameStats.Event_MarinesSpawned();

		// tell all players to switch to their first marines
		// loop through all clients, count number of players on each team
		for ( int i = 1; i <= gpGlobals->maxClients; i++ )
		{
			CASW_Player* pPlayer = ToASW_Player(UTIL_PlayerByIndex(i));

			if ( pPlayer )
			{
				pPlayer->SwitchMarine(0);
				OrderNearbyMarines( pPlayer, ASW_ORDER_FOLLOW, false );
			}
		}

		// Set up starting stats
		CASW_Game_Resource *pGameResource = ASWGameResource();
		if ( pGameResource )
		{
			int iMaxMarineResources = pGameResource->GetMaxMarineResources();
			for ( int i = 0; i < iMaxMarineResources; i++ )
			{
				CASW_Marine_Resource *pMR = pGameResource->GetMarineResource( i );
				if ( pMR )
				{
					CASW_Marine *pMarine = pMR->GetMarineEntity();
					if ( pMarine )
					{
						pMR->m_TimelineAmmo.RecordValue( pMarine->GetAllAmmoCount() );
						pMR->m_TimelineHealth.RecordValue( pMarine->GetHealth() );
						pMR->m_TimelinePosX.RecordValue( pMarine->GetAbsOrigin().x );
						pMR->m_TimelinePosY.RecordValue( pMarine->GetAbsOrigin().y );
					}
				}
			}
		}

		// Start up any button hints
		for ( int i = 0; i < IASW_Use_Area_List::AutoList().Count(); i++ )
		{
			CASW_Use_Area *pArea = static_cast< CASW_Use_Area* >( IASW_Use_Area_List::AutoList()[ i ] );
			pArea->UpdateWaitingForInput();
		}
		if( g_pScriptVM )
		{
			HSCRIPT hGameplayStartFunc = g_pScriptVM->LookupFunction( "OnGameplayStart" );
			if ( hGameplayStartFunc )
			{
				ScriptStatus_t nStatus = g_pScriptVM->Call( hGameplayStartFunc, NULL, false, NULL );
				if ( nStatus != SCRIPT_DONE )
				{
					DevWarning( "OnGameplayStart VScript function did not finish!\n" );
				}
				g_pScriptVM->ReleaseFunction( hGameplayStartFunc );
			}

			if ( g_pScriptVM->ValueExists( "g_ModeScript" ) )
			{
				ScriptVariant_t hModeScript;
				if ( g_pScriptVM->GetValue( "g_ModeScript", &hModeScript ) )
				{
					if ( HSCRIPT hFunction = g_pScriptVM->LookupFunction( "OnGameplayStart", hModeScript ) )
					{
						ScriptStatus_t nStatus = g_pScriptVM->Call( hFunction, hModeScript, false, NULL );
						if ( nStatus != SCRIPT_DONE )
						{
							DevWarning( "OnGameplayStart VScript function did not finish!\n" );
						}

						g_pScriptVM->ReleaseFunction( hFunction );
					}
					g_pScriptVM->ReleaseValue( hModeScript );
				}
			}
		}
	}
	else
	{
		// still have more marines to spawn, set up our next launch stage
		m_fNextLaunchingStep = gpGlobals->curtime + ASW_LAUNCHING_STEP;
	}
}

void CAlienSwarm::ReportMissingEquipment()
{
	if (m_hEquipReq.Get())
		m_hEquipReq->ReportMissingEquipment();
}

void CAlienSwarm::ReportNeedTwoPlayers()
{
#ifdef GAME_DLL		
	UTIL_ClientPrintAll( ASW_HUD_PRINTTALKANDCONSOLE, "#asw_need_two_players" );
#endif
}

void CAlienSwarm::RestartMissionCountdown( CASW_Player *pPlayer )
{
	if ( GetGameState() != ASW_GS_INGAME )
	{
		RestartMission( pPlayer );
		return;
	}

	SetForceReady( ASW_FR_INGAME_RESTART );
}

void CAlienSwarm::RestartMission( CASW_Player *pPlayer, bool bForce, bool bSkipFail )
{
	// don't allow restarting if we're on the campaign map, as this does Bad Things (tm)
	if ( GetGameState() >= ASW_GS_CAMPAIGNMAP )
		return;

	// if a player is requesting the restart, then check everyone is ready
	if ( pPlayer && GetGameState() > ASW_GS_INGAME )	// allow restart without readiness during the game/briefing
	{
		// check other players are ready for the restart
		if ( !bForce && ASWGameResource() && !ASWGameResource()->AreAllOtherPlayersReady( pPlayer->entindex() ) )
		{
			Msg( "not all players are ready!\n" );
			return;
		}
	}

	if ( !bSkipFail && GetGameState() == ASW_GS_INGAME && gpGlobals->curtime - ASWGameRules()->m_fMissionStartedTime > 30.0f )
	{
		// They've been playing a bit... go to the mission fail screen instead!
		ASWGameRules()->MissionComplete( false );
		return;
	}

	// notify players of our mission restart
	IGameEvent *event = gameeventmanager->CreateEvent( "asw_mission_restart" );
	if ( event )
	{
		m_iMissionRestartCount++;
		event->SetInt( "restartcount", m_iMissionRestartCount );
		gameeventmanager->FireEvent( event );
	}

	StopStim();

	// if we're ingame, then upload for state (we don't do this once the mission is over, as stats are already sent on MissionComplete)
	// Stats todo:
	//if (GetGameState() == ASW_GS_INGAME && ASWGameStats())
		//ASWGameStats()->AddMapRecord();

	if ( GetCampaignSave() )
	{
		CASW_Campaign_Save *pSave = GetCampaignSave();
		pSave->SaveGameToFile();
	}

	SetForceReady( ASW_FR_NONE );

	if ( ASWGameResource() )
		ASWGameResource()->RememberLeaderID();

	if ( !asw_instant_restart.GetBool() )
	{
		if ( asw_instant_restart_debug.GetBool() )
		{
			Msg( "Not performing instant restart - disabled by convar.\n" );
		}

		if ( GetCampaignSave() )
			ChangeLevel_Campaign( STRING( gpGlobals->mapname ) );
		else
			engine->ChangeLevel( STRING( gpGlobals->mapname ), NULL );

		return;
	}

	if ( asw_instant_restart_debug.GetBool() )
		Msg( "Performing Instant Restart with %d entities, %d edicts.\n", gEntList.NumberOfEntities(), gEntList.NumberOfEdicts() );

	// find the first entity in the entity list
	CBaseEntity *pEnt = gEntList.FirstEnt();

	// as long as we've got a valid pointer, keep looping through the list
	while ( pEnt != NULL )
	{
		if ( m_MapResetFilter.ShouldCreateEntity( pEnt->GetClassname() ) )
		{
			if ( asw_instant_restart_debug.GetBool() )
				DevMsg( "Destroying entity %d: %s\n", pEnt->entindex(), pEnt->GetClassname() );

			// resetting this entity
			CBaseEntity *pNextEntity = gEntList.NextEnt( pEnt );
			UTIL_Remove( pEnt ); // mark entity for deletion
			pEnt = pNextEntity;
		}
		else
		{
			if ( asw_instant_restart_debug.GetBool() )
				DevMsg( "Keeping entity %d: %s\n", pEnt->entindex(), pEnt->GetClassname() );

			// keeping this entity, so don't destroy it
			pEnt = gEntList.NextEnt( pEnt );
		}
	}

	// Clear out the event queue
	g_EventQueue.Clear();

	// causes all marked entity to be actually removed
	gEntList.CleanupDeleteList();

	engine->AllowImmediateEdictReuse();
	debugoverlay->ClearAllOverlays();

	RevertSavedConvars();

	// clear out gamerules
	FullReset();
	ASWSpawnSelection()->LevelShutdownPostEntity();

	if ( asw_instant_restart_debug.GetBool() )
		Msg( "Instant Restart after cleanup has %d entities, %d edicts.\n", gEntList.NumberOfEntities(), gEntList.NumberOfEdicts() );

	// clear squad
	g_ASWSquadFormation.LevelInitPreEntity();
	ASWSpawnSelection()->LevelInitPreEntity();

	// with any unrequired entities removed, we use MapEntity_ParseAllEntities to reparse the map entities
	// this in effect causes them to spawn back to their normal position.
	MapEntity_ParseAllEntities( engine->GetMapEntitiesString(), &m_MapResetFilter, true );

	if ( asw_instant_restart_debug.GetBool() )
		Msg( "Instant Restart after reparse has %d entities, %d edicts.\n", gEntList.NumberOfEntities(), gEntList.NumberOfEdicts() );

	// let the players know the mission is restarting
	//UTIL_ClientPrintAll( HUD_PRINTCENTER, "Restarting Mission" );

	// reset our game state and setup game resource, etc
	LevelInitPostEntity();

	// re-init some systems
	TheTonemapSystem()->LevelInitPostEntity();
	PostProcessSystem()->LevelInitPostEntity();
	ColorCorrectionSystem()->LevelInitPostEntity();
	FogSystem()->LevelInitPostEntity();
	ASWDirector()->LevelInitPostEntity();
	GameTimescale()->LevelInitPostEntity();
	g_ASWSquadFormation.LevelInitPostEntity();
	CAI_DynamicLink::gm_bInitialized = false;
	CAI_DynamicLink::InitDynamicLinks();

	// respawn players
	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CASW_Player *pPlayerToReset = ToASW_Player( UTIL_PlayerByIndex( i ) );

		if ( pPlayerToReset )
		{
			pPlayerToReset->ResetFragCount();
			pPlayerToReset->ResetDeathCount();
			pPlayerToReset->Spawn();

			if ( pPlayerToReset->m_bSentJoinedMessage )
			{
				pPlayerToReset->m_bSentJoinedMessage = false;
				pPlayerToReset->OnFullyJoined( false );
			}
		}
	}

	if ( asw_instant_restart_debug.GetBool() )
		Msg( "Instant Restart completed with %d entities, %d edicts.\n", gEntList.NumberOfEntities(), gEntList.NumberOfEdicts() );
}

// issues a changelevel command with the campaign argument and save name
void CAlienSwarm::ChangeLevel_Campaign(const char *map)
{
	Assert(ASWGameResource());
	if ( !Q_strnicmp( map, "random", 6 ) && GetCampaignSave() )
	{
		// schedule level generation
		if ( !m_pMapBuilder )
		{
			Msg("Failed to create map builder\n");
			return;
		}
		ASWGameResource()->m_iRandomMapSeed = RandomInt( 1, 65535 );
		Q_snprintf( ASWGameResource()->m_szMapGenerationStatus.GetForModify(), 128, "Generating map..." );
		const char *szNewMapName = UTIL_VarArgs( "campaignrandommap%d.vmf", GetCampaignSave()->m_iNumMissionsComplete.Get() );
		KeyValues *pMissionSettings = new KeyValues( "MissionSettings" );	// TODO: These will need to be filled in with data for random maps in a campaign to work
		KeyValues *pMissionDefinition = new KeyValues( "MissionDefinition" );
		m_pMapBuilder->ScheduleMapGeneration( szNewMapName, Plat_FloatTime() + 1.0f, pMissionSettings, pMissionDefinition );
		return;
	}

	engine->ChangeLevel( UTIL_VarArgs( "%s %s %s", map, IsCampaignGame() ? "campaign" : "single_mission", ASWGameResource()->GetCampaignSaveName() ), NULL );
}

// called when the marines have finished the mission and want to save their progress and pick the next map to play
//  (Save and proceed)
void CAlienSwarm::CampaignSaveAndShowCampaignMap(CASW_Player* pPlayer, bool bForce)
{
	if (!bForce && pPlayer)
	{
		// abort if other players aren't ready
		if (!ASWGameResource()->AreAllOtherPlayersReady(pPlayer->entindex()))
			return;
	}

	if (m_iGameState != ASW_GS_DEBRIEF)
	{
		Msg("Unable to CampaignSaveAndShowCampaignMap as game isn't at the debriefing\n");
		return;
	}

	if (!GetMissionSuccess())
	{
		Msg("Unable to CampaignSaveAndShowCampaignMap as mission was failed\n");
		return;
	}

	CASW_Campaign_Save* pSave = GetCampaignSave();
	if (!pSave)
	{
		Msg("Unable to CampaignSaveAndShowCampaignMap as we have no campaign savegame loaded!\n");
		return;
	}

	if ( IsCampaignGame() == 0 && ASWGameRules() && ASWGameRules()->m_szCycleNextMap.Get()[0] != '\0' )
	{
		// just advance to the transition screen; we don't need to update the save as it will be going away.
		SetGameState( ASW_GS_CAMPAIGNMAP );
		return;
	}

	if ( IsCampaignGame() != 1 || !GetCampaignInfo() )
	{
		Msg( "Unable to CampaignSaveAndShowCampaignMap as this isn't a campaign game!\n" );
		return;
	}

	//Orange. OUTRO_MAP is not implemented. This check is to prevent abuse of manual cl_campaignsas when finishing last map in a campaign.
	if (CampaignMissionsLeft() <= 1) //at this point number not yet decreased
	{
		return;
	}

	SetForceReady(ASW_FR_NONE);

	// give each marine some skill points for finishing the mission	
	if (ASWGameResource())
	{
		for (int i=0;i<ASW_NUM_MARINE_PROFILES;i++)
		{
			CASW_Marine_Profile *pProfile = MarineProfileList()->m_Profiles[i];
			if (pProfile)
			{
				bool bOnMission = false;
				if (!pSave->IsMarineAlive(i))
				{
					Msg("Giving %s no points since s/he's dead.\n", pProfile->m_ShortName);
				}
				else
				{
					for (int k=0;k<ASWGameResource()->GetMaxMarineResources();k++)
					{
						CASW_Marine_Resource *pMR = ASWGameResource()->GetMarineResource(k);					
						if (pMR && pMR->GetProfile() == pProfile)
						{
							bOnMission = true;
							break;						
						}
					}
					//if (bOnMission)
					{
						// check the debrief stats to see how many skill points to give
						int iPointsToGive = ASW_SKILL_POINTS_PER_MISSION;
						if (m_hDebriefStats.Get())
						{
							iPointsToGive = m_hDebriefStats->GetSkillPointsAwarded(i);
						}
						//Msg("Giving %s %d points since s/he was on the mission.\n", pProfile->m_ShortName, iPointsToGive );
						for (int sp=0;sp<iPointsToGive;sp++)
						{
							pSave->IncreaseMarineSkill( i, ASW_SKILL_SLOT_SPARE );
						}
 					}
// 					else
// 					{
// 						Msg("Giving %s only 2 points since s/he wasn't on the mission.\n", pProfile->m_ShortName);
// 						pSave->IncreaseMarineSkill( i, ASW_SKILL_SLOT_SPARE );
// 						pSave->IncreaseMarineSkill( i, ASW_SKILL_SLOT_SPARE );
// 					}
				}
			}
		}
		ASWGameResource()->UpdateMarineSkills(pSave);
	}

	Msg("CAlienSwarm::CampaignSaveAndShowCampaignMap saving game and switching to campaign map mode\n");
	pSave->SetMissionComplete(GetCampaignSave()->m_iCurrentPosition);
	// save our awarded medals to file
	m_Medals.AddMedalsToCampaignSave(pSave);

	// if we're starting a new level, update the skill undo state for our marines
	if ( Q_strncmp( STRING(gpGlobals->mapname), "Lobby", 5 ) )		// unless we're on the lobby map, then don't update as it's not really moving to a new level, we're just loading a previous save or starting a new one
		GetCampaignSave()->UpdateSkillUndoState();

	// clear all wounding from the save
	for (int i=0;i<ASW_NUM_MARINE_PROFILES;i++)
	{
		GetCampaignSave()->SetMarineWounded(i, false);
	}

	// set wounding/death of marines
	for (int i=0;i<ASWGameResource()->GetMaxMarineResources();i++)
	{
		CASW_Marine_Resource *pMR = ASWGameResource()->GetMarineResource(i);
		if (pMR)
		{
			if (pMR->GetHealthPercent() <= 0 && asw_campaign_death.GetBool() )
			{
				// tell the campaign save that the marine is DEAD!
				Msg("Setting marine %d dead\n", i);
				GetCampaignSave()->SetMarineDead(pMR->GetProfileIndex(), true);
				GetCampaignSave()->SetMarineWounded(pMR->GetProfileIndex(), false);
			}
			else if (pMR->m_bTakenWoundDamage && asw_campaign_wounding.GetBool() )
			{
				// tell the campaign save that the marine is wounded
				GetCampaignSave()->SetMarineWounded(pMR->GetProfileIndex(), true);
				GetCampaignSave()->SetMarineDead(pMR->GetProfileIndex(), false);
			}
			else
			{
				GetCampaignSave()->SetMarineDead(pMR->GetProfileIndex(), false);
				GetCampaignSave()->SetMarineWounded(pMR->GetProfileIndex(), false);
			}
		}
	}

	// increase parasite kill counts
	for (int i=0;i<ASWGameResource()->GetMaxMarineResources();i++)
	{
		CASW_Marine_Resource *pMR = ASWGameResource()->GetMarineResource(i);
		if (pMR)
		{
			GetCampaignSave()->AddParasitesKilled(pMR->GetProfileIndex(), pMR->m_iParasitesKilled);			
		}
	}

	pSave->SaveGameToFile();

	// make sure all players are marked as not ready
	if (ASWGameResource())
	{
		for (int i=0;i<ASW_MAX_READY_PLAYERS;i++)
		{
			ASWGameResource()->m_bPlayerReady.Set(i, rd_ready_mark_override.GetBool());
		}
	}
	SetGameState(ASW_GS_CAMPAIGNMAP);
	GetCampaignSave()->SelectDefaultNextCampaignMission();
}

// moves the marines from one location to another
bool CAlienSwarm::RequestCampaignMove( int iTargetMission )
{
	// only allow campaign moves if the campaign map is up
	if ( m_iGameState != ASW_GS_CAMPAIGNMAP )
		return false;

	if ( !GetCampaignSave() )
		return false;

	if ( m_szCycleNextMap.Get()[0] != '\0' )
	{
		GetCampaignSave()->SaveGameToFile();

		if ( ASWGameResource() )
			ASWGameResource()->RememberLeaderID();

		ChangeLevel_Campaign( m_szCycleNextMap );

		return true;
	}

	if ( !GetCampaignInfo() )
		return false;

	GetCampaignSave()->SetMoveDestination( iTargetMission );

	return true;
}

// moves the marines from one location to another
bool CAlienSwarm::RequestCampaignLaunchMission( int iTargetMission )
{
	// only allow campaign moves if the campaign map is up
	if ( m_iGameState != ASW_GS_CAMPAIGNMAP || iTargetMission == 0 )	// 0 is the dropzone
		return false;

	if ( !GetCampaignSave() || !GetCampaignInfo() )
		return false;

	// don't allow the launch if we're not at this location
	if ( GetCampaignSave()->m_iCurrentPosition != iTargetMission )
	{
		Msg( "RequestCampaignLaunchMission %d failed as current location is %d\n", iTargetMission, GetCampaignSave()->m_iCurrentPosition.Get() );
		return false;
	}

	const RD_Campaign_Mission_t *pMission = GetCampaignInfo()->GetMission( iTargetMission );
	if ( !pMission )
	{
		Msg( "RequestCampaignLaunchMission %d failed as couldn't get this mission from the Campaign Info\n", iTargetMission );
		return false;
	}

	// save it!
	GetCampaignSave()->SaveGameToFile();

	Msg( "CAlienSwarm::RequestCampaignLaunchMission changing mission to %s\n", pMission->MapName );
	if ( ASWGameResource() )
		ASWGameResource()->RememberLeaderID();
	ChangeLevel_Campaign( pMission->MapName );
	return true;
}

void CAlienSwarm::VerifySpawnLocation( CASW_Marine *pMarine )
{
	// check spawn location is clear
	Vector vecPos = pMarine->GetAbsOrigin();
	trace_t tr;
	UTIL_TraceHull( vecPos,
		vecPos + Vector( 0, 0, 1 ),
		pMarine->CollisionProp()->OBBMins(),
		pMarine->CollisionProp()->OBBMaxs(),
		MASK_PLAYERSOLID,
		pMarine,
		COLLISION_GROUP_NONE,
		&tr );
	if( tr.fraction == 1.0 )		// current location is fine
		return;

	// now find the nearest clear info node
	CAI_Node *pNode = NULL;
	CAI_Node *pNearest = NULL;
	float fNearestDist = -1;

	for (int i=0;i<pMarine->GetNavigator()->GetNetwork()->NumNodes();i++)
	{
		pNode = pMarine->GetNavigator()->GetNetwork()->GetNode(i);
		if (!pNode)
			continue;
		float dist = pMarine->GetAbsOrigin().DistTo(pNode->GetOrigin());
		if (dist < fNearestDist || fNearestDist == -1)
		{
			// check the spot is clear
			vecPos = pNode->GetOrigin();		
			UTIL_TraceHull( vecPos,
				vecPos + Vector( 0, 0, 1 ),
				pMarine->CollisionProp()->OBBMins(),
				pMarine->CollisionProp()->OBBMaxs(),
				MASK_PLAYERSOLID,
				pMarine,
				COLLISION_GROUP_NONE,
				&tr );
			if( tr.fraction == 1.0 )
			{
				fNearestDist = dist;
				pNearest = pNode;
			}
		}
	}
	// found a valid node, teleport there
	if (pNearest)
	{
		vecPos = pNearest->GetOrigin();
		pMarine->Teleport( &vecPos, NULL, NULL );
	}
}

// spawns a marine for each marine in the marine resource list
bool CAlienSwarm::SpawnNextMarine()
{
	if (!ASWGameResource())
		return false;
	
	if (m_iMarinesSpawned == 0)
	{
		if (IsTutorialMap())
			m_pSpawningSpot = CASW_TutorialStartPoint::GetTutorialStartPoint(0);
		else
			m_pSpawningSpot = GetMarineSpawnPoint(NULL);
	}

	if (!m_pSpawningSpot)
	{
		Msg("Failed to spawn a marine! No more spawn points could be found.\n");
		return false;
	}	

	// reactivedrop: security measure, in case players were able to hack and 
	// select more marines than it is allowed by current challenge
	if (m_iMarinesSpawned + 1 > ASWGameResource()->m_iMaxMarines)
	{
		Warning("Attempted to spawn a marine beyond the allowed number of marines!\n");
		return false;
	}

	for (int i=m_iMarinesSpawned;i<ASWGameResource()->GetMaxMarineResources() && m_pSpawningSpot;i++)
	{
		CASW_Marine_Resource* pMR = ASWGameResource()->GetMarineResource(i);
		if (!pMR)
			continue;
		
		if ( !SpawnMarineAt( pMR, m_pSpawningSpot->GetAbsOrigin(), m_pSpawningSpot->GetAbsAngles(), false ) )
			return false;

		m_iMarinesSpawned++;

		// grab the next spawn spot
		if (IsTutorialMap())
		{
			m_pSpawningSpot = CASW_TutorialStartPoint::GetTutorialStartPoint(i+1);
		}
		else
		{
			
			// reactivedrop: if we don't find next spawn point then use the 
			// last one 
			CBaseEntity *spawn_pt =	GetMarineSpawnPoint(m_pSpawningSpot);
			if (!spawn_pt)
			{
				Warning("Failed to find a pMarine spawn point.  Map must "
						"have 8 info_player_start points!\n");
			}
			else
			{
				m_pSpawningSpot = spawn_pt;
			}
		}
	}

	return true;
}

/**
@param bResurrection  if true, we are resurrecting a marine while the map is in progress; restore initial weapon values rather than save them
*/
bool CAlienSwarm::SpawnMarineAt( CASW_Marine_Resource * RESTRICT pMR, const Vector &vecPos, const QAngle &angFacing, bool bResurrection )
{
	Assert( pMR );
	// create the marine
	CASW_Marine * RESTRICT pMarine = (CASW_Marine*) CreateEntityByName( "asw_marine" );
	if ( !pMarine )
	{
		Msg("ERROR - Failed to spawn a pMarine!");
		return false;
	}

	if (pMR->GetProfile())
	{
		pMarine->SetName(AllocPooledString(pMR->GetProfile()->m_ShortName));
	}
	// set the pMarine's commander, pMarine resource, etc
	pMarine->SetMarineResource( pMR );
	pMarine->SetCommander( pMR->m_Commander );
	pMarine->SetInitialCommander( pMR->m_Commander.Get() );
	pMarine->ChangeTeam( pMR->GetTeamNumber() );
	pMarine->Spawn();

	// position him
	pMarine->SetLocalOrigin( vecPos + Vector(0,0,1) );
	VerifySpawnLocation( pMarine );
	pMarine->m_vecLastSafePosition = vecPos + Vector(0,0,1);
	pMarine->SetAbsVelocity( vec3_origin );
	pMarine->SetAbsAngles( angFacing );
	pMarine->m_fHoldingYaw = angFacing[YAW];

	int iMarineHealth = MarineSkills()->GetSkillBasedValueByMarineResource(pMR, ASW_MARINE_SKILL_HEALTH);
	int iMarineMaxHealth = iMarineHealth;

	// half the pMarine's health if he's wounded
	if (IsCampaignGame() && GetCampaignSave())
	{
		if (GetCampaignSave()->IsMarineWounded(pMR->GetProfileIndex()))
		{
			iMarineHealth *= 0.5f;
			iMarineMaxHealth = iMarineHealth;
			pMR->m_bHealthHalved = true;
		}
	}
	else if ( ASWHoldoutMode() && bResurrection )
	{
		iMarineHealth *= 0.66;
	}

	pMarine->SetHealth(iMarineHealth);
	pMarine->SetMaxHealth(iMarineMaxHealth);
	pMR->m_TimelineHealth.RecordValue( iMarineHealth );

	pMR->SetMarineEntity(pMarine);

	if ( ASWHoldoutMode() && bResurrection )
	{
		// give the pMarine the equipment selected on the briefing screen
		for ( int iWpnSlot = 0; iWpnSlot < ASW_MAX_EQUIP_SLOTS; ++ iWpnSlot )
			GiveStartingWeaponToMarine( pMarine, pMR->m_iInitialWeaponsInSlots[ iWpnSlot ], iWpnSlot );
	}
	else
	{
		if ( ASWDeathmatchMode() )
		{
			if ( ASWDeathmatchMode()->IsGunGameEnabled() )
			{
				int weapon_id = ASWDeathmatchMode()->GetWeaponIndexByFragsCount( ASWDeathmatchMode()->GetFragCount( pMR ) );
				GiveStartingWeaponToMarine( pMarine, weapon_id , 0 );
			}
			else if ( ASWDeathmatchMode()->IsTeamDeathmatchEnabled() ||
				( rd_deathmatch_loadout_allowed.GetBool() &&
					ASWDeathmatchMode()->IsDeathmatchEnabled() ) )
			{
				// give the pMarine the equipment selected on the briefing screen
				for ( int iWpnSlot = 0; iWpnSlot < ASW_MAX_EQUIP_SLOTS; ++iWpnSlot )
				{
					int weapon_index = pMR->m_iInitialWeaponsInSlots[iWpnSlot];
					if ( weapon_index < 0 )
					{
						Warning( "When spawning marine the weapon_index is -1 \n" );
						weapon_index = pMR->m_iWeaponsInSlots.Get( iWpnSlot );
					}
					GiveStartingWeaponToMarine( pMarine, weapon_index, iWpnSlot );
				}
			}
			else
			{
				// give the pistols only in deathmatch, railgun in InstaGib
				GiveStartingWeaponToMarine( pMarine, rd_default_weapon.GetInt(), 0 );
				// give a heal gun for medic by default to compensate theirs weakness
				if ( pMR->GetProfile()->GetMarineClass() == MARINE_CLASS_MEDIC )
				{
					// heal gun
					GiveStartingWeaponToMarine( pMarine, 11, 1 );
				}
			}
		}
		else
		{
			for ( int iWpnSlot = 0; iWpnSlot < ASW_MAX_EQUIP_SLOTS; iWpnSlot++ )
			{
				GiveStartingWeaponToMarine( pMarine, ApplyWeaponSelectionRules( iWpnSlot, pMR->m_iWeaponsInSlots.Get( iWpnSlot ) ), iWpnSlot );
			}
		}

		// store off his initial equip selection for stats tracking
		for ( int iWpnSlot = 0; iWpnSlot < ASW_MAX_EQUIP_SLOTS; ++ iWpnSlot )
		{
			pMR->m_iInitialWeaponsInSlots[ iWpnSlot ] = pMR->m_iWeaponsInSlots.Get( iWpnSlot );
		}
	}

	pMarine->GetMarineResource()->UpdateWeaponIndices();

	return true;
}

CBaseEntity* CAlienSwarm::GetMarineSpawnPoint(CBaseEntity *pStartEntity)
{	
	do
	{
		pStartEntity = gEntList.FindEntityByClassname( pStartEntity, "info_player_start");
		if (pStartEntity && IsValidMarineStart(pStartEntity))
			return pStartEntity;
	} while (pStartEntity!=NULL);

	return NULL;
}

// make sure this spot doesn't have a marine on it already
bool CAlienSwarm::IsValidMarineStart(CBaseEntity *pSpot)
{
	//CBaseEntity *ent = NULL;
/*
	for ( CEntitySphereQuery sphere( pSpot->GetAbsOrigin(), 128 ); ent = sphere.GetCurrentEntity(); sphere.NextEntity() )
	{
		CASW_Marine* marine;
		marine = CASW_Marine::AsMarine( ent );
		if (marine!=NULL)
		{
			Msg("rejecting this start spot as a marine is nearby\n");
			return false;
		}
	}*/

	//Msg("this start spot is good\n");
	return true;
}

void CAlienSwarm::StartStim( float duration, CBaseEntity *pSource )
{
	m_flStimEndTime = gpGlobals->curtime + duration;
	m_flStimStartTime = gpGlobals->curtime;
	m_hStartStimPlayer = pSource;
}

void CAlienSwarm::StopStim()
{
	m_flStimEndTime = gpGlobals->curtime;
}

void CAlienSwarm::ThinkUpdateTimescale() RESTRICT 
{
	if ( GetGameState() != ASW_GS_INGAME || m_bMissionFailed )
	{
		// No slowdown when we're not in game
		GameTimescale()->SetDesiredTimescale( 1.0f );
		return;
	}

	if ( m_bDeathCamSlowdown )
	{
		if ( gpGlobals->curtime >= m_fMarineDeathTime && gpGlobals->curtime <= m_fMarineDeathTime + asw_time_scale_delay.GetFloat() + asw_marine_death_cam_time.GetFloat() )
		{
			// Wait for the delay before invuln starts
			if ( gpGlobals->curtime > m_fMarineDeathTime + asw_time_scale_delay.GetFloat() )
			{
				MarineInvuln( true );
			}

			GameTimescale()->SetDesiredTimescaleAtTime( asw_marine_death_cam_time_scale.GetFloat(), asw_marine_death_cam_time_interp.GetFloat(), CGameTimescale::INTERPOLATOR_EASE_IN_OUT, m_fMarineDeathTime + asw_time_scale_delay.GetFloat() );
			return;
		}
		else if ( gpGlobals->curtime > m_fMarineDeathTime + asw_time_scale_delay.GetFloat() + asw_marine_death_cam_time.GetFloat() + 1.5f )
		{
			// Wait for a longer delay before invuln stops
			MarineInvuln( false );

			m_nMarineForDeathCam = -1;
			m_fMarineDeathTime = 0.0f;
		}
	}
	else if ( !m_bDeathCamSlowdown && m_bMarineInvuln )
	{
		MarineInvuln( false );
		m_nMarineForDeathCam = -1;
		m_fMarineDeathTime = 0.0f;
	}

	if ( m_flStimEndTime > gpGlobals->curtime )
	{
		GameTimescale()->SetDesiredTimescale( asw_stim_time_scale.GetFloat(), 1.5f, CGameTimescale::INTERPOLATOR_EASE_IN_OUT, asw_time_scale_delay.GetFloat() );
		return;
	}

	GameTimescale()->SetDesiredTimescale( 1.0f, 1.5f, CGameTimescale::INTERPOLATOR_EASE_IN_OUT, asw_time_scale_delay.GetFloat() );
}

void CAlienSwarm::PlayerThink( CBasePlayer *pPlayer )
{
}

// --------------------------------------------------------------------------------------------------- //
// Voice helper
// --------------------------------------------------------------------------------------------------- //

class CVoiceGameMgrHelper : public IVoiceGameMgrHelper
{
public:
	virtual bool		CanPlayerHearPlayer( CBasePlayer *pListener, CBasePlayer *pTalker, bool &bProximity )
	{
		// players can always hear each other in Infested
		return true;
	}
};
CVoiceGameMgrHelper g_VoiceGameMgrHelper;
IVoiceGameMgrHelper *g_pVoiceGameMgrHelper = &g_VoiceGameMgrHelper;

// World.cpp calls this but we don't use it in Infested.
void InitBodyQue()
{
}

void CAlienSwarm::OnSteamRelayNetworkStatusChanged( SteamRelayNetworkStatus_t *pParam )
{
	DevMsg( 2, "Steam reports relay connection status: %s\n", pParam->m_debugMsg );
	// only update the approximate ping location if we're sure
	if ( !pParam->m_bPingMeasurementInProgress )
	{
		SteamNetworkPingLocation_t location;
		float flAge = SteamNetworkingUtils()->GetLocalPingLocation( location );
		if ( flAge >= 0 )
		{
			DevMsg( 2, "Updated server ping location. New age: %f\n", flAge );
			SetPingLocation( location );
		}
	}
}

void CAlienSwarm::Think()
{
	if ( m_bShuttingDown ) return;

	if ( !m_bObtainedPingLocation && SteamNetworkingUtils() )
	{
		SteamNetworkPingLocation_t location;
		float flAge = SteamNetworkingUtils()->GetLocalPingLocation( location );
		if ( flAge >= 0 )
		{
			SetPingLocation( location );
		}
	}

	// If a briefing camera gets deleted, check whether there's another one we can switch to.
	if ( m_bHadBriefingCamera && !m_hBriefingCamera )
	{
		m_hBriefingCamera = gEntList.FindEntityByClassname( NULL, "rd_briefing_camera" );
		m_bHadBriefingCamera = !!m_hBriefingCamera;
	}

	ThinkUpdateTimescale();
	GetVoiceGameMgr()->Update( gpGlobals->frametime );
	if ( m_iGameState <= ASW_GS_BRIEFING )
	{
		SetSkillLevel( asw_skill.GetInt() );
	}

	switch (m_iGameState)
	{
	case ASW_GS_BRIEFING:	
		{
			// let our mission chooser source think while we're relatively idle in the briefing, in case it needs to be scanning for missions
			if ( missionchooser && missionchooser->LocalMissionSource() )
			{
				missionchooser->LocalMissionSource()->IdleThink();
			}

			if ( ASWDeathmatchMode() )
			{
				// omit briefing and immediately start mission
				m_bShouldStartMission = true;
			}

			if (m_bShouldStartMission)
				StartMission();

			if (m_fReserveMarinesEndTime != 0 && gpGlobals->curtime > m_fReserveMarinesEndTime)
			{
				UnreserveMarines();
			}
			CheckForceReady();
		}
		break;
	case ASW_GS_LAUNCHING:
		{
			UpdateLaunching();
		}
		break;
	case ASW_GS_DEBRIEF:
		{
			CheckForceReady();
			if (gpGlobals->curtime >= m_fRemoveAliensTime && m_fRemoveAliensTime != 0)
			{
				RemoveAllAliens();
				RemoveNoisyWeapons();

				if ( GetMissionFailed() && rd_auto_fast_restart.GetBool() )
				{
					RestartMission( NULL, true, true );
				}
			}
		}
		break;
	case ASW_GS_INGAME:
		{
			if ( m_iForceReadyType == ASW_FR_INGAME_RESTART )
			{
				CheckForceReady();
			}
			CheckTechFailure();
			CheckDeathmatchFinish();
		}
		break;
	case ASW_GS_OUTRO:
		{
			if (gpGlobals->curtime > m_fLaunchOutroMapTime)
			{
#ifdef OUTRO_MAP
				Msg("[S] m_fLaunchOutroMapTime is up, doing changelevel!\n");
				m_fLaunchOutroMapTime = 0;
				if (engine->IsDedicatedServer())
				{
					// change to a single mission
					//engine->ChangeLevel(asw_default_mission.GetString(), NULL);
				}
				else
				{
					// move server to the outro
					CASW_Campaign_Info *pCampaign = GetCampaignInfo();
					const char *pszOutro = "outro_jacob";	 // the default
					if (pCampaign)
					{
						const char *pszCustomOutro = STRING(pCampaign->m_OutroMap);
						if (pszCustomOutro && ( !Q_strnicmp( pszCustomOutro, "outro", 5 ) ))
						{
							pszOutro = pszCustomOutro;
							Msg("[S] Using custom outro\n");
						}
						else
						{
							Msg("[S] No valid custom outro defined in campaign info, using default\n");
						}
					}
					engine->ChangeLevel(pszOutro, NULL);
				}
#endif
			}
		}
		break;
	}

	CheckChallengeConVars();

	if (m_iCurrentVoteType != ASW_VOTE_NONE)
		UpdateVote();

	if ( m_pMapBuilder && IsCampaignGame() )
	{
		if ( m_pMapBuilder->IsBuildingMission() )
		{
			m_pMapBuilder->Update( Plat_FloatTime() );
			ASWGameResource()->m_fMapGenerationProgress = m_pMapBuilder->GetProgress();
			if ( Q_strcmp( m_pMapBuilder->GetStatusMessage(), ASWGameResource()->m_szMapGenerationStatus.Get() ) )
			{
				Q_snprintf( ASWGameResource()->m_szMapGenerationStatus.GetForModify(), 128, "%s", m_pMapBuilder->GetStatusMessage() );
			}
		}
		else if ( m_pMapBuilder->GetProgress() == 1.0f )		// finished building
		{
			// check if all the clients have finished generating this map
			bool bAllFinished = true;
			if ( asw_client_build_maps.GetBool() )
			{
				CBasePlayer *pListenServer = engine->IsDedicatedServer() ? NULL : UTIL_GetListenServerHost();
				for ( int i = 1; i <= gpGlobals->maxClients; i++ )
				{
					CASW_Player* pOtherPlayer = ToASW_Player(UTIL_PlayerByIndex(i));
					if ( pOtherPlayer && pOtherPlayer->IsConnected()
								&& !pOtherPlayer->IsAnyBot()
								&& pOtherPlayer != pListenServer					// listen server doesn't generate the map clientside
								&& pOtherPlayer->m_fMapGenerationProgress.Get() < 1.0f )
					{
						bAllFinished = false;
						break;
					}
				}
			}

			if ( bAllFinished )
			{
				//launch the map
				char fixedname[ 512 ];
				Q_strncpy( fixedname, m_pMapBuilder->GetMapName(), sizeof( fixedname ) );
				Q_StripExtension( fixedname, fixedname, sizeof( fixedname ) );
				ChangeLevel_Campaign( fixedname );
			}
		}
	}
}



inline unsigned int ThreadShutdown(void* pParam)
{
	const float delay = gpGlobals->interval_per_tick * 1000;
	DevMsg("Shutdown delayed: %f ms\n", delay);
	ThreadSleep(delay);

	// send quit and execute command within the same frame
	engine->ServerCommand("quit\n");
	engine->ServerExecute();

	return 0;
}

void CAlienSwarm::Shutdown() 
{
	m_bShuttingDown = true;
	CreateSimpleThread(ThreadShutdown, engine);
}

void CAlienSwarm::OnServerHibernating()
{
	int iPlayers = 0;
	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CASW_Player* pOtherPlayer = ToASW_Player( UTIL_PlayerByIndex( i ) );

		if ( pOtherPlayer && pOtherPlayer->IsConnected() && !pOtherPlayer->IsAnyBot() )
		{
			iPlayers++;
		}
	}

	if ( iPlayers <= 0 )
	{
		// when server has no players, switch to the default campaign

		IASW_Mission_Chooser_Source *pSource = missionchooser ? missionchooser->LocalMissionSource() : NULL;
		if ( !pSource )
			return;

		const char *szCampaignName = asw_default_campaign.GetString();
		const char *szMissionName = NULL;
		const RD_Campaign_t *pCampaign = ReactiveDropMissions::GetCampaign( szCampaignName );
		if ( !pCampaign || !pCampaign->Installed )
		{
			Warning( "Unable to find default campaign %s when server started hibernating.", szCampaignName );
			return;
		}

		szMissionName = pCampaign->Missions.Count() > 1 ? pCampaign->Missions[1].MapName : NULL;
		if ( !szMissionName )
		{
			Warning( "Unabled to find starting mission for campaign %s when server started hibernating.", szCampaignName );
			return;
		}

		char szSaveFilename[MAX_PATH]{};
		if ( !pSource->ASW_Campaign_CreateNewSaveGame( &szSaveFilename[0], sizeof( szSaveFilename ), szCampaignName, ( gpGlobals->maxClients > 1 ), szMissionName ) )
		{
			Warning( "Unable to create new save game when server started hibernating.\n" );
			return;
		}

		// quit server
		if ( !IsLobbyMap() && rd_server_shutdown_when_empty.GetBool() )
		{
			Shutdown();
		}

		// reset difficulty and challenge
		asw_skill.SetValue( 2 );
		SetSkillLevel( asw_skill.GetInt() );
		EnableChallenge( "0" );

		engine->ServerCommand( CFmtStr( "%s %s campaign %s\n",
			"changelevel",
			szMissionName,
			szSaveFilename ) );
	}
}

ConVar asw_respawn_marine_enable( "asw_respawn_marine_enable", "0", FCVAR_CHEAT, "Enables respawning marines.", true, 0, true, 1 );

// Respawn a dead marine.
void CAlienSwarm::Resurrect( CASW_Marine_Resource * RESTRICT pMR, CASW_Marine *pRespawnNearMarine )  
{
	if ( !asw_respawn_marine_enable.GetBool() )
	{
		Msg( "Respawning marines is not enabled on this server.\n" );
		return;
	}

	//AssertMsg1( !pMR->IsAlive() && 
	//((gpGlobals->curtime - pMR->m_fDeathTime) >= asw_marine_resurrection_interval.GetFloat() ),
	//"Tried to respawn %s before its time!", pMR->GetProfile()->GetShortName() );

	CAI_Network* RESTRICT pNetwork = g_pBigAINet;
	if ( !pNetwork || !pNetwork->NumNodes() )
	{
		Warning("Error: Can't resurrect marines as this map has no node network\n");
		return;
	}

	// walk over the network to find a node close enough to the marines. (For now I'll just choose one at random.)
	//const Vector &vecMarineHullMins = NAI_Hull::Mins( HULL_HUMAN );
	//const Vector &vecMarineHullMaxs = NAI_Hull::Maxs( HULL_HUMAN );
	const int iNumNodes = pNetwork->NumNodes();
	CAI_Node * RESTRICT pSpawnNode = NULL;
	float flNearestDist = FLT_MAX;
	Vector vecSpawnPos;
	Vector vecChosenSpawnPos;
	for ( int i = 0; i < iNumNodes; ++i )
	{
		CAI_Node * RESTRICT const pNode = pNetwork->GetNode( i );
		if ( !pNode || pNode->GetType() != NODE_GROUND )
			continue;

		vecSpawnPos = pNode->GetPosition( HULL_HUMAN );

		// find the nearest marine to this node
		//float flDistance = 0;
		CASW_Marine *pMarine = pRespawnNearMarine; //dynamic_cast<CASW_Marine*>(UTIL_ASW_NearestMarine( vecSpawnPos, flDistance ));
		if ( !pMarine )
			return;

		// TODO: check for exit triggers
#if 0
		// check node isn't in an exit trigger
		bool bInsideEscapeArea = false;
		for ( int d=0; d < pSpawnMan->m_EscapeTriggers.Count(); d++ )
		{
			if (pSpawnMan->m_EscapeTriggers[d]->CollisionProp()->IsPointInBounds( vecPos ) )
			{
				bInsideEscapeArea = true;
				break;
			}
		}
		if ( bInsideEscapeArea )
			continue;
#endif

		float flDist = pMarine->GetAbsOrigin().DistToSqr( vecSpawnPos );
		if ( flDist < flNearestDist )
		{
			// check if there's a route from this node to the marine(s)
			AI_Waypoint_t * RESTRICT const pRoute  = ASWPathUtils()->BuildRoute( vecSpawnPos, pMarine->GetAbsOrigin(), NULL, 100 );
			if ( !pRoute )
			{
				continue;
			}
			else
			{
				ASWPathUtils()->DeleteRoute( pRoute ); // don't leak routes
			}

			// if down here, have a candidate node
			pSpawnNode = pNode;
			flNearestDist = flDist;
			vecChosenSpawnPos = vecSpawnPos;
		}
	}

	if ( !pSpawnNode ) // no acceptable resurrect locations
		return;

	// WISE FWOM YOUW GWAVE!
	ScriptResurrect( pMR, vecChosenSpawnPos );
}

// Respawn a dead marine. DEPRECATED, UNUSED
void CAlienSwarm::Resurrect( CASW_Marine_Resource * RESTRICT pMR )  
{
	static CBaseEntity *spawn_spot = NULL;

	spawn_spot = ASWGameRules()->GetMarineSpawnPoint(spawn_spot);
	if (!spawn_spot)
		spawn_spot = ASWGameRules()->GetMarineSpawnPoint(NULL);

	if (!spawn_spot) {
		Msg( "Failed to find spawn spot" );
		return;
	}
	if ( !SpawnMarineAt( pMR, spawn_spot->GetAbsOrigin(), spawn_spot->GetAbsAngles(), true ) )
	{
		Msg( "Failed to resurrect marine %s\n", pMR->GetProfile()->GetShortName() );
		return;
	}
	else
	{
		CASW_Marine *pMarine = pMR->GetMarineEntity();
		AssertMsg1( pMarine, "SpawnMarineAt failed to populate marine resource %s with a marine entity!\n", pMR->GetProfile()->GetShortName() );
		// switch commander to the marine if he hasn't already got one selected
		if ( !pMR->GetCommander()->GetNPC() )
			pMR->GetCommander()->SwitchMarine(0 );
		pMarine->PerformResurrectionEffect();
	}
}

CASW_Marine* CAlienSwarm::ScriptResurrect( CASW_Marine_Resource* RESTRICT pMR, Vector vecSpawnPos, bool bEffect )
{
	CASW_Marine* pMarine = NULL;
	
	if ( !pMR || GetGameState() != ASW_GS_INGAME ) return NULL;

	if ( !SpawnMarineAt( pMR, vecSpawnPos + Vector( 0, 0, 1 ), QAngle( 0, 0, 0 ), true ) )
	{
		Msg( "Failed to resurrect marine %s\n", pMR->GetProfile()->GetShortName() );
		return NULL;
	}
	else
	{
		pMarine = pMR->GetMarineEntity();
		AssertMsg1( pMarine, "SpawnMarineAt failed to populate marine resource %s with a marine entity!\n", pMR->GetProfile()->GetShortName() );
		if ( !pMR->GetCommander()->GetNPC() )
			pMR->GetCommander()->SwitchMarine( 0 );
		
		if ( bEffect )
			pMarine->PerformResurrectionEffect();
	}

	return pMarine;
}

void CAlienSwarm::MarineInvuln()
{
	MarineInvuln( !m_bMarineInvuln );
}

void CAlienSwarm::MarineInvuln( bool bInvuln )
{
	m_bMarineInvuln = bInvuln;

	/*char text[256];
	Q_snprintf( text,sizeof(text), m_bMarineInvuln ? "Marines now invulnerable\n" : "Marines can now be hurt\n" );
	UTIL_ClientPrintAll( ASW_HUD_PRINTTALKANDCONSOLE, text );*/
}

void CAlienSwarm::KillAllMarines()
{
	for ( int i = 0; i < ASW_MAX_MARINE_RESOURCES; i++ )
	{
		CASW_Marine_Resource* pMR = ASWGameResource()->GetMarineResource(i);
		if (pMR!=NULL && pMR->GetMarineEntity()!=NULL && pMR->GetMarineEntity()->GetHealth() > 0)
		{
			pMR->GetMarineEntity()->Suicide(); 
		}
	}
}

void CAlienSwarm::ResetScores()
{
	for ( int i = 0; i < ASW_MAX_MARINE_RESOURCES; i++ )
	{
		CASW_Marine_Resource* pMR = ASWGameResource()->GetMarineResource(i);
		if ( pMR != NULL && pMR->GetCommander() != NULL )
		{
			CASW_Player *p = pMR->GetCommander();
			p->ResetScores();
		}
	}
}

bool CAlienSwarm::CanHaveAmmo( CBaseCombatCharacter *pPlayer, int iAmmoIndex )
{
	if ( iAmmoIndex > -1 )
	{
		// Get the max carrying capacity for this ammo
		int iMaxCarry = GetAmmoDef()->MaxCarry( iAmmoIndex, pPlayer );

		// asw - allow carrying more ammo if we have duplicate guns
		CASW_Marine* pMarine = CASW_Marine::AsMarine( pPlayer );		
		if (pMarine)
		{
			int iGuns = pMarine->GetNumberOfWeaponsUsingAmmo(iAmmoIndex);
			iMaxCarry *= iGuns;
		}

		// Does the player have room for more of this type of ammo?
		if ( pPlayer->GetAmmoCount( iAmmoIndex ) < iMaxCarry )
			return true;
	}

	return false;
}

void CAlienSwarm::GiveStartingWeaponToMarine(CASW_Marine* pMarine, int iEquipIndex, int iSlot)
{
	if ( !pMarine || iEquipIndex == -1 || iSlot < 0 || iSlot >= ASW_MAX_EQUIP_SLOTS )
		return;

	CASW_EquipItem *pEquip = ASWEquipmentList()->GetItemForSlot( iSlot, iEquipIndex );
	Assert( pEquip );
	if ( !pEquip )
		return;

	const char* szWeaponClass = STRING( pEquip->m_EquipClass );
	Assert( szWeaponClass );
	if ( !szWeaponClass )
		return;
		
	CASW_Weapon* pWeapon = dynamic_cast<CASW_Weapon*>(pMarine->Weapon_Create(szWeaponClass));
	Assert( pWeapon );
	if ( !pWeapon )
		return;

	// If I have a name, make my weapon match it with "_weapon" appended
	if ( pMarine->GetEntityName() != NULL_STRING )
	{
		const char *pMarineName = STRING(pMarine->GetEntityName());
		const char *pError = UTIL_VarArgs("%s_weapon", pMarineName);
		string_t pooledName = AllocPooledString(pError);
		pWeapon->SetName( pooledName );
	}

	// set the amount of bullets in the gun
	//Msg("Giving starting waepon to marine: %s ",szWeaponClass);
	int iPrimaryAmmo = pWeapon->GetDefaultClip1();	
	int iSecondaryAmmo = pWeapon->GetDefaultClip2();
	// adjust here for medical satchel charges if the marine has the skill for it
	if ( !stricmp(szWeaponClass, "asw_weapon_medical_satchel") || !stricmp(szWeaponClass, "asw_weapon_heal_grenade") )
	{
		if (pMarine->GetMarineProfile() && pMarine->GetMarineProfile()->CanUseFirstAid())
		{
			iPrimaryAmmo = MarineSkills()->GetSkillBasedValueByMarine(pMarine, ASW_MARINE_SKILL_HEALING, ASW_MARINE_SUBSKILL_HEALING_CHARGES);
			iSecondaryAmmo = MarineSkills()->GetSkillBasedValueByMarine(pMarine, ASW_MARINE_SKILL_HEALING, ASW_MARINE_SUBSKILL_SELF_HEALING_CHARGES);
		}
	}
	if ( !stricmp(szWeaponClass, "asw_weapon_heal_gun") )
	{
		if (pMarine->GetMarineProfile() && pMarine->GetMarineProfile()->CanUseFirstAid())
		{
			iPrimaryAmmo = MarineSkills()->GetSkillBasedValueByMarine(pMarine, ASW_MARINE_SKILL_HEALING, ASW_MARINE_SUBSKILL_HEAL_GUN_CHARGES);
		}
	}
	if (!stricmp(szWeaponClass, "asw_weapon_healamp_gun"))
	{
		if (pMarine->GetMarineProfile() && pMarine->GetMarineProfile()->CanUseFirstAid())
		{
			iPrimaryAmmo = MarineSkills()->GetSkillBasedValueByMarine( pMarine, ASW_MARINE_SKILL_HEALING, ASW_MARINE_SUBSKILL_HEALAMP_GUN_CHARGES );
			iSecondaryAmmo = MarineSkills()->GetSkillBasedValueByMarine( pMarine, ASW_MARINE_SKILL_DRUGS, ASW_MARINE_SUBSKILL_HEALAMP_GUN_AMP_CHARGES );
		}
	}
	if ( !stricmp(szWeaponClass, "asw_weapon_flares") ||
		 !stricmp(szWeaponClass, "asw_weapon_gas_grenades") || 
		 !stricmp(szWeaponClass, "asw_weapon_grenades") || 
		 !stricmp(szWeaponClass, "asw_weapon_mines") ||
		 !stricmp(szWeaponClass, "asw_weapon_electrified_armor") ||
		 !stricmp(szWeaponClass, "asw_weapon_buff_grenade") ||
		 !stricmp(szWeaponClass, "asw_weapon_hornet_barrage") ||
		 !stricmp(szWeaponClass, "asw_weapon_heal_grenade") ||
		 !stricmp(szWeaponClass, "asw_weapon_t75") ||
		 !stricmp(szWeaponClass, "asw_weapon_freeze_grenades") ||
		 !stricmp(szWeaponClass, "asw_weapon_bait") ||
		 !stricmp(szWeaponClass, "asw_weapon_smart_bomb") ||
		 !stricmp(szWeaponClass, "asw_weapon_jump_jet") ||
		 !stricmp(szWeaponClass, "asw_weapon_tesla_trap")
		)
	{
		iPrimaryAmmo += asw_bonus_charges.GetInt();
	}

	if ( !stricmp(szWeaponClass, "asw_weapon_ammo_satchel" ) ) 
	{
		iPrimaryAmmo += rd_ammo_bonus.GetInt();
	}

	pWeapon->SetClip1( iPrimaryAmmo );
	// set secondary bullets in the gun
	//Msg("Setting secondary bullets for %s to %d\n", szWeaponClass, iSecondaryAmmo);
	pWeapon->SetClip2( iSecondaryAmmo );
	
	// equip the weapon
	pMarine->Weapon_Equip_In_Index( pWeapon, iSlot );

	// set the number of clips
	if ( pWeapon->GetPrimaryAmmoType() != -1 )
	{
		int iClips = GetAmmoDef()->MaxCarry( pWeapon->GetPrimaryAmmoType(), pMarine );

		//Msg("Giving %d bullets for primary ammo type %d\n", iClips, pWeapon->GetPrimaryAmmoType());
		pMarine->GiveAmmo( iClips, pWeapon->GetPrimaryAmmoType(), true );
	}
	else
	{
		//Msg("No clips as no primary ammo type\n");
	}
	// if it's primary, switch to it
	if (iSlot == 0)
	{
		// temp comment
		pMarine->Weapon_Switch( pWeapon );
		pWeapon->SetWeaponVisible(true);
	}
	else
	{
		pWeapon->SetWeaponVisible(false);
	}

	if (rd_server_marine_backpacks.GetBool() && iSlot == 1)
	{
		//pMarine->RemoveBackPackModel();
		pMarine->CreateBackPackModel(pWeapon);
	}

}

// find all pickups in the level and increment charges
void CAlienSwarm::AddBonusChargesToPickups()
{
	CBaseEntity *ent = NULL;
	while ( (ent = gEntList.NextEnt(ent)) != NULL )
	{
		const char *szClass = ent->GetClassname();
		if ( !stricmp(szClass, "asw_pickup_flares") ||
			!stricmp(szClass, "asw_pickup_grenades") || 
			!stricmp(szClass, "asw_pickup_mines")
			)
		{
			CASW_Pickup_Weapon *pPickup = dynamic_cast<CASW_Pickup_Weapon*>(ent);
			if ( pPickup )
			{
				pPickup->m_iBulletsInGun += asw_bonus_charges.GetInt();
			}
		}
	}
}

// AI Class stuff

void CAlienSwarm::InitDefaultAIRelationships()
{
	BaseClass::InitDefaultAIRelationships();

	//  Allocate memory for default relationships
	CBaseCombatCharacter::AllocateDefaultRelationships();

	// set up faction relationships
	CAI_BaseNPC::SetDefaultFactionRelationship(FACTION_MARINES, FACTION_ALIENS, D_HATE, 0 );
	CAI_BaseNPC::SetDefaultFactionRelationship(FACTION_MARINES, FACTION_MARINES, D_LIKE, 0 );
	CAI_BaseNPC::SetDefaultFactionRelationship(FACTION_MARINES, FACTION_BAIT, D_NEUTRAL, 0 );
	CAI_BaseNPC::SetDefaultFactionRelationship(FACTION_MARINES, FACTION_NEUTRAL, D_NEUTRAL, 0 );
	CAI_BaseNPC::SetDefaultFactionRelationship(FACTION_MARINES, FACTION_COMBINE, D_HATE, 0 );
	CAI_BaseNPC::SetDefaultFactionRelationship(FACTION_MARINES, FACTION_ROBOTS, D_HATE, 0 );

	CAI_BaseNPC::SetDefaultFactionRelationship(FACTION_ALIENS, FACTION_ALIENS, D_LIKE, 0 );
	CAI_BaseNPC::SetDefaultFactionRelationship(FACTION_ALIENS, FACTION_MARINES, D_HATE, 0 );
	CAI_BaseNPC::SetDefaultFactionRelationship(FACTION_ALIENS, FACTION_BAIT, D_HATE, 999 );
	CAI_BaseNPC::SetDefaultFactionRelationship(FACTION_ALIENS, FACTION_NEUTRAL, D_NEUTRAL, 0 );
	CAI_BaseNPC::SetDefaultFactionRelationship(FACTION_ALIENS, FACTION_COMBINE, D_HATE, 0 );
	CAI_BaseNPC::SetDefaultFactionRelationship(FACTION_ALIENS, FACTION_ROBOTS, D_HATE, 0 );

	CAI_BaseNPC::SetDefaultFactionRelationship(FACTION_NEUTRAL, FACTION_NEUTRAL, D_NEUTRAL, 0 );
	CAI_BaseNPC::SetDefaultFactionRelationship(FACTION_NEUTRAL, FACTION_MARINES, D_NEUTRAL, 0 );
	CAI_BaseNPC::SetDefaultFactionRelationship(FACTION_NEUTRAL, FACTION_BAIT, D_NEUTRAL, 0 );
	CAI_BaseNPC::SetDefaultFactionRelationship(FACTION_NEUTRAL, FACTION_ALIENS, D_NEUTRAL, 0 );
	CAI_BaseNPC::SetDefaultFactionRelationship(FACTION_NEUTRAL, FACTION_COMBINE, D_NEUTRAL, 0 );
	CAI_BaseNPC::SetDefaultFactionRelationship(FACTION_NEUTRAL, FACTION_ROBOTS, D_NEUTRAL, 0 );

	CAI_BaseNPC::SetDefaultFactionRelationship(FACTION_COMBINE, FACTION_COMBINE, D_LIKE, 0 );
	CAI_BaseNPC::SetDefaultFactionRelationship(FACTION_COMBINE, FACTION_NEUTRAL, D_NEUTRAL, 0 );
	CAI_BaseNPC::SetDefaultFactionRelationship(FACTION_COMBINE, FACTION_MARINES, D_HATE, 0 );
	CAI_BaseNPC::SetDefaultFactionRelationship(FACTION_COMBINE, FACTION_BAIT, D_NEUTRAL, 0 );
	CAI_BaseNPC::SetDefaultFactionRelationship(FACTION_COMBINE, FACTION_ALIENS, D_HATE, 0 );
	CAI_BaseNPC::SetDefaultFactionRelationship(FACTION_COMBINE, FACTION_ROBOTS, D_HATE, 0 );

	CAI_BaseNPC::SetDefaultFactionRelationship(FACTION_ROBOTS, FACTION_ROBOTS, D_LIKE, 0 );
	CAI_BaseNPC::SetDefaultFactionRelationship(FACTION_ROBOTS, FACTION_NEUTRAL, D_NEUTRAL, 0 );
	CAI_BaseNPC::SetDefaultFactionRelationship(FACTION_ROBOTS, FACTION_MARINES, D_HATE, 0 );
	CAI_BaseNPC::SetDefaultFactionRelationship(FACTION_ROBOTS, FACTION_BAIT, D_NEUTRAL, 0 );
	CAI_BaseNPC::SetDefaultFactionRelationship(FACTION_ROBOTS, FACTION_ALIENS, D_HATE, 0 );
	CAI_BaseNPC::SetDefaultFactionRelationship(FACTION_ROBOTS, FACTION_COMBINE, D_HATE, 0 );

	// Matching HL2 defaults: Wildlife is scared of everything except other wildlife, barnacles (if we ever add them), and invisible NPCs.
	for ( int nClass = CLASS_NONE; nClass < LAST_ASW_ENTITY_CLASS; nClass++ )
	{
		if ( nClass != CLASS_EARTH_FAUNA && nClass != CLASS_BARNACLE && nClass != CLASS_BULLSEYE )
		{
			CBaseCombatCharacter::SetDefaultRelationship( CLASS_EARTH_FAUNA, (Class_T)nClass, D_HT, 0 );
		}
	}
}

CASW_Mission_Manager* CAlienSwarm::GetMissionManager()
{
	return m_pMissionManager;
}

void CAlienSwarm::CheatCompleteMission()
{
	if (m_iGameState == ASW_GS_INGAME && GetMissionManager())
		GetMissionManager()->CheatCompleteMission();
}

void CAlienSwarm::FinishDeathmatchRound( CASW_Marine_Resource *winner )
{
	winner->GetDisplayName( m_szDeathmatchWinnerName.GetForModify(), sizeof( m_szDeathmatchWinnerName ) );

	if ( ASWDeathmatchMode()->IsTeamDeathmatchEnabled() )
	{
		int iTeam = winner->GetTeamNumber();
		V_strncpy( m_szDeathmatchWinnerName.GetForModify(), GetGlobalTeam( iTeam )->GetName(), sizeof( m_szDeathmatchWinnerName ) );

		for ( int i = 0; i < ASW_MAX_MARINE_RESOURCES; i++ )
		{
			CASW_Marine_Resource *pMR = ASWGameResource()->GetMarineResource( i );
			if ( !pMR )
			{
				continue;
			}

			if ( pMR->GetTeamNumber() == iTeam )
			{
				CASW_Marine *pWinnerMarine = pMR->GetMarineEntity();
				if ( pWinnerMarine )
				{
					char szWinnerName[ MAX_PLAYER_NAME_LENGTH ];
					pMR->GetDisplayName( szWinnerName, sizeof( szWinnerName ) );
					UTIL_ClientPrintAll( ASW_HUD_PRINTTALKANDCONSOLE, "#asw_player_invulnurable", szWinnerName );
					pWinnerMarine->m_takedamage = DAMAGE_NO;
				}
			}
		}
	}
	else
	{
		CASW_Marine *pWinnerMarine = winner->GetMarineEntity();
		if ( pWinnerMarine )
		{
			UTIL_ClientPrintAll( ASW_HUD_PRINTTALKANDCONSOLE, "#asw_player_invulnurable", m_szDeathmatchWinnerName );
			pWinnerMarine->m_takedamage = DAMAGE_NO;

			for ( int i = 1; i <= gpGlobals->maxClients; i++ )
			{
				CASW_Player *pPlayer = ToASW_Player( UTIL_PlayerByIndex( i ) );
				if ( !pPlayer )
				{
					continue;
				}

				if ( pPlayer->GetSpectatingNPC() || !pPlayer->GetNPC() || !pPlayer->GetNPC()->IsAlive() || pPlayer->GetNPC()->GetHealth() <= 0 )
				{
					pPlayer->SetSpectatingNPC( pWinnerMarine );
				}
			}
		}
	}

	UTIL_ClientPrintAll( ASW_HUD_PRINTTALKANDCONSOLE, "#asw_player_won_deathmatch", m_szDeathmatchWinnerName );
	
	m_fDeathmatchFinishTime = gpGlobals->curtime + rd_deathmatch_ending_time.GetFloat();
	m_iDeathmatchFinishCount = rd_deathmatch_ending_time.GetInt();

	m_fMarineDeathTime = gpGlobals->curtime;
	m_vMarineDeathPos = m_vMarineDeathPosDeathmatch;
	m_nMarineForDeathCam = m_nMarineForDeathCamDeathmatch;
}

// deletes all entities by their name or their class name
static void DeleteAllEntities( const char *str )
{
	// Otherwise remove based on name or classname
	int iCount = 0;
	CBaseEntity *ent = NULL;
	while ( ( ent = gEntList.NextEnt( ent ) ) != NULL )
	{
		if ( ( ent->GetEntityName() != NULL_STRING	&& FStrEq( str, STRING( ent->GetEntityName() ) ) ) ||
			( ent->m_iClassname != NULL_STRING	&& FStrEq( str, STRING( ent->m_iClassname ) ) ) ||
			( ent->GetClassname() != NULL && FStrEq( str, ent->GetClassname() ) ) )
		{
			UTIL_Remove( ent );
			iCount++;
		}
	}
}

// removes all alien NPCs from map
static void ClearHouse()
{
	if ( ASWSpawnManager() )
	{
		int nCount = ASWSpawnManager()->GetNumAlienClasses();
		for ( int i = 0; i < nCount; i++ )
		{
			DeleteAllEntities( ASWSpawnManager()->GetAlienClass( i )->m_pszAlienClass ) ;
		}
	}
	DeleteAllEntities("asw_alien_goo");
	DeleteAllEntities("asw_grub_sac");
	DeleteAllEntities("asw_spawner");
	DeleteAllEntities("asw_egg");
	DeleteAllEntities("asw_drone_uber");
	DeleteAllEntities("npc_antlionguard_normal");
	DeleteAllEntities("npc_antlionguard_cavern");
}

void CAlienSwarm::MissionComplete( bool bSuccess )
{
	if ( m_iGameState >= ASW_GS_DEBRIEF )	// already completed the mission
		return;

	StopStim();

	// setting these variables will make the player's go into their debrief screens
	if ( bSuccess )
	{
		m_bMissionSuccess = true;
		IGameEvent *event = gameeventmanager->CreateEvent( "mission_success" );
		if ( event )
		{
			event->SetString( "strMapName", STRING( gpGlobals->mapname ) );
			gameeventmanager->FireEvent( event );
		}
	}
	else
	{
		m_bMissionFailed = true;
		m_nFailAdvice = ASWFailAdvice()->UseCurrentFailAdvice();
		IGameEvent *event = gameeventmanager->CreateEvent( "mission_failed" );
		if ( event )
		{
			event->SetString( "strMapName", STRING( gpGlobals->mapname ) );
			gameeventmanager->FireEvent( event );
		}
	}

	Assert( m_szCycleNextMap.Get()[0] == '\0' ); // shouldn't be initialized yet
	if ( IsCampaignGame() == 0 )
	{
		int iMission = ReactiveDropMissions::GetMissionIndex( STRING( gpGlobals->mapname ) );
		if ( iMission != -1 )
		{
			const RD_Mission_t *pMission = ReactiveDropMissions::GetMission( iMission );
			Assert( pMission );

			int iMode = 0;
			const char *szTag = "unknown";
			if ( pMission->HasTag( "deathmatch" ) )
			{
				iMode = rd_mapcycle_deathmatch.GetInt();
				szTag = "deathmatch";
			}
			else if ( pMission->HasTag( "endless" ) )
			{
				iMode = rd_mapcycle_endless.GetInt();
				szTag = "endless";
			}
			else if ( pMission->HasTag( "bonus" ) )
			{
				iMode = rd_mapcycle_bonus.GetInt();
				szTag = "bonus";
			}
			else if ( GetCampaignSave() )
			{
				Assert( !"mission is not campaign, bonus, endless, or deathmatch." );
				Warning( "Mission is in mission chooser but is not Campaign, Bonus, Endless, or Deathmatch. Tell Ben!\n" );
			}

			CSplitString IgnoreMissions( rd_mapcycle_ignore.GetString(), "," );

			switch ( iMode )
			{
			case 0:
			{
				// Mode 0: no cycle; show "New Campaign" button instead of "Continue"
				break;
			}
			case 1:
			{
				// Mode 1: cycle in mission chooser order for missions with the same tag; if this is the only mission, show "New Campaign" instead

				bool bFound = false;
				for ( int iNextMission = iMission + 1; iNextMission < ReactiveDropMissions::CountMissions(); iNextMission++ )
				{
					const RD_Mission_t *pNextMission = ReactiveDropMissions::GetMission( iNextMission );
					Assert( pNextMission );

					bool bIgnore = false;
					FOR_EACH_VEC( IgnoreMissions, i )
					{
						if ( !V_stricmp( pNextMission->BaseName, IgnoreMissions[i] ) )
						{
							bIgnore = true;
							break;
						}
					}

					if ( !bIgnore && pNextMission->HasTag( szTag ) )
					{
						V_strncpy( m_szCycleNextMap.GetForModify(), pNextMission->BaseName, sizeof( m_szCycleNextMap ) );
						bFound = true;
						break;
					}
				}

				if ( bFound )
					break;

				for ( int iNextMission = 0; iNextMission < iMission; iNextMission++ )
				{
					const RD_Mission_t *pNextMission = ReactiveDropMissions::GetMission( iNextMission );
					Assert( pNextMission );

					bool bIgnore = false;
					FOR_EACH_VEC( IgnoreMissions, i )
					{
						if ( !V_stricmp( pNextMission->BaseName, IgnoreMissions[i] ) )
						{
							bIgnore = true;
							break;
						}
					}

					if ( !bIgnore && pNextMission->HasTag( szTag ) )
					{
						V_strncpy( m_szCycleNextMap.GetForModify(), pNextMission->BaseName, sizeof( m_szCycleNextMap ) );
						break;
					}
				}

				break;
			}
			case 2:
			{
				// Mode 2: like mode 1 but cycle in a random order (not remembered, so this isn't a shuffle and it can repeat a mission before all missions are played)

				int iChosenMission = -1;
				int nPossibilities = 0;
				for ( int iNextMission = 0; iNextMission < ReactiveDropMissions::CountMissions(); iNextMission++ )
				{
					if ( iMission == iNextMission )
						continue;

					const RD_Mission_t *pNextMission = ReactiveDropMissions::GetMission( iNextMission );
					Assert( pNextMission );

					bool bIgnore = false;
					FOR_EACH_VEC( IgnoreMissions, i )
					{
						if ( !V_stricmp( pNextMission->BaseName, IgnoreMissions[i] ) )
						{
							bIgnore = true;
							break;
						}
					}

					if ( !bIgnore && pNextMission->HasTag( szTag ) )
					{
						if ( RandomInt( 0, nPossibilities ) == 0 )
						{
							iChosenMission = iNextMission;
						}

						nPossibilities++;
					}
				}

				if ( iChosenMission != -1 )
				{
					const RD_Mission_t *pChosenMission = ReactiveDropMissions::GetMission( iChosenMission );
					Assert( pChosenMission );

					V_strncpy( m_szCycleNextMap.GetForModify(), pChosenMission->BaseName, sizeof( m_szCycleNextMap ) );
				}

				break;
			}
			default:
			{
				Warning( "rd_mapcycle_%s has an unknown mode number %d\n", szTag, iMode );
				break;
			}
			}
		}
	}

	SetGameState( ASW_GS_DEBRIEF );

	// Clear out any force ready state if we fail or succeed in the middle so that we always give a chance to award XP
	SetForceReady( ASW_FR_NONE );

	bool bSinglePlayer = false;

	CASW_Game_Resource *pGameResource = ASWGameResource();
	if ( pGameResource )
	{
		bSinglePlayer = pGameResource->IsOfflineGame();

		for ( int i = 0; i < ASW_MAX_READY_PLAYERS; i++ )
		{
			// make sure all players are marked as not ready to leave debrief
			pGameResource->m_bPlayerReady.Set( i, rd_ready_mark_override.GetBool() );

			if ( !rd_ready_mark_override.GetBool() )
			{
				CASW_Player *pPlayer = ToASW_Player( UTIL_PlayerByIndex( i + 1 ) );
				if ( pPlayer && !pGameResource->GetFirstMarineResourceForPlayer( pPlayer ) )
				{
					// player had no marines this round - mark them as ready in debriefing
					pGameResource->m_bPlayerReady.Set( i, true );
				}
			}
		}

		if ( bSuccess )
		{
			// If they got out with half the total squad health, we're calling that well done
			float fTotalHealthPercentage = 0.0f;
			int nNumMarines = 0;

			for ( int i = 0; i < pGameResource->GetMaxMarineResources(); i++ )
			{
				CASW_Marine_Resource *pMR = pGameResource->GetMarineResource( i );
				if ( pMR )
				{
					fTotalHealthPercentage += pMR->GetHealthPercent();
					nNumMarines++;
				}
			}

			pGameResource->OnMissionCompleted( ( nNumMarines <= 0 ) ? ( false ) : ( fTotalHealthPercentage / nNumMarines > 0.5f ) );
		}
		else
		{
			pGameResource->OnMissionFailed();
		}
	}

	DevMsg( "Set game state to debrief\n" );

	// set all players to FL_FROZEN and calc their XP serverside
	for ( int i = 1; i <= MAX_PLAYERS; i++ )
	{
		CBasePlayer *pPlayer = UTIL_PlayerByIndex( i );

		if ( pPlayer )
		{
			pPlayer->AddFlag( FL_FROZEN );
		}
	}

	if ( rd_clearhouse_on_mission_complete.GetBool() )
		ClearHouse();

	// freeze all the npcs, because Freeze(-1) doesn't work at all
	// and Freeze(9999) makes NPCs look frozen we disable think function	
	const bool bAllDead = ASWGameRules()->GetMissionManager()->AllMarinesDead();
	CAI_BaseNPC *npc = gEntList.NextEntByClass( ( CAI_BaseNPC * )NULL );
	if ( !bAllDead )
		while ( npc )
		{
			npc->SetThink( NULL );
			npc->StopLoopingSounds(); // helps against buzzers' noize, parasites still do idle sounds, but those are played on client
			npc = gEntList.NextEntByClass( npc );
		}
	// disable all the spawners
	CBaseEntity *ent = NULL;
	while ( ( ent = gEntList.FindEntityByClassname( ent, "asw_spawner" ) ) != NULL )
	{
		// BenLubar(deathmatch-improvements): reduce network traffic by 
		// disabling spawners instead of deleting them
		ent->SetThink( NULL );
	}
	// make marine invulnerable
	for ( int i = 0; pGameResource && i < pGameResource->GetMaxMarineResources(); i++ )
	{
		CASW_Marine_Resource *pMR = pGameResource->GetMarineResource( i );
		if ( pMR )
		{
			CASW_Marine *pMarine = pMR->GetMarineEntity();
			if ( pMarine && pMarine->GetHealth() > 0 )
			{
				pMarine->m_takedamage = DAMAGE_NO;
				pMarine->AddFlag( FL_FROZEN );
			}

			pMR->m_TimelineFriendlyFire.RecordFinalValue( 0.0f );
			pMR->m_TimelineKillsTotal.RecordFinalValue( 0.0f );
			pMR->m_TimelineAmmo.RecordFinalValue( pMarine ? pMarine->GetAllAmmoCount() : 0.0f );
			pMR->m_TimelineHealth.RecordFinalValue( pMarine ? pMarine->GetHealth() : 0.0f );
			pMR->m_TimelinePosX.RecordFinalValue( pMarine ? pMarine->GetAbsOrigin().x : pMR->m_TimelinePosX.GetValueAtInterp( 1.0f ) );
			pMR->m_TimelinePosY.RecordFinalValue( pMarine ? pMarine->GetAbsOrigin().y : pMR->m_TimelinePosY.GetValueAtInterp( 1.0f ) );
			pMR->m_TimelineScore.RecordFinalValue( 0.0f );
		}
	}

	// award medals	
#ifndef _DEBUG
	if ( !m_bCheated )
#endif
		m_Medals.AwardMedals();

	// create stats entity to network down all the interesting numbers
	m_hDebriefStats = dynamic_cast< CASW_Debrief_Stats * >( CreateEntityByName( "asw_debrief_stats" ) );

	if ( m_hDebriefStats.Get() == NULL )
	{
		Msg( "ASW: Error! Failed to create Debrief Stats\n" );
		return;
	}
	else if ( pGameResource )
	{
		// fill in debrief stats
		int iTotalKills = 0;
		float fWorstPenalty = 0;
		for ( int i = 0; i < ASW_MAX_MARINE_RESOURCES; i++ )
		{
			CASW_Marine_Resource *pMR = pGameResource->GetMarineResource( i );
			if ( !pMR )
				continue;

			int iKills = 0;
			if ( pMR->IsInhabited() )
			{
				CASW_Player *pPlayer = pMR->GetCommander();
				if ( pPlayer )
					iKills = pPlayer->FragCount();
				else
					iKills = pMR->m_iAliensKilled;
			}
			else
				iKills = pMR->m_iBotFrags;

			m_hDebriefStats->m_iKills.Set( i, iKills );
			iTotalKills += iKills;

			float acc = 0;
			if ( pMR->m_iPlayerShotsFired > 0 )
			{
				acc = float( pMR->m_iPlayerShotsFired - pMR->m_iPlayerShotsMissed ) / float( pMR->m_iPlayerShotsFired );
				acc *= 100.0f;
			}
			m_hDebriefStats->m_fAccuracy.Set( i, acc );
			m_hDebriefStats->m_iFF.Set( i, pMR->m_fFriendlyFireDamageDealt );
			m_hDebriefStats->m_iDamage.Set( i, pMR->m_fDamageTaken );
			m_hDebriefStats->m_iShotsFired.Set( i, pMR->m_iShotsFired );
			m_hDebriefStats->m_iShotsHit.Set( i, pMR->m_iPlayerShotsFired - pMR->m_iPlayerShotsMissed );
			m_hDebriefStats->m_iWounded.Set( i, pMR->m_bTakenWoundDamage );
			m_hDebriefStats->m_iAliensBurned.Set( i, pMR->m_iAliensBurned );
			m_hDebriefStats->m_iHealthHealed.Set( i, pMR->m_iMedicHealing );
			m_hDebriefStats->m_iFastHacks.Set( i, pMR->m_iFastDoorHacks + pMR->m_iFastComputerHacks );
			m_hDebriefStats->m_iAmmoDeployed.Set( i, pMR->m_iAmmoDeployed );
			m_hDebriefStats->m_iSentryGunsDeployed.Set( i, pMR->m_iSentryGunsDeployed );
			m_hDebriefStats->m_iSentryFlamerDeployed.Set( i, pMR->m_iSentryFlamerDeployed );
			m_hDebriefStats->m_iSentryFreezeDeployed.Set( i, pMR->m_iSentryFreezeDeployed );
			m_hDebriefStats->m_iSentryCannonDeployed.Set( i, pMR->m_iSentryCannonDeployed );
			m_hDebriefStats->m_iMedkitsUsed.Set( i, pMR->m_iMedkitsUsed );
			m_hDebriefStats->m_iFlaresUsed.Set( i, pMR->m_iFlaresUsed );
			m_hDebriefStats->m_iAdrenalineUsed.Set( i, pMR->m_iAdrenalineUsed );
			m_hDebriefStats->m_iTeslaTrapsDeployed.Set( i, pMR->m_iTeslaTrapsDeployed );
			m_hDebriefStats->m_iFreezeGrenadesThrown.Set( i, pMR->m_iFreezeGrenadesThrown );
			m_hDebriefStats->m_iElectricArmorUsed.Set( i, pMR->m_iElectricArmorUsed );
			m_hDebriefStats->m_iHealGunHeals.Set( i, pMR->m_iHealGunHeals );
			m_hDebriefStats->m_iHealBeaconHeals.Set( i, pMR->m_iHealBeaconHeals );
			m_hDebriefStats->m_iHealGunHeals_Self.Set( i, pMR->m_iHealGunHeals_Self );
			m_hDebriefStats->m_iHealBeaconHeals_Self.Set( i, pMR->m_iHealBeaconHeals_Self );
			m_hDebriefStats->m_iDamageAmpsUsed.Set( i, pMR->m_iDamageAmpsUsed );
			m_hDebriefStats->m_iHealBeaconsDeployed.Set( i, pMR->m_iHealBeaconsDeployed );
			m_hDebriefStats->m_iMedkitHeals_Self.Set( i, pMR->m_iMedkitHeals_Self );
			m_hDebriefStats->m_iGrenadeExtinguishMarine.Set( i, pMR->m_iGrenadeExtinguishMarine );
			m_hDebriefStats->m_iGrenadeFreezeAlien.Set( i, pMR->m_iGrenadeFreezeAlien );
			m_hDebriefStats->m_iDamageAmpAmps.Set( i, pMR->m_iDamageAmpAmps );
			m_hDebriefStats->m_iNormalArmorReduction.Set( i, pMR->m_iNormalArmorReduction );
			m_hDebriefStats->m_iElectricArmorReduction.Set( i, pMR->m_iElectricArmorReduction );
			m_hDebriefStats->m_iHealAmpGunHeals.Set( i, pMR->m_iHealAmpGunHeals );
			m_hDebriefStats->m_iHealAmpGunAmps.Set( i, pMR->m_iHealAmpGunAmps );
			m_hDebriefStats->m_iMedRifleHeals.Set( i, pMR->m_iMedRifleHeals );
			m_hDebriefStats->m_iBiomassIgnited.Set( i, pMR->m_iBiomassIgnited );

			// Set starting equips for the marine
			m_hDebriefStats->m_iStartingEquip0.Set( i, pMR->m_iInitialWeaponsInSlots[0] );
			m_hDebriefStats->m_iStartingEquip1.Set( i, pMR->m_iInitialWeaponsInSlots[1] );
			m_hDebriefStats->m_iStartingEquip2.Set( i, pMR->m_iInitialWeaponsInSlots[2] );

			pMR->m_WeaponStats.Sort( &CASW_Marine_Resource::CompareWeaponStats );
			for ( int j = 0; j < 8 && j < pMR->m_WeaponStats.Count(); j++ )
			{
				m_hDebriefStats->SetWeaponStats( i, j,
					pMR->m_WeaponStats[j].m_WeaponClass,
					pMR->m_WeaponStats[j].m_nDamage,
					pMR->m_WeaponStats[j].m_nFFDamage,
					pMR->m_WeaponStats[j].m_nShotsFired,
					pMR->m_WeaponStats[j].m_nShotsHit,
					pMR->m_WeaponStats[j].m_nKills );
			}

			// store the worst penalty for use later when penalizing skill points
			float fPenalty = pMR->m_fFriendlyFireDamageDealt * 2 + pMR->m_fDamageTaken;
			if ( fPenalty > fWorstPenalty )
				fWorstPenalty = fPenalty;

			// award an additional skill point if they acheived certain medals in this mission:
			int iMedalPoints = 0;
#ifdef AWARD_SKILL_POINTS_FOR_MEDALS
			iMedalPoints += ( m_Medals.HasMedal( MEDAL_PERFECT, pMR, true ) ) ? 1 : 0;
			iMedalPoints += ( m_Medals.HasMedal( MEDAL_IRON_HAMMER, pMR, true ) ) ? 1 : 0;
			iMedalPoints += ( m_Medals.HasMedal( MEDAL_INCENDIARY_DEFENCE, pMR, true ) ) ? 1 : 0;
			iMedalPoints += ( m_Medals.HasMedal( MEDAL_IRON_SWORD, pMR, true ) ) ? 1 : 0;
			iMedalPoints += ( m_Medals.HasMedal( MEDAL_SWARM_SUPPRESSION, pMR, true ) ) ? 1 : 0;
			iMedalPoints += ( m_Medals.HasMedal( MEDAL_SILVER_HALO, pMR, true ) ) ? 1 : 0;
			iMedalPoints += ( m_Medals.HasMedal( MEDAL_GOLDEN_HALO, pMR, true ) ) ? 1 : 0;
			iMedalPoints += ( m_Medals.HasMedal( MEDAL_ELECTRICAL_SYSTEMS_EXPERT, pMR, true ) ) ? 1 : 0;
			iMedalPoints += ( m_Medals.HasMedal( MEDAL_COMPUTER_SYSTEMS_EXPERT, pMR, true ) ) ? 1 : 0;
#endif

			// give each marine a base of 4 skill points
			m_hDebriefStats->m_iSkillPointsAwarded.Set( i, ASW_SKILL_POINTS_PER_MISSION + iMedalPoints );

			// tell everyone about bouncing shot kills for debugging:
			if ( pMR->m_iAliensKilledByBouncingBullets > 0 )
			{
				char buffer[256];
				Q_snprintf( buffer, sizeof( buffer ), "%d", pMR->m_iAliensKilledByBouncingBullets );
				UTIL_ClientPrintAll( ASW_HUD_PRINTTALKANDCONSOLE, "#asw_bouncing_kills", pMR->GetProfile()->m_ShortName, buffer );
			}
		}

		// penalize skill points if each marine's penalty is over threshold of 60 and at least 20% of the worst marine's penalty
#ifdef PENALIZE_SKILL_POINTS
		for ( int i = 0; i < ASW_MAX_MARINE_RESOURCES; i++ )
		{
			CASW_Marine_Resource *pMR = pGameResource->GetMarineResource( i );
			if ( !pMR )
				continue;

			float fPenalty = pMR->m_fFriendlyFireDamageDealt * 2 + pMR->m_fDamageTaken;
			if ( fPenalty > 60 && fPenalty >= fWorstPenalty * 0.8f )
			{
				int points = m_hDebriefStats->m_iSkillPointsAwarded[i];
				m_hDebriefStats->m_iSkillPointsAwarded.Set( i, points - 1 );
				// double penalty if they've done really badly
				if ( fPenalty >= 200 )
				{
					points = m_hDebriefStats->m_iSkillPointsAwarded[i];
					m_hDebriefStats->m_iSkillPointsAwarded.Set( i, points - 1 );
				}
			}

			// a marine is dead, give him nothing
			if ( pMR->GetHealthPercent() <= 0 )
			{
				if ( asw_campaign_death.GetBool() )
				{
					m_hDebriefStats->m_iSkillPointsAwarded.Set( i, 0 );
				}
				else
				{
					// penalize one skill point if they died
					int points = MAX( 0, m_hDebriefStats->m_iSkillPointsAwarded[i] - 1 );
					m_hDebriefStats->m_iSkillPointsAwarded.Set( i, points );
				}
			}
		}
#endif

		// fill in debrief stats team stats/time taken/etc
		m_hDebriefStats->m_iTotalKills = iTotalKills;
		m_hDebriefStats->m_fTimeTaken = gpGlobals->curtime - m_fMissionStartedTime;

		for ( int i = 0; i < ASW_MAX_MARINE_RESOURCES; i++ )
		{
			CASW_Marine_Resource *pMR = pGameResource->GetMarineResource( i );
			if ( pMR && pMR->m_iScore > -1 )
			{
				m_hDebriefStats->m_iLeaderboardScore.GetForModify( i ) = pMR->m_iScore;
			}
			else if ( pMR && pMR->m_flFinishedMissionTime >= 0 )
			{
				m_hDebriefStats->m_iLeaderboardScore.GetForModify( i ) = int( pMR->m_flFinishedMissionTime * 1000 );
			}
			else
			{
				m_hDebriefStats->m_iLeaderboardScore.GetForModify( i ) = int( m_hDebriefStats->m_fTimeTaken * 1000 );
			}
		}

		// calc the speedrun time
		int speedrun_time = 180;	// default of 3 mins
		if ( GetWorldEntity() && GetSpeedrunTime() > 0 )
			speedrun_time = GetSpeedrunTime();

		// put in the previous best times/kills for the debrief stats
		const char *mapName = STRING( gpGlobals->mapname );
		if ( MapScores() )
		{
			m_hDebriefStats->m_fBestTimeTaken = MapScores()->GetBestTime( mapName, GetSkillLevel() );
			m_hDebriefStats->m_iBestKills = MapScores()->GetBestKills( mapName, GetSkillLevel() );
			m_hDebriefStats->m_iSpeedrunTime = speedrun_time;

			if ( GetMissionSuccess() )
			{
				// notify players if we beat the speedrun time, even if we already have uber unlocked
				m_hDebriefStats->m_bBeatSpeedrunTime = ( m_hDebriefStats->m_fTimeTaken.Get() < speedrun_time );

				// notify the mapscores of our data so it can save it
				MapScores()->OnMapCompleted( mapName, GetSkillLevel(), m_hDebriefStats->m_fTimeTaken.Get(), m_hDebriefStats->m_iTotalKills.Get() );
			}
		}

		m_hDebriefStats->Spawn();

		for ( int i = 1; i <= gpGlobals->maxClients; i++ )
		{
			CASW_Player *pOtherPlayer = ToASW_Player( UTIL_PlayerByIndex( i ) );
			if ( pOtherPlayer )
			{
				pOtherPlayer->AwardExperience();
			}
		}
	}

	// reset the progress if we finish the tutorial successfully
	if ( IsTutorialMap() && bSuccess )
	{
		asw_tutorial_save_stage.SetValue( 0 );
	}

	// shut everything up
	StopAllAmbientSounds();

	// store stats for uploading
	CASW_GameStats.Event_MissionComplete( m_bMissionSuccess, m_nFailAdvice, ASWFailAdvice()->GetFailAdviceStatus() );

	// set a timer to remove all the aliens once clients have had a chance to fade out
	m_fRemoveAliensTime = gpGlobals->curtime + 2.4f;
}

void CAlienSwarm::RemoveAllAliens()
{
	m_fRemoveAliensTime = 0;
	for ( int i = 0; i < ASWSpawnManager()->GetNumAlienClasses(); i++ )
	{
		string_t iszClassName = ASWSpawnManager()->GetAlienClass( i )->m_iszAlienClass;
		CBaseEntity *ent = NULL;
		while ( ( ent = gEntList.FindEntityByClassnameFast( ent, iszClassName ) ) != NULL )
		{
			UTIL_Remove( ent );
		}
	}
}

void CAlienSwarm::RemoveNoisyWeapons()
{
	CBaseEntity *ent = NULL;
	while ( ( ent = gEntList.FindEntityByClassname( ent, "asw_weapon_chainsaw" ) ) != NULL )
	{
		UTIL_Remove( ent );
	}
}

// send the minimap line draw to everyone
void CAlienSwarm::BroadcastMapLine(CASW_Player *pPlayer, int linetype, int world_x, int world_y)
{
	CRecipientFilter filter;
	filter.AddAllPlayers();
	filter.RemoveRecipient(pPlayer);

	UserMessageBegin( filter, "ASWMapLine" ); // create message 
	WRITE_BYTE( (char) linetype );
	WRITE_BYTE( pPlayer->entindex() );
	WRITE_LONG( world_x );	// send the location of the map line dot
	WRITE_LONG( world_y );
	MessageEnd(); //send message
}

void CAlienSwarm::BlipSpeech(int iMarine)
{
	if (!ASWGameResource())
		return;
	CASW_Marine_Resource *pMR = ASWGameResource()->GetMarineResource(iMarine);
	if (!pMR || !pMR->GetProfile() || !pMR->GetProfile()->CanHack())		// check the requested speech is coming from a tech
		return;
	// check no-one else said some blip speech recently
	if (gpGlobals->curtime - m_fLastBlipSpeechTime < ASW_BLIP_SPEECH_INTERVAL * 0.8f)
		return;

	CASW_Marine *pMarine = pMR->GetMarineEntity();
	if (!pMarine)
		return;

	CASW_MarineSpeech *pSpeech = pMarine->GetMarineSpeech();
	if (!pSpeech)
		return;

	IGameEvent * event = gameeventmanager->CreateEvent( "scanner_important" );
	if ( event )
	{
		gameeventmanager->FireEvent( event );
	}

	if ( !m_bPlayedBlipSpeech || random->RandomFloat() < asw_blip_speech_chance.GetFloat() )
	{
		pSpeech->Chatter(CHATTER_SCANNER);
		m_bPlayedBlipSpeech = true;
	}

	m_fLastBlipSpeechTime = gpGlobals->curtime;
}

void CAlienSwarm::MarineKilled( CASW_Marine *pMarine, const CTakeDamageInfo &info )
{
	if ( GetCampaignSave() )
	{
		GetCampaignSave()->OnMarineKilled();
	}

	for ( int i = 0; i < IASW_Marines_Past_Area_List::AutoList().Count(); i++ )
	{
		CASW_Marines_Past_Area *pArea = static_cast< CASW_Marines_Past_Area* >( IASW_Marines_Past_Area_List::AutoList()[ i ] );
		pArea->OnMarineKilled( pMarine );
	}

	if ( !ASWDeathmatchMode() && rd_quake_sounds.GetInt() >= 2 )
	{
		CASW_Marine *pKillerMarine = CASW_Marine::AsMarine( info.GetAttacker() );
		CASW_Player *pPlayer = pKillerMarine && pKillerMarine->IsInhabited() ? pKillerMarine->GetCommander() : NULL;
		if ( pPlayer )
		{
			if ( gpGlobals->curtime - pPlayer->m_fLastFragTime < rd_killingspree_time_limit.GetFloat() )
			{
				++pPlayer->m_iKillingSpree;
				switch ( pPlayer->m_iKillingSpree )
				{
				case 2:
					BroadcastSound( "DM.double_kill" );
					break;
				case 3:
					BroadcastSound( "DM.tripple_kill" );
					break;
				case 4:
					BroadcastSound( "DM.monster_kill" );
					break;
				case 5:
					BroadcastSound( "DM.godlike" );
					break;
				}
			}
			else
			{
				pPlayer->m_iKillingSpree = 1;
			}

			pPlayer->m_fLastFragTime = gpGlobals->curtime;
		}
	}
}

void CAlienSwarm::MarineKnockedOut( CASW_Marine *pMarine )
{
	if (GetMissionManager())
		GetMissionManager()->CheckMissionComplete();
}

class GroundNodeFilter : public INearestNodeFilter
{
public:
	virtual bool IsValid( CAI_Node *pNode ) 
	{
		return pNode->GetType() == NODE_GROUND;
	};
	virtual bool ShouldContinue() 
	{
		return true;
	}
};

void CAlienSwarm::AlienKilled(CBaseEntity *pAlien, const CTakeDamageInfo &info)
{
	if (asw_debug_alien_damage.GetBool())
	{
		Msg("Alien %s killed by attacker %s inflicter %s\n", pAlien->GetClassname(), 
			info.GetAttacker() ? info.GetAttacker()->GetClassname() : "unknown",
			info.GetInflictor() ? info.GetInflictor()->GetClassname() : "unknown");
	}
	if (GetMissionManager())
		GetMissionManager()->AlienKilled(pAlien);

	if ( ASWHoldoutMode() )
	{
		ASWHoldoutMode()->OnAlienKilled( pAlien, info );
	}

	CASW_Shieldbug* pSB = NULL;
	if ( pAlien && pAlien->Classify() == CLASS_ASW_SHIELDBUG )
	{
		pSB = assert_cast<CASW_Shieldbug*>(pAlien);
	}

	CASW_Marine* pMarine = NULL;
	CBaseEntity* pAttacker = info.GetAttacker();
	if ( pAttacker && pAttacker->Classify() == CLASS_ASW_MARINE )
		pMarine = assert_cast<CASW_Marine*>(pAttacker);

	if ( pMarine )
	{
		CASW_Player *pCommander = pMarine->GetCommander();
		if ( pCommander && pMarine->IsInhabited() )
		{
			int iClassIndex = GetAlienClassIndex( pAlien );
			if ( iClassIndex >= 0 )
			{
				CSingleUserRecipientFilter filter( pCommander );
				filter.MakeReliable();
				UserMessageBegin( filter, "RDAlienKillStat" );
					WRITE_SHORT( iClassIndex );
				MessageEnd();
			}
		}

		CASW_Marine_Resource *pMR = pMarine->GetMarineResource();
		if ( pMR )
		{
			CASW_Game_Resource *pGameResource = ASWGameResource();
			if ( pGameResource && ( !pAlien || pAlien->Classify() != CLASS_ASW_GRUB ) )
			{
				if ( pMR->GetCommander() && pMR->IsInhabited() && pGameResource->GetNumMarines( NULL, true ) > 3 )
				{
					pMR->m_iAliensKilledSinceLastFriendlyFireIncident++;
					if ( pMR->m_iAliensKilledSinceLastFriendlyFireIncident > 25 && !pMR->m_bAwardedFFPartialAchievement )
					{
						pMR->m_bAwardedFFPartialAchievement = true;
						pMR->GetCommander()->AwardAchievement( ACHIEVEMENT_ASW_KILL_WITHOUT_FRIENDLY_FIRE );
					}
				}

				if ( pMarine->GetDamageBuffEndTime() > gpGlobals->curtime && pMarine->m_hLastBuffGrenade.Get() && pMarine->m_hLastBuffGrenade->GetBuffedMarineCount() >= 4 )
				{
					pGameResource->m_iAliensKilledWithDamageAmp++;

					if ( !pGameResource->m_bAwardedDamageAmpAchievement && pMR->GetCommander() && pMR->IsInhabited() )
					{
						static const int nRequiredDamageAmpKills = 15;
						if ( pGameResource->m_iAliensKilledWithDamageAmp >= nRequiredDamageAmpKills )
						{
							pGameResource->m_bAwardedDamageAmpAchievement = true;
							for ( int i = 1; i <= gpGlobals->maxClients; i++ )	
							{
								CASW_Player *pPlayer = ToASW_Player( UTIL_PlayerByIndex( i ) );
								CASW_Marine *pOtherMarine = pPlayer ? CASW_Marine::AsMarine( pPlayer->GetNPC() ) : NULL;
								if ( !pPlayer || !pPlayer->IsConnected() || !pOtherMarine )
									continue;

								pPlayer->AwardAchievement( ACHIEVEMENT_ASW_GROUP_DAMAGE_AMP );
								if ( pOtherMarine->GetMarineResource() )
								{
									pOtherMarine->GetMarineResource()->m_bDamageAmpMedal = true;
								}
							}
						}
					}
				}
			}

			//if (pMarine->m_fDieHardTime > 0)
			//{
				//pMR->m_iLastStandKills++;
			//}

			// count rad volume kills
			if (pAlien && pAlien->Classify() != CLASS_EARTH_FAUNA && pAlien->Classify() != CLASS_ASW_GRUB)
			{
				int nOldBarrelKills = pMR->m_iBarrelKills;
				CASW_Radiation_Volume *pRad = dynamic_cast<CASW_Radiation_Volume*>(info.GetInflictor());
				if (pRad)
				{
					pMR->m_iBarrelKills++;
				}

				// count prop kills
				CPhysicsProp *pProp = dynamic_cast<CPhysicsProp*>(info.GetInflictor());
				if (pProp)
				{
					pMR->m_iBarrelKills++;
				}
				if ( pMR->GetCommander() && pMR->IsInhabited() )
				{
					if ( nOldBarrelKills < asw_medal_barrel_kills.GetInt() && pMR->m_iBarrelKills >= asw_medal_barrel_kills.GetInt() )
					{
						pMR->GetCommander()->AwardAchievement( ACHIEVEMENT_ASW_BARREL_KILLS );
					}
				}
			}

			if ( pMR->m_iSavedLife < asw_medal_lifesaver_kills.GetInt() && pAlien->Classify() == CLASS_ASW_DRONE )
			{
				// check if the alien was after another marine and was close to him
				if (pAlien->GetEnemy() != pMarine && pAlien->GetEnemy()
					&& pAlien->GetEnemy()->Classify() == CLASS_ASW_MARINE
					&& pAlien->GetEnemy()->GetHealth() <= 5
					&& pAlien->GetAbsOrigin().DistTo(pAlien->GetEnemy()->GetAbsOrigin()) < asw_medal_lifesaver_dist.GetFloat())
					pMR->m_iSavedLife++;
			}

			if ( pSB )
			{
				pMR->m_iShieldbugsKilled++;
			}

			if ( pAlien->Classify() == CLASS_ASW_PARASITE )
			{
				CASW_Parasite *pPara = assert_cast<CASW_Parasite*>(pAlien);
				if (!pPara->m_bDefanged)
				{
					pMR->m_iParasitesKilled++;
				}
			}
		}

		if (!m_bDoneCrashShieldbugConv)
		{			
			if (pSB && random->RandomFloat() < 1.0f)
			{				
				// see if crash was nearby
				CASW_Game_Resource *pGameResource = ASWGameResource();
				if (pGameResource)
				{
					CASW_Marine *pCrash = NULL;
					if (pMarine->GetMarineProfile()->m_VoiceType == ASW_VOICE_CRASH && pMarine->GetHealth() > 0)
						pCrash = pMarine;
					if (!pCrash)
					{
						for (int i=0;i<pGameResource->GetMaxMarineResources();i++)
						{
							CASW_Marine_Resource *pOtherMR = pGameResource->GetMarineResource(i);
							CASW_Marine *pOtherMarine = pOtherMR ? pOtherMR->GetMarineEntity() : NULL;
							if (pOtherMarine && (pMarine->GetAbsOrigin().DistTo(pOtherMarine->GetAbsOrigin()) < 600)
										&& pOtherMarine->GetHealth() > 0 && pOtherMarine->GetMarineProfile()
										&& pOtherMarine->GetMarineProfile()->m_VoiceType == ASW_VOICE_CRASH)
							{
								pCrash = pOtherMarine;
								break;
							}
						}
					}
					if (pCrash)
					{
						if (CASW_MarineSpeech::StartConversation(CONV_BIG_ALIEN, pMarine))
						{
							m_bDoneCrashShieldbugConv = true;
							return;
						}
					}
				}
			}
		}
			
		// check for doing an conversation from this kill
		if (pMarine->GetMarineProfile())
		{
			if (gpGlobals->curtime > m_fNextWWKillConv)
			{
				if (pMarine->GetMarineProfile()->m_VoiceType == ASW_VOICE_WILDCAT)
				{
					m_fNextWWKillConv = gpGlobals->curtime + random->RandomInt(asw_ww_chatter_interval_min.GetInt(), asw_ww_chatter_interval_max.GetInt());
					if (CASW_MarineSpeech::StartConversation(CONV_WILDCAT_KILL, pMarine))
						return;
				}
				else if (pMarine->GetMarineProfile()->m_VoiceType == ASW_VOICE_WOLFE)
				{
					m_fNextWWKillConv = gpGlobals->curtime + random->RandomInt(asw_ww_chatter_interval_min.GetInt(), asw_ww_chatter_interval_max.GetInt());
					if (CASW_MarineSpeech::StartConversation(CONV_WOLFE_KILL, pMarine))
						return;
				}
			}

			if (gpGlobals->curtime > m_fNextCompliment)
			{
				if (m_bSargeAndJaeger && pMarine->GetMarineProfile()->m_VoiceType == ASW_VOICE_SARGE)
				{
					CASW_Marine *pOtherMarine = ASWGameResource()->FindMarineByVoiceType(ASW_VOICE_JAEGER);
					if (CASW_MarineSpeech::StartConversation(CONV_COMPLIMENT_SARGE, pOtherMarine))
						m_fNextCompliment = gpGlobals->curtime + random->RandomInt(asw_compliment_chatter_interval_min.GetInt(), asw_compliment_chatter_interval_max.GetInt());
					else
						m_fNextCompliment = gpGlobals->curtime + random->RandomInt(asw_compliment_chatter_interval_min.GetInt()*0.2f, asw_compliment_chatter_interval_max.GetInt()-0.2f);
				}
				else if (m_bSargeAndJaeger && pMarine->GetMarineProfile()->m_VoiceType == ASW_VOICE_JAEGER)
				{
					CASW_Marine *pOtherMarine = ASWGameResource()->FindMarineByVoiceType(ASW_VOICE_SARGE);
					if (CASW_MarineSpeech::StartConversation(CONV_COMPLIMENT_JAEGER, pOtherMarine))
						m_fNextCompliment = gpGlobals->curtime + random->RandomInt(asw_compliment_chatter_interval_min.GetInt(), asw_compliment_chatter_interval_max.GetInt());
					else
						m_fNextCompliment = gpGlobals->curtime + random->RandomInt(asw_compliment_chatter_interval_min.GetInt()*0.2f, asw_compliment_chatter_interval_max.GetInt()-0.2f);
				}
				else if (m_bWolfeAndWildcat && pMarine->GetMarineProfile()->m_VoiceType == ASW_VOICE_WILDCAT)
				{
					CASW_Marine *pOtherMarine = ASWGameResource()->FindMarineByVoiceType(ASW_VOICE_WOLFE);
					if (CASW_MarineSpeech::StartConversation(CONV_COMPLIMENT_WILDCAT,  pOtherMarine))
						m_fNextCompliment = gpGlobals->curtime + random->RandomInt(asw_compliment_chatter_interval_min.GetInt(), asw_compliment_chatter_interval_max.GetInt());
					else
						m_fNextCompliment = gpGlobals->curtime + random->RandomInt(asw_compliment_chatter_interval_min.GetInt()*0.2f, asw_compliment_chatter_interval_max.GetInt()-0.2f);
				}
				else if (m_bWolfeAndWildcat && pMarine->GetMarineProfile()->m_VoiceType == ASW_VOICE_WOLFE)
				{
					CASW_Marine *pOtherMarine = ASWGameResource()->FindMarineByVoiceType(ASW_VOICE_WILDCAT);
					if (CASW_MarineSpeech::StartConversation(CONV_COMPLIMENT_WOLFE, pOtherMarine))
						m_fNextCompliment = gpGlobals->curtime + random->RandomInt(asw_compliment_chatter_interval_min.GetInt(), asw_compliment_chatter_interval_max.GetInt());
					else
						m_fNextCompliment = gpGlobals->curtime + random->RandomInt(asw_compliment_chatter_interval_min.GetInt()*0.2f, asw_compliment_chatter_interval_max.GetInt()-0.2f);
				}
			}
		}
	}

	CASW_Sentry_Top *pSentry = NULL;
	if ( !pMarine )
	{
		pSentry = dynamic_cast< CASW_Sentry_Top* >(pAttacker);
	}
	
	if ( pSentry )
	{
		// count sentry kills
		if ( pSentry->GetSentryBase() && pSentry->GetSentryBase()->m_hDeployer.Get())
		{
			pMarine = pSentry->GetSentryBase()->m_hDeployer.Get();
			if (pMarine && pMarine->GetMarineResource())
			{
				pMarine->GetMarineResource()->m_iSentryKills++;
			}
		}
	}

	CFire *pFire = NULL;
	if ( !pMarine )
	{
		pFire = dynamic_cast< CFire* >(pAttacker);
	}

	if ( pFire )
	{
		CBaseEntity* pOwner = pFire->GetOwner();
		if ( pOwner && pOwner->Classify() == CLASS_ASW_MARINE )
			pMarine = assert_cast< CASW_Marine* >(pOwner);
	}

	// send a game event for achievements to use
	IGameEvent *pEvent = gameeventmanager->CreateEvent( "alien_died", false );
	if ( !pEvent )
		return;

	CBaseEntity *pWeapon = NULL;

	if ( pSentry )
	{
		pWeapon = pSentry;
	}
	else if ( pFire )
	{
		pWeapon = pFire;
	}
	else
	{
		pWeapon = info.GetWeapon();
	}
	
	pEvent->SetInt( "alien", pAlien ? pAlien->Classify() : 0 );
	pEvent->SetInt( "marine", pMarine ? pMarine->entindex() : 0 );		
	pEvent->SetInt( "weapon", pWeapon ? pWeapon->Classify() : 0 );

	gameeventmanager->FireEvent( pEvent );

	// riflemod: added frags counter
	if ( !ASWDeathmatchMode() && pMarine && pMarine->GetCommander() && ( !pAlien || pAlien->Classify() != CLASS_ASW_GRUB ) )
	{
		CASW_Marine_Resource *pMR = pMarine->GetMarineResource();
		if (pMR)
		{
			pMR->m_iAliensKilled++;
			pMR->m_TimelineKillsTotal.RecordValue(1.0f);
		}

		CASW_Player *pPlayer = pMarine->GetCommander();
		int nFrags = 0;
		if ( pMarine->IsInhabited() && pPlayer )
		{
			pPlayer->IncrementFragCount( 1 );
			nFrags = pPlayer->FragCount();
		}
		else
		{
			if (pMR)
			{
				pMR->m_iBotFrags += 1;
				nFrags = pMR->m_iBotFrags;
			}
		}

		// reactivedrop: if enabled we spawn medkits every rd_spawn_medkits.GetInt() frags
		// and ammo every rd_spawn_ammo.GetInt() frags
		if ( rd_spawn_medkits.GetInt() || rd_spawn_ammo.GetInt() )
		{
			const int iFragsForMedkit = rd_spawn_medkits.GetInt();
			const int iFragsForAmmo = rd_spawn_ammo.GetInt();

			if ( ( iFragsForMedkit && nFrags % iFragsForMedkit == 0 ) || ( iFragsForAmmo && nFrags % iFragsForAmmo == 0 ) )	//DRAVEN ~FRAGD0~
			{
				CAI_Network *pNetwork = pMarine->GetNavigator() ? pMarine->GetNavigator()->GetNetwork() : NULL;
				if ( pNetwork && pAlien )
				{
					GroundNodeFilter filter;
					int nNode = pNetwork->NearestNodeToPoint(NULL, pAlien->GetAbsOrigin(), false, &filter);
					if (nNode != NO_NODE)
					{
						CAI_Node *pNode = pNetwork->GetNode(nNode);

						if (pNode && pNode->GetType() == NODE_GROUND )
						{
							Vector vecDest = pNode->GetPosition(HULL_HUMAN);
							if ( iFragsForMedkit && nFrags % iFragsForMedkit == 0 )														//DRAVEN ~FRAGD0~
							{
								CASW_Pickup_Weapon_Medkit *pMedkit = (CASW_Pickup_Weapon_Medkit *) CreateEntityByName( "asw_pickup_medkit" );
								pMedkit->m_iBulletsInGun = 1;
								UTIL_SetOrigin( pMedkit, pAlien->GetAbsOrigin() + Vector( 0, 0, 10) ); // raise it slightly up, so it doesn't spawn inside ground
								DispatchSpawn( pMedkit );
							}

							if ( iFragsForAmmo && nFrags % iFragsForAmmo == 0 )															//DRAVEN ~FRAGD0~
							{
								CBaseEntity *pAmmoDrop = CreateEntityByName( "asw_ammo_drop" );	
								UTIL_SetOrigin( pAmmoDrop, vecDest );
								DispatchSpawn( pAmmoDrop );
								//pAmmoDrop->Spawn();
							}
						}
					}
				}
			}
		}
	}
}

#endif /* not CLIENT_DLL */

bool CAlienSwarm::ShouldCollide( int collisionGroup0, int collisionGroup1 )
{	
	// HL2 treats movement and tracing against players the same, so just remap here
	if ( collisionGroup0 == COLLISION_GROUP_PLAYER_MOVEMENT )
	{
		collisionGroup0 = COLLISION_GROUP_PLAYER;
	}

	if( collisionGroup1 == COLLISION_GROUP_PLAYER_MOVEMENT )
	{
		collisionGroup1 = COLLISION_GROUP_PLAYER;
	}
	
	// aliens shouldn't walk into players (but will this make them get stuck? - might need some extra push out stuff for em)
	if ((collisionGroup0 == ASW_COLLISION_GROUP_ALIEN || collisionGroup0 == ASW_COLLISION_GROUP_BIG_ALIEN)
		&& collisionGroup1 == COLLISION_GROUP_PLAYER)
	{
		return true;
	}

	// grubs don't collide with one another
	if (collisionGroup0 == ASW_COLLISION_GROUP_GRUBS
		&& collisionGroup1 == ASW_COLLISION_GROUP_GRUBS)
	{
		//Msg("Skipped houndeye col\n");
		return false;
	}

	// reactivedrop: bots don't collide with one another
	if ( collisionGroup0 == ASW_COLLISION_GROUP_BOTS && collisionGroup1 == ASW_COLLISION_GROUP_BOTS )
	{
		return false;
	}

	// asw test, let drones pass through one another
#ifndef CLIENT_DLL
	if ( collisionGroup0 == ASW_COLLISION_GROUP_ALIEN && collisionGroup1 == ASW_COLLISION_GROUP_ALIEN && asw_springcol.GetBool() )
	{
		return false;
	}
#endif

	if ( collisionGroup0 > collisionGroup1 )
	{
		// swap so that lowest is always first
		V_swap( collisionGroup0, collisionGroup1 );
	}

#define SHOULD_COLLIDE( group0, group1, should ) \
	ASSERT_INVARIANT( group0 <= group1 ); \
	if ( collisionGroup0 == group0 && collisionGroup1 == group1 ) \
		return should
#define ALWAYS_COLLIDE( group, should ) \
	if ( collisionGroup0 == group || collisionGroup1 == group ) \
		return should

	// players don't collide with buzzers (since the buzzers use vphysics collision and that makes the player get stuck)
	SHOULD_COLLIDE( COLLISION_GROUP_PLAYER, ASW_COLLISION_GROUP_BUZZER, false );

	// this collision group only blocks drones
	SHOULD_COLLIDE( ASW_COLLISION_GROUP_ALIEN, ASW_COLLISION_GROUP_BLOCK_DRONES, true );
	SHOULD_COLLIDE( ASW_COLLISION_GROUP_BLOCK_DRONES, ASW_COLLISION_GROUP_BIG_ALIEN, true );
	ALWAYS_COLLIDE( ASW_COLLISION_GROUP_BLOCK_DRONES, false );

	// marines don't collide with other marines
	if ( !asw_marine_collision.GetBool() )
	{
		SHOULD_COLLIDE( COLLISION_GROUP_PLAYER, COLLISION_GROUP_PLAYER, false );
	}

	// eggs and parasites don't collide
	SHOULD_COLLIDE( ASW_COLLISION_GROUP_PARASITE, ASW_COLLISION_GROUP_EGG, false );
	
	// so our parasites don't stop gibs from flying out of people alongside them
	SHOULD_COLLIDE( COLLISION_GROUP_DEBRIS, ASW_COLLISION_GROUP_PARASITE, false );

	// parasites don't get blocked by big aliens (reactivedrop: and normal aliens)
	SHOULD_COLLIDE( ASW_COLLISION_GROUP_PARASITE, ASW_COLLISION_GROUP_BIG_ALIEN, false );
	SHOULD_COLLIDE( ASW_COLLISION_GROUP_PARASITE, ASW_COLLISION_GROUP_ALIEN, false );

	// turn our prediction collision into normal player collision and pass it up
	if ( collisionGroup0 == ASW_COLLISION_GROUP_MARINE_POSITION_PREDICTION )
		collisionGroup0 = COLLISION_GROUP_PLAYER;
	if ( collisionGroup1 == ASW_COLLISION_GROUP_MARINE_POSITION_PREDICTION )
		collisionGroup1 = COLLISION_GROUP_PLAYER;

	if ( collisionGroup0 > collisionGroup1 )
	{
		// we may have just messed up the order, so fix it if we did
		V_swap( collisionGroup0, collisionGroup1 );
	}

	// grubs don't collide with zombies, aliens or marines
	SHOULD_COLLIDE( COLLISION_GROUP_PLAYER, ASW_COLLISION_GROUP_GRUBS, false );
	SHOULD_COLLIDE( COLLISION_GROUP_NPC, ASW_COLLISION_GROUP_GRUBS, false );
	SHOULD_COLLIDE( ASW_COLLISION_GROUP_GRUBS, ASW_COLLISION_GROUP_ALIEN, false );
	SHOULD_COLLIDE( ASW_COLLISION_GROUP_GRUBS, ASW_COLLISION_GROUP_BIG_ALIEN, false );
	SHOULD_COLLIDE( ASW_COLLISION_GROUP_GRUBS, ASW_COLLISION_GROUP_BOTS, false );

	// reactivedrop: bots don't collide with zombies, aliens, marines, grenades and sentries
	SHOULD_COLLIDE( COLLISION_GROUP_PLAYER, ASW_COLLISION_GROUP_BOTS, false );
	SHOULD_COLLIDE( COLLISION_GROUP_NPC, ASW_COLLISION_GROUP_BOTS, false );
	SHOULD_COLLIDE( ASW_COLLISION_GROUP_GRENADES, ASW_COLLISION_GROUP_BOTS, false );
	SHOULD_COLLIDE( ASW_COLLISION_GROUP_ALIEN, ASW_COLLISION_GROUP_BOTS, false );
	SHOULD_COLLIDE( ASW_COLLISION_GROUP_BIG_ALIEN, ASW_COLLISION_GROUP_BOTS, false );
	SHOULD_COLLIDE( ASW_COLLISION_GROUP_SENTRY, ASW_COLLISION_GROUP_BOTS, false );

	SHOULD_COLLIDE( ASW_COLLISION_GROUP_SHOTGUN_PELLET, ASW_COLLISION_GROUP_SHOTGUN_PELLET, false );

	// grenades don't collide with ceilings
	SHOULD_COLLIDE( ASW_COLLISION_GROUP_GRENADES, ASW_COLLISION_GROUP_CEILINGS, false );
	SHOULD_COLLIDE( ASW_COLLISION_GROUP_NPC_GRENADES, ASW_COLLISION_GROUP_CEILINGS, false );
	SHOULD_COLLIDE( ASW_COLLISION_GROUP_PLAYER_MISSILE, ASW_COLLISION_GROUP_CEILINGS, false );
	SHOULD_COLLIDE( ASW_COLLISION_GROUP_ALIEN_MISSILE, ASW_COLLISION_GROUP_CEILINGS, false );

	// combine balls don't hit each other
	SHOULD_COLLIDE( HL2COLLISION_GROUP_COMBINE_BALL, HL2COLLISION_GROUP_COMBINE_BALL, false );

	// the pellets that the flamer shoots.  Doesn't collide with small aliens or marines, DOES collide with doors and shieldbugs
	SHOULD_COLLIDE( ASW_COLLISION_GROUP_EGG, ASW_COLLISION_GROUP_FLAMER_PELLETS, false );
	SHOULD_COLLIDE( ASW_COLLISION_GROUP_PARASITE, ASW_COLLISION_GROUP_FLAMER_PELLETS, false );
	SHOULD_COLLIDE( ASW_COLLISION_GROUP_ALIEN, ASW_COLLISION_GROUP_FLAMER_PELLETS, false );
	SHOULD_COLLIDE( COLLISION_GROUP_PLAYER, ASW_COLLISION_GROUP_FLAMER_PELLETS, false );
	SHOULD_COLLIDE( ASW_COLLISION_GROUP_FLAMER_PELLETS, ASW_COLLISION_GROUP_BOTS, false );
	SHOULD_COLLIDE( COLLISION_GROUP_NPC, ASW_COLLISION_GROUP_FLAMER_PELLETS, false );
	SHOULD_COLLIDE( COLLISION_GROUP_DEBRIS, ASW_COLLISION_GROUP_FLAMER_PELLETS, false );
	SHOULD_COLLIDE( ASW_COLLISION_GROUP_SHOTGUN_PELLET, ASW_COLLISION_GROUP_FLAMER_PELLETS, false );
	SHOULD_COLLIDE( ASW_COLLISION_GROUP_FLAMER_PELLETS, ASW_COLLISION_GROUP_FLAMER_PELLETS, false );
	SHOULD_COLLIDE( ASW_COLLISION_GROUP_SENTRY, ASW_COLLISION_GROUP_FLAMER_PELLETS, false );
	SHOULD_COLLIDE( ASW_COLLISION_GROUP_SENTRY_PROJECTILE, ASW_COLLISION_GROUP_FLAMER_PELLETS, false );
	SHOULD_COLLIDE( ASW_COLLISION_GROUP_IGNORE_NPCS, ASW_COLLISION_GROUP_FLAMER_PELLETS, false );
	SHOULD_COLLIDE( COLLISION_GROUP_WEAPON, ASW_COLLISION_GROUP_FLAMER_PELLETS, false );
	SHOULD_COLLIDE( ASW_COLLISION_GROUP_GRENADES, ASW_COLLISION_GROUP_FLAMER_PELLETS, false );
	ALWAYS_COLLIDE( ASW_COLLISION_GROUP_FLAMER_PELLETS, true );

	// the pellets that the extinguisher shoots. Unlike flamer pellets collides with aliens and marines
	SHOULD_COLLIDE( COLLISION_GROUP_DEBRIS, ASW_COLLISION_GROUP_EXTINGUISHER_PELLETS, false );
	SHOULD_COLLIDE( ASW_COLLISION_GROUP_SHOTGUN_PELLET, ASW_COLLISION_GROUP_EXTINGUISHER_PELLETS, false );
	SHOULD_COLLIDE( ASW_COLLISION_GROUP_FLAMER_PELLETS, ASW_COLLISION_GROUP_EXTINGUISHER_PELLETS, false );
	SHOULD_COLLIDE( ASW_COLLISION_GROUP_SENTRY, ASW_COLLISION_GROUP_EXTINGUISHER_PELLETS, false );
	SHOULD_COLLIDE( ASW_COLLISION_GROUP_SENTRY_PROJECTILE, ASW_COLLISION_GROUP_EXTINGUISHER_PELLETS, false );
	SHOULD_COLLIDE( ASW_COLLISION_GROUP_EXTINGUISHER_PELLETS, ASW_COLLISION_GROUP_EXTINGUISHER_PELLETS, false );
	SHOULD_COLLIDE( COLLISION_GROUP_WEAPON, ASW_COLLISION_GROUP_EXTINGUISHER_PELLETS, false );
	SHOULD_COLLIDE( ASW_COLLISION_GROUP_GRENADES, ASW_COLLISION_GROUP_EXTINGUISHER_PELLETS, false );
	ALWAYS_COLLIDE( ASW_COLLISION_GROUP_EXTINGUISHER_PELLETS, true );

	if ( collisionGroup1 == ASW_COLLISION_GROUP_PASSABLE )
		return false;
	Assert( collisionGroup0 != ASW_COLLISION_GROUP_PASSABLE );

	// fire wall pieces don't get blocked by aliens/marines
	SHOULD_COLLIDE( ASW_COLLISION_GROUP_EGG, ASW_COLLISION_GROUP_IGNORE_NPCS, false );
	SHOULD_COLLIDE( ASW_COLLISION_GROUP_PARASITE, ASW_COLLISION_GROUP_IGNORE_NPCS, false );
	SHOULD_COLLIDE( ASW_COLLISION_GROUP_ALIEN, ASW_COLLISION_GROUP_IGNORE_NPCS, false );
	SHOULD_COLLIDE( COLLISION_GROUP_PLAYER, ASW_COLLISION_GROUP_IGNORE_NPCS, false );
	SHOULD_COLLIDE( ASW_COLLISION_GROUP_IGNORE_NPCS, ASW_COLLISION_GROUP_BOTS, false );
	SHOULD_COLLIDE( COLLISION_GROUP_NPC, ASW_COLLISION_GROUP_IGNORE_NPCS, false );
	SHOULD_COLLIDE( COLLISION_GROUP_DEBRIS, ASW_COLLISION_GROUP_IGNORE_NPCS, false );
	SHOULD_COLLIDE( ASW_COLLISION_GROUP_SHOTGUN_PELLET, ASW_COLLISION_GROUP_IGNORE_NPCS, false );
	SHOULD_COLLIDE( ASW_COLLISION_GROUP_GRENADES, ASW_COLLISION_GROUP_IGNORE_NPCS, false );
	SHOULD_COLLIDE( ASW_COLLISION_GROUP_IGNORE_NPCS, ASW_COLLISION_GROUP_IGNORE_NPCS, false );
	SHOULD_COLLIDE( ASW_COLLISION_GROUP_BIG_ALIEN, ASW_COLLISION_GROUP_IGNORE_NPCS, false );
	ALWAYS_COLLIDE( ASW_COLLISION_GROUP_IGNORE_NPCS, true );

	// Only let projectile blocking debris collide with grenades
	SHOULD_COLLIDE( COLLISION_GROUP_DEBRIS_BLOCK_PROJECTILE, ASW_COLLISION_GROUP_GRENADES, true );
	SHOULD_COLLIDE( COLLISION_GROUP_DEBRIS_BLOCK_PROJECTILE, ASW_COLLISION_GROUP_NPC_GRENADES, true );

	// Grenades hit everything but debris, weapons, other projectiles and marines
	SHOULD_COLLIDE( ASW_COLLISION_GROUP_NPC_GRENADES, ASW_COLLISION_GROUP_NPC_GRENADES, false );
	SHOULD_COLLIDE( COLLISION_GROUP_DEBRIS, ASW_COLLISION_GROUP_GRENADES, false );
	SHOULD_COLLIDE( COLLISION_GROUP_WEAPON, ASW_COLLISION_GROUP_GRENADES, false );
	SHOULD_COLLIDE( COLLISION_GROUP_PROJECTILE, ASW_COLLISION_GROUP_GRENADES, false );
	SHOULD_COLLIDE( ASW_COLLISION_GROUP_GRENADES, ASW_COLLISION_GROUP_GRENADES, false );
	SHOULD_COLLIDE( COLLISION_GROUP_PLAYER, ASW_COLLISION_GROUP_GRENADES, false );
	SHOULD_COLLIDE( ASW_COLLISION_GROUP_GRENADES, ASW_COLLISION_GROUP_BOTS, false );
	SHOULD_COLLIDE( COLLISION_GROUP_DOOR_BLOCKER, ASW_COLLISION_GROUP_GRENADES, false );

	// sentries dont collide with marines or their own projectiles
	SHOULD_COLLIDE( ASW_COLLISION_GROUP_SENTRY, ASW_COLLISION_GROUP_SENTRY_PROJECTILE, false );
	SHOULD_COLLIDE( ASW_COLLISION_GROUP_PLAYER_MISSILE, ASW_COLLISION_GROUP_SENTRY, false );
	SHOULD_COLLIDE( ASW_COLLISION_GROUP_SHOTGUN_PELLET, ASW_COLLISION_GROUP_SENTRY, false );
	SHOULD_COLLIDE( COLLISION_GROUP_PLAYER, ASW_COLLISION_GROUP_SENTRY, false );
	SHOULD_COLLIDE( ASW_COLLISION_GROUP_SENTRY, ASW_COLLISION_GROUP_BOTS, false );

#ifndef CLIENT_DLL	// this isn't necessary on client
	// reactivedrop: sentries don't collide with aliens
	if ( !rd_sentry_block_aliens.GetBool() )
	{
		SHOULD_COLLIDE( ASW_COLLISION_GROUP_ALIEN, ASW_COLLISION_GROUP_SENTRY, false );
	}
#endif

	// sentry projectiles only collide with doors, walls and shieldbugs
	SHOULD_COLLIDE( ASW_COLLISION_GROUP_EGG, ASW_COLLISION_GROUP_SENTRY_PROJECTILE, false );
	SHOULD_COLLIDE( ASW_COLLISION_GROUP_PARASITE, ASW_COLLISION_GROUP_SENTRY_PROJECTILE, false );
	SHOULD_COLLIDE( ASW_COLLISION_GROUP_ALIEN, ASW_COLLISION_GROUP_SENTRY_PROJECTILE, false );
	SHOULD_COLLIDE( COLLISION_GROUP_PLAYER, ASW_COLLISION_GROUP_SENTRY_PROJECTILE, false );
	SHOULD_COLLIDE( COLLISION_GROUP_NPC, ASW_COLLISION_GROUP_SENTRY_PROJECTILE, false );
	SHOULD_COLLIDE( COLLISION_GROUP_DEBRIS, ASW_COLLISION_GROUP_SENTRY_PROJECTILE, false );
	SHOULD_COLLIDE( ASW_COLLISION_GROUP_SHOTGUN_PELLET, ASW_COLLISION_GROUP_SENTRY_PROJECTILE, false );
	SHOULD_COLLIDE( ASW_COLLISION_GROUP_SENTRY, ASW_COLLISION_GROUP_SENTRY_PROJECTILE, false );
	SHOULD_COLLIDE( COLLISION_GROUP_WEAPON, ASW_COLLISION_GROUP_SENTRY_PROJECTILE, false );
	ALWAYS_COLLIDE( ASW_COLLISION_GROUP_SENTRY_PROJECTILE, true );

	SHOULD_COLLIDE( COLLISION_GROUP_NPC, ASW_COLLISION_GROUP_ALIEN_MISSILE, false );
	SHOULD_COLLIDE( ASW_COLLISION_GROUP_SHOTGUN_PELLET, ASW_COLLISION_GROUP_ALIEN_MISSILE, false );
	SHOULD_COLLIDE( COLLISION_GROUP_WEAPON, ASW_COLLISION_GROUP_ALIEN_MISSILE, false );
	SHOULD_COLLIDE( COLLISION_GROUP_PROJECTILE, ASW_COLLISION_GROUP_ALIEN_MISSILE, false );
	SHOULD_COLLIDE( ASW_COLLISION_GROUP_ALIEN_MISSILE, ASW_COLLISION_GROUP_ALIEN_MISSILE, false );
	SHOULD_COLLIDE( COLLISION_GROUP_PLAYER, ASW_COLLISION_GROUP_ALIEN_MISSILE, false ); // NOTE: alien projectiles do not collide with marines using their normal touch functions.  Instead we do lag comped traces

	SHOULD_COLLIDE( ASW_COLLISION_GROUP_PLAYER_MISSILE, ASW_COLLISION_GROUP_PLAYER_MISSILE, false );
	SHOULD_COLLIDE( ASW_COLLISION_GROUP_SHOTGUN_PELLET, ASW_COLLISION_GROUP_PLAYER_MISSILE, false );
	SHOULD_COLLIDE( COLLISION_GROUP_DEBRIS, ASW_COLLISION_GROUP_PLAYER_MISSILE, false );
	SHOULD_COLLIDE( COLLISION_GROUP_WEAPON, ASW_COLLISION_GROUP_PLAYER_MISSILE, false );
	SHOULD_COLLIDE( COLLISION_GROUP_PROJECTILE, ASW_COLLISION_GROUP_PLAYER_MISSILE, false );
	SHOULD_COLLIDE( ASW_COLLISION_GROUP_ALIEN_MISSILE, ASW_COLLISION_GROUP_PLAYER_MISSILE, false );

	// allow missile-player collisions for deathmatch
	if ( !ASWDeathmatchMode() )
	{
		SHOULD_COLLIDE( COLLISION_GROUP_PLAYER, ASW_COLLISION_GROUP_PLAYER_MISSILE, false );
		SHOULD_COLLIDE( ASW_COLLISION_GROUP_PLAYER_MISSILE, ASW_COLLISION_GROUP_BOTS, false );
	}

	// HL2 collision rules
	//If collisionGroup0 is not a player then NPC_ACTOR behaves just like an NPC.
	ASSERT_INVARIANT( COLLISION_GROUP_PLAYER <= COLLISION_GROUP_NPC_ACTOR );
	if ( collisionGroup1 == COLLISION_GROUP_NPC_ACTOR && collisionGroup0 != COLLISION_GROUP_PLAYER )
	{
		collisionGroup1 = COLLISION_GROUP_NPC;
	}

	if ( collisionGroup0 > collisionGroup1 )
	{
		// once again, we may have messed up the order
		V_swap( collisionGroup0, collisionGroup1 );
	}

	// grubs don't collide with each other
	SHOULD_COLLIDE( ASW_COLLISION_GROUP_GRUBS, ASW_COLLISION_GROUP_GRUBS, false );

	// parasites don't collide with each other (fixes double head jumping and crazy leaps when multiple parasites exit a victim)
	SHOULD_COLLIDE( ASW_COLLISION_GROUP_PARASITE, ASW_COLLISION_GROUP_PARASITE, false );

	// weapons and NPCs don't collide
	ASSERT_INVARIANT( COLLISION_GROUP_WEAPON <= HL2COLLISION_GROUP_FIRST_NPC && HL2COLLISION_GROUP_FIRST_NPC <= HL2COLLISION_GROUP_LAST_NPC );
	if ( collisionGroup0 == COLLISION_GROUP_WEAPON && (collisionGroup1 >= HL2COLLISION_GROUP_FIRST_NPC && collisionGroup1 <= HL2COLLISION_GROUP_LAST_NPC ) )
		return false;

	//players don't collide against NPC Actors.
	//I could've done this up where I check if collisionGroup0 is NOT a player but I decided to just
	//do what the other checks are doing in this function for consistency sake.
	SHOULD_COLLIDE( COLLISION_GROUP_PLAYER, COLLISION_GROUP_NPC_ACTOR, false );

	// In cases where NPCs are playing a script which causes them to interpenetrate while riding on another entity,
	// such as a train or elevator, you need to disable collisions between the actors so the mover can move them.
	SHOULD_COLLIDE( COLLISION_GROUP_NPC_SCRIPTED, COLLISION_GROUP_NPC_SCRIPTED, false );

	// reactivedrop: players don't collide with unborrowing aliens
	SHOULD_COLLIDE( COLLISION_GROUP_PLAYER, COLLISION_GROUP_NPC_SCRIPTED, false );

	return BaseClass::ShouldCollide( collisionGroup0, collisionGroup1 ); 
}

static bool CanPickupUnrestrictedWeapon( int iWeaponIndex, const ConVar &unrestrictedGuns )
{
	CUtlVector<int> vecWepList;
	{
		// convert string array to number array
		CUtlStringList szWepList;
		V_SplitString( unrestrictedGuns.GetString(), " ", szWepList );
		for ( int i = 0; i < szWepList.Count(); ++i )
		{
			int iWepId = atoi( szWepList[i] );
			if ( iWepId < 0 )
			{
				Warning( "Incorrect weapon ID=%i found in %s\n", iWepId, unrestrictedGuns.GetName() );
				continue;
			}
			vecWepList.AddToTail( iWepId );
		}
	}

	if ( vecWepList.HasElement( iWeaponIndex ) )
		return true;

	return false;
}

bool CAlienSwarm::MarineCanPickup(CASW_Marine_Resource* pMarineResource, const char* szWeaponClass, const char* szSwappingClass)
{
	if (!ASWEquipmentList() || !pMarineResource)
		return false;
	// need to get the weapon data associated with this class
	CASW_WeaponInfo* pWeaponData = ASWEquipmentList()->GetWeaponDataFor(szWeaponClass);
	if (!pWeaponData)
		return false;

	CASW_Marine_Profile* pProfile = pMarineResource->GetProfile();
	if (!pProfile)
		return false;

	bool bCheckRestriction = true;
	if ( !pWeaponData->m_bExtra && rd_weapons_regular_class_unrestricted.GetInt() != -1 )
	{
		if ( rd_weapons_regular_class_unrestricted.GetInt() == -2 || CanPickupUnrestrictedWeapon( ASWEquipmentList()->GetRegularIndex( szWeaponClass ), rd_weapons_regular_class_unrestricted ) )
			bCheckRestriction = false;
	}
	else if ( pWeaponData->m_bExtra && rd_weapons_extra_class_unrestricted.GetInt() != -1 )
	{
		if ( rd_weapons_extra_class_unrestricted.GetInt() == -2 || CanPickupUnrestrictedWeapon( ASWEquipmentList()->GetExtraIndex( szWeaponClass ), rd_weapons_extra_class_unrestricted ) )
			bCheckRestriction = false;
	}

	if ( bCheckRestriction )
	{
		// check various class skills
		if (pWeaponData->m_bTech && !pProfile->CanHack())
		{
			Q_snprintf( m_szPickupDenial, sizeof(m_szPickupDenial), "#asw_requires_tech");
			return false;
		}

		if (pWeaponData->m_bFirstAid && !pProfile->CanUseFirstAid())
		{
			Q_snprintf( m_szPickupDenial, sizeof(m_szPickupDenial), "#asw_requires_medic");
			return false;
		}

		if (pWeaponData->m_bSpecialWeapons && pProfile->GetMarineClass() != MARINE_CLASS_SPECIAL_WEAPONS)
		{
			Q_snprintf( m_szPickupDenial, sizeof(m_szPickupDenial), "#asw_requires_sw");
			return false;
		}

		if (pWeaponData->m_bSapper && pProfile->GetMarineClass() != MARINE_CLASS_NCO)
		{
			Q_snprintf( m_szPickupDenial, sizeof(m_szPickupDenial), "#asw_requires_nco");
			return false;
		}
	}

// 	if (pWeaponData->m_bSarge && !pProfile->m_bSarge)
// 	{
// 		Q_snprintf( m_szPickupDenial, sizeof(m_szPickupDenial), "#asw_sarge_only");
// 		return false;
// 	}

	if (pWeaponData->m_bTracker && !pProfile->CanScanner())
	{
		Q_snprintf( m_szPickupDenial, sizeof(m_szPickupDenial), "TRACKING ONLY");
		return false;
	}	

	// reactivedrop: this code doesn't work and is not needed 
	// the pickup of unique weapons is handled correctly in 
	// CASW_Marine::GetWeaponPositionForPickup(), so turning this code off
// 	if (pWeaponData->m_bUnique)
// 	{
// 		// if we're swapping a unique item for the same unique item, allow the pickup
// 		if (szSwappingClass && !Q_strcmp(szWeaponClass, szSwappingClass))
// 			return true;
// 		
// 		// check if we have one of these already
// 		// todo: shouldn't use these vars when ingame, but should check the marine's inventory?
// 		for ( int iWpnSlot = 0; iWpnSlot < ASW_MAX_EQUIP_SLOTS; ++ iWpnSlot )
// 		{
// 			CASW_EquipItem* pItem = ASWEquipmentList()->GetItemForSlot( iWpnSlot, pMarineResource->m_iWeaponsInSlots[ iWpnSlot ] );
// 			if ( !pItem )
// 				continue;
// 
// 			const char* szItemClass = STRING(pItem->m_EquipClass);
// 			if ( !Q_strcmp(szItemClass, szWeaponClass) )
// 			{
// 				Q_snprintf( m_szPickupDenial, sizeof(m_szPickupDenial), "#asw_cannot_carry_two");
// 				return false;
// 			}
// 		}
// 	}

	return true;
}

bool CAlienSwarm::MarineCanSelectInLobby(CASW_Marine_Resource* pMarineResource, const char* szWeaponClass, const char* szSwappingClass)
{
	if (MarineCanPickup(pMarineResource, szWeaponClass, szSwappingClass))
	{
		CASW_WeaponInfo* pWeaponData = ASWEquipmentList()->GetWeaponDataFor(szWeaponClass);
		if (!pWeaponData)
			return false;

		if (pWeaponData->m_bUnique)
		{
			// if we're swapping a unique item for the same unique item, allow the pickup
			if (szSwappingClass && !Q_strcmp(szWeaponClass, szSwappingClass))
				return true;
		
			// check if we have one of these already
			// todo: shouldn't use these vars when ingame, but should check the marine's inventory?
			for ( int iWpnSlot = 0; iWpnSlot < ASW_MAX_EQUIP_SLOTS; ++ iWpnSlot )
			{
				CASW_EquipItem* pItem = ASWEquipmentList()->GetItemForSlot( iWpnSlot, pMarineResource->m_iWeaponsInSlots[ iWpnSlot ] );
				if ( !pItem )
					continue;

				const char* szItemClass = STRING(pItem->m_EquipClass);
				if ( !Q_strcmp(szItemClass, szWeaponClass) )
				{
					Q_snprintf( m_szPickupDenial, sizeof(m_szPickupDenial), "#asw_cannot_carry_two");
					return false;
				}
			}
		}
		return true;
	}
	return false;
}


void CAlienSwarm::CreateStandardEntities( void )
{

#ifndef CLIENT_DLL
	// Create the entity that will send our data to the client.

	BaseClass::CreateStandardEntities();

#ifdef _DEBUG
	CBaseEntity *pEnt = 
#endif
	CBaseEntity::Create( "asw_gamerules", vec3_origin, vec3_angle );
	Assert( pEnt );

	// riflemod: create health and extra item regeneration entities 
	CBaseEntity::Create("asw_health_regen", vec3_origin, vec3_angle);
	//CBaseEntity::Create("asw_item_regen", vec3_origin, vec3_angle);
#endif
}

// return true if our marine is using a weapon that can autoaim at flares
//  and if the target is inside a flare radius
bool CAlienSwarm::CanFlareAutoaimAt(CASW_Inhabitable_NPC* pAimer, CBaseEntity *pEntity)
{
	CASW_Marine *pMarine = CASW_Marine::AsMarine( pAimer );
	if (!pMarine || !pEntity || !g_pHeadFlare || !pEntity->IsNPC() )
		return false;

	CASW_Weapon* pWeapon = pMarine->GetActiveASWWeapon();
	if (!pWeapon)
		return false;

	if (!pWeapon->ShouldFlareAutoaim())
		return false;

	// go through all our flares and check if this entity is inside any of them
	CASW_Flare_Projectile* pFlare = g_pHeadFlare;
	float dist = 0;
	Vector diff;
	while (pFlare!=NULL)
	{
		diff = pEntity->GetAbsOrigin() - pFlare->GetAbsOrigin();
		dist = diff.Length();
		if (dist <= asw_flare_autoaim_radius.GetFloat())
			return true;

		pFlare = pFlare->m_pNextFlare;
	}

	return false;
}

// returns 0 if it's a single mission, 1 if it's a campaign game
int CAlienSwarm::IsCampaignGame()
{
	CASW_Game_Resource* pGameResource = ASWGameResource();
	if (!pGameResource)
	{
		//Msg("Warning, IsCampaignGame called without asw game resource!\n");
		return 0;
	}

	return pGameResource->IsCampaignGame();
}

CASW_Campaign_Save* CAlienSwarm::GetCampaignSave()
{
	CASW_Game_Resource* pGameResource = ASWGameResource();
	if (!pGameResource)
		return NULL;
	return pGameResource->GetCampaignSave();
}

const RD_Campaign_t *CAlienSwarm::GetCampaignInfo()
{
	CASW_Game_Resource* pGameResource = ASWGameResource();
	if (!pGameResource)
		return NULL;

	if (IsCampaignGame() != 1)
		return NULL;

	// if the campaign info has previously been setup, then just return that
	if ( pGameResource->m_pCampaignInfo )
		return pGameResource->m_pCampaignInfo;

	// will only set up the campaign info if the campaign save is here (should've been created in gamerules constructor (and networked down each client))
	CASW_Campaign_Save *pSave = GetCampaignSave();
	if ( !pSave )
		return NULL;

	// our savegame is setup, so we can ask it for the name of our campaign and try to load it
	pGameResource->m_pCampaignInfo.SetCampaign( pSave->GetCampaignName() );

#ifndef CLIENT_DLL
	// created and loaded okay, notify the asw game resource that some new marine skills are to be networked about
	if ( pGameResource->m_pCampaignInfo )
		pGameResource->UpdateMarineSkills( pSave );
#endif

	if ( !pGameResource->m_pCampaignInfo )
	{
		// failed to load the specified campaign
#ifdef CLIENT_DLL
		engine->ClientCmd( "disconnect\n" );
#else
		engine->ServerCommand( "disconnect\n" );
#endif
	}

	return pGameResource->m_pCampaignInfo;
}


extern bool IsExplosionTraceBlocked( trace_t *ptr );

#ifndef CLIENT_DLL
//-----------------------------------------------------------------------------
// Custom version of radius damage that doesn't hurt marines so much and has special properties for burn damage
//-----------------------------------------------------------------------------
#define ROBUST_RADIUS_PROBE_DIST 16.0f // If a solid surface blocks the explosion, this is how far to creep along the surface looking for another way to the target
void CAlienSwarm::RadiusDamage( const CTakeDamageInfo &info, const Vector &vecSrcIn, float flRadius, int iClassIgnore, CBaseEntity *pEntityIgnore )
{
	const int MASK_RADIUS_DAMAGE = MASK_SHOT & ( ~CONTENTS_HITBOX );
	CBaseEntity *pEntity = NULL;
	trace_t		tr;
	float		flAdjustedDamage, falloff;
	Vector		vecSpot;

	Vector vecSrc = vecSrcIn;

	if ( asw_debug_alien_damage.GetBool() )
	{
		NDebugOverlay::Circle( vecSrc, QAngle( -90.0f, 0, 0 ), flRadius, 255, 160, 0, 127, true, 4.0f );
		NDebugOverlay::Circle( vecSrc, QAngle( -90.0f, 0, 0 ), rd_radial_damage_no_falloff_distance.GetFloat(), 255, 160, 0, 255, true, 4.0f );
	}

	if ( flRadius > rd_radial_damage_no_falloff_distance.GetFloat() )
		falloff = info.GetDamage() / ( flRadius - rd_radial_damage_no_falloff_distance.GetFloat() );
	else
		falloff = 1.0;

	float fMarineRadius = flRadius * asw_marine_explosion_protection.GetFloat();
	float fMarineFalloff = falloff / MAX( 0.01f, asw_marine_explosion_protection.GetFloat() );

	if ( info.GetDamageCustom() & DAMAGE_FLAG_NO_FALLOFF )
	{
		falloff = 0.0f;
		fMarineFalloff = 0.0f;
	}
	else if ( info.GetDamageCustom() & DAMAGE_FLAG_HALF_FALLOFF )
	{
		falloff *= 0.5f;
		fMarineFalloff *= 0.5f;
	}

	vecSrc.z += 1;// in case grenade is lying on the ground

	float flHalfRadiusSqr = Square( flRadius / 2.0f );
	//float flMarineHalfRadiusSqr = flHalfRadiusSqr * asw_marine_explosion_protection.GetFloat();

	// iterate on all entities in the vicinity.
	for ( CEntitySphereQuery sphere( vecSrc, flRadius ); ( pEntity = sphere.GetCurrentEntity() ) != NULL; sphere.NextEntity() )
	{
		// This value is used to scale damage when the explosion is blocked by some other object.
		float flBlockedDamagePercent = 0.0f;

		if ( pEntity == pEntityIgnore )
			continue;

		if ( pEntity->m_takedamage == DAMAGE_NO )
			continue;

		// UNDONE: this should check a damage mask, not an ignore
		if ( iClassIgnore != CLASS_NONE && pEntity->Classify() == iClassIgnore )
		{// houndeyes don't hurt other houndeyes with their attack
			continue;
		}

		// check if this is a marine and if so, he may be outside the explosion radius				
		if ( pEntity->Classify() == CLASS_ASW_MARINE )
		{
			if ( ( vecSrc - pEntity->WorldSpaceCenter() ).Length() > fMarineRadius )
				continue;
		}

		// Check that the explosion can 'see' this entity.
		vecSpot = pEntity->BodyTarget( vecSrc, false );
		UTIL_TraceLine( vecSrc, vecSpot, MASK_RADIUS_DAMAGE, info.GetInflictor(), COLLISION_GROUP_NONE, &tr );

		if ( old_radius_damage.GetBool() )
		{
			if ( tr.fraction != 1.0 && tr.m_pEnt != pEntity )
				continue;
		}
		else
		{
			if ( tr.fraction != 1.0 )
			{
				if ( IsExplosionTraceBlocked( &tr ) )
				{
					if ( ShouldUseRobustRadiusDamage( pEntity ) )
					{
						if ( vecSpot.DistToSqr( vecSrc ) > flHalfRadiusSqr )
						{
							// Only use robust model on a target within one-half of the explosion's radius.
							continue;
						}

						Vector vecToTarget = vecSpot - tr.endpos;
						VectorNormalize( vecToTarget );

						// We're going to deflect the blast along the surface that 
						// interrupted a trace from explosion to this target.
						Vector vecUp, vecDeflect;
						CrossProduct( vecToTarget, tr.plane.normal, vecUp );
						CrossProduct( tr.plane.normal, vecUp, vecDeflect );
						VectorNormalize( vecDeflect );

						// Trace along the surface that intercepted the blast...
						UTIL_TraceLine( tr.endpos, tr.endpos + vecDeflect * ROBUST_RADIUS_PROBE_DIST, MASK_RADIUS_DAMAGE, info.GetInflictor(), COLLISION_GROUP_NONE, &tr );
						//NDebugOverlay::Line( tr.startpos, tr.endpos, 255, 255, 0, false, 10 );

						// ...to see if there's a nearby edge that the explosion would 'spill over' if the blast were fully simulated.
						UTIL_TraceLine( tr.endpos, vecSpot, MASK_RADIUS_DAMAGE, info.GetInflictor(), COLLISION_GROUP_NONE, &tr );
						//NDebugOverlay::Line( tr.startpos, tr.endpos, 255, 0, 0, false, 10 );

						if ( tr.fraction != 1.0 && tr.DidHitWorld() )
						{
							// Still can't reach the target.
							continue;
						}
						// else fall through
					}
					else
					{
						continue;
					}
				}

				// UNDONE: Probably shouldn't let children block parents either?  Or maybe those guys should set their owner if they want this behavior?
				if ( tr.m_pEnt && tr.m_pEnt != pEntity && tr.m_pEnt->GetOwnerEntity() != pEntity )
				{
					// Some entity was hit by the trace, meaning the explosion does not have clear
					// line of sight to the entity that it's trying to hurt. If the world is also
					// blocking, we do no damage.
					CBaseEntity *pBlockingEntity = tr.m_pEnt;
					//Msg( "%s may be blocked by %s...", pEntity->GetClassname(), pBlockingEntity->GetClassname() );

					UTIL_TraceLine( vecSrc, vecSpot, CONTENTS_SOLID, info.GetInflictor(), COLLISION_GROUP_NONE, &tr );

					if ( tr.fraction != 1.0 )
					{
						continue;
					}

					// asw - don't let npcs reduce the damage from explosions
					if ( !pBlockingEntity->IsNPC() )
					{
						// Now, if the interposing object is physics, block some explosion force based on its mass.
						if ( pBlockingEntity->VPhysicsGetObject() )
						{
							const float MASS_ABSORB_ALL_DAMAGE = 350.0f;
							float flMass = pBlockingEntity->VPhysicsGetObject()->GetMass();
							float scale = flMass / MASS_ABSORB_ALL_DAMAGE;

							// Absorbed all the damage.
							if ( scale >= 1.0f )
							{
								continue;
							}

							ASSERT( scale > 0.0f );
							flBlockedDamagePercent = scale;
							//Msg("Object (%s) weighing %fkg blocked %f percent of explosion damage\n", pBlockingEntity->GetClassname(), flMass, scale * 100.0f);
						}
						else
						{
							// Some object that's not the world and not physics. Generically block 25% damage
							flBlockedDamagePercent = 0.25f;
						}
					}
				}
			}
		}

		float flFalloffDist = ( vecSrc - tr.endpos ).Length();
		if ( flFalloffDist > rd_radial_damage_no_falloff_distance.GetFloat() )
		{
			flFalloffDist -= rd_radial_damage_no_falloff_distance.GetFloat();

			// decrease damage for marines
			if ( pEntity->Classify() == CLASS_ASW_MARINE )
				flAdjustedDamage = flFalloffDist * fMarineFalloff;
			else
				flAdjustedDamage = flFalloffDist * falloff;

			flAdjustedDamage = info.GetDamage() - flAdjustedDamage;
		}
		else
		{
			flAdjustedDamage = info.GetDamage();
		}

		if ( flAdjustedDamage <= 0 )
			continue;

		// the explosion can 'see' this entity, so hurt them!
		if ( tr.startsolid )
		{
			// if we're stuck inside them, fixup the position and distance
			tr.endpos = vecSrc;
			tr.fraction = 0.0;
		}

		// make explosions hurt asw_doors more
		if ( FClassnameIs( pEntity, "asw_door" ) )
			flAdjustedDamage *= asw_door_explosion_boost.GetFloat();

		CTakeDamageInfo adjustedInfo = info;
		//Msg("%s: Blocked damage: %f percent (in:%f  out:%f)\n", pEntity->GetClassname(), flBlockedDamagePercent * 100, flAdjustedDamage, flAdjustedDamage - (flAdjustedDamage * flBlockedDamagePercent) );
		adjustedInfo.SetDamage( flAdjustedDamage - ( flAdjustedDamage * flBlockedDamagePercent ) );

		// Now make a consideration for skill level!
		if ( info.GetAttacker() && info.GetAttacker()->IsPlayer() && pEntity->IsNPC() )
		{
			// An explosion set off by the player is harming an NPC. Adjust damage accordingly.
			adjustedInfo.AdjustPlayerDamageInflictedForSkillLevel();
		}

		// asw - if this is burn damage, don't kill the target, let him burn for a bit
		if ( ( adjustedInfo.GetDamageType() & DMG_BURN ) && adjustedInfo.GetDamage() > 3 )
		{
			if ( adjustedInfo.GetDamage() > pEntity->GetHealth() )
			{
				int newDamage = pEntity->GetHealth() - random->RandomInt( 8, 23 );
				if ( newDamage <= 3 )
					newDamage = 3;
				adjustedInfo.SetDamage( newDamage );
			}

			// check if this damage is coming from an incendiary grenade that might need to collect stats
			if ( adjustedInfo.GetInflictor() && adjustedInfo.GetInflictor()->Classify() == CLASS_ASW_GRENADE_VINDICATOR )
			{
				CASW_Grenade_Vindicator *pGrenade = assert_cast< CASW_Grenade_Vindicator * >( adjustedInfo.GetInflictor() );
				pGrenade->BurntAlien( pEntity );
			}
		}

		Vector dir = vecSpot - vecSrc;
		VectorNormalize( dir );

		// If we don't have a damage force, manufacture one
		if ( adjustedInfo.GetDamagePosition() == vec3_origin || adjustedInfo.GetDamageForce() == vec3_origin )
		{
			CalculateExplosiveDamageForce( &adjustedInfo, dir, vecSrc );
		}
		else
		{
			// Assume the force passed in is the maximum force. Decay it based on falloff.
			float flForce = adjustedInfo.GetDamageForce().Length() * falloff;
			adjustedInfo.SetDamageForce( dir * flForce );
			adjustedInfo.SetDamagePosition( vecSrc );
		}

		if ( tr.fraction != 1.0 && pEntity == tr.m_pEnt )
		{
			ClearMultiDamage();
			pEntity->DispatchTraceAttack( adjustedInfo, dir, &tr );
			ApplyMultiDamage();
		}
		else
		{
			pEntity->TakeDamage( adjustedInfo );
		}

		if ( asw_debug_alien_damage.GetBool() )
		{
			Msg( "Explosion did %f damage to %d:%s\n", adjustedInfo.GetDamage(), pEntity->entindex(), pEntity->GetClassname() );
			NDebugOverlay::Line( vecSrc, pEntity->WorldSpaceCenter(), 255, 255, 0, false, 4 );
			NDebugOverlay::EntityText( pEntity->entindex(), 0, CFmtStr( "%d", ( int )adjustedInfo.GetDamage() ), 4.0, 255, 255, 255, 255 );
		}

		// Now hit all triggers along the way that respond to damage... 
		pEntity->TraceAttackToTriggers( adjustedInfo, vecSrc, tr.endpos, dir );
	}
}

bool CAlienSwarm::ShouldUseRobustRadiusDamage( CBaseEntity *pEntity )
{
	if ( !pEntity )
		return false;

	return pEntity->Classify() == CLASS_ASW_ALIEN_GOO;
}

ConVar asw_stumble_knockback( "asw_stumble_knockback", "300", FCVAR_CHEAT, "Velocity given to aliens that get knocked back" );
ConVar asw_stumble_lift( "asw_stumble_lift", "300", FCVAR_CHEAT, "Upwards velocity given to aliens that get knocked back" );

extern ConVar asw_shieldbug_knockdown_force;
extern ConVar asw_shieldbug_knockdown_lift;

void CAlienSwarm::StumbleAliensInRadius( CBaseEntity *pInflictor, const Vector &vecSrcIn, float flRadius )
{
	const int MASK_RADIUS_DAMAGE = MASK_SHOT&(~CONTENTS_HITBOX);
	CBaseEntity *pEntity = NULL;
	trace_t		tr;
	Vector		vecSpot;

	Vector vecSrc = vecSrcIn;

	vecSrc.z += 1;// in case grenade is lying on the ground

	float flHalfRadiusSqr = Square( flRadius / 2.0f );
	//float flMarineHalfRadiusSqr = flHalfRadiusSqr * asw_marine_explosion_protection.GetFloat();

	// iterate on all entities in the vicinity.
	for ( CEntitySphereQuery sphere( vecSrc, flRadius ); (pEntity = sphere.GetCurrentEntity()) != NULL; sphere.NextEntity() )
	{
		if ( pEntity->m_takedamage == DAMAGE_NO )
			continue;

		if ( !pEntity->IsNPC() )
			continue;

		// knockdown marines 
		if (pEntity->Classify() == CLASS_ASW_MARINE )
		{
			if ( rd_jumpjet_knockdown_marines.GetBool() )
			{
				// don't knockdown itself
				if (pEntity == pInflictor)
					continue;
				Vector vecForceDir = pEntity->WorldSpaceCenter() - pInflictor->WorldSpaceCenter();
				vecForceDir.z = 0;
				vecForceDir.NormalizeInPlace();
				vecForceDir *= asw_shieldbug_knockdown_force.GetFloat();
				vecForceDir += Vector( 0, 0, asw_shieldbug_knockdown_lift.GetFloat() );
				((CASW_Marine*)pEntity)->Knockdown( pInflictor, vecForceDir );
			}

			continue;
		}

		if ( !pEntity->IsAlienClassType() )
			continue;

		// Check that the explosion can 'see' this entity.
		vecSpot = pEntity->BodyTarget( vecSrc, false );
		UTIL_TraceLine( vecSrc, vecSpot, MASK_RADIUS_DAMAGE, pInflictor, COLLISION_GROUP_NONE, &tr );


		if ( tr.fraction != 1.0 )
		{
			if ( IsExplosionTraceBlocked(&tr) )
			{
				if( ShouldUseRobustRadiusDamage( pEntity ) )
				{
					if( vecSpot.DistToSqr( vecSrc ) > flHalfRadiusSqr )
					{
						// Only use robust model on a target within one-half of the explosion's radius.
						continue;
					}

					Vector vecToTarget = vecSpot - tr.endpos;
					VectorNormalize( vecToTarget );

					// We're going to deflect the blast along the surface that 
					// interrupted a trace from explosion to this target.
					Vector vecUp, vecDeflect;
					CrossProduct( vecToTarget, tr.plane.normal, vecUp );
					CrossProduct( tr.plane.normal, vecUp, vecDeflect );
					VectorNormalize( vecDeflect );

					// Trace along the surface that intercepted the blast...
					UTIL_TraceLine( tr.endpos, tr.endpos + vecDeflect * ROBUST_RADIUS_PROBE_DIST, MASK_RADIUS_DAMAGE, pInflictor, COLLISION_GROUP_NONE, &tr );
					//NDebugOverlay::Line( tr.startpos, tr.endpos, 255, 255, 0, false, 10 );

					// ...to see if there's a nearby edge that the explosion would 'spill over' if the blast were fully simulated.
					UTIL_TraceLine( tr.endpos, vecSpot, MASK_RADIUS_DAMAGE, pInflictor, COLLISION_GROUP_NONE, &tr );
					//NDebugOverlay::Line( tr.startpos, tr.endpos, 255, 0, 0, false, 10 );

					if( tr.fraction != 1.0 && tr.DidHitWorld() )
					{
						// Still can't reach the target.
						continue;
					}
					// else fall through
				}
				else
				{
					continue;
				}
			}

			// UNDONE: Probably shouldn't let children block parents either?  Or maybe those guys should set their owner if they want this behavior?
			if( tr.m_pEnt && tr.m_pEnt != pEntity && tr.m_pEnt->GetOwnerEntity() != pEntity )
			{
				// Some entity was hit by the trace, meaning the explosion does not have clear
				// line of sight to the entity that it's trying to hurt. If the world is also
				// blocking, we do no damage.
				//CBaseEntity *pBlockingEntity = tr.m_pEnt;
				//Msg( "%s may be blocked by %s...", pEntity->GetClassname(), pBlockingEntity->GetClassname() );

				UTIL_TraceLine( vecSrc, vecSpot, CONTENTS_SOLID, pInflictor, COLLISION_GROUP_NONE, &tr );

				if( tr.fraction != 1.0 )
				{
					continue;
				}
			}
		}

		CASW_Alien* pAlien = assert_cast<CASW_Alien*>(pEntity);
		Vector vecToTarget = pAlien->WorldSpaceCenter() - pInflictor->WorldSpaceCenter();
		vecToTarget.z = 0;
		VectorNormalize( vecToTarget );
		pAlien->Knockback( vecToTarget * asw_stumble_knockback.GetFloat() + Vector( 0, 0, 1 ) * asw_stumble_lift.GetFloat() );
		pAlien->ForceFlinch( vecSrc );
	}
}

void CAlienSwarm::ShockNearbyAliens( CASW_Marine *pMarine, CASW_Weapon *pWeaponSource )
{
	if ( !pMarine )
		return;

	const float flRadius = 160.0f;
	const float flRadiusSqr = flRadius * flRadius;

	// debug stun radius
	//NDebugOverlay::Circle( GetAbsOrigin() + Vector( 0, 0, 1.0f ), QAngle( -90.0f, 0, 0 ), flRadius, 255, 0, 0, 0, true, 5.0f );

	CAI_BaseNPC **ppAIs = g_AI_Manager.AccessAIs();
	int nAIs = g_AI_Manager.NumAIs();

	for ( int i = 0; i < nAIs; i++ )
	{
		CAI_BaseNPC *pNPC = ppAIs[ i ];

		if( !pNPC->IsAlive() )
			continue;

		// ignore hidden objects
		if ( pNPC->IsEffectActive( EF_NODRAW ) )
			continue;

		// Disregard things that want to be disregarded
		if( pNPC->Classify() == CLASS_NONE )
			continue; 

		// Disregard bullseyes
		if( pNPC->Classify() == CLASS_BULLSEYE )
			continue;

		// ignore marines
		if( pNPC->Classify() == CLASS_ASW_MARINE || pNPC->Classify() == CLASS_ASW_COLONIST )
			continue;

		float flDist = (pMarine->GetAbsOrigin() - pNPC->GetAbsOrigin()).LengthSqr();
		if( flDist > flRadiusSqr )
			continue;

		CRecipientFilter filter;
		filter.AddAllPlayers();
		UserMessageBegin( filter, "ASWEnemyZappedByThorns" );
		WRITE_SHORT( pMarine->entindex() );
		WRITE_SHORT( pNPC->entindex() );
		MessageEnd();

		ClearMultiDamage();	
		CTakeDamageInfo shockDmgInfo( pWeaponSource, pMarine, 5.0f, DMG_SHOCK );					
		Vector vecDir = pNPC->WorldSpaceCenter() - pMarine->WorldSpaceCenter();
		VectorNormalize( vecDir );
		shockDmgInfo.SetDamagePosition( pNPC->WorldSpaceCenter() * vecDir * -20.0f );
		shockDmgInfo.SetDamageForce( vecDir );
		shockDmgInfo.ScaleDamageForce( 1.0 );
		shockDmgInfo.SetWeapon( pWeaponSource );

		trace_t tr;
		UTIL_TraceLine( pMarine->WorldSpaceCenter(), pNPC->WorldSpaceCenter(), MASK_SHOT, pMarine, COLLISION_GROUP_NONE, &tr );
		pNPC->DispatchTraceAttack( shockDmgInfo, vecDir, &tr );
		ApplyMultiDamage();
	}
}

void CAlienSwarm::FreezeAliensInRadius( CBaseEntity *pInflictor, float flFreezeAmount, const Vector &vecSrcIn, float flRadius )
{
	const int MASK_RADIUS_DAMAGE = MASK_SHOT&(~CONTENTS_HITBOX);
	CBaseEntity *pEntity = NULL;
	trace_t		tr;
	Vector		vecSpot;
	int nFrozen = 0;
	int nFrozenStat = 0;
	Vector vecSrc = vecSrcIn;

	vecSrc.z += 1;// in case grenade is lying on the ground

	float flHalfRadiusSqr = Square( flRadius / 2.0f );
	//float flMarineHalfRadiusSqr = flHalfRadiusSqr * asw_marine_explosion_protection.GetFloat();

	CASW_Marine *pInflictorMarine = CASW_Marine::AsMarine( pInflictor );

	// iterate on all entities in the vicinity.
	for ( CEntitySphereQuery sphere( vecSrc, flRadius ); (pEntity = sphere.GetCurrentEntity()) != NULL; sphere.NextEntity() )
	{
		if ( pEntity->m_takedamage == DAMAGE_NO )
			continue;

		if ( !pEntity->IsNPC() )
			continue;

		// don't stumble marines
		if ( pEntity->Classify() == CLASS_ASW_MARINE || pEntity->Classify() == CLASS_ASW_COLONIST )
		{
#ifdef GAME_DLL		
			// but, do extinguish them if they are on fire
			CBaseAnimating *pAnim = assert_cast<CBaseAnimating *>(pEntity);
			if ( pAnim->IsOnFire() )
			{
				CEntityFlame *pFireChild = dynamic_cast<CEntityFlame *>( pAnim->GetEffectEntity() );
				if ( pFireChild )
				{
					pAnim->SetEffectEntity( NULL );
					UTIL_Remove( pFireChild );	
				}			
				pAnim->Extinguish();

				CASW_Marine *pInflictorMarine = CASW_Marine::AsMarine( pInflictor );
				CASW_Marine_Resource *pMR = pInflictorMarine ? pInflictorMarine->GetMarineResource() : NULL;
				if ( pMR )
				{
					ADD_STAT( m_iGrenadeExtinguishMarine, 1 );
				}
			}
#endif
			if ( !ASWDeathmatchMode() && ( !pInflictor || ( pInflictorMarine && pInflictorMarine->IRelationType( pEntity ) == D_LI ) ) )
				continue;
		}

		if ( !pEntity->IsInhabitableNPC() )
			continue;

		// Check that the explosion can 'see' this entity.
		vecSpot = pEntity->BodyTarget( vecSrc, false );
		UTIL_TraceLine( vecSrc, vecSpot, MASK_RADIUS_DAMAGE, pInflictor, COLLISION_GROUP_NONE, &tr );

		if ( tr.fraction != 1.0 )
		{
			if ( IsExplosionTraceBlocked(&tr) )
			{
				if( ShouldUseRobustRadiusDamage( pEntity ) )
				{
					if( vecSpot.DistToSqr( vecSrc ) > flHalfRadiusSqr )
					{
						// Only use robust model on a target within one-half of the explosion's radius.
						continue;
					}

					Vector vecToTarget = vecSpot - tr.endpos;
					VectorNormalize( vecToTarget );

					// We're going to deflect the blast along the surface that 
					// interrupted a trace from explosion to this target.
					Vector vecUp, vecDeflect;
					CrossProduct( vecToTarget, tr.plane.normal, vecUp );
					CrossProduct( tr.plane.normal, vecUp, vecDeflect );
					VectorNormalize( vecDeflect );

					// Trace along the surface that intercepted the blast...
					UTIL_TraceLine( tr.endpos, tr.endpos + vecDeflect * ROBUST_RADIUS_PROBE_DIST, MASK_RADIUS_DAMAGE, pInflictor, COLLISION_GROUP_NONE, &tr );
					//NDebugOverlay::Line( tr.startpos, tr.endpos, 255, 255, 0, false, 10 );

					// ...to see if there's a nearby edge that the explosion would 'spill over' if the blast were fully simulated.
					UTIL_TraceLine( tr.endpos, vecSpot, MASK_RADIUS_DAMAGE, pInflictor, COLLISION_GROUP_NONE, &tr );
					//NDebugOverlay::Line( tr.startpos, tr.endpos, 255, 0, 0, false, 10 );

					if( tr.fraction != 1.0 && tr.DidHitWorld() )
					{
						// Still can't reach the target.
						continue;
					}
					// else fall through
				}
				else
				{
					continue;
				}
			}

			// UNDONE: Probably shouldn't let children block parents either?  Or maybe those guys should set their owner if they want this behavior?
			if( tr.m_pEnt && tr.m_pEnt != pEntity && tr.m_pEnt->GetOwnerEntity() != pEntity )
			{
				// Some entity was hit by the trace, meaning the explosion does not have clear
				// line of sight to the entity that it's trying to hurt. If the world is also
				// blocking, we do no damage.
				//CBaseEntity *pBlockingEntity = tr.m_pEnt;
				//Msg( "%s may be blocked by %s...", pEntity->GetClassname(), pBlockingEntity->GetClassname() );

				UTIL_TraceLine( vecSrc, vecSpot, CONTENTS_SOLID, pInflictor, COLLISION_GROUP_NONE, &tr );

				if( tr.fraction != 1.0 )
				{
					continue;
				}
			}
		}
#ifdef GAME_DLL
		CASW_Inhabitable_NPC *pAlien = assert_cast< CASW_Inhabitable_NPC * >( pEntity );
		CBaseAnimating *pAnim = pAlien;
		if ( pAnim->IsOnFire() )
		{
			CEntityFlame *pFireChild = dynamic_cast<CEntityFlame *>( pAnim->GetEffectEntity() );
			if ( pFireChild )
			{
				pAnim->SetEffectEntity( NULL );
				UTIL_Remove( pFireChild );	
			}			
			pAnim->Extinguish();
		}

		if ( pAlien->IsAlive() && !pAlien->IsFrozen() )
		{
			nFrozenStat++;
		}
		pAlien->Freeze( flFreezeAmount, pInflictor, NULL );
		nFrozen++;
#endif
	}
#ifdef GAME_DLL
	CASW_Marine_Resource *pMR = pInflictorMarine ? pInflictorMarine->GetMarineResource() : NULL;
	if ( pMR && nFrozenStat )
	{
		ADD_STAT( m_iGrenadeFreezeAlien, nFrozenStat );
	}
	if ( nFrozen >= 6 && pInflictorMarine && pInflictorMarine->IsInhabited() && pInflictorMarine->GetCommander() )
	{
		pInflictorMarine->GetCommander()->AwardAchievement( ACHIEVEMENT_ASW_FREEZE_GRENADE );
		if ( pMR )
		{
			pMR->m_bFreezeGrenadeMedal = true;
		}
	}
#endif
}

void CAlienSwarm::ClientCommandKeyValues( edict_t *pEntity, KeyValues *pKeyValues )
{
#ifdef GAME_DLL

	CASW_Player *pPlayer = ( CASW_Player * )CBaseEntity::Instance( pEntity );
	if ( !pPlayer )
		return;

	char const *szCommand = pKeyValues->GetName();

	if ( FStrEq( szCommand, "XPUpdate" ) )
	{
		pPlayer->SetNetworkedExperience( pKeyValues->GetInt( "xp" ) );
		pPlayer->SetNetworkedPromotion( pKeyValues->GetInt( "pro" ) );
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: called each time a player uses a "cmd" command
// Input  : *pEdict - the player who issued the command
//			Use engine.Cmd_Argv,  engine.Cmd_Argv, and engine.Cmd_Argc to get 
//			pointers the character string command.
//-----------------------------------------------------------------------------
bool CAlienSwarm::ClientCommand( CBaseEntity *pEdict, const CCommand &args )
{
	if( BaseClass::ClientCommand( pEdict, args ) )
		return true;

	CASW_Player *pPlayer = (CASW_Player *) pEdict;

	if ( pPlayer->ClientCommand( args ) )
		return true;

	const char *pcmd = args[0];
	if ( FStrEq( pcmd, "achievement_earned" ) )
	{
		if ( pPlayer && pPlayer->ShouldAnnounceAchievement() )
		{
			// let's check this came from the client .dll and not the console
			unsigned short mask = UTIL_GetAchievementEventMask();
			int iPlayerID = pPlayer->GetUserID();

			int iAchievement = atoi( args[1] ) ^ mask;
			int code = ( iPlayerID ^ iAchievement ) ^ mask;

			if ( code == atoi( args[2] ) )
			{
				IGameEvent * event = gameeventmanager->CreateEvent( "achievement_earned" );
				if ( event )
				{
					event->SetInt( "player", pEdict->entindex() );
					event->SetInt( "achievement", iAchievement );
					gameeventmanager->FireEvent( event );
				}

				pPlayer->OnAchievementEarned( iAchievement );

				CASW_Marine *pMarine = CASW_Marine::AsMarine( pPlayer->GetNPC() );
				CASW_Marine_Resource *pMR = pMarine ? pMarine->GetMarineResource() : NULL;
				if ( !pMR )
				{
					pMR = ASWGameResource()->GetFirstMarineResourceForPlayer( pPlayer );
				}
				
				if ( pMR )
				{
					pMR->m_aAchievementsEarned.AddToTail( iAchievement );

					if ( pMR->m_bAwardedMedals )
					{
						// already earned medals, i.e. this achievement was earned during the debrief.
						//  need to update the medal string with the new achievement

						bool bHasMedal = false;
						for ( int i = 0; i < pMR->m_CharMedals.Count(); i++ )
						{
							if ( MedalMatchesAchievement( pMR->m_CharMedals[i], iAchievement ) )
							{
								bHasMedal = true;
								break;
							}
						}
						
						if ( !bHasMedal )
						{
							char achievement_buffer[ 255 ];
							achievement_buffer[0] = 0;

							if ( pMR->m_MedalsAwarded.Get()[0] )
							{
								Q_snprintf( achievement_buffer, sizeof( achievement_buffer ), "%s %d", pMR->m_MedalsAwarded.Get(), -iAchievement );
							}
							else
							{
								Q_snprintf( achievement_buffer, sizeof( achievement_buffer ), "%d", -iAchievement );
							}
							Q_snprintf( pMR->m_MedalsAwarded.GetForModify(), 255, "%s", achievement_buffer );
						}
					}
				}
			}
		}

		return true;
	}

	return false;
}

#endif // #ifndef CLIENT_DLL

bool CAlienSwarm::CanSpendPoint(CASW_Player *pPlayer, int iProfileIndex, int nSkillSlot)
{
	if (!ASWGameResource() || !pPlayer)
	{
		//Msg("returning false cos this isn't campaign\n");
		return false;
	}

	CASW_Game_Resource *pGameResource = ASWGameResource();
	// only allow spending if we have the marine selected in multiplayer
	CASW_Marine_Profile *pProfile = NULL;

	if ( pGameResource->IsRosterSelected(iProfileIndex) || IsOfflineGame() )
	{
		bool bSelectedByMe = false;
		for (int i=0; i<pGameResource->GetMaxMarineResources();i++)
		{
			CASW_Marine_Resource *pMarineResource = pGameResource->GetMarineResource(i);
			if (pMarineResource && pMarineResource->GetProfile()->m_ProfileIndex == iProfileIndex)
			{
				pProfile = pMarineResource->GetProfile();
				bSelectedByMe = (pMarineResource->GetCommander() == pPlayer);
				break;
			}
		}
		if (!bSelectedByMe && !IsOfflineGame() )
		{
			//Msg("Returning false because this is multiplayer and he's not selected by me\n");
			return false;
		}
	}
	else
	{
		//Msg("returning false because marine isn't selected and this is multiplayer\n");
		return false;
	}

	if (!pProfile && MarineProfileList())
	{		
		pProfile = MarineProfileList()->GetProfile(iProfileIndex);
	}

	// check the marine isn't dead
	CASW_Campaign_Save *pSave = pGameResource->GetCampaignSave();
	if (!pSave || !pSave->IsMarineAlive(iProfileIndex) || !pProfile)
	{
		//Msg("returning false cos this isn't campaign: save = %d marinealiev=%d pProfile=%d index=%d\n",
			//pSave, pSave ? pSave->IsMarineAlive(iProfileIndex) : 0, pProfile, iProfileIndex );
		return false;
	}
	
	int iCurrentSkillValue = ASWGameResource()->GetMarineSkill( iProfileIndex, nSkillSlot );
	int iMaxSkillValue = MarineSkills()->GetMaxSkillPoints( pProfile->GetSkillMapping( nSkillSlot ) );
	int iSparePoints = ASWGameResource()->GetMarineSkill( iProfileIndex, ASW_SKILL_SLOT_SPARE );
	
	//Msg("returning comparison\n");
	return ((iCurrentSkillValue < iMaxSkillValue) && (iSparePoints > 0));
}

#ifndef CLIENT_DLL
bool CAlienSwarm::SpendSkill(int iProfileIndex, int nSkillSlot)
{
	if (iProfileIndex < 0 || iProfileIndex >= ASW_NUM_MARINE_PROFILES )
		return false;

	if (nSkillSlot < 0 || nSkillSlot >= ASW_SKILL_SLOT_SPARE)		// -1 since the last skill is 'spare' points
		return false;

	if (!ASWGameResource())
		return false;

	CASW_Marine_Profile *pProfile = MarineProfileList()->m_Profiles[iProfileIndex];
	if (!pProfile)
		return false;

	int iCurrentSkillValue = ASWGameResource()->GetMarineSkill(iProfileIndex, nSkillSlot);
	int iMaxSkillValue = MarineSkills()->GetMaxSkillPoints( pProfile->GetSkillMapping( nSkillSlot ) );
	int iSparePoints = ASWGameResource()->GetMarineSkill(iProfileIndex, ASW_SKILL_SLOT_SPARE);

	// skill is maxed out
	if (iCurrentSkillValue >= iMaxSkillValue)
		return false;

	// check we have some spare points
	if (iSparePoints <= 0)
		return false;

	// grab our current campaign save game
	CASW_Campaign_Save *pSave = ASWGameResource()->GetCampaignSave();
	if (!pSave)
		return false;

	// don't spend points on dead marines
	if (!pSave->IsMarineAlive(iProfileIndex))
		return false;

	// spend the point
	pSave->IncreaseMarineSkill(iProfileIndex, nSkillSlot);
	pSave->ReduceMarineSkill(iProfileIndex, ASW_SKILL_SLOT_SPARE);
	ASWGameResource()->UpdateMarineSkills(pSave);
	pSave->SaveGameToFile();	// save with the new stats

	// trigger an animation for anyone to see	
	CReliableBroadcastRecipientFilter users;
	users.MakeReliable();
	UserMessageBegin( users, "ASWSkillSpent" );
		WRITE_BYTE( iProfileIndex );
		WRITE_BYTE( nSkillSlot );
	MessageEnd();

	return true;
}

bool CAlienSwarm::SkillsUndo(CASW_Player *pPlayer, int iProfileIndex)
{
	if (!pPlayer)
		return false;

	if (!ASWGameResource())
		return false;

	if (iProfileIndex < 0 || iProfileIndex >= ASW_NUM_MARINE_PROFILES )
		return false;

	if (GetGameState() != ASW_GS_BRIEFING)
		return false;

	// grab our current campaign save game
	CASW_Campaign_Save *pSave = ASWGameResource()->GetCampaignSave();
	if (!pSave)
		return false;

	// don't undo if marine is dead
	if (!ASWGameResource()->GetCampaignSave()->IsMarineAlive(iProfileIndex))
		return false;

	bool bSinglePlayer = ASWGameResource()->IsOfflineGame();

	// check we have this marine selected
	CASW_Game_Resource *pGameResource = ASWGameResource();
	if (pGameResource->IsRosterSelected(iProfileIndex))
	{		
		bool bSelectedByMe = false;
		for (int i=0; i<pGameResource->GetMaxMarineResources();i++)
		{
			CASW_Marine_Resource *pMarineResource = pGameResource->GetMarineResource(i);
			if (pMarineResource && pMarineResource->GetProfile()->m_ProfileIndex == iProfileIndex)
			{
				bSelectedByMe = (pMarineResource->GetCommander() == pPlayer);
				break;                                     
			}
		}
		if (!bSelectedByMe)
			return false;
	}
	else if (!bSinglePlayer)	// has to be selected in multiplayer to manipulate skills
		return false;


	// revert skills
	pSave->RevertSkillsToUndoState(iProfileIndex);
	ASWGameResource()->UpdateMarineSkills(pSave);
	pSave->SaveGameToFile();	// save with the new stats
	return true;
}

void CAlienSwarm::OnSkillLevelChanged( int iNewLevel )
{
	DevMsg( "Skill level changed %d\n", iNewLevel );
	if ( !ASWDeathmatchMode() && !( GetGameState() == ASW_GS_BRIEFING || GetGameState() == ASW_GS_DEBRIEF ) )
	{
		m_bCheated = true;
	}

	const char *szDifficulty = "normal";
	if (iNewLevel == 1)	//  easy
	{
		m_iMissionDifficulty = 3;
		szDifficulty = "easy";
	}
	else if (iNewLevel == 3) // hard
	{
		m_iMissionDifficulty = 7;
		szDifficulty = "hard";
	}
	else if (iNewLevel == 4) // insane
	{
		m_iMissionDifficulty = 10;
		szDifficulty = "insane";
	}
	else if (iNewLevel == 5) // imba
	{
		m_iMissionDifficulty = 13;
		szDifficulty = "imba";
	}
	else  // normal
	{
		m_iMissionDifficulty = 5;
	}

	m_iMissionDifficulty += rd_difficulty_tier.GetInt() * 12;

	// modify mission difficulty by campaign modifier
	if ( IsCampaignGame() )
	{
		if ( GetCampaignInfo() && GetCampaignSave() )
		{
			int iCurrentLoc = GetCampaignSave()->m_iCurrentPosition;
			const RD_Campaign_Mission_t *mission = GetCampaignInfo()->GetMission( iCurrentLoc );
			if ( mission )
			{
				m_iMissionDifficulty += mission->DifficultyModifier;
			}
		}
	}

	// reduce difficulty by 1 for each missing marine
	if ( ASWGameResource() && asw_adjust_difficulty_by_number_of_marines.GetBool() )
	{
		int nMarines = ASWGameResource()->GetNumMarines( NULL, false );
		if ( nMarines == 3 )
		{
			m_iMissionDifficulty--;
		}
		else if ( nMarines <= 2 )
		{
			m_iMissionDifficulty -= 2;
		}
	}
	// rd_increase_difficulty_by_number_of_marines exists because ASBI disables
	// asw_adjust_difficulty_by_number_of_marines, but we want difficulty to still be
	// higher for 4+ players even in ASBI
	if ( ASWGameResource() && rd_increase_difficulty_by_number_of_marines.GetBool() )
	{
		int nMarines = ASWGameResource()->GetNumMarines( NULL, false );
		if ( nMarines > 7 )
		{
			m_iMissionDifficulty += 4;
		}
		else if ( nMarines == 7 )
		{
			m_iMissionDifficulty += 3;
		}
		else if ( nMarines == 6 )
		{
			m_iMissionDifficulty += 2;
		}
		else if ( nMarines == 5 )
		{
			m_iMissionDifficulty += 1;
		}
	}
	// make sure difficulty doesn't go too low
	m_iMissionDifficulty = MAX( m_iMissionDifficulty, 2 );

	// modify health of all live aliens
	if ( ASWSpawnManager() )
	{
		for ( int i = 0; i < ASWSpawnManager()->GetNumAlienClasses(); i++ )
		{
			FindAndModifyAlienHealth( ASWSpawnManager()->GetAlienClass( i )->m_pszAlienClass );
		}
	}

	CBaseEntity *pDoor = NULL;
	while ( ( pDoor = gEntList.FindEntityByClassname( pDoor, "asw_door" ) ) != NULL )
	{
		assert_cast< CASW_Door * >( pDoor )->UpdateDoorHealthOnMissionStart( m_iMissionDifficulty );
	}

	if ( gameeventmanager )
	{
		IGameEvent * event = gameeventmanager->CreateEvent( "difficulty_changed" );
		if ( event )
		{
			event->SetInt( "newDifficulty", iNewLevel );
			event->SetInt( "oldDifficulty", m_iSkillLevel );
			event->SetString( "strDifficulty", szDifficulty );
			gameeventmanager->FireEvent( event );
		}
	}

	UpdateMatchmakingTagsCallback( NULL, "0", 0.0f );
		
	m_iSkillLevel = iNewLevel;
}

void CAlienSwarm::FindAndModifyAlienHealth( const char *szClass )
{
	if ( !szClass || szClass[0] == 0 )
		return;

	CBaseEntity *pEntity = NULL;
	while ( ( pEntity = gEntList.FindEntityByClassname( pEntity, szClass ) ) != NULL )
	{
		IASW_Spawnable_NPC *pNPC = dynamic_cast< IASW_Spawnable_NPC * >( pEntity );
		Assert( pNPC );
		if ( pNPC )
		{
			pNPC->SetHealthByDifficultyLevel();
		}
	}
}

void CAlienSwarm::RequestSkill( CASW_Player *pPlayer, int nSkill )
{
	if ( !( m_iGameState == ASW_GS_BRIEFING || m_iGameState == ASW_GS_DEBRIEF ) )	// don't allow skill change outside of briefing
		return;

	if ( nSkill >= 1 && nSkill <= 5 && ASWGameResource() && ASWGameResource()->GetLeader() == pPlayer )
	{
		ConVar *var = (ConVar *)cvar->FindVar( "asw_skill" );
		if (var)
		{
			int iOldSkill = var->GetInt();

			var->SetValue( nSkill );

			if ( iOldSkill != var->GetInt() )
			{
				CReliableBroadcastRecipientFilter filter;
				filter.RemoveRecipient( pPlayer );		// notify everyone except the player changing the difficulty level
				switch(var->GetInt())
				{
				case 1: UTIL_ClientPrintFilter( filter, ASW_HUD_PRINTTALKANDCONSOLE, "#asw_set_difficulty_easy", pPlayer->GetPlayerName() ); break;
				case 2: UTIL_ClientPrintFilter( filter, ASW_HUD_PRINTTALKANDCONSOLE, "#asw_set_difficulty_normal", pPlayer->GetPlayerName() ); break;
				case 3: UTIL_ClientPrintFilter( filter, ASW_HUD_PRINTTALKANDCONSOLE, "#asw_set_difficulty_hard", pPlayer->GetPlayerName() ); break;
				case 4: UTIL_ClientPrintFilter( filter, ASW_HUD_PRINTTALKANDCONSOLE, "#asw_set_difficulty_insane", pPlayer->GetPlayerName() ); break;
				case 5: UTIL_ClientPrintFilter( filter, ASW_HUD_PRINTTALKANDCONSOLE, "#asw_set_difficulty_imba", pPlayer->GetPlayerName() ); break;
				}
			}
		}
	}
}

void CAlienSwarm::RequestSkillUp(CASW_Player *pPlayer)
{
	if (m_iGameState != ASW_GS_BRIEFING)	// don't allow skill change outside of briefing
		return;

	if (m_iSkillLevel < 5 && ASWGameResource() && ASWGameResource()->GetLeader() == pPlayer)
	{
		ConVar *var = (ConVar *)cvar->FindVar( "asw_skill" );
		if (var)
		{
			var->SetValue(m_iSkillLevel + 1);
		}
	}
		//SetSkillLevel(m_iSkillLevel + 1);
}

void CAlienSwarm::RequestSkillDown(CASW_Player *pPlayer)
{
	if (m_iGameState != ASW_GS_BRIEFING)	// don't allow skill change outside of briefing
		return;

	if (m_iSkillLevel > 1 && ASWGameResource() && ASWGameResource()->GetLeader() == pPlayer)
	{
		ConVar *var = (ConVar *)cvar->FindVar( "asw_skill" );
		if (var)
		{
			var->SetValue(m_iSkillLevel - 1);
		}
	}

		//SetSkillLevel(m_iSkillLevel - 1);
}

/*
ConVar asw_alien_health_scale_easy(		"asw_alien_health_scale_easy",		"1.0", 0, "How much alien health is changed per mission difficulty level");
ConVar asw_alien_health_scale_normal(	"asw_alien_health_scale_normal",	"2.0", 0, "How much alien health is changed per mission difficulty level");
ConVar asw_alien_health_scale_hard(		"asw_alien_health_scale_hard",		"3.0", 0, "How much alien health is changed per mission difficulty level");
ConVar asw_alien_health_scale_insane(	"asw_alien_health_scale_insane",	"3.5", 0, "How much alien health is changed per mission difficulty level");
ConVar asw_alien_health_scale_brutal(	"asw_alien_health_scale_brutal",	"4.0", 0, "How much alien health is changed per mission difficulty level");
*/

// alters alien health by 20% per notch away from 8
float CAlienSwarm::ModifyAlienHealthBySkillLevel(float health)
{
	float fDiff = GetMissionDifficulty() - 5;
	float f = 1.0 + fDiff * asw_difficulty_alien_health_step.GetFloat();
	
	// commented and added mine scale factors
	/*float f = 1.0f; 
	switch (m_iSkillLevel)
	{
	case 1:
		f = asw_alien_health_scale_easy.GetFloat();
		break;
	case 2:
		f = asw_alien_health_scale_normal.GetFloat();
		break;
	case 3:
		f = asw_alien_health_scale_hard.GetFloat();
		break;
	case 4:
		f = asw_alien_health_scale_insane.GetFloat();
		break;
	case 5:
		f = asw_alien_health_scale_brutal.GetFloat();
		break;
	default:
		Assert(false && "m_iSkillLevel unknown value found in ModifyAlienHealthBySkillLevel()");
	}
	// end of mine code //*/

	return f * health * rd_heavy_scale.GetFloat();
}

// alters damage by 20% per notch away from 8
float CAlienSwarm::ModifyAlienDamageBySkillLevel( float flDamage )
{
	if (asw_debug_alien_damage.GetBool())
		Msg("Modifying alien damage by difficulty level. Base damage is %f.  Modified is ", flDamage);
	float fDiff = GetMissionDifficulty() - 5;
	float f = 1.0 + fDiff * asw_difficulty_alien_damage_step.GetFloat();
	if (asw_debug_alien_damage.GetBool())
		Msg("%f\n", (f* flDamage));
	return MAX(1.0f, f * flDamage);
}

void CAlienSwarm::ClientSettingsChanged( CBasePlayer *pPlayer )
{
	const char *pszNewName = engine->GetClientConVarValue( pPlayer->entindex(), "name" );

	const char *pszOldName = pPlayer->GetPlayerName();

	CASW_Player *pASWPlayer = (CASW_Player*)pPlayer;

	// first check if new name is really different
	if (  pszOldName[0] != 0 && 
		  Q_strncmp( pszOldName, pszNewName, MAX_PLAYER_NAME_LENGTH-1 ) )
	{
		// ok, player send different name

		// check if player is allowed to change it's name
		if ( pASWPlayer->CanChangeName() )
		{
			// change name instantly
			pASWPlayer->ChangeName( pszNewName );
		}
		else
		{
			// no change allowed, force engine to use old name again
			engine->ClientCommand( pPlayer->edict(), "name \"%s\"", pszOldName );
		}
	}

	const char *pszFov = engine->GetClientConVarValue( pPlayer->entindex(), "fov_desired" );
	if ( pszFov )
	{
		int iFov = atoi(pszFov);
		if ( pASWPlayer->GetASWControls() == ASWC_TOPDOWN )
			iFov = clamp( iFov, 20, 75 );
		else
			iFov = clamp( iFov, 20, 120 );
		pPlayer->SetDefaultFOV( iFov );
	}
}

// something big in the level has exploded and failed the mission for us
void CAlienSwarm::ExplodedLevel()
{
	if (GetGameState() == ASW_GS_INGAME)
	{
		MissionComplete(false);
		// kill all marines
		CASW_Game_Resource *pGameResource = ASWGameResource();
		if (pGameResource)
		{
			for (int i=0;i<pGameResource->GetMaxMarineResources();i++)
			{
				CASW_Marine_Resource *pMR = pGameResource->GetMarineResource(i);
				if (!pMR)
					continue;
				
				CASW_Marine *pMarine = pMR->GetMarineEntity();
				if (!pMarine || pMarine->GetHealth() <= 0)
					continue;

				//CTakeDamageInfo info(NULL, NULL, Vector(0,0,0), pMarine->GetAbsOrigin(), 2000,
						//DMG_NEVERGIB);
				//pMarine->TakeDamage(info);

				//if (pMarine->m_iHealth > 0)
				//{
					pMarine->m_iHealth = 0;
					pMarine->Event_Killed( CTakeDamageInfo( pMarine, pMarine, 0, DMG_NEVERGIB ) );
					pMarine->Event_Dying();
				//}
			}
		}
	}
}

edict_t *CAlienSwarm::DoFindClientInPVS( edict_t *pEdict, unsigned char *pvs, unsigned pvssize )
{
	CBaseEntity *pe = GetContainingEntity( pEdict );
	if ( !pe )
		return NULL;

	Vector view = pe->EyePosition();
	bool bCorpseCanSee;
	CASW_Marine *pEnt = UTIL_ASW_AnyMarineCanSee(view, 384.0f, bCorpseCanSee);
	if (pEnt)
		return pEnt->edict();

	return NULL;		// returns a marine who can see us	
}

void CAlienSwarm::SetInfoHeal( CASW_Info_Heal *pInfoHeal )
{
	if ( m_hInfoHeal.Get() )
	{
		Msg("Warning: Only 1 asw_info_heal allowed per map!\n" );
	}
	m_hInfoHeal = pInfoHeal;
}

CASW_Info_Heal *CAlienSwarm::GetInfoHeal()
{
	return m_hInfoHeal.Get();
}

// returns the lowest skill level a mission was completed on, in a campaign game
int CAlienSwarm::GetLowestSkillLevelPlayed()
{
	if (!IsCampaignGame() || !GetCampaignSave() || GetCampaignSave()->m_iLowestSkillLevelPlayed == 0)
	{
		return GetSkillLevel();
	}

	return GetCampaignSave()->m_iLowestSkillLevelPlayed;
}

void CAlienSwarm::ClearLeaderKickVotes(CASW_Player *pPlayer, bool bClearLeader, bool bClearKick)
{
	if (!pPlayer || !ASWGameResource())
		return;

	int iSlotIndex = pPlayer->entindex();		// keep index 1 based for comparing the player set indices
	if (iSlotIndex <= 0 || iSlotIndex > ASW_MAX_READY_PLAYERS) //valid range before making zero based is [1..ASW_MAX_READY_PLAYERS]
		return;

	// unflag any players voting for him
	for ( int i = 1; i <= gpGlobals->maxClients; i++ )	
	{
		CASW_Player* pOtherPlayer = ToASW_Player(UTIL_PlayerByIndex(i));

		if ( pOtherPlayer && pOtherPlayer->IsConnected())
		{
			if ( bClearLeader && pOtherPlayer->m_iLeaderVoteIndex == iSlotIndex )
				pOtherPlayer->m_iLeaderVoteIndex = -1;
			if ( bClearKick && pOtherPlayer->m_iKickVoteIndex == iSlotIndex )
				pOtherPlayer->m_iKickVoteIndex = -1;
		}
	}

	iSlotIndex -= 1;	// make index zero based for our total arrays

	// reset his totals
	if (bClearLeader)
		ASWGameResource()->m_iLeaderVotes.Set(iSlotIndex, 0);
	if (bClearKick)
		ASWGameResource()->m_iKickVotes.Set(iSlotIndex, 0);
}

void CAlienSwarm::SetLeaderVote( CASW_Player *pPlayer, int iPlayerIndex )
{
	Assert( pPlayer && pPlayer->CanVote() );
	if ( !pPlayer || !pPlayer->CanVote() )
	{
		return;
	}

	CASW_Player *pOtherPlayer = ToASW_Player( UTIL_PlayerByIndex( iPlayerIndex ) );
	Assert( !pOtherPlayer || pOtherPlayer->CanBeLeader() );
	if ( pOtherPlayer && !pOtherPlayer->CanBeLeader() )
	{
		return;
	}

	// if we're leader, then allow us to give leadership over to someone immediately
	if ( ASWGameResource() && pPlayer == ASWGameResource()->GetLeader() )
	{
		if ( pOtherPlayer && pOtherPlayer != pPlayer )
		{
			ASWGameResource()->SetLeader( pOtherPlayer );
			CASW_Game_Resource::s_bLeaderGivenDifficultySuggestion = false;
			UTIL_ClientPrintAll( ASW_HUD_PRINTTALKANDCONSOLE, "#asw_player_made_leader", pOtherPlayer->GetPlayerName() );
			ClearLeaderKickVotes( pOtherPlayer, true, false );
			return;
		}
	}

	int iOldPlayer = pPlayer->m_iLeaderVoteIndex;
	pPlayer->m_iLeaderVoteIndex = iPlayerIndex;

	// if we were previously voting for someone, update their vote count	
	if ( iOldPlayer != -1 )
	{
		// this loop goes through every player, counting how many players have voted for the old guy
		int iOldPlayerVotes = 0;
		for ( int i = 1; i <= gpGlobals->maxClients; i++ )
		{
			CASW_Player *pVoter = ToASW_Player( UTIL_PlayerByIndex( i ) );

			if ( pVoter && pVoter->IsConnected() && pVoter->CanVote() )
			{
				if ( pVoter->m_iLeaderVoteIndex == iOldPlayer )
					iOldPlayerVotes++;
			}
		}

		// updates the target player's game resource entry with the number of leader votes against him
		if ( iOldPlayer > 0 && iOldPlayer <= ASW_MAX_READY_PLAYERS && ASWGameResource() )
		{
			ASWGameResource()->m_iLeaderVotes.Set( iOldPlayer - 1, iOldPlayerVotes );
		}
	}

	if ( !pOtherPlayer )
	{
		return;
	}

	// check if this player has enough votes now
	int iVotes = 0;
	int iPlayers = 0;
	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CASW_Player *pVoter = ToASW_Player( UTIL_PlayerByIndex( i ) );

		if ( pVoter && pVoter->IsConnected() && pVoter->CanVote() )
		{
			if ( pVoter->m_iLeaderVoteIndex == iPlayerIndex )
				iVotes++;

			iPlayers++;
		}
	}

	if ( iPlayerIndex > 0 && iPlayerIndex <= ASW_MAX_READY_PLAYERS && ASWGameResource() )
	{
		ASWGameResource()->m_iLeaderVotes.Set( iPlayerIndex - 1, iVotes );
	}

	int iVotesNeeded = Ceil2Int( asw_vote_leader_fraction.GetFloat() * iPlayers );
	// make sure we're not rounding down the number of needed players
	if ( iVotesNeeded < 2 )
		iVotesNeeded = 2;

	if ( iPlayerIndex > 0 && iVotes >= iVotesNeeded )
	{
		// make this player leader!
		if ( pOtherPlayer && ASWGameResource() )
		{
			ASWGameResource()->SetLeader( pOtherPlayer );
			CASW_Game_Resource::s_bLeaderGivenDifficultySuggestion = false;
			UTIL_ClientPrintAll( ASW_HUD_PRINTTALKANDCONSOLE, "#asw_player_made_leader", pOtherPlayer->GetPlayerName() );
			ClearLeaderKickVotes( pOtherPlayer, true, false );
		}
	}
	else if ( iVotes == 1 && iPlayerIndex != -1 )	// if this is the first vote of this kind, check about announcing it
	{
		pPlayer->m_iKLVotesStarted++;
		if ( pPlayer->m_iKLVotesStarted < 3 || ( gpGlobals->curtime - pPlayer->m_fLastKLVoteTime ) > 10.0f && pOtherPlayer )
		{
			UTIL_ClientPrintAll( ASW_HUD_PRINTTALKANDCONSOLE, "#asw_started_leader_vote", pPlayer->GetPlayerName(), pOtherPlayer->GetPlayerName() );
		}
		pPlayer->m_fLastKLVoteTime = gpGlobals->curtime;
	}
}

void CAlienSwarm::SetKickVote( CASW_Player *pPlayer, int iPlayerIndex )
{
	Assert( pPlayer && pPlayer->CanVote() );
	if ( !pPlayer || !pPlayer->CanVote() )
	{
		return;
	}

	CASW_Player *pOtherPlayer = ToASW_Player( UTIL_PlayerByIndex( iPlayerIndex ) );
	Assert( !pOtherPlayer || pOtherPlayer->CanBeKicked() );
	if ( pOtherPlayer && !pOtherPlayer->CanBeKicked() )
	{
		return;
	}

	// if we're leader & this is listen server, then allow us to kick immediately
	if ( ASWGameResource() && pPlayer == ASWGameResource()->GetLeader() && !engine->IsDedicatedServer() )
	{
		if ( pOtherPlayer && pOtherPlayer != pPlayer )
		{
			// kick this player
			ClearLeaderKickVotes( pOtherPlayer, false, true );

			ClientPrint( pOtherPlayer, HUD_PRINTCONSOLE, "#asw_kicked_by_vote" );
			UTIL_ClientPrintAll( ASW_HUD_PRINTTALKANDCONSOLE, "#asw_player_kicked", pOtherPlayer->GetPlayerName() );

			bool bPlayerCrashed = false;
			INetChannelInfo *pNetChanInfo = engine->GetPlayerNetInfo( pOtherPlayer->entindex() );
			if ( !pNetChanInfo || pNetChanInfo->IsTimingOut() )
			{
				// don't ban the player
				DevMsg( "Will not ban kicked player: net channel was idle for %.2f sec.\n", pNetChanInfo ? pNetChanInfo->GetTimeSinceLastReceived() : 0.0f );
				bPlayerCrashed = true;
			}
			if ( ( sv_vote_kick_ban_duration.GetInt() > 0 ) && !bPlayerCrashed )
			{
				// don't roll the kick command into this, it will fail on a lan, where kickid will go through
				engine->ServerCommand( CFmtStr( "banid %d %d;", sv_vote_kick_ban_duration.GetInt(), pOtherPlayer->GetUserID() ) );
			}

			char buffer[256];
			Q_snprintf( buffer, sizeof( buffer ), "kickid %d Kicked by player vote.\n", pOtherPlayer->GetUserID() );
			Msg( "sending command: %s\n", buffer );
			engine->ServerCommand( buffer );

			return;
		}
	}

	int iOldPlayer = pPlayer->m_iKickVoteIndex;
	pPlayer->m_iKickVoteIndex = iPlayerIndex;

	// if we were previously voting for someone, update their vote count	
	if ( iOldPlayer != -1 )
	{
		// this loop goes through every player, counting how many players have voted for the old guy
		int iOldPlayerVotes = 0;
		for ( int i = 1; i <= gpGlobals->maxClients; i++ )
		{
			CASW_Player *pVoter = ToASW_Player( UTIL_PlayerByIndex( i ) );

			if ( pVoter && pVoter->IsConnected() && pVoter->CanVote() )
			{
				if ( pVoter->m_iKickVoteIndex == iOldPlayer )
					iOldPlayerVotes++;
			}
		}

		// updates the target player's game resource entry with the number of kick votes against him
		if ( iOldPlayer > 0 && iOldPlayer <= ASW_MAX_READY_PLAYERS && ASWGameResource() )
		{
			ASWGameResource()->m_iKickVotes.Set( iOldPlayer - 1, iOldPlayerVotes );
		}
	}

	if ( !pOtherPlayer )
	{
		return;
	}

	// check if this player has enough votes now to be kicked
	int iVotes = 0;
	int iPlayers = 0;
	// this loop goes through every player, counting how many players there are and how many there are that
	//  have voted for the same player to be kicked as the one that started this function
	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CASW_Player *pVoter = ToASW_Player( UTIL_PlayerByIndex( i ) );

		if ( pVoter && pVoter->IsConnected() && pVoter->CanVote() )
		{
			if ( pVoter->m_iKickVoteIndex == iPlayerIndex )
				iVotes++;

			iPlayers++;
		}
	}

	// updates the target player's game resource entry with the number of kick votes against him
	if ( iPlayerIndex > 0 && iPlayerIndex <= ASW_MAX_READY_PLAYERS && ASWGameResource() )
	{
		ASWGameResource()->m_iKickVotes.Set( iPlayerIndex - 1, iVotes );
	}

	int iVotesNeeded = Ceil2Int( asw_vote_kick_fraction.GetFloat() * iPlayers );
	// make sure we're not rounding down the number of needed players
	if ( iVotesNeeded < 2 )
	{
		iVotesNeeded = 2;
	}

	Msg( "Players %d, Votes %d, Votes needed %d\n", iPlayers, iVotes, iVotesNeeded );

	if ( iPlayerIndex > 0 && iVotes >= iVotesNeeded )
	{
		// kick this player
		ClearLeaderKickVotes( pOtherPlayer, false, true );

		Msg( "kick voting %d out (votes %d/%d)\n", iPlayerIndex, iVotes, iVotesNeeded );
		ClientPrint( pOtherPlayer, HUD_PRINTCONSOLE, "#asw_kicked_by_vote" );
		UTIL_ClientPrintAll( ASW_HUD_PRINTTALKANDCONSOLE, "#asw_player_kicked", pOtherPlayer->GetPlayerName() );

		bool bPlayerCrashed = false;
		INetChannelInfo *pNetChanInfo = engine->GetPlayerNetInfo( pOtherPlayer->entindex() );
		if ( !pNetChanInfo || pNetChanInfo->IsTimingOut() )
		{
			// don't ban the player
			DevMsg( "Will not ban kicked player: net channel was idle for %.2f sec.\n", pNetChanInfo ? pNetChanInfo->GetTimeSinceLastReceived() : 0.0f );
			bPlayerCrashed = true;
		}

		if ( sv_vote_kick_ban_duration.GetInt() > 0 && !bPlayerCrashed )
		{
			// don't roll the kick command into this, it will fail on a lan, where kickid will go through
			engine->ServerCommand( CFmtStr( "banid %d %d;", sv_vote_kick_ban_duration.GetInt(), pOtherPlayer->GetUserID() ) );
		}

		char buffer[256];
		Q_snprintf( buffer, sizeof( buffer ), "kickid %d Kicked by player vote.\n", pOtherPlayer->GetUserID() );
		Msg( "sending command: %s\n", buffer );
		engine->ServerCommand( buffer );
	}
	else if ( iVotes == 1 && iPlayerIndex != -1 )	// if this is the first vote of this kind, check about announcing it
	{
		pPlayer->m_iKLVotesStarted++;
		if ( pPlayer->m_iKLVotesStarted < 3 || ( gpGlobals->curtime - pPlayer->m_fLastKLVoteTime ) > 10.0f )
		{
			UTIL_ClientPrintAll( ASW_HUD_PRINTTALKANDCONSOLE, "#asw_started_kick_vote", pPlayer->GetPlayerName(), pOtherPlayer->GetPlayerName() );
		}
		pPlayer->m_fLastKLVoteTime = gpGlobals->curtime;
	}
}

void CAlienSwarm::StartVote( CASW_Player *pPlayer, int iVoteType, const char *szVoteName, int nCampaignIndex )
{
	Assert( pPlayer && pPlayer->CanVote() );
	if ( !pPlayer || !pPlayer->CanVote() )
		return;

	if ( GetCurrentVoteType() != ASW_VOTE_NONE )
	{
		ClientPrint( pPlayer, ASW_HUD_PRINTTALKANDCONSOLE, "#asw_vote_already_in_progress" );
		return;
	}

	if ( iVoteType <= ASW_VOTE_SAVED_CAMPAIGN || iVoteType > ASW_VOTE_CHANGE_MISSION )
	{
		return;
	}

	if ( !Q_stricmp( szVoteName, "lobby" ) )
	{
		return;
	}

	// Check this is a valid vote, i.e.:
	//   if it's a map change, check that map exists
	//   if it's a campaign change, check that campaign exists
	//   if it's a saved campaign game, check that save exists
	// This can all be done by querying the local mission source...
	if ( !missionchooser || !missionchooser->LocalMissionSource() )
		return;

	IASW_Mission_Chooser_Source *pMissionSource = missionchooser->LocalMissionSource();

	const RD_Mission_t *pMission = NULL;
	if ( iVoteType == ASW_VOTE_CHANGE_MISSION )
	{
		pMission = ReactiveDropMissions::GetMission( szVoteName );
		if ( !pMission )
		{
			ClientPrint( pPlayer, ASW_HUD_PRINTTALKANDCONSOLE, "#asw_mission_doesnt_exist" );
			return;
		}
	}
	if ( iVoteType == ASW_VOTE_SAVED_CAMPAIGN && !pMissionSource->SavedCampaignExists( szVoteName ) )
	{
		ClientPrint( pPlayer, ASW_HUD_PRINTTALKANDCONSOLE, "#asw_save_doesnt_exist" );
		return;
	}

	const RD_Campaign_t *pContainingCampaign = NULL;
	if ( iVoteType == ASW_VOTE_CHANGE_MISSION && nCampaignIndex != -1 )
	{
		pContainingCampaign = ReactiveDropMissions::GetCampaign( nCampaignIndex );
		if ( !pContainingCampaign )
		{
			ClientPrint( pPlayer, ASW_HUD_PRINTTALKANDCONSOLE, "#asw_campaign_doesnt_exist" );
			return;
		}
	}

	// start the new vote!
	m_iCurrentVoteType = iVoteType;
	Q_strncpy( m_szCurrentVoteName, szVoteName, 128 );
	// store a pretty description if we can
	if ( iVoteType == ASW_VOTE_CHANGE_MISSION )
	{
		Q_strncpy( m_szCurrentVoteDescription.GetForModify(), STRING( pMission->MissionTitle ), 128 );
		Q_strncpy( m_szCurrentVoteMapName.GetForModify(), szVoteName, 128 );
		if ( !pContainingCampaign )
		{
			Q_strncpy( m_szCurrentVoteCampaignName.GetForModify(), "", 128 );
		}
		else
		{
			Q_strncpy( m_szCurrentVoteCampaignName.GetForModify(), pContainingCampaign->BaseName, 128 );
		}
	}
	else if ( iVoteType == ASW_VOTE_SAVED_CAMPAIGN )
	{
		Q_strncpy( m_szCurrentVoteDescription.GetForModify(), pMissionSource->GetPrettySavedCampaignName( szVoteName ), 128 );
	}
	m_iCurrentVoteYes = 0;
	m_iCurrentVoteNo = 0;
	m_fVoteEndTime = gpGlobals->curtime + asw_vote_duration.GetFloat();
	m_PlayersVoted.Purge();

	// clear out current votes for all players
	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CASW_Player *pOtherPlayer = ToASW_Player( UTIL_PlayerByIndex( i ) );
		if ( pOtherPlayer )
			pOtherPlayer->m_iMapVoted = 0;
	}

	// print a message telling players about the new vote that has started and how to bring up their voting options
	const char *desc = m_szCurrentVoteDescription;
	if ( iVoteType == ASW_VOTE_CHANGE_MISSION )
	{
		UTIL_ClientPrintAll( ASW_HUD_PRINTTALKANDCONSOLE, "#asw_vote_mission_start", pPlayer->GetPlayerName(), desc );
	}
	else if ( iVoteType == ASW_VOTE_SAVED_CAMPAIGN )
	{
		UTIL_ClientPrintAll( ASW_HUD_PRINTTALKANDCONSOLE, "#asw_vote_saved_start", pPlayer->GetPlayerName(), desc );
	}
	// 	else if (iVoteType == ASW_VOTE_CAMPAIGN)
	// 	{
	// 		UTIL_ClientPrintAll(ASW_HUD_PRINTTALKANDCONSOLE, "#asw_vote_campaign_start", pPlayer->GetPlayerName(), desc);		
	// 	}

	//UTIL_ClientPrintAll(ASW_HUD_PRINTTALKANDCONSOLE, "#asw_press_vote_key", "^%playerlist%" );
	CastVote( pPlayer, true );
}

void CAlienSwarm::CastVote( CASW_Player *pPlayer, bool bVoteYes )
{
	Assert( pPlayer && pPlayer->CanVote() );
	if ( !pPlayer || !pPlayer->CanVote() )
	{
		return;
	}

	if ( m_iCurrentVoteType == ASW_VOTE_NONE )
	{
		return;
	}

	// get an ID for this player
	const char *pszNetworkID = pPlayer->GetASWNetworkID();

	// check this player hasn't voted already
	for ( int i = 0; i < m_PlayersVoted.Count(); i++ )
	{
		const char *p = STRING( m_PlayersVoted[i] );
		if ( !Q_strcmp( p, pszNetworkID ) )
		{
			ClientPrint( pPlayer, ASW_HUD_PRINTTALKANDCONSOLE, "#asw_already_voted" );
			return;
		}
	}

	pPlayer->m_iMapVoted = bVoteYes ? 2 : 1;

	// add this player to the list of those who have voted
	string_t stringID = AllocPooledString( pszNetworkID );
	m_PlayersVoted.AddToTail( stringID );

	// count his vote
	if ( bVoteYes )
		m_iCurrentVoteYes++;
	else
		m_iCurrentVoteNo++;

	UpdateVote();
}

// removes a player's vote from the current vote (used when they disconnect)
void CAlienSwarm::RemoveVote( CASW_Player *pPlayer )
{
	Assert( pPlayer && pPlayer->CanVote() );
	if ( !pPlayer || !pPlayer->CanVote() || pPlayer->m_iMapVoted.Get() == 0 )
	{
		return;
	}

	if ( m_iCurrentVoteType == ASW_VOTE_NONE )
	{
		return;
	}

	// count his vote
	if ( pPlayer->m_iMapVoted.Get() == 2 )
		m_iCurrentVoteYes--;
	else
		m_iCurrentVoteNo--;

	UpdateVote();
}

void CAlienSwarm::UpdateVote()
{
	if ( m_iCurrentVoteType == ASW_VOTE_NONE )
	{
		return;
	}

	// check if a yes/no total has reached the amount needed to make a decision
	int iNumPlayers = 0;
	for ( int i = 1; i < gpGlobals->maxClients; i++ )
	{
		CASW_Player *pPlayer = ToASW_Player( UTIL_PlayerByIndex( i ) );
		if ( pPlayer && pPlayer->IsConnected() && pPlayer->CanVote() )
		{
			iNumPlayers++;
		}
	}

	int iNeededVotes = Ceil2Int( asw_vote_map_fraction.GetFloat() * iNumPlayers );
	// make sure we're not rounding down the number of needed players
	if ( iNeededVotes < 1 )
	{
		iNeededVotes = 1;
	}

	bool bSinglePlayer = ASWGameResource() && ASWGameResource()->IsOfflineGame();

	if ( m_iCurrentVoteYes >= iNeededVotes )
	{
		if ( ASWGameResource() )
			ASWGameResource()->RememberLeaderID();

		// make it so
		UTIL_ClientPrintAll( ASW_HUD_PRINTTALKANDCONSOLE, "#asw_vote_passed" );
		if ( m_iCurrentVoteType == ASW_VOTE_CHANGE_MISSION )
		{
			// if we're ingame, then upload for state (we don't do this once the mission is over, as stats are already sent on MissionComplete)
			// stats todo:
			//if (GetGameState() == ASW_GS_INGAME && ASWGameStats())
				//ASWGameStats()->AddMapRecord();

			const char *szCampaignName = GetCurrentVoteCampaignName();
			const char *szMissionMode = "campaign";
			if ( !szCampaignName || !szCampaignName[0] )
			{
				szMissionMode = "single_mission";
				szCampaignName = asw_default_campaign.GetString();
			}

			// start a new campaign on the specified mission
			IASW_Mission_Chooser_Source *pSource = missionchooser ? missionchooser->LocalMissionSource() : NULL;
			if ( !pSource )
				return;

			char szSaveFilename[MAX_PATH];
			szSaveFilename[0] = 0;
			const char *szStartingMission = GetCurrentVoteName();
			if ( !pSource->ASW_Campaign_CreateNewSaveGame( &szSaveFilename[0], sizeof( szSaveFilename ), szCampaignName, ( gpGlobals->maxClients > 1 ), szStartingMission ) )
			{
				Msg( "Unable to create new save game.\n" );
				return;
			}
			engine->ServerCommand( CFmtStr( "changelevel %s %s %s\n",
				szStartingMission ? szStartingMission : "lobby",
				szMissionMode,
				szSaveFilename ) );
		}
		else if ( m_iCurrentVoteType == ASW_VOTE_SAVED_CAMPAIGN )
		{
			// if we're ingame, then upload for state (we don't do this once the mission is over, as stats are already sent on MissionComplete)
			// stats todo:
			//if (GetGameState() == ASW_GS_INGAME && ASWGameStats())
				//ASWGameStats()->AddMapRecord();

			// check the save file still exists (in very rare cases it could have been deleted automatically and the player clicked really fast!)
			if ( !missionchooser || !missionchooser->LocalMissionSource() || missionchooser->LocalMissionSource()->SavedCampaignExists( GetCurrentVoteName() ) )
			{
				// load this saved campaign game
				char szMapCommand[MAX_PATH];
				Q_snprintf( szMapCommand, sizeof( szMapCommand ), "asw_server_loadcampaign %s %s change\n",
					GetCurrentVoteName(),
					bSinglePlayer ? "SP" : "MP"
				);
				engine->ServerCommand( szMapCommand );
			}
		}

		m_iCurrentVoteType = ( int )ASW_VOTE_NONE;
		m_iCurrentVoteYes = 0;
		m_iCurrentVoteNo = 0;
		m_fVoteEndTime = 0;
		m_PlayersVoted.Purge();
	}
	else if ( asw_vote_map_fraction.GetFloat() <= 1.0f && ( m_iCurrentVoteNo >= iNeededVotes || gpGlobals->curtime >= m_fVoteEndTime		// check if vote has timed out also
		|| ( m_iCurrentVoteNo + m_iCurrentVoteYes ) >= iNumPlayers ) )			// or if everyone's voted and it didn't trigger a yes
	{
		// the people decided against this vote, clear it all
		UTIL_ClientPrintAll( ASW_HUD_PRINTTALKANDCONSOLE, "#asw_vote_failed" );
		m_iCurrentVoteType = ( int )ASW_VOTE_NONE;
		m_iCurrentVoteYes = 0;
		m_iCurrentVoteNo = 0;
		m_fVoteEndTime = 0;
		m_PlayersVoted.Purge();
	}
}

void CAlienSwarm::StopAllAmbientSounds()
{
	CBaseEntity *pAmbient = gEntList.FindEntityByClassname( NULL, "ambient_generic" );
	while ( pAmbient != NULL )
	{
		//Msg("sending ambient generic fade out to entity %d %s\n", pAmbient->entindex(), STRING(pAmbient->GetEntityName()));
		pAmbient->KeyValue("fadeout", "1");
		pAmbient->SetNextThink( gpGlobals->curtime + 0.1f );
		pAmbient = gEntList.FindEntityByClassname( pAmbient, "ambient_generic" );
	}

	CBaseEntity *pFire = gEntList.FindEntityByClassname( NULL, "env_fire" );
	while ( pFire != NULL )
	{
		FireSystem_ExtinguishFire(pFire);
		pFire = gEntList.FindEntityByClassname( pFire, "env_fire" );
	}
}

void CAlienSwarm::StartAllAmbientSounds()
{
	CBaseEntity *pAmbient = gEntList.FindEntityByClassname( NULL, "ambient_generic" );
	while ( pAmbient != NULL )
	{		
		pAmbient->KeyValue("aswactivate", "1");		
		pAmbient = gEntList.FindEntityByClassname( pAmbient, "ambient_generic" );
	}
}

bool CAlienSwarm::AllowSoundscapes()
{
	if (UTIL_ASW_MissionHasBriefing(STRING(gpGlobals->mapname)) && GetGameState() != ASW_GS_INGAME)
		return false;

	return true;
}

void CAlienSwarm::SetForceReady(int iForceReadyType)
{
	if (iForceReadyType < ASW_FR_NONE || iForceReadyType > ASW_FR_INGAME_RESTART)
		return;
	// don't allow restarting if we're on the campaign map, as this does Bad Things (tm)  NOTE: Campaign stuff has it's own force launch code
	if (GetGameState() == ASW_GS_CAMPAIGNMAP)
		return;
	// make sure force ready types are being requested at the right stage of the game
	if (iForceReadyType == ASW_FR_BRIEFING && GetGameState() != ASW_GS_BRIEFING)
		return;
	if (iForceReadyType >= ASW_FR_CONTINUE && iForceReadyType <= ASW_FR_CAMPAIGN_MAP && GetGameState() != ASW_GS_DEBRIEF)
		return;
	// don't allow force ready to be changed if there's already a force ready in progress (unless we're clearing it)
	if (m_iForceReadyType != ASW_FR_NONE && iForceReadyType != ASW_FR_NONE)
		return;
	if ( iForceReadyType == ASW_FR_INGAME_RESTART && GetGameState() != ASW_GS_INGAME )
		return;

	m_iForceReadyType = iForceReadyType;

	if (iForceReadyType == ASW_FR_NONE)
	{
		m_fForceReadyTime = 0;
		m_iForceReadyCount = 0;
		return;
	}

	m_fForceReadyTime = rd_auto_fast_restart.GetBool() ? gpGlobals->curtime + 1.9f : gpGlobals->curtime + 5.9f;
	m_iForceReadyCount = rd_auto_fast_restart.GetBool() ? 1 : 5;

	// check to see if it should end immediately
	CheckForceReady();

	// if it didn't, print a message saying we're starting a countdown
	if (m_iForceReadyType == ASW_FR_INGAME_RESTART)
	{
		//UTIL_ClientPrintAll( ASW_HUD_PRINTTALKANDCONSOLE, "#asw_ingame_restart_5" );
		m_flRestartingMissionTime = m_fForceReadyTime;
	}
	else if (m_iForceReadyType > ASW_FR_NONE)
	{
		UTIL_ClientPrintAll( ASW_HUD_PRINTTALKANDCONSOLE, "#asw_force_ready_5" );
	}
}

void CAlienSwarm::CheckForceReady()
{
	if (m_iForceReadyType <= ASW_FR_NONE || !ASWGameResource())
		return;

	// abort briefing force ready if no marines are selected
	if (m_iForceReadyType == ASW_FR_BRIEFING && !ASWGameResource()->AtLeastOneMarine())
	{
		SetForceReady(ASW_FR_NONE);
		return;
	}

	if ( ASWGameResource()->GetLeader() && GetGameState()!=ASW_GS_INGAME )
	{
		// check if everyone made themselves ready anyway
		if ( ASWGameResource()->AreAllOtherPlayersReady(ASWGameResource()->GetLeader()->entindex()) )
		{
			FinishForceReady();
			return;
		}
	}

	int iSecondsLeft = m_fForceReadyTime - gpGlobals->curtime;
	if (iSecondsLeft < m_iForceReadyCount)
	{
		m_iForceReadyCount = iSecondsLeft;
		if ( m_iForceReadyType == ASW_FR_INGAME_RESTART )
		{
// 			switch (iSecondsLeft)
// 			{
// 				case 4:  UTIL_ClientPrintAll( ASW_HUD_PRINTTALKANDCONSOLE, "#asw_ingame_restart_4" ); break;
// 				case 3:  UTIL_ClientPrintAll( ASW_HUD_PRINTTALKANDCONSOLE, "#asw_ingame_restart_3" ); break;
// 				case 2:  UTIL_ClientPrintAll( ASW_HUD_PRINTTALKANDCONSOLE, "#asw_ingame_restart_2" ); break;
// 				case 1:  UTIL_ClientPrintAll( ASW_HUD_PRINTTALKANDCONSOLE, "#asw_ingame_restart_1" ); break;
// 				default: break;
// 			}
		}
		else
		{
			switch (iSecondsLeft)
			{
				case 4:  UTIL_ClientPrintAll( ASW_HUD_PRINTTALKANDCONSOLE, "#asw_force_ready_4" ); break;
				case 3:  UTIL_ClientPrintAll( ASW_HUD_PRINTTALKANDCONSOLE, "#asw_force_ready_3" ); break;
				case 2:  UTIL_ClientPrintAll( ASW_HUD_PRINTTALKANDCONSOLE, "#asw_force_ready_2" ); break;
				case 1:  UTIL_ClientPrintAll( ASW_HUD_PRINTTALKANDCONSOLE, "#asw_force_ready_1" ); break;
				default: break;
			}
		}
		if (iSecondsLeft <= 0)
		{
			FinishForceReady();
			return;									
		}
	}		
}

void CAlienSwarm::FinishForceReady()
{
	CASW_Game_Resource *pGameResource = ASWGameResource();
	if (!pGameResource)
		return;

	switch (m_iForceReadyType)
	{
		case ASW_FR_BRIEFING:		// forcing a start of the mission
			{
				SetForceReady(ASW_FR_NONE);	

				// check we actually have some marines selected before starting			
				int m = pGameResource->GetMaxMarineResources();	
				bool bCanStart = false;
				for (int i=0;i<m;i++)
				{
					if (pGameResource->GetMarineResource(i))
					{
						bCanStart = true;
					}
				}
				if (!bCanStart)
					return;

				SetMaxMarines();
				
				m_bShouldStartMission = true;			
			}
			break;
		case ASW_FR_CONTINUE:	// force save and continue
			{
				SetForceReady(ASW_FR_NONE);	
				CampaignSaveAndShowCampaignMap(NULL, true);
			}
			break;
		case ASW_FR_RESTART:	// force a mission restart
		case ASW_FR_INGAME_RESTART:
			{
				SetForceReady(ASW_FR_NONE);

				if ( gpGlobals->curtime - m_fMissionStartedTime > 30.0f && GetGameState() == ASW_GS_INGAME )
				{
					MissionComplete( false );		
				}
				else
				{
					RestartMission( NULL, true );
				}
			}
			break;
		case ASW_FR_CAMPAIGN_MAP:
			{
				SetForceReady(ASW_FR_NONE);	
				// launch campaign map without saving
				// make sure all players are marked as not ready
				for (int i=0;i<ASW_MAX_READY_PLAYERS;i++)
				{
					pGameResource->m_bPlayerReady.Set(i, rd_ready_mark_override.GetBool());
				}				
				SetGameState(ASW_GS_CAMPAIGNMAP);
				if ( GetCampaignSave() )
					GetCampaignSave()->SelectDefaultNextCampaignMission();
			}
			break;

		default: break;
	};	
}

void CAlienSwarm::CheckDeathmatchFinish()
{
	if (m_fDeathmatchFinishTime == 0.0f)
		return;

	int iSecondsLeft = m_fDeathmatchFinishTime - gpGlobals->curtime;
	if (iSecondsLeft < m_iDeathmatchFinishCount && iSecondsLeft >= 0)
	{
		m_iDeathmatchFinishCount = iSecondsLeft;

		char seconds_left_str[64] = { 0 };
		itoa(iSecondsLeft, seconds_left_str, 10);

		UTIL_ClientPrintAll( ASW_HUD_PRINTTALKANDCONSOLE, "#rd_round_ends_in", seconds_left_str );
	}

	if (gpGlobals->curtime >= m_fDeathmatchFinishTime)
	{
		m_fDeathmatchFinishTime = 0.0f;
		V_memset( m_szDeathmatchWinnerName.GetForModify(), 0, sizeof( m_szDeathmatchWinnerName ) );
		MissionComplete( true );
	}
}

void CAlienSwarm::OnSVCheatsChanged()
{
	if ( !sv_cheats )
	{
		sv_cheats = cvar->FindVar( "sv_cheats" );
	}

	if ( sv_cheats && sv_cheats->GetBool() )
	{
		m_bCheated = true;
	}
}
//ConCommand asw_notify_ch( "asw_notify_ch", StartedCheating_f, "Internal use", 0 );

int CAlienSwarm::GetSpeedrunTime( void )
{
	Assert( g_pSwarmProxy );

	return g_pSwarmProxy->m_iSpeedrunTime;
}

int CAlienSwarm::GetJumpJetType(void)
{
	Assert( g_pSwarmProxy );

	return g_pSwarmProxy->m_iJumpJetType;
}

void CAlienSwarm::BroadcastSound( const char *sound )
{
	CBroadcastRecipientFilter filter;
	filter.MakeReliable();

	UserMessageBegin ( filter, "BroadcastAudio" );
		WRITE_STRING( sound );
	MessageEnd();
}

void CAlienSwarm::OnPlayerFullyJoined( CASW_Player *pPlayer )
{
	if ( !pPlayer )
	{
		return;
	}

	// Set briefing start time
	m_fBriefingStartedTime = gpGlobals->curtime;

	if ( rd_auto_kick_high_ping_player.GetInt() != 0 )
	{
		int ping, packetloss;
		UTIL_GetPlayerConnectionInfo( pPlayer->entindex(), ping, packetloss );
		if ( ping > rd_auto_kick_high_ping_player.GetInt() )
		{
			engine->ServerCommand( CFmtStr( "kickid %s 'Your ping is too high for this server, find a server closer to you'\n", pPlayer->GetASWNetworkID() ) );
		}
	}

	bool bDifficultyRestricted = false;
	const unsigned int iBrutal = 5;
	
	// if brutal and low level
	if ( GetSkillLevel() >= iBrutal && rd_auto_kick_low_level_player.GetBool() ) bDifficultyRestricted = true;

	// if brutal or challenge
	if ( (GetSkillLevel() >= iBrutal || V_strcmp(rd_challenge.GetString(), "0") != 0 ) && rd_auto_kick_low_level_player_if_brutal_or_challenge.GetBool() ) bDifficultyRestricted = true;

	// if brutal or asbi/asb2
	if ( (GetSkillLevel() >= iBrutal || V_strstr(rd_challenge.GetString(), "asbi") != NULL || V_strstr(rd_challenge.GetString(), "asb2") != NULL ) && rd_auto_kick_low_level_player_if_brutal_or_asbi.GetBool() ) bDifficultyRestricted = true;

	if ( bDifficultyRestricted && engine->IsDedicatedServer() )
	{
		// players below level 30 are considered new
		if ( !UTIL_ASW_CommanderLevelAtLeast( pPlayer, 30 ) )
		{
			engine->ServerCommand( CFmtStr( "kickid %s 'This server restricts this difficulty level to players of level 30 or above'\n", pPlayer->GetASWNetworkID() ) );
		}
	}

	bool bHadMarinesLastRound = false;
	bool bWasSpectatorLastRound = false;

	if ( GetCampaignSave() )
	{
		char buffer[256];
		Q_snprintf(buffer, sizeof(buffer), "%s%s", pPlayer->GetPlayerName(), pPlayer->GetASWNetworkID());
		for ( int i = 0; i < ASW_NUM_MARINE_PROFILES; i++ )
		{
			if ( !Q_strcmp( buffer, STRING( GetCampaignSave()->m_LastCommanders[i] ) ) )
			{
				bHadMarinesLastRound = true;
				break;
			}
		}

		// BenLubar: Mark connected players as ready if they were spectating in the previous mission.
		if ( !bHadMarinesLastRound )
		{
			const char *pszNetworkID = pPlayer->GetASWNetworkID();
			FOR_EACH_VEC( GetCampaignSave()->m_PlayerIDs, i )
			{
				if ( !V_strcmp( STRING( GetCampaignSave()->m_PlayerIDs[i] ), pszNetworkID ) )
				{
					// We were spectating last time, so let's spectate again.
					bWasSpectatorLastRound = true;
					break;
				}
			}
		}
	}

	int index = pPlayer->entindex() - 1;
	// do not mark leader ready, it makes its crown disappear
	if ( ( rd_ready_mark_override.GetBool() || bWasSpectatorLastRound ) && index >= 0 && index < ASW_MAX_READY_PLAYERS && ASWGameResource()->GetLeader() != pPlayer )
	{
		ASWGameResource()->m_bPlayerReady.Set( index, true );
	}

	if ( ASWDeathmatchMode() )
	{
		ASWDeathmatchMode()->OnPlayerFullyJoined( pPlayer );
	}
}

void CAlienSwarm::DropPowerup( CBaseEntity *pSource, const CTakeDamageInfo &info, const char *pszSourceClass )
{
	if ( !asw_drop_powerups.GetBool() || !pSource )
		return;

	if ( !ASWHoldoutMode() )
		return;

	float flChance = 0.0f;
	if ( m_fLastPowerupDropTime == 0 )
	{
		flChance = 0.05f;
		m_fLastPowerupDropTime = gpGlobals->curtime;
	}

	if ( (m_fLastPowerupDropTime + 4) < gpGlobals->curtime )
	{
		flChance = MIN( (gpGlobals->curtime - m_fLastPowerupDropTime)/100.0f, 0.5f );
	}

	//Msg( "Powerup chance = %f\n", flChance );

	if ( RandomFloat( 0.0f, 1.0f ) > flChance )
		return;

	m_fLastPowerupDropTime = gpGlobals->curtime;

	int nPowerupType = RandomInt( 0, NUM_POWERUP_TYPES-1 );

	const char *m_szPowerupClassname;
	switch ( nPowerupType )
	{
	case POWERUP_TYPE_FREEZE_BULLETS:
		m_szPowerupClassname = "asw_powerup_freeze_bullets"; break;
	case POWERUP_TYPE_FIRE_BULLETS:
		m_szPowerupClassname = "asw_powerup_fire_bullets"; break;
	case POWERUP_TYPE_ELECTRIC_BULLETS:
		m_szPowerupClassname = "asw_powerup_electric_bullets"; break;
	//case POWERUP_TYPE_CHEMICAL_BULLETS:
	//	m_szPowerupClassname = "asw_powerup_chemical_bullets"; break;
	//case POWERUP_TYPE_EXPLOSIVE_BULLETS:
	//	m_szPowerupClassname = "asw_powerup_explosive_bullets"; break;
	case POWERUP_TYPE_INCREASED_SPEED:
		m_szPowerupClassname = "asw_powerup_increased_speed"; break;
	default:
		return; break;
	}

	CASW_Powerup *pPowerup = (CASW_Powerup *)CreateEntityByName( m_szPowerupClassname );		
	UTIL_SetOrigin( pPowerup, pSource->WorldSpaceCenter() );
	pPowerup->Spawn();

	Vector vel = -info.GetDamageForce();
	vel.NormalizeInPlace();
	vel *= RandomFloat( 10, 20 );
	vel += RandomVector( -10, 10 );
	vel.z = RandomFloat( 40, 60 );
	pPowerup->SetAbsVelocity( vel );
}

void CAlienSwarm::CheckTechFailure()
{	
	if ( m_flTechFailureRestartTime > 0 && gpGlobals->curtime >= m_flTechFailureRestartTime )
	{
		CASW_Game_Resource *pGameResource = ASWGameResource();
		if ( !pGameResource )
			return;

		// count number of live techs
		bool bTech = false;
		for (int i=0;i<pGameResource->GetMaxMarineResources();i++)
		{
			CASW_Marine_Resource *pMR = pGameResource->GetMarineResource(i);
			if (pMR && pMR->GetHealthPercent() > 0 && pMR->GetProfile() && pMR->GetProfile()->CanHack())
			{
				bTech = true;
				break;
			}
		}
		if ( !bTech && pGameResource->CountAllAliveMarines() > 0 )
		{
			// tell all clients that it's time to restart
			CReliableBroadcastRecipientFilter users;
			users.MakeReliable();
			UserMessageBegin( users, "ASWTechFailure" );
			MessageEnd();

			SetForceReady( ASW_FR_INGAME_RESTART );
		}
	}
}
#endif  // !CLIENT_DLL

void CAlienSwarm::RefreshSkillData ( bool forceUpdate )
{
#ifndef CLIENT_DLL
	if ( !forceUpdate )
	{
		if ( GlobalEntity_IsInTable( "skill.cfg" ) )
			return;
	}
	GlobalEntity_Add( "skill.cfg", STRING(gpGlobals->mapname), GLOBAL_ON );
	char	szExec[256];

	ConVar const *skill = cvar->FindVar( "asw_skill" );

	SetSkillLevel( skill ? skill->GetInt() : 1 );

	// HL2 current only uses one skill config file that represents MEDIUM skill level and
	// synthesizes EASY and HARD. (sjb)
	Q_snprintf( szExec,sizeof(szExec), "exec skill_manifest.cfg\n" );

	engine->ServerCommand( szExec );
	engine->ServerExecute();
#endif // not CLIENT_DLL
}

bool CAlienSwarm::IsMultiplayer()
{
	// always true - controls behavior of achievement manager, etc.
	return true;
}

bool CAlienSwarm::IsOfflineGame()
{
	return ( ASWGameResource() && ASWGameResource()->IsOfflineGame() );
}

bool CAlienSwarm::IsAnniversaryWeek()
{
	ISteamUtils *pUtils = SteamUtils();
#ifdef GAME_DLL
	if ( !pUtils )
		pUtils = SteamGameServerUtils();
#endif
	Assert( pUtils );
	if ( !pUtils )
	{
		return false;
	}

	// Require sv_cheats as well so this convar can't easily be used to make a double experience "challenge".
	if ( rd_anniversary_week_debug.GetInt() != -1 && ConVarRef( "sv_cheats" ).GetBool() )
	{
		return rd_anniversary_week_debug.GetBool();
	}

	// Previously, this was in local time; however, we need it to be the same on the client and the server.
	// Therefore, we are moving it to GMT and extending the week by 1 day to compensate.
	// The anniversary week now takes place from the 20th to the 27th.
	tm curtime;
	Plat_gmtime( pUtils->GetServerRealTime(), &curtime );
	return ( curtime.tm_mday >= 20 && curtime.tm_mday <= 27 ) && curtime.tm_mon == 3;
}

int CAlienSwarm::CampaignMissionsLeft()
{
	if ( !IsCampaignGame() )
		return 2;

	const RD_Campaign_t *pCampaign = GetCampaignInfo();
	if ( !pCampaign )
		return 2;

	CASW_Campaign_Save *pSave = GetCampaignSave();
	if ( !pSave )
		return 2;

	int iNumMissions = pCampaign->Missions.Count() - 1;	// minus one, for the atmospheric entry which isn't a completable mission

	return ( iNumMissions - pSave->m_iNumMissionsComplete );
}

bool CAlienSwarm::MarineCanPickupAmmo(CASW_Marine *pMarine, CASW_Ammo *pAmmo)
{
	if (!pMarine || !pAmmo)
		return false;

	int iGuns = pMarine->GetNumberOfWeaponsUsingAmmo(pAmmo->m_iAmmoIndex);
	if (iGuns <= 0)
	{
		// just show the name of the ammo, without the 'take'
#ifdef CLIENT_DLL
		Q_snprintf(m_szPickupDenial, sizeof(m_szPickupDenial), "%s", pAmmo->m_szNoGunText);
#endif
		return false;
	}

	if ( pAmmo->m_iAmmoIndex < 0 || pAmmo->m_iAmmoIndex >= MAX_AMMO_SLOTS )
		return false;

	int iMax = GetAmmoDef()->MaxCarry(pAmmo->m_iAmmoIndex, pMarine) * iGuns;
	int iAdd = MIN( 1, iMax - pMarine->GetAmmoCount(pAmmo->m_iAmmoIndex) );
	if ( iAdd < 1 )
	{
#ifdef CLIENT_DLL
		Q_snprintf(m_szPickupDenial, sizeof(m_szPickupDenial), "%s", pAmmo->m_szAmmoFullText);
#endif
		return false;
	}

	return true;
}

bool CAlienSwarm::MarineCanPickupPowerup(CASW_Marine *pMarine, CASW_Powerup *pPowerup)
{
	if ( !pMarine )
		return false;

	CASW_Weapon* pWeapon = pMarine->GetActiveASWWeapon();
	if ( !pWeapon || !IsBulletBasedWeaponClass( pWeapon->Classify() ) )
	{
#ifdef CLIENT_DLL
		Q_snprintf(m_szPickupDenial, sizeof(m_szPickupDenial), "%s", "#asw_powerup_requires_bulletammo");
#endif
		return false;
	}
	
	return true;
}

bool CAlienSwarm::MarineHasRoomInAmmoBag(CASW_Marine *pMarine, int iAmmoIndex)
{
	if (!pMarine)
		return false;

	if ( iAmmoIndex < 0 || iAmmoIndex >= MAX_AMMO_SLOTS )
		return false;

	CASW_Weapon_Ammo_Bag *pBag = dynamic_cast<CASW_Weapon_Ammo_Bag*>(pMarine->GetWeapon(0));
	if (pBag)
	{
		if (pBag->HasRoomForAmmo(iAmmoIndex))
			return true;
	}

	pBag = dynamic_cast<CASW_Weapon_Ammo_Bag*>(pMarine->GetWeapon(1));
	if (pBag)
	{
		if (pBag->HasRoomForAmmo(iAmmoIndex))
			return true;
	}

	return false;
}

#ifdef GAME_DLL
void CheatsChangeCallback( IConVar *pConVar, const char *pOldString, float flOldValue )
{
	if ( ASWGameRules() )
	{
		ASWGameRules()->OnSVCheatsChanged();
	}
}
#endif

#ifdef GAME_DLL
static void CreateCake( const char *mapname )
{
	Vector origin( 0, 0, 0 );
	if ( FStrEq( mapname, "asi-jac1-landingbay_01" ) )
	{
		origin = Vector( -8444, -468, 852 );
	}
	else if ( FStrEq( mapname, "asi-jac1-landingbay_02" ) )
	{
		origin = Vector( -5087, 4764, 816 );
	}
	else if ( FStrEq( mapname, "asi-jac1-landingbay_pract" ) )
	{
		origin = Vector( -4706, 2265, 753 );
	}
	else if ( FStrEq( mapname, "asi-jac2-deima" ) )
	{
		origin = Vector( 853, 5452, -52 );
	}
	else if ( FStrEq( mapname, "asi-jac3-rydberg" ) )
	{
		origin = Vector( -2786, -1282, 660 );
	}
	else if ( FStrEq( mapname, "asi-jac4-residential" ) )
	{
		origin = Vector( -1404, -3520, -8 );
	}
	else if ( FStrEq( mapname, "asi-jac6-sewerjunction" ) )
	{
		origin = Vector( 208, 1416, -548 );
	}
	else if ( FStrEq( mapname, "asi-jac7-timorstation" ) )
	{
		origin = Vector( -151, -5440, -97 );
	}
	else if ( FStrEq( mapname, "dm_deima" ) )
	{
		origin = Vector( -528, 1884, 12 );
	}
	else if ( FStrEq( mapname, "dm_desert" ) )
	{
		origin = Vector( 548, 4159, 76 );
	}
	else if ( FStrEq( mapname, "dm_residential" ) )
	{
		origin = Vector( -2036, -7628, -28 );
	}
	else if ( FStrEq( mapname, "dm_testlab" ) )
	{
		origin = Vector( -3746, -1314, -52 );
	}
	else if ( FStrEq( mapname, "example_map_1" ) )
	{
		origin = Vector( -128, -832, 12 );
	}
	else if ( FStrEq( mapname, "example_map_2" ) )
	{
		origin = Vector( -240, -240, -52 );
	}
	else if ( FStrEq( mapname, "example_map_3" ) )
	{
		origin = Vector( -128, 3136, 12 );
	}
	else if ( FStrEq( mapname, "rd-area9800lz" ) )
	{
		origin = Vector( -1829, 2136, 12 );
	}
	else if ( FStrEq( mapname, "rd-area9800pp1" ) )
	{
		origin = Vector( -2428, -988, -188 );
	}
	else if ( FStrEq( mapname, "rd-area9800pp2" ) )
	{
		origin = Vector( -1129, 233, 184 );
	}
	else if ( FStrEq( mapname, "rd-area9800wl" ) )
	{
		origin = Vector( 2006, 280, -556 );
	}
	else if ( FStrEq( mapname, "rd-bonus_mission1" ) )
	{
		origin = Vector( 506, -166, 6 );
	}
	else if ( FStrEq( mapname, "rd-bonus_mission2" ) )
	{
		origin = Vector( -7997, -10414, -51 );
	}
	else if ( FStrEq( mapname, "rd-bonus_mission3" ) )
	{
		origin = Vector( 8614, 2973, -847 );
	}
	else if ( FStrEq( mapname, "rd-bonus_mission4" ) )
	{
		origin = Vector( -4976, 2217, 753 );
	}
	else if ( FStrEq( mapname, "rd-bonus_mission5" ) )
	{
		origin = Vector( 951, -2120, 304 );
	}
	else if ( FStrEq( mapname, "rd-bonus_mission6" ) )
	{
		origin = Vector( -3346, -357, -327 );
	}
	else if ( FStrEq( mapname, "rd-bonus_mission7" ) )
	{
		origin = Vector( 236, 1785, 14 );
	}
	else if ( FStrEq( mapname, "rd-lan1_bridge" ) )
	{
		origin = Vector( 344, -4035, 28 );
	}
	else if ( FStrEq( mapname, "rd-lan2_sewer" ) )
	{
		origin = Vector( -4258, -2022, -18 );
	}
	else if ( FStrEq( mapname, "rd-lan3_maintenance" ) )
	{
		origin = Vector( -4758, 478, -51 );
	}
	else if ( FStrEq( mapname, "rd-lan4_vent" ) )
	{
		origin = Vector( -1699, -726, -73 );
	}
	else if ( FStrEq( mapname, "rd-lan5_complex" ) )
	{
		origin = Vector( -560, -4214, -228 );
	}
	else if ( FStrEq( mapname, "rd-ocs1storagefacility" ) )
	{
		origin = Vector( 985, 472, -356 );
	}
	else if ( FStrEq( mapname, "rd-ocs2landingbay7" ) )
	{
		origin = Vector( -928, -1592, 316 );
	}
	else if ( FStrEq( mapname, "rd-ocs3uscmedusa" ) )
	{
		origin = Vector( -610, -2179, 60 );
	}
	else if ( FStrEq( mapname, "rd-par1unexpected_encounter" ) )
	{
		origin = Vector( 1101, 4036, 163 );
	}
	else if ( FStrEq( mapname, "rd-par2hostile_places" ) )
	{
		origin = Vector( 1372, -1135, 204 );
	}
	else if ( FStrEq( mapname, "rd-par3close_contact" ) )
	{
		origin = Vector( -2706, 4050, 533 );
	}
	else if ( FStrEq( mapname, "rd-par4high_tension" ) )
	{
		origin = Vector( 1886, -1437, -338 );
	}
	else if ( FStrEq( mapname, "rd-par5crucial_point" ) )
	{
		origin = Vector( -7471, -3522, -145 );
	}
	else if ( FStrEq( mapname, "rd-res1forestentrance" ) )
	{
		origin = Vector( -1187, 2342, 104 );
	}
	else if ( FStrEq( mapname, "rd-res2research7" ) )
	{
		origin = Vector( 944, 1498, 76 );
	}
	else if ( FStrEq( mapname, "rd-res3miningcamp" ) )
	{
		origin = Vector( -3246, 1018, 14 );
	}
	else if ( FStrEq( mapname, "rd-res4mines" ) )
	{
		origin = Vector( -10252, 1372, -2244 );
	}
	else if ( FStrEq( mapname, "rd-tft1desertoutpost" ) )
	{
		origin = Vector( -1976, 2090, -119 );
	}
	else if ( FStrEq( mapname, "rd-tft2abandonedmaintenance" ) )
	{
		origin = Vector( -1628, -184, -884 );
	}
	else if ( FStrEq( mapname, "rd-tft3spaceport" ) )
	{
		origin = Vector( -1028, -1090, 1084 );
	}
	else if ( FStrEq( mapname, "rd-til1midnightport" ) )
	{
		origin = Vector( -4777, 3085, 76 );
	}
	else if ( FStrEq( mapname, "rd-til2roadtodawn" ) )
	{
		origin = Vector( 29, 1849, 108 );
	}
	else if ( FStrEq( mapname, "rd-til3arcticinfiltration" ) )
	{
		origin = Vector( -1156, 4177, 437 );
	}
	else if ( FStrEq( mapname, "rd-til4area9800" ) )
	{
		origin = Vector( 1161, -257, -492 );
	}
	else if ( FStrEq( mapname, "rd-til5coldcatwalks" ) )
	{
		origin = Vector( -1789, -4679, 12 );
	}
	else if ( FStrEq( mapname, "rd-til6yanaurusmine" ) )
	{
		origin = Vector( 5295, 8550, -396 );
	}
	else if ( FStrEq( mapname, "rd-til7factory" ) )
	{
		origin = Vector( -752, 1472, 12 );
	}
	else if ( FStrEq( mapname, "rd-til8comcenter" ) )
	{
		origin = Vector( 1907, 5053, -238 );
	}
	else if ( FStrEq( mapname, "rd-til9syntekhospital" ) )
	{
		origin = Vector( -2753, 1659, 4604 );
	}
	else if ( FStrEq( mapname, "rd-nh01_logisticsarea" ) )
	{
		origin = Vector( 922, 1444, 192 );
	}
	else if ( FStrEq( mapname, "rd-nh02_platformxvii" ) )
	{
		origin = Vector( 560, -1536, 205 );
	}
	else if ( FStrEq( mapname, "rd-nh03_groundworklabs" ) )
	{
		origin = Vector( -5622, 6546, -1106 );
	}
	else if ( FStrEq( mapname, "rd-bio1operationx5" ) )
	{
		origin = Vector( -3052, 1708, -72 );
	}
	else if ( FStrEq( mapname, "rd-bio2invisiblethreat" ) )
	{
		origin = Vector( 1080, 1528, 172 );
	}
	else if ( FStrEq( mapname, "rd-bio3biogenlabs" ) )
	{
		origin = Vector( 312, 2186, 156 );
	}
#ifdef RD_6A_CAMPAIGNS_ACCIDENT32
	else if ( FStrEq( mapname, "rd-acc1_infodep" ) )
	{
		origin = Vector( 3232, 4240, 300 );
	}
	else if ( FStrEq( mapname, "rd-acc2_powerhood" ) )
	{
		origin = Vector( 512, 725, 296 );
	}
	else if ( FStrEq( mapname, "rd-acc3_rescenter" ) )
	{
		origin = Vector( 2528, 3424, -92 );
	}
	else if ( FStrEq( mapname, "rd-acc4_confacility" ) )
	{
		origin = Vector( 352, -2640, 492 );
	}
	else if ( FStrEq( mapname, "rd-acc5_j5connector" ) )
	{
		origin = Vector( 1448, 1712, 233 );
	}
	else if ( FStrEq( mapname, "rd-acc6_labruins" ) )
	{
		origin = Vector( 704, 824, -500 );
	}
	else if ( FStrEq( mapname, "rd-acc_complex" ) )
	{
		origin = Vector( -3925, -443, -20 );
	}
#endif
#ifdef RD_6A_CAMPAIGNS_ADANAXIS
	else if ( FStrEq( mapname, "rd-ada_sector_a9" ) )
	{
		origin = Vector( 2236, -1496, 1520 );
	}
	else if ( FStrEq( mapname, "rd-ada_nexus_subnode" ) )
	{
		origin = Vector( -1732, 1848, 381 );
	}
	else if ( FStrEq( mapname, "rd-ada_neon_carnage" ) )
	{
		origin = Vector( 3730, -3524, 84 );
	}
	else if ( FStrEq( mapname, "rd-ada_fuel_junction" ) )
	{
		origin = Vector( -652, 3390, 348 );
	}
	else if ( FStrEq( mapname, "rd-ada_dark_path" ) )
	{
		origin = Vector( -7592, -5936, 524 );
	}
	else if ( FStrEq( mapname, "rd-ada_forbidden_outpost" ) )
	{
		origin = Vector( 2300, 3796, -568 );
	}
	else if ( FStrEq( mapname, "rd-ada_new_beginning" ) )
	{
		origin = Vector( -480, -734, 1132 );
	}
	else if ( FStrEq( mapname, "rd-ada_anomaly" ) )
	{
		origin = Vector( -3808, -1632, 452 );
	}
#endif

	if ( origin.IsZeroFast() )
		return;

	// coordinates above are center; we need bottom
	origin += Vector( 0, 0, -12 );

	CBaseEntity *pCake = CreateEntityByName( "prop_dynamic" );
	pCake->SetModelName( AllocPooledString( "models/props/cake/cake.mdl" ) );
	pCake->Precache();
	pCake->SetAbsOrigin( origin );
	pCake->Spawn();
	// UTIL_DropToFloor( pCake, MASK_SOLID );
	pCake->SetMoveType( MOVETYPE_NONE );

	CSprite *pCakeSprite = assert_cast< CSprite * >( CreateEntityByName( "env_sprite" ) );
	pCakeSprite->SetModelName( AllocPooledString( "materials/sprites/light_glow03.vmt") );
	pCakeSprite->Precache();
	pCakeSprite->SetGlowProxySize( 2.0f );
	pCakeSprite->SetScale( 0.15f );
	pCakeSprite->SetColor( 220, 205, 120 );
	pCakeSprite->SetRenderAlpha( 190.0f );
	pCakeSprite->SetRenderMode( kRenderWorldGlow );
	pCakeSprite->m_flSpriteFramerate = 10.0f;
	pCakeSprite->SetAbsOrigin( origin + Vector( 0, 0, 24 ) );
	pCakeSprite->Spawn();
}
#endif

void CAlienSwarm::LevelInitPostEntity()
{
	// check if we're the intro/outro map
	char mapName[255];
#ifdef CLIENT_DLL
	Q_FileBase( engine->GetLevelName(), mapName, sizeof(mapName) );	
#else
	Q_strcpy( mapName, STRING(gpGlobals->mapname) );
#endif

	m_bIsTutorial = ( !V_strnicmp( mapName, "tutorial", 8 ) );
	m_bIsLobby = ( !V_stricmp( mapName, "lobby" ) );

	if ( ASWHoldoutMode() )
	{
		ASWHoldoutMode()->LevelInitPostEntity();
	}

#ifdef GAME_DLL
	if (ASWDeathmatchMode())
	{
		ASWDeathmatchMode()->LevelInitPostEntity();
	}
#endif

	if ( missionchooser && missionchooser->RandomMissions() )
	{
		missionchooser->RandomMissions()->LevelInitPostEntity( mapName );
	}

#ifndef CLIENT_DLL
	engine->ServerCommand("exec newmapsettings\n");

	m_bPlayedBlipSpeech = false;
	m_bQuickStart = false;
	m_flRestartingMissionTime = 0;

	KeyValues *pLaunchOptions = engine->GetLaunchOptions();
	if ( pLaunchOptions )
	{
		for ( KeyValues *pKey = pLaunchOptions->GetFirstSubKey(); pKey; pKey = pKey->GetNextKey() )
		{
			if ( !Q_stricmp( pKey->GetString(), "quickstart" ) )
			{
				m_bQuickStart = true;
			}
		}
	}

	// create the game resource
	if (ASWGameResource()!=NULL)
	{
		Msg("ASW: ASWGameResource already exists, removing it\n");
		UTIL_Remove( ASWGameResource() );
	}
	
	CreateEntityByName( "asw_game_resource" );
	
	if (ASWGameResource() == NULL)
	{
		Msg("ASW: Error! Failed to create ASWGameResource\n");
		return;
	}

	if ( !gEntList.FindEntityByClassname( NULL, "env_tonemap_controller" ) )
	{
		DevWarning( "No env_tonemap_controller found in level. Creating one and setting bloom scale to a safe value.\n" );
		CBaseEntity *pTonemap = CreateEntityByName( "env_tonemap_controller" );
		if ( pTonemap )
		{
			DispatchSpawn( pTonemap );
			variant_t scale;
			scale.SetFloat( 0.25f );
			pTonemap->AcceptInput( "SetBloomScale", pTonemap, pTonemap, scale, 0 );

			TheTonemapSystem()->LevelInitPostEntity();
		}
	}

	// find the objective entities
	ASWGameResource()->FindObjectives();

	// setup the savegame entity, if we're in a campaign game
	if ( ASWGameResource()->GetCampaignSaveName()[0] )
	{
		if ( !ASWGameResource()->CreateCampaignSave() )
		{
			Msg( "ERROR: Failed to create campaign save object\n" );
		}
		else
		{
			if ( !ASWGameResource()->GetCampaignSave()->LoadGameFromFile( ASWGameResource()->GetCampaignSaveName() ) )
			{
				Msg( "ERROR: Failed to load campaign save game: %s\n", ASWGameResource()->GetCampaignSaveName() );
			}
			else
			{
				m_bChallengeActiveThisCampaign = IsCampaignGame() && ASWGameResource()->GetCampaignSave()->m_bChallengeEverActive;

				// update our difficulty level with the relevant campaign modifier
				OnSkillLevelChanged( m_iSkillLevel );
				ReserveMarines();
			}
		}
	}
	// todo: if we fail to load the campaign save file above, then gracefully fall into single mission mode?

	// make sure we're on easy mode for the tutorial
	if ( IsTutorialMap() )
	{
		asw_skill.SetValue( 1 );
		m_iSkillLevel = asw_skill.GetInt();
		OnSkillLevelChanged( m_iSkillLevel );
	}

	SetGameState(ASW_GS_BRIEFING);
	mm_swarm_state.SetValue( "briefing" );
	SetMaxMarines();

	// create the burning system
	CASW_Burning *pFire = assert_cast< CASW_Burning * >( CreateEntityByName( "asw_burning" ) );
	if ( pFire )
		pFire->Spawn();

	if ( !sv_cheats )
	{
		sv_cheats = cvar->FindVar( "sv_cheats" );
	}

	if ( sv_cheats )
	{
		m_bCheated = sv_cheats->GetBool();
		if ( !scriptmanager )
			m_bCheated = true;
		static bool s_bInstalledCheatsChangeCallback = false;
		if ( !s_bInstalledCheatsChangeCallback )
		{
			sv_cheats->InstallChangeCallback( CheatsChangeCallback );
			s_bInstalledCheatsChangeCallback = true;
		}
	}

	m_hBriefingCamera = gEntList.FindEntityByClassname( NULL, "rd_briefing_camera" );
	m_bHadBriefingCamera = !!m_hBriefingCamera;

	if ( IsAnniversaryWeek() )
	{
		CreateCake( mapName );
	}

	if ( IsLobbyMap() )
	{
		OnServerHibernating();
	}
#endif  // !CLIENT_DLL
}

void CAlienSwarm::LevelShutdownPostEntity()
{
#ifdef CLIENT_DLL
	RevertSavedConvars();
#endif
}

int CAlienSwarm::TotalInfestDamage()
{
	switch ( m_iSkillLevel )
	{
	case 1:
		return sk_asw_parasite_infest_dmg_easy.GetInt();
	case 2:
		return sk_asw_parasite_infest_dmg_normal.GetInt();
	case 3:
		return sk_asw_parasite_infest_dmg_hard.GetInt();
	case 4:
		return sk_asw_parasite_infest_dmg_insane.GetInt();	// BARELY survivable with Bastille and heal beacon
	case 5:
		return sk_asw_parasite_infest_dmg_brutal.GetInt();
	}

	// ASv1 Total infest damage = 90 + difficulty * 20;
	// Infest damage rate = Total infest damage / 20;
	//
	// i.e.:
	// 120 at difficulty 1	 -   6.0 damage/sec  - 2 full skill satchel heals.  ASv1: 1 medpack/satchel
	// 160 at difficulty 3   -   8.0 damage/sec   - 2 full skill satchel heals
	// 200 at difficulty 5	 -  10.0 damage/sec   - 3 full skill satchel heals  ASv1: 2 satchel heals
	// 240 at difficulty 7   -  12.0 damage/sec  - 3+ full skill satchel heals  ASv1: 3 satchel heals, if you get there instantly
	// 260 at difficulty 8   -  13.0 damage/sec  - 3+ full skill satchel heals  Can't outheal
	// 300 at difficulty 10  -  15.0 damage/sec  - 4- full skill satchel heals  Can't outheal  Can survive if healed by a level 3 xenowound marine if they're quick.
	//                                             (double number of heals needed for a 0 skill medic)
	//
	// Med satchel heals 4 health every 0.33 seconds, which is 12 hps/sec (same as ASv1)	
	// Med satchel heals 25 hps at skill 0, 65 hps at skill 5.
	// Med satchel has 4 to 9 charges.
	//  Giving a total hps healed for 1 satchel = 100 to 675
	//  Medkit heals 50 hps for normal marines and 50 to 80 for medics)
	// In ASv1 satchel healed 25 to 85.   Charges were: 6, 7 (at 3 star) or 8 (at 4 star+)
	// Giving a total hps healed for 1 satchel = 150 to 680
	return sk_asw_parasite_infest_dmg_normal.GetInt();
}

bool CAlienSwarm::ShouldPlayStimMusic()
{
	return (gpGlobals->curtime > m_fPreventStimMusicTime.Get());
}

const char* CAlienSwarm::GetFailAdviceText( void )
{
	switch ( ASWGameRules()->m_nFailAdvice )
	{
	case ASW_FAIL_ADVICE_LOW_AMMO:				return "#asw_fail_advice_low_ammo";
	case ASW_FAIL_ADVICE_INFESTED_LOTS:			return "#asw_fail_advice_infested_lots";
	case ASW_FAIL_ADVICE_INFESTED:				return "#asw_fail_advice_infested";
	case ASW_FAIL_ADVICE_SWARMED:				return "#asw_fail_advice_swarmed";
	case ASW_FAIL_ADVICE_FRIENDLY_FIRE:			return "#asw_fail_advice_friendly_fire";
	case ASW_FAIL_ADVICE_HACKER_DAMAGED:		return "#asw_fail_advice_hacker_damaged";
	case ASW_FAIL_ADVICE_WASTED_HEALS:			return "#asw_fail_advice_wasted_heals";
	case ASW_FAIL_ADVICE_IGNORED_HEALING:		return "#asw_fail_advice_ignored_healing";
	case ASW_FAIL_ADVICE_IGNORED_ADRENALINE:	return "#asw_fail_advice_ignored_adrenaline";
	case ASW_FAIL_ADVICE_IGNORED_SECONDARY:		return "#asw_fail_advice_ignored_secondary";
	case ASW_FAIL_ADVICE_IGNORED_WELDER:		return "#asw_fail_advice_ignored_welder";
	case ASW_FAIL_ADVICE_IGNORED_LIGHTING:		return "#asw_fail_advice_ignored_lighting";	// TODO
	case ASW_FAIL_ADVICE_SLOW_PROGRESSION:		return "#asw_fail_advice_slow_progression";
	case ASW_FAIL_ADVICE_SHIELD_BUG:			return "#asw_fail_advice_shield_bug";
	case ASW_FAIL_ADVICE_DIED_ALONE:			return "#asw_fail_advice_died_alone";
	case ASW_FAIL_ADVICE_NO_MEDICS:				return "#asw_fail_advice_no_medics";

	case ASW_FAIL_ADVICE_DEFAULT:
	default:
		switch ( RandomInt( 0, 2 ) )
		{
		case 0: return "#asw_fail_advice_teamwork";
		case 1: return "#asw_fail_advice_loadout";
		case 2: return "#asw_fail_advice_routes";
		}
	}

	return "#asw_fail_advice_teamwork";
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : iDmgType - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CAlienSwarm::Damage_IsTimeBased( int iDmgType )
{
	// Damage types that are time-based.
	return ( ( iDmgType & ( DMG_PARALYZE | DMG_NERVEGAS | DMG_RADIATION | DMG_DROWNRECOVER | DMG_ACID | DMG_SLOWBURN ) ) != 0 );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : iDmgType - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CAlienSwarm::Damage_ShouldGibCorpse( int iDmgType )
{
	// Damage types that gib the corpse.
	return ( ( iDmgType & ( DMG_CRUSH | DMG_FALL | DMG_BLAST | DMG_SONIC | DMG_CLUB | DMG_INFEST ) ) != 0 );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : iDmgType - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CAlienSwarm::Damage_NoPhysicsForce( int iDmgType )
{
	// Damage types that don't have to supply a physics force & position.
	int iTimeBasedDamage = Damage_GetTimeBased();
	return ( ( iDmgType & ( DMG_FALL | DMG_BURN | DMG_PLASMA | DMG_DROWN | iTimeBasedDamage | DMG_CRUSH | DMG_PHYSGUN | DMG_PREVENT_PHYSICS_FORCE | DMG_INFEST ) ) != 0 );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : iDmgType - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CAlienSwarm::Damage_ShouldNotBleed( int iDmgType )
{
	// Damage types that don't make the player bleed.
	return ( ( iDmgType & ( DMG_ACID ) ) != 0 );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CAlienSwarm::Damage_GetTimeBased( void )
{
	int iDamage = ( DMG_PARALYZE | DMG_NERVEGAS | DMG_RADIATION | DMG_DROWNRECOVER | DMG_ACID | DMG_SLOWBURN );
	return iDamage;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int	CAlienSwarm::Damage_GetShouldGibCorpse( void )
{
	int iDamage = ( DMG_CRUSH | DMG_FALL | DMG_BLAST | DMG_SONIC | DMG_CLUB | DMG_INFEST  );
	return iDamage;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int	CAlienSwarm::Damage_GetNoPhysicsForce( void )
{
	int iTimeBasedDamage = Damage_GetTimeBased();
	int iDamage = ( DMG_FALL | DMG_BURN | DMG_PLASMA | DMG_DROWN | iTimeBasedDamage | DMG_CRUSH | DMG_PHYSGUN | DMG_PREVENT_PHYSICS_FORCE | DMG_INFEST );
	return iDamage;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int	CAlienSwarm::Damage_GetShouldNotBleed( void )
{
	int iDamage = ( DMG_ACID );
	return iDamage;
}

// movement uses this axis to decide where the marine should go from his forward/sidemove
const QAngle& CAlienSwarm::GetTopDownMovementAxis()
{
	static QAngle axis = ASW_MOVEMENT_AXIS;
	return axis;
}

bool CAlienSwarm::IsHardcoreFF()
{
	return ( asw_marine_ff_absorption.GetInt() != 1 || asw_sentry_friendly_fire_scale.GetFloat() != 0.0f );
}

bool CAlienSwarm::IsOnslaught()
{
	return ( asw_horde_override.GetBool() || asw_wanderer_override.GetBool() );
}

bool CAlienSwarm::HaveSavedConvar( const ConVarRef & ref )
{
	Assert( ref.IsValid() );

	UtlSymId_t iSymbol = m_SavedConvars.Find( ref.GetName() );
	if ( iSymbol == m_SavedConvars.InvalidIndex() )
	{
		return false;
	}

	return m_SavedConvars[ iSymbol ] != NULL_STRING;
}

#ifdef CLIENT_DLL
void __MsgFunc_SavedConvar( bf_read &msg )
{
	Assert( ASWGameRules() );
	if ( !ASWGameRules() )
	{
		return;
	}

	char szKey[2048];
	char szValue[2048];
	bool bReadKey = msg.ReadString( szKey, sizeof( szKey ) );
	Assert( bReadKey );
	bool bReadValue = msg.ReadString( szValue, sizeof( szValue ) );
	Assert( bReadValue );
	if ( bReadKey && bReadValue )
	{
		ConVarRef ref( szKey );
		Assert( ref.IsValid() );
		Assert( ref.IsFlagSet( FCVAR_REPLICATED ) );
		if ( ref.IsValid() && ref.IsFlagSet( FCVAR_REPLICATED ) )
		{
			ASWGameRules()->m_SavedConvars[szKey] = AllocPooledString( szValue );
		}
	}
}
USER_MESSAGE_REGISTER( SavedConvar );
#endif

void CAlienSwarm::SaveConvar( const ConVarRef & ref )
{
	Assert( ref.IsValid() );

	if ( HaveSavedConvar( ref ) )
	{
		// already saved, don't override.
		return;
	}

#ifdef GAME_DLL
	// BenLubar: Send saved replicated convars to the client so that it can reset them if the player disconnects during the mission.
	if ( ref.IsFlagSet( FCVAR_REPLICATED ) )
	{
		CReliableBroadcastRecipientFilter filter;
		UserMessageBegin( filter, "SavedConvar" );
		WRITE_STRING( ref.GetName() );
		WRITE_STRING( ref.GetString() );
		MessageEnd();
	}
#endif
	m_SavedConvars[ ref.GetName() ] = AllocPooledString( ref.GetString() );
}

void CAlienSwarm::RevertSingleConvar( ConVarRef & ref )
{
	Assert( ref.IsValid() );

	if ( !HaveSavedConvar( ref ) )
	{
		// don't have a saved value
		return;
	}

	string_t & saved = m_SavedConvars[ ref.GetName() ];
	ref.SetValue( STRING( saved ) );
	saved = NULL_STRING;
}

void CAlienSwarm::RevertSavedConvars()
{
	// revert saved convars
	for ( int i = 0; i < m_SavedConvars.GetNumStrings(); i++ )
	{
		const char *pszName = m_SavedConvars.String( i );
		string_t iszValue = m_SavedConvars[i];
		ConVarRef ref( pszName );
		Assert( ref.IsValid() );
		if ( iszValue != NULL_STRING )
		{
			ref.SetValue( STRING( iszValue ) );
		}
	}
	m_SavedConvars.Purge();
}

#ifdef GAME_DLL

void CAlienSwarm::EnableChallenge( const char *szChallengeName )
{
	extern ConVar rd_challenge;

	const RD_Challenge_t *pSummary = ReactiveDropChallenges::GetSummary( szChallengeName );
	if ( !pSummary || ( ASWDeathmatchMode() ? !pSummary->AllowDeathmatch : !pSummary->AllowCoop ) )
	{
		if ( pSummary )
		{
			Warning( "Challenge '%s' is not allowed in this game mode.\n", szChallengeName );
		}

		szChallengeName = "0";
	}

	bool bChanged = !!V_strcmp( rd_challenge.GetString(), szChallengeName );
	KeyValues::AutoDelete pKV( "CHALLENGE" );
	bool bEnabled = ReactiveDropChallenges::ReadData( pKV, szChallengeName );

	ResetChallengeConVars();
	if ( ASWDeathmatchMode() )
	{
		ASWDeathmatchMode()->ApplyDeathmatchConVars();

		// we can change challenge modes mid-round for deathmatch, which needs a little bit of clean-up.
		CBaseEntity *pThinker = NULL;
		while ( ( pThinker = gEntList.FindEntityByClassname( pThinker, "asw_challenge_thinker" ) ) != NULL )
		{
			UTIL_Remove( pThinker );
		}

		if ( g_pScriptVM )
		{
			g_pScriptVM->ClearValue( "g_ModeScript" );
		}
	}

	if ( bEnabled )
	{
		ApplyChallengeConVars( pKV );
		EnforceWeaponSelectionRules();
	}

	if ( !bEnabled )
	{
		szChallengeName = "0";

		if ( V_strcmp( rd_challenge.GetString(), "0" ) )
		{
			UTIL_ClientPrintAll( ASW_HUD_PRINTTALKANDCONSOLE, "#rd_challenge_disabled" );
		}
	}
	else if ( bChanged )
	{
		UTIL_ClientPrintAll( ASW_HUD_PRINTTALKANDCONSOLE, "#rd_challenge_enabled", pKV->GetString( "name", szChallengeName ) );
	}
	// if rd_max_marines changed we need to update marines limits using SetMaxMarines
	SetMaxMarines();
	EnforceMaxMarines();
	rd_challenge.SetValue( szChallengeName );
	OnSkillLevelChanged( m_iSkillLevel );

	if ( V_strcmp( rd_challenge.GetString(), "0" ) )
	{
		V_strncpy( m_szGameDescription, ReactiveDropChallenges::DisplayName( rd_challenge.GetString() ), sizeof( m_szGameDescription ) );
		if ( const wchar_t *pwszChallengeName = g_pVGuiLocalize->Find( m_szGameDescription ) )
		{
			g_pVGuiLocalize->ConvertUnicodeToANSI( pwszChallengeName, m_szGameDescription, sizeof( m_szGameDescription ) );
		}
	}
	else
	{
		V_strncpy( m_szGameDescription, "Alien Swarm: Reactive Drop", sizeof( m_szGameDescription ) );
	}
}

void CAlienSwarm::CheckChallengeConVars()
{
	// make sure none of the variables were changed
	for ( int i = 0; i < m_SavedConvars_Challenge.GetNumStrings(); i++ )
	{
		ConVarRef ref( m_SavedConvars_Challenge.String( i ) );
		const char *pszDesiredValue = STRING( m_SavedConvars_Challenge[i] );

		if ( ref.IsValid() && Q_strcmp( ref.GetString(), pszDesiredValue ) )
		{
			Warning( "Fixing challenge convar %s\n", ref.GetName() );
			ref.SetValue( pszDesiredValue );
		}
	}
}

void CAlienSwarm::ResetChallengeConVars()
{
	CUtlStringList names;
	names.EnsureCapacity( m_SavedConvars_Challenge.GetNumStrings() );
	for ( int i = 0; i < m_SavedConvars_Challenge.GetNumStrings(); i++ )
	{
		names.CopyAndAddToTail( m_SavedConvars_Challenge.String( i ) );
	}

	// purge before we reset the convars so the write protection doesn't trigger.
	m_SavedConvars_Challenge.Purge();

	FOR_EACH_VEC( names, i )
	{
		ConVarRef ref( names[i] );
		RevertSingleConvar( ref );
	}
}

void CAlienSwarm::ApplyChallengeConVars( KeyValues *pKV )
{
	KeyValues *pConVars = pKV->FindKey( "convars" );
	if ( !pConVars )
	{
		return;
	}

	FOR_EACH_VALUE( pConVars, pCV )
	{
		ConVarRef ref( pCV->GetName() );
		if ( ref.IsValid() )
		{
			SaveConvar( ref );
			ref.SetValue( pCV->GetString() );
			// use the actual value to make sure we don't run into issues with truncated values not being equal.
			m_SavedConvars_Challenge[ref.GetName()] = AllocPooledString( ref.GetString() );
		}
		else
		{
			Warning( "Invalid ConVar %s found in challenge %s\n", pCV->GetName(), pKV->GetString( "name", "unknown" ) );
		}
	}
}

class CASW_Challenge_Thinker : public CLogicalEntity
{
public:
	DECLARE_CLASS( CASW_Challenge_Thinker, CLogicalEntity );

	CASW_Challenge_Thinker()
	{
		m_iszScriptThinkFunction = AllocPooledString( "Update" );
	}

	virtual void RunVScripts()
	{
		if ( ValidateScriptScope() )
		{
			// https://github.com/ReactiveDrop/reactivedrop_public_src/issues/138
			// We need to make sure our scope includes every value that might be looked up from it.
			// If we don't, global variables will be inherited by our scope and functions will be run twice.

			HSCRIPT hScope = GetScriptScope();
			Assert( hScope );
			for ( int i = 0; i < NUM_SCRIPT_GAME_EVENTS; i++ )
			{
				g_pScriptVM->SetValue( hScope, CFmtStr( "OnGameEvent_%s", g_ScriptGameEventList[i] ), SCRIPT_VARIANT_NULL );
			}
			g_pScriptVM->SetValue( hScope, "OnTakeDamage_Alive_Any", SCRIPT_VARIANT_NULL );
			g_pScriptVM->SetValue( hScope, "UserConsoleCommand", SCRIPT_VARIANT_NULL );
			g_pScriptVM->SetValue( hScope, "OnMissionStart", SCRIPT_VARIANT_NULL );
			g_pScriptVM->SetValue( hScope, "OnGameplayStart", SCRIPT_VARIANT_NULL );
		}

		BaseClass::RunVScripts();
	}

	virtual void UpdateOnRemove()
	{
		if ( GetScriptScope() )
		{
			g_pScriptVM->SetValue( "g_ModeScript", SCRIPT_VARIANT_NULL );
		}

		BaseClass::UpdateOnRemove();
	}
};

LINK_ENTITY_TO_CLASS( asw_challenge_thinker, CASW_Challenge_Thinker );

void CAlienSwarm::ApplyChallenge()
{
	if ( IsTutorialMap() )
	{
		EnableChallenge( "0" );
	}

	if ( !V_strcmp( rd_challenge.GetString(), "0" ) )
	{
		// no challenge
		return;
	}

	m_bChallengeActiveThisCampaign = true;
	m_bChallengeActiveThisMission = true;
	if ( GetCampaignSave() )
	{
		GetCampaignSave()->m_bChallengeEverActive = true;
		GetCampaignSave()->SaveGameToFile();
	}

	CheckChallengeConVars();

	if ( !filesystem->FileExists( CFmtStr( "scripts/vscripts/challenge_%s.nut", rd_challenge.GetString() ), "GAME" ) )
	{
		// no script
		return;
	}

	if ( !g_pScriptVM )
	{
		Error( "No VM to setup challenge script!\n" );
		return;
	}

	CASW_Challenge_Thinker *pThinker = assert_cast<CASW_Challenge_Thinker *>( CreateEntityByName( "asw_challenge_thinker" ) );
	if ( !pThinker )
	{
		Error( "Could not create challenge thinker!\n" );
		return;
	}
	pThinker->m_iszVScripts = AllocPooledString( CFmtStr( "challenge_%s", rd_challenge.GetString() ) );
	if ( !pThinker->ValidateScriptScope() )
	{
		Error( "Could not create challenge script scope!\n" );
		UTIL_Remove( pThinker );
		return;
	}
	g_pScriptVM->SetValue( "g_ModeScript", pThinker->GetScriptScope() );
	DispatchSpawn( pThinker, true );
}

#endif // GAME_DLL above

static int GetAllowedWeaponId( int iEquipSlot, int iWeaponIndex, const ConVar &allowedGuns, const ConVar &isInverted )
{
	int nNumRegular;
	switch ( iEquipSlot )
	{
	case ASW_INVENTORY_SLOT_PRIMARY:
	case ASW_INVENTORY_SLOT_SECONDARY:
		nNumRegular = ASWEquipmentList()->GetNumRegular( true );
		break;
	case ASW_INVENTORY_SLOT_EXTRA:
		nNumRegular = ASWEquipmentList()->GetNumExtra( true );
		break;
	default:
		Assert( false && "Invalid iEquipSlot" );
		return 0;
	}

	CUtlVector<int> vecWepList;
	{
		// convert string array to number array
		CUtlStringList szWepList;
		V_SplitString( allowedGuns.GetString(), " ", szWepList );
		for ( int i = 0; i < szWepList.Count(); ++i )
		{
			int iWepId = atoi( szWepList[i] );
			if ( iWepId < 0 || iWepId >= nNumRegular )
			{
				Warning( "Incorrect weapon ID=%i found in %s\n", iWepId, allowedGuns.GetName() );
				continue;
			}
			vecWepList.AddToTail( iWepId );
		}
	}

	CUtlVector<int> vecAllowedGuns;
	for ( int i = 0; i < nNumRegular; ++i )	// fill vecAllowedGuns with allowed guns
	{
		if ( isInverted.GetBool() && !vecWepList.HasElement(i) )
		{
			vecAllowedGuns.AddToTail( i );
		}
		else if ( !isInverted.GetBool() && vecWepList.HasElement( i ) )
		{
			vecAllowedGuns.AddToTail( i );
		}
	}

	CUtlVector<int> vecAllowedGunsUnsorted;
	if ( isInverted.GetBool() )	// don't do unsorting for inverted list, just copy
	{
		vecAllowedGunsUnsorted = vecAllowedGuns;
	}
	else
	{
		for ( int i = 0; i < vecWepList.Count(); ++i )	// unsort the vecAllowedGuns, we want to give the first available weapon from allowedGuns by default. E.g. in Minigun Carnage we will do this: rd_weapons_regular_allowed "15 7 14 5 17 6" and when selecting a marine player will get minigun as the primary weapon(previously it was giving the sentry gun)
		{
			if ( vecAllowedGuns.HasElement( vecWepList[i] ) )
			{
				vecAllowedGunsUnsorted.AddToTail( vecWepList[i] );
			}
		}
	}

	if ( vecAllowedGunsUnsorted.HasElement( iWeaponIndex ) )
		return iWeaponIndex;
	else if ( vecAllowedGunsUnsorted.Count() > 0 )
		return vecAllowedGunsUnsorted[0];
	else
	{
		Warning( "No valid weapon IDs found in %s!\n", allowedGuns.GetName() );
		return 0;
	}
}

// reactivedrop: This function reads rd_weapons_<slot>_allowed and checks whether
// the iWeaponIndex is allowed for pMR marine. If not it returns an ID of an
// alternative allowed weapon
int CAlienSwarm::ApplyWeaponSelectionRules( int iEquipSlot, int iWeaponIndex )
{
#ifdef CLIENT_DLL
	if ( !rd_weapon_requirement_override.GetBool() )
	{
		if ( CASW_Equip_Req::ForceWeaponUnlocked( STRING( ASWEquipmentList()->GetItemForSlot( iEquipSlot, iWeaponIndex )->m_EquipClass ) ) )
		{
			return iWeaponIndex;
		}
	}
#else
	CASW_Equip_Req *pEquipReq = m_hEquipReq.Get();
	if ( pEquipReq && !rd_weapon_requirement_override.GetBool() )
	{
		for ( int i = 0; i < CASW_Equip_Req::ASW_MAX_EQUIP_REQ_CLASSES; i++ )
		{
			CASW_EquipItem* pItem = ASWEquipmentList()->GetItemForSlot(iEquipSlot, iWeaponIndex);
			if ( pItem && !Q_stricmp( pEquipReq->GetEquipClass( i ), STRING( pItem->m_EquipClass ) ) )
			{
				return iWeaponIndex;
			}
		}
	}
#endif

	if ( V_stricmp( rd_weapons_regular_allowed.GetString(), "-1" ) && iEquipSlot < ASW_INVENTORY_SLOT_EXTRA )
	{
		return GetAllowedWeaponId( iEquipSlot, iWeaponIndex, rd_weapons_regular_allowed, rd_weapons_regular_allowed_inverted );
	}

	if ( V_stricmp( rd_weapons_extra_allowed.GetString(), "-1" ) && iEquipSlot == ASW_INVENTORY_SLOT_EXTRA )
	{
		return GetAllowedWeaponId( iEquipSlot, iWeaponIndex, rd_weapons_extra_allowed, rd_weapons_extra_allowed_inverted );
	}

	return iWeaponIndex;
}

bool CAlienSwarm::ShouldAllowCameraRotation( void )
{
	if ( ASWGameRules()->GetGameState() != ASW_GS_INGAME )
		return false;

	if ( m_iOverrideAllowRotateCamera != -1 )
		return m_iOverrideAllowRotateCamera != 0;

	Assert( g_pSwarmProxy );
	if ( !g_pSwarmProxy )
		return true;

	return !g_pSwarmProxy->m_bDisallowCameraRotation;
}

#ifdef GAME_DLL
void CAlienSwarm::CheckLeaderboardReady()
{
	Assert( ASWGameResource() );
	if ( !ASWGameResource() )
	{
		return;
	}

	if ( m_bSentLeaderboardReady )
	{
		return;
	}

	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CASW_Player *pPlayer = assert_cast<CASW_Player *>( UTIL_PlayerByIndex( i ) );
		if ( pPlayer && ASWGameResource()->GetFirstMarineResourceForPlayer( pPlayer ) )
		{
			if ( !pPlayer->m_bLeaderboardReady )
			{
				return;
			}
		}
	}

	m_bSentLeaderboardReady = true;
	CBroadcastRecipientFilter filter;
	UserMessageBegin( filter, "RDLeaderboardReady" );
	// no content, just a notification
	MessageEnd();
}
#endif

#ifdef GAME_DLL
bool CAlienSwarm::ShouldAllowMarineStrafePush(void)
{
	if (!rda_marine_allow_strafe.GetBool())
		return false;

	if (ASWGameRules()->GetGameState() != ASW_GS_INGAME)
		return false;

	return true;
}

void CAlienSwarm::SetPingLocation( const SteamNetworkPingLocation_t & location )
{
	char szLocation[k_cchMaxSteamNetworkingPingLocationString];
	SteamNetworkingUtils()->ConvertPingLocationToString( location, szLocation, sizeof( szLocation ) );
	m_bObtainedPingLocation = true;
	Assert( V_strlen( szLocation ) < sizeof( m_szApproximatePingLocation ) );
	if ( V_strlen( szLocation ) < sizeof( m_szApproximatePingLocation ) )
	{
		V_strncpy( m_szApproximatePingLocation.GetForModify(), szLocation, sizeof( m_szApproximatePingLocation ) );
	}
	else
	{
		*m_szApproximatePingLocation.GetForModify() = '\0';
		Warning( "Ping location is too long to network! %s\n", szLocation );
	}
}
#endif
