#include "cbase.h"

#ifdef CLIENT_DLL
	#include "c_asw_marine.h"
	#include "c_asw_marine_resource.h"
	#include "c_asw_game_resource.h"
	#define CASW_Marine C_ASW_Marine
	#define CASW_Marine_Resource C_ASW_Marine_Resource
	#define CASW_Game_Resource C_ASW_Game_Resource
#else
	#include "asw_marine.h"
	#include "asw_marine_resource.h"
	#include "asw_game_resource.h"
#endif
#include "rd_gamerules_convar.h"
#include "asw_marine_skills.h"
#include "asw_marine_profile.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static CRD_GameRulesConVarCollection s_ASWMarineSkillsConVars( "asw_marine_skills" );

// Never add anything to the start or middle of this list!
// Never remove anything from this list!
// Demos and networking will break!
enum
{
	AMSC_LEADERSHIP_ACCURACY_CHANCE_BASE,
	AMSC_LEADERSHIP_DAMAGE_RESIST_BASE,
	AMSC_VINDICATOR_DMG_BASE,
	AMSC_VINDICATOR_PELLETS_BASE,
	AMSC_AUTOGUN_BASE,
	AMSC_AUTOGUN_MINIGUN_DMG_BASE,
	AMSC_AUTOGUN_CRYO_SPINUP_BASE,
	AMSC_AUTOGUN_CRYO_FREEZE_BASE,
	AMSC_AUTOGUN_CRYO_RADIUS_BASE,
	AMSC_PIERCING_BASE,
	AMSC_STOPPING_POWER_AIRBLAST_STRENGTH_BASE,
	AMSC_HEALING_CHARGES_BASE,
	AMSC_SELF_HEALING_CHARGES_BASE,
	AMSC_HEALING_MEDRIFLE_HEALING_CHARGES_BASE,
	AMSC_HEALING_MEDKIT_HPS_BASE,
	AMSC_HEALING_HPS_BASE,
	AMSC_HEALING_GRENADE_BASE,
	AMSC_HEALING_GUN_CHARGES_BASE,
	AMSC_HEALING_GUN_BASE,
	AMSC_HEALING_AMP_GUN_CHARGES_BASE,
	AMSC_XENOWOUNDS_BASE,
	AMSC_DRUGS_BASE,
	AMSC_DRUGS_HEALAMP_BASE,
	AMSC_HACKING_SPEED_BASE,
	AMSC_SCANNER_BASE,
	AMSC_ENGINEERING_WELDING_BASE,
	AMSC_ENGINEERING_SENTRY_BASE,
	AMSC_ENGINEERING_FIRERATE_BASE,
	AMSC_ENGINEERING_SHIELD_HEALTH_BASE,
	AMSC_ENGINEERING_SHIELD_DURATION_BASE,
	AMSC_GRENADES_RADIUS_BASE,
	AMSC_GRENADES_DMG_BASE,
	AMSC_GRENADES_INCENDIARY_DMG_BASE,
	AMSC_GRENADES_CLUSTER_DMG_BASE,
	AMSC_GRENADES_CLUSTERS_BASE,
	AMSC_GRENADES_FLECHETTE_DMG_BASE,
	AMSC_GRENADES_HORNET_DMG_BASE,
	AMSC_GRENADES_HORNET_COUNT_BASE,
	AMSC_GRENADES_HORNET_INTERVAL_BASE,
	AMSC_GRENADES_FREEZE_RADIUS_BASE,
	AMSC_GRENADES_FREEZE_DURATION_BASE,
	AMSC_GRENADES_SMART_COUNT_BASE,
	AMSC_GRENADES_SMART_INTERVAL_BASE,
	AMSC_HEALTH_BASE,
	AMSC_MELEE_DMG_BASE,
	AMSC_MELEE_FORCE_BASE,
	AMSC_MELEE_SPEED_BASE,
	AMSC_RELOADING_BASE,
	AMSC_RELOADING_FAST_BASE,
	AMSC_AGILITY_MOVESPEED_BASE,
	AMSC_LEADERSHIP_ACCURACY_CHANCE_STEP,
	AMSC_LEADERSHIP_DAMAGE_RESIST_STEP,
	AMSC_VINDICATOR_DMG_STEP,
	AMSC_VINDICATOR_PELLETS_STEP,
	AMSC_AUTOGUN_STEP,
	AMSC_AUTOGUN_MINIGUN_DMG_STEP,
	AMSC_AUTOGUN_CRYO_SPINUP_STEP,
	AMSC_AUTOGUN_CRYO_FREEZE_STEP,
	AMSC_AUTOGUN_CRYO_RADIUS_STEP,
	AMSC_PIERCING_STEP,
	AMSC_STOPPING_POWER_AIRBLAST_STRENGTH_STEP,
	AMSC_HEALING_CHARGES_STEP,
	AMSC_SELF_HEALING_CHARGES_STEP,
	AMSC_HEALING_MEDRIFLE_HEALING_CHARGES_STEP,
	AMSC_HEALING_HPS_STEP,
	AMSC_HEALING_GRENADE_STEP,
	AMSC_HEALING_GUN_CHARGES_STEP,
	AMSC_HEALING_GUN_STEP,
	AMSC_HEALING_AMP_GUN_CHARGES_STEP,
	AMSC_HEALING_MEDKIT_HPS_STEP,
	AMSC_XENOWOUNDS_STEP,
	AMSC_DRUGS_STEP,
	AMSC_DRUGS_HEALAMP_STEP,
	AMSC_HACKING_SPEED_STEP,
	AMSC_SCANNER_STEP,
	AMSC_ENGINEERING_WELDING_STEP,
	AMSC_ENGINEERING_SENTRY_STEP,
	AMSC_ENGINEERING_FIRERATE_STEP,
	AMSC_ENGINEERING_SHIELD_HEALTH_STEP,
	AMSC_ENGINEERING_SHIELD_DURATION_STEP,
	AMSC_GRENADES_RADIUS_STEP,
	AMSC_GRENADES_DMG_STEP,
	AMSC_GRENADES_INCENDIARY_DMG_STEP,
	AMSC_GRENADES_CLUSTER_DMG_STEP,
	AMSC_GRENADES_CLUSTERS_STEP,
	AMSC_GRENADES_FLECHETTE_DMG_STEP,
	AMSC_GRENADES_HORNET_DMG_STEP,
	AMSC_GRENADES_HORNET_COUNT_STEP,
	AMSC_GRENADES_HORNET_INTERVAL_STEP,
	AMSC_GRENADES_FREEZE_RADIUS_STEP,
	AMSC_GRENADES_FREEZE_DURATION_STEP,
	AMSC_GRENADES_SMART_COUNT_STEP,
	AMSC_GRENADES_SMART_INTERVAL_STEP,
	AMSC_HEALTH_STEP,
	AMSC_MELEE_DMG_STEP,
	AMSC_MELEE_FORCE_STEP,
	AMSC_MELEE_SPEED_STEP,
	AMSC_RELOADING_STEP,
	AMSC_RELOADING_FAST_STEP,
	AMSC_AGILITY_MOVESPEED_STEP,
	AMSC_AGILITY_RELOAD_STEP, // unused
	AMSC_MINES_FIRES_BASE,
	AMSC_MINES_FIRES_STEP, // unused
	AMSC_MINES_DURATION_BASE,
	AMSC_MINES_DURATION_STEP,
	AMSC_ACCURACY_RIFLE_DMG_BASE,
	AMSC_ACCURACY_RIFLE_DMG_STEP,
	AMSC_ACCURACY_PRIFLE_DMG_BASE,
	AMSC_ACCURACY_PRIFLE_DMG_STEP,
	AMSC_ACCURACY_SHOTGUN_DMG_BASE,
	AMSC_ACCURACY_SHOTGUN_DMG_STEP,
	AMSC_ACCURACY_RAILGUN_DMG_BASE,
	AMSC_ACCURACY_RAILGUN_DMG_STEP,
	AMSC_ACCURACY_FLAMER_DMG_BASE,
	AMSC_ACCURACY_FLAMER_DMG_STEP,
	AMSC_ACCURACY_PISTOL_DMG_BASE,
	AMSC_ACCURACY_PISTOL_DMG_STEP,
	AMSC_ACCURACY_BULLDOG_DMG_BASE,
	AMSC_ACCURACY_BULLDOG_DMG_STEP,
	AMSC_ACCURACY_PDW_DMG_BASE,
	AMSC_ACCURACY_PDW_DMG_STEP,
	AMSC_MUZZLE_FLASH_BASE,
	AMSC_MUZZLE_FLASH_STEP,
	AMSC_ACCURACY_SNIPER_RIFLE_DMG_BASE,
	AMSC_ACCURACY_SNIPER_RIFLE_DMG_STEP,
	AMSC_ACCURACY_TESLA_CANNON_DMG_BASE,
	AMSC_ACCURACY_TESLA_CANNON_DMG_STEP,
	AMSC_ACCURACY_HEAVY_RIFLE_DMG_BASE,
	AMSC_ACCURACY_HEAVY_RIFLE_DMG_STEP,
	AMSC_ACCURACY_MEDRIFLE_DMG_BASE,
	AMSC_ACCURACY_MEDRIFLE_DMG_STEP,
	AMSC_ACCURACY_AR2_DMG_BASE,
	AMSC_ACCURACY_AR2_DMG_STEP,
	AMSC_ACCURACY_DEVASTATOR_DMG_BASE,
	AMSC_ACCURACY_DEVASTATOR_DMG_STEP,
	AMSC_ACCURACY_50CALMG_DMG_BASE,
	AMSC_ACCURACY_50CALMG_DMG_STEP,
	AMSC_ACCURACY_MINING_LASER_DMG_BASE,
	AMSC_ACCURACY_MINING_LASER_DMG_STEP,
	AMSC_ACCURACY_SHIELD_RIFLE_DMG_BASE,
	AMSC_ACCURACY_SHIELD_RIFLE_DMG_STEP,
	AMSC_ACCURACY_CRYO_DMG_BASE,
	AMSC_ACCURACY_CRYO_DMG_STEP,
	AMSC_ACCURACY_SHIELD_DAMAGE_BASE,
	AMSC_ACCURACY_SHIELD_DAMAGE_STEP,
	AMSC_LASER_MINES_BASE,
	AMSC_LASER_MINES_MODERATE,
	AMSC_LASER_MINES_EXPERT,
	AMSC_VINDICATOR_MUZZLE_FLASH_BASE,
	AMSC_VINDICATOR_MUZZLE_FLASH_STEP,
	AMSC_MINES_FIRES_MODERATE,
	AMSC_MINES_FIRES_EXPERT,

	// ^^ Always add new entries here! ^^
};

// Leadership (Officer)
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_LEADERSHIP_ACCURACY_CHANCE, asw_skill_leadership_accuracy_chance, 0, 0.03, "", ASW_MARINE_SKILL_LEADERSHIP, ASW_MARINE_SUBSKILL_LEADERSHIP_ACCURACY_CHANCE );
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_LEADERSHIP_DAMAGE_RESIST, asw_skill_leadership_damage_resist, 0, 0.06, "", ASW_MARINE_SKILL_LEADERSHIP, ASW_MARINE_SUBSKILL_LEADERSHIP_DAMAGE_RESIST );

// Vindicator Damage (Officer)
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_VINDICATOR_DMG, asw_skill_vindicator_dmg, 0, 2.0, "", ASW_MARINE_SKILL_VINDICATOR, ASW_MARINE_SUBSKILL_VINDICATOR_DAMAGE );
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_VINDICATOR_PELLETS, asw_skill_vindicator_pellets, 7, 0, "", ASW_MARINE_SKILL_VINDICATOR, ASW_MARINE_SUBSKILL_VINDICATOR_PELLETS );
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_VINDICATOR_MUZZLE_FLASH, asw_skill_vindicator_muzzle_flash, 1.0, 0.2, "", ASW_MARINE_SKILL_VINDICATOR, ASW_MARINE_SUBSKILL_VINDICATOR_MUZZLE );

// Heavy Weapons (Special Weapons)
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_AUTOGUN, asw_skill_autogun, 0, 1, "", ASW_MARINE_SKILL_AUTOGUN, ASW_MARINE_SUBSKILL_AUTOGUN_DMG );
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_AUTOGUN_MINIGUN_DMG, asw_skill_autogun_minigun_dmg, 0, 1, "", ASW_MARINE_SKILL_AUTOGUN, ASW_MARINE_SUBSKILL_AUTOGUN_MINIGUN_DMG );
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_AUTOGUN_CRYO_SPINUP, asw_skill_autogun_cryo_spinup, 0.25, 0.25, "", ASW_MARINE_SKILL_AUTOGUN, ASW_MARINE_SUBSKILL_AUTOGUN_CRYO_SPINUP );
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_AUTOGUN_CRYO_FREEZE, asw_skill_autogun_cryo_freeze, 0.15, 0, "", ASW_MARINE_SKILL_AUTOGUN, ASW_MARINE_SUBSKILL_AUTOGUN_CRYO_FREEZE );
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_AUTOGUN_CRYO_RADIUS, asw_skill_autogun_cryo_radius, 64, 0, "", ASW_MARINE_SKILL_AUTOGUN, ASW_MARINE_SUBSKILL_AUTOGUN_CRYO_RADIUS );

// Stopping Power (Special Weapons)
// ASW_MARINE_SUBSKILL_STOPPING_POWER is handled per weapon
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_PIERCING, asw_skill_piercing, 0, 0.20, "", ASW_MARINE_SKILL_STOPPING_POWER, ASW_MARINE_SUBSKILL_PIERCING_CHANCE );
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_STOPPING_POWER_AIRBLAST_STRENGTH, asw_skill_stopping_power_airblast_strength, 0, 0, "", ASW_MARINE_SKILL_STOPPING_POWER, ASW_MARINE_SUBSKILL_AIRBLAST_STRENGTH );

// Healing (Medic)
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_HEALING_CHARGES, asw_skill_healing_charges, 4, 1, "", ASW_MARINE_SKILL_HEALING, ASW_MARINE_SUBSKILL_HEALING_CHARGES );
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_SELF_HEALING_CHARGES, asw_skill_self_healing_charges, 2, 0.5, "", ASW_MARINE_SKILL_HEALING, ASW_MARINE_SUBSKILL_SELF_HEALING_CHARGES );
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_HEALING_HPS, asw_skill_healing_hps, 25, 8, "", ASW_MARINE_SKILL_HEALING, ASW_MARINE_SUBSKILL_HEALING_HPS );
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_HEALING_MEDKIT_HPS, asw_skill_healing_medkit_hps, 50, 5, "", ASW_MARINE_SKILL_HEALING, ASW_MARINE_SUBSKILL_HEALING_MEDKIT_HPS );
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_HEALING_GRENADE, asw_skill_healing_grenade, 120, 30, "", ASW_MARINE_SKILL_HEALING, ASW_MARINE_SUBSKILL_HEAL_GRENADE_HEAL_AMOUNT );
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_HEALING_GUN_CHARGES, asw_skill_healing_gun_charges, 40, 10, "", ASW_MARINE_SKILL_HEALING, ASW_MARINE_SUBSKILL_HEAL_GUN_CHARGES );
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_HEALING_GUN, asw_skill_healing_gun, 5, 1, "", ASW_MARINE_SKILL_HEALING, ASW_MARINE_SUBSKILL_HEAL_GUN_HEAL_AMOUNT );
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_HEALING_AMP_GUN_CHARGES, asw_skill_healing_amp_gun_charges, 20, 5, "", ASW_MARINE_SKILL_HEALING, ASW_MARINE_SUBSKILL_HEALAMP_GUN_CHARGES );
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_HEALING_MEDRIFLE_HEALING_CHARGES, asw_skill_healing_medrifle_healing_charges, 50, 0, "", ASW_MARINE_SKILL_HEALING, ASW_MARINE_SUBSKILL_MEDRIFLE_HEALING_CHARGES );

// Infestation Curing (Medic)
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_XENOWOUNDS, asw_skill_xenowounds, 100, -25, "", ASW_MARINE_SKILL_XENOWOUNDS, ASW_MARINE_SUBSKILL_XENOWOUNDS_INFESTATION_CURING );

// Combat Drugs (Medic)
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_DRUGS, asw_skill_drugs, 5, 0.8, "", ASW_MARINE_SKILL_DRUGS, ASW_MARINE_SUBSKILL_STIM_DURATION );
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_DRUGS_HEALAMP, asw_skill_drugs_healamp, 90, 20, "", ASW_MARINE_SKILL_DRUGS, ASW_MARINE_SUBSKILL_HEALAMP_GUN_AMP_CHARGES );

// Hacking (Tech)
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_HACKING_SPEED, asw_skill_hacking_speed, 2.0, 0.1, "", ASW_MARINE_SKILL_HACKING, ASW_MARINE_SUBSKILL_HACKING_SPEED_SCALE );
// TODO: ASW_MARINE_SUBSKILL_HACKING_TUMBLER_COUNT_REDUCTION
// TODO: ASW_MARINE_SUBSKILL_HACKING_ENTRIES_PER_TUMBLER_REDUCTION

// Scanner (Tech)
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_SCANNER, asw_skill_scanner, 600, 150, "", ASW_MARINE_SKILL_SCANNER, ASW_MARINE_SUBSKILL_SCANNER_RANGE );

// Engineering (Tech)
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_ENGINEERING_WELDING, asw_skill_engineering_welding, 0.8, 0.5, "", ASW_MARINE_SKILL_ENGINEERING, ASW_MARINE_SUBSKILL_ENGINEERING_WELDING );
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_ENGINEERING_SENTRY, asw_skill_engineering_sentry, 1.0, 0.25, "", ASW_MARINE_SKILL_ENGINEERING, ASW_MARINE_SUBSKILL_ENGINEERING_SENTRY );
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_ENGINEERING_FIRERATE, asw_skill_engineering_firerate, 0.0, 0.005, "", ASW_MARINE_SKILL_ENGINEERING, ASW_MARINE_SUBSKILL_ENGINEERING_FIRERATE );
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_ENGINEERING_SHIELD_HEALTH, asw_skill_engineering_shield_health, 300, 50, "", ASW_MARINE_SKILL_ENGINEERING, ASW_MARINE_SUBSKILL_ENGINEERING_SHIELD_HEALTH );
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_ENGINEERING_SHIELD_DURATION, asw_skill_engineering_shield_duration, 6, 1, "", ASW_MARINE_SKILL_ENGINEERING, ASW_MARINE_SUBSKILL_ENGINEERING_SHIELD_DURATION );

// Damage Bonus
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_ACCURACY_RIFLE_DMG, asw_skill_accuracy_rifle_dmg, 0, 1, "", ASW_MARINE_SKILL_ACCURACY, ASW_MARINE_SUBSKILL_ACCURACY_RIFLE_DMG );
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_ACCURACY_PRIFLE_DMG, asw_skill_accuracy_prifle_dmg, 0, 1, "", ASW_MARINE_SKILL_ACCURACY, ASW_MARINE_SUBSKILL_ACCURACY_PRIFLE_DMG );
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_ACCURACY_SHOTGUN_DMG, asw_skill_accuracy_shotgun_dmg, 0, 2, "", ASW_MARINE_SKILL_ACCURACY, ASW_MARINE_SUBSKILL_ACCURACY_SHOTGUN_DMG );
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_ACCURACY_RAILGUN_DMG, asw_skill_accuracy_railgun_dmg, 0, 10, "", ASW_MARINE_SKILL_ACCURACY, ASW_MARINE_SUBSKILL_ACCURACY_RAILGUN_DMG );
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_ACCURACY_FLAMER_DMG, asw_skill_accuracy_flamer_dmg, 0, 0.5, "", ASW_MARINE_SKILL_ACCURACY, ASW_MARINE_SUBSKILL_ACCURACY_FLAMER_DMG );
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_ACCURACY_PISTOL_DMG, asw_skill_accuracy_pistol_dmg, 0, 4, "", ASW_MARINE_SKILL_ACCURACY, ASW_MARINE_SUBSKILL_ACCURACY_PISTOL_DMG );
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_ACCURACY_PDW_DMG, asw_skill_accuracy_pdw_dmg, 0, 1.0, "", ASW_MARINE_SKILL_ACCURACY, ASW_MARINE_SUBSKILL_ACCURACY_PDW_DMG );
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_ACCURACY_SNIPER_RIFLE_DMG, asw_skill_accuracy_sniper_rifle_dmg, 0, 20, "", ASW_MARINE_SKILL_ACCURACY, ASW_MARINE_SUBSKILL_ACCURACY_SNIPER_RIFLE_DMG );
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_ACCURACY_TESLA_CANNON_DMG, asw_skill_accuracy_tesla_cannon_dmg, 0, 0.25, "", ASW_MARINE_SKILL_ACCURACY, ASW_MARINE_SUBSKILL_ACCURACY_TESLA_CANNON_DMG );
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_ACCURACY_HEAVY_RIFLE_DMG, asw_skill_accuracy_heavy_rifle_dmg, 0, 2, "", ASW_MARINE_SKILL_ACCURACY, ASW_MARINE_SUBSKILL_ACCURACY_HEAVY_RIFLE_DMG );
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_ACCURACY_MEDRIFLE_DMG, asw_skill_accuracy_medrifle_dmg, 0, 2, "", ASW_MARINE_SKILL_ACCURACY, ASW_MARINE_SUBSKILL_ACCURACY_MEDRIFLE_DMG );
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_MUZZLE_FLASH, asw_skill_muzzle_flash, 1.0, 0.2, "", ASW_MARINE_SKILL_ACCURACY, ASW_MARINE_SUBSKILL_ACCURACY_MUZZLE );
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_ACCURACY_BULLDOG_DMG, asw_skill_accuracy_deagle_dmg, 0, 0, "", ASW_MARINE_SKILL_ACCURACY, ASW_MARINE_SUBSKILL_ACCURACY_DEAGLE_DMG );
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_ACCURACY_AR2_DMG, asw_skill_accuracy_ar2_dmg, 0, 2, "", ASW_MARINE_SKILL_ACCURACY, ASW_MARINE_SUBSKILL_ACCURACY_AR2_DMG );
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_ACCURACY_DEVASTATOR_DMG, asw_skill_accuracy_devastator_dmg, 0, 1, "", ASW_MARINE_SKILL_ACCURACY, ASW_MARINE_SUBSKILL_ACCURACY_DEVASTATOR_DMG );
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_ACCURACY_50CALMG_DMG, asw_skill_accuracy_50calmg_dmg, 0, 1, "", ASW_MARINE_SKILL_ACCURACY, ASW_MARINE_SUBSKILL_ACCURACY_50CALMG_DMG );
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_ACCURACY_MINING_LASER_DMG, asw_skill_accuracy_mining_laser_dmg, 0, 1, "", ASW_MARINE_SKILL_ACCURACY, ASW_MARINE_SUBSKILL_ACCURACY_MINING_LASER_DMG );
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_ACCURACY_SHIELD_RIFLE_DMG, asw_skill_accuracy_shield_rifle_dmg, 0, 2, "", ASW_MARINE_SKILL_ACCURACY, ASW_MARINE_SUBSKILL_ACCURACY_SHIELD_RIFLE_DMG );
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_ACCURACY_CRYO_DMG, asw_skill_accuracy_cryo_dmg, 0, 0.5, "", ASW_MARINE_SKILL_ACCURACY, ASW_MARINE_SUBSKILL_ACCURACY_CRYO_DMG );
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_ACCURACY_SHIELD_DAMAGE, asw_skill_accuracy_shield_damage, 30, 20, "", ASW_MARINE_SKILL_ACCURACY, ASW_MARINE_SUBSKILL_ACCURACY_SHIELD_DAMAGE );

// Explosives Bonus
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_GRENADES_RADIUS, asw_skill_grenades_radius, 280, 20, "", ASW_MARINE_SKILL_GRENADES, ASW_MARINE_SUBSKILL_GRENADE_RADIUS );
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_GRENADES_DMG, asw_skill_grenades_dmg, 128, 20, "", ASW_MARINE_SKILL_GRENADES, ASW_MARINE_SUBSKILL_GRENADE_DMG );
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_GRENADES_INCENDIARY_DMG, asw_skill_grenades_incendiary_dmg, 20, 0, "", ASW_MARINE_SKILL_GRENADES, ASW_MARINE_SUBSKILL_GRENADE_INCENDIARY_DMG );
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_GRENADES_CLUSTER_DMG, asw_skill_grenades_cluster_dmg, 0, 10, "", ASW_MARINE_SKILL_GRENADES, ASW_MARINE_SUBSKILL_GRENADE_CLUSTER_DMG );
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_GRENADES_CLUSTERS, asw_skill_grenades_clusters, 0, 1, "", ASW_MARINE_SKILL_GRENADES, ASW_MARINE_SUBSKILL_GRENADE_CLUSTERS );
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_GRENADES_FLECHETTE_DMG, asw_skill_grenades_flechette_dmg, 10, 1, "", ASW_MARINE_SKILL_GRENADES, ASW_MARINE_SUBSKILL_GRENADE_FLECHETTE_DMG );
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_GRENADES_HORNET_DMG, asw_skill_grenades_hornet_dmg, 50, 1, "", ASW_MARINE_SKILL_GRENADES, ASW_MARINE_SUBSKILL_GRENADE_HORNET_DMG );
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_GRENADES_HORNET_COUNT, asw_skill_grenades_hornet_count, 8, 0, "", ASW_MARINE_SKILL_GRENADES, ASW_MARINE_SUBSKILL_GRENADE_HORNET_COUNT );
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_GRENADES_HORNET_INTERVAL, asw_skill_grenades_hornet_interval, 0.09, 0, "", ASW_MARINE_SKILL_GRENADES, ASW_MARINE_SUBSKILL_GRENADE_HORNET_INTERVAL );
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_GRENADES_FREEZE_RADIUS, asw_skill_grenades_freeze_radius, 210, 0, "", ASW_MARINE_SKILL_GRENADES, ASW_MARINE_SUBSKILL_GRENADE_FREEZE_RADIUS );
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_GRENADES_FREEZE_DURATION, asw_skill_grenades_freeze_duration, 3.0, 0.3, "", ASW_MARINE_SKILL_GRENADES, ASW_MARINE_SUBSKILL_GRENADE_FREEZE_DURATION );
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_GRENADES_SMART_COUNT, asw_skill_grenades_smart_count, 32, 0, "", ASW_MARINE_SKILL_GRENADES, ASW_MARINE_SUBSKILL_GRENADE_SMART_BOMB_COUNT );
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_GRENADES_SMART_INTERVAL, asw_skill_grenades_smart_interval, 0.09, 0, "", ASW_MARINE_SKILL_GRENADES, ASW_MARINE_SUBSKILL_GRENADE_SMART_BOMB_INTERVAL );
GameRulesConVarSkill5_2( s_ASWMarineSkillsConVars, AMSC_LASER_MINES, asw_skill_laser_mines, 1, 2, 3, "number of laser mines to deploy per Explosives Bonus skill", ASW_MARINE_SKILL_GRENADES, ASW_MARINE_SUBSKILL_GRENADE_LASER_MINES );
GameRulesConVarSkill5_2( s_ASWMarineSkillsConVars, AMSC_MINES_FIRES, asw_skill_mines_fires, 1, 2, 3, "number of additional incendiary mine fires on each side per Explosives Bonus skill", ASW_MARINE_SKILL_GRENADES, ASW_MARINE_SUBSKILL_GRENADE_MINES_FIRES );
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_MINES_DURATION, asw_skill_mines_duration, 10.0, 5.0, "", ASW_MARINE_SKILL_GRENADES, ASW_MARINE_SUBSKILL_GRENADE_MINES_DURATION );

// Health Bonus
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_HEALTH, asw_skill_health, 80, 15, "", ASW_MARINE_SKILL_HEALTH, ASW_MARINE_SUBSKILL_HEALTH );

// Melee Damage Bonus
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_MELEE_DMG, asw_skill_melee_dmg, 30, 6, "", ASW_MARINE_SKILL_MELEE, ASW_MARINE_SUBSKILL_MELEE_DMG );
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_MELEE_FORCE, asw_skill_melee_force, 10, 1.0, "", ASW_MARINE_SKILL_MELEE, ASW_MARINE_SUBSKILL_MELEE_FORCE );
// TODO: ASW_MARINE_SUBSKILL_MELEE_FLINCH
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_MELEE_SPEED, asw_skill_melee_speed, 1.0, 0.1, "", ASW_MARINE_SKILL_MELEE, ASW_MARINE_SUBSKILL_MELEE_SPEED );

// Reload Speed Bonus
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_RELOADING, asw_skill_reloading, 1.4, -0.14, "", ASW_MARINE_SKILL_RELOADING, ASW_MARINE_SUBSKILL_RELOADING_SPEED_SCALE );
// TODO: ASW_MARINE_SUBSKILL_RELOADING_SOUND
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_RELOADING_FAST, asw_skill_reloading_fast, 1.0, 0.05, "", ASW_MARINE_SKILL_RELOADING, ASW_MARINE_SUBSKILL_RELOADING_FAST_WIDTH_SCALE );

// Speed Bonus
GameRulesConVarSkill( s_ASWMarineSkillsConVars, AMSC_AGILITY_MOVESPEED, asw_skill_agility_movespeed, 290, 10, "", ASW_MARINE_SKILL_AGILITY, ASW_MARINE_SUBSKILL_AGILITY_MOVE_SPEED );

CASW_Marine_Skills::CASW_Marine_Skills()
{
#ifndef CLIENT_DLL
	m_hLastSkillMarine = NULL;
#endif
}

// accessor functions to get at any game variables that are based on a skill
float CASW_Marine_Skills::GetSkillBasedValueByMarine( CASW_Marine *pMarine, ASW_Skill iSkillIndex, int iSubSkill )
{
	if ( !pMarine )
		return GetSkillBasedValue( NULL, iSkillIndex, iSubSkill, 0 );

	CASW_Marine_Profile *pProfile = pMarine->GetMarineProfile();
	Assert( pProfile );
	if ( !pProfile )
		return GetSkillBasedValue( NULL, iSkillIndex, iSubSkill, 0 );

	return GetSkillBasedValue( pProfile, iSkillIndex, iSubSkill );
}

float CASW_Marine_Skills::GetSkillBasedValueByMarineResource( CASW_Marine_Resource *pMarineResource, ASW_Skill iSkillIndex, int iSubSkill )
{
	Assert( pMarineResource );
	if ( !pMarineResource )
		return GetSkillBasedValue( NULL, iSkillIndex, iSubSkill, 0 );

	return GetSkillBasedValue( pMarineResource->GetProfile(), iSkillIndex, iSubSkill );
}

float CASW_Marine_Skills::GetSkillBasedValue( CASW_Marine_Profile *pProfile, ASW_Skill iSkillIndex, int iSubSkill, int iSkillPoints )
{
	if ( iSkillPoints == -1 )
	{
		iSkillPoints = GetSkillPoints( pProfile, iSkillIndex );
	}

	Assert( iSkillPoints >= 0 );
	Assert( iSkillPoints <= GetMaxSkillPoints( iSkillIndex ) );

	switch ( iSkillIndex )
	{
	case ASW_MARINE_SKILL_LEADERSHIP:
		if ( iSkillPoints <= 0 )
			return 0.0f;

		switch ( iSubSkill )
		{
		case ASW_MARINE_SUBSKILL_LEADERSHIP_ACCURACY_CHANCE:
			return asw_skill_leadership_accuracy_chance_base.GetFloat() + asw_skill_leadership_accuracy_chance_step.GetFloat() * iSkillPoints;
		case ASW_MARINE_SUBSKILL_LEADERSHIP_DAMAGE_RESIST:
			return asw_skill_leadership_damage_resist_base.GetFloat() + asw_skill_leadership_damage_resist_step.GetFloat() * iSkillPoints;
		default:
			Assert( 0 );
			return 0.0f;
		}
	case ASW_MARINE_SKILL_VINDICATOR:
		switch ( iSubSkill )
		{
		case ASW_MARINE_SUBSKILL_VINDICATOR_DAMAGE:
			return asw_skill_vindicator_dmg_base.GetFloat() + asw_skill_vindicator_dmg_step.GetFloat() * iSkillPoints;
		case ASW_MARINE_SUBSKILL_VINDICATOR_PELLETS:
			return asw_skill_vindicator_pellets_base.GetFloat() + asw_skill_vindicator_pellets_step.GetFloat() * iSkillPoints;
		case ASW_MARINE_SUBSKILL_VINDICATOR_MUZZLE:
			return asw_skill_vindicator_muzzle_flash_base.GetFloat() + asw_skill_vindicator_muzzle_flash_step.GetFloat() * iSkillPoints;
		default:
			Assert( 0 );
			return 0.0f;
		}
	case ASW_MARINE_SKILL_AUTOGUN:
		switch ( iSubSkill )
		{
		case ASW_MARINE_SUBSKILL_AUTOGUN_DMG:
			return asw_skill_autogun_base.GetFloat() + asw_skill_autogun_step.GetFloat() * iSkillPoints;
		case ASW_MARINE_SUBSKILL_AUTOGUN_MUZZLE:
			return asw_skill_muzzle_flash_base.GetFloat() + asw_skill_muzzle_flash_step.GetFloat() * iSkillPoints;
		case ASW_MARINE_SUBSKILL_AUTOGUN_MINIGUN_DMG:
			return asw_skill_autogun_minigun_dmg_base.GetFloat() + asw_skill_autogun_minigun_dmg_step.GetFloat() * iSkillPoints;
		case ASW_MARINE_SUBSKILL_AUTOGUN_CRYO_SPINUP:
			return asw_skill_autogun_cryo_spinup_base.GetFloat() + asw_skill_autogun_cryo_spinup_step.GetFloat() * iSkillPoints;
		case ASW_MARINE_SUBSKILL_AUTOGUN_CRYO_FREEZE:
			return asw_skill_autogun_cryo_freeze_base.GetFloat() + asw_skill_autogun_cryo_freeze_step.GetFloat() * iSkillPoints;
		case ASW_MARINE_SUBSKILL_AUTOGUN_CRYO_RADIUS:
			return asw_skill_autogun_cryo_radius_base.GetFloat() + asw_skill_autogun_cryo_radius_step.GetFloat() * iSkillPoints;
		default:
			Assert( 0 );
			return 0.0f;
		}
	case ASW_MARINE_SKILL_STOPPING_POWER_OBSOLETE:
		Assert( iSubSkill == 0 );
		return iSkillPoints;
	case ASW_MARINE_SKILL_STOPPING_POWER:
		switch ( iSubSkill )
		{
		case ASW_MARINE_SUBSKILL_STOPPING_POWER:
			return iSkillPoints;
		case ASW_MARINE_SUBSKILL_PIERCING_CHANCE:
			return asw_skill_piercing_base.GetFloat() + asw_skill_piercing_step.GetFloat() * iSkillPoints;
		case ASW_MARINE_SUBSKILL_AIRBLAST_STRENGTH:
			return asw_skill_stopping_power_airblast_strength_base.GetFloat() + asw_skill_stopping_power_airblast_strength_step.GetFloat() * iSkillPoints;
		}
	case ASW_MARINE_SKILL_HEALING:
		switch ( iSubSkill )
		{
		case ASW_MARINE_SUBSKILL_HEALING_CHARGES:
			return asw_skill_healing_charges_base.GetFloat() + asw_skill_healing_charges_step.GetFloat() * iSkillPoints;
		case ASW_MARINE_SUBSKILL_SELF_HEALING_CHARGES:
			return asw_skill_self_healing_charges_base.GetFloat() + asw_skill_self_healing_charges_step.GetFloat() * iSkillPoints;
		case ASW_MARINE_SUBSKILL_HEALING_HPS:
			return asw_skill_healing_hps_base.GetFloat() + asw_skill_healing_hps_step.GetFloat() * iSkillPoints;
		case ASW_MARINE_SUBSKILL_HEAL_GRENADE_HEAL_AMOUNT:
			return asw_skill_healing_grenade_base.GetFloat() + asw_skill_healing_grenade_step.GetFloat() * iSkillPoints;
		case ASW_MARINE_SUBSKILL_HEAL_GUN_CHARGES:
			return asw_skill_healing_gun_charges_base.GetFloat() + asw_skill_healing_gun_charges_step.GetFloat() * iSkillPoints;
		case ASW_MARINE_SUBSKILL_HEAL_GUN_HEAL_AMOUNT:
			return asw_skill_healing_gun_base.GetFloat() + asw_skill_healing_gun_step.GetFloat() * iSkillPoints;
		case ASW_MARINE_SUBSKILL_HEALAMP_GUN_CHARGES:
			return asw_skill_healing_amp_gun_charges_base.GetFloat() + asw_skill_healing_amp_gun_charges_step.GetFloat() * iSkillPoints;
		case ASW_MARINE_SUBSKILL_HEALING_MEDKIT_HPS:
			return asw_skill_healing_medkit_hps_base.GetFloat() + asw_skill_healing_medkit_hps_step.GetFloat() * iSkillPoints;
		case ASW_MARINE_SUBSKILL_MEDRIFLE_HEALING_CHARGES:
			return asw_skill_healing_medrifle_healing_charges_base.GetFloat() + asw_skill_healing_medrifle_healing_charges_step.GetFloat() * iSkillPoints;
		default:
			Assert( 0 );
			return 0.0f;
		}
	case ASW_MARINE_SKILL_XENOWOUNDS:
		Assert( iSubSkill == ASW_MARINE_SUBSKILL_XENOWOUNDS_INFESTATION_CURING );
		return asw_skill_xenowounds_base.GetFloat() + asw_skill_xenowounds_step.GetFloat() * iSkillPoints;
	case ASW_MARINE_SKILL_DRUGS:
		switch ( iSubSkill )
		{
		case ASW_MARINE_SUBSKILL_STIM_DURATION:
			return asw_skill_drugs_base.GetFloat() + asw_skill_drugs_step.GetFloat() * iSkillPoints;
		case ASW_MARINE_SUBSKILL_HEALAMP_GUN_AMP_CHARGES:
			return asw_skill_drugs_healamp_base.GetFloat() + asw_skill_drugs_healamp_step.GetFloat() * iSkillPoints;
		default:
			Assert( 0 );
			return 0.0f;
		}
	case ASW_MARINE_SKILL_HACKING:
		switch ( iSubSkill )
		{
		case ASW_MARINE_SUBSKILL_HACKING_SPEED_SCALE:
			return asw_skill_hacking_speed_base.GetFloat() + asw_skill_hacking_speed_step.GetFloat() * iSkillPoints;
		case ASW_MARINE_SUBSKILL_HACKING_TUMBLER_COUNT_REDUCTION:
			if ( iSkillPoints >= 3 )
				return 1;
			return 0;
		case ASW_MARINE_SUBSKILL_HACKING_ENTRIES_PER_TUMBLER_REDUCTION:
			if ( iSkillPoints >= 4 )
				return 4;
			if ( iSkillPoints >= 2 )
				return 2;
			return 0;
		default:
			Assert( 0 );
			return 0.0f;
		}
	case ASW_MARINE_SKILL_SCANNER:
		Assert( iSubSkill == ASW_MARINE_SUBSKILL_SCANNER_RANGE );
		return asw_skill_scanner_base.GetFloat() + asw_skill_scanner_step.GetFloat() * iSkillPoints;
	case ASW_MARINE_SKILL_ENGINEERING:
		if ( iSkillPoints <= 0 )
			return 0.0f;

		switch ( iSubSkill )
		{
		case ASW_MARINE_SUBSKILL_ENGINEERING_WELDING:
			return asw_skill_engineering_welding_base.GetFloat() + asw_skill_engineering_welding_step.GetFloat() * iSkillPoints;
		case ASW_MARINE_SUBSKILL_ENGINEERING_SENTRY:
			return asw_skill_engineering_sentry_base.GetFloat() + asw_skill_engineering_sentry_step.GetFloat() * iSkillPoints;
		case ASW_MARINE_SUBSKILL_ENGINEERING_FIRERATE:
			return asw_skill_engineering_firerate_base.GetFloat() + asw_skill_engineering_firerate_step.GetFloat() * iSkillPoints;
		case ASW_MARINE_SUBSKILL_ENGINEERING_SHIELD_HEALTH:
			return asw_skill_engineering_shield_health_base.GetFloat() + asw_skill_engineering_shield_health_step.GetFloat() * iSkillPoints;
		case ASW_MARINE_SUBSKILL_ENGINEERING_SHIELD_DURATION:
			return asw_skill_engineering_shield_duration_base.GetFloat() + asw_skill_engineering_shield_duration_step.GetFloat() * iSkillPoints;
		default:
			Assert( 0 );
			return 0.0f;
		}
	case ASW_MARINE_SKILL_ACCURACY:
		switch ( iSubSkill )
		{
		case ASW_MARINE_SUBSKILL_ACCURACY_RIFLE_DMG:
			return asw_skill_accuracy_rifle_dmg_base.GetFloat() + asw_skill_accuracy_rifle_dmg_step.GetFloat() * iSkillPoints;
		case ASW_MARINE_SUBSKILL_ACCURACY_PRIFLE_DMG:
			return asw_skill_accuracy_prifle_dmg_base.GetFloat() + asw_skill_accuracy_prifle_dmg_step.GetFloat() * iSkillPoints;
		case ASW_MARINE_SUBSKILL_ACCURACY_SHOTGUN_DMG:
			return asw_skill_accuracy_shotgun_dmg_base.GetFloat() + asw_skill_accuracy_shotgun_dmg_step.GetFloat() * iSkillPoints;
		case ASW_MARINE_SUBSKILL_ACCURACY_RAILGUN_DMG:
			return asw_skill_accuracy_railgun_dmg_base.GetFloat() + asw_skill_accuracy_railgun_dmg_step.GetFloat() * iSkillPoints;
		case ASW_MARINE_SUBSKILL_ACCURACY_FLAMER_DMG:
			return asw_skill_accuracy_flamer_dmg_base.GetFloat() + asw_skill_accuracy_flamer_dmg_step.GetFloat() * iSkillPoints;
		case ASW_MARINE_SUBSKILL_ACCURACY_PISTOL_DMG:
			return asw_skill_accuracy_pistol_dmg_base.GetFloat() + asw_skill_accuracy_pistol_dmg_step.GetFloat() * iSkillPoints;
		case ASW_MARINE_SUBSKILL_ACCURACY_PDW_DMG:
			return asw_skill_accuracy_pdw_dmg_base.GetFloat() + asw_skill_accuracy_pdw_dmg_step.GetFloat() * iSkillPoints;
		case ASW_MARINE_SUBSKILL_ACCURACY_SNIPER_RIFLE_DMG:
			return asw_skill_accuracy_sniper_rifle_dmg_base.GetFloat() + asw_skill_accuracy_sniper_rifle_dmg_step.GetFloat() * iSkillPoints;
		case ASW_MARINE_SUBSKILL_ACCURACY_TESLA_CANNON_DMG:
			return asw_skill_accuracy_tesla_cannon_dmg_base.GetFloat() + asw_skill_accuracy_tesla_cannon_dmg_step.GetFloat() * iSkillPoints;
		case ASW_MARINE_SUBSKILL_ACCURACY_HEAVY_RIFLE_DMG:
			return asw_skill_accuracy_heavy_rifle_dmg_base.GetFloat() + asw_skill_accuracy_heavy_rifle_dmg_step.GetFloat() * iSkillPoints;
		case ASW_MARINE_SUBSKILL_ACCURACY_MEDRIFLE_DMG:
			return asw_skill_accuracy_medrifle_dmg_base.GetFloat() + asw_skill_accuracy_medrifle_dmg_step.GetFloat() * iSkillPoints;
		case ASW_MARINE_SUBSKILL_ACCURACY_MUZZLE:
			return asw_skill_muzzle_flash_base.GetFloat() + asw_skill_muzzle_flash_step.GetFloat() * iSkillPoints;
		case ASW_MARINE_SUBSKILL_ACCURACY_DEAGLE_DMG:
			return asw_skill_accuracy_deagle_dmg_base.GetFloat() + asw_skill_accuracy_deagle_dmg_step.GetFloat() * iSkillPoints;
		case ASW_MARINE_SUBSKILL_ACCURACY_AR2_DMG:
			return asw_skill_accuracy_ar2_dmg_base.GetFloat() + asw_skill_accuracy_ar2_dmg_step.GetFloat() * iSkillPoints;
		case ASW_MARINE_SUBSKILL_ACCURACY_DEVASTATOR_DMG:
			return asw_skill_accuracy_devastator_dmg_base.GetFloat() + asw_skill_accuracy_devastator_dmg_step.GetFloat() * iSkillPoints;
		case ASW_MARINE_SUBSKILL_ACCURACY_50CALMG_DMG:
			return asw_skill_accuracy_50calmg_dmg_base.GetFloat() + asw_skill_accuracy_50calmg_dmg_step.GetFloat() * iSkillPoints;
		case ASW_MARINE_SUBSKILL_ACCURACY_MINING_LASER_DMG:
			return asw_skill_accuracy_mining_laser_dmg_base.GetFloat() + asw_skill_accuracy_mining_laser_dmg_step.GetFloat() * iSkillPoints;
		case ASW_MARINE_SUBSKILL_ACCURACY_SHIELD_RIFLE_DMG:
			return asw_skill_accuracy_shield_rifle_dmg_base.GetFloat() + asw_skill_accuracy_shield_rifle_dmg_step.GetFloat() * iSkillPoints;
		case ASW_MARINE_SUBSKILL_ACCURACY_CRYO_DMG:
			return asw_skill_accuracy_cryo_dmg_base.GetFloat() + asw_skill_accuracy_cryo_dmg_step.GetFloat() * iSkillPoints;
		case ASW_MARINE_SUBSKILL_ACCURACY_SHIELD_DAMAGE:
			return asw_skill_accuracy_shield_damage_base.GetFloat() + asw_skill_accuracy_shield_damage_step.GetFloat() * iSkillPoints;
		default:
			Assert( 0 );
			return 0.0f;
		}
	case ASW_MARINE_SKILL_GRENADES:
		switch ( iSubSkill )
		{
		case ASW_MARINE_SUBSKILL_GRENADE_RADIUS:
			return asw_skill_grenades_radius_base.GetFloat() + asw_skill_grenades_radius_step.GetFloat() * iSkillPoints;
		case ASW_MARINE_SUBSKILL_GRENADE_DMG:
			return asw_skill_grenades_dmg_base.GetFloat() + asw_skill_grenades_dmg_step.GetFloat() * iSkillPoints;
		case ASW_MARINE_SUBSKILL_GRENADE_INCENDIARY_DMG:
			return asw_skill_grenades_incendiary_dmg_base.GetFloat() + asw_skill_grenades_incendiary_dmg_step.GetFloat() * iSkillPoints;
		case ASW_MARINE_SUBSKILL_GRENADE_CLUSTER_DMG:
			return asw_skill_grenades_cluster_dmg_base.GetFloat() + asw_skill_grenades_cluster_dmg_step.GetFloat() * iSkillPoints;
		case ASW_MARINE_SUBSKILL_GRENADE_CLUSTERS:
			return asw_skill_grenades_clusters_base.GetFloat() + asw_skill_grenades_clusters_step.GetFloat() * iSkillPoints;
		case ASW_MARINE_SUBSKILL_GRENADE_FLECHETTE_DMG:
			return asw_skill_grenades_flechette_dmg_base.GetFloat() + asw_skill_grenades_flechette_dmg_step.GetFloat() * iSkillPoints;
		case ASW_MARINE_SUBSKILL_GRENADE_HORNET_DMG:
			return asw_skill_grenades_hornet_dmg_base.GetFloat() + asw_skill_grenades_hornet_dmg_step.GetFloat() * iSkillPoints;
		case ASW_MARINE_SUBSKILL_GRENADE_HORNET_COUNT:
			return asw_skill_grenades_hornet_count_base.GetFloat() + asw_skill_grenades_hornet_count_step.GetFloat() * iSkillPoints;
		case ASW_MARINE_SUBSKILL_GRENADE_HORNET_INTERVAL:
			return asw_skill_grenades_hornet_interval_base.GetFloat() + asw_skill_grenades_hornet_interval_step.GetFloat() * iSkillPoints;
		case ASW_MARINE_SUBSKILL_GRENADE_FREEZE_RADIUS:
			return asw_skill_grenades_freeze_radius_base.GetFloat() + asw_skill_grenades_freeze_radius_step.GetFloat() * iSkillPoints;
		case ASW_MARINE_SUBSKILL_GRENADE_FREEZE_DURATION:
			return asw_skill_grenades_freeze_duration_base.GetFloat() + asw_skill_grenades_freeze_duration_step.GetFloat() * iSkillPoints;
		case ASW_MARINE_SUBSKILL_GRENADE_SMART_BOMB_COUNT:
			return asw_skill_grenades_smart_count_base.GetFloat() + asw_skill_grenades_smart_count_step.GetFloat() * iSkillPoints;
		case ASW_MARINE_SUBSKILL_GRENADE_SMART_BOMB_INTERVAL:
			return asw_skill_grenades_smart_interval_base.GetFloat() + asw_skill_grenades_smart_interval_step.GetFloat() * iSkillPoints;
		case ASW_MARINE_SUBSKILL_GRENADE_LASER_MINES:
			if ( iSkillPoints >= 4 )
				return asw_skill_laser_mines_expert.GetFloat();
			if ( iSkillPoints >= 2 )
				return asw_skill_laser_mines_moderate.GetFloat();
			return asw_skill_laser_mines_base.GetFloat();
		case ASW_MARINE_SUBSKILL_GRENADE_MINES_FIRES:
			if ( iSkillPoints >= 4 )
				return asw_skill_mines_fires_expert.GetFloat();
			if ( iSkillPoints >= 2 )
				return asw_skill_mines_fires_moderate.GetFloat();
			return asw_skill_mines_fires_base.GetFloat();
		case ASW_MARINE_SUBSKILL_GRENADE_MINES_DURATION:
			return asw_skill_mines_duration_base.GetFloat() + asw_skill_mines_duration_step.GetFloat() * iSkillPoints;
		default:
			Assert( 0 );
			return 0.0f;
		}
	case ASW_MARINE_SKILL_HEALTH:
		Assert( iSubSkill == ASW_MARINE_SUBSKILL_HEALTH );
		return asw_skill_health_base.GetFloat() + asw_skill_health_step.GetFloat() * iSkillPoints;
	case ASW_MARINE_SKILL_MELEE:
		switch ( iSubSkill )
		{
		case ASW_MARINE_SUBSKILL_MELEE_DMG:
			return asw_skill_melee_dmg_base.GetFloat() + asw_skill_melee_dmg_step.GetFloat() * iSkillPoints;
		case ASW_MARINE_SUBSKILL_MELEE_FORCE:
			return asw_skill_melee_force_base.GetFloat() + asw_skill_melee_force_step.GetFloat() * iSkillPoints;
		case ASW_MARINE_SUBSKILL_MELEE_FLINCH:
			// return a different length flinch (large, small, tiny) based on our skill points
			if ( iSkillPoints >= 4 )
				return 2;
			if ( iSkillPoints >= 2 )
				return 1;
			return 0;
		case ASW_MARINE_SUBSKILL_MELEE_SPEED:
			return asw_skill_melee_speed_base.GetFloat() + asw_skill_melee_speed_step.GetFloat() * iSkillPoints;
		default:
			Assert( 0 );
			return 0.0f;
		}
	case ASW_MARINE_SKILL_RELOADING:
		switch ( iSubSkill )
		{
		case ASW_MARINE_SUBSKILL_RELOADING_SPEED_SCALE:
			return asw_skill_reloading_base.GetFloat() + asw_skill_reloading_step.GetFloat() * iSkillPoints;
		case ASW_MARINE_SUBSKILL_RELOADING_SOUND:
			return iSkillPoints;
		case ASW_MARINE_SUBSKILL_RELOADING_FAST_WIDTH_SCALE:
			return asw_skill_reloading_fast_base.GetFloat() + asw_skill_reloading_fast_step.GetFloat() * iSkillPoints;
		default:
			Assert( 0 );
			return 0.0f;
		}
	case ASW_MARINE_SKILL_AGILITY:
		Assert( iSubSkill == ASW_MARINE_SUBSKILL_AGILITY_MOVE_SPEED );
		return asw_skill_agility_movespeed_base.GetFloat() + asw_skill_agility_movespeed_step.GetFloat() * iSkillPoints;
	default:
		Assert( !"Unknown marine skill" );
		return 0;
	}
}

float CASW_Marine_Skills::GetBestSkillValue( ASW_Skill iSkillIndex, int iSubSkill )
{
	CASW_Game_Resource *pGameResource = ASWGameResource();
	Assert( pGameResource );
	Assert( MarineProfileList() );
	if ( !pGameResource || !MarineProfileList() )
		return 0;

	float fBestSkill = -1;

	// find the live marine with the highest value for this skill
	for ( int i = 0; i < pGameResource->GetMaxMarineResources(); i++ )
	{
		CASW_Marine_Resource *pMR = pGameResource->GetMarineResource( i );
		if ( pMR && pMR->GetHealthPercent() > 0 && pMR->IsAlive() && pMR->GetProfile() )
		{
			float skill = GetSkillBasedValueByMarineResource( pMR, iSkillIndex, iSubSkill );
			if ( skill > fBestSkill )
				fBestSkill = skill;
		}
	}

	return fBestSkill;
}

float CASW_Marine_Skills::GetHighestSkillValueNearby( const Vector &pos, float MaxDistance, ASW_Skill iSkillIndex, int iSubSkill )
{
#ifndef CLIENT_DLL
	m_hLastSkillMarine = NULL;
#endif
	CASW_Game_Resource *pGameResource = ASWGameResource();
	Assert( pGameResource );
	Assert( MarineProfileList() );
	if ( !pGameResource || !MarineProfileList() )
		return 0;

	float fBestSkill = -1;

	// find the live marine with the highest value for this skill
	for ( int i = 0; i < pGameResource->GetMaxMarineResources(); i++ )
	{
		CASW_Marine_Resource *pMR = pGameResource->GetMarineResource( i );
		if ( pMR && pMR->GetHealthPercent() > 0 && pMR->IsAlive() && pMR->GetProfile() && pMR->GetMarineEntity() )
		{
			// check he's near enough
			float dist = pMR->GetMarineEntity()->GetAbsOrigin().DistTo( pos );
			if ( dist > MaxDistance )
				continue;

			float skill = GetSkillBasedValueByMarineResource( pMR, iSkillIndex, iSubSkill );
			if ( skill > fBestSkill )
			{
				fBestSkill = skill;
#ifndef CLIENT_DLL
				m_hLastSkillMarine = pMR->GetMarineEntity();
#endif
			}
		}
	}

	return fBestSkill;
}

float CASW_Marine_Skills::GetLowestSkillValueNearby( const Vector &pos, float MaxDistance, ASW_Skill iSkillIndex, int iSubSkill )
{
#ifndef CLIENT_DLL
	m_hLastSkillMarine = NULL;
#endif
	CASW_Game_Resource *pGameResource = ASWGameResource();
	Assert( pGameResource );
	Assert( MarineProfileList() );
	if ( !pGameResource || !MarineProfileList() )
		return 0;

	float fBestSkill = -1;

	// find the live marine with the highest value for this skill
	for ( int i = 0; i < pGameResource->GetMaxMarineResources(); i++ )
	{
		CASW_Marine_Resource *pMR = pGameResource->GetMarineResource( i );
		if ( pMR && pMR->GetHealthPercent() > 0 && pMR->IsAlive() && pMR->GetProfile() && pMR->GetMarineEntity() )
		{
			// check he's near enough
			float dist = pMR->GetMarineEntity()->GetAbsOrigin().DistTo( pos );
			if ( dist > MaxDistance )
				continue;

			float skill = GetSkillBasedValueByMarineResource( pMR, iSkillIndex, iSubSkill );
			if ( fBestSkill == -1 || skill < fBestSkill )
			{
#ifndef CLIENT_DLL
				m_hLastSkillMarine = pMR->GetMarineEntity();
#endif
				fBestSkill = skill;
			}
		}
	}

	return fBestSkill;
}

#ifdef GAME_DLL
CASW_Marine *CASW_Marine_Skills::CheckSkillChanceNearby( CBaseEntity *pAlly, const Vector &pos, float MaxDistance, ASW_Skill iSkillIndex, int iSubSkill )
{
	CASW_Game_Resource *pGameResource = ASWGameResource();
	Assert( pGameResource );
	Assert( MarineProfileList() );
	if ( !pGameResource || !MarineProfileList() )
		return NULL;

	MaxDistance = Square( MaxDistance );

	// find the live marine with the highest value for this skill
	for ( int i = 0; i < pGameResource->GetMaxMarineResources(); i++ )
	{
		CASW_Marine_Resource *pMR = pGameResource->GetMarineResource( i );
		if ( pMR && pMR->GetHealthPercent() > 0 && pMR->IsAlive() && pMR->GetProfile() && pMR->GetMarineEntity() )
		{
			if ( pAlly && pMR->GetMarineEntity()->IRelationType( pAlly ) != D_LI )
				continue;

			// check he's near enough
			float dist = pMR->GetMarineEntity()->GetAbsOrigin().DistToSqr( pos );
			if ( dist > MaxDistance )
				continue;

			float skill = GetSkillBasedValueByMarineResource( pMR, iSkillIndex, iSubSkill );
			if ( skill > 0 && skill > RandomFloat() )
			{
				return pMR->GetMarineEntity();
			}
		}
	}

	return NULL;
}
#endif

static const char *const s_szSkillImageName[ASW_NUM_MARINE_SKILLS] =
{
#define ENUM_ITEM( name, icon, title, desc, max ) icon,
	ASW_SKILL_ENUM
#undef ENUM_ITEM
};

static const char *const s_szSkillName[ASW_NUM_MARINE_SKILLS] =
{
#define ENUM_ITEM( name, icon, title, desc, max ) title,
	ASW_SKILL_ENUM
#undef ENUM_ITEM
};

static const char *const s_szSkillDescription[ASW_NUM_MARINE_SKILLS] =
{
#define ENUM_ITEM( name, icon, title, desc, max ) desc,
	ASW_SKILL_ENUM
#undef ENUM_ITEM
};

static const int s_nMaxSkillPoints[ASW_NUM_MARINE_SKILLS] =
{
#define ENUM_ITEM( name, icon, title, desc, max ) max,
	ASW_SKILL_ENUM
#undef ENUM_ITEM
};

#ifdef CLIENT_DLL
const char *CASW_Marine_Skills::GetSkillImage( ASW_Skill nSkillIndex )
{
	Assert( nSkillIndex >= 0 && nSkillIndex < ASW_NUM_MARINE_SKILLS );
	if ( nSkillIndex < 0 || nSkillIndex >= ASW_NUM_MARINE_SKILLS )
		return "";

	return s_szSkillImageName[nSkillIndex];
}

const char *CASW_Marine_Skills::GetSkillName( ASW_Skill nSkillIndex )
{
	Assert( nSkillIndex >= 0 && nSkillIndex < ASW_NUM_MARINE_SKILLS );
	if ( nSkillIndex < 0 || nSkillIndex >= ASW_NUM_MARINE_SKILLS )
		return "";

	return s_szSkillName[nSkillIndex];
}

const char *CASW_Marine_Skills::GetSkillDescription( ASW_Skill nSkillIndex )
{
	Assert( nSkillIndex >= 0 && nSkillIndex < ASW_NUM_MARINE_SKILLS );
	if ( nSkillIndex < 0 || nSkillIndex >= ASW_NUM_MARINE_SKILLS )
		return "";

	return s_szSkillDescription[nSkillIndex];
}
#endif

int CASW_Marine_Skills::GetSkillPoints( CASW_Marine_Profile *pProfile, ASW_Skill iSkillIndex )
{
	CASW_Game_Resource *pGameResource = ASWGameResource();
	Assert( MarineProfileList() );
	Assert( pProfile );
	if ( !MarineProfileList() || ( !pProfile ) )
		return 0;

	int iProfileIndex = pProfile->m_ProfileIndex;
	Assert( iProfileIndex >= 0 && iProfileIndex < ASW_NUM_MARINE_PROFILES );
	if ( iProfileIndex < 0 || iProfileIndex >= ASW_NUM_MARINE_PROFILES )
		return 0;

	if ( pGameResource )
	{
		int nSkillSlot = pGameResource->GetSlotForSkill( iProfileIndex, iSkillIndex );
		if ( nSkillSlot != -1 )
		{
			// get the skill points from the ASWGameResource
			return pGameResource->GetMarineSkill( iProfileIndex, nSkillSlot );
		}
	}
	else
	{
		for ( int i = 0; i < ASW_NUM_SKILL_SLOTS - 1; i++ )
		{
			if ( pProfile->GetSkillMapping( i ) == iSkillIndex )
			{
				return pProfile->GetStaticSkillPoints( i );
			}
		}
	}

	return 0; // assume zero skill points if the marine doesn't have this skill
}

int CASW_Marine_Skills::GetMaxSkillPoints( ASW_Skill nSkillIndex )
{
	Assert( nSkillIndex >= 0 && nSkillIndex < ASW_NUM_MARINE_SKILLS );
	if ( nSkillIndex < 0 || nSkillIndex >= ASW_NUM_MARINE_SKILLS )
		return 0;

	return s_nMaxSkillPoints[nSkillIndex];
}

const char *g_szSkillNames[ASW_NUM_MARINE_SKILLS] =
{
#define ENUM_ITEM( name, ... ) "ASW_MARINE_SKILL_" #name,
	ASW_SKILL_ENUM
#undef ENUM_ITEM
};

ASW_Skill SkillFromString( const char *szSkill )
{
	int nSkills = NELEMS( g_szSkillNames );
	for ( int i = 0; i < nSkills; i++ )
	{
		if ( !V_stricmp( szSkill, g_szSkillNames[i] ) )
		{
			return ( ASW_Skill )i;
		}
	}

	return ASW_MARINE_SKILL_INVALID;
}

const char *SkillToString( ASW_Skill nSkill )
{
	Assert( nSkill >= 0 && nSkill < ASW_NUM_MARINE_SKILLS );
	if ( nSkill < 0 || nSkill >= NELEMS( g_szSkillNames ) )
	{
		return "ASW_MARINE_SKILL_INVALID";
	}

	return g_szSkillNames[nSkill];
}

int SubSkillFromString( const char *szSubSkill, ASW_Skill nExpectedSkill )
{
#define ENUM_ITEM( category, name, value ) \
	if ( !V_stricmp( szSubSkill, "ASW_MARINE_SUBSKILL_" #name ) ) \
	{ \
		return nExpectedSkill == ASW_MARINE_SKILL_INVALID || nExpectedSkill == ASW_MARINE_SKILL_##category ? value : -1; \
	}
	ASW_SUBSKILL_ENUM
#undef ENUM_ITEM

	return -1;
}

const char *SubSkillToString( ASW_Skill nSkill, int nSubSkill )
{
#define ENUM_ITEM( category, name, value ) \
	if ( nSkill == ASW_MARINE_SKILL_##category && nSubSkill == value ) \
	{ \
		return "ASW_MARINE_SUBSKILL_" #name; \
	}
#undef ENUM_ITEM

	return "ASW_MARINE_SUBSKILL_INVALID";
}

// global instance
CASW_Marine_Skills g_MarineSkills;
CASW_Marine_Skills *MarineSkills()
{
	return &g_MarineSkills;
}
