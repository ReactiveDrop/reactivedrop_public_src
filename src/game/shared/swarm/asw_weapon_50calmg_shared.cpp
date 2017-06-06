#include "cbase.h"

#include "asw_weapon_50calmg_shared.h"
#include "in_buttons.h"
#ifdef CLIENT_DLL
	#include "c_asw_player.h"
	#include "c_asw_marine.h"
#else
	#include "asw_player.h"
	#include "asw_marine.h"
#endif


#ifdef CLIENT_DLL
#include "c_asw_player.h"
#include "c_asw_weapon.h"
#include "c_asw_marine.h"
#include "c_asw_fx.h"
#include "c_te_legacytempents.h"
#else
#include "asw_lag_compensation.h"
#include "asw_marine.h"
#include "asw_marine_resource.h"
#include "asw_player.h"
#include "asw_weapon.h"
#include "npcevent.h"
#include "shot_manipulator.h"
#include "asw_shotgun_pellet.h"
#include "asw_marine_speech.h"
#include "asw_weapon_ammo_bag_shared.h"
#endif
#include "asw_marine_skills.h"
#include "asw_marine_profile.h"
#include "ai_debug_shared.h"
#include "asw_weapon_parse.h"
#include "asw_deathmatch_mode_light.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED(ASW_Weapon_50CalMG, DT_ASW_Weapon_50CalMG)

BEGIN_NETWORK_TABLE(CASW_Weapon_50CalMG, DT_ASW_Weapon_50CalMG)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CASW_Weapon_50CalMG)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(asw_weapon_50calmg, CASW_Weapon_50CalMG);
PRECACHE_WEAPON_REGISTER(asw_weapon_50calmg);

#ifndef CLIENT_DLL
BEGIN_DATADESC(CASW_Weapon_50CalMG)
END_DATADESC()
#endif

extern ConVar asw_weapon_max_shooting_distance;
extern ConVar asw_weapon_force_scale;
#ifdef GAME_DLL
extern ConVar asw_debug_marine_damage;
extern ConVar asw_DebugAutoAim;
#endif 

CASW_Weapon_50CalMG::CASW_Weapon_50CalMG()
{
}

CASW_Weapon_50CalMG::~CASW_Weapon_50CalMG()
{

}

void CASW_Weapon_50CalMG::Precache()
{
	PrecacheModel("swarm/sprites/whiteglow1.vmt");
	PrecacheModel("swarm/sprites/greylaser1.vmt");
	PrecacheScriptSound("ASW_Weapon.Empty");
	PrecacheScriptSound("ASW_Weapon.Reload3");

	BaseClass::Precache();
}

// float CASW_Weapon_50CalMG::GetMovementScale()
// {
// 	return ShouldMarineMoveSlow() ? 0.5f : 0.8f;
// }

float CASW_Weapon_50CalMG::GetWeaponDamage()
{
	//float flDamage = 18.0f;
	float flDamage = GetWeaponInfo()->m_flBaseDamage;

// if we decide to allow it in PvP, uncomment and implement this
//	if (ASWDeathmatchMode())
//	{
//		extern ConVar rd_pvp_50calmg_dmg;
//		flDamage = rd_pvp_50calmg_dmg.GetFloat();
//	}

	if (GetMarine())
	{
		flDamage += MarineSkills()->GetSkillBasedValueByMarine(GetMarine(), ASW_MARINE_SKILL_ACCURACY, ASW_MARINE_SUBSKILL_ACCURACY_RIFLE_DMG);
	}

	return flDamage;
}

// just dry fire by default
void CASW_Weapon_50CalMG::SecondaryAttack()
{
	// Only the player fires this way so we can cast
	CASW_Player *pPlayer = GetCommander();
	if (!pPlayer)
		return;

	CASW_Marine *pMarine = GetMarine();
	if (!pMarine)
		return;

	SendWeaponAnim( ACT_VM_DRYFIRE );
	BaseClass::WeaponSound( EMPTY );
	m_flNextSecondaryAttack = gpGlobals->curtime + 0.5f;
}
