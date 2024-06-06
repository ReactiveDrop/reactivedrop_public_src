#include "cbase.h"
#include "asw_weapon_rifle_burst_shared.h"
#ifdef CLIENT_DLL
#include "c_asw_marine.h"
#include "c_asw_player.h"
#define CASW_Inhabitable_NPC C_ASW_Inhabitable_NPC
#else
#include "asw_marine.h"
#include "asw_marine_resource.h"
#include "asw_player.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


#ifdef RD_7A_WEAPONS
extern ConVar asw_weapon_max_shooting_distance;
extern ConVar asw_weapon_force_scale;
extern ConVar asw_debug_marine_damage;
extern ConVar asw_DebugAutoAim;

IMPLEMENT_NETWORKCLASS_ALIASED( ASW_Weapon_Rifle_Burst, DT_ASW_Weapon_Rifle_Burst );

BEGIN_NETWORK_TABLE( CASW_Weapon_Rifle_Burst, DT_ASW_Weapon_Rifle_Burst )
#ifdef CLIENT_DLL
	RecvPropInt( RECVINFO( m_iBurstShots ) ),
#else
	SendPropInt( SENDINFO( m_iBurstShots ), 8, SPROP_CHANGES_OFTEN | SPROP_UNSIGNED ),
#endif
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CASW_Weapon_Rifle_Burst )
	DEFINE_PRED_FIELD( m_iBurstShots, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()
#else
BEGIN_DATADESC( CASW_Weapon_Rifle_Burst )
	DEFINE_FIELD( m_iBurstShots, FIELD_INTEGER ),
END_DATADESC()
#endif

CASW_Weapon_Rifle_Burst::CASW_Weapon_Rifle_Burst()
{
#ifdef CLIENT_DLL
	m_iTracerShot = 0;
	m_iMuzzleShot = 0;
#endif
	m_iBurstShots = 0;
}

CASW_Weapon_Rifle_Burst::~CASW_Weapon_Rifle_Burst()
{
}

#ifdef CLIENT_DLL
const char *CASW_Weapon_Rifle_Burst::GetTracerEffectName()
{
	if ( m_iTracerShot >= GetBurstCount() )
		m_iTracerShot = 0;

	return GetTracerEffectName( m_iTracerShot++ );
}

const char *CASW_Weapon_Rifle_Burst::GetMuzzleEffectName()
{
	if ( m_iMuzzleShot >= GetBurstCount() )
		m_iMuzzleShot = 0;

	return GetMuzzleEffectName( m_iMuzzleShot++ );
}
#endif

void CASW_Weapon_Rifle_Burst::PrimaryAttack()
{
	if ( UsesClipsForAmmo1() && m_iClip1 == 0 )
	{
		Reload();
		return;
	}

	Assert( m_iBurstShots == 0 );
	m_iBurstShots += GetBurstCount();
	if ( !m_bShotDelayed )
	{
		m_bIsFiring = true;
		m_bShotDelayed = true;
		m_flDelayedFire = m_flNextPrimaryAttack;

		// actual shooting happens in DelayedAttack.
		// but if we wait for that, we're going to be a tick late for the fist shot.
		// ...so we simply don't wait.
		DelayedAttack();
	}
	Assert( m_bIsFiring );
	m_flNextPrimaryAttack += GetFireRate();
}

void CASW_Weapon_Rifle_Burst::DelayedAttack()
{
	if ( UsesClipsForAmmo1() && m_iClip1 == 0 )
	{
		// if we ran out, stop shooting and let the player's next PrimaryAttack decide whether we reload or do nothing
		m_bShotDelayed = false;
		m_iBurstShots = 0;
		return;
	}

	CBaseCombatCharacter *pOwner = GetOwner();
	if ( !pOwner || !pOwner->IsInhabitableNPC() )
		return;

	CASW_Inhabitable_NPC *pNPC = assert_cast< CASW_Inhabitable_NPC * >( pOwner );
	CASW_Player *pPlayer = GetCommander();

	Assert( m_bIsFiring );
	WeaponSound( SINGLE );
	if ( m_iClip1 <= AmmoClickPoint() )
	{
		LowAmmoSound();
	}

	// tell the marine to tell its weapon to draw the muzzle flash
	pNPC->DoMuzzleFlash();

	// sets the animation on the weapon model itself
	SendWeaponAnim( GetPrimaryAttackActivity() );

#ifdef GAME_DLL	// check for turning on lag compensation
	if ( pPlayer && pNPC->IsInhabited() )
	{
		CASW_Lag_Compensation::RequestLagCompensation( pPlayer, pPlayer->GetCurrentUserCommand() );
	}
#endif

	FireBulletsInfo_t info;
	info.m_vecSrc = pNPC->Weapon_ShootPosition();
	CASW_Marine *pMarine = CASW_Marine::AsMarine( pNPC );
	if ( pPlayer && pMarine && pMarine->IsInhabited() )
	{
		info.m_vecDirShooting = pPlayer->GetAutoaimVectorForMarine( pMarine, GetAutoAimAmount(), GetVerticalAdjustOnlyAutoAimAmount() );	// 45 degrees = 0.707106781187
	}
	else
	{
#ifdef CLIENT_DLL
		Msg( "Error, clientside firing of a weapon that's being controlled by an AI marine\n" );
#else
		info.m_vecDirShooting = pNPC->GetActualShootTrajectory( info.m_vecSrc );
#endif
	}

	info.m_iShots = 0;
	float flBurstFireRate = GetBurstFireRate();
	float flRestFireRate = GetFireRate() * GetBurstRestRatio() / ( 1.0f + GetBurstRestRatio() );
	int nShotsPerBurst = GetBurstCount();
	while ( m_flDelayedFire <= gpGlobals->curtime && m_iBurstShots > 0 )
	{
		m_flDelayedFire += flBurstFireRate;

		m_iBurstShots--;
		info.m_iShots++;

		if ( m_iBurstShots % nShotsPerBurst == 0 )
		{
			m_flDelayedFire += flRestFireRate;
		}

		if ( flBurstFireRate <= 0.0f )
		{
			break;
		}
	}

	if ( m_iBurstShots <= 0 )
	{
		m_bShotDelayed = false;
		bool bAttack1, bAttack2, bReload, bOldReload, bOldAttack1;
		GetButtons( bAttack1, bAttack2, bReload, bOldReload, bOldAttack1 );
		if ( !bAttack1 )
		{
			m_bIsFiring = false;
			OnStoppedFiring();
		}
	}

	// Make sure we don't fire more than the amount in the clip
	if ( UsesClipsForAmmo1() )
	{
		info.m_iShots = MIN( info.m_iShots, m_iClip1 );
		m_iClip1 -= info.m_iShots;

#ifdef GAME_DLL
		if ( m_iClip1 <= 0 && pMarine && pMarine->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 )
		{
			pMarine->OnWeaponOutOfAmmo( true );
		}
#endif
	}
	else
	{
		info.m_iShots = MIN( info.m_iShots, pNPC->GetAmmoCount( m_iPrimaryAmmoType ) );
		pNPC->RemoveAmmo( info.m_iShots, m_iPrimaryAmmoType );
	}

	info.m_flDistance = asw_weapon_max_shooting_distance.GetFloat();
	info.m_iAmmoType = m_iPrimaryAmmoType;
	info.m_iTracerFreq = 1; // asw tracer test everytime
	info.m_flDamageForceScale = asw_weapon_force_scale.GetFloat();

	info.m_vecSpread = GetBulletSpread();
	info.m_flDamage = GetWeaponDamage();
#ifndef CLIENT_DLL
	if ( asw_debug_marine_damage.GetBool() )
		Msg( "Weapon dmg = %f\n", info.m_flDamage );
	Assert( SupportsDamageModifiers() );
	if ( pMarine && pMarine->GetMarineResource() )
		pMarine->GetMarineResource()->OnFired_ScaleDamage( info );
	if ( asw_DebugAutoAim.GetBool() )
	{
		NDebugOverlay::Line( info.m_vecSrc, info.m_vecSrc + info.m_vecDirShooting * info.m_flDistance, 64, 0, 64, true, 1.0 );
	}
#endif

	pNPC->FireBullets( info );

	// increment shooting stats
#ifndef CLIENT_DLL
	if ( pMarine && pMarine->GetMarineResource() )
	{
		pMarine->GetMarineResource()->UsedWeapon( this, info.m_iShots );
		pMarine->OnWeaponFired( this, info.m_iShots );
	}
#endif
}

void CASW_Weapon_Rifle_Burst::ItemPostFrame()
{
	BaseClass::ItemPostFrame();

	if ( m_iBurstShots )
	{
		m_bIsFiring = true;
	}
}

float CASW_Weapon_Rifle_Burst::GetFireRate()
{
	return BaseClass::GetFireRate() * GetBurstCount();
}

float CASW_Weapon_Rifle_Burst::GetBurstFireRate()
{
	return BaseClass::GetFireRate() / ( 1.0f + GetBurstRestRatio() );
}

bool CASW_Weapon_Rifle_Burst::CanHolster()
{
	if ( !HolsterCancelsBurstFire() && m_iBurstShots > 0 )
		return false;

	return BaseClass::CanHolster();
}

void CASW_Weapon_Rifle_Burst::ClearIsFiring()
{
	BaseClass::ClearIsFiring();

	m_iBurstShots = 0;
	m_bShotDelayed = false;
}
#endif
