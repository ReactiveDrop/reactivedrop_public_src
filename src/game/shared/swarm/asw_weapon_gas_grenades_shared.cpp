#include "cbase.h"
#include "asw_weapon_gas_grenades_shared.h"
#include "in_buttons.h"

#ifdef CLIENT_DLL
#include "c_asw_player.h"
#include "c_asw_weapon.h"
#include "c_asw_marine.h"
#include "prediction.h"
#else
#include "asw_marine.h"
#include "asw_player.h"
#include "asw_weapon.h"
#include "npcevent.h"
#include "shot_manipulator.h"
#include "asw_gas_grenade_projectile.h"
#endif
#include "engine/IVDebugOverlay.h"
#include "asw_util_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


ConVar asw_gas_grenade_duration( "asw_gas_grenade_duration", "30.0", FCVAR_CHEAT | FCVAR_REPLICATED, "Duration of the gas grenade" );
ConVar asw_gas_grenade_fuse( "asw_gas_grenade_fuse", "1", FCVAR_CHEAT | FCVAR_REPLICATED, "Fuse time on gas grenades" );
ConVar asw_gas_grenade_damage( "asw_gas_grenade_damage", "20.0", FCVAR_CHEAT | FCVAR_REPLICATED, "Damage the gas grenade inflicts" );
ConVar asw_gas_grenade_damage_interval( "asw_gas_grenade_damage_interval", "0.3", FCVAR_CHEAT | FCVAR_REPLICATED, "Interval of the gas grenade damage" );
ConVar asw_gas_grenade_cloud_width( "asw_gas_grenade_cloud_width", "100.0", FCVAR_CHEAT | FCVAR_REPLICATED, "Width of the gas grenade cloud" );

IMPLEMENT_NETWORKCLASS_ALIASED( ASW_Weapon_Gas_Grenades, DT_ASW_Weapon_Gas_Grenades )

BEGIN_NETWORK_TABLE( CASW_Weapon_Gas_Grenades, DT_ASW_Weapon_Gas_Grenades )
#ifdef CLIENT_DLL
	// recvprops
#else
	// sendprops
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CASW_Weapon_Gas_Grenades )
	
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( asw_weapon_gas_grenades, CASW_Weapon_Gas_Grenades );
PRECACHE_WEAPON_REGISTER( asw_weapon_gas_grenades );

#ifndef CLIENT_DLL

//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CASW_Weapon_Gas_Grenades )
	DEFINE_FIELD( m_flSoonestPrimaryAttack, FIELD_TIME ),
END_DATADESC()

ConVar asw_gas_grenade_throw_speed("asw_gas_grenade_throw_speed", "50.0f", FCVAR_CHEAT, "Velocity of thrown gas_grenades");

#endif /* not client */

ConVar asw_gas_grenade_launch_delay("asw_gas_grenade_launch_delay", "0.15f", FCVAR_REPLICATED, "Delay before gas_grenades are thrown");
ConVar asw_gas_grenade_refire_time("asw_gas_grenade_refire_time", "0.1f", FCVAR_REPLICATED, "Time between starting a new gas_grenade throw");
#define ASW_GAS_GRENADES_FASTEST_REFIRE_TIME		asw_gas_grenade_refire_time.GetFloat()

CASW_Weapon_Gas_Grenades::CASW_Weapon_Gas_Grenades()
{
	m_flSoonestPrimaryAttack = 0;
}


CASW_Weapon_Gas_Grenades::~CASW_Weapon_Gas_Grenades()
{

}

bool CASW_Weapon_Gas_Grenades::OffhandActivate()
{
	if (!GetMarine() || GetMarine()->GetFlags() & FL_FROZEN)	// don't allow this if the marine is frozen
		return false;

	PrimaryAttack();	

	return true;
}

#define FLARE_PROJECTILE_AIR_VELOCITY	400

void CASW_Weapon_Gas_Grenades::PrimaryAttack( void )
{	
	// Only the player fires this way so we can cast
	CASW_Player *pPlayer = GetCommander();
	if ( !pPlayer )
		return;

	CASW_Marine *pMarine = GetMarine();
	bool bThisActive = (pMarine && pMarine->GetActiveWeapon() == this);

	// gas_grenade weapon is lost when all gas_grenades are gone
	if ( UsesClipsForAmmo1() && m_iClip1 <= 0 )
	{
		return;
	}

	if (pMarine && gpGlobals->curtime > m_flDelayedFire)
	{
		
#ifdef CLIENT_DLL
		if ( !prediction->InPrediction() || prediction->IsFirstTimePredicted() )
		{
			pMarine->DoAnimationEvent( PLAYERANIMEVENT_THROW_GRENADE );
		}
#else
		pMarine->DoAnimationEvent( PLAYERANIMEVENT_THROW_GRENADE );
		pMarine->OnWeaponFired( this, 1 );
#endif

		// start our delayed attack
		m_bShotDelayed = true;
		m_flDelayedFire = gpGlobals->curtime + asw_gas_grenade_launch_delay.GetFloat();
		// reactivedrop: what's this??? TODO: investigate 
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = 0.36f;
		// make sure our primary weapon can't fire while we do the throw anim
		if (!bThisActive && pMarine->GetActiveASWWeapon())
		{
			// if we're offhand activating, make sure our primary weapon can't fire until we're done
			//pMarine->GetActiveASWWeapon()->m_flNextPrimaryAttack = m_flNextPrimaryAttack  + asw_gas_grenade_launch_delay.GetFloat();
			
			// reactivedrop: preventing cheating, firing gas_grenade can greatly
			// increase fire rate for primary weapon, when using with scripts
			pMarine->GetActiveASWWeapon()->m_flNextPrimaryAttack = 
				MAX(pMarine->GetActiveASWWeapon()->m_flNextPrimaryAttack, 
					m_flNextPrimaryAttack + asw_gas_grenade_launch_delay.GetFloat());
			pMarine->GetActiveASWWeapon()->m_bIsFiring = false;
		}
	}
}

void CASW_Weapon_Gas_Grenades::DelayedAttack()
{
	m_bShotDelayed = false;
	
	CASW_Player *pPlayer = GetCommander();
	if ( !pPlayer )
		return;

	CASW_Marine *pMarine = GetMarine();
	if ( !pMarine || pMarine->GetWaterLevel() == 3 )
		return;

	// sets the animation on the marine holding this weapon
	//pMarine->SetAnimation( PLAYER_ATTACK1 );
#ifndef CLIENT_DLL
	Vector vecSrc = pMarine->GetOffhandThrowSource();
	Vector vecDest = pMarine->GetOffhandThrowDest();
	Vector newVel = UTIL_LaunchVector( vecSrc, vecDest, GetThrowGravity() ) * 28.0f;
		
	CASW_Gas_Grenade_Projectile *pGas_Grenade = CASW_Gas_Grenade_Projectile::Gas_Grenade_Projectile_Create( vecSrc, QAngle( 90, 0, 0 ), newVel, AngularImpulse( 0, 0, 720 ), pMarine, this );
	if ( pGas_Grenade )
	{
		float flDuration = pGas_Grenade->GetDuration();

		//CALL_ATTRIB_HOOK_FLOAT( flDuration, mod_duration );

		pGas_Grenade->SetDuration( flDuration );
		pGas_Grenade->SetGravity( GetThrowGravity() );

		IGameEvent * event = gameeventmanager->CreateEvent( "gas_grenade_placed" );
		if ( event )
		{
			event->SetInt( "entindex", pGas_Grenade->entindex() );
			event->SetInt( "marine", pMarine->entindex() );
			gameeventmanager->FireEvent( event );
		}
	}
#endif
	// decrement ammo
	m_iClip1 -= 1;

#ifndef CLIENT_DLL
	DestroyIfEmpty( true );
#endif

	m_flSoonestPrimaryAttack = gpGlobals->curtime + ASW_GAS_GRENADES_FASTEST_REFIRE_TIME;
	
	if (m_iClip1 > 0)		// only force the fire wait time if we have ammo for another shot
		m_flNextPrimaryAttack = gpGlobals->curtime + GetFireRate();
	else
		m_flNextPrimaryAttack = gpGlobals->curtime;
}

void CASW_Weapon_Gas_Grenades::Precache()
{
	BaseClass::Precache();

#ifndef CLIENT_DLL
	UTIL_PrecacheOther( "asw_gas_grenade_projectile" );
#endif
}

// this weapon doesn't reload
bool CASW_Weapon_Gas_Grenades::Reload()
{
	return false;
}

void CASW_Weapon_Gas_Grenades::ItemPostFrame( void )
{
	BaseClass::ItemPostFrame();

	if ( m_bInReload )
		return;
	
	CBasePlayer *pOwner = GetCommander();
	if ( !pOwner )
		return;

	//Allow a refire as fast as the player can click
	if ( ( ( pOwner->m_nButtons & IN_ATTACK ) == false ) && ( m_flSoonestPrimaryAttack < gpGlobals->curtime ) )
	{
		m_flNextPrimaryAttack = gpGlobals->curtime - 0.1f;
	}
}


int CASW_Weapon_Gas_Grenades::ASW_SelectWeaponActivity(int idealActivity)
{
	// we just use the normal 'no weapon' anims for this
	return idealActivity;
}

#ifdef CLIENT_DLL

void CASW_Weapon_Gas_Grenades::OnDataChanged(DataUpdateType_t updateType)
{
	if ( updateType == DATA_UPDATE_CREATED )
	{
		SetNextClientThink( CLIENT_THINK_ALWAYS );
	}

	BaseClass::OnDataChanged( updateType );
}

void CASW_Weapon_Gas_Grenades::ClientThink()
{
	BaseClass::ClientThink();

	CASW_Player *pPlayer = GetCommander();
	if ( !pPlayer )
		return;

	CASW_Marine *pMarine = GetMarine();
	if ( pMarine && pMarine->IsInhabited() && pMarine->GetActiveWeapon() == this )
	{
		ShowThrowArc();
	}
}

void CASW_Weapon_Gas_Grenades::ShowThrowArc()
{
	CASW_Player *pPlayer = GetCommander();
	if ( !pPlayer )
		return;

	CASW_Marine *pMarine = GetMarine();
	if ( !pMarine || pMarine->GetWaterLevel() == 3 )
		return;

	Vector vecSrc = pMarine->GetOffhandThrowSource();
	Vector vecDest = pMarine->GetOffhandThrowDest();
	Vector vecVelocity = UTIL_LaunchVector( vecSrc, vecDest, GetThrowGravity() ) * 28.0f;
	UTIL_Check_Throw( vecSrc, vecVelocity, GetThrowGravity(), Vector( -2, -2, -2 ), Vector( 2, 2, 2 ), MASK_SOLID, ASW_COLLISION_GROUP_GRENADES, NULL, true );
}

#endif
