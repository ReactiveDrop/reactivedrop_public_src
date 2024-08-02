#include "cbase.h"
#include "asw_weapon_incendiary_grenades_shared.h"
#ifdef CLIENT_DLL
#include "c_asw_marine.h"
#else
#include "asw_grenade_vindicator.h"
#include "asw_marine.h"
#include "asw_marine_speech.h"
#endif
#include "asw_gamerules.h"
#include "asw_util_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


#ifdef RD_7A_WEAPONS
#define ASW_FLARES_FASTEST_REFIRE_TIME		0.1f

extern ConVar asw_debug_marine_damage;

IMPLEMENT_NETWORKCLASS_ALIASED( ASW_Weapon_Incendiary_Grenades, DT_ASW_Weapon_Incendiary_Grenades );

BEGIN_NETWORK_TABLE( CASW_Weapon_Incendiary_Grenades, DT_ASW_Weapon_Incendiary_Grenades )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CASW_Weapon_Incendiary_Grenades )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( asw_weapon_incendiary_grenades, CASW_Weapon_Incendiary_Grenades );
PRECACHE_WEAPON_REGISTER( asw_weapon_incendiary_grenades );

#ifndef CLIENT_DLL
BEGIN_DATADESC( CASW_Weapon_Incendiary_Grenades )
END_DATADESC()
#endif

void CASW_Weapon_Incendiary_Grenades::Precache()
{
	BaseClass::Precache();

#ifndef CLIENT_DLL
	UTIL_PrecacheOther( "asw_grenade_vindicator" );
#endif
}

void CASW_Weapon_Incendiary_Grenades::DelayedAttack( void )
{
	m_bShotDelayed = false;

	CASW_Player *pPlayer = GetCommander();
	if ( !pPlayer )
		return;

	CASW_Marine *pMarine = GetMarine();
	if ( !pMarine || pMarine->GetWaterLevel() == WL_Eyes )
		return;

#ifndef CLIENT_DLL
	Vector vecSrc = pMarine->GetOffhandThrowSource();
	Vector vecDest = pMarine->GetOffhandThrowDest();
	Vector newVel = UTIL_LaunchVector( vecSrc, vecDest, GetThrowGravity() ) * 28.0f;

	pMarine->GetMarineSpeech()->Chatter( CHATTER_GRENADE );

	if ( ASWGameRules() )
		ASWGameRules()->m_fLastFireTime = gpGlobals->curtime;

	float fGrenadeDamage = MarineSkills()->GetSkillBasedValueByMarine( pMarine, ASW_MARINE_SKILL_GRENADES, ASW_MARINE_SUBSKILL_GRENADE_INCENDIARY_DMG );
	float fGrenadeRadius = MarineSkills()->GetSkillBasedValueByMarine( pMarine, ASW_MARINE_SKILL_GRENADES, ASW_MARINE_SUBSKILL_GRENADE_RADIUS );
	if ( asw_debug_marine_damage.GetBool() )
	{
		Msg( "Grenade damage = %f radius = %f\n", fGrenadeDamage, fGrenadeRadius );
	}

	CASW_Grenade_Vindicator *pGrenade = CASW_Grenade_Vindicator::Vindicator_Grenade_Create( fGrenadeDamage, fGrenadeRadius,
		vecSrc, pMarine->EyeAngles(), newVel, AngularImpulse( 0, 0, 0 ), pMarine, this );
	if ( pGrenade )
	{
		pGrenade->SetGravity( GetThrowGravity() );
	}

#endif
	// decrement ammo
	m_iClip1 -= 1;

#ifndef CLIENT_DLL
	DestroyIfEmpty( true );
#endif

	m_flSoonestPrimaryAttack = gpGlobals->curtime + ASW_FLARES_FASTEST_REFIRE_TIME;
	if ( m_iClip1 > 0 )		// only force the fire wait time if we have ammo for another shot
		m_flNextPrimaryAttack = gpGlobals->curtime + GetFireRate();
	else
		m_flNextPrimaryAttack = gpGlobals->curtime;
}
#endif
