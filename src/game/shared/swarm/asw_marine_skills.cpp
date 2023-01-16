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
#include "asw_marine_skills.h"
#include "asw_marine_profile.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// base convars
ConVar asw_skill_leadership_accuracy_chance_base( "asw_skill_leadership_accuracy_chance_base", "0", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_leadership_damage_resist_base( "asw_skill_leadership_damage_resist_base", "0", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_vindicator_dmg_base( "asw_skill_vindicator_dmg_base", "0", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_vindicator_pellets_base( "asw_skill_vindicator_pellets_base", "7", FCVAR_REPLICATED | FCVAR_CHEAT );

ConVar asw_skill_autogun_base( "asw_skill_autogun_base", "0", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_piercing_base( "asw_skill_piercing_base", "0", FCVAR_REPLICATED | FCVAR_CHEAT );

ConVar asw_skill_healing_charges_base( "asw_skill_healing_charges_base", "4", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_self_healing_charges_base( "asw_skill_self_healing_charges_base", "2", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_healing_medkit_hps_base( "asw_skill_healing_medkit_hps_base", "50", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_healing_hps_base( "asw_skill_healing_hps_base", "25", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_healing_grenade_base( "asw_skill_healing_grenade_base", "120", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_healing_gun_charges_base( "asw_skill_healing_gun_charges_base", "40", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_healing_gun_base( "asw_skill_healing_gun_base", "5", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_healing_amp_gun_charges_base( "asw_skill_healing_amp_gun_charges_base", "20", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_xenowounds_base( "asw_skill_xenowounds_base", "100", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_drugs_base( "asw_skill_drugs_base", "5", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_drugs_healamp_base( "asw_skill_drugs_healamp_base", "90", FCVAR_REPLICATED | FCVAR_CHEAT );

ConVar asw_skill_hacking_speed_base( "asw_skill_hacking_speed_base", "2.0", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_scanner_base( "asw_skill_scanner_base", "600", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_engineering_welding_base( "asw_skill_engineering_welding_base", "0.8", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_engineering_sentry_base( "asw_skill_engineering_sentry_base", "1.0", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_engineering_firerate_base( "asw_skill_engineering_firerate_base", "0.0", FCVAR_REPLICATED | FCVAR_CHEAT );

ConVar asw_skill_grenades_radius_base( "asw_skill_grenades_radius_base", "280", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_grenades_dmg_base( "asw_skill_grenades_dmg_base", "128", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_grenades_incendiary_dmg_base( "asw_skill_grenades_incendiary_dmg_base", "20", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_grenades_cluster_dmg_base( "asw_skill_grenades_cluster_dmg_base", "0", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_grenades_clusters_base( "asw_skill_grenades_clusters_base", "0", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_grenades_flechette_dmg_base( "asw_skill_grenades_flechette_dmg_base", "10", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_grenades_hornet_dmg_base( "asw_skill_grenades_hornet_dmg_base", "50", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_grenades_hornet_count_base( "asw_skill_grenades_hornet_count_base", "8", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_grenades_hornet_interval_base( "asw_skill_grenades_hornet_interval_base", "0.09", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_grenades_freeze_radius_base( "asw_skill_grenades_freeze_radius_base", "210", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_grenades_freeze_duration_base( "asw_skill_grenades_freeze_duration_base", "3.0", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_grenades_smart_count_base( "asw_skill_grenades_smart_count_base", "32", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_grenades_smart_interval_base( "asw_skill_grenades_smart_interval_base", "0.09", FCVAR_REPLICATED | FCVAR_CHEAT );

ConVar asw_skill_health_base( "asw_skill_health_base", "80", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_melee_dmg_base( "asw_skill_melee_dmg_base", "30", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_melee_force_base( "asw_skill_melee_force_base", "10", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_melee_speed_base( "asw_skill_melee_speed_base", "1.0", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_reloading_base( "asw_skill_reloading_base", "1.4", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_reloading_fast_base( "asw_skill_reloading_fast_base", "1.0", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_agility_movespeed_base( "asw_skill_agility_movespeed_base", "290", FCVAR_REPLICATED | FCVAR_CHEAT );

// step convars
ConVar asw_skill_leadership_accuracy_chance_step( "asw_skill_leadership_accuracy_chance_step", "0.03", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_leadership_damage_resist_step( "asw_skill_leadership_damage_resist_step", "0.06", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_vindicator_dmg_step( "asw_skill_vindicator_dmg_step", "2.0", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_vindicator_pellets_step( "asw_skill_vindicator_pellets_step", "0", FCVAR_REPLICATED | FCVAR_CHEAT );

ConVar asw_skill_autogun_step( "asw_skill_autogun_step", "1", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_piercing_step( "asw_skill_piercing_step", "0.20", FCVAR_REPLICATED | FCVAR_CHEAT );

ConVar asw_skill_healing_charges_step( "asw_skill_healing_charges_step", "1", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_self_healing_charges_step( "asw_skill_self_healing_charges_step", "0.5", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_healing_hps_step( "asw_skill_healing_hps_step", "8", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_healing_grenade_step( "asw_skill_healing_grenade_step", "30", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_healing_gun_charges_step( "asw_skill_healing_gun_charges_step", "10", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_healing_gun_step( "asw_skill_healing_gun_step", "1", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_healing_amp_gun_charges_step( "asw_skill_healing_amp_gun_charges_step", "5", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_healing_medkit_hps_step( "asw_skill_healing_medkit_hps_step", "5", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_xenowounds_step( "asw_skill_xenowounds_step", "-25", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_drugs_step( "asw_skill_drugs_step", "0.8", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_drugs_healamp_step( "asw_skill_drugs_healamp_step", "20", FCVAR_REPLICATED | FCVAR_CHEAT );

ConVar asw_skill_hacking_speed_step( "asw_skill_hacking_speed_step", "0.1", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_scanner_step( "asw_skill_scanner_step", "150", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_engineering_welding_step( "asw_skill_engineering_welding_step", "0.5", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_engineering_sentry_step( "asw_skill_engineering_sentry_step", "0.25", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_engineering_firerate_step( "asw_skill_engineering_firerate_step", "0.005", FCVAR_REPLICATED | FCVAR_CHEAT );

ConVar asw_skill_grenades_radius_step( "asw_skill_grenades_radius_step", "20", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_grenades_dmg_step( "asw_skill_grenades_dmg_step", "20", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_grenades_incendiary_dmg_step( "asw_skill_grenades_incendiary_dmg_step", "0", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_grenades_cluster_dmg_step( "asw_skill_grenades_cluster_dmg_step", "10", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_grenades_clusters_step( "asw_skill_grenades_clusters_step", "1", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_grenades_flechette_dmg_step( "asw_skill_grenades_flechette_dmg_step", "1", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_grenades_hornet_dmg_step( "asw_skill_grenades_hornet_dmg_step", "1", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_grenades_hornet_count_step( "asw_skill_grenades_hornet_count_step", "0", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_grenades_hornet_interval_step( "asw_skill_grenades_hornet_interval_step", "0", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_grenades_freeze_radius_step( "asw_skill_grenades_freeze_radius_step", "0", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_grenades_freeze_duration_step( "asw_skill_grenades_freeze_duration_step", "0.3", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_grenades_smart_count_step( "asw_skill_grenades_smart_count_step", "0", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_grenades_smart_interval_step( "asw_skill_grenades_smart_interval_step", "0", FCVAR_REPLICATED | FCVAR_CHEAT );

ConVar asw_skill_health_step( "asw_skill_health_step", "15", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_melee_dmg_step( "asw_skill_melee_dmg_step", "6", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_melee_force_step( "asw_skill_melee_force_step", "1.0", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_melee_speed_step( "asw_skill_melee_speed_step", "0.1", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_reloading_step( "asw_skill_reloading_step", "-0.14", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_reloading_fast_step( "asw_skill_reloading_fast_step", "0.05", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_agility_movespeed_step( "asw_skill_agility_movespeed_step", "10", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_agility_reload_step( "asw_skill_agility_reload_step", "0", FCVAR_REPLICATED | FCVAR_CHEAT );

ConVar asw_skill_mines_fires_base( "asw_skill_mines_fires_base", "1", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_mines_fires_step( "asw_skill_mines_fires_step", "0.5", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_mines_duration_base( "asw_skill_mines_duration_base", "10.0", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_mines_duration_step( "asw_skill_mines_duration_step", "5.0", FCVAR_REPLICATED | FCVAR_CHEAT );

// accuracy convars
ConVar asw_skill_accuracy_rifle_dmg_base( "asw_skill_accuracy_rifle_dmg_base", "0", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_accuracy_rifle_dmg_step( "asw_skill_accuracy_rifle_dmg_step", "1", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_accuracy_prifle_dmg_base( "asw_skill_accuracy_prifle_dmg_base", "0", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_accuracy_prifle_dmg_step( "asw_skill_accuracy_prifle_dmg_step", "1", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_accuracy_shotgun_dmg_base( "asw_skill_accuracy_shotgun_dmg_base", "0", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_accuracy_shotgun_dmg_step( "asw_skill_accuracy_shotgun_dmg_step", "2", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_accuracy_railgun_dmg_base( "asw_skill_accuracy_railgun_dmg_base", "0", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_accuracy_railgun_dmg_step( "asw_skill_accuracy_railgun_dmg_step", "10", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_accuracy_flamer_dmg_base( "asw_skill_accuracy_flamer_dmg_base", "0", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_accuracy_flamer_dmg_step( "asw_skill_accuracy_flamer_dmg_step", "0.5", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_accuracy_pistol_dmg_base( "asw_skill_accuracy_pistol_dmg_base", "0", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_accuracy_pistol_dmg_step( "asw_skill_accuracy_pistol_dmg_step", "4", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_accuracy_deagle_dmg_base( "asw_skill_accuracy_deagle_dmg_base", "0", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_accuracy_deagle_dmg_step( "asw_skill_accuracy_deagle_dmg_step", "0", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_accuracy_pdw_dmg_base( "asw_skill_accuracy_pdw_dmg_base", "0", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_accuracy_pdw_dmg_step( "asw_skill_accuracy_pdw_dmg_step", "1.0", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_muzzle_flash_base( "asw_skill_muzzle_flash_base", "1.0", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_muzzle_flash_step( "asw_skill_muzzle_flash_step", "0.2", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_accuracy_sniper_rifle_dmg_base( "asw_skill_accuracy_sniper_rifle_dmg_base", "0", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_accuracy_sniper_rifle_dmg_step( "asw_skill_accuracy_sniper_rifle_dmg_step", "20", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_accuracy_tesla_cannon_dmg_base( "asw_skill_accuracy_tesla_cannon_dmg_base", "0", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_accuracy_tesla_cannon_dmg_step( "asw_skill_accuracy_tesla_cannon_dmg_step", "0.25", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_accuracy_heavy_rifle_dmg_base( "asw_skill_accuracy_heavy_rifle_dmg_base", "0", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_accuracy_heavy_rifle_dmg_step( "asw_skill_accuracy_heavy_rifle_dmg_step", "2", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_accuracy_medrifle_dmg_base( "asw_skill_accuracy_medrifle_dmg_base", "0", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_accuracy_medrifle_dmg_step( "asw_skill_accuracy_medrifle_dmg_step", "2", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_accuracy_ar2_dmg_base( "asw_skill_accuracy_ar2_dmg_base", "0", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_skill_accuracy_ar2_dmg_step( "asw_skill_accuracy_ar2_dmg_step", "2", FCVAR_REPLICATED | FCVAR_CHEAT );

ConVar asw_skill_laser_mines_base( "asw_skill_laser_mines_base", "1", FCVAR_REPLICATED | FCVAR_CHEAT, "Number of laser mines to deploy by marines with no Explosives skills", true, 1, true, 10 );
ConVar asw_skill_laser_mines_moderate( "asw_skill_laser_mines_moderate", "2", FCVAR_REPLICATED | FCVAR_CHEAT, "Number of laser mines to deploy by marines with moderate(>1) Explosives skills", true, 1, true, 10 );
ConVar asw_skill_laser_mines_expert( "asw_skill_laser_mines_expert", "3", FCVAR_REPLICATED | FCVAR_CHEAT, "Number of laser mines to deploy by marines with expert(>3) Explosives skills. Currently only Jaeger have it", true, 1, true, 10 );

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
			return asw_skill_muzzle_flash_base.GetFloat() + asw_skill_muzzle_flash_step.GetFloat() * iSkillPoints;
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
		default:
			Assert( 0 );
			return 0.0f;
		}
	case ASW_MARINE_SKILL_STOPPING_POWER:
		Assert( iSubSkill == 0 );
		return iSkillPoints;
	case ASW_MARINE_SKILL_PIERCING:
		Assert( iSubSkill == 0 );
		return asw_skill_piercing_base.GetFloat() + asw_skill_piercing_step.GetFloat() * iSkillPoints;
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
		default:
			Assert( 0 );
			return 0.0f;
		}
	case ASW_MARINE_SKILL_XENOWOUNDS:
		Assert( iSubSkill == 0 );
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
		Assert( iSubSkill == 0 );
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
				return asw_skill_laser_mines_expert.GetInt();
			if ( iSkillPoints >= 2 )
				return asw_skill_laser_mines_moderate.GetInt();
			return asw_skill_laser_mines_base.GetInt();
		case ASW_MARINE_SUBSKILL_GRENADE_MINES_FIRES:
			return asw_skill_mines_fires_base.GetFloat() + asw_skill_mines_fires_step.GetFloat() * iSkillPoints;
		case ASW_MARINE_SUBSKILL_GRENADE_MINES_DURATION:
			return asw_skill_mines_duration_base.GetFloat() + asw_skill_mines_duration_step.GetFloat() * iSkillPoints;
		default:
			Assert( 0 );
			return 0.0f;
		}
	case ASW_MARINE_SKILL_HEALTH:
		Assert( iSubSkill == 0 );
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
		Assert( iSubSkill == 0 );
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
	Assert( pGameResource );
	Assert( MarineProfileList() );
	Assert( pProfile );
	if ( !pGameResource || !MarineProfileList() || ( !pProfile ) )
		return 0;

	int iProfileIndex = pProfile->m_ProfileIndex;
	Assert( iProfileIndex >= 0 && iProfileIndex < MarineProfileList()->m_NumProfiles );
	if ( iProfileIndex < 0 || iProfileIndex >= MarineProfileList()->m_NumProfiles )
		return 0;

	int nSkillSlot = pGameResource->GetSlotForSkill( iProfileIndex, iSkillIndex );
	if ( nSkillSlot != -1 )
	{
		// get the skill points from the ASWGameResource
		return pGameResource->GetMarineSkill( iProfileIndex, nSkillSlot );
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
