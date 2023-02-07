#include "cbase.h"

#include "asw_weapon_combat_rifle_shared.h"
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

IMPLEMENT_NETWORKCLASS_ALIASED(ASW_Weapon_CombatRifle, DT_ASW_Weapon_CombatRifle)

BEGIN_NETWORK_TABLE(CASW_Weapon_CombatRifle, DT_ASW_Weapon_CombatRifle)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CASW_Weapon_CombatRifle)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(asw_weapon_combat_rifle, CASW_Weapon_CombatRifle);
PRECACHE_WEAPON_REGISTER(asw_weapon_combat_rifle);

#ifndef CLIENT_DLL
BEGIN_DATADESC(CASW_Weapon_CombatRifle)
END_DATADESC()
#endif


#define PELLET_AIR_VELOCITY	2500
#define PELLET_WATER_VELOCITY	1500
static const int NUM_SHOTGUN_PELLETS = 14;

extern ConVar asw_weapon_max_shooting_distance;
extern ConVar asw_weapon_force_scale;
#ifdef GAME_DLL
extern ConVar asw_debug_marine_damage;
extern ConVar asw_DebugAutoAim;
#endif 

CASW_Weapon_CombatRifle::CASW_Weapon_CombatRifle()
{
}

CASW_Weapon_CombatRifle::~CASW_Weapon_CombatRifle()
{

}

void CASW_Weapon_CombatRifle::Precache()
{
	PrecacheModel("swarm/sprites/whiteglow1.vmt");
	PrecacheModel("swarm/sprites/greylaser1.vmt");
	PrecacheScriptSound("ASW_Weapon.Empty");
	PrecacheScriptSound("ASW_Weapon.Reload3");
	//PrecacheScriptSound("ASW_Weapon_CombatRifle.SingleFP");
	//PrecacheScriptSound("ASW_Weapon_CombatRifle.Single");


	BaseClass::Precache();
}

float CASW_Weapon_CombatRifle::GetWeaponDamage()
{
	//float flDamage = 18.0f;
	float flDamage = GetWeaponInfo()->m_flBaseDamage;

	extern ConVar rd_combat_rifle_dmg_base;
	if ( rd_combat_rifle_dmg_base.GetFloat() > 0 )
	{
		flDamage = rd_combat_rifle_dmg_base.GetFloat();
	}

	if (GetMarine())
	{
		flDamage += MarineSkills()->GetSkillBasedValueByMarine(GetMarine(), ASW_MARINE_SKILL_ACCURACY, ASW_MARINE_SUBSKILL_ACCURACY_RIFLE_DMG);
	}

	return flDamage;
}

void CASW_Weapon_CombatRifle::SecondaryAttack()
{
	CASW_Player *pPlayer = GetCommander();
	if (!pPlayer)
		return;

	CASW_Marine *pMarine = GetMarine();
	if (!pMarine)
		return;
	
	//Must have ammo
	bool bUsesSecondary = UsesSecondaryAmmo();
	bool bUsesClips = UsesClipsForAmmo2();
	int iAmmoCount = pMarine->GetAmmoCount(m_iSecondaryAmmoType);
	bool bInWater = (pMarine->GetWaterLevel() == 3);
	if ((bUsesSecondary &&
		((bUsesClips && m_iClip2 <= 0) ||
		(!bUsesClips && iAmmoCount <= 0)
		))
		|| bInWater || m_bInReload)
	{
		SendWeaponAnim(ACT_VM_DRYFIRE);
		BaseClass::WeaponSound(EMPTY);
		m_flNextSecondaryAttack = gpGlobals->curtime + 0.5f;
		return;
	}

	// MUST call sound before removing a round from the clip of a CMachineGun
	WeaponSound(SPECIAL1);
		
	m_bIsFiring = true;

	// tell the marine to tell its weapon to draw the muzzle flash
	pMarine->DoMuzzleFlash();

	// sets the animation on the weapon model iteself
	//SendWeaponAnim(GetPrimaryAttackActivity());

#ifdef GAME_DLL	// check for turning on lag compensation
	if (pPlayer && pMarine->IsInhabited())
	{
		CASW_Lag_Compensation::RequestLagCompensation(pPlayer, pPlayer->GetCurrentUserCommand());
	}
#endif

	Vector vecSrc = pMarine->Weapon_ShootPosition();
	// hull trace out to this shoot position, so we can be sure we're not firing from over an alien's head
	trace_t tr;
	CTraceFilterSimple tracefilter(pMarine, COLLISION_GROUP_NONE);
	Vector vecMarineMiddle(pMarine->GetAbsOrigin());
	vecMarineMiddle.z = vecSrc.z;
	AI_TraceHull(vecMarineMiddle, vecSrc, Vector(-10, -10, -20), Vector(10, 10, 10), MASK_SHOT, &tracefilter, &tr);
	vecSrc = tr.endpos;

	Vector vecAiming = vec3_origin;
	if ( pMarine->IsInhabited() )
	{
		vecAiming = pPlayer->GetAutoaimVectorForMarine(pMarine, GetAutoAimAmount(), GetVerticalAdjustOnlyAutoAimAmount());	// 45 degrees = 0.707106781187
	}
	else
	{
#ifndef CLIENT_DLL
		vecAiming = pMarine->GetActualShootTrajectory(vecSrc);
#endif
	}

#ifndef CLIENT_DLL
	if (asw_DebugAutoAim.GetBool())
	{
		NDebugOverlay::Line(vecSrc, vecSrc + vecAiming * asw_weapon_max_shooting_distance.GetFloat(), 64, 0, 64, false, 120.0);
	}
#endif
	// We can't use num_pellets field from asw_weapon_combat_rifle.txt
	// because it changes the "Damage" UI value. E.g. instead of 5 it shows 
	// 35 if we set num_pellets 7
	// so we harcode the value here for now
	FireBulletsInfo_t info( NUM_SHOTGUN_PELLETS, vecSrc, vecAiming, GetAngularBulletSpread(), asw_weapon_max_shooting_distance.GetFloat(), m_iSecondaryAmmoType, false );
	info.m_pAttacker = pMarine;
	info.m_iTracerFreq = 1;
	info.m_nFlags = FIRE_BULLETS_NO_PIERCING_SPARK | FIRE_BULLETS_HULL | FIRE_BULLETS_ANGULAR_SPREAD;
	info.m_flDamage = GetWeaponDamage() * 6;
	info.m_flDamageForceScale = asw_weapon_force_scale.GetFloat();
#ifndef CLIENT_DLL
	if (asw_debug_marine_damage.GetBool())
		Msg("Weapon dmg = %f\n", info.m_flDamage);
	pMarine->GetMarineResource()->OnFired_ScaleDamage( info );
#endif

	pMarine->FirePenetratingBullets(info, 0, 1, 0, false);

	// increment shooting stats
#ifndef CLIENT_DLL
	if (pMarine && pMarine->GetMarineResource())
	{
		pMarine->GetMarineResource()->UsedWeapon( this, 1 );
		pMarine->OnWeaponFired( this, 1, true );
	}
#endif

	// decrement ammo
	if (bUsesClips)
	{
		m_iClip2 -= 1;
	}
	else
	{
		pMarine->RemoveAmmo(1, m_iSecondaryAmmoType);
	}

	// Can shoot again immediately
	m_flNextPrimaryAttack = gpGlobals->curtime + 0.5f;

	m_flNextSecondaryAttack = gpGlobals->curtime + 1.0f;
}

