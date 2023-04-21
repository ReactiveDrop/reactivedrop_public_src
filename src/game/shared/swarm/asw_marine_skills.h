#ifndef _INCLUDED_ASW_MARINE_SKILLS_H
#define _INCLUDED_ASW_MARINE_SKILLS_H
#ifdef _WIN32
#pragma once
#endif

#include "asw_shareddefs.h"

// marine skill enumerations

#define ASW_SKILL_ENUM \
	/* NCO */ \
	ENUM_ITEM( LEADERSHIP, "swarm/SkillButtons/Leadership", "#asw_leadership", "#asw_leadership_desc", 5 ) \
	ENUM_ITEM( VINDICATOR, "swarm/SkillButtons/Vindicator", "#asw_vindicator", "#asw_vindicator_desc", 5 ) \
	/* special weapons */ \
	ENUM_ITEM( AUTOGUN, "swarm/SkillButtons/Autogun", "#asw_autogunsk", "#asw_autogunsk_desc", 5 ) \
	ENUM_ITEM( STOPPING_POWER, "swarm/SkillButtons/Secondary", "#asw_stopping", "#asw_stopping_desc", 5 ) /* note: skill code doesn't calculate the actual % chance, because that is an attribute of the individual guns.  See their weapon info for the stopping power % increase per skill point */ \
	ENUM_ITEM( PIERCING, "swarm/SkillButtons/Piercing", "#asw_piercingbullets", "#asw_piercingbullets_desc", 5 ) \
	/* medic */ \
	ENUM_ITEM( HEALING, "swarm/SkillButtons/Healing", "#asw_healing", "#asw_healing_desc", 5 ) \
	ENUM_ITEM( XENOWOUNDS, "swarm/SkillButtons/Xenowound", "#asw_xenowound", "#asw_xenowound_desc", 3 ) \
	ENUM_ITEM( DRUGS, "swarm/SkillButtons/Drugs", "#asw_combatdrugs", "#asw_combatdrugs_desc", 5 ) \
	/* tech */ \
	ENUM_ITEM( HACKING, "swarm/SkillButtons/Hacking", "#asw_hacking", "#asw_hacking_desc", 5 ) \
	ENUM_ITEM( SCANNER, "swarm/SkillButtons/Scanner", "#asw_scanner", "#asw_scanner_desc", 3 ) \
	ENUM_ITEM( ENGINEERING, "swarm/SkillButtons/Engineering", "#asw_engineering", "#asw_engineering_desc", 3 ) \
	\
	ENUM_ITEM( ACCURACY, "swarm/SkillButtons/Accuracy", "#asw_accuracy_skill", "#asw_accuracy_skill_desc", 5 ) \
	ENUM_ITEM( GRENADES, "swarm/SkillButtons/Grenade", "#asw_grenades", "#asw_grenades_desc", 5 ) \
	ENUM_ITEM( HEALTH, "swarm/SkillButtons/Health", "#asw_health", "#asw_health_desc", 5 ) \
	ENUM_ITEM( MELEE, "swarm/SkillButtons/Melee", "#asw_melee", "#asw_melee_desc", 5 ) \
	ENUM_ITEM( RELOADING, "swarm/SkillButtons/SkillReload", "#asw_reloading", "#asw_reloading_desc", 5 ) \
	ENUM_ITEM( AGILITY, "swarm/SkillButtons/Agility", "#asw_agility", "#asw_agility_desc", 5 ) \
	\
	ENUM_ITEM( SPARE, "swarm/SkillButtons/Spare", "#asw_points", "#asw_points_desc", 99 )

// skills
enum ASW_Skill
{
	ASW_MARINE_SKILL_INVALID = -1,

#define ENUM_ITEM( name, ... ) ASW_MARINE_SKILL_##name,
	ASW_SKILL_ENUM
#undef ENUM_ITEM

	ASW_NUM_MARINE_SKILLS
};

ASW_Skill SkillFromString( const char *szSkill );
const char *SkillToString( ASW_Skill nSkill );

#define ASW_SUBSKILL_ENUM \
	ENUM_ITEM( LEADERSHIP, LEADERSHIP_ACCURACY_CHANCE, 0 ) \
	ENUM_ITEM( LEADERSHIP, LEADERSHIP_DAMAGE_RESIST, 1 ) \
	\
	ENUM_ITEM( VINDICATOR, VINDICATOR_DAMAGE, 0 ) \
	ENUM_ITEM( VINDICATOR, VINDICATOR_PELLETS, 1 ) \
	ENUM_ITEM( VINDICATOR, VINDICATOR_MUZZLE, 2 ) \
	\
	ENUM_ITEM( GRENADES, GRENADE_RADIUS, 0 ) \
	ENUM_ITEM( GRENADES, GRENADE_DMG, 1 ) \
	ENUM_ITEM( GRENADES, GRENADE_INCENDIARY_DMG, 2 ) \
	ENUM_ITEM( GRENADES, GRENADE_CLUSTER_DMG, 3 ) \
	ENUM_ITEM( GRENADES, GRENADE_CLUSTERS, 4 ) \
	ENUM_ITEM( GRENADES, GRENADE_FLECHETTE_DMG, 5 ) \
	ENUM_ITEM( GRENADES, GRENADE_HORNET_DMG, 6 ) \
	ENUM_ITEM( GRENADES, GRENADE_HORNET_COUNT, 7 ) \
	ENUM_ITEM( GRENADES, GRENADE_HORNET_INTERVAL, 8 ) \
	ENUM_ITEM( GRENADES, GRENADE_FREEZE_RADIUS, 9 ) \
	ENUM_ITEM( GRENADES, GRENADE_FREEZE_DURATION, 10 ) \
	ENUM_ITEM( GRENADES, GRENADE_SMART_BOMB_COUNT, 11 ) \
	ENUM_ITEM( GRENADES, GRENADE_SMART_BOMB_INTERVAL, 12 ) \
	ENUM_ITEM( GRENADES, GRENADE_LASER_MINES, 13 ) \
	ENUM_ITEM( GRENADES, GRENADE_MINES_FIRES, 14 ) \
	ENUM_ITEM( GRENADES, GRENADE_MINES_DURATION, 15 ) \
	\
	ENUM_ITEM( MELEE, MELEE_DMG, 0 ) \
	ENUM_ITEM( MELEE, MELEE_FORCE, 1 ) \
	ENUM_ITEM( MELEE, MELEE_FLINCH, 2 ) \
	ENUM_ITEM( MELEE, MELEE_SPEED, 3 ) \
	\
	ENUM_ITEM( ENGINEERING, ENGINEERING_WELDING, 0 ) \
	ENUM_ITEM( ENGINEERING, ENGINEERING_SENTRY, 1 ) \
	ENUM_ITEM( ENGINEERING, ENGINEERING_FIRERATE, 2 ) \
	\
	ENUM_ITEM( HEALING, HEALING_CHARGES, 0 ) \
	ENUM_ITEM( HEALING, SELF_HEALING_CHARGES, 1 ) \
	ENUM_ITEM( HEALING, HEALING_HPS, 2 ) \
	ENUM_ITEM( HEALING, HEALING_MEDKIT_HPS, 3 ) \
	ENUM_ITEM( HEALING, HEAL_GRENADE_HEAL_AMOUNT, 4 ) \
	ENUM_ITEM( HEALING, HEAL_GUN_CHARGES, 5 ) \
	ENUM_ITEM( HEALING, HEAL_GUN_HEAL_AMOUNT, 6 ) \
	ENUM_ITEM( HEALING, HEALAMP_GUN_CHARGES, 7 ) \
	ENUM_ITEM( HEALING, MEDRIFLE_HEALING_CHARGES, 8 ) \
	\
	ENUM_ITEM( DRUGS, STIM_DURATION, 0 ) \
	ENUM_ITEM( DRUGS, HEALAMP_GUN_AMP_CHARGES, 1 ) \
	\
	ENUM_ITEM( AUTOGUN, AUTOGUN_DMG, 0 ) \
	ENUM_ITEM( AUTOGUN, AUTOGUN_MUZZLE, 1 ) \
	\
	ENUM_ITEM( ACCURACY, ACCURACY_RIFLE_DMG, 0 ) \
	ENUM_ITEM( ACCURACY, ACCURACY_PRIFLE_DMG, 1 ) \
	ENUM_ITEM( ACCURACY, ACCURACY_SHOTGUN_DMG, 2 ) \
	ENUM_ITEM( ACCURACY, ACCURACY_RAILGUN_DMG, 3 ) \
	ENUM_ITEM( ACCURACY, ACCURACY_FLAMER_DMG, 4 ) \
	ENUM_ITEM( ACCURACY, ACCURACY_PISTOL_DMG, 5 ) \
	ENUM_ITEM( ACCURACY, ACCURACY_PDW_DMG, 6 ) \
	ENUM_ITEM( ACCURACY, ACCURACY_SNIPER_RIFLE_DMG, 7 ) \
	ENUM_ITEM( ACCURACY, ACCURACY_TESLA_CANNON_DMG, 8 ) \
	ENUM_ITEM( ACCURACY, ACCURACY_HEAVY_RIFLE_DMG, 9 ) \
	ENUM_ITEM( ACCURACY, ACCURACY_MEDRIFLE_DMG, 10 ) \
	ENUM_ITEM( ACCURACY, ACCURACY_MUZZLE, 11 ) \
	ENUM_ITEM( ACCURACY, ACCURACY_DEAGLE_DMG, 12 ) \
	ENUM_ITEM( ACCURACY, ACCURACY_AR2_DMG, 13 ) \
	ENUM_ITEM( ACCURACY, ACCURACY_DEVASTATOR_DMG, 14 ) \
	\
	ENUM_ITEM( HACKING, HACKING_SPEED_SCALE, 0 ) \
	ENUM_ITEM( HACKING, HACKING_TUMBLER_COUNT_REDUCTION, 1 ) \
	ENUM_ITEM( HACKING, HACKING_ENTRIES_PER_TUMBLER_REDUCTION, 2 ) \
	\
	ENUM_ITEM( RELOADING, RELOADING_SPEED_SCALE, 0 ) \
	ENUM_ITEM( RELOADING, RELOADING_SOUND, 1 ) \
	ENUM_ITEM( RELOADING, RELOADING_FAST_WIDTH_SCALE, 2 )

// subskills - used to select different elements of the game that are affected by the same skill
enum
{
#define ENUM_ITEM( category, name, value ) ASW_MARINE_SUBSKILL_##name = value,
	ASW_SUBSKILL_ENUM
#undef ENUM_ITEM
};

int SubSkillFromString( const char *szSubSkill, ASW_Skill nExpectedSkill = ASW_MARINE_SKILL_INVALID );
const char *SubSkillToString( ASW_Skill nSkill, int nSubSkill );

#ifdef CLIENT_DLL
class C_ASW_Marine;
class C_ASW_Marine_Resource;
#else
class CASW_Marine;
class CASW_Marine_Resource;
#endif
class CASW_Marine_Profile;

// this class holds accessor functions for getting at the variables that are affected by skills
class CASW_Marine_Skills
{
public:
	CASW_Marine_Skills();
#ifdef CLIENT_DLL
	// get the value based on the current skills of that marine
	float GetSkillBasedValueByMarine( C_ASW_Marine *pMarine, ASW_Skill iSkillIndex, int iSubSkill = 0 );
	float GetSkillBasedValueByMarineResource( C_ASW_Marine_Resource *pMarineResource, ASW_Skill iSkillIndex, int iSubSkill = 0 );
#else
	float GetSkillBasedValueByMarine( CASW_Marine *pMarine, ASW_Skill iSkillIndex, int iSubSkill = 0 );
	float GetSkillBasedValueByMarineResource( CASW_Marine_Resource *pMarineResource, ASW_Skill iSkillIndex, int iSubSkill = 0 );
#endif
	// returns the live marine with the best skill of this type
	float GetBestSkillValue( ASW_Skill iSkillIndex, int iSubSkill = 0 );
	float GetHighestSkillValueNearby( const Vector &pos, float MaxDistance, ASW_Skill iSkillIndex, int iSubSkill = 0 );
	float GetLowestSkillValueNearby( const Vector &pos, float MaxDistance, ASW_Skill iSkillIndex, int iSubSkill = 0 );

	// get the value based on the current skills of that marine, or based on some specified number of points in that skill
	float GetSkillBasedValue( CASW_Marine_Profile *pProfile, ASW_Skill iSkillIndex, int iSubSkill = 0, int iSkillPoints = -1 );
#ifndef CLIENT_DLL
	CASW_Marine *CheckSkillChanceNearby( CBaseEntity *pAlly, const Vector &pos, float MaxDistance, ASW_Skill iSkillIndex, int iSubSkill = 0 );
	// returns the marine last used by the GetHighest/Lowest functions
	CASW_Marine *GetLastSkillMarine() { return m_hLastSkillMarine.Get(); }
	CHandle<CASW_Marine> m_hLastSkillMarine;
#else
	const char *GetSkillImage( ASW_Skill nSkillIndex );
	const char *GetSkillName( ASW_Skill nSkillIndex );
	const char *GetSkillDescription( ASW_Skill nSkillIndex );
#endif
	int GetSkillPoints( CASW_Marine_Profile *pProfile, ASW_Skill iSkillIndex );
	int GetMaxSkillPoints( ASW_Skill nSkillIndex );
};

// global accessor
CASW_Marine_Skills *MarineSkills();

#endif // _INCLUDED_ASW_MARINE_SKILLS_H

