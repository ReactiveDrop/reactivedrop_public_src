#include "cbase.h"
#include "asw_weapon_ar2_shared.h"
#include "in_buttons.h"

#ifdef CLIENT_DLL
#include "c_asw_player.h"
#include "c_asw_weapon.h"
#include "c_asw_marine.h"
#define CASW_Marine C_ASW_Marine
#else
#include "asw_marine.h"
#include "asw_marine_speech.h"
#include "asw_player.h"
#include "asw_weapon.h"
#include "npcevent.h"
#include "shot_manipulator.h"
#include "prop_combine_ball.h"
#include "asw_fail_advice.h"
#include "effect_dispatch_data.h"
#include "asw_gamerules.h"
#endif
#include "asw_marine_skills.h"
#include "asw_weapon_parse.h"
#include "particle_parse.h"
#include "asw_deathmatch_mode_light.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED( ASW_Weapon_AR2, DT_ASW_Weapon_AR2 )

BEGIN_NETWORK_TABLE( CASW_Weapon_AR2, DT_ASW_Weapon_AR2 )
#ifdef CLIENT_DLL
	// recvprops
#else
	// sendprops
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CASW_Weapon_AR2 )
	
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( asw_weapon_ar2, CASW_Weapon_AR2 );
PRECACHE_WEAPON_REGISTER( asw_weapon_ar2 );

#ifndef CLIENT_DLL
extern ConVar asw_debug_marine_damage;

extern ConVar sk_npc_dmg_ar2;
ConVar sk_weapon_ar2_alt_fire_radius( "sk_weapon_ar2_alt_fire_radius", "10", FCVAR_CHEAT );
ConVar sk_weapon_ar2_alt_fire_duration( "sk_weapon_ar2_alt_fire_duration", "2", FCVAR_CHEAT );
ConVar sk_weapon_ar2_alt_fire_mass( "sk_weapon_ar2_alt_fire_mass", "150", FCVAR_CHEAT );

//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CASW_Weapon_AR2 )
	DEFINE_FIELD( m_flDelayedFire, FIELD_TIME ),
	DEFINE_FIELD( m_bShotDelayed, FIELD_BOOLEAN ),
END_DATADESC()

acttable_t	CASW_Weapon_AR2::m_acttable[] = 
{
	{ ACT_RANGE_ATTACK1,			ACT_RANGE_ATTACK_AR2,			true },
	{ ACT_RELOAD,					ACT_RELOAD_SMG1,				true },		// FIXME: hook to AR2 unique
	{ ACT_IDLE,						ACT_IDLE_SMG1,					true },		// FIXME: hook to AR2 unique
	{ ACT_IDLE_ANGRY,				ACT_IDLE_ANGRY_SMG1,			true },		// FIXME: hook to AR2 unique

	{ ACT_WALK,						ACT_WALK_RIFLE,					true },

// Readiness activities (not aiming)
	{ ACT_IDLE_RELAXED,				ACT_IDLE_SMG1_RELAXED,			false },//never aims
	{ ACT_IDLE_STIMULATED,			ACT_IDLE_SMG1_STIMULATED,		false },
	{ ACT_IDLE_AGITATED,			ACT_IDLE_ANGRY_SMG1,			false },//always aims

	{ ACT_WALK_RELAXED,				ACT_WALK_RIFLE_RELAXED,			false },//never aims
	{ ACT_WALK_STIMULATED,			ACT_WALK_RIFLE_STIMULATED,		false },
	{ ACT_WALK_AGITATED,			ACT_WALK_AIM_RIFLE,				false },//always aims

	{ ACT_RUN_RELAXED,				ACT_RUN_RIFLE_RELAXED,			false },//never aims
	{ ACT_RUN_STIMULATED,			ACT_RUN_RIFLE_STIMULATED,		false },
	{ ACT_RUN_AGITATED,				ACT_RUN_AIM_RIFLE,				false },//always aims

// Readiness activities (aiming)
	{ ACT_IDLE_AIM_RELAXED,			ACT_IDLE_SMG1_RELAXED,			false },//never aims	
	{ ACT_IDLE_AIM_STIMULATED,		ACT_IDLE_AIM_RIFLE_STIMULATED,	false },
	{ ACT_IDLE_AIM_AGITATED,		ACT_IDLE_ANGRY_SMG1,			false },//always aims

	{ ACT_WALK_AIM_RELAXED,			ACT_WALK_RIFLE_RELAXED,			false },//never aims
	{ ACT_WALK_AIM_STIMULATED,		ACT_WALK_AIM_RIFLE_STIMULATED,	false },
	{ ACT_WALK_AIM_AGITATED,		ACT_WALK_AIM_RIFLE,				false },//always aims

	{ ACT_RUN_AIM_RELAXED,			ACT_RUN_RIFLE_RELAXED,			false },//never aims
	{ ACT_RUN_AIM_STIMULATED,		ACT_RUN_AIM_RIFLE_STIMULATED,	false },
	{ ACT_RUN_AIM_AGITATED,			ACT_RUN_AIM_RIFLE,				false },//always aims
//End readiness activities

	{ ACT_WALK_AIM,					ACT_WALK_AIM_RIFLE,				true },
	{ ACT_WALK_CROUCH,				ACT_WALK_CROUCH_RIFLE,			true },
	{ ACT_WALK_CROUCH_AIM,			ACT_WALK_CROUCH_AIM_RIFLE,		true },
	{ ACT_RUN,						ACT_RUN_RIFLE,					true },
	{ ACT_RUN_AIM,					ACT_RUN_AIM_RIFLE,				true },
	{ ACT_RUN_CROUCH,				ACT_RUN_CROUCH_RIFLE,			true },
	{ ACT_RUN_CROUCH_AIM,			ACT_RUN_CROUCH_AIM_RIFLE,		true },
	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_GESTURE_RANGE_ATTACK_AR2,	false },
	{ ACT_COVER_LOW,				ACT_COVER_SMG1_LOW,				false },		// FIXME: hook to AR2 unique
	{ ACT_RANGE_AIM_LOW,			ACT_RANGE_AIM_AR2_LOW,			false },
	{ ACT_RANGE_ATTACK1_LOW,		ACT_RANGE_ATTACK_SMG1_LOW,		true },		// FIXME: hook to AR2 unique
	{ ACT_RELOAD_LOW,				ACT_RELOAD_SMG1_LOW,			false },
	{ ACT_GESTURE_RELOAD,			ACT_GESTURE_RELOAD_SMG1,		true },
//	{ ACT_RANGE_ATTACK2, ACT_RANGE_ATTACK_AR2_GRENADE, true },
};

IMPLEMENT_ACTTABLE( CASW_Weapon_AR2 );
#endif /* not client */

CASW_Weapon_AR2::CASW_Weapon_AR2()
{
	m_fMinRange1 = 65;
	m_fMaxRange1 = 2048;

	m_fMinRange2 = 256;
	m_fMaxRange2 = 1024;
}

CASW_Weapon_AR2::~CASW_Weapon_AR2()
{
}

void CASW_Weapon_AR2::Precache()
{
	BaseClass::Precache();

#ifdef GAME_DLL
	UTIL_PrecacheOther( "prop_combine_ball" );
	UTIL_PrecacheOther( "env_entity_dissolver" );
#endif
}

float CASW_Weapon_AR2::GetWeaponDamage()
{
	float flDamage = GetWeaponInfo()->m_flBaseDamage;

	extern ConVar rd_ar2_dmg_base;
	if ( rd_ar2_dmg_base.GetFloat() > 0 )
	{
		flDamage = rd_ar2_dmg_base.GetFloat();
	}

	if ( GetMarine() )
	{
		flDamage += MarineSkills()->GetSkillBasedValueByMarine( GetMarine(), ASW_MARINE_SKILL_ACCURACY, ASW_MARINE_SUBSKILL_ACCURACY_AR2_DMG );
	}
#ifdef GAME_DLL
	else if ( ASWGameRules() )
	{
		flDamage = ASWGameRules()->ModifyAlienDamageBySkillLevel( sk_npc_dmg_ar2.GetFloat() );
	}
#endif

	return flDamage;
}

void CASW_Weapon_AR2::ItemPostFrame()
{
	// See if we need to fire off our secondary round
	if ( m_bShotDelayed && gpGlobals->curtime > m_flDelayedFire )
	{
		DelayedAttack();
	}

	BaseClass::ItemPostFrame();
}

void CASW_Weapon_AR2::SecondaryAttack()
{
	if ( m_bShotDelayed )
		return;

	CBaseCombatCharacter *pOwner = GetOwner();
	if ( !pOwner )
		return;

	// Must have ammo
	bool bUsesSecondary = UsesSecondaryAmmo();
	bool bUsesClips = UsesClipsForAmmo2();
	int iAmmoCount = pOwner->GetAmmoCount( m_iSecondaryAmmoType );
	if ( ( bUsesSecondary &&
		( ( bUsesClips && m_iClip2 <= 0 ) ||
			( !bUsesClips && iAmmoCount <= 0 )
			) )
		|| m_bInReload )
	{
		SendWeaponAnim( ACT_VM_DRYFIRE );
		BaseClass::WeaponSound( EMPTY );
		m_flNextSecondaryAttack = gpGlobals->curtime + 0.5f;
		return;
	}

	// MUST call sound before removing a round from the clip of a CMachineGun
	BaseClass::WeaponSound( SPECIAL1 );

	m_bShotDelayed = true;
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flDelayedFire = gpGlobals->curtime + 0.5f;
}

void CASW_Weapon_AR2::DelayedAttack( void )
{
	m_bShotDelayed = false;

	CBaseCombatCharacter *pOwner = GetOwner();
	if ( !pOwner )
		return;

	m_flNextSecondaryAttack = pOwner->m_flNextAttack = gpGlobals->curtime + 0.66f;

	WeaponSound( WPN_DOUBLE );

#ifdef GAME_DLL
	Vector vecSrc = pOwner->Weapon_ShootPosition();
	Vector vecAiming = pOwner->BodyDirection2D();
	Vector impactPoint = vecSrc + ( vecAiming * MAX_TRACE_LENGTH );

	Vector vecVelocity = vecAiming * 1000.0f;

	// Fire the combine ball
	CreateCombineBall( vecSrc,
		vecVelocity,
		sk_weapon_ar2_alt_fire_radius.GetFloat(),
		sk_weapon_ar2_alt_fire_mass.GetFloat(),
		sk_weapon_ar2_alt_fire_duration.GetFloat(),
		pOwner,
		this );

	if ( CASW_Marine *pMarine = GetMarine() )
	{
		pMarine->GetMarineSpeech()->Chatter( CHATTER_GRENADE );
		pMarine->OnWeaponFired( this, 1, true );
		ASWFailAdvice()->OnMarineUsedSecondary();
	}
#endif

	if ( UsesClipsForAmmo2() )
	{
		m_iClip2 -= 1;
	}
	else
	{
		pOwner->RemoveAmmo( 1, m_iSecondaryAmmoType );
	}

	// Can shoot again immediately
	m_flNextPrimaryAttack = gpGlobals->curtime + 0.5f;

	// Can blow up after a short delay (so have time to release mouse button)
	m_flNextSecondaryAttack = gpGlobals->curtime + 1.0f;
}

bool CASW_Weapon_AR2::CanHolster()
{
	if ( m_bShotDelayed )
		return false;

	return BaseClass::CanHolster();
}

#ifdef GAME_DLL
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CASW_Weapon_AR2::FireNPCSecondaryAttack( CBaseCombatCharacter *pOperator, bool bUseWeaponAngles )
{
	WeaponSound( WPN_DOUBLE );

	if ( !GetOwner() )
		return;
		
	CAI_BaseNPC *pNPC = GetOwner()->MyNPCPointer();
	if ( !pNPC )
		return;
	
	// Fire!
	Vector vecSrc;
	Vector vecAiming;

	if ( bUseWeaponAngles )
	{
		QAngle	angShootDir;
		GetAttachment( LookupAttachment( "muzzle" ), vecSrc, angShootDir );
		AngleVectors( angShootDir, &vecAiming );
	}
	else 
	{
		vecSrc = pNPC->Weapon_ShootPosition( );
		
		Vector vecTarget;

		CNPC_Combine *pSoldier = dynamic_cast<CNPC_Combine *>( pNPC );
		if ( pSoldier )
		{
			// In the distant misty past, elite soldiers tried to use bank shots.
			// Therefore, we must ask them specifically what direction they are shooting.
			vecTarget = pSoldier->GetAltFireTarget();
		}
		else
		{
			// All other users of the AR2 alt-fire shoot directly at their enemy.
			if ( !pNPC->GetEnemy() )
				return;
				
			vecTarget = pNPC->GetEnemy()->BodyTarget( vecSrc );
		}

		vecAiming = vecTarget - vecSrc;
		VectorNormalize( vecAiming );
	}

	Vector impactPoint = vecSrc + ( vecAiming * MAX_TRACE_LENGTH );

	float flAmmoRatio = 1.0f;
	float flDuration = RemapValClamped( flAmmoRatio, 0.0f, 1.0f, 0.5f, sk_weapon_ar2_alt_fire_duration.GetFloat() );
	float flRadius = RemapValClamped( flAmmoRatio, 0.0f, 1.0f, 4.0f, sk_weapon_ar2_alt_fire_radius.GetFloat() );

	// Fire the bullets
	Vector vecVelocity = vecAiming * 1000.0f;

	// Fire the combine ball
	CreateCombineBall(	vecSrc, 
		vecVelocity, 
		flRadius, 
		sk_weapon_ar2_alt_fire_mass.GetFloat(),
		flDuration,
		pNPC,
		this );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEvent - 
//			*pOperator - 
//-----------------------------------------------------------------------------
void CASW_Weapon_AR2::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	switch( pEvent->Event() )
	{ 
		case EVENT_WEAPON_AR2_ALTFIRE:
			{
				FireNPCSecondaryAttack( pOperator, false );
			}
			break;

		default:
			CBaseCombatWeapon::Operator_HandleAnimEvent( pEvent, pOperator );
			break;
	}
}
#endif
