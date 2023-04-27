#include "cbase.h"
#include "asw_weapon_electrified_armor.h"
#include "in_buttons.h"

#ifdef CLIENT_DLL
#include "c_asw_player.h"
#include "c_asw_weapon.h"
#include "c_asw_marine.h"
#else
#include "asw_marine.h"
#include "asw_player.h"
#include "asw_weapon.h"
#include "npcevent.h"
#include "asw_marine_resource.h"
#endif
#include "asw_gamerules.h"
#include "asw_marine_skills.h"
#include "particle_parse.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED( ASW_Weapon_Electrified_Armor, DT_ASW_Weapon_Electrified_Armor )

BEGIN_NETWORK_TABLE( CASW_Weapon_Electrified_Armor, DT_ASW_Weapon_Electrified_Armor )
#ifdef CLIENT_DLL
#else
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CASW_Weapon_Electrified_Armor )
	
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( asw_weapon_electrified_armor, CASW_Weapon_Electrified_Armor );
PRECACHE_WEAPON_REGISTER( asw_weapon_electrified_armor );

ConVar asw_electrified_armor_duration( "asw_electrified_armor_duration", "12.0f", FCVAR_CHEAT | FCVAR_REPLICATED, "Duration of electrified armor when activated" );
extern ConVar rm_destroy_empty_weapon;

#ifndef CLIENT_DLL
//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CASW_Weapon_Electrified_Armor )
	
END_DATADESC()

#endif /* not client */

CASW_Weapon_Electrified_Armor::CASW_Weapon_Electrified_Armor()
{

}


CASW_Weapon_Electrified_Armor::~CASW_Weapon_Electrified_Armor()
{

}

bool CASW_Weapon_Electrified_Armor::OffhandActivate()
{	
	if (!GetMarine() || GetMarine()->GetFlags() & FL_FROZEN)	// don't allow this if the marine is frozen
		return false;

	if (m_flNextPrimaryAttack < gpGlobals->curtime)
		PrimaryAttack();

	return true;
}

void CASW_Weapon_Electrified_Armor::PrimaryAttack( void )
{
	if ( !ASWGameRules() )
		return;
	
	CASW_Marine *pMarine = GetMarine();
	if ( !pMarine || pMarine->IsElectrifiedArmorActive() )
		return;

	if ( m_iClip1 <= 0 )
		return;

	// sets the animation on the marine holding this weapon
	//pMarine->SetAnimation( PLAYER_ATTACK1 );
#ifndef CLIENT_DLL

	float flDuration = asw_electrified_armor_duration.GetFloat();

	pMarine->AddElectrifiedArmor( flDuration, this );

	// stun aliens within the radius
	ASWGameRules()->ShockNearbyAliens( pMarine, this );
	
// 	CTakeDamageInfo info( this, GetOwnerEntity(), 5, DMG_SHOCK );
// 	info.SetDamageCustom( DAMAGE_FLAG_NO_FALLOFF );
// 	RadiusDamage( info, GetAbsOrigin(), flRadius, CLASS_ASW_MARINE, NULL );
	DispatchParticleEffect( "electrified_armor_burst", pMarine->GetAbsOrigin(), QAngle( 0, 0, 0 ) );
	
	// count as a shot fired
	if ( pMarine->GetMarineResource() )
	{
		pMarine->GetMarineResource()->UsedWeapon( this , 1 );
		pMarine->OnWeaponFired( this, 1 );
	}
#endif
	// decrement ammo
	m_iClip1 -= 1;

	m_flNextPrimaryAttack = gpGlobals->curtime + 4.0f;

#ifdef GAME_DLL
	DestroyIfEmpty( true );
#endif
}

void CASW_Weapon_Electrified_Armor::Precache()
{
	BaseClass::Precache();

	PrecacheParticleSystem( "electrified_armor_burst" );
}

int CASW_Weapon_Electrified_Armor::ASW_SelectWeaponActivity(int idealActivity)
{
	// we just use the normal 'no weapon' anims for this
	return idealActivity;
}

#ifndef CLIENT_DLL
void CASW_Weapon_Electrified_Armor::MarineDropped( CASW_Marine *pMarine )
{
	BaseClass::MarineDropped( pMarine );

	if ( rm_destroy_empty_weapon.GetBool() && m_iClip1 <= 0 )
	{
		// if we get dropped while active, we still need to destroy on empty
		Kill();
	}
}

void CASW_Weapon_Electrified_Armor::ItemPostFrame()
{
	BaseClass::ItemPostFrame();

	CASW_Marine *pMarine = GetMarine();
	if ( pMarine && !pMarine->IsElectrifiedArmorActive() )
	{
		DestroyIfEmpty( true );
	}
}
#else
void CASW_Weapon_Electrified_Armor::ClientThink()
{
	BaseClass::ClientThink();

	CASW_Marine *pMarine = GetMarine();
	SetBodygroup( 0, !pMarine || !pMarine->IsElectrifiedArmorActive() ? 0 : 1 );
}
#endif