#include "cbase.h"
#include "asw_weapon_pdw_shared.h"
#include "in_buttons.h"

#ifdef CLIENT_DLL
#include "c_asw_player.h"
#include "c_asw_weapon.h"
#include "c_asw_marine.h"
#include "c_te_legacytempents.h"
#include "c_breakableprop.h"
#include "fx.h"
#include "c_asw_fx.h"
#define CASW_Marine C_ASW_Marine
#define CASW_Inhabitable_NPC C_ASW_Inhabitable_NPC
#else
#include "asw_lag_compensation.h"
#include "asw_marine.h"
#include "asw_player.h"
#include "asw_weapon.h"
#include "asw_marine_resource.h"
#include "npcevent.h"
#include "shot_manipulator.h"
#include "asw_marine_speech.h"
#include "asw_weapon_ammo_bag_shared.h"
#endif
#include "asw_marine_skills.h"
#include "asw_weapon_parse.h"
#include "asw_deathmatch_mode_light.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED( ASW_Weapon_PDW, DT_ASW_Weapon_PDW )

BEGIN_NETWORK_TABLE( CASW_Weapon_PDW, DT_ASW_Weapon_PDW )
#ifdef CLIENT_DLL
	// recvprops
#else
	// sendprops
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CASW_Weapon_PDW )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( asw_weapon_pdw, CASW_Weapon_PDW );
PRECACHE_WEAPON_REGISTER(asw_weapon_pdw);

ConVar asw_pdw_max_shooting_distance( "asw_pdw_max_shooting_distance", "400", FCVAR_REPLICATED, "Maximum distance of the hitscan weapons." );
extern ConVar asw_weapon_max_shooting_distance;
extern ConVar asw_weapon_force_scale;

CASW_Weapon_PDW::CASW_Weapon_PDW()
{
}


CASW_Weapon_PDW::~CASW_Weapon_PDW()
{

}

#ifndef CLIENT_DLL
extern ConVar asw_debug_marine_damage;
//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CASW_Weapon_PDW )
	
END_DATADESC()

acttable_t	CASW_Weapon_PDW::m_acttable[] = 
{
	{ ACT_RANGE_ATTACK1,			ACT_RANGE_ATTACK_SMG1,			true },
	{ ACT_RELOAD,					ACT_RELOAD_SMG1,				true },
	{ ACT_IDLE,						ACT_IDLE_SMG1,					true },
	{ ACT_IDLE_ANGRY,				ACT_IDLE_ANGRY_SMG1,			true },

	{ ACT_WALK,						ACT_WALK_RIFLE,					true },
	{ ACT_WALK_AIM,					ACT_WALK_AIM_RIFLE,				true  },
	
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
	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_GESTURE_RANGE_ATTACK_SMG1,	true },
	{ ACT_RANGE_ATTACK1_LOW,		ACT_RANGE_ATTACK_SMG1_LOW,		true },
	{ ACT_COVER_LOW,				ACT_COVER_SMG1_LOW,				false },
	{ ACT_RANGE_AIM_LOW,			ACT_RANGE_AIM_SMG1_LOW,			false },
	{ ACT_RELOAD_LOW,				ACT_RELOAD_SMG1_LOW,			false },
	{ ACT_GESTURE_RELOAD,			ACT_GESTURE_RELOAD_SMG1,		true },
};

IMPLEMENT_ACTTABLE( CASW_Weapon_PDW );

void CASW_Weapon_PDW::Spawn()
{
	BaseClass::Spawn();
}

// just dry fire by default
void CASW_Weapon_PDW::SecondaryAttack( void )
{
	// Only the player fires this way so we can cast
	CASW_Player *pPlayer = GetCommander();
	if (!pPlayer)
		return;

	CASW_Marine *pMarine = GetMarine();
	if (!pMarine)
		return;

	SendWeaponAnim( ACT_VM_DRYFIRE );
	BaseClass::WeaponSound( EMPTY );
	m_flNextSecondaryAttack = gpGlobals->curtime + 0.5f;
}

#endif /* not client */

void CASW_Weapon_PDW::Precache()
{
	PrecacheModel( "models/weapons/pdw/pdw_single.mdl" );

	BaseClass::Precache();
}

void CASW_Weapon_PDW::PrimaryAttack()
{
	// If my clip is empty (and I use clips) start reload
	if ( UsesClipsForAmmo1() && !m_iClip1 ) 
	{		
		Reload();
		return;
	}

	CASW_Player *pPlayer = GetCommander();
	CBaseCombatCharacter *pOwner = GetOwner();
	CASW_Inhabitable_NPC *pNPC = pOwner && pOwner->IsInhabitableNPC() ? assert_cast< CASW_Inhabitable_NPC * >( pOwner ) : NULL;

	if ( pNPC )
	{
		m_bIsFiring = true;

		// MUST call sound before removing a round from the clip of a CMachineGun
		WeaponSound( SINGLE );

		if ( m_iClip1 <= AmmoClickPoint() )
		{
			LowAmmoSound();
		}

		// tell the marine to tell its weapon to draw the muzzle flash
		pNPC->DoMuzzleFlash();

		// sets the animation on the weapon model iteself
		SendWeaponAnim( GetPrimaryAttackActivity() );

		// sets the animation on the marine holding this weapon
		//pMarine->SetAnimation( PLAYER_ATTACK1 );

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

		// To make the firing framerate independent, we may have to fire more than one bullet here on low-framerate systems, 
		// especially if the weapon we're firing has a really fast rate of fire.
		info.m_iShots = 0;
		float fireRate = GetFireRate();
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

			for ( int k = 0; k < info.m_iShots; k++ )
			{
				m_iClip1 -= 1;
#ifdef GAME_DLL
				if ( m_iClip1 <= 0 && pMarine && pMarine->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 )
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
		}
		else
		{
			info.m_iShots = MIN( info.m_iShots, pNPC->GetAmmoCount( m_iPrimaryAmmoType ) );
			for ( int k = 0; k < info.m_iShots; k++ )
			{
				pNPC->RemoveAmmo( info.m_iShots, m_iPrimaryAmmoType );;
			}
		}

		info.m_flDistance = asw_weapon_max_shooting_distance.GetFloat();
		info.m_iAmmoType = m_iPrimaryAmmoType;
		info.m_iTracerFreq = 1; // asw tracer test everytime
		info.m_vecSpread = GetBulletSpread();
		info.m_flDamage = GetWeaponDamage();
		info.m_flDamageForceScale = asw_weapon_force_scale.GetFloat();
#ifndef CLIENT_DLL
		if ( asw_debug_marine_damage.GetBool() )
			Msg( "Weapon dmg = %f\n", info.m_flDamage );
		if ( pMarine && pMarine->GetMarineResource() )
			pMarine->GetMarineResource()->OnFired_ScaleDamage( info );
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
}

int CASW_Weapon_PDW::ASW_SelectWeaponActivity(int idealActivity)
{
	switch( idealActivity )
	{
		case ACT_WALK:			idealActivity = ACT_MP_WALK_ITEM1; break;
		case ACT_RUN:			idealActivity = ACT_MP_RUN_ITEM1; break;
		case ACT_IDLE:			idealActivity = ACT_MP_STAND_ITEM1; break;
		case ACT_RELOAD:		idealActivity = ACT_RELOAD_PISTOL; break;	// short (pistol) reload
		case ACT_RANGE_ATTACK1:		idealActivity = ACT_MP_ATTACK_STAND_ITEM1; break;
		case ACT_CROUCHIDLE:		idealActivity = ACT_MP_CROUCHWALK_ITEM1; break;			
		case ACT_JUMP:		idealActivity = ACT_MP_JUMP_ITEM1; break;
		default:
			return BaseClass::ASW_SelectWeaponActivity(idealActivity);
	}
	return idealActivity;
}



float CASW_Weapon_PDW::GetWeaponDamage()
{
	//float flDamage = 18.0f;
	float flDamage = GetWeaponInfo()->m_flBaseDamage;

	extern ConVar rd_pdw_dmg_base;
	if ( rd_pdw_dmg_base.GetFloat() > 0 )
	{
		flDamage = rd_pdw_dmg_base.GetFloat();
	}

	if ( GetMarine() )
	{
		flDamage += MarineSkills()->GetSkillBasedValueByMarine(GetMarine(), ASW_MARINE_SKILL_ACCURACY, ASW_MARINE_SUBSKILL_ACCURACY_PDW_DMG);
	}

	//CALL_ATTRIB_HOOK_FLOAT( flDamage, mod_damage_done );

	return flDamage;
}

// user message based tracer type
const char* CASW_Weapon_PDW::GetUTracerType()
{
	// BenLubar: for some reason, ASWUTracerDualLeft makes the right-hand gun fire. ugh.
	return GetOwner() && GetOwner()->Classify() == CLASS_COMBINE ? "ASWUTracerDualLeft" : "ASWUTracerDual";
}

float CASW_Weapon_PDW::GetFireRate()
{
	//float flRate = 0.035f;
	float flRate = GetWeaponInfo()->m_flFireRate;

	//CALL_ATTRIB_HOOK_FLOAT( flRate, mod_fire_rate );

	return flRate;
}

#ifdef GAME_DLL
// Combine soldiers don't have animations for akimbo-style weapons, so only give them one PDW.
const char *CASW_Weapon_PDW::GetViewModel( int viewmodelindex ) const
{
	if ( GetOwner() && GetOwner()->Classify() == CLASS_COMBINE )
		return "models/weapons/pdw/pdw_single.mdl";

	return BaseClass::GetViewModel( viewmodelindex );
}

const char *CASW_Weapon_PDW::GetWorldModel( void ) const
{
	if ( GetOwner() && GetOwner()->Classify() == CLASS_COMBINE )
		return "models/weapons/pdw/pdw_single.mdl";

	return BaseClass::GetWorldModel();
}
#endif
