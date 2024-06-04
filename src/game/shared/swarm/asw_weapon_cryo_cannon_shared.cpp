#include "cbase.h"
#include "asw_weapon_cryo_cannon_shared.h"
#include "asw_marine_skills.h"
#ifdef CLIENT_DLL
#include "c_asw_extinguisher_projectile.h"
#include "c_asw_marine.h"
#include "soundenvelope.h"
#else
#include "asw_extinguisher_projectile.h"
#include "asw_player.h"
#include "asw_marine.h"
#include "asw_marine_resource.h"
#include "te_effect_dispatch.h"
#include "shot_manipulator.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


#ifdef RD_7A_WEAPONS
ConVar asw_cryo_cannon_spin_rate_threshold( "asw_cryo_cannon_spin_rate_threshold", "0.75", FCVAR_CHEAT | FCVAR_REPLICATED, "Minimum barrel spin rate before cryo cannon will fire" );
ConVar asw_cryo_cannon_spin_down_rate( "asw_cryo_cannon_spin_down_rate", "0.4", FCVAR_CHEAT | FCVAR_REPLICATED, "Spin down speed of cryo cannon" );
#ifdef CLIENT_DLL
ConVar asw_cryo_cannon_pitch_min( "asw_cryo_cannon_pitch_min", "50", FCVAR_NONE, "Pitch of barrel spin sound" );
ConVar asw_cryo_cannon_pitch_max( "asw_cryo_cannon_pitch_max", "150", FCVAR_NONE, "Pitch of barrel spin sound" );
#endif

IMPLEMENT_NETWORKCLASS_ALIASED( ASW_Weapon_Cryo_Cannon, DT_ASW_Weapon_Cryo_Cannon );

BEGIN_NETWORK_TABLE( CASW_Weapon_Cryo_Cannon, DT_ASW_Weapon_Cryo_Cannon )
#ifdef CLIENT_DLL
	RecvPropFloat( RECVINFO( m_flSpinRate ) ),
#else
	SendPropFloat( SENDINFO( m_flSpinRate ), 0, SPROP_NOSCALE ),
#endif
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CASW_Weapon_Cryo_Cannon )
	DEFINE_PRED_FIELD_TOL( m_flSpinRate, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, TD_MSECTOLERANCE ),
	DEFINE_PRED_FIELD( m_bShouldUpdateActivityClient, FIELD_BOOLEAN, FTYPEDESC_PRIVATE ),
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( asw_weapon_cryo_cannon, CASW_Weapon_Cryo_Cannon );
PRECACHE_WEAPON_REGISTER( asw_weapon_cryo_cannon );

#ifndef CLIENT_DLL
BEGIN_DATADESC( CASW_Weapon_Cryo_Cannon )
END_DATADESC()
#endif

CASW_Weapon_Cryo_Cannon::CASW_Weapon_Cryo_Cannon()
{
#ifdef CLIENT_DLL
	m_pBarrelSpinSound = NULL;
	m_pFireSound = NULL;
	m_hEffect = NULL;
	m_bShouldUpdateActivityClient = false;
#endif
	m_flSpinRate = 0.0f;
}

void CASW_Weapon_Cryo_Cannon::Precache()
{
#ifndef CLIENT_DLL
	UTIL_PrecacheOther( "asw_extinguisher_projectile" );
#endif

	PrecacheScriptSound( "ASW_CryoCannon.Spin" );
	PrecacheScriptSound( "ASW_CryoCannon.Fire" );
	PrecacheScriptSound( "ASW_CryoCannon.Stop" );
	PrecacheEffect( "ExtinguisherCloud" );
	PrecacheParticleSystem( "rd_cryocannon" );
	PrecacheParticleSystem( "rd_cryocannon_explode" );

	BaseClass::Precache();
}

void CASW_Weapon_Cryo_Cannon::Spawn()
{
	BaseClass::Spawn();

	UseClientSideAnimation();
}

#ifdef CLIENT_DLL
void CASW_Weapon_Cryo_Cannon::ClientThink()
{
	BaseClass::ClientThink();

	UpdateSpinningBarrel();
	UpdateFiringEffects();
}

void CASW_Weapon_Cryo_Cannon::UpdateSpinningBarrel()
{
	if ( GetSpinRate() > 0.1f && GetSequenceActivity( GetSequence() ) != ACT_VM_PRIMARYATTACK )
	{
		SetActivity( ACT_VM_PRIMARYATTACK, 0 );
		m_bShouldUpdateActivityClient = true;
	}

	if ( GetSpinRate() < 0.1f && m_bShouldUpdateActivityClient )
	{
		SetActivity( ACT_VM_IDLE, 0 );
		m_bShouldUpdateActivityClient = false;
	}

	if ( GetSpinRate() > 0.01f )
	{
		if ( !m_pBarrelSpinSound )
		{
			CPASAttenuationFilter filter( this );
			m_pBarrelSpinSound = CSoundEnvelopeController::GetController().SoundCreate( filter, entindex(), "ASW_CryoCannon.Spin" );
			CSoundEnvelopeController::GetController().Play( m_pBarrelSpinSound, 0.0, 100 );
		}
		CSoundEnvelopeController::GetController().SoundChangeVolume( m_pBarrelSpinSound, MIN( 1.0f, GetSpinRate() * 3.0f ), 0.0f );
		CSoundEnvelopeController::GetController().SoundChangePitch( m_pBarrelSpinSound, asw_cryo_cannon_pitch_min.GetFloat() + ( GetSpinRate() * ( asw_cryo_cannon_pitch_max.GetFloat() - asw_cryo_cannon_pitch_min.GetFloat() ) ), 0.0f );
	}
	else
	{
		if ( m_pBarrelSpinSound )
		{
			CSoundEnvelopeController::GetController().SoundDestroy( m_pBarrelSpinSound );
			m_pBarrelSpinSound = NULL;
		}
	}
}

void CASW_Weapon_Cryo_Cannon::UpdateFiringEffects()
{
	if ( m_bIsFiring )
	{
		if ( !m_hEffect )
		{
			m_hEffect = ParticleProp()->Create( "rd_cryocannon", PATTACH_POINT_FOLLOW, "muzzle" );
		}

		if ( !m_pFireSound )
		{
			CPASAttenuationFilter filter( this );
			m_pFireSound = CSoundEnvelopeController::GetController().SoundCreate( filter, entindex(), "ASW_CryoCannon.Fire" );
			CSoundEnvelopeController::GetController().Play( m_pFireSound, 1.0f, 100 );
		}
		CSoundEnvelopeController::GetController().SoundChangePitch( m_pFireSound, asw_cryo_cannon_pitch_min.GetFloat() + ( GetSpinRate() * ( asw_cryo_cannon_pitch_max.GetFloat() - asw_cryo_cannon_pitch_min.GetFloat() ) ), 0.0f );
	}
	else
	{
		if ( m_hEffect )
		{
			m_hEffect->StopEmission();
			m_hEffect = NULL;
		}

		if ( m_pFireSound )
		{
			CSoundEnvelopeController::GetController().SoundDestroy( m_pFireSound );
			m_pFireSound = NULL;

			EmitSound( "ASW_CryoCannon.Stop" );
		}
	}
}

void CASW_Weapon_Cryo_Cannon::OnDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnDataChanged( updateType );

	if ( updateType == DATA_UPDATE_CREATED )
	{
		SetNextClientThink( CLIENT_THINK_ALWAYS );
	}
}

void CASW_Weapon_Cryo_Cannon::SetDormant( bool bDormant )
{
	if ( bDormant )
	{
		if ( m_pBarrelSpinSound )
		{
			CSoundEnvelopeController::GetController().SoundDestroy( m_pBarrelSpinSound );
			m_pBarrelSpinSound = NULL;
		}

		if ( m_pFireSound )
		{
			CSoundEnvelopeController::GetController().SoundDestroy( m_pFireSound );
			m_pFireSound = NULL;
		}
	}
	BaseClass::SetDormant( bDormant );
}

void CASW_Weapon_Cryo_Cannon::UpdateOnRemove()
{
	if ( m_pBarrelSpinSound )
	{
		CSoundEnvelopeController::GetController().SoundDestroy( m_pBarrelSpinSound );
		m_pBarrelSpinSound = NULL;
	}

	if ( m_pFireSound )
	{
		CSoundEnvelopeController::GetController().SoundDestroy( m_pFireSound );
		m_pFireSound = NULL;
	}

	BaseClass::UpdateOnRemove();
}
#endif

void CASW_Weapon_Cryo_Cannon::PrimaryAttack()
{
	// can't attack until the barrel is ready
	if ( GetSpinRate() < asw_cryo_cannon_spin_rate_threshold.GetFloat() )
		return;

	// If my clip is empty (and I use clips) start reload
	if ( UsesClipsForAmmo1() && !m_iClip1 )
	{
		Reload();
		return;
	}

	CASW_Player *pPlayer = GetCommander();
	CASW_Marine *pMarine = GetMarine();
	if ( !pMarine )
		return;

	m_bIsFiring = true;

	if ( m_iClip1 <= AmmoClickPoint() )
	{
		LowAmmoSound();
	}

	int iShots = 0;
	float fireRate = GetFireRate() * ( 1.0f / MAX( GetSpinRate(), asw_cryo_cannon_spin_rate_threshold.GetFloat() ) );
	while ( m_flNextPrimaryAttack <= gpGlobals->curtime )
	{
		m_flNextPrimaryAttack = m_flNextPrimaryAttack + fireRate;
		iShots++;
		if ( !fireRate )
			break;

		if ( pMarine->GetWaterLevel() == 3 )
		{
			WeaponSound( EMPTY );
			m_flNextPrimaryAttack = gpGlobals->curtime + fireRate;
			return;
		}
	}

	if ( iShots == 0 )
	{
		return;
	}

	if ( UsesClipsForAmmo1() )
	{
		iShots = MIN( iShots, m_iClip1 );
		m_iClip1 -= iShots;
	}
	else
	{
		iShots = MIN( iShots, pMarine->GetAmmoCount( m_iPrimaryAmmoType ) );
		pMarine->RemoveAmmo( iShots, m_iPrimaryAmmoType );
	}

	if ( iShots )
	{
		m_bIsFiring = true;
	}

#ifndef CLIENT_DLL
	float flDamage = GetWeaponDamage();
	float flFreezeAmount = MarineSkills()->GetSkillBasedValueByMarine( pMarine, ASW_MARINE_SKILL_AUTOGUN, ASW_MARINE_SUBSKILL_AUTOGUN_CRYO_FREEZE );
	float flExplosionRadius = MarineSkills()->GetSkillBasedValueByMarine( pMarine, ASW_MARINE_SKILL_AUTOGUN, ASW_MARINE_SUBSKILL_AUTOGUN_CRYO_RADIUS );

	Vector vecSrc = pMarine->Weapon_ShootPosition();
	Vector vecAiming = vec3_origin;
	if ( pPlayer && pMarine->IsInhabited() )
	{
		vecAiming = pPlayer->GetAutoaimVectorForMarine( pMarine, GetAutoAimAmount(), GetVerticalAdjustOnlyAutoAimAmount() );
	}
	else
	{
		vecAiming = pMarine->GetActualShootTrajectory( vecSrc );
	}

	CShotManipulator Manipulator( vecAiming );
	AngularImpulse rotSpeed( 0, 0, 720 );

	for ( int iShot = 0; iShot < iShots; iShot++ )
	{
		Vector newVel = Manipulator.ApplySpread( GetBulletSpread() );
		newVel *= EXTINGUISHER_PROJECTILE_AIR_VELOCITY;
		newVel *= ( 1.0 + ( 0.1 * random->RandomFloat( -1, 1 ) ) );
		// aim it downwards a bit
		newVel.z -= 40.0f;
		CASW_Extinguisher_Projectile *pProjectile = CASW_Extinguisher_Projectile::Extinguisher_Projectile_Create( vecSrc, QAngle( 0, 0, 0 ), newVel, rotSpeed, pMarine, this );
		pProjectile->m_flDamage = flDamage;
		pProjectile->m_flFreezeAmount = flFreezeAmount;
		pProjectile->m_flExplosionRadius = flExplosionRadius;
		pProjectile->m_bUseHullFreezeScale = true;

		// check for putting ourselves out
		if ( pMarine->IsOnFire() )
		{
			pMarine->Extinguish( pMarine, this );
			// spawn a cloud effect on this marine
			CEffectData	data;
			data.m_vOrigin = pMarine->GetAbsOrigin();
			//data.m_nEntIndex = pMarine->entindex();
			CPASFilter filter( data.m_vOrigin );
			filter.SetIgnorePredictionCull( true );
			DispatchEffect( filter, 0.0, "ExtinguisherCloud", data );
		}
	}

	// increment shooting stats
	CASW_Marine_Resource *pMR = pMarine->GetMarineResource();
	if ( pMR )
	{
		pMR->UsedWeapon( this, iShots );
		pMarine->OnWeaponFired( this, iShots );
	}
#endif
}

void CASW_Weapon_Cryo_Cannon::SecondaryAttack()
{
	m_flNextSecondaryAttack = gpGlobals->curtime + 0.5f;
}

void CASW_Weapon_Cryo_Cannon::ItemPostFrame()
{
	BaseClass::ItemPostFrame();

	UpdateSpinRate();
}

void CASW_Weapon_Cryo_Cannon::ItemBusyFrame()
{
	BaseClass::ItemBusyFrame();

	UpdateSpinRate();
}

void CASW_Weapon_Cryo_Cannon::UpdateSpinRate()
{
	bool bAttack1, bAttack2, bReload, bOldReload, bOldAttack1;
	GetButtons( bAttack1, bAttack2, bReload, bOldReload, bOldAttack1 );

	CASW_Marine *pMarine = GetMarine();
	bool bMeleeing = pMarine && ( pMarine->GetCurrentMeleeAttack() != NULL );
	bool bWalking = pMarine && ( pMarine->m_bWalking.Get() || pMarine->m_bForceWalking.Get()
#ifdef GAME_DLL
		// the logic for bot crouching is somewhat complicated; just check their animation
		|| ( !pMarine->IsInhabited() && pMarine->GetActivity() == ACT_CROUCHIDLE )
#endif
		);

	bool bSpinUp = pMarine && !m_bInReload && !bMeleeing && ( bAttack1 || bAttack2 || bWalking );
	if ( bSpinUp )
	{
		m_flSpinRate = MIN( 1.0f, GetSpinRate() + gpGlobals->frametime * MarineSkills()->GetSkillBasedValueByMarine( pMarine, ASW_MARINE_SKILL_AUTOGUN, ASW_MARINE_SUBSKILL_AUTOGUN_CRYO_SPINUP ) );
	}
	else
	{
		m_flSpinRate = MAX( 0.0f, GetSpinRate() - gpGlobals->frametime * asw_cryo_cannon_spin_down_rate.GetFloat() * ( ( m_bInReload || bMeleeing ) ? 3.0f : 1.0f ) );
	}
}

bool CASW_Weapon_Cryo_Cannon::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	// stop the barrel spinning
	m_flSpinRate = 0.0f;

	return BaseClass::Holster( pSwitchingTo );
}

void CASW_Weapon_Cryo_Cannon::Drop( const Vector &vecVelocity )
{
	// stop the barrel spinning
	m_flSpinRate = 0.0f;

	return BaseClass::Drop( vecVelocity );
}

float CASW_Weapon_Cryo_Cannon::GetMovementScale()
{
	return ShouldMarineMoveSlow() ? 0.4f : 0.95f;
}

bool CASW_Weapon_Cryo_Cannon::ShouldMarineMoveSlow()
{
	bool bAttack1, bAttack2, bReload, bOldReload, bOldAttack1;
	GetButtons( bAttack1, bAttack2, bReload, bOldReload, bOldAttack1 );

	return ( BaseClass::ShouldMarineMoveSlow() || bAttack2 || bAttack1 || GetSpinRate() >= 0.99f );
}

float CASW_Weapon_Cryo_Cannon::GetWeaponBaseDamageOverride()
{
	extern ConVar rd_cryo_cannon_dmg_base;
	return rd_cryo_cannon_dmg_base.GetFloat();
}
int CASW_Weapon_Cryo_Cannon::GetWeaponSkillId()
{
	return ASW_MARINE_SKILL_ACCURACY;
}
int CASW_Weapon_Cryo_Cannon::GetWeaponSubSkillId()
{
	return ASW_MARINE_SUBSKILL_ACCURACY_CRYO_DMG;
}
#endif
