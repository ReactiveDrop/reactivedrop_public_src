#include "cbase.h"
#include "asw_weapon_freeze_grenades.h"
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
#include "asw_grenade_freeze.h"
#include "asw_marine_speech.h"
#include "asw_gamerules.h"
#endif
#include "asw_marine_skills.h"
#include "asw_util_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define ASW_FLARES_FASTEST_REFIRE_TIME		0.1f

extern ConVar asw_grenade_throw_delay;

IMPLEMENT_NETWORKCLASS_ALIASED( ASW_Weapon_Freeze_Grenades, DT_ASW_Weapon_Freeze_Grenades )

BEGIN_NETWORK_TABLE( CASW_Weapon_Freeze_Grenades, DT_ASW_Weapon_Freeze_Grenades )
#ifdef CLIENT_DLL
	// recvprops
#else
	// sendprops
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CASW_Weapon_Freeze_Grenades )
	
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( asw_weapon_freeze_grenades, CASW_Weapon_Freeze_Grenades );
PRECACHE_WEAPON_REGISTER( asw_weapon_freeze_grenades );

#ifndef CLIENT_DLL
extern ConVar asw_debug_marine_damage;
//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CASW_Weapon_Freeze_Grenades )
END_DATADESC()

#endif /* not client */

#define FLARE_PROJECTILE_AIR_VELOCITY	400

#ifndef CLIENT_DLL
float CASW_Weapon_Freeze_Grenades::GetBoomRadius( CASW_Marine *pMarine )
{
	return MarineSkills()->GetSkillBasedValueByMarine(pMarine, ASW_MARINE_SKILL_GRENADES, ASW_MARINE_SUBSKILL_GRENADE_FREEZE_RADIUS);
}
#endif

void CASW_Weapon_Freeze_Grenades::DelayedAttack( void )
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
	if (asw_debug_marine_damage.GetBool())
	{
		Msg( "Freeze grenade radius = %f \n", fGrenadeRadius );
	}
	pMarine->GetMarineSpeech()->Chatter( CHATTER_GRENADE );

	// freeze aliens completely, plus the freeze duration
	float flFreezeAmount = 1.0f + MarineSkills()->GetSkillBasedValueByMarine(pMarine, ASW_MARINE_SKILL_GRENADES, ASW_MARINE_SUBSKILL_GRENADE_FREEZE_DURATION);

	if (ASWGameRules())
		ASWGameRules()->m_fLastFireTime = gpGlobals->curtime;

	CASW_Grenade_Freeze *pGrenade = CASW_Grenade_Freeze::Freeze_Grenade_Create( 
		1.0f,
		flFreezeAmount,
		fGrenadeRadius,
		0,
		vecSrc, pMarine->EyeAngles(), newVel, AngularImpulse(0,0,0), pMarine, this );
	if ( pGrenade )
	{
		pGrenade->SetGravity( GetThrowGravity() );
		pGrenade->SetExplodeOnWorldContact( true );
	}
	
#endif
		// decrement ammo
	m_iClip1 -= 1;

#ifndef CLIENT_DLL
	DestroyIfEmpty( true );
#endif

	m_flSoonestPrimaryAttack = gpGlobals->curtime + ASW_FLARES_FASTEST_REFIRE_TIME;
	if (m_iClip1 > 0)		// only force the fire wait time if we have ammo for another shot
		m_flNextPrimaryAttack = gpGlobals->curtime + GetFireRate();
	else
		m_flNextPrimaryAttack = gpGlobals->curtime;
}

void CASW_Weapon_Freeze_Grenades::Precache()
{	
	BaseClass::Precache();

#ifndef CLIENT_DLL
	UTIL_PrecacheOther( "asw_grenade_freeze" );
#endif
}
