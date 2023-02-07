#include "cbase.h"
#include "asw_weapon_ricochet_shared.h"
#include "in_buttons.h"

#ifdef CLIENT_DLL
#define CASW_Weapon C_ASW_Weapon
#define CASW_Marine C_ASW_Marine
#define CBasePlayer C_BasePlayer
#include "c_asw_player.h"
#include "c_asw_weapon.h"
#include "c_asw_marine.h"
#include "c_asw_marine_resource.h"
#include "precache_register.h"
#include "c_te_effect_dispatch.h"
#include "iviewrender_beams.h"			// laser beam
#else
#include "asw_lag_compensation.h"
#include "asw_marine.h"
#include "asw_player.h"
#include "asw_weapon.h"
#include "npcevent.h"
#include "te_effect_dispatch.h"
#include "asw_marine_resource.h"
#include "asw_marine_speech.h"
#include "decals.h"
#include "ammodef.h"
#include "asw_weapon_ammo_bag_shared.h"
#include "asw_rocket.h"
#endif
#include "asw_bouncing_pellet_shared.h"
#include "shot_manipulator.h"
#include "asw_marine_skills.h"
#include "fmtstr.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED( ASW_Weapon_Ricochet, DT_ASW_Weapon_Ricochet )

BEGIN_NETWORK_TABLE( CASW_Weapon_Ricochet, DT_ASW_Weapon_Ricochet )
#ifdef CLIENT_DLL
	// recvprops
#else
	// sendprops	
#endif
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CASW_Weapon_Ricochet )	
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( asw_weapon_ricochet, CASW_Weapon_Ricochet );
PRECACHE_WEAPON_REGISTER( asw_weapon_ricochet );

extern ConVar asw_weapon_max_shooting_distance;

#ifndef CLIENT_DLL
extern ConVar asw_debug_marine_damage;
#else
extern ConVar asw_laser_sight_min_distance;
#endif

ConVar rd_ricochet_bounces( "rd_ricochet_bounces", "3", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar rd_ricochet_shotgun_bounces( "rd_ricochet_shotgun_bounces", "1", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar rd_ricochet_shotgun_pellets( "rd_ricochet_shotgun_pellets", "7", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar rd_ricochet_shotgun_damage_scale( "rd_ricochet_shotgun_damage_scale", "0.3", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar rd_ricochet_shotgun_spread( "rd_ricochet_shotgun_spread", "20", FCVAR_REPLICATED | FCVAR_CHEAT );

CASW_Weapon_Ricochet::CASW_Weapon_Ricochet()
{
}

CASW_Weapon_Ricochet::~CASW_Weapon_Ricochet()
{
}

void CASW_Weapon_Ricochet::Precache()
{	
	BaseClass::Precache();

	PrecacheParticleSystem( "weapon_laser_sight_ricochet" );
}

void CASW_Weapon_Ricochet::PrimaryAttack()
{
	// If my clip is empty (and I use clips) start reload
	if ( UsesClipsForAmmo1() && !m_iClip1 )
	{
		Reload();
		return;
	}

	CASW_Player *pPlayer = GetCommander();
	CASW_Marine *pMarine = GetMarine();

	bool bAttack1, bAttack2, bReload, bOldReload, bOldAttack1;
	GetButtons( bAttack1, bAttack2, bReload, bOldReload, bOldAttack1 );

	if ( pMarine )		// firing from a marine
	{
		m_bIsFiring = true;

		// MUST call sound before removing a round from the clip of a CMachineGun
		WeaponSound( SINGLE );

		if ( m_iClip1 <= AmmoClickPoint() )
		{
			LowAmmoSound();
		}

		// sets the animation on the weapon model iteself
		SendWeaponAnim( GetPrimaryAttackActivity() );

#ifdef GAME_DLL	// check for turning on lag compensation
		if ( pPlayer && pMarine->IsInhabited() )
		{
			CASW_Lag_Compensation::RequestLagCompensation( pPlayer, pPlayer->GetCurrentUserCommand() );
		}
#endif

		FireBulletsInfo_t info;
		info.m_vecSrc = pMarine->Weapon_ShootPosition();
		if ( pPlayer && pMarine->IsInhabited() )
		{
			info.m_vecDirShooting = pPlayer->GetAutoaimVectorForMarine( pMarine, GetAutoAimAmount(), GetVerticalAdjustOnlyAutoAimAmount() );	// 45 degrees = 0.707106781187
		}
		else
		{
#ifdef CLIENT_DLL
			Msg( "Error, clientside firing of a weapon that's being controlled by an AI marine\n" );
#else
			info.m_vecDirShooting = pMarine->GetActualShootTrajectory( info.m_vecSrc );
#endif
		}

		info.m_flDistance = asw_weapon_max_shooting_distance.GetFloat();
		info.m_iAmmoType = m_iPrimaryAmmoType;
		info.m_iTracerFreq = 1;  // asw tracer test everytime

		// To make the firing framerate independent, we may have to fire more than one bullet here on low-framerate systems, 
		// especially if the weapon we're firing has a really fast rate of fire.
		info.m_iShots = 0;
		float fireRate = bAttack2 ? 1.0f : GetFireRate();
		while ( m_flNextPrimaryAttack <= gpGlobals->curtime )
		{
			m_flNextPrimaryAttack = m_flNextPrimaryAttack + fireRate;
			info.m_iShots++;
			if ( !fireRate )
				break;
		}

		// Make sure we don't fire more than the amount in the clip
		if ( UsesClipsForAmmo1() )
		{
			info.m_iShots = MIN( info.m_iShots, m_iClip1 );
			m_iClip1 -= info.m_iShots;

#ifdef GAME_DLL
			if ( m_iClip1 <= 0 && pMarine->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 )
			{
				// check he doesn't have ammo in an ammo bay
				CASW_Weapon_Ammo_Bag *pAmmoBag = NULL;
				CASW_Weapon *pWeapon = pMarine->GetASWWeapon( 0 );
				if ( pWeapon && pWeapon->Classify() == CLASS_ASW_AMMO_BAG )
					pAmmoBag = assert_cast< CASW_Weapon_Ammo_Bag * >( pWeapon );

				if ( !pAmmoBag )
				{
					pWeapon = pMarine->GetASWWeapon( 1 );
					if ( pWeapon && pWeapon->Classify() == CLASS_ASW_AMMO_BAG )
						pAmmoBag = assert_cast< CASW_Weapon_Ammo_Bag * >( pWeapon );
				}
				if ( !pAmmoBag || !pAmmoBag->CanGiveAmmoToWeapon( this ) )
					pMarine->OnWeaponOutOfAmmo( true );
			}
#endif
		}
		else
		{
			info.m_iShots = MIN( info.m_iShots, pMarine->GetAmmoCount( m_iPrimaryAmmoType ) );
			pMarine->RemoveAmmo( info.m_iShots, m_iPrimaryAmmoType );
		}

		int nBounces;

		if ( bAttack2 )
		{
			info.m_vecSpread.x = info.m_vecSpread.y = info.m_vecSpread.z = rd_ricochet_shotgun_spread.GetFloat();
			info.m_nFlags = FIRE_BULLETS_NO_PIERCING_SPARK | FIRE_BULLETS_HULL | FIRE_BULLETS_ANGULAR_SPREAD;

			info.m_iShots *= rd_ricochet_shotgun_pellets.GetInt();
			info.m_flDamage = MAX( GetWeaponDamage() * rd_ricochet_shotgun_damage_scale.GetFloat(), 1.0f );

			nBounces = rd_ricochet_shotgun_bounces.GetInt();
		}
		else
		{
			info.m_vecSpread = GetBulletSpread();
			info.m_flDamage = GetWeaponDamage();

			nBounces = rd_ricochet_bounces.GetInt();
		}

#ifndef CLIENT_DLL
		if ( asw_debug_marine_damage.GetBool() )
			Msg( "Weapon dmg = %f\n", info.m_flDamage );
		pMarine->GetMarineResource()->OnFired_ScaleDamage( info );
#endif

		pMarine->FireBouncingBullets( info, nBounces );

		// increment shooting stats
#ifndef CLIENT_DLL
		if ( pMarine->GetMarineResource() )
		{
			pMarine->GetMarineResource()->UsedWeapon( this, info.m_iShots );
			pMarine->OnWeaponFired( this, info.m_iShots );
		}
#endif
	}
}

void CASW_Weapon_Ricochet::SecondaryAttack()
{
	// prevent secondary fire from interrupting the gun
	m_flNextSecondaryAttack = FLT_MAX;
}

#ifdef CLIENT_DLL
void CASW_Weapon_Ricochet::UpdateBounceLaser()
{
	if ( !m_pLaserPointerEffect )
		return;

	CASW_Player *pPlayer = GetCommander();
	CASW_Marine *pMarine = GetMarine();
	if ( !pPlayer || !pMarine || !pMarine->IsInhabited() || !m_bLocalPlayerControlling )
		return;

	Vector vecSrc = pMarine->WorldSpaceCenter();
	Vector vecDir = pPlayer->GetAutoaimVectorForMarine( pMarine, GetAutoAimAmount(), GetVerticalAdjustOnlyAutoAimAmount() );
	if ( pMarine->m_flLaserSightLength > asw_laser_sight_min_distance.GetFloat() )
	{
		vecDir = pMarine->m_vLaserSightCorrection;
	}

	float flRemainingDist = asw_weapon_max_shooting_distance.GetFloat();

	trace_t tr;
	UTIL_TraceLine( vecSrc, vecSrc + vecDir * flRemainingDist, MASK_SHOT, pMarine, COLLISION_GROUP_NONE, &tr );

	bool bAttack1, bAttack2, bReload, bOldReload, bOldAttack1;
	GetButtons( bAttack1, bAttack2, bReload, bOldReload, bOldAttack1 );

	int nBounces = bAttack2 ? rd_ricochet_shotgun_bounces.GetInt() : rd_ricochet_bounces.GetInt();
	for ( int i = 0; i < 5; i++ )
	{
		flRemainingDist -= vecSrc.DistTo( tr.endpos );

		vecSrc = tr.endpos;
		if ( tr.DidHit() && i < nBounces && flRemainingDist > 0 )
		{
			Vector vecNewDir = vecDir;
			// reflect the X+Y off the surface (leave Z intact so the shot is more likely to stay flat and hit enemies)
			float proj = ( vecNewDir ).Dot( tr.plane.normal );
			VectorMA( vecNewDir, -proj * 2, tr.plane.normal, vecNewDir );
			vecDir.x = vecNewDir.x;
			vecDir.y = vecNewDir.y;

			UTIL_TraceLine( vecSrc, vecSrc + vecDir * flRemainingDist, MASK_SHOT, NULL, COLLISION_GROUP_NONE, &tr );
		}

		m_pLaserPointerEffect->SetControlPoint( i + 10, vecSrc );
		m_pLaserPointerEffect->SetControlPointForwardVector( i + 10, vecDir );
	}
}

bool CASW_Weapon_Ricochet::Simulate()
{
	bool bContinue = BaseClass::Simulate();

	UpdateBounceLaser();

	return bContinue;
}
#endif
