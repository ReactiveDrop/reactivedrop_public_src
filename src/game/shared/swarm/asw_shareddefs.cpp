#include "cbase.h"
#include "asw_shareddefs.h"
#include "asw_medals_shared.h"
#include "asw_achievements.h"

bool IsAlienClass( Class_T npc_class )
{
	return ( npc_class == CLASS_ASW_DRONE ||
		npc_class == CLASS_ASW_PARASITE ||
		npc_class == CLASS_ASW_SHIELDBUG ||
		npc_class == CLASS_ASW_BUZZER ||
		npc_class == CLASS_ASW_HARVESTER ||
		npc_class == CLASS_ASW_GRUB ||
		npc_class == CLASS_ASW_QUEEN_DIVER ||
		npc_class == CLASS_ASW_QUEEN_GRABBER ||
		npc_class == CLASS_ASW_ALIEN_GOO ||
		npc_class == CLASS_ASW_SHAMAN ||
		npc_class == CLASS_ASW_BOOMER ||
		npc_class == CLASS_ASW_RANGER ||
		npc_class == CLASS_ASW_EGG ||
		npc_class == CLASS_HEADCRAB ||
		npc_class == CLASS_ZOMBIE ||
		npc_class == CLASS_ANTLION ||
		npc_class == CLASS_ASW_ANTLIONGUARD ||
		npc_class == CLASS_ASW_JUGGERNAUT ||
		npc_class == CLASS_ASW_MEATBUG ||
		npc_class == CLASS_ASW_DETONATOR ||
		npc_class == CLASS_ASW_RUNNER ||
		npc_class == CLASS_ASW_FLOCK ||
		npc_class == CLASS_ASW_SPITTER ||
		npc_class == CLASS_ASW_WATCHER
		);
}

// used by powerups
bool IsBulletBasedWeaponClass( Class_T weapon_class )
{
	return ( weapon_class == CLASS_ASW_RIFLE ||
		weapon_class == CLASS_ASW_MINIGUN ||
		weapon_class == CLASS_ASW_PDW ||
		weapon_class == CLASS_ASW_FLECHETTE ||
		weapon_class == CLASS_ASW_SHOTGUN ||
		weapon_class == CLASS_ASW_RICOCHET ||
		weapon_class == CLASS_ASW_RAILGUN ||
		weapon_class == CLASS_ASW_PRIFLE ||
		weapon_class == CLASS_ASW_PISTOL ||
		weapon_class == CLASS_ASW_AUTOGUN ||
		weapon_class == CLASS_ASW_ASSAULT_SHOTGUN ||
		weapon_class == CLASS_ASW_DEAGLE ||
		weapon_class == CLASS_ASW_DEVASTATOR ||
		weapon_class == CLASS_ASW_COMBAT_RIFLE ||
		weapon_class == CLASS_ASW_COMBAT_RIFLE_SHOTGUN ||
		weapon_class == CLASS_ASW_50CALMG ||
		weapon_class == CLASS_ASW_HEAVY_RIFLE ||
		weapon_class == CLASS_ASW_MEDRIFLE ||
		weapon_class == CLASS_ASW_AR2 );
}

bool IsSentryClass( Class_T entity_class )
{
	return ( entity_class == CLASS_ASW_SENTRY_GUN ||
			entity_class == CLASS_ASW_SENTRY_FLAMER ||
			entity_class == CLASS_ASW_SENTRY_FREEZE ||
			entity_class == CLASS_ASW_SENTRY_CANNON ||
			entity_class == CLASS_ASW_TESLA_TRAP ||
			entity_class == CLASS_ASW_SENTRY_RAILGUN ||
			entity_class == CLASS_ASW_REMOTE_TURRET );
}

ConVar asw_visrange_generic( "asw_visrange_generic", "400", FCVAR_CHEAT | FCVAR_REPLICATED, "Vismon range" );

#ifdef CLIENT_DLL
IMPLEMENT_AUTO_LIST( IHealthTrackedAutoList );
#endif

// certain props are exempt from our auto stuck freeing code, since they break easily
bool CanMarineGetStuckOnProp( const char *szModelName )
{
	if ( !Q_stricmp( szModelName, "models/props/crates/supplycrate.mdl" ) )
		return false;

	if ( !Q_stricmp( szModelName, "models/swarmprops/barrelsandcrates/cardbox1breakable.mdl" ) )
		return false;

	return true;
}

class CASW_Medal_Achievement_Pair
{
public:
	CASW_Medal_Achievement_Pair( int nMedal, int nAchievement )
	{
		m_nMedal = nMedal;
		m_nAchievement = nAchievement;
	}
	int m_nMedal;
	int m_nAchievement;
};

CASW_Medal_Achievement_Pair s_MedalAchievements[]=
{
	CASW_Medal_Achievement_Pair( MEDAL_CLEAR_FIRING, ACHIEVEMENT_ASW_NO_FRIENDLY_FIRE ),
	CASW_Medal_Achievement_Pair( MEDAL_SHIELDBUG_ASSASSIN, ACHIEVEMENT_ASW_SHIELDBUG ),
	CASW_Medal_Achievement_Pair( MEDAL_EXPLOSIVES_MERIT, ACHIEVEMENT_ASW_GRENADE_MULTI_KILL ),
	CASW_Medal_Achievement_Pair( MEDAL_SHARPSHOOTER, ACHIEVEMENT_ASW_ACCURACY ),
	CASW_Medal_Achievement_Pair( MEDAL_PERFECT, ACHIEVEMENT_ASW_NO_DAMAGE_TAKEN ),
	CASW_Medal_Achievement_Pair( MEDAL_IRON_FIST, ACHIEVEMENT_ASW_MELEE_KILLS ),
	CASW_Medal_Achievement_Pair( MEDAL_COLLATERAL_DAMAGE, ACHIEVEMENT_ASW_BARREL_KILLS ),
	CASW_Medal_Achievement_Pair( MEDAL_GOLDEN_HALO, ACHIEVEMENT_ASW_INFESTATION_CURING ),
	CASW_Medal_Achievement_Pair( MEDAL_BLOOD_HALO, ACHIEVEMENT_ASW_ALL_HEALING ),
	CASW_Medal_Achievement_Pair( MEDAL_PEST_CONTROL, ACHIEVEMENT_ASW_EGGS_BEFORE_HATCH ),
	CASW_Medal_Achievement_Pair( MEDAL_EXTERMINATOR, ACHIEVEMENT_ASW_MELEE_PARASITE ),
	CASW_Medal_Achievement_Pair( MEDAL_ELECTRICAL_SYSTEMS_EXPERT, ACHIEVEMENT_ASW_PROTECT_TECH ),
	CASW_Medal_Achievement_Pair( MEDAL_SMALL_ARMS_SPECIALIST, ACHIEVEMENT_ASW_FAST_RELOADS_IN_A_ROW ),
	CASW_Medal_Achievement_Pair( MEDAL_GUNFIGHTER, ACHIEVEMENT_ASW_GROUP_DAMAGE_AMP ),
	CASW_Medal_Achievement_Pair( MEDAL_BUGSTOMPER, ACHIEVEMENT_ASW_BOOMER_KILL_EARLY ),
	CASW_Medal_Achievement_Pair( MEDAL_RECKLESS_EXPLOSIVES_MERIT, ACHIEVEMENT_ASW_STUN_GRENADE ),
	CASW_Medal_Achievement_Pair( MEDAL_LIFESAVER, ACHIEVEMENT_ASW_DODGE_RANGER_SHOT ),
	CASW_Medal_Achievement_Pair( MEDAL_HUNTER, ACHIEVEMENT_ASW_FREEZE_GRENADE ),
	CASW_Medal_Achievement_Pair( MEDAL_SPEED_RUN_LANDING_BAY, ACHIEVEMENT_ASW_SPEEDRUN_LANDING_BAY ),
	CASW_Medal_Achievement_Pair( MEDAL_SPEED_RUN_DESCENT, ACHIEVEMENT_ASW_SPEEDRUN_DESCENT ),
	CASW_Medal_Achievement_Pair( MEDAL_SPEED_RUN_OUTSIDE, ACHIEVEMENT_ASW_SPEEDRUN_DEIMA ),
	CASW_Medal_Achievement_Pair( MEDAL_SPEED_RUN_PLANT, ACHIEVEMENT_ASW_SPEEDRUN_RYDBERG ),
	CASW_Medal_Achievement_Pair( MEDAL_SPEED_RUN_OFFICE, ACHIEVEMENT_ASW_SPEEDRUN_RESIDENTIAL ),
	CASW_Medal_Achievement_Pair( MEDAL_SPEED_RUN_DESCENT, ACHIEVEMENT_ASW_SPEEDRUN_DESCENT ),
	CASW_Medal_Achievement_Pair( MEDAL_SPEED_RUN_SEWERS, ACHIEVEMENT_ASW_SPEEDRUN_SEWER ),
	CASW_Medal_Achievement_Pair( MEDAL_SPEED_RUN_MINE, ACHIEVEMENT_ASW_SPEEDRUN_TIMOR ),
	CASW_Medal_Achievement_Pair( MEDAL_ALL_SURVIVE_A_MISSION, ACHIEVEMENT_ASW_MISSION_NO_DEATHS )	
};

bool MedalMatchesAchievement( int nMedalIndex, int nAchievementIndex )
{
	int nPairs = NELEMS( s_MedalAchievements );
	for ( int i = 0; i < nPairs; i++ )
	{
		if ( nMedalIndex == s_MedalAchievements[i].m_nMedal && nAchievementIndex == s_MedalAchievements[i].m_nAchievement )
			return true;
	}
	return false;
}

int GetAchievementIndexForMedal( int nMedalIndex )
{
	int nPairs = NELEMS( s_MedalAchievements );
	for ( int i = 0; i < nPairs; i++ )
	{
		if ( nMedalIndex == s_MedalAchievements[i].m_nMedal )
			return s_MedalAchievements[i].m_nAchievement;
	}
	return -1;
}

const char *ClassToString( ASW_Marine_Class nClass )
{
	switch ( nClass )
	{
	default:
	case MARINE_CLASS_UNDEFINED:
		return "MARINE_CLASS_UNDEFINED";
	case MARINE_CLASS_NCO:
		return "MARINE_CLASS_NCO";
	case MARINE_CLASS_SPECIAL_WEAPONS:
		return "MARINE_CLASS_SPECIAL_WEAPONS";
	case MARINE_CLASS_MEDIC:
		return "MARINE_CLASS_MEDIC";
	case MARINE_CLASS_TECH:
		return "MARINE_CLASS_TECH";
	}
}

ASW_Marine_Class ClassFromString( const char *szClass )
{
	if ( !V_stricmp( szClass, "MARINE_CLASS_NCO" ) )
	{
		return MARINE_CLASS_NCO;
	}
	if ( !V_stricmp( szClass, "MARINE_CLASS_SPECIAL_WEAPONS" ) )
	{
		return MARINE_CLASS_SPECIAL_WEAPONS;
	}
	if ( !V_stricmp( szClass, "MARINE_CLASS_MEDIC" ) )
	{
		return MARINE_CLASS_MEDIC;
	}
	if ( !V_stricmp( szClass, "MARINE_CLASS_TECH" ) )
	{
		return MARINE_CLASS_TECH;
	}
	return MARINE_CLASS_UNDEFINED;
}
