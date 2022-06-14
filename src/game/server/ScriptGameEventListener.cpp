//========= Copyright Valve Corporation, All rights reserved. ==============================//
//
// Purpose: Intercepts game events for VScript and call OnGameEvent_<eventname>.
//
//==========================================================================================//
#include "cbase.h"
#include "scriptgameeventlistener.h"
#include "vscript_server.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CScriptGameEventListener::CScriptGameEventListener()
{
}

CScriptGameEventListener::~CScriptGameEventListener()
{
	StopListeningForAllEvents();
}

void CScriptGameEventListener::FireGameEvent( IGameEvent *event )
{
	if ( !g_pScriptVM )
		return;

	char szGameEventFunctionName[256];
	Q_snprintf( szGameEventFunctionName, sizeof(szGameEventFunctionName), "OnGameEvent_%s", event->GetName() );

	HSCRIPT hGameEventFunc = g_pScriptVM->LookupFunction( szGameEventFunctionName );
	if ( hGameEventFunc )
	{
		ScriptVariant_t hGameEventTable;
		g_pScriptVM->CreateTable( hGameEventTable );
		SetVScriptEventValues( event, hGameEventTable );
		ScriptStatus_t nStatus = g_pScriptVM->Call( hGameEventFunc, NULL, false, NULL, hGameEventTable );
		if ( nStatus != SCRIPT_DONE )
		{
			DevWarning( "%s VScript function did not finish!\n", szGameEventFunctionName );
		}
		g_pScriptVM->ReleaseFunction( hGameEventFunc );
		g_pScriptVM->ReleaseValue( hGameEventTable );
	}

	if ( g_pScriptVM->ValueExists( "g_ModeScript" ) )
	{
		ScriptVariant_t hModeScript;
		if ( g_pScriptVM->GetValue( "g_ModeScript", &hModeScript ) )
		{
			if ( HSCRIPT hFunction = g_pScriptVM->LookupFunction( szGameEventFunctionName, hModeScript ) )
			{
				ScriptVariant_t hGameEventTable;
				g_pScriptVM->CreateTable( hGameEventTable );

				SetVScriptEventValues( event, hGameEventTable );

				ScriptStatus_t nStatus = g_pScriptVM->Call( hFunction, hModeScript, false, NULL, hGameEventTable );
				if ( nStatus != SCRIPT_DONE )
				{
					DevWarning( "%s VScript function did not finish!\n", szGameEventFunctionName );
				}

				g_pScriptVM->ReleaseFunction( hFunction );
				g_pScriptVM->ReleaseValue( hGameEventTable );
			}
			g_pScriptVM->ReleaseValue( hModeScript );
		}
	}
}

void CScriptGameEventListener::SetVScriptEventValues( IGameEvent *event, HSCRIPT table )
{
	if ( !g_pScriptVM ) return;

	if ( !event->IsEmpty("restartcount") )
		g_pScriptVM->SetValue( table, "restartcount", event->GetInt("restartcount") );
	if ( !event->IsEmpty("userid") )
		g_pScriptVM->SetValue( table, "userid", event->GetInt("userid") );
	if ( !event->IsEmpty("entindex") )
		g_pScriptVM->SetValue( table, "entindex", event->GetInt("entindex") );
	if ( !event->IsEmpty("health") )
		g_pScriptVM->SetValue( table, "health", event->GetFloat("health") );
	if ( !event->IsEmpty("amount") )
		g_pScriptVM->SetValue( table, "amount", event->GetFloat("amount") );
	if ( !event->IsEmpty("attacker") )
		g_pScriptVM->SetValue( table, "attacker", event->GetInt("attacker") );
	if ( !event->IsEmpty("attackerindex") )
		g_pScriptVM->SetValue( table, "attackerindex", event->GetInt("attackerindex") );
	if ( !event->IsEmpty("friendlyfire") )
		g_pScriptVM->SetValue( table, "friendlyfire", event->GetBool("friendlyfire") );
	if ( !event->IsEmpty("new_marine") )
		g_pScriptVM->SetValue( table, "new_marine", event->GetInt("new_marine") );
	if ( !event->IsEmpty("old_marine") )
		g_pScriptVM->SetValue( table, "old_marine", event->GetInt("old_marine") );
	if ( !event->IsEmpty("old_index") )
		g_pScriptVM->SetValue( table, "old_index", event->GetInt("old_index") );
	if ( !event->IsEmpty("count") )
		g_pScriptVM->SetValue( table, "count", event->GetInt("count") );
	if ( !event->IsEmpty("new_index") )
		g_pScriptVM->SetValue( table, "new_index", event->GetInt("new_index") );
	if ( !event->IsEmpty("name") )
		g_pScriptVM->SetValue( table, "name", event->GetString("name") );
	if ( !event->IsEmpty("classname") )
		g_pScriptVM->SetValue( table, "classname", event->GetString("classname") );
	if ( !event->IsEmpty("entityname") )
		g_pScriptVM->SetValue( table, "entityname", event->GetString("entityname") );
	if ( !event->IsEmpty("type") )
		g_pScriptVM->SetValue( table, "type", event->GetString("type") );
	if ( !event->IsEmpty("extinguisher") )
		g_pScriptVM->SetValue( table, "extinguisher", event->GetInt("extinguisher") );
	if ( !event->IsEmpty("curer") )
		g_pScriptVM->SetValue( table, "curer", event->GetInt("curer") );
	if ( !event->IsEmpty("slot") )
		g_pScriptVM->SetValue( table, "slot", event->GetInt("slot") );
	if ( !event->IsEmpty("replace") )
		g_pScriptVM->SetValue( table, "replace", event->GetBool("replace") );
	if ( !event->IsEmpty("offhand") )
		g_pScriptVM->SetValue( table, "offhand", event->GetBool("offhand") );
	if ( !event->IsEmpty("marine") )
		g_pScriptVM->SetValue( table, "marine", event->GetInt("marine") );
	if ( !event->IsEmpty("lost") )
		g_pScriptVM->SetValue( table, "lost", event->GetInt("lost") );
	if ( !event->IsEmpty("clipsize") )
		g_pScriptVM->SetValue( table, "clipsize", event->GetInt("clipsize") );
	if ( !event->IsEmpty("clipsremaining") )
		g_pScriptVM->SetValue( table, "clipsremaining", event->GetInt("clipsremaining") );
	if ( !event->IsEmpty("clipsmax") )
		g_pScriptVM->SetValue( table, "clipsmax", event->GetInt("clipsmax") );
	if ( !event->IsEmpty("inhabited") )
		g_pScriptVM->SetValue( table, "inhabited", event->GetBool("inhabited") );
	if ( !event->IsEmpty("alien") )
		g_pScriptVM->SetValue( table, "alien", event->GetInt("alien") );
	if ( !event->IsEmpty("weapon") )
		g_pScriptVM->SetValue( table, "weapon", event->GetInt("weapon") );
	if ( !event->IsEmpty("reloads") )
		g_pScriptVM->SetValue( table, "reloads", event->GetInt("reloads") );
	if ( !event->IsEmpty("newDifficulty") )
		g_pScriptVM->SetValue( table, "newDifficulty", event->GetInt("newDifficulty") );
	if ( !event->IsEmpty("oldDifficulty") )
		g_pScriptVM->SetValue( table, "oldDifficulty", event->GetInt("oldDifficulty") );
	if ( !event->IsEmpty("strDifficulty") )
		g_pScriptVM->SetValue( table, "strDifficulty", event->GetString("strDifficulty") );
	if ( !event->IsEmpty("player") )
		g_pScriptVM->SetValue( table, "player", event->GetInt("player") );
	if ( !event->IsEmpty("achievement") )
		g_pScriptVM->SetValue( table, "achievement", event->GetInt("achievement") );
	if ( !event->IsEmpty("strMapName") )
		g_pScriptVM->SetValue( table, "strMapName", event->GetString("strMapName") );
	if ( !event->IsEmpty("prop") )
		g_pScriptVM->SetValue( table, "prop", event->GetInt("prop") );
	if ( !event->IsEmpty("locked") )
		g_pScriptVM->SetValue( table, "locked", event->GetBool("locked") );
	if ( !event->IsEmpty("fadeintime") )
		g_pScriptVM->SetValue( table, "fadeintime", event->GetFloat("fadeintime") );
	if ( !event->IsEmpty("fadeouttime") )
		g_pScriptVM->SetValue( table, "fadeouttime", event->GetFloat("fadeouttime") );
	if ( !event->IsEmpty("defaulttrack") )
		g_pScriptVM->SetValue( table, "defaulttrack", event->GetString("defaulttrack") );
	if ( !event->IsEmpty("level") )
		g_pScriptVM->SetValue( table, "level", event->GetInt("level") );
	if ( !event->IsEmpty("state") )
		g_pScriptVM->SetValue( table, "state", event->GetInt("state") );
	if ( !event->IsEmpty("teamid") )
		g_pScriptVM->SetValue( table, "teamid", event->GetInt("teamid") );
	if ( !event->IsEmpty("teamname") )
		g_pScriptVM->SetValue( table, "teamname", event->GetString("teamname") );
	if ( !event->IsEmpty("score") )
		g_pScriptVM->SetValue( table, "score", event->GetInt("score") );
	if ( !event->IsEmpty("team") )
		g_pScriptVM->SetValue( table, "team", event->GetInt("team") );
	if ( !event->IsEmpty("sound") )
		g_pScriptVM->SetValue( table, "sound", event->GetString("sound") );
	if ( !event->IsEmpty("oldteam") )
		g_pScriptVM->SetValue( table, "oldteam", event->GetInt("oldteam") );
	if ( !event->IsEmpty("disconnect") )
		g_pScriptVM->SetValue( table, "disconnect", event->GetBool("disconnect") );
	if ( !event->IsEmpty("autoteam") )
		g_pScriptVM->SetValue( table, "autoteam", event->GetBool("autoteam") );
	if ( !event->IsEmpty("silent") )
		g_pScriptVM->SetValue( table, "silent", event->GetBool("silent") );
	if ( !event->IsEmpty("class") )
		g_pScriptVM->SetValue( table, "class", event->GetString("class") );
	if ( !event->IsEmpty("teamonly") )
		g_pScriptVM->SetValue( table, "teamonly", event->GetBool("teamonly") );
	if ( !event->IsEmpty("text") )
		g_pScriptVM->SetValue( table, "text", event->GetString("text") );
	if ( !event->IsEmpty("kills") )
		g_pScriptVM->SetValue( table, "kills", event->GetInt("kills") );
	if ( !event->IsEmpty("deaths") )
		g_pScriptVM->SetValue( table, "deaths", event->GetInt("deaths") );
	if ( !event->IsEmpty("mode") )
		g_pScriptVM->SetValue( table, "mode", event->GetInt("mode") );
	if ( !event->IsEmpty("entity") )
		g_pScriptVM->SetValue( table, "entity", event->GetInt("entity") );
	if ( !event->IsEmpty("oldname") )
		g_pScriptVM->SetValue( table, "oldname", event->GetString("oldname") );
	if ( !event->IsEmpty("newname") )
		g_pScriptVM->SetValue( table, "newname", event->GetString("newname") );
	if ( !event->IsEmpty("hintmessage") )
		g_pScriptVM->SetValue( table, "hintmessage", event->GetString("hintmessage") );
	if ( !event->IsEmpty("mapname") )
		g_pScriptVM->SetValue( table, "mapname", event->GetString("mapname") );
	if ( !event->IsEmpty("roundslimit") )
		g_pScriptVM->SetValue( table, "roundslimit", event->GetInt("roundslimit") );
	if ( !event->IsEmpty("timelimit") )
		g_pScriptVM->SetValue( table, "timelimit", event->GetInt("timelimit") );
	if ( !event->IsEmpty("fraglimit") )
		g_pScriptVM->SetValue( table, "fraglimit", event->GetInt("fraglimit") );
	if ( !event->IsEmpty("objective") )
		g_pScriptVM->SetValue( table, "objective", event->GetString("objective") );
	if ( !event->IsEmpty("winner") )
		g_pScriptVM->SetValue( table, "winner", event->GetInt("winner") );
	if ( !event->IsEmpty("reason") )
		g_pScriptVM->SetValue( table, "reason", event->GetInt("reason") );
	if ( !event->IsEmpty("message") )
		g_pScriptVM->SetValue( table, "message", event->GetString("message") );
	if ( !event->IsEmpty("full_reset") )
		g_pScriptVM->SetValue( table, "full_reset", event->GetBool("full_reset") );
	if ( !event->IsEmpty("hostname") )
		g_pScriptVM->SetValue( table, "hostname", event->GetString("hostname") );
	if ( !event->IsEmpty("rushes") )
		g_pScriptVM->SetValue( table, "rushes", event->GetInt("rushes") );
	if ( !event->IsEmpty("target") )
		g_pScriptVM->SetValue( table, "target", event->GetInt("target") );
	if ( !event->IsEmpty("material") )
		g_pScriptVM->SetValue( table, "material", event->GetInt("material") );
	if ( !event->IsEmpty("entindex_killed") )
		g_pScriptVM->SetValue( table, "entindex_killed", event->GetInt("entindex_killed") );
	if ( !event->IsEmpty("entindex_attacker") )
		g_pScriptVM->SetValue( table, "entindex_attacker", event->GetInt("entindex_attacker") );
	if ( !event->IsEmpty("entindex_inflictor") )
		g_pScriptVM->SetValue( table, "entindex_inflictor", event->GetInt("entindex_inflictor") );
	if ( !event->IsEmpty("damagebits") )
		g_pScriptVM->SetValue( table, "damagebits", event->GetInt("damagebits") );
	if ( !event->IsEmpty("numadvanced") )
		g_pScriptVM->SetValue( table, "numadvanced", event->GetInt("numadvanced") );
	if ( !event->IsEmpty("numbronze") )
		g_pScriptVM->SetValue( table, "numbronze", event->GetInt("numbronze") );
	if ( !event->IsEmpty("numsilver") )
		g_pScriptVM->SetValue( table, "numsilver", event->GetInt("numsilver") );
	if ( !event->IsEmpty("numgold") )
		g_pScriptVM->SetValue( table, "numgold", event->GetInt("numgold") );
	if ( !event->IsEmpty("forceupload") )
		g_pScriptVM->SetValue( table, "forceupload", event->GetBool("forceupload") );
	if ( !event->IsEmpty("achievement_name") )
		g_pScriptVM->SetValue( table, "achievement_name", event->GetString("achievement_name") );
	if ( !event->IsEmpty("cur_val") )
		g_pScriptVM->SetValue( table, "cur_val", event->GetInt("cur_val") );
	if ( !event->IsEmpty("max_val") )
		g_pScriptVM->SetValue( table, "max_val", event->GetInt("max_val") );
	if ( !event->IsEmpty("subject") )
		g_pScriptVM->SetValue( table, "subject", event->GetInt("subject") );
	if ( !event->IsEmpty("group") )
		g_pScriptVM->SetValue( table, "group", event->GetString("group") );
	if ( !event->IsEmpty("enabled") )
		g_pScriptVM->SetValue( table, "enabled", event->GetInt("enabled") );
	if ( !event->IsEmpty("hint_name") )
		g_pScriptVM->SetValue( table, "hint_name", event->GetString("hint_name") );
	if ( !event->IsEmpty("hint_replace_key") )
		g_pScriptVM->SetValue( table, "hint_replace_key", event->GetString("hint_replace_key") );
	if ( !event->IsEmpty("hint_target") )
		g_pScriptVM->SetValue( table, "hint_target", event->GetInt("hint_target") );
	if ( !event->IsEmpty("hint_activator_userid") )
		g_pScriptVM->SetValue( table, "hint_activator_userid", event->GetInt("hint_activator_userid") );
	if ( !event->IsEmpty("hint_timeout") )
		g_pScriptVM->SetValue( table, "hint_timeout", event->GetInt("hint_timeout") );
	if ( !event->IsEmpty("hint_icon_onscreen") )
		g_pScriptVM->SetValue( table, "hint_icon_onscreen", event->GetString("hint_icon_onscreen") );
	if ( !event->IsEmpty("hint_icon_offscreen") )
		g_pScriptVM->SetValue( table, "hint_icon_offscreen", event->GetString("hint_icon_offscreen") );
	if ( !event->IsEmpty("hint_caption") )
		g_pScriptVM->SetValue( table, "hint_caption", event->GetString("hint_caption") );
	if ( !event->IsEmpty("hint_activator_caption") )
		g_pScriptVM->SetValue( table, "hint_activator_caption", event->GetString("hint_activator_caption") );
	if ( !event->IsEmpty("hint_color") )
		g_pScriptVM->SetValue( table, "hint_color", event->GetString("hint_color") );
	if ( !event->IsEmpty("hint_icon_offset") )
		g_pScriptVM->SetValue( table, "hint_icon_offset", event->GetFloat("hint_icon_offset") );
	if ( !event->IsEmpty("hint_range") )
		g_pScriptVM->SetValue( table, "hint_range", event->GetFloat("hint_range") );
	if ( !event->IsEmpty("hint_flags") )
		g_pScriptVM->SetValue( table, "hint_flags", event->GetInt("hint_flags") );
	if ( !event->IsEmpty("hint_binding") )
		g_pScriptVM->SetValue( table, "hint_binding", event->GetString("hint_binding") );
	if ( !event->IsEmpty("hint_allow_nodraw_target") )
		g_pScriptVM->SetValue( table, "hint_allow_nodraw_target", event->GetBool("hint_allow_nodraw_target") );
	if ( !event->IsEmpty("hint_nooffscreen") )
		g_pScriptVM->SetValue( table, "hint_nooffscreen", event->GetBool("hint_nooffscreen") );
	if ( !event->IsEmpty("hint_forcecaption") )
		g_pScriptVM->SetValue( table, "hint_forcecaption", event->GetBool("hint_forcecaption") );
	if ( !event->IsEmpty("hint_local_player_only") )
		g_pScriptVM->SetValue( table, "hint_local_player_only", event->GetBool("hint_local_player_only") );
	if ( !event->IsEmpty("clients") )
		g_pScriptVM->SetValue( table, "clients", event->GetInt("clients") );
	if ( !event->IsEmpty("slots") )
		g_pScriptVM->SetValue( table, "slots", event->GetInt("slots") );
	if ( !event->IsEmpty("proxies") )
		g_pScriptVM->SetValue( table, "proxies", event->GetInt("proxies") );
	if ( !event->IsEmpty("master") )
		g_pScriptVM->SetValue( table, "master", event->GetString("master") );
	if ( !event->IsEmpty("index") )
		g_pScriptVM->SetValue( table, "index", event->GetInt("index") );
	if ( !event->IsEmpty("rank") )
		g_pScriptVM->SetValue( table, "rank", event->GetFloat("rank") );
	if ( !event->IsEmpty("posx") )
		g_pScriptVM->SetValue( table, "posx", event->GetInt("posx") );
	if ( !event->IsEmpty("posy") )
		g_pScriptVM->SetValue( table, "posy", event->GetInt("posy") );
	if ( !event->IsEmpty("posz") )
		g_pScriptVM->SetValue( table, "posz", event->GetInt("posz") );
	if ( !event->IsEmpty("theta") )
		g_pScriptVM->SetValue( table, "theta", event->GetInt("theta") );
	if ( !event->IsEmpty("phi") )
		g_pScriptVM->SetValue( table, "phi", event->GetInt("phi") );
	if ( !event->IsEmpty("offset") )
		g_pScriptVM->SetValue( table, "offset", event->GetInt("offset") );
	if ( !event->IsEmpty("fov") )
		g_pScriptVM->SetValue( table, "fov", event->GetFloat("fov") );
	if ( !event->IsEmpty("target1") )
		g_pScriptVM->SetValue( table, "target1", event->GetInt("target1") );
	if ( !event->IsEmpty("target2") )
		g_pScriptVM->SetValue( table, "target2", event->GetInt("target2") );
	if ( !event->IsEmpty("distance") )
		g_pScriptVM->SetValue( table, "distance", event->GetInt("distance") );
	if ( !event->IsEmpty("inertia") )
		g_pScriptVM->SetValue( table, "inertia", event->GetInt("inertia") );
	if ( !event->IsEmpty("ineye") )
		g_pScriptVM->SetValue( table, "ineye", event->GetInt("ineye") );
	if ( !event->IsEmpty("address") )
		g_pScriptVM->SetValue( table, "address", event->GetString("address") );
	if ( !event->IsEmpty("port") )
		g_pScriptVM->SetValue( table, "port", event->GetInt("port") );
	if ( !event->IsEmpty("game") )
		g_pScriptVM->SetValue( table, "game", event->GetString("game") );
	if ( !event->IsEmpty("maxplayers") )
		g_pScriptVM->SetValue( table, "maxplayers", event->GetInt("maxplayers") );
	if ( !event->IsEmpty("os") )
		g_pScriptVM->SetValue( table, "os", event->GetString("os") );
	if ( !event->IsEmpty("dedicated") )
		g_pScriptVM->SetValue( table, "dedicated", event->GetBool("dedicated") );
	if ( !event->IsEmpty("password") )
		g_pScriptVM->SetValue( table, "password", event->GetBool("password") );
	if ( !event->IsEmpty("reason") )
		g_pScriptVM->SetValue( table, "reason", event->GetString("reason") );
	if ( !event->IsEmpty("cvarname") )
		g_pScriptVM->SetValue( table, "cvarname", event->GetString("cvarname") );
	if ( !event->IsEmpty("cvarvalue") )
		g_pScriptVM->SetValue( table, "cvarvalue", event->GetString("cvarvalue") );
	if ( !event->IsEmpty("networkid") )
		g_pScriptVM->SetValue( table, "networkid", event->GetString("networkid") );
	if ( !event->IsEmpty("ip") )
		g_pScriptVM->SetValue( table, "ip", event->GetString("ip") );
	if ( !event->IsEmpty("duration") )
		g_pScriptVM->SetValue( table, "duration", event->GetString("duration") );
	if ( !event->IsEmpty("by") )
		g_pScriptVM->SetValue( table, "by", event->GetString("by") );
	if ( !event->IsEmpty("kicked") )
		g_pScriptVM->SetValue( table, "kicked", event->GetBool("kicked") );
	if ( !event->IsEmpty("bot") )
		g_pScriptVM->SetValue( table, "bot", event->GetBool("bot") );
	if ( !event->IsEmpty("reviver") )
		g_pScriptVM->SetValue( table, "reviver", event->GetInt("reviver") );
	if ( !event->IsEmpty("splitscreenplayer") )
		g_pScriptVM->SetValue( table, "splitscreenplayer", event->GetInt("splitscreenplayer") );
	if (!event->IsEmpty("medic_entindex"))
		g_pScriptVM->SetValue(table, "medic_entindex", event->GetInt("medic_entindex"));
	if (!event->IsEmpty("patient_entindex"))
		g_pScriptVM->SetValue(table, "patient_entindex", event->GetInt("patient_entindex"));
	if (!event->IsEmpty("amount_healed"))
		g_pScriptVM->SetValue(table, "amount_healed", event->GetInt("amount_healed"));
	if (!event->IsEmpty("weapon_class"))
		g_pScriptVM->SetValue(table, "weapon_class", event->GetString("weapon_class"));
}

bool CScriptGameEventListener::Init()
{
	ListenForGameEvent( "asw_mission_restart" );
	ListenForGameEvent( "gameui_activated" );
	ListenForGameEvent( "gameui_hidden" );
	ListenForGameEvent( "player_fullyjoined" );
	ListenForGameEvent( "player_should_switch" );
	ListenForGameEvent( "player_commanding" );
	ListenForGameEvent( "player_command_follow" );
	ListenForGameEvent( "player_command_hold" );
	ListenForGameEvent( "player_alt_fire" );
	ListenForGameEvent( "player_heal_target" );
	ListenForGameEvent( "player_heal_target_none" );
	ListenForGameEvent( "player_heal" );
	ListenForGameEvent( "player_give_ammo" );
	ListenForGameEvent( "player_deploy_ammo" );
	ListenForGameEvent( "player_dropped_weapon" );
	ListenForGameEvent( "marine_selected" );
	ListenForGameEvent( "marine_ignited" );
	ListenForGameEvent( "marine_extinguished" );
	ListenForGameEvent( "marine_infested" );
	ListenForGameEvent( "marine_infested_cured" );
	ListenForGameEvent( "marine_infested_killed" );
	ListenForGameEvent( "marine_healed" );
	ListenForGameEvent( "marine_no_ammo" );
	ListenForGameEvent( "ammo_pickup" );
	ListenForGameEvent( "item_pickup" );
	ListenForGameEvent( "weapon_reload" );
	ListenForGameEvent( "weapon_offhanded" );
	ListenForGameEvent( "door_recommend_destroy" );
	ListenForGameEvent( "door_destroyed" );
	ListenForGameEvent( "door_welded_visible" );
	ListenForGameEvent( "door_unwelded" );
	ListenForGameEvent( "door_recommend_weld" );
	ListenForGameEvent( "door_welded" );
	ListenForGameEvent( "sentry_placed" );
	ListenForGameEvent( "sentry_start_building" );
	ListenForGameEvent( "sentry_stop_building" );
	ListenForGameEvent( "sentry_complete" );
	ListenForGameEvent( "sentry_rotated" );
	ListenForGameEvent( "sentry_dismantled" );
	ListenForGameEvent( "alien_ignited" );
	ListenForGameEvent( "boulder_lasered" );
	ListenForGameEvent( "physics_visible" );
	ListenForGameEvent( "physics_melee" );
	ListenForGameEvent( "recommend_hold_position" );
	ListenForGameEvent( "scanner_important" );
	ListenForGameEvent( "general_hint" );
	ListenForGameEvent( "movement_hint" );
	ListenForGameEvent( "movement_hint_success" );
	ListenForGameEvent( "alien_died" );
	ListenForGameEvent( "fast_reload" );
	ListenForGameEvent( "difficulty_changed" );
	ListenForGameEvent( "achievement_earned" );
	ListenForGameEvent( "mission_success" );
	ListenForGameEvent( "mission_failed" );
	ListenForGameEvent( "alien_hurt" );
	ListenForGameEvent( "marine_hurt" );
	ListenForGameEvent( "pickup_selected" );
	ListenForGameEvent( "sentry_selected" );
	ListenForGameEvent( "button_area_active" );
	ListenForGameEvent( "button_area_inactive" );
	ListenForGameEvent( "button_area_used" );
	ListenForGameEvent( "jukebox_play_random" );
	ListenForGameEvent( "jukebox_stop" );
	ListenForGameEvent( "level_up" );
	ListenForGameEvent( "campaign_changed" );
	ListenForGameEvent( "swarm_state_changed" );
	ListenForGameEvent( "team_info" );
	ListenForGameEvent( "team_score" );
	ListenForGameEvent( "teamplay_broadcast_audio" );
	ListenForGameEvent( "player_team" );
	ListenForGameEvent( "player_class" );
	ListenForGameEvent( "player_death" );
	ListenForGameEvent( "player_hurt" );
	ListenForGameEvent( "player_chat" );
	ListenForGameEvent( "player_score" );
	ListenForGameEvent( "player_spawn" );
	ListenForGameEvent( "player_shoot" );
	ListenForGameEvent( "player_use" );
	ListenForGameEvent( "player_changename" );
	ListenForGameEvent( "player_hintmessage" );
	ListenForGameEvent( "game_init" );
	ListenForGameEvent( "game_newmap" );
	ListenForGameEvent( "game_start" );
	ListenForGameEvent( "game_end" );
	ListenForGameEvent( "round_start" );
	ListenForGameEvent( "round_end" );
	ListenForGameEvent( "round_start_pre_entity" );
	ListenForGameEvent( "teamplay_round_start" );
	ListenForGameEvent( "hostname_changed" );
	ListenForGameEvent( "finale_start" );
	ListenForGameEvent( "game_message" );
	ListenForGameEvent( "break_breakable" );
	ListenForGameEvent( "break_prop" );
	ListenForGameEvent( "entity_killed" );
	ListenForGameEvent( "bonus_updated" );
	ListenForGameEvent( "player_stats_updated" );
	ListenForGameEvent( "achievement_event" );
	ListenForGameEvent( "achievement_earned" );
	ListenForGameEvent( "achievement_write_failed" );
	ListenForGameEvent( "physgun_pickup" );
	ListenForGameEvent( "flare_ignite_npc" );
	ListenForGameEvent( "helicopter_grenade_punt_miss" );
	ListenForGameEvent( "user_data_downloaded" );
	ListenForGameEvent( "ragdoll_dissolved" );
	ListenForGameEvent( "gameinstructor_draw" );
	ListenForGameEvent( "gameinstructor_nodraw" );
	ListenForGameEvent( "map_transition" );
	ListenForGameEvent( "entity_visible" );
	ListenForGameEvent( "set_instructor_group_enabled" );
	ListenForGameEvent( "instructor_server_hint_create" );
	ListenForGameEvent( "instructor_server_hint_stop" );
	ListenForGameEvent( "server_spawn" );
	ListenForGameEvent( "server_pre_shutdown" );
	ListenForGameEvent( "server_shutdown" );
	ListenForGameEvent( "server_cvar" );
	ListenForGameEvent( "server_message" );
	ListenForGameEvent( "server_addban" );
	ListenForGameEvent( "server_removeban" );
	ListenForGameEvent( "player_connect" );
	ListenForGameEvent( "player_info" );
	ListenForGameEvent( "player_disconnect" );
	ListenForGameEvent( "player_activate" );
	ListenForGameEvent( "player_connect_full" );
	ListenForGameEvent( "player_say" );
	ListenForGameEvent( "hltv_status" );
	ListenForGameEvent( "hltv_cameraman" );
	ListenForGameEvent( "hltv_rank_camera" );
	ListenForGameEvent( "hltv_rank_entity" );
	ListenForGameEvent( "hltv_fixed" );
	ListenForGameEvent( "hltv_chase" );
	ListenForGameEvent( "hltv_message" );
	ListenForGameEvent( "hltv_title" );
	ListenForGameEvent( "hltv_chat" );
	ListenForGameEvent( "marine_incapacitated" );
	ListenForGameEvent( "marine_revived" );
	ListenForGameEvent( "alien_spawn" );
	ListenForGameEvent( "buzzer_spawn" );
	ListenForGameEvent( "marine_spawn" );
	ListenForGameEvent( "colonist_spawn" );
	ListenForGameEvent( "weapon_reload_finish" );
	ListenForGameEvent( "heal_beacon_placed" );
	ListenForGameEvent( "damage_amplifier_placed" );
	ListenForGameEvent( "weapon_fire" );
	ListenForGameEvent( "weapon_offhand_activate" );
	ListenForGameEvent( "laser_mine_active" );
	ListenForGameEvent( "cluster_grenade_create" );
	ListenForGameEvent( "tesla_trap_placed" );
	ListenForGameEvent( "fire_mine_placed" );
	ListenForGameEvent( "laser_mine_placed" );
	ListenForGameEvent( "gas_grenade_placed" );
	ListenForGameEvent( "flare_placed" );
	ListenForGameEvent( "rocket_fired" );

	return true;
}

static CScriptGameEventListener	s_ScriptGameEventListener;
CScriptGameEventListener *g_pScriptGameEventListener = &s_ScriptGameEventListener;
