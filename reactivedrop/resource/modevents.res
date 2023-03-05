//=========== (C) Copyright 1999 Valve, L.L.C. All rights reserved. ===========
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//=============================================================================

// No spaces in event names, max length 32
// All strings are case sensitive
// total game event byte length must be < 1024
//
// valid data key types are:
//   none   : value is not networked
//   string : a zero terminated string
//   bool   : unsigned int, 1 bit
//   byte   : unsigned int, 8 bit
//   short  : signed int, 16 bit
//   long   : signed int, 32 bit
//   float  : float, 32 bit

"infestedevents"
{
	"asw_mission_restart"				// marines are restarting the mission
	{
		"restartcount"		"short"		// number of restarts in this play session
	}
	
	"gameui_activated"
	{
	}
	
	"gameui_hidden"
	{
	}
	
	"player_fullyjoined"
	{
		"userid"	"short"		// user ID on server
		"name"		"string"	// player name
	}
	
	"player_should_switch"
	{
		"userid"		"short"		// user ID on server
	}
	
	"player_commanding"
	{
		"userid"		"short"		// user ID on server
		"new_marine"	"long"		// new marine entindex
		"new_index"		"byte"		// index of the new marine
		"count"			"byte"		// number of marines commanded
	}
	
	"player_command_follow"
	{
		"userid"		"short"		// user ID on server
	}
	
	"player_command_hold"
	{
		"userid"		"short"		// user ID on server
	}
	
	"player_alt_fire"
	{
		"userid"		"short"		// user ID on server
	}
	
	"player_heal_target"
	{
		"userid"		"short"		// user ID on server
		"entindex"		"long"		// entindex of of the target marine
	}
	
	"player_heal_target_none"
	{
		"userid"		"short"		// user ID on server
	}
	
	"player_heal"
	{
		"userid"		"short"		// user ID on server
		"entindex"		"long"		// entindex of of the target marine
	}
	
	"player_give_ammo"
	{
		"userid"		"short"		// user ID on server
		"entindex"		"long"		// entindex of of the target marine
	}
	
	"player_deploy_ammo"
	{
		"userid"		"short"		// user ID on server
		"entindex"		"long"		// entindex of the ammo
		"marine"		"long"   	// marine entindex who placed the ammo
	}
	
	"player_dropped_weapon"
	{
		"userid"		"short"		// user ID on server
		"entindex"		"long"		// entindex of the dropped weapon
	}
	
	"marine_selected"
	{
		"userid"		"short"		// user ID on server
		"new_marine"	"long"		// new marine entindex
		"old_marine"	"long"		// old marine entindex
		"old_index"		"byte"		// index of the current marine
		"count"			"byte"		// number of marines commanded
	}
	
	"marine_ignited"
	{
		"entindex"		"long"		// entindex of of the target marine
	}
	
	"marine_extinguished"
	{
		"entindex"		"short"		// entindex of of the target marine
		"healer"		"short"		// entindex of extinguisher
		"weapon"		"short"		// healing weapon or extinguisher particle
	}
	
	"marine_infested"
	{
		"entindex"		"long"		// entindex of of the target marine
	}
	
	"marine_infested_cured"
	{
		"entindex"		"long"		// entindex of of the target marine
		"userid"		"short"		// user ID of the curer (if marine is inhabited)
		"marine"		"short"		// marine that cured infestation
		"weapon"		"short"		// healing weapon that cured the infestation
	}
	
	"marine_infested_killed"
	{
		"userid"		"short"		// user ID on server
		"entindex"		"long"		// entindex of of the target marine
	}
	
	"marine_no_ammo"
	{
		"userid"		"short"		// user ID on server
		"entindex"		"long"		// entindex of of the target marine
		"count"			"byte"		// number of marines commanded
	}
	
	"ammo_pickup"
	{
		"userid"	"short"		// user ID on server
		"entindex"	"long"		// entindex of of the marine that picked it up
	}
	
	"item_pickup"
	{
		"userid"	"short"		// user ID on server
		"entindex"	"long"		// item entindex
		"classname"	"string"	// item name
		"slot"		"short"		// slot the item was put in
		"replace"	"bool"		// did it replace another weapon
		"offhand"	"bool"		// can it be offhanded?
	}
	
	"weapon_reload"
	{
		"userid"			"short"		// user ID on server
		"marine"			"long"		// entindex of the marine
		"lost"				"short"		// ammo lost in the clip
		"clipsize"			"short"		// max ammo for that weapon
		"clipsremaining"	"short"		// remaining clips
		"clipsmax"			"short"		// remaining clips
	}
	
	"weapon_offhanded"
	{
		"userid"	"short"		// user ID on server
	}
	
	"door_recommend_destroy"
	{
		"userid"	"short"		// user ID on server
		"entindex"	"long"		// door entindex
	}
	
	"door_destroyed"
	{
		"userid"	"short"		// user ID on server
		"entindex"	"long"		// door entindex
	}
	
	"door_welded_visible"
	{
		"userid"	"short"		// user ID on server
		"subject"	"long"		// door entindex
		"entityname"	"string"	// name of the entity they see
	}
	
	"door_unwelded"
	{
		"userid"	"short"		// user ID on server
		"entindex"	"long"		// door entindex
	}
	
	"door_recommend_weld"
	{
		"entindex"	"long"		// door entindex
	}
	
	"door_welded"
	{
		"userid"	"short"		// user ID on server
		"entindex"	"long"		// door entindex
		"inhabited" "bool"		// is the marine inhabited
	}
	
	"sentry_placed"
	{
		"userid"	"short"		// user ID on server
	}
	
	"sentry_start_building"
	{
		"userid"	"short"		// user ID on server
		"entindex"	"long"		// sentry entindex
	}
	
	"sentry_stop_building"
	{
		"userid"	"short"		// user ID on server
		"entindex"	"long"		// sentry entindex
	}
	
	"sentry_complete"
	{
		"userid"	"short"		// user ID on server
		"entindex"	"long"		// sentry entindex
		"marine"	"long"   	// marine entindex who made the sentry
	}
	
	"sentry_rotated"
	{
		"userid"	"short"		// user ID on server
		"entindex"	"long"		// sentry entindex
	}
	
	"sentry_dismantled"
	{
		"userid"	"short"		// user ID on server
		"entindex"	"long"		// sentry entindex
	}
	
	"alien_ignited"
	{
		"userid"	"short"		// user ID on server
		"entindex"	"long"		// thing that got burned
	}
	
	"boulder_lasered"
	{
		"userid"	"short"		// user ID on server
		"entindex"	"long"		// thing that got burned
	}
	
	"physics_visible"
	{
		"userid"		"short"		// The player who sees the entity
		"subject"		"long"		// Entindex of the entity they see
		"type"			"string"	// Type of the entity they see
		"entityname"	"string"	// name of the entity they see
	}
	
	"physics_melee"
	{
		"attacker"		"short"		// The player who sees the entity
		"entindex"		"long"		// Entindex of the entity they kicked
	}
	
	"recommend_hold_position"
	{
	}
	
	"scanner_important"
	{
	}
	
	"general_hint"
	{
		"userid"		"short"		// The player who activated it
	}
	
	"movement_hint"
	{
		"userid"		"short"		// The player who needs the hint
	}
	
	"movement_hint_success"
	{
		"userid"		"short"		// The player succeeded
	}
	
	"alien_died"
	{
		"alien"			"short"		// Alien Classify() number
		"marine"		"short"		// entindex of the marine that killed the alien (0 if none)
		"weapon"		"short"		// Classify() of the weapon that killed the alien (0 if none)
	}
	
	"fast_reload"
	{	
		"marine"		"short"		// entindex of the marine reloading
		"reloads"		"byte"		// number of fast reloads the marine has done in a row
	}
					  
	"fast_reload_fail"
	{	
		"marine"		"short"		// entindex of the marine reloading
	}
	
	"difficulty_changed"
	{
		"newDifficulty"	"short"
		"oldDifficulty"	"short"
		"strDifficulty" "string" // new difficulty as string
	}
	
	"achievement_earned"
	{
		"player"	"byte"		// entindex of the player
		"achievement"	"short"		// achievement ID
	}
	
	"mission_success"
	{
		"strMapName" "string"
	}
	
	"mission_failed"
	{
		"strMapName" "string"
	}
	
	"marine_incapacitated"
	{
		"entindex"		"short"		// entindex of the marine
	}
	
	"marine_revived"
	{
		"entindex"		"short"		// entindex of the marine who was revived
		"reviver"		"short"		// entindex of the marine doing the reviving
	}
	
	"alien_spawn"
	{
		"entindex"		"long"		// entindex of the alien
	}
	
	"buzzer_spawn"
	{
		"entindex"		"long"		// entindex of the buzzer
	}
	
	"marine_spawn"
	{
		"userid"		"short"		// user ID on server
		"entindex"		"long"		// entindex of the marine
	}
	
	"marine_healed"
	{
		"medic_entindex"	"short"		// entindex of the marine medic
		"patient_entindex"	"short"		// entindex of the marine patient 
		"amount_healed"		"short"		// amount of meds given
		"amount"			"short"		// amount of meds given (limited to max health)
		"weapon"			"short"		// entindex of healing weapon
		"weapon_class"		"string"	// classname of the healing device
	}

	"colonist_spawn"
	{
		"entindex"		"long"		// entindex of the colonist
	}
	
	"weapon_reload_finish"
	{
		"userid"			"short"		// user ID on server
		"marine"			"long"		// entindex of the marine
		"clipsize"			"short"		// max ammo for that weapon
		"clipsremaining"	"short"		// remaining clips
		"clipsmax"			"short"		// max clips
	}
	
	"heal_beacon_placed"
	{
		"entindex"		"long"		// entindex of the heal beacon
		"marine"		"long"   	// marine entindex who placed the heal beacon
	}
	
	"damage_amplifier_placed"
	{
		"entindex"		"long"		// entindex of the damage amplifier
		"marine"		"long"   	// marine entindex who placed the damage amplifier
	}
	
	"tesla_trap_placed"
	{
		"entindex"		"long"		// entindex of the tesla trap
		"marine"		"long"   	// marine entindex who placed the tesla trap
	}

	"fire_mine_placed"
	{
		"entindex"		"long"		// entindex of the fire mine
		"marine"		"long"   	// marine entindex who placed the fire mine
	}

	"laser_mine_placed"
	{
		"entindex"		"long"		// entindex of the laser mine
		"marine"		"long"   	// marine entindex who placed the laser mine
	}

	"gas_grenade_placed"
	{
		"entindex"		"long"		// entindex of the gas grenade
		"marine"		"long"   	// marine entindex who placed the gas grenade
	}

	"flare_placed"
	{
		"entindex"		"long"		// entindex of the flare
		"marine"		"long"   	// marine entindex who placed the flare
	}

	"rocket_fired"
	{
		"entindex"		"long"		// entindex of the rocket
		"marine"		"long"   	// marine entindex who fired the rocket
	}

	"weapon_fire"
	{
		"userid"			"short"		// user ID on server
		"marine"			"long"		// entindex of the marine
		"weapon"			"long"		// entindex of the weapon
	}
	
	"weapon_offhand_activate"
	{
		"userid"			"short"		// user ID on server
		"marine"			"long"		// entindex of the marine
		"weapon"			"long"		// entindex of the weapon
	}
	
	"laser_mine_active"
	{
		"entindex"			"long"		// entindex of the laser mine
		"marine"			"long"		// entindex of the marine
	}

	"cluster_grenade_create"
	{
		"entindex"			"long"		// entindex of the cluster grenade
		"marine"			"long"		// entindex of the marine
		"weapon"			"long"		// entindex of the weapon
	}

	"fast_hack_success"
	{
		"entindex"		"long"		// entindex of the hack area
		"marine"		"long"   	// marine entindex who is hacking
	}

	"fast_hack_failed"
	{
		"entindex"		"long"		// entindex of the hack area
		"marine"		"long"   	// marine entindex who is hacking
	}

	"entity_frozen"
	{
		"entindex"		"short"		// entindex of frozen entity
		"attacker"		"short"		// entindex of attacker
		"weapon"		"short"		// entindex of weapon (grenade or sentry base)
	}

// CLIENTSIDE

	"alien_hurt"
	{
		"attacker"			"short"		// user ID on server
		"entindex"			"long"		// the alien
		"amount"			"float"		// damage done
	}
	
	"marine_hurt"
	{
		"userid"	"short"   	// player index who was hurt				
		"entindex"	"long"   	// marine entindex who was hurt	
		"attacker"	"short"	 	// player index who attacked
		"attackerindex"	"long"	// entindex of entity who attacked
		"health"	"float"		// remaining percent health points
		"friendlyfire"	"bool"	// false in deathmatch if players were not allies
	}
	
	"pickup_selected"
	{
		"entindex"	"long"		// item entindex
		"classname"	"string"	// classname of the entity
	}
	
	"sentry_selected"
	{
		"entindex"	"long"		// sentry entindex
	}
	
	"button_area_active"
	{
		"userid"	"short"   	// player who sees the active button
		"entindex"	"long"		// trigger entindex
		"prop"		"long"		// prop entindex
		"locked"	"bool"		// does it need hacking?
	}
	
	"button_area_inactive"
	{
		"entindex"	"long"		// trigger entindex
	}
	
	"button_area_used"
	{
		"userid"	"short"		// user ID on server
		"entindex"	"long"		// item entindex
	}
	"jukebox_play_random"
	{
		"fadeintime"	"float"
		"defaulttrack"	"string"
	}
	"jukebox_stop"
	{
		"fadeouttime"	"float"
	}
	"level_up"
	{
		"level"	"byte"
	}
	"campaign_changed"
	{
		"campaign" "string"
	}
	"swarm_state_changed"
	{
		"state" "short"
	}
}
