#include "cbase.h"
#include "asw_weapon_grenade_box_shared.h"
#ifdef CLIENT_DLL
#include "c_asw_marine.h"
#include "c_asw_player.h"
#include "prediction.h"
#else
#include "asw_marine.h"
#include "asw_player.h"
#endif
#include "in_buttons.h"
#include "asw_weapon_parse.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar asw_grenade_throw_delay( "asw_grenade_throw_delay", "0.15", FCVAR_REPLICATED | FCVAR_CHEAT, "Delay before grenade entity is spawned when throwing" );

IMPLEMENT_NETWORKCLASS_ALIASED( ASW_Weapon_Grenade_Box, DT_ASW_Weapon_Grenade_Box )

BEGIN_NETWORK_TABLE( CASW_Weapon_Grenade_Box, DT_ASW_Weapon_Grenade_Box )
#ifdef CLIENT_DLL
	// recvprops
#else
	// sendprops
#endif
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CASW_Weapon_Grenade_Box )
END_PREDICTION_DATA()
#else
BEGIN_DATADESC( CASW_Weapon_Grenade_Box )
	DEFINE_FIELD( m_flSoonestPrimaryAttack, FIELD_TIME ),
END_DATADESC()
#endif

CASW_Weapon_Grenade_Box::CASW_Weapon_Grenade_Box()
{
	m_fMinRange1 = 0;
	m_fMaxRange1 = 2048;

	m_fMinRange2 = 256;
	m_fMaxRange2 = 1024;

	m_flSoonestPrimaryAttack = gpGlobals->curtime;
}

CASW_Weapon_Grenade_Box::~CASW_Weapon_Grenade_Box()
{
}

void CASW_Weapon_Grenade_Box::Precache()
{
	BaseClass::Precache();

	PrecacheScriptSound( "ASW_Grenade.Throw" );
}

bool CASW_Weapon_Grenade_Box::OffhandActivate()
{
	if ( !GetMarine() || ( GetMarine()->GetFlags() & FL_FROZEN ) )	// don't allow this if the marine is frozen
		return false;

	PrimaryAttack();

	return true;
}

void CASW_Weapon_Grenade_Box::PrimaryAttack()
{
	CASW_Player *pPlayer = GetCommander();

	if ( !pPlayer )
		return;

	CASW_Marine *pMarine = GetMarine();
	bool bThisActive = ( pMarine && pMarine->GetActiveWeapon() == this );

	// grenade weapon is lost when all grenades are gone
	if ( UsesClipsForAmmo1() && m_iClip1 <= 0 )
	{
		return;
	}

	if ( pMarine && gpGlobals->curtime > m_flDelayedFire )		// firing from a marine
	{
#ifndef CLIENT_DLL
		pMarine->OnWeaponFired( this, 1 );
#else
		if ( !prediction->InPrediction() || prediction->IsFirstTimePredicted() )
#endif
		{
			pMarine->DoAnimationEvent( PLAYERANIMEVENT_THROW_GRENADE );

			EmitSound( "ASW_Grenade.Throw" );
		}

		// start our delayed attack
		m_bShotDelayed = true;
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flDelayedFire = gpGlobals->curtime + asw_grenade_throw_delay.GetFloat();
		if ( !bThisActive && pMarine->GetActiveASWWeapon() )
		{
			// reactivedrop: preventing cheating, firing flare can greatly
			// increase fire rate for primary weapon, when using with scripts
			pMarine->GetActiveASWWeapon()->m_flNextPrimaryAttack =
				MAX( pMarine->GetActiveASWWeapon()->m_flNextPrimaryAttack,
					m_flNextPrimaryAttack + 0.4f );
			pMarine->GetActiveASWWeapon()->m_bIsFiring = false;
		}
	}
}

void CASW_Weapon_Grenade_Box::Equip( CBaseCombatCharacter *pOwner )
{
	BaseClass::Equip( pOwner );

	Assert( ViewModelIsMarineAttachment() );

	// we don't want the marine skin number here
	m_nSkin = GetWeaponInfo()->m_iPlayerModelSkin;
	// set remaining grenade count (the body group is in descending order with 1 grenade in the last)
	SetBodygroup( 1, MAX( 0, GetBodygroupCount( 1 ) - m_iClip1 ) );
}

void CASW_Weapon_Grenade_Box::ItemPostFrame()
{
	BaseClass::ItemPostFrame();

	if ( m_bInReload )
		return;

	CASW_Player *pOwner = GetCommander();
	if ( pOwner == NULL )
		return;

	// Allow a refire as fast as the player can click
	if ( ( ( pOwner->m_nButtons & IN_ATTACK ) == false ) && ( m_flSoonestPrimaryAttack < gpGlobals->curtime ) )
	{
		m_flNextPrimaryAttack = gpGlobals->curtime - 0.1f;
	}

	if ( IsBeingCarried() )
	{
		// we don't want the marine skin number here
		m_nSkin = GetWeaponInfo()->m_iPlayerModelSkin;
		// set remaining grenade count (the body group is in descending order with 1 grenade in the last)
		SetBodygroup( 1, MAX( 0, GetBodygroupCount( 1 ) - m_iClip1 ) );
	}
}
