#include "cbase.h"

#include "asw_weapon_deagle_shared.h"
#include "in_buttons.h"
#ifdef CLIENT_DLL
	#include "c_asw_player.h"
	#include "c_asw_marine.h"
#else
	#include "asw_player.h"
	#include "asw_marine.h"
#endif

#include "asw_marine_skills.h"
#include "asw_weapon_parse.h"
#include "asw_deathmatch_mode_light.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar rd_deagle_bigalien_dmg_scale("rd_deagle_bigalien_dmg_scale", "0.5", FCVAR_CHEAT | FCVAR_REPLICATED, "Used to scale down desert eagles damage against Shieldbug, Mortarbug, Harvester, Drone Uber, Queen");

IMPLEMENT_NETWORKCLASS_ALIASED( ASW_Weapon_DEagle, DT_ASW_Weapon_DEagle )

BEGIN_NETWORK_TABLE( CASW_Weapon_DEagle, DT_ASW_Weapon_DEagle )
// #ifdef CLIENT_DLL
// // recvprops
// RecvPropBool( RECVINFO( m_bCanShoot ) ),
// #else
// // sendprops
// SendPropBool( SENDINFO( m_bCanShoot ) ),
// #endif
END_NETWORK_TABLE()

#if defined( CLIENT_DLL )
BEGIN_PREDICTION_DATA( CASW_Weapon_DEagle )
	DEFINE_FIELD( m_bCanShoot, FIELD_BOOLEAN ),
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( asw_weapon_deagle, CASW_Weapon_DEagle );
PRECACHE_WEAPON_REGISTER(asw_weapon_deagle);

#ifndef CLIENT_DLL
BEGIN_DATADESC( CASW_Weapon_DEagle )
	DEFINE_FIELD( m_bCanShoot, FIELD_BOOLEAN ),
END_DATADESC()
#endif

CASW_Weapon_DEagle::CASW_Weapon_DEagle() 
	: m_bCanShoot(true)
{
}

CASW_Weapon_DEagle::~CASW_Weapon_DEagle() 
{

}

void CASW_Weapon_DEagle::Precache() 
{
	PrecacheModel( "swarm/sprites/whiteglow1.vmt" );
	PrecacheModel( "swarm/sprites/greylaser1.vmt" );
	PrecacheScriptSound("ASW_Pistol.ReloadA");
	PrecacheScriptSound("ASW_Pistol.ReloadB");

	BaseClass::Precache();
}

void CASW_Weapon_DEagle::ItemPostFrame() 
{
	CBasePlayer *pOwner = GetCommander();

	if ( !m_bCanShoot && pOwner && !( pOwner->m_afButtonReleased & IN_ATTACK ) && ( pOwner->m_nButtons & IN_ATTACK ) )
	{
		// pretend we're still on cooldown so we don't run the weapon_fire event
		m_flNextPrimaryAttack = MAX( gpGlobals->curtime + TICK_INTERVAL, m_flNextPrimaryAttack );
	}

	// we don't call BaseClass::ItemPostFrame() to prevent pistol fast shooting
	CASW_Weapon::ItemPostFrame();

	if ( pOwner && ( pOwner->m_afButtonReleased & IN_ATTACK ) )
	{
		m_bCanShoot = true;
	}
}

void CASW_Weapon_DEagle::PrimaryAttack() 
{
	if ( m_bCanShoot )
	{
		BaseClass::PrimaryAttack();
		m_currentPistol = ASW_WEAPON_PISTOL_LEFT;

		// use this only for players, but not for AI
		if ( GetMarine() && GetMarine()->IsInhabited() )
		{
			m_bCanShoot = false;
		}
	}
}

float CASW_Weapon_DEagle::GetWeaponBaseDamageOverride()
{
	extern ConVar rd_deagle_dmg_base;
	return rd_deagle_dmg_base.GetFloat();
}
int CASW_Weapon_DEagle::GetWeaponSkillId()
{
	return ASW_MARINE_SKILL_ACCURACY;
}
int CASW_Weapon_DEagle::GetWeaponSubSkillId()
{
	return ASW_MARINE_SUBSKILL_ACCURACY_DEAGLE_DMG;
}

void CASW_Weapon_DEagle::FireBullets(CASW_Marine *pMarine, FireBulletsInfo_t *pBulletsInfo)
{
	pMarine->FireBullets( *pBulletsInfo );
	//pMarine->FirePenetratingBullets(*pBulletsInfo, 2, 3.5f, 0, true, NULL, false);
}

int CASW_Weapon_DEagle::ASW_SelectWeaponActivity(int idealActivity)
{
	switch (idealActivity)
	{
	case ACT_WALK:			idealActivity = ACT_WALK_AIM_PISTOL; break;
	case ACT_RUN:			idealActivity = ACT_RUN_AIM_PISTOL; break;
	case ACT_IDLE:			idealActivity = ACT_IDLE_PISTOL; break;
	case ACT_RELOAD:		idealActivity = ACT_RELOAD_PISTOL; break;	// short (pistol) reload
	case ACT_RANGE_ATTACK1:		idealActivity = ACT_RANGE_ATTACK_PISTOL; break;
	//case ACT_CROUCHIDLE:		idealActivity = ACT_MP_CROUCHWALK_ITEM1; break;
	//case ACT_JUMP:		idealActivity = ACT_MP_JUMP_ITEM1; break;
	//default:
	//	return BaseClass::ASW_SelectWeaponActivity(idealActivity);
	}
	return idealActivity;
}

float CASW_Weapon_DEagle::GetFireRate( void )
{
	float flRate = GetEquipItem()->m_flFireRate;

#ifdef CLIENT_DLL
	return flRate;

#else
	CASW_Marine *pMarine = GetMarine();
	if ( !pMarine || pMarine->IsInhabited() )
	{
		return flRate;
	}

	RandomSeed( gpGlobals->curtime * 10 );
	float rate = RandomFloat( flRate, flRate * 2 );

	return rate;
#endif
}

void CASW_Weapon_DEagle::ItemBusyFrame()
{
	BaseClass::ItemBusyFrame();

	CBasePlayer *pOwner = GetCommander();

	if (pOwner == NULL)
		return;

	if (pOwner->m_afButtonReleased & IN_ATTACK)
	{
		m_bCanShoot = true;
	}
}

const Vector &CASW_Weapon_DEagle::GetBulletSpread( void )
{
	const static Vector cone = Vector( 0.13053, 0.13053, 0.02 );	// VECTOR_CONE_15DEGREES with flattened Z
	const static Vector cone_duck = Vector( 0.05234, 0.05234, 0.01 ); // VECTOR_CONE_6DEGREES with flattened Z

	CASW_Marine *marine = GetMarine();

	if ( marine )
	{
		if ( marine->GetLocalVelocity().IsZero() && marine->m_bWalking )
			return cone_duck;
	}
	return cone;
}
