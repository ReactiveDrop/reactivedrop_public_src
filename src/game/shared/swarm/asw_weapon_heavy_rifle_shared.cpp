#include "cbase.h"
#include "asw_weapon_heavy_rifle_shared.h"
#include "in_buttons.h"

#ifdef CLIENT_DLL
#include "c_asw_player.h"
#include "c_asw_weapon.h"
#include "c_asw_marine.h"
#define CASW_Marine C_ASW_Marine
#else
#include "asw_marine.h"
#include "asw_player.h"
#include "asw_weapon.h"
#include "npcevent.h"
#include "shot_manipulator.h"
#include "asw_fail_advice.h"
#include "effect_dispatch_data.h"
#endif
#include "asw_marine_skills.h"
#include "asw_weapon_parse.h"
#include "particle_parse.h"
#include "asw_deathmatch_mode_light.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static const char *s_pFastFireThink = "HeavyRifleFastFireThink";

ConVar rd_heavy_rifle_bigalien_dmg_scale( "rd_heavy_rifle_bigalien_dmg_scale", "1.3", FCVAR_CHEAT | FCVAR_REPLICATED, "Used to scale up heavy rifle's damage against Shieldbug, Mortarbug, Harvester, Drone Uber, Queen" );
ConVar rd_heavy_rifle_fastfire_dmg_scale( "rd_heavy_rifle_fastfire_dmg_scale", "2.0", FCVAR_CHEAT | FCVAR_REPLICATED, "Used to scale up heavy rifle's damage during fast fire mode" );
ConVar rd_heavy_rifle_fastfire_duration(  "rd_heavy_rifle_fastfire_duration",  "3.0", FCVAR_CHEAT | FCVAR_REPLICATED, "Number of seconds fast fire lasts" );
ConVar rd_heavy_rifle_fastfire_fire_rate_multiplier( "rd_heavy_rifle_fastfire_fire_rate_multiplier", "0.7", FCVAR_CHEAT | FCVAR_REPLICATED, "A multiplier which is applied to rifle's fire rate during fast fire mode" );

IMPLEMENT_NETWORKCLASS_ALIASED( ASW_Weapon_Heavy_Rifle, DT_ASW_Weapon_Heavy_Rifle )

BEGIN_NETWORK_TABLE( CASW_Weapon_Heavy_Rifle, DT_ASW_Weapon_Heavy_Rifle )
#ifdef CLIENT_DLL
	// recvprops
	RecvPropBool( RECVINFO( m_bFastFire ) ),
#else
	// sendprops
	SendPropBool( SENDINFO( m_bFastFire ) ),
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CASW_Weapon_Heavy_Rifle )
	
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( asw_weapon_heavy_rifle, CASW_Weapon_Heavy_Rifle );
PRECACHE_WEAPON_REGISTER(asw_weapon_heavy_rifle);

#ifndef CLIENT_DLL
extern ConVar asw_debug_marine_damage;
//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CASW_Weapon_Heavy_Rifle )
	
END_DATADESC()

#endif /* not client */

CASW_Weapon_Heavy_Rifle::CASW_Weapon_Heavy_Rifle()
{

}


CASW_Weapon_Heavy_Rifle::~CASW_Weapon_Heavy_Rifle()
{

}

void CASW_Weapon_Heavy_Rifle::Precache()
{
	BaseClass::Precache();

	PrecacheScriptSound( "ASW_Weapon_HeavyRifle.ChargeOn" );
	PrecacheScriptSound( "ASW_Weapon_HeavyRifle.ChargeOff" );
	PrecacheParticleSystem( "buffgrenade_attach_arc" );
	PrecacheParticleSystem( "mining_laser_exhaust" );
}


float CASW_Weapon_Heavy_Rifle::GetWeaponDamage()
{
	float flDamage = GetWeaponInfo()->m_flBaseDamage;

	extern ConVar rd_heavy_rifle_dmg_base;
	if ( rd_heavy_rifle_dmg_base.GetFloat() > 0 )
	{
		flDamage = rd_heavy_rifle_dmg_base.GetFloat();
	}

	if ( GetMarine() )
	{
		flDamage += MarineSkills()->GetSkillBasedValueByMarine(GetMarine(), ASW_MARINE_SKILL_ACCURACY, ASW_MARINE_SUBSKILL_ACCURACY_HEAVY_RIFLE_DMG);
	}

	if ( !ASWDeathmatchMode() && m_bFastFire )
	{
		flDamage *= rd_heavy_rifle_fastfire_dmg_scale.GetFloat();
	}

	return flDamage;
}

// just dry fire by default
void CASW_Weapon_Heavy_Rifle::SecondaryAttack()
{
	// Only the player fires this way so we can cast
	CASW_Player *pPlayer = GetCommander();
	if ( !pPlayer )
		return;

	CASW_Marine *pMarine = GetMarine();
	if ( !pMarine )
		return;

	//Must have ammo
	bool bUsesSecondary = UsesSecondaryAmmo();
	bool bUsesClips = UsesClipsForAmmo2();
	int iAmmoCount = pMarine->GetAmmoCount(m_iSecondaryAmmoType);
	if ( ( bUsesSecondary && ( ( bUsesClips && m_iClip2 <= 0) || ( !bUsesClips && iAmmoCount <= 0 ) ) ) || m_bInReload || m_bFastFire )
	{
		SendWeaponAnim( ACT_VM_DRYFIRE );
		BaseClass::WeaponSound( EMPTY );
		m_flNextSecondaryAttack = gpGlobals->curtime + 0.5f;
		return;
	}

	m_iClip2--;
	m_bFastFire = true;
	EmitSound( "ASW_Weapon_HeavyRifle.ChargeOn" );
	m_flNextSecondaryAttack = gpGlobals->curtime + 0.5f;
	SetContextThink( &CASW_Weapon_Heavy_Rifle::StopFastFire, gpGlobals->curtime + rd_heavy_rifle_fastfire_duration.GetFloat(), s_pFastFireThink );
}

float CASW_Weapon_Heavy_Rifle::GetFireRate()
{
	float flRate = GetWeaponInfo()->m_flFireRate;

	if ( m_bFastFire )
	{
		flRate *= rd_heavy_rifle_fastfire_fire_rate_multiplier.GetFloat();
	}

	return flRate;
}

const Vector& CASW_Weapon_Heavy_Rifle::GetBulletSpread( void )
{
	static Vector vec15degrees = Vector( 0.13053, 0.02618, 0.13053 );	// VECTOR_CONE_15DEGREES with y as VECTOR_CONE_3DEGREES
	static Vector vec6degrees = Vector( 0.05234, 0.02618, 0.05234 );		// VECTOR_CONE_6DEGREES with y as VECTOR_CONE_3DEGREES

	CASW_Marine *marine = GetMarine();

	if ( marine )
	{
		if ( marine->GetAbsVelocity() == Vector( 0, 0, 0 ) && marine->m_bWalking && !m_bFastFire )
			return vec6degrees;
	}
	return vec15degrees;
}

void CASW_Weapon_Heavy_Rifle::StopFastFire()
{
	m_bFastFire = false;

	if ( !m_bInReload )
	{
		m_flNextPrimaryAttack = gpGlobals->curtime + 1.0f;
		m_flNextSecondaryAttack = gpGlobals->curtime + 1.0f;
	}

	DispatchParticleEffect( "mining_laser_exhaust", PATTACH_POINT_FOLLOW, this, "muzzle" );

	EmitSound( "ASW_Weapon_HeavyRifle.ChargeOff" );
}

#ifdef CLIENT_DLL
void CASW_Weapon_Heavy_Rifle::ClientThink()
{
	BaseClass::ClientThink();

	if ( m_bFastFire != m_hFastFireParticle.IsValid() )
	{
		if ( m_bFastFire )
		{
			m_hFastFireParticle.Set( ParticleProp()->Create( "buffgrenade_attach_arc", PATTACH_POINT_FOLLOW, "eject1" ) );
			ParticleProp()->AddControlPoint( m_hFastFireParticle, 1, this, PATTACH_POINT_FOLLOW, "muzzle" );
		}
		else
		{
			ParticleProp()->StopEmissionAndDestroyImmediately( m_hFastFireParticle.GetObject() );
			m_hFastFireParticle.Set( NULL );
		}
	}
}
#endif
