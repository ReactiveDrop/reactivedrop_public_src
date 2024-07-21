#include "cbase.h"
#include "asw_weapon_grenades.h"
#include "in_buttons.h"

#ifdef CLIENT_DLL
#include "c_asw_player.h"
#include "c_asw_weapon.h"
#include "c_asw_marine.h"
#include "prediction.h"
#else
#include "asw_marine.h"
#include "asw_player.h"
#include "asw_weapon.h"
#include "npcevent.h"
#include "shot_manipulator.h"
#include "asw_grenade_cluster.h"
#include "asw_marine_speech.h"
#include "asw_gamerules.h"
#endif
#include "asw_marine_skills.h"
#include "asw_util_shared.h"
#include "asw_weapon_parse.h"
#include "asw_equipment_list.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define ASW_FLARES_FASTEST_REFIRE_TIME		0.1f

IMPLEMENT_NETWORKCLASS_ALIASED( ASW_Weapon_Grenades, DT_ASW_Weapon_Grenades )

BEGIN_NETWORK_TABLE( CASW_Weapon_Grenades, DT_ASW_Weapon_Grenades )
#ifdef CLIENT_DLL
	// recvprops
#else
	// sendprops
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CASW_Weapon_Grenades )
	
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( asw_weapon_grenades, CASW_Weapon_Grenades );
PRECACHE_WEAPON_REGISTER(asw_weapon_grenades);

#ifndef CLIENT_DLL
extern ConVar asw_debug_marine_damage;
//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CASW_Weapon_Grenades )
END_DATADESC()

#endif /* not client */

float CASW_Weapon_Grenades::GetWeaponBaseDamageOverride()
{
	extern ConVar rd_grenades_dmg_base;
	return rd_grenades_dmg_base.GetFloat();
}
int CASW_Weapon_Grenades::GetWeaponSkillId()
{
	return ASW_MARINE_SKILL_GRENADES;
}
int CASW_Weapon_Grenades::GetWeaponSubSkillId()
{
	return ASW_MARINE_SUBSKILL_GRENADE_CLUSTER_DMG;
}

#ifndef CLIENT_DLL
float CASW_Weapon_Grenades::GetBoomDamage( CASW_Marine *pMarine )
{
	float flBaseDamage = g_ASWEquipmentList.GetExtra( ASW_EQUIP_GRENADES )->m_flBaseDamage;

	extern ConVar rd_grenades_dmg_base;
	if ( rd_grenades_dmg_base.GetFloat() )
	{
		flBaseDamage = rd_grenades_dmg_base.GetFloat();
	}

	return flBaseDamage + MarineSkills()->GetSkillBasedValueByMarine( pMarine, ASW_MARINE_SKILL_GRENADES, ASW_MARINE_SUBSKILL_GRENADE_CLUSTER_DMG );
}

float CASW_Weapon_Grenades::GetBoomRadius( CASW_Marine *pMarine )
{
	return MarineSkills()->GetSkillBasedValueByMarine( pMarine, ASW_MARINE_SKILL_GRENADES, ASW_MARINE_SUBSKILL_GRENADE_RADIUS );
}

#endif

void CASW_Weapon_Grenades::DelayedAttack( void )
{
	m_bShotDelayed = false;
	
	CASW_Player *pPlayer = GetCommander();
	if ( !pPlayer )
		return;

	CASW_Marine *pMarine = GetMarine();
	if ( !pMarine || pMarine->GetWaterLevel() == 3 )
		return;

#ifndef CLIENT_DLL
	Vector vecSrc = pMarine->GetOffhandThrowSource();
	Vector vecDest = pMarine->GetOffhandThrowDest();
	Vector newVel = UTIL_LaunchVector( vecSrc, vecDest, GetThrowGravity() ) * 28.0f;

	float fGrenadeRadius = GetBoomRadius( pMarine );
	float fGrenadeDamage = GetBoomDamage( pMarine );
	int iClusters = MarineSkills()->GetSkillBasedValueByMarine(pMarine, ASW_MARINE_SKILL_GRENADES, ASW_MARINE_SUBSKILL_GRENADE_CLUSTERS);
	if (asw_debug_marine_damage.GetBool())
	{
		Msg("Grenade damage = %f radius = %f clusters = %d\n", fGrenadeDamage, fGrenadeRadius, iClusters);
	}
	pMarine->GetMarineSpeech()->Chatter(CHATTER_GRENADE);


	if (ASWGameRules())
		ASWGameRules()->m_fLastFireTime = gpGlobals->curtime;

	CASW_Grenade_Cluster *pGrenade = CASW_Grenade_Cluster::Cluster_Grenade_Create( 
		fGrenadeDamage,
		fGrenadeRadius,
		iClusters,
		vecSrc, pMarine->EyeAngles(), newVel, AngularImpulse(0,0,0), pMarine, this );

	if ( pGrenade )
	{
		pGrenade->SetGravity( GetThrowGravity() );
	}
#endif
	// decrement ammo
	m_iClip1 -= 1;

#ifndef CLIENT_DLL
	DestroyIfEmpty( true );
	pMarine->OnWeaponFired( this, 1 );
#endif

	m_flSoonestPrimaryAttack = gpGlobals->curtime + ASW_FLARES_FASTEST_REFIRE_TIME;
	if (m_iClip1 > 0)		// only force the fire wait time if we have ammo for another shot
		m_flNextPrimaryAttack = gpGlobals->curtime + GetFireRate();
	else
		m_flNextPrimaryAttack = gpGlobals->curtime;
}

void CASW_Weapon_Grenades::Precache()
{	
	BaseClass::Precache();	

#ifndef CLIENT_DLL
	UTIL_PrecacheOther( "asw_grenade_cluster" );
#endif
}
