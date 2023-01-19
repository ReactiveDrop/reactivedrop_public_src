#include "cbase.h"
#include "asw_weapon_shotgun_shared.h"
#include "in_buttons.h"

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

IMPLEMENT_NETWORKCLASS_ALIASED( ASW_Weapon_Shotgun, DT_ASW_Weapon_Shotgun )

BEGIN_NETWORK_TABLE( CASW_Weapon_Shotgun, DT_ASW_Weapon_Shotgun )
#ifdef CLIENT_DLL
	// recvprops
	RecvPropTime( RECVINFO( m_fSlowTime ) ),
#else
	// sendprops
	SendPropTime( SENDINFO( m_fSlowTime ) ),
#endif
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CASW_Weapon_Shotgun )
	DEFINE_PRED_FIELD_TOL( m_fSlowTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, TD_MSECTOLERANCE ),
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( asw_weapon_shotgun, CASW_Weapon_Shotgun );
PRECACHE_WEAPON_REGISTER(asw_weapon_shotgun);

extern ConVar asw_weapon_max_shooting_distance;
extern ConVar asw_weapon_force_scale;
#ifndef CLIENT_DLL
extern ConVar asw_debug_marine_damage;
extern ConVar asw_DebugAutoAim;

BEGIN_DATADESC( CASW_Weapon_Shotgun )
	
END_DATADESC()

acttable_t	CASW_Weapon_Shotgun::m_acttable[] = 
{
	{ ACT_IDLE,						ACT_IDLE_SMG1,					true },	// FIXME: hook to shotgun unique

	{ ACT_RANGE_ATTACK1,			ACT_RANGE_ATTACK_SHOTGUN,			true },
	{ ACT_RELOAD,					ACT_RELOAD_SHOTGUN,					false },
	{ ACT_WALK,						ACT_WALK_RIFLE,						true },
	{ ACT_IDLE_ANGRY,				ACT_IDLE_ANGRY_SHOTGUN,				true },

// Readiness activities (not aiming)
	{ ACT_IDLE_RELAXED,				ACT_IDLE_SHOTGUN_RELAXED,		false },//never aims
	{ ACT_IDLE_STIMULATED,			ACT_IDLE_SHOTGUN_STIMULATED,	false },
	{ ACT_IDLE_AGITATED,			ACT_IDLE_SHOTGUN_AGITATED,		false },//always aims

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

	{ ACT_WALK_AIM,					ACT_WALK_AIM_SHOTGUN,				true },
	{ ACT_WALK_CROUCH,				ACT_WALK_CROUCH_RIFLE,				true },
	{ ACT_WALK_CROUCH_AIM,			ACT_WALK_CROUCH_AIM_RIFLE,			true },
	{ ACT_RUN,						ACT_RUN_RIFLE,						true },
	{ ACT_RUN_AIM,					ACT_RUN_AIM_SHOTGUN,				true },
	{ ACT_RUN_CROUCH,				ACT_RUN_CROUCH_RIFLE,				true },
	{ ACT_RUN_CROUCH_AIM,			ACT_RUN_CROUCH_AIM_RIFLE,			true },
	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_GESTURE_RANGE_ATTACK_SHOTGUN,	true },
	{ ACT_RANGE_ATTACK1_LOW,		ACT_RANGE_ATTACK_SHOTGUN_LOW,		true },
	{ ACT_RELOAD_LOW,				ACT_RELOAD_SHOTGUN_LOW,				false },
	{ ACT_GESTURE_RELOAD,			ACT_GESTURE_RELOAD_SHOTGUN,			false },
};

IMPLEMENT_ACTTABLE( CASW_Weapon_Shotgun );
#endif /* not client */

ConVar rd_shotgun_fire_rate( "rd_shotgun_fire_rate", "0", FCVAR_REPLICATED | FCVAR_CHEAT, "Fire rate of shotgun", true, 0, false, 0 );
ConVar rd_shotgun_secondary_delay( "rd_shotgun_secondary_delay", "0.1", FCVAR_REPLICATED | FCVAR_CHEAT );

CASW_Weapon_Shotgun::CASW_Weapon_Shotgun()
{
	m_fMinRange1	= 0;
	m_fMaxRange1	= 320;

	m_fMinRange2	= 256;
	m_fMaxRange2	= 1024;

	m_fSlowTime = 0;

#ifdef CLIENT_DLL
	m_flEjectBrassTime = 0.0f;
	m_nEjectBrassCount = 0;
#endif
}


CASW_Weapon_Shotgun::~CASW_Weapon_Shotgun()
{

}

Activity CASW_Weapon_Shotgun::GetPrimaryAttackActivity( void )
{
	return ACT_VM_PRIMARYATTACK;
}

#define PELLET_AIR_VELOCITY	2500
#define PELLET_WATER_VELOCITY	1500

void CASW_Weapon_Shotgun::PrimaryAttack( void )
{
	// If my clip is empty (and I use clips) start reload
	if ( UsesClipsForAmmo1() && !m_iClip1 ) 
	{
		Reload();
		return;
	}

	CASW_Player *pPlayer = GetCommander();
	CASW_Marine *pMarine = GetMarine();

	if (pMarine)		// firing from a marine
	{
		// MUST call sound before removing a round from the clip of a CMachineGun
		WeaponSound(SINGLE);
		if (m_iClip1 <= AmmoClickPoint())
		{
			LowAmmoSound();
		}

		m_bIsFiring = true;

		// tell the marine to tell its weapon to draw the muzzle flash
		pMarine->DoMuzzleFlash();

		// sets the animation on the weapon model iteself
		SendWeaponAnim( GetPrimaryAttackActivity() );

#ifdef GAME_DLL	// check for turning on lag compensation
		if ( pPlayer && pMarine->IsInhabited() )
		{
			if (!m_bShotDelayed)
			{
				CASW_Lag_Compensation::RequestLagCompensation( pPlayer, pPlayer->GetCurrentUserCommand() );
			}
			else
			{
				CASW_Lag_Compensation::RequestLagCompensation( pPlayer, pPlayer->GetLastUserCommand() );
			}
		}
#endif

		Vector vecSrc = pMarine->Weapon_ShootPosition( );
		// hull trace out to this shoot position, so we can be sure we're not firing from over an alien's head
		trace_t tr;
		CTraceFilterSimple tracefilter(pMarine, COLLISION_GROUP_NONE);
		Vector vecMarineMiddle(pMarine->GetAbsOrigin());
		vecMarineMiddle.z = vecSrc.z;
		AI_TraceHull( vecMarineMiddle, vecSrc, Vector( -10, -10, -20 ), Vector( 10, 10, 10 ), MASK_SHOT, &tracefilter, &tr );
		vecSrc = tr.endpos;

		Vector vecAiming = vec3_origin;
		if ( pPlayer && pMarine->IsInhabited() )
		{
			vecAiming = pPlayer->GetAutoaimVectorForMarine(pMarine, GetAutoAimAmount(), GetVerticalAdjustOnlyAutoAimAmount());	// 45 degrees = 0.707106781187
		}
		else
		{
#ifndef CLIENT_DLL
			vecAiming = pMarine->GetActualShootTrajectory( vecSrc );
#endif
		}

#ifndef CLIENT_DLL
		if (asw_DebugAutoAim.GetBool())
		{
			NDebugOverlay::Line(vecSrc, vecSrc + vecAiming * asw_weapon_max_shooting_distance.GetFloat(), 64, 0, 64, false, 120.0);
		}
#endif
		int iPellets = GetNumPellets();
		for (int i=0;i<iPellets;i++)
		{
			FireBulletsInfo_t info( 1, vecSrc, vecAiming, GetAngularBulletSpread(), asw_weapon_max_shooting_distance.GetFloat(), m_iPrimaryAmmoType );
			info.m_pAttacker = pMarine;
			info.m_iTracerFreq = 1;
			info.m_nFlags = FIRE_BULLETS_NO_PIERCING_SPARK | FIRE_BULLETS_HULL | FIRE_BULLETS_ANGULAR_SPREAD;
			info.m_flDamage = GetWeaponDamage();
			info.m_flDamageForceScale = asw_weapon_force_scale.GetFloat();
#ifndef CLIENT_DLL
			if (asw_debug_marine_damage.GetBool())
				Msg("Weapon dmg = %f\n", info.m_flDamage);
			info.m_flDamage *= pMarine->GetMarineResource()->OnFired_GetDamageScale();
#endif

			FireShotgunPellet( pMarine, info, i );
		}

		// increment shooting stats
#ifndef CLIENT_DLL
		if ( pMarine->GetMarineResource() )
		{
			pMarine->GetMarineResource()->UsedWeapon(this, 1);
			pMarine->OnWeaponFired( this, GetNumPellets() );
		}
#endif

		// decrement ammo
		m_iClip1 -= 1;
#ifdef GAME_DLL
		if ( m_iClip1 <= 0 && pMarine->GetAmmoCount(m_iPrimaryAmmoType) <= 0 )
		{
			// check he doesn't have ammo in an ammo bay
			CASW_Weapon_Ammo_Bag* pAmmoBag = NULL;
			CASW_Weapon* pWeapon = pMarine->GetASWWeapon(0);
			if ( pWeapon && pWeapon->Classify() == CLASS_ASW_AMMO_BAG )
				pAmmoBag = assert_cast<CASW_Weapon_Ammo_Bag*>(pWeapon);

			if (!pAmmoBag)
			{
				pWeapon = pMarine->GetASWWeapon(1);
				if ( pWeapon && pWeapon->Classify() == CLASS_ASW_AMMO_BAG )
					pAmmoBag = assert_cast<CASW_Weapon_Ammo_Bag*>(pWeapon);
			}
			if ( !pAmmoBag || !pAmmoBag->CanGiveAmmoToWeapon(this) )
				pMarine->OnWeaponOutOfAmmo(true);
		}
#endif
	}
	
	if (m_iClip1 > 0)		// only force the fire wait time if we have ammo for another shot
		m_flNextPrimaryAttack = gpGlobals->curtime + GetFireRate();
	else
		m_flNextPrimaryAttack = gpGlobals->curtime;
	m_fSlowTime = gpGlobals->curtime + 0.1f;
}

void CASW_Weapon_Shotgun::SecondaryAttack()
{
	if ( m_flNextPrimaryAttack <= gpGlobals->curtime )
	{
		PrimaryAttack();

		if ( m_iClip1 > 0 )
		{
			m_bShotDelayed = true;
			m_flDelayedFire = gpGlobals->curtime + rd_shotgun_secondary_delay.GetFloat();
		}
	}
}

void CASW_Weapon_Shotgun::DelayedAttack()
{
	PrimaryAttack();

	m_bShotDelayed = false;
}

void CASW_Weapon_Shotgun::FireShotgunPellet( CASW_Marine *pMarine, const FireBulletsInfo_t &info, int iSeed )
{
	pMarine->FirePenetratingBullets( info, 0, 1.0f, iSeed, false );
}

void CASW_Weapon_Shotgun::Precache()
{
	PrecacheModel( "swarm/sprites/whiteglow1.vmt" );
	PrecacheModel( "swarm/sprites/greylaser1.vmt" );

	PrecacheScriptSound("ASW_Shotgun.ReloadA");
	PrecacheScriptSound("ASW_Shotgun.ReloadB");
	PrecacheScriptSound("ASW_Shotgun.ReloadC");

	BaseClass::Precache();
}

bool CASW_Weapon_Shotgun::ShouldMarineMoveSlow()
{
	return m_fSlowTime > gpGlobals->curtime;
}

#ifdef CLIENT_DLL
	ConVar asw_shotgun_shell_delay( "asw_shotgun_shell_delay", "0.8f", FCVAR_NONE, "Delay on shell casing after firing shotgun" );

	void CASW_Weapon_Shotgun::OnMuzzleFlashed()
	{
		BaseClass::OnMuzzleFlashed();
		
		Vector attachOrigin;
		QAngle attachAngles;
		
		m_nEjectBrassCount++;
		m_flEjectBrassTime = gpGlobals->curtime + asw_shotgun_shell_delay.GetFloat();
		if( GetAttachment( LookupAttachment("muzzle"), attachOrigin, attachAngles ) )
		{
			FX_ASW_ShotgunSmoke(attachOrigin, attachAngles);
		}
	}
#endif

bool CASW_Weapon_Shotgun::SupportsBayonet()
{
	return true;
}

float CASW_Weapon_Shotgun::GetWeaponDamage()
{
	float flDamage = GetWeaponInfo()->m_flBaseDamage;

	extern ConVar rd_shotgun_dmg_base;
	if ( rd_shotgun_dmg_base.GetFloat() > 0 )
	{
		flDamage = rd_shotgun_dmg_base.GetFloat();
	}

	if ( GetMarine() )
	{
		flDamage += MarineSkills()->GetSkillBasedValueByMarine(GetMarine(), ASW_MARINE_SKILL_ACCURACY, ASW_MARINE_SUBSKILL_ACCURACY_SHOTGUN_DMG);
	}

	//CALL_ATTRIB_HOOK_FLOAT( flDamage, mod_damage_done );

	return flDamage;
}

int CASW_Weapon_Shotgun::GetNumPellets()
{
	return GetWeaponInfo()->m_iNumPellets;
}


#ifdef CLIENT_DLL
bool CASW_Weapon_Shotgun::ShouldUseFastReloadAnim()
{
	if (GetMarine())
	{
		return (MarineSkills()->GetSkillBasedValueByMarine(GetMarine(), ASW_MARINE_SKILL_RELOADING) < 1.06f);		
	}
	return true;
}

const char* CASW_Weapon_Shotgun::GetPartialReloadSound(int iPart)
{
	if (ShouldUseFastReloadAnim())	// if we're using the pistol anim, use different sounds
	{
		switch (iPart)
		{
			case 1: return "ASW_Shotgun.ReloadC"; break;
			case 2: return NULL; break;		// no sound at the end if we're doing a quick reload
			default: break;
		};
		return "ASW_Shotgun.ReloadB";
	}
	switch (iPart)
	{
		case 1: return "ASW_Shotgun.ReloadB"; break;
		case 2: return "ASW_Shotgun.ReloadC"; break;
		default: break;
	};
	return "ASW_Shotgun.ReloadA";
}
#endif

int CASW_Weapon_Shotgun::ASW_SelectWeaponActivity(int idealActivity)
{
#ifdef CLIENT_DLL
	if (idealActivity == ACT_RELOAD && ShouldUseFastReloadAnim())	// if our marine is so skilled the normal reload will look bad, use the pistol style reload
	{
		return ACT_RELOAD_PISTOL;
	}
#endif
	switch( idealActivity )
	{		
		case ACT_WALK:			idealActivity = ACT_WALK_AIM_RIFLE; break;
		case ACT_RUN:			idealActivity = ACT_RUN_AIM_RIFLE; break;
		case ACT_IDLE:			idealActivity = ACT_IDLE_RIFLE; break;		
		case ACT_RELOAD:		idealActivity = ACT_RELOAD; break;		
		default: break;
	}
	return idealActivity;
}

float CASW_Weapon_Shotgun::GetFireRate()
{
	//float flRate = 1.0f;
	float flRate = GetWeaponInfo()->m_flFireRate;

	if ( rd_shotgun_fire_rate.GetFloat() > 0 )
		flRate = rd_shotgun_fire_rate.GetFloat();


	//CALL_ATTRIB_HOOK_FLOAT( flRate, mod_fire_rate );

	return flRate;
}

#ifdef CLIENT_DLL
void CASW_Weapon_Shotgun::ClientThink()
{
	BaseClass::ClientThink();

	if ( m_nEjectBrassCount > 0 && gpGlobals->curtime >= m_flEjectBrassTime )
	{
		int iAttachment = LookupAttachment( "eject1" );
		if( iAttachment != -1 )
		{
			for ( int i = 0; i < m_nEjectBrassCount; i++ )
			{
				EjectParticleBrass( "weapon_shell_casing_shotgun", iAttachment );
			}
		}
		m_nEjectBrassCount = 0;
	}
}
#endif

#ifdef GAME_DLL
void CASW_Weapon_Shotgun::FireNPCPrimaryAttack( CBaseCombatCharacter *pOperator, bool bUseWeaponAngles )
{
	Vector vecShootOrigin, vecShootDir;
	CAI_BaseNPC *npc = pOperator->MyNPCPointer();
	ASSERT( npc != NULL );
	WeaponSound( SINGLE_NPC );
	pOperator->DoMuzzleFlash();
	m_iClip1 = m_iClip1 - 1;

	if ( bUseWeaponAngles )
	{
		QAngle	angShootDir;
		GetAttachment( LookupAttachment( "muzzle" ), vecShootOrigin, angShootDir );
		AngleVectors( angShootDir, &vecShootDir );
	}
	else
	{
		vecShootOrigin = pOperator->Weapon_ShootPosition();
		vecShootDir = npc->GetActualShootTrajectory( vecShootOrigin );
	}

	pOperator->FireBullets( GetNumPellets(), vecShootOrigin, vecShootDir, GetBulletSpread(), MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 1 );
}

void CASW_Weapon_Shotgun::Operator_ForceNPCFire( CBaseCombatCharacter *pOperator, bool bSecondary, CBaseEntity *pTarget )
{
	// Ensure we have enough rounds in the clip
	m_iClip1++;

	FireNPCPrimaryAttack( pOperator, true );
}
#endif
