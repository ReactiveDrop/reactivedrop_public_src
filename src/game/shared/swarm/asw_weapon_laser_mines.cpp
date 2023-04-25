#include "cbase.h"
#include "asw_weapon_laser_mines.h"
#include "in_buttons.h"
#include "asw_marine_skills.h"

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
#include "asw_laser_mine.h"
#include "asw_marine_skills.h"
#include "asw_marine_speech.h"
#endif
#include "asw_deathmatch_mode_light.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define ASW_FLARES_FASTEST_REFIRE_TIME		0.1f

IMPLEMENT_NETWORKCLASS_ALIASED( ASW_Weapon_Laser_Mines, DT_ASW_Weapon_Laser_Mines )

BEGIN_NETWORK_TABLE( CASW_Weapon_Laser_Mines, DT_ASW_Weapon_Laser_Mines )
#ifdef CLIENT_DLL
	// recvprops
#else
	// sendprops
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CASW_Weapon_Laser_Mines )
	
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( asw_weapon_laser_mines, CASW_Weapon_Laser_Mines );
PRECACHE_WEAPON_REGISTER( asw_weapon_laser_mines );

#ifndef CLIENT_DLL

//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CASW_Weapon_Laser_Mines )
	DEFINE_FIELD( m_flSoonestPrimaryAttack, FIELD_TIME ),
END_DATADESC()

#endif /* not client */

CASW_Weapon_Laser_Mines::CASW_Weapon_Laser_Mines()
{
	m_fMinRange1	= 0;
	m_fMaxRange1	= 2048;

	m_fMinRange2	= 256;
	m_fMaxRange2	= 1024;

	m_flSoonestPrimaryAttack = gpGlobals->curtime;
}


CASW_Weapon_Laser_Mines::~CASW_Weapon_Laser_Mines()
{

}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Activity
//-----------------------------------------------------------------------------
Activity CASW_Weapon_Laser_Mines::GetPrimaryAttackActivity( void )
{
	return ACT_VM_PRIMARYATTACK;
}

bool CASW_Weapon_Laser_Mines::OffhandActivate()
{
	if (!GetMarine() || GetMarine()->GetFlags() & FL_FROZEN)	// don't allow this if the marine is frozen
		return false;
	PrimaryAttack();

	return true;
}

#define ASW_MINE_VELOCITY	140

void CASW_Weapon_Laser_Mines::PrimaryAttack( void )
{	
	// Only the player fires this way so we can cast
	CASW_Player *pPlayer = GetCommander();

	if (!pPlayer)
		return;

	CASW_Marine *pMarine = GetMarine();
	bool bThisActive = (pMarine && pMarine->GetActiveWeapon() == this);

	// mine weapon is lost when all mines are gone
	if ( UsesClipsForAmmo1() && m_iClip1 <= 0 )
	{
		return;
	}

	if (pMarine && gpGlobals->curtime > m_flDelayedFire)		// firing from a marine
	{
#ifdef CLIENT_DLL
		if ( !prediction->InPrediction() || prediction->IsFirstTimePredicted() )
		{
#endif	
			Vector vecSrc = pMarine->GetAbsOrigin();
			vecSrc.z += 16.0f;	// place lower to catch shorter aliens
			Vector	vecAiming = pPlayer->GetAutoaimVectorForMarine(pMarine, GetAutoAimAmount(), GetVerticalAdjustOnlyAutoAimAmount());	// 45 degrees = 0.707106781187
			vecAiming.z = 0;
			vecAiming.NormalizeInPlace();

			const int nMinesPerShot = MarineSkills()->GetSkillBasedValueByMarine( pMarine, ASW_MARINE_SKILL_GRENADES, ASW_MARINE_SUBSKILL_GRENADE_LASER_MINES );
			const float flSpread = 30.0f; // spread of mine throwing in degrees	

			// try to predict whether mine throw is valid, if not then dont do the animation.
			// small problem: this prediction happens a little earlier than the actual throw, so there will be occassional cases when the animation does play when it shouldnt have or it will not play when it should have. though still better than always playing imo. 
			if ( GetThrownMineCount( nMinesPerShot, vecSrc, vecAiming, flSpread ) > 0 )
				pMarine->DoAnimationEvent( PLAYERANIMEVENT_THROW_GRENADE );
#ifdef CLIENT_DLL
		}
#endif

		// start our delayed attack
		m_bShotDelayed = true;
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flDelayedFire = gpGlobals->curtime + 0.2f;
		if (!bThisActive && pMarine->GetActiveASWWeapon())
		{
			// if we're offhand activating, make sure our primary weapon can't fire until we're done
			//pMarine->GetActiveASWWeapon()->m_flNextPrimaryAttack = m_flNextPrimaryAttack  + 0.4f;

			// reactivedrop: preventing cheating, firing flare can greatly
			// increase fire rate for primary weapon, when using with scripts
			pMarine->GetActiveASWWeapon()->m_flNextPrimaryAttack =
				MAX(pMarine->GetActiveASWWeapon()->m_flNextPrimaryAttack,
					m_flNextPrimaryAttack + 0.4f);
			pMarine->GetActiveASWWeapon()->m_bIsFiring = false;
		}
	}
}

void CASW_Weapon_Laser_Mines::DelayedAttack( void )
{
	m_bShotDelayed = false;

	CASW_Player *pPlayer = GetCommander();
	if ( !pPlayer )
		return;

	CASW_Marine *pMarine = GetMarine();
	if ( !pMarine || pMarine->GetWaterLevel() == 3 )
		return;

	bool bOnGround = false;

	Vector vecSrc = pMarine->GetAbsOrigin();
	vecSrc.z += 16.0f;	// place lower to catch shorter aliens
	Vector	vecAiming = pPlayer->GetAutoaimVectorForMarine(pMarine, GetAutoAimAmount(), GetVerticalAdjustOnlyAutoAimAmount());	// 45 degrees = 0.707106781187
	vecAiming.z = 0;
	vecAiming.NormalizeInPlace();

	const int nMinesPerShot = MarineSkills()->GetSkillBasedValueByMarine( pMarine, ASW_MARINE_SKILL_GRENADES, ASW_MARINE_SUBSKILL_GRENADE_LASER_MINES );
	const float flSpread = 30.0f; // spread of mine throwing in degrees
	int nThrown = 0;

	for ( int i = 0; i < nMinesPerShot; i++ )
	{
		CBaseEntity *pParent = NULL;
		trace_t tr;
		Vector vecMineAiming = vecAiming;
		
		if ( !ValidateThrow( i, nMinesPerShot, vecSrc, vecAiming, flSpread, bOnGround, pParent, tr, vecMineAiming ) )
			continue;

#ifndef CLIENT_DLL
		// calculate the laser aim offset relative to the mine facing
		QAngle angFacing, angLaser, angLaserOffset;
		VectorAngles( tr.plane.normal, angFacing );
		if ( bOnGround )
		{
			angLaser = angFacing;
		}
		else
		{
			VectorAngles( -vecMineAiming, angLaser );
		}
		RotationDelta( angFacing, angLaser, &angLaserOffset );

		CASW_Laser_Mine *pMine = CASW_Laser_Mine::ASW_Laser_Mine_Create( tr.endpos, angFacing, angLaserOffset, pMarine, pParent, true, this );
		IGameEvent * event = gameeventmanager->CreateEvent( "laser_mine_placed" );
		if ( event )
		{
			event->SetInt( "entindex", pMine->entindex() );
			event->SetInt( "marine", pMarine->entindex() );
			gameeventmanager->FireEvent( event );
		}
		nThrown++;
		pMarine->OnWeaponFired( this, 1 );
#endif
	}
	if ( nThrown <= 0 )
		return;

#ifndef CLIENT_DLL	
	pMarine->GetMarineSpeech()->Chatter(CHATTER_MINE_DEPLOYED);
#endif

	// sets the animation on the weapon model itself
	SendWeaponAnim( GetPrimaryAttackActivity() );

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

bool CASW_Weapon_Laser_Mines::ValidateThrow( int nMineNumber, int nMinesPerShot, Vector vecSrc, Vector vecAiming, float flSpread, bool &bOnGround, CBaseEntity *pParent, trace_t &tr, Vector &vecMineAiming )
{
	CASW_Player *pPlayer = GetCommander();
	if ( !pPlayer )
		return false;

	CASW_Marine *pMarine = GetMarine();
	if ( !pMarine || pMarine->GetWaterLevel() == 3 )
		return false;
	
	// throw each mine out at a different angle
	QAngle angRot = vec3_angle;
	angRot[ YAW ] = ( (float) nMineNumber / (float) nMinesPerShot ) * flSpread - ( 0.5f * flSpread );
	VectorRotate( vecAiming, angRot, vecMineAiming );

	// trace for a wall in front of the marine
	const float flDeployDistance = 180.0f;
	//if ( i != 1 )		// randomize all but the middle mine's distance slightly
	//{
		//flDeployDistance += SharedRandomFloat( "LaserMineVariation", -20.0f, 20.0f );
	//}

	if ( !pMarine->IsInhabited() )
	{			
		bOnGround = true;
#ifndef CLIENT_DLL	

		Vector vecDest = pMarine->m_vecOffhandItemSpot;

		vecDest.z += 16;
		if ( nMineNumber == 0 || nMineNumber == 2 )		// drop to the side
		{
			Vector vecPerpendicular;
			VectorRotate( vecAiming, QAngle( 0, 90, 0 ), vecPerpendicular );
			Vector vecNewDest = vecDest + vecPerpendicular * ( ( nMineNumber == 0 ) ? 32 : -32 );
			UTIL_TraceLine( vecDest, vecNewDest, MASK_SOLID, pMarine, COLLISION_GROUP_NONE, &tr );		// trace out to the sides
			if ( tr.startsolid )
				return false;

			if ( !tr.DidHit() )
			{
				// trace down again
				vecDest = vecNewDest;
				UTIL_TraceLine( vecDest, vecDest - Vector( 0, 0, 128 ), MASK_SOLID, pMarine, COLLISION_GROUP_NONE, &tr );
			}
		}
		else
		{
			UTIL_TraceLine( vecDest, vecDest - Vector( 0, 0, 128 ), MASK_SOLID, pMarine, COLLISION_GROUP_NONE, &tr );
		}
		if ( tr.startsolid )
			return false;

		if ( !tr.DidHit() )
			return false;
#endif
	}
	else
	{					
		UTIL_TraceLine( vecSrc, vecSrc + vecMineAiming * flDeployDistance, MASK_SOLID, pMarine, COLLISION_GROUP_NONE, &tr );

		if ( tr.startsolid )		// if we started in solid, trace again from the marine's center
		{
			vecSrc.x = pMarine->WorldSpaceCenter().x;
			vecSrc.y = pMarine->WorldSpaceCenter().y;
			UTIL_TraceLine( vecSrc, vecSrc + vecMineAiming * flDeployDistance, MASK_SOLID, pMarine, COLLISION_GROUP_NONE, &tr );
			if ( tr.startsolid )
				return false;
		}

		if ( !tr.DidHit() )
		{
			// do another trace to try and put it on the ground
//#ifndef CLIENT_DLL		
			Vector vecDest = pMarine->GetOffhandThrowDest();
			if ( vecDest.DistTo( vecSrc ) > flDeployDistance )
			{
				trace_t tr2;
				UTIL_TraceLine( tr.endpos, tr.endpos + Vector( 0, 0, -128 ), MASK_SOLID, pMarine, COLLISION_GROUP_NONE, &tr2 );
				tr = tr2;
			}
			else
			{
				// just put it under the xhair
				vecDest.z += 16;
				if ( nMineNumber == 0 || nMineNumber == 2 )		// drop to the side
				{
					Vector vecPerpendicular;
					VectorRotate( vecAiming, QAngle( 0, 90, 0 ), vecPerpendicular );
					Vector vecNewDest = vecDest + vecPerpendicular * ( ( nMineNumber == 0 ) ? 32 : -32 );
					UTIL_TraceLine( vecDest, vecNewDest, MASK_SOLID, pMarine, COLLISION_GROUP_NONE, &tr );		// trace out to the sides
					if ( tr.startsolid )
						return false;

					if ( !tr.DidHit() )
					{
						// trace down again
						vecDest = vecNewDest;
						UTIL_TraceLine( vecDest, vecDest - Vector( 0, 0, 128 ), MASK_SOLID, pMarine, COLLISION_GROUP_NONE, &tr );
					}
				}
				else
				{
					UTIL_TraceLine( vecDest, vecDest - Vector( 0, 0, 128 ), MASK_SOLID, pMarine, COLLISION_GROUP_NONE, &tr );
				}
			}
				
			if ( tr.startsolid )
				return false;

			if ( !tr.DidHit() )
				return false;
//#endif
			bOnGround = true;
		}
	}

//#ifndef CLIENT_DLL
	if ( tr.m_pEnt && !tr.m_pEnt->IsWorld() )
	{
		pParent = tr.m_pEnt;
	}
	// no attaching to alien noses
	if ( pParent && (pParent->IsNPC() || pParent->Classify() == CLASS_ASW_STATUE) )
		return false;
//#endif

	return true;
}

int CASW_Weapon_Laser_Mines::GetThrownMineCount( int nMinesPerShot, Vector vecSrc, Vector vecAiming, float flSpread )
{
	int nThrown = 0;
	
	for ( int i = 0; i < nMinesPerShot; i++ )
	{
		CBaseEntity *pParent = NULL;
		trace_t tr;
		Vector vecMineAiming = vecAiming;
		bool bOnGround = false;
		
		if ( ValidateThrow( i, nMinesPerShot, vecSrc, vecAiming, flSpread, bOnGround, pParent, tr, vecMineAiming ) )
			nThrown++;
	}

	return nThrown;
}

void CASW_Weapon_Laser_Mines::Precache()
{
	BaseClass::Precache();	
#ifndef CLIENT_DLL
	UTIL_PrecacheOther( "asw_laser_mine" );
#endif
}

// mines don't reload
bool CASW_Weapon_Laser_Mines::Reload()
{
	return false;
}

void CASW_Weapon_Laser_Mines::ItemPostFrame( void )
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

int CASW_Weapon_Laser_Mines::GetMaxClip1(void) const
{
	if (ASWDeathmatchMode())
	{
		return 5;
	}
	else
	{
		return BaseClass::GetMaxClip1();
	}
}

int CASW_Weapon_Laser_Mines::GetDefaultClip1(void) const
{
	if (ASWDeathmatchMode())
	{
		return 5;
	}
	else
	{
		return BaseClass::GetDefaultClip1();
	}
}
