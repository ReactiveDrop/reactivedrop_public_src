#include "cbase.h"
#include "asw_weapon_sentry_shared.h"
#include "in_buttons.h"
#include "asw_marine_skills.h"

#ifdef CLIENT_DLL
#include "c_asw_player.h"
#include "c_asw_weapon.h"
#include "c_asw_marine.h"
#else
#include "asw_marine.h"
#include "asw_player.h"
#include "asw_weapon.h"
#include "npcevent.h"
#include "asw_sentry_base.h"
#include "func_movelinear.h"
#include "asw_bait.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar rd_sentry_is_attacked_by_aliens( "rd_sentry_is_attacked_by_aliens", "1", FCVAR_CHEAT | FCVAR_REPLICATED, "If set to 0 aliens will not try to damage sentries." );
ConVar rd_debug_sentry_placement( "rd_debug_sentry_placement", "0", FCVAR_CHEAT | FCVAR_REPLICATED);
extern ConVar rd_server_marine_backpacks;

IMPLEMENT_NETWORKCLASS_ALIASED( ASW_Weapon_Sentry, DT_ASW_Weapon_Sentry )

BEGIN_NETWORK_TABLE( CASW_Weapon_Sentry, DT_ASW_Weapon_Sentry )
#ifdef CLIENT_DLL
	RecvPropInt( RECVINFO( m_nSentryAmmo ) ),
	RecvPropInt( RECVINFO( m_nMaxSentryAmmo ) ),
#else
	SendPropInt( SENDINFO( m_nSentryAmmo ) ),
	SendPropInt( SENDINFO( m_nMaxSentryAmmo ) ),
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CASW_Weapon_Sentry )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( asw_weapon_sentry, CASW_Weapon_Sentry );
PRECACHE_WEAPON_REGISTER(asw_weapon_sentry);

#ifndef CLIENT_DLL
//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CASW_Weapon_Sentry )
	DEFINE_KEYFIELD( m_nSentryAmmo, FIELD_INTEGER, "SentryAmmo" ),
END_DATADESC()

BEGIN_ENT_SCRIPTDESC( CASW_Weapon_Sentry, CASW_Weapon, "sentry gun case" )
	DEFINE_SCRIPTFUNC( GetSentryAmmo, "returns the amount of ammo for the contained sentry" )
	DEFINE_SCRIPTFUNC( SetSentryAmmo, "changes the amount of ammo for the contained sentry" )
END_SCRIPTDESC()
#endif /* not client */

CASW_Weapon_Sentry::CASW_Weapon_Sentry()
{
	m_fMinRange1	= 0;
	m_fMaxRange1	= 2048;

	m_fMinRange2	= 256;
	m_fMaxRange2	= 1024;

#ifndef CLIENT_DLL
	m_iSentryMunitionType = CASW_Sentry_Base::kAUTOGUN;
	m_nMaxSentryAmmo = m_nSentryAmmo = CASW_Sentry_Base::GetBaseAmmoForGunType( (CASW_Sentry_Base::GunType_t) m_iSentryMunitionType );
#else
	m_flNextDeployCheckThink = 0;
	m_bDisplayActive = false;
#endif
	m_bDisplayValid = true;
}


CASW_Weapon_Sentry::~CASW_Weapon_Sentry()
{

}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Activity
//-----------------------------------------------------------------------------
Activity CASW_Weapon_Sentry::GetPrimaryAttackActivity( void )
{
	return ACT_VM_PRIMARYATTACK;
}

void CASW_Weapon_Sentry::PrimaryAttack( void )
{
	DeploySentry();
}

bool CASW_Weapon_Sentry::OffhandActivate()
{
	if (!GetMarine() || GetMarine()->GetFlags() & FL_FROZEN)	// don't allow this if the marine is frozen
		return false;
	PrimaryAttack();

	return true;
}

#ifdef CLIENT_DLL
void CASW_Weapon_Sentry::OnDataChanged( DataUpdateType_t type )
{	
	BaseClass::OnDataChanged( type );

	if ( type == DATA_UPDATE_CREATED)
	{
		SetNextClientThink(CLIENT_THINK_ALWAYS);
	}

	CASW_Marine *pMarine = GetMarine();
	if ( pMarine )
	{
		bool bSentryActive = ( pMarine->GetActiveASWWeapon() == this );
		if ( bSentryActive && m_bDisplayActive == false )
		{
			m_hOwningMarine = pMarine;
			pMarine->CreateSentryBuildDisplay();
			m_bDisplayActive = true;
		}
		else if ( !bSentryActive && m_bDisplayActive == true )
		{	
			pMarine->DestroySentryBuildDisplay();
			m_bDisplayActive = false;
		}
	}	
}

void CASW_Weapon_Sentry::UpdateOnRemove()
{
	BaseClass::UpdateOnRemove();

	DestroySentryBuildDisplay( static_cast<CASW_Marine*>(m_hOwningMarine.Get()) );
	m_bDisplayActive = false;
}

void CASW_Weapon_Sentry::ClientThink( void )
{
	BaseClass::ClientThink();

	CASW_Marine *pMarine;
	if ( m_hOwningMarine.Get() )
	{
		// this means it's been removed or dropped, so destroy the display
		if ( !GetMarine() )
		{
			DestroySentryBuildDisplay( static_cast<CASW_Marine*>(m_hOwningMarine.Get()) );
			m_bDisplayActive = false;
			m_hOwningMarine = NULL;
			return;
		}

		pMarine = static_cast<CASW_Marine*>(m_hOwningMarine.Get());
	}
	else
		pMarine = GetMarine();

	if ( pMarine )
	{
		bool bSentryActive = ( pMarine->GetActiveASWWeapon() == this );
		if ( bSentryActive && m_flNextDeployCheckThink < gpGlobals->curtime )
		{
			CASW_Marine *pMarine = GetMarine();
			if (pMarine && pMarine->GetActiveASWWeapon() == this )
			{
				m_bDisplayValid = FindValidSentrySpot();
				pMarine->SetSentryBuildDisplayEnabled( m_bDisplayValid );
			}

			m_flNextDeployCheckThink = gpGlobals->curtime + 0.2;
		}
	}
}

void CASW_Weapon_Sentry::DestroySentryBuildDisplay( CASW_Marine *pMarine )
{
	if ( pMarine )
	{
		pMarine->DestroySentryBuildDisplay();
	}
}

#endif //CLIENT_DLL

void CASW_Weapon_Sentry::Drop( const Vector &vecVelocity )
{
#ifdef CLIENT_DLL
	DestroySentryBuildDisplay( static_cast<CASW_Marine*>(m_hOwningMarine.Get()) );
	m_bDisplayActive = false;
#endif

	BaseClass::Drop( vecVelocity );
}

bool CASW_Weapon_Sentry::Holster( CBaseCombatWeapon *pSwitchingTo )
{
#ifdef CLIENT_DLL
	DestroySentryBuildDisplay( static_cast<CASW_Marine*>(m_hOwningMarine.Get()) );
	m_bDisplayActive = false;
#endif

	return BaseClass::Holster( pSwitchingTo );
}

bool CASW_Weapon_Sentry::FindValidSentrySpot()
{
	CASW_Marine *pMarine = GetMarine();
	if ( !pMarine )
		return false;

	//QAngle ang = pMarine->ASWEyeAngles();
	QAngle ang = pMarine->GetAbsAngles();
	ang.x = 0;
	matrix3x4_t matrix;
	AngleMatrix( ang, matrix );
	Vector vecSpawnPos;
	VectorTransform(Vector(70, 0, 0), matrix, vecSpawnPos);

	// check we have room for the sentry
	vecSpawnPos += pMarine->GetAbsOrigin();
#ifndef CLIENT_DLL
	if ( !pMarine->IsInhabited() && vecSpawnPos.DistTo( pMarine->m_vecOffhandItemSpot ) < 150.0f )
	{
		vecSpawnPos.x = pMarine->m_vecOffhandItemSpot.x;
		vecSpawnPos.y = pMarine->m_vecOffhandItemSpot.y;
	}
#endif

	Vector vecSentryMins = Vector(-26,-26,0);
	Vector vecSentryMaxs = Vector(26,26,60);

	trace_t tr;
	UTIL_TraceHull( vecSpawnPos + Vector(0, 0, 50),
		vecSpawnPos + Vector( 0, 0, -50 ),
		vecSentryMins,
		vecSentryMaxs,
		MASK_SOLID,
		pMarine,
		COLLISION_GROUP_NONE,
		&tr );

	if ( rd_debug_sentry_placement.GetBool() )
		debugoverlay->AddBoxOverlay( tr.endpos, vecSentryMins, vecSentryMaxs, vec3_angle, 255, 255, tr.DidHitNonWorldEntity() ? 255 : 0, 32, 0.2f );

	CBaseEntity *pElevator = NULL;

	// Allow placing the sentry on moving objects, as long as they're not physics-based or alive.
	if ( tr.DidHitNonWorldEntity() )
	{
		pElevator = tr.m_pEnt;
		if ( !pElevator || ( pElevator->GetMoveType() != MOVETYPE_NONE && pElevator->GetMoveType() != MOVETYPE_PUSH ) )
			return false;
	}

	if ( tr.startsolid || tr.allsolid || ( tr.DidHit() && tr.fraction <= 0 ) || tr.fraction >= 1.0f ) // if there's something in the way, or no floor, don't deploy
		return false;

	vecSpawnPos = tr.endpos;

	// check the feet have ground to stand on
	trace_t trace;
	Vector vecTestMins = Vector(-3, -3, -3);
	Vector vecTestMaxs = Vector(3, 3, 3);

	Vector vecTest;
	VectorTransform( Vector( -46, -25, 0 ), matrix, vecTest );
	vecTest += vecSpawnPos;

	UTIL_TraceHull( vecTest + Vector( 0, 0, 8 ), vecTest - Vector( 0, 0, 20 ), vecTestMins, vecTestMaxs, MASK_SOLID, pMarine, COLLISION_GROUP_NONE, &trace );

	if ( rd_debug_sentry_placement.GetBool() )
		debugoverlay->AddBoxOverlay( vecTest, vecTestMins, vecTestMaxs, vec3_angle, 255, trace.fraction == 1.0 ? 0 : 255, 0, 255, 0.2f );

	if ( trace.fraction == 1.0 || ( trace.DidHitNonWorldEntity() ? trace.m_pEnt : NULL ) != pElevator )
		return false;

	VectorTransform( Vector( -46, 25, 0 ), matrix, vecTest );
	vecTest += vecSpawnPos;

	UTIL_TraceHull( vecTest + Vector( 0, 0, 8 ), vecTest - Vector( 0, 0, 20 ), vecTestMins, vecTestMaxs, MASK_SOLID, pMarine, COLLISION_GROUP_NONE, &trace );

	if ( rd_debug_sentry_placement.GetBool() )
		debugoverlay->AddBoxOverlay( vecTest, vecTestMins, vecTestMaxs, vec3_angle, 255, trace.fraction == 1.0 ? 0 : 255, 0, 255, 0.2f );

	if ( trace.fraction == 1.0 || ( trace.DidHitNonWorldEntity() ? trace.m_pEnt : NULL ) != pElevator )
		return false;

	VectorTransform( Vector( 21, -19, 0 ), matrix, vecTest );
	vecTest += vecSpawnPos;

	UTIL_TraceHull( vecTest + Vector( 0, 0, 8 ), vecTest - Vector( 0, 0, 20 ), vecTestMins, vecTestMaxs, MASK_SOLID, pMarine, COLLISION_GROUP_NONE, &trace );

	if ( rd_debug_sentry_placement.GetBool() )
		debugoverlay->AddBoxOverlay( vecTest, vecTestMins, vecTestMaxs, vec3_angle, 255, trace.fraction == 1.0 ? 0 : 255, 0, 255, 0.2f );

	if ( trace.fraction == 1.0 || ( trace.DidHitNonWorldEntity() ? trace.m_pEnt : NULL ) != pElevator )
		return false;

	VectorTransform( Vector( 21, 19, 0 ), matrix, vecTest );
	vecTest += vecSpawnPos;

	UTIL_TraceHull( vecTest + Vector( 0, 0, 8 ), vecTest - Vector( 0, 0, 20 ), vecTestMins, vecTestMaxs, MASK_SOLID, pMarine, COLLISION_GROUP_NONE, &trace );

	if ( rd_debug_sentry_placement.GetBool() )
		debugoverlay->AddBoxOverlay( vecTest, vecTestMins, vecTestMaxs, vec3_angle, 255, trace.fraction == 1.0 ? 0 : 255, 0, 255, 0.2f );

	if ( trace.fraction == 1.0 || ( trace.DidHitNonWorldEntity() ? trace.m_pEnt : NULL ) != pElevator )
		return false;

#ifdef GAME_DLL
	m_vecValidSentrySpot = vecSpawnPos;
	m_angValidSentryFacing = ang;
	m_hValidSentryParent = pElevator;
#endif

	return true;
}

void CASW_Weapon_Sentry::DeploySentry()
{
	CASW_Marine *pMarine = GetMarine();
	if ( !pMarine )		// firing from a marine
		return;

	if ( !FindValidSentrySpot() )
		return;

	// MUST call sound before removing a round from the clip of a CMachineGun
	WeaponSound(SINGLE);

	// sets the animation on the marine holding this weapon
	bool bSentryActive = (pMarine->GetActiveASWWeapon() == this);

#ifndef CLIENT_DLL
	CASW_Sentry_Base* pBase = (CASW_Sentry_Base *)CreateEntityByName( "asw_sentry_base" );	
	
    //Msg("Abs angles %f %f %f", m_angValidSentryFacing.x, m_angValidSentryFacing.y, m_angValidSentryFacing.z);
    pBase->SetAbsAngles( m_angValidSentryFacing );
    pBase->m_hDeployer = pMarine;
    pBase->SetGunType( m_iSentryMunitionType );
	pBase->SetAmmo( m_nSentryAmmo );

    UTIL_SetOrigin( pBase, m_vecValidSentrySpot );
    pBase->SetAbsVelocity( vec3_origin );
    pBase->Spawn();
    pBase->PlayDeploySound();

    // reactivedrop: create a bait near the sentry for aliens to attack sentry
	if ( rd_sentry_is_attacked_by_aliens.GetBool() )
	{
		float sentry_angle = m_angValidSentryFacing.y; //degrees 
		CASW_Bait *pEnt1 = NULL;

		Vector bait_ang = Vector(cos(DEG2RAD(sentry_angle)), sin(DEG2RAD(sentry_angle)), 0);
		const float BAIT_OFFSETX = 40.0f;
		const float BAIT_OFFSETY = 40.0f;
		Vector bait_dir = bait_ang.Normalized() * BAIT_OFFSETX;
		{
			Vector bait_vec = m_vecValidSentrySpot + bait_dir + Vector(0, 0, 10);
			pEnt1 = CASW_Bait::Bait_Create( bait_vec, QAngle(90,0,0), vec3_origin, AngularImpulse(0, 0, 0), pBase );
			if ( pEnt1 )
			{
				pEnt1->SetDuration( 10000 );
				pEnt1->AddEffects( EF_NODRAW );
			}
		}

		CASW_Bait *pEnt2 = NULL;
		{
			Vector bait_vec = m_vecValidSentrySpot - bait_dir + Vector(0, 0, 10);
			pEnt2 = CASW_Bait::Bait_Create( bait_vec, QAngle(90,0,0), vec3_origin, AngularImpulse(0, 0, 0), pBase );
			if ( pEnt2 )
			{
				pEnt2->SetDuration( 10000 );
				pEnt2->AddEffects( EF_NODRAW );
			}
		}

		bait_dir = bait_ang.Normalized() * BAIT_OFFSETY;
		bait_ang = Vector( cos( DEG2RAD( sentry_angle + 90 ) ), sin( DEG2RAD( sentry_angle + 90 ) ), 0 );
		CASW_Bait *pEnt3 = NULL;
		{
			Vector bait_vec = m_vecValidSentrySpot + bait_dir + Vector( 0, 0, 10 );
			pEnt3 = CASW_Bait::Bait_Create( bait_vec, QAngle( 90, 0, 0 ), vec3_origin, AngularImpulse( 0, 0, 0 ), pBase );
			if ( pEnt3 )
			{
				pEnt3->SetDuration( 10000 );
				pEnt3->AddEffects( EF_NODRAW );
			}
		}

		CASW_Bait *pEnt4 = NULL;
		{
			Vector bait_vec = m_vecValidSentrySpot - bait_dir + Vector( 0, 0, 10 );
			pEnt4 = CASW_Bait::Bait_Create( bait_vec, QAngle( 90, 0, 0 ), vec3_origin, AngularImpulse( 0, 0, 0 ), pBase );
			if ( pEnt4 )
			{
				pEnt4->SetDuration( 10000 );
				pEnt4->AddEffects( EF_NODRAW );
			}
		}

		pBase->SetBait( pEnt1, pEnt2, pEnt3, pEnt4 );
	}

	IGameEvent * event = gameeventmanager->CreateEvent( "sentry_placed" );
	if ( event )
	{
		CBasePlayer *pPlayer = pMarine->GetCommander();
		event->SetInt( "userid", ( pPlayer ? pPlayer->GetUserID() : 0 ) );

		gameeventmanager->FireEvent( event );
	}

	if ( m_hValidSentryParent.Get() )
	{
		pBase->SetParent( m_hValidSentryParent );
	}

	// auto start setting it up
	pBase->ActivateUseIcon( pMarine, ASW_USE_RELEASE_QUICK );

	pMarine->Weapon_Detach(this);
	pMarine->OnWeaponFired( this, 1 );
	Kill();

	if (rd_server_marine_backpacks.GetBool())
	{
		pMarine->RemoveBackPackModel();
	}
#else
	pMarine->DestroySentryBuildDisplay();
#endif				
	if (bSentryActive)
		pMarine->SwitchToNextBestWeapon(NULL);
}

void CASW_Weapon_Sentry::Precache()
{
	BaseClass::Precache();
}

//============================================

IMPLEMENT_NETWORKCLASS_ALIASED( ASW_Weapon_Sentry_Flamer, DT_ASW_Weapon_Sentry_Flamer )

BEGIN_NETWORK_TABLE( CASW_Weapon_Sentry_Flamer, DT_ASW_Weapon_Sentry_Flamer )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CASW_Weapon_Sentry_Flamer )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( asw_weapon_sentry_flamer, CASW_Weapon_Sentry_Flamer );
PRECACHE_WEAPON_REGISTER( asw_weapon_sentry_flamer );

CASW_Weapon_Sentry_Flamer::CASW_Weapon_Sentry_Flamer()
{
#ifndef CLIENT_DLL
	m_iSentryMunitionType = CASW_Sentry_Base::kFLAME;
	m_nMaxSentryAmmo = m_nSentryAmmo = CASW_Sentry_Base::GetBaseAmmoForGunType( (CASW_Sentry_Base::GunType_t) m_iSentryMunitionType );
#endif
}

//============================================

IMPLEMENT_NETWORKCLASS_ALIASED( ASW_Weapon_Sentry_Cannon, DT_ASW_Weapon_Sentry_Cannon )

BEGIN_NETWORK_TABLE( CASW_Weapon_Sentry_Cannon, DT_ASW_Weapon_Sentry_Cannon )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CASW_Weapon_Sentry_Cannon )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( asw_weapon_sentry_cannon, CASW_Weapon_Sentry_Cannon );
PRECACHE_WEAPON_REGISTER( asw_weapon_sentry_cannon );

CASW_Weapon_Sentry_Cannon::CASW_Weapon_Sentry_Cannon()
{
#ifndef CLIENT_DLL
	m_iSentryMunitionType = CASW_Sentry_Base::kCANNON;
	m_nMaxSentryAmmo = m_nSentryAmmo = CASW_Sentry_Base::GetBaseAmmoForGunType( (CASW_Sentry_Base::GunType_t) m_iSentryMunitionType );
#endif
}

int CASW_Weapon_Sentry_Cannon::GetWeaponSkillId() 
{
	return ASW_MARINE_SKILL_GRENADES; 
}
int CASW_Weapon_Sentry_Cannon::GetWeaponSubSkillId()
{
	return ASW_MARINE_SUBSKILL_GRENADE_CLUSTER_DMG;
}

//============================================

IMPLEMENT_NETWORKCLASS_ALIASED( ASW_Weapon_Sentry_Freeze, DT_ASW_Weapon_Sentry_Freeze )

BEGIN_NETWORK_TABLE( CASW_Weapon_Sentry_Freeze, DT_ASW_Weapon_Sentry_Freeze )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CASW_Weapon_Sentry_Freeze )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( asw_weapon_sentry_freeze, CASW_Weapon_Sentry_Freeze );
PRECACHE_WEAPON_REGISTER( asw_weapon_sentry_freeze );

CASW_Weapon_Sentry_Freeze::CASW_Weapon_Sentry_Freeze()
{
#ifndef CLIENT_DLL
	m_iSentryMunitionType = CASW_Sentry_Base::kICE;
	m_nMaxSentryAmmo = m_nSentryAmmo = CASW_Sentry_Base::GetBaseAmmoForGunType( (CASW_Sentry_Base::GunType_t) m_iSentryMunitionType );
#endif
}
