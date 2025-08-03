#include "cbase.h"

#include "asw_weapon_devastator_shared.h"
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
#include "asw_marine_profile.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED(ASW_Weapon_Devastator, DT_ASW_Weapon_Devastator)

BEGIN_NETWORK_TABLE(CASW_Weapon_Devastator, DT_ASW_Weapon_Devastator)
#ifdef CLIENT_DLL
	// recvprops
	RecvPropBool( RECVINFO( m_bLockedFire ) ),
#else
	// sendprops
	SendPropBool( SENDINFO( m_bLockedFire ) ),
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CASW_Weapon_Devastator)
#ifdef CLIENT_DLL
	DEFINE_PRED_FIELD( m_bLockedFire, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
#endif
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(asw_weapon_devastator, CASW_Weapon_Devastator);
PRECACHE_WEAPON_REGISTER(asw_weapon_devastator);

#ifndef CLIENT_DLL
BEGIN_DATADESC(CASW_Weapon_Devastator)
//DEFINE_FIELD( m_bCanShoot, FIELD_TIME ),
END_DATADESC()
#endif

static Vector cone_duck( 14, 14, 14 );

static void On_rd_devastator_bullet_spread_duck_Changed( IConVar* var, const char* /*pOldValue*/, float /*flOldValue*/ )
{
	float newSpread = ConVarRef( var ).GetFloat();
	cone_duck.Init( newSpread, newSpread, newSpread );
}

ConVar rd_devastator_dynamic_bullet_spread( "rd_devastator_dynamic_bullet_spread", "1", FCVAR_REPLICATED | FCVAR_CHEAT, "Controls if crouching decreases bullet spread for devastator" );
ConVar rd_devastator_bullet_spread_duck (   "rd_devastator_bullet_spread_duck",   "14", FCVAR_REPLICATED | FCVAR_CHEAT, "Devastator's bullet spread when ducking (crouching)",  true, 1.0f, true, 60.0f, &On_rd_devastator_bullet_spread_duck_Changed );
ConVar rd_devastator_lockmode_enabled(		"rd_devastator_lockmode_enabled",	   "1", FCVAR_REPLICATED | FCVAR_CHEAT, "Enables lock-mode secondary attack. Marine is locked in place but weapon fire rate and penetration gets higher" );
ConVar rd_devastator_lockmode_firerate(     "rd_devastator_lockmode_firerate",   "1.3", FCVAR_REPLICATED | FCVAR_CHEAT, "Scale factor of the fire rate in lock-mode",			true, 0.2f, true, 3.0f );

CASW_Weapon_Devastator::CASW_Weapon_Devastator()
{
}

CASW_Weapon_Devastator::~CASW_Weapon_Devastator()
{

}

void CASW_Weapon_Devastator::Precache()
{
	PrecacheModel( "swarm/sprites/whiteglow1.vmt" );
	PrecacheModel( "swarm/sprites/greylaser1.vmt");
	PrecacheScriptSound( "ASW_Weapon.Empty" );
	PrecacheScriptSound( "ASW_Weapon.Reload3" );
	PrecacheScriptSound( "ASW_Weapon_Devastator.SingleFP" );
	PrecacheScriptSound( "ASW_Weapon_Devastator.Single" );
	PrecacheScriptSound( "ASW_Weapon_Devastator.ReloadA" );
	PrecacheScriptSound( "ASW_Weapon_Devastator.ReloadB" );
	PrecacheScriptSound( "ASW_Weapon_Devastator.ReloadC" );

	BaseClass::Precache();
}

float CASW_Weapon_Devastator::GetWeaponBaseDamageOverride()
{
	extern ConVar rd_devastator_dmg_base;
	return rd_devastator_dmg_base.GetFloat();
}
int CASW_Weapon_Devastator::GetWeaponSkillId()
{
	return ASW_MARINE_SKILL_ACCURACY;
}
int CASW_Weapon_Devastator::GetWeaponSubSkillId()
{
	return ASW_MARINE_SUBSKILL_ACCURACY_DEVASTATOR_DMG;
}

float CASW_Weapon_Devastator::GetMovementScale()
{
	if ( m_bLockedFire )
	{
		return 0.000001f;	// 0 makes marine run faster for some reason
	}
	return ShouldMarineMoveSlow() ? 0.3f : 0.9f;
}

#ifndef CLIENT_DLL
#else
const char *CASW_Weapon_Devastator::GetPartialReloadSound( int iPart )
{
	switch ( iPart )
	{
	case 1: return "ASW_Weapon_Devastator.ReloadB"; break;
	case 2: return "ASW_Weapon_Devastator.ReloadC"; break;
	default: break;
	};
	return "ASW_Weapon_Devastator.ReloadA";
}
#endif

bool CASW_Weapon_Devastator::HasSecondaryAttack()
{
	return rd_devastator_lockmode_enabled.GetBool();
}

bool CASW_Weapon_Devastator::ShouldMarineMoveSlow()
{
	bool bAttack1, bAttack2, bReload, bOldReload, bOldAttack1;
	GetButtons(bAttack1, bAttack2, bReload, bOldReload, bOldAttack1);

	return ( BaseClass::ShouldMarineMoveSlow() || bAttack1 || m_bLockedFire );
}

void CASW_Weapon_Devastator::FireShotgunPellet( CASW_Inhabitable_NPC *pNPC, const FireBulletsInfo_t &info, int iSeed )
{
	CASW_Marine *pMarine = CASW_Marine::AsMarine( pNPC );
	if ( !pMarine )
	{
		BaseClass::FireShotgunPellet( pNPC, info, iSeed );
		return;
	}

	float fPiercingChance = MarineSkills()->GetSkillBasedValueByMarine( pMarine, ASW_MARINE_SKILL_STOPPING_POWER, ASW_MARINE_SUBSKILL_PIERCING_CHANCE );
	if (m_bLockedFire)
		fPiercingChance = 1;

	if (fPiercingChance > 0)
	{
		pMarine->FirePenetratingBullets(info, 1, fPiercingChance, iSeed, false );
	}
	else
	{
		pMarine->FirePenetratingBullets(info, 0, 1.0f, iSeed, false );
	}
}

const Vector& CASW_Weapon_Devastator::GetAngularBulletSpread()
{
	const static Vector cone( 22, 22, 22 );

	CASW_Marine *marine = GetMarine();

	if ( marine && rd_devastator_dynamic_bullet_spread.GetBool() )
	{
		if ( marine->GetLocalVelocity().IsZero() && marine->m_bWalking )
			return cone_duck;
	}
	return cone;
}

bool CASW_Weapon_Devastator::Reload( void )
{
	m_bLockedFire = false;

	return BaseClass::Reload();
}

bool CASW_Weapon_Devastator::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	m_bLockedFire = false;

	return BaseClass::Holster( pSwitchingTo );
}

void CASW_Weapon_Devastator::Drop( const Vector &vecVelocity )
{	
	m_bLockedFire = false;

	BaseClass::Drop( vecVelocity );
}

void CASW_Weapon_Devastator::ItemPostFrame( void )
{
	BaseClass::ItemPostFrame();

	CASW_Marine* pMarine = GetMarine();
	if ( !pMarine || !pMarine->IsAlive() )
	{
		m_bLockedFire = false;
		return;
	}
	if ( pMarine->GetCurrentMeleeAttack() )
		m_bLockedFire = false;
}

void CASW_Weapon_Devastator::ItemBusyFrame( void )
{
	BaseClass::ItemBusyFrame();

	CASW_Marine* pMarine = GetMarine();
	if ( !pMarine || !pMarine->IsAlive() )
	{
		m_bLockedFire = false;
		return;
	}
	if ( pMarine->GetCurrentMeleeAttack() )
		m_bLockedFire = false;
}

void CASW_Weapon_Devastator::SecondaryAttack()
{
	if ( rd_devastator_lockmode_enabled.GetBool() )
	{
		m_bLockedFire = !m_bLockedFire;
		m_flNextSecondaryAttack = gpGlobals->curtime + 1.0f;
		if ( m_bLockedFire )
		{
			m_flNextPrimaryAttack = gpGlobals->curtime + 0.5f;
			WeaponSound( BURST );
		}
	}
	else
	{
		CASW_Player *pPlayer = GetCommander();
		if ( !pPlayer )
			return;

		CASW_Marine *pMarine = GetMarine();
		if ( !pMarine )
			return;

		// dry fire
		SendWeaponAnim( ACT_VM_DRYFIRE );
		BaseClass::WeaponSound( EMPTY );
		m_flNextSecondaryAttack = gpGlobals->curtime + 0.5f;
	}
}

float CASW_Weapon_Devastator::GetFireRate()
{
	if ( m_bLockedFire )
	{
		return GetEquipItem()->m_flFireRate / rd_devastator_lockmode_firerate.GetFloat();
	}
	else
	{
		return GetEquipItem()->m_flFireRate;
	}
}
