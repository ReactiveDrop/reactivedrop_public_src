#include "cbase.h"
#include "asw_weapon_mines_shared.h"
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
#include "shot_manipulator.h"
#include "asw_mine.h"
#include "asw_marine_skills.h"
#include "asw_marine_speech.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define ASW_FLARES_FASTEST_REFIRE_TIME		0.1f

IMPLEMENT_NETWORKCLASS_ALIASED( ASW_Weapon_Mines, DT_ASW_Weapon_Mines )

BEGIN_NETWORK_TABLE( CASW_Weapon_Mines, DT_ASW_Weapon_Mines )
#ifdef CLIENT_DLL
	// recvprops
#else
	// sendprops
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CASW_Weapon_Mines )
	
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( asw_weapon_mines, CASW_Weapon_Mines );
PRECACHE_WEAPON_REGISTER(asw_weapon_mines);

#ifndef CLIENT_DLL

//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CASW_Weapon_Mines )
	DEFINE_FIELD( m_flSoonestPrimaryAttack, FIELD_TIME ),
END_DATADESC()

#endif /* not client */

CASW_Weapon_Mines::CASW_Weapon_Mines()
{
	m_fMinRange1	= 0;
	m_fMaxRange1	= 2048;

	m_fMinRange2	= 256;
	m_fMaxRange2	= 1024;

	m_flSoonestPrimaryAttack = gpGlobals->curtime;
}


CASW_Weapon_Mines::~CASW_Weapon_Mines()
{

}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Activity
//-----------------------------------------------------------------------------
Activity CASW_Weapon_Mines::GetPrimaryAttackActivity( void )
{
	return ACT_VM_PRIMARYATTACK;
}

bool CASW_Weapon_Mines::OffhandActivate()
{
	if (!GetMarine() || GetMarine()->GetFlags() & FL_FROZEN)	// don't allow this if the marine is frozen
		return false;
	PrimaryAttack();

	return true;
}

#define ASW_MINE_VELOCITY	140

void CASW_Weapon_Mines::PrimaryAttack( void )
{	
	// Only the player fires this way so we can cast
	CASW_Player *pPlayer = GetCommander();

	if (!pPlayer)
		return;

	CASW_Marine *pMarine = GetMarine();

	// mine weapon is lost when all mines are gone
	if ( UsesClipsForAmmo1() && m_iClip1 <= 0 )
	{
		return;
	}

	if ( !pMarine || pMarine->GetWaterLevel() == 3 )
		return;

	// MUST call sound before removing a round from the clip of a CMachineGun
	//WeaponSound(SINGLE);

	// tell the marine to tell its weapon to draw the muzzle flash
	//pMarine->DoMuzzleFlash();

	// sets the animation on the weapon model iteself
	SendWeaponAnim( GetPrimaryAttackActivity() );

	//pMarine->DoAnimationEvent(PLAYERANIMEVENT_HEAL);

	// sets the animation on the marine holding this weapon
	//pMarine->SetAnimation( PLAYER_ATTACK1 );
#ifndef CLIENT_DLL
	Vector	vecSrc		= pMarine->Weapon_ShootPosition( );
	Vector	vecAiming	= pPlayer->GetAutoaimVectorForMarine(pMarine, GetAutoAimAmount(), GetVerticalAdjustOnlyAutoAimAmount());	// 45 degrees = 0.707106781187

	if ( !pMarine->IsInhabited() && vecSrc.DistTo( pMarine->m_vecOffhandItemSpot ) < 150.0f )
	{
		vecSrc.x = pMarine->m_vecOffhandItemSpot.x;
		vecSrc.y = pMarine->m_vecOffhandItemSpot.y;
		vecSrc.z += 50.0f;
	}
	
	QAngle ang = pPlayer->EyeAngles();
	ang.x = 0;
	ang.z = 0;
	CShotManipulator Manipulator( vecAiming );
	AngularImpulse rotSpeed(0,0,720);
	
	// create a pellet at some random spread direction			
	Vector newVel = Manipulator.ApplySpread(GetBulletSpread());

	newVel *= ASW_MINE_VELOCITY;
	if ( !pMarine->IsInhabited() )
	{
		newVel = vec3_origin;
	}
	CASW_Mine *pMine = CASW_Mine::ASW_Mine_Create( vecSrc, ang,
			newVel, rotSpeed, pMarine, this );
	if ( pMine )
	{
		float flDurationScale = 1.0f;
		int iExtraFires = 0;
		//CALL_ATTRIB_HOOK_FLOAT( flDurationScale, mod_duration );
		//CALL_ATTRIB_HOOK_INT( iExtraFires, mod_firewall_length );

		pMine->m_flDurationScale = flDurationScale;
		pMine->m_iExtraFires = iExtraFires;

		pMarine->OnWeaponFired( this, 1 );

		IGameEvent * event = gameeventmanager->CreateEvent( "fire_mine_placed" );
		if ( event )
		{
			event->SetInt( "entindex", pMine->entindex() );
			event->SetInt( "marine", pMarine->entindex() );
			gameeventmanager->FireEvent( event );
		}
	}

	pMarine->GetMarineSpeech()->Chatter(CHATTER_MINE_DEPLOYED);
#endif
	// decrement ammo
	m_iClip1 -= 1;

#ifndef CLIENT_DLL
	DestroyIfEmpty( true );
#endif

	m_flSoonestPrimaryAttack = gpGlobals->curtime + ASW_FLARES_FASTEST_REFIRE_TIME;
	if (m_iClip1 > 0)		// only force the fire wait time if we have ammo for another shot
		m_flNextPrimaryAttack = gpGlobals->curtime + GetFireRate();
	else
		m_flNextPrimaryAttack = gpGlobals->curtime;
}

void CASW_Weapon_Mines::Precache()
{
	BaseClass::Precache();	
#ifndef CLIENT_DLL
	UTIL_PrecacheOther( "asw_mine" );
#endif
}

// mines don't reload
bool CASW_Weapon_Mines::Reload()
{
	return false;
}

void CASW_Weapon_Mines::ItemPostFrame( void )
{
	BaseClass::ItemPostFrame();

	if ( m_bInReload )
		return;
	
	CBasePlayer *pOwner = GetCommander();

	if ( pOwner == NULL )
		return;

	//Allow a refire as fast as the player can click
	if ( ( ( pOwner->m_nButtons & IN_ATTACK ) == false ) && ( m_flSoonestPrimaryAttack < gpGlobals->curtime ) )
	{
		m_flNextPrimaryAttack = gpGlobals->curtime - 0.1f;
	}
}
