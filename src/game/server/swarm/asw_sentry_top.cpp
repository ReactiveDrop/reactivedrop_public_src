#include "cbase.h"
#include "props.h"
#include "asw_sentry_base.h"
#include "asw_sentry_top.h"
#include "asw_player.h"
#include "asw_marine.h"
#include "ammodef.h"
#include "asw_gamerules.h"
#include "beam_shared.h"
#include "asw_fail_advice.h"
#include "asw_target_dummy_shared.h"
#include "asw_drone_advanced.h"
#include "asw_deathmatch_mode.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define SENTRY_TOP_MODEL "models/sentry_gun/machinegun_top.mdl"

LINK_ENTITY_TO_CLASS( asw_sentry_top, CASW_Sentry_Top );
PRECACHE_REGISTER( asw_sentry_top );

IMPLEMENT_SERVERCLASS_ST( CASW_Sentry_Top, DT_ASW_Sentry_Top )
	SendPropEHandle( SENDINFO( m_hSentryBase ) ),
	SendPropInt( SENDINFO( m_iSentryAngle ) ),
	SendPropAngle( SENDINFO( m_fDeployYaw ), 11 ),
	SendPropAngle( SENDINFO( m_fCenterAimYaw ), 11, SPROP_CHANGES_OFTEN ),
	SendPropBool( SENDINFO( m_bLowAmmo ) ),
END_SEND_TABLE()

ConVar asw_sentry_friendly_target( "asw_sentry_friendly_target", "0", FCVAR_CHEAT, "If 1, sentries ignore AI relationships and fire at all targets." );
ConVar rd_sentry_rotates_aim_instantly( "rd_sentry_rotates_aim_instantly", "1", FCVAR_CHEAT, "If 1, rotating a sentry will instantly change its enemy acquisition arc. Otherwise, the arc changes as the sentry turns while idle." );
extern ConVar asw_sentry_friendly_fire_scale;


//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CASW_Sentry_Top )
	DEFINE_THINKFUNC( AnimThink ),

	DEFINE_FIELD( m_fTurnRate, FIELD_FLOAT ),
	DEFINE_FIELD( m_flTimeFirstFired, FIELD_TIME ),
	DEFINE_FIELD( m_fLastThinkTime, FIELD_TIME ),
	DEFINE_FIELD( m_fNextFireTime, FIELD_TIME ),
	DEFINE_FIELD( m_fGoalYaw, FIELD_FLOAT ),
	DEFINE_FIELD( m_fDeployYaw, FIELD_FLOAT ),
	DEFINE_FIELD( m_fCenterAimYaw, FIELD_FLOAT ),
	DEFINE_FIELD( m_fCurrentYaw, FIELD_FLOAT ),
	DEFINE_FIELD( m_iEnemySkip, FIELD_INTEGER ),
	DEFINE_FIELD( m_hEnemy, FIELD_EHANDLE ),
	DEFINE_FIELD( m_iCanSeeError, FIELD_INTEGER ),
	DEFINE_FIELD( m_flNextTurnSound, FIELD_TIME ),
	DEFINE_FIELD( m_hSentryBase, FIELD_EHANDLE ),
	DEFINE_FIELD( m_iBaseTurnRate, FIELD_INTEGER ),
	DEFINE_FIELD( m_iSentryAngle, FIELD_INTEGER ),
	DEFINE_FIELD( m_fTurnRate, FIELD_FLOAT ),
	DEFINE_KEYFIELD( m_flShootRange, FIELD_FLOAT, "TurretRange" ),
	DEFINE_FIELD( m_bHasHysteresis, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bLowAmmo, FIELD_BOOLEAN ),	
END_DATADESC()

BEGIN_ENT_SCRIPTDESC( CASW_Sentry_Top, CBaseCombatCharacter, "sentry top" )
	DEFINE_SCRIPTFUNC_NAMED( ScriptPreventFiringUntil, "PreventFiringUntil", "" )
END_SCRIPTDESC()


#define ASW_SENTRY_FIRE_RATE 0.1f		// time in seconds between each shot

CASW_Sentry_Top::CASW_Sentry_Top()
{
	m_flShootRange = ASW_SENTRY_RANGE;
	m_iAmmoType = GetAmmoDef()->Index( "ASW_R" );
	m_iBaseTurnRate = ASW_SENTRY_TURNRATE;
	m_iSentryAngle = ASW_SENTRY_ANGLE;
	m_bLowAmmo = false;
}


CASW_Sentry_Top::~CASW_Sentry_Top()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CASW_Sentry_Top::Spawn( void )
{
	SetSolid( SOLID_NONE );
	SetSolidFlags( 0 );
	SetMoveCollide( MOVECOLLIDE_DEFAULT );
	SetMoveType( MOVETYPE_NONE );
	SetCollisionGroup( COLLISION_GROUP_DEBRIS );

	Precache();
	SetTopModel();

	BaseClass::Spawn();

	AddEFlags( EFL_NO_DISSOLVE | EFL_NO_MEGAPHYSCANNON_RAGDOLL | EFL_NO_PHYSCANNON_INTERACTION );
	ChangeFaction( FACTION_MARINES );

	m_fDeployYaw = GetAbsAngles().y;
	m_fCenterAimYaw = GetAbsAngles().y;
	m_fCurrentYaw = GetAbsAngles().y;

	if ( GetMoveParent() )
	{
		m_fDeployYaw -= GetMoveParent()->GetAbsAngles().y;
	}

	SetThink( &CASW_Sentry_Top::AnimThink );
	SetNextThink( gpGlobals->curtime + 0.01f );
}

void CASW_Sentry_Top::Precache()
{
	PrecacheModel( SENTRY_TOP_MODEL );
	PrecacheModel( "models/sentry_gun/freeze_top.mdl" );
	PrecacheModel( "models/sentry_gun/grenade_top.mdl" );
	PrecacheModel( "models/sentry_gun/flame_top.mdl" );
	PrecacheScriptSound( "ASW_Sentry.Fire" );
	PrecacheScriptSound( "ASW_Sentry.Turn" );
	PrecacheScriptSound( "ASW_Sentry.AmmoWarning" );
	PrecacheScriptSound( "ASW_Sentry.OutOfAmmo" );
	PrecacheScriptSound( "ASW_Sentry.Deploy" );
	PrecacheScriptSound( "ASW_Sentry.CannonFire" );
	PrecacheScriptSound( "ASW_Sentry.FlameLoop" );
	PrecacheScriptSound( "ASW_Sentry.FlameStop" );
	PrecacheScriptSound( "ASW_Sentry.IceLoop" );
	PrecacheScriptSound( "ASW_Sentry.IceStop" );
	PrecacheParticleSystem( "asw_flamethrower" );
	PrecacheParticleSystem( "asw_freezer_spray" );
	BaseClass::Precache();
}

void CASW_Sentry_Top::SetTopModel()
{
	SetModel( SENTRY_TOP_MODEL );
}

int CASW_Sentry_Top::ShouldTransmit( const CCheckTransmitInfo *pInfo )
{
	return FL_EDICT_ALWAYS;
}

int CASW_Sentry_Top::UpdateTransmitState()
{
	return SetTransmitState( FL_EDICT_FULLCHECK );
}

void CASW_Sentry_Top::ScriptPreventFiringUntil( float fNextFireTime )
{
	m_fNextFireTime = fNextFireTime;
}

void CASW_Sentry_Top::AnimThink( void )
{	
	float deltatime = gpGlobals->curtime - m_fLastThinkTime;
	m_fLastThinkTime = gpGlobals->curtime;
	SetNextThink( gpGlobals->curtime + 0.01f );
		
	m_iEnemySkip++;
	if (m_iEnemySkip >= 5)
	{
		m_iEnemySkip = 0;
		FindEnemy();
	}

	UpdateGoal();
	TurnToGoal(deltatime);
	CheckFiring();
	StudioFrameAdvance();
}

void CASW_Sentry_Top::SetSentryBase(CASW_Sentry_Base* pSentryBase)
{
	m_hSentryBase = pSentryBase;
	SetParent(pSentryBase);
	SetLocalOrigin(vec3_origin);
}

float CASW_Sentry_Top::GetDeployYaw()
{
	float fDeployYaw = m_fDeployYaw;
	if ( GetMoveParent() )
	{
		fDeployYaw += GetMoveParent()->GetAbsAngles().y;
	}

	float fCurrentYaw = GetAbsAngles().y;

	return fCurrentYaw + anglemod( fDeployYaw - fCurrentYaw );
}

void CASW_Sentry_Top::SetDeployYaw( float yaw )
{
	if ( GetMoveParent() )
	{
		yaw -= GetMoveParent()->GetAbsAngles().y;
	}

	m_fDeployYaw = anglemod( yaw );
}

void CASW_Sentry_Top::UpdateGoal()
{
	if ( !m_hEnemy.IsValid() || !m_hEnemy.Get() )
	{
		m_fGoalYaw = GetDeployYaw();
	}
	else
	{
		// set our goal yaw to point at the enemy
		m_fGoalYaw = GetYawTo( m_hEnemy );
	}
}

void CASW_Sentry_Top::TurnToGoal( float deltatime )
{
	float fDist = m_fGoalYaw - m_fCurrentYaw;

	if ( fDist != 0.0f )
	{
		PlayTurnSound();

		if ( fDist > 180.0f )
		{
			fDist = -( 360.0f - fDist );
		}
		else if ( fDist < -180.0f )
		{
			fDist = 360.0f + fDist;
		}

		// set our turn rate depending on if we have an enemy or not
		float fTurnRate = ASW_SENTRY_TURNRATE * 0.5f;
		if ( m_hEnemy.IsValid() && m_hEnemy.Get() )
			fTurnRate = ASW_SENTRY_TURNRATE;

		if ( fabs( fDist ) < deltatime * fTurnRate )
		{
			m_fCurrentYaw = m_fGoalYaw;
		}
		else
		{
			// turn it		
			m_fCurrentYaw += deltatime * fTurnRate * ( fDist > 0.0f ? 1.0f : -1.0f );

			if ( m_fCurrentYaw < -180.0f )
			{
				m_fCurrentYaw += 360.0f;
			}
			else if ( m_fCurrentYaw > 180.0f )
			{
				m_fCurrentYaw -= 360.0f;
			}
		}

		if ( !m_hEnemy.IsValid() || !m_hEnemy.Get() )
		{
			m_fCenterAimYaw = m_fCurrentYaw;
		}
	}

	QAngle ang = GetAbsAngles();
	ang.y = m_fCurrentYaw;
	SetAbsAngles( ang );
}

void CASW_Sentry_Top::FindEnemy()
{
	bool bFindNewEnemy = true;
	bool bHadEnemy = (m_hEnemy.IsValid() && m_hEnemy.Get());
	// if have an enemy and it is alive
	if (m_hEnemy.IsValid() && m_hEnemy.Get() && m_hEnemy->GetHealth() > 0 )
	{		
		// check for LOS to enemy
		if (CanSee(m_hEnemy))
			bFindNewEnemy = false;
		// reject if enemy is somehow invalid now
		CAI_BaseNPC *pNPC = m_hEnemy.Get()->MyNPCPointer();
		if ( pNPC && !IsValidEnemy(pNPC) )
			bFindNewEnemy = true;
		Disposition_t rel = IRelationType( m_hEnemy );
		if ( !asw_sentry_friendly_target.GetBool() && rel != D_HATE && rel != D_FEAR )
			bFindNewEnemy = true;
	}

	if (bFindNewEnemy)
	{
		if ( g_vecTargetDummies.Count() > 0 )
		{
			m_hEnemy = g_vecTargetDummies[0];
		}
		else
		{
			m_hEnemy = SelectOptimalEnemy();
		}
	}
	// acquired a new enemy
	if (!bHadEnemy && m_hEnemy.IsValid() && m_hEnemy.Get())
	{
		PlayTurnSound();
	}
}


Vector CASW_Sentry_Top::GetEnemyVelocity( CBaseEntity *pEnemy )
{
	if ( !pEnemy )
		pEnemy = m_hEnemy.Get();

	// hacky quick hacky and dirty hack hack hack to deal with GetVelocity() returning
	// ideal rather than actual velocity for drones
	if ( pEnemy )
	{
		if ( pEnemy->Classify() == CLASS_ASW_DRONE )
		{
			return assert_cast< CASW_Drone_Advanced * >( pEnemy )->GetMotor()->GetCurVel();
		}
		else
		{
			Vector vel;
			pEnemy->GetVelocity( &vel );
			return vel;
		}
	}
	return vec3_origin;
}

CAI_BaseNPC *CASW_Sentry_Top::SelectOptimalEnemy()
{
	// search through all npcs, any that are in LOS and have health
	CAI_BaseNPC **ppAIs = g_AI_Manager.AccessAIs();

	for ( int i = 0; i < g_AI_Manager.NumAIs(); i++ )
	{
		if ( ppAIs[i]->GetHealth() > 0 && CanSee( ppAIs[i] ) && IsValidEnemy( ppAIs[i] ) )
		{
			if ( !asw_sentry_friendly_target.GetBool() )
			{
				Disposition_t rel = IRelationType( ppAIs[i] );
				if ( rel != D_HATE && rel != D_FEAR )
				{
					Assert( rel == D_LIKE || rel == D_NEUTRAL );
					continue;
				}
			}

			return ppAIs[i];
		}
	}

	return NULL;
}

void CASW_Sentry_Top::PlayTurnSound()
{
	if ( gpGlobals->curtime >= m_flNextTurnSound )
	{
		EmitSound( "ASW_Sentry.Turn" );
		m_flNextTurnSound = gpGlobals->curtime + 0.5f;
	}
}

void CASW_Sentry_Top::Fire( void )
{
	if ( m_flTimeFirstFired == 0.0f )
	{
		m_flTimeFirstFired = gpGlobals->curtime;
	}
}

void CASW_Sentry_Top::OnUsedQuarterAmmo( void )
{
	if ( gpGlobals->curtime - m_flTimeFirstFired < 30.0f )
	{
		ASWFailAdvice()->OnSentryUsedWell();	
	}
}

void CASW_Sentry_Top::OnLowAmmo( void )
{
	EmitSound( "ASW_Sentry.AmmoWarning" );
	m_bLowAmmo = true;
}

void CASW_Sentry_Top::OnOutOfAmmo( void )
{
	EmitSound( "ASW_Sentry.OutOfAmmo" );
	if ( ASWDeathmatchMode() )
	{
		//Msg ("Sentry top taking damage to death %i \n", GetSentryBase()->GetHealth() + 1);
		CTakeDamageInfo info( this,
			this,
			GetSentryBase()->GetHealth() + 1,
			DMG_GENERIC );
		GetSentryBase()->TakeDamage( info );
	}
}

bool CASW_Sentry_Top::CanSee( CBaseEntity *pEnt )
{
	if ( !pEnt )
		return false;
	// check if it's a valid target
	// check if its in range
	Vector vFiringPos = GetFiringPosition();

	Vector diff = pEnt->WorldSpaceCenter() - vFiringPos;
	float distance = diff.Length();
	if ( distance > GetRange() )
	{
		m_iCanSeeError = 0;
		return false;
	}
	// check the z angle is within X degrees of horizontal, if the z diff is significant
	if ( fabs( diff.z ) > 50.0f )
	{
		float fPitchDiff = fabs( UTIL_AngleDiff( GetPitchTo( pEnt ), 0 ) );
		if ( fPitchDiff > 360.0f )
			fPitchDiff -= 360.0f;
		if ( fabs( fPitchDiff ) > 30.0f )
		{
			m_iCanSeeError = 1;
			return false;
		}
	}
	// check if the angle is within X degrees of our deploy radius
	float fYawDiff = fabs( UTIL_AngleDiff( CASW_Sentry_Top::GetYawTo( pEnt ), rd_sentry_rotates_aim_instantly.GetBool() ? GetDeployYaw() : m_fCenterAimYaw ) );
	if ( fYawDiff > 360.0f )
		fYawDiff -= 360.0f;

	if ( fabs( fYawDiff ) > ASW_SENTRY_ANGLE )
	{
		m_iCanSeeError = 1;
		return false;
	}

	// do a trace from our shoot position to the enemy
	trace_t		tr;

	if ( Classify() == CLASS_ASW_SENTRY_FLAMER )
	{
		CTraceFilterSkipClassname traceFilter( GetSentryBase(), "asw_flamer_projectile", COLLISION_GROUP_NONE );
		UTIL_TraceLine( vFiringPos, pEnt->WorldSpaceCenter(), MASK_OPAQUE_AND_NPCS, &traceFilter, &tr );
	}
	else if ( Classify() == CLASS_ASW_SENTRY_FREEZE )
	{
		CTraceFilterSkipClassname traceFilter( GetSentryBase(), "asw_extinguisher_projectile", COLLISION_GROUP_NONE );
		UTIL_TraceLine( vFiringPos, pEnt->WorldSpaceCenter(), MASK_OPAQUE_AND_NPCS, &traceFilter, &tr );
	}
	else //if ( Classify() == CLASS_ASW_SENTRY_GUN || Classify() == CLASS_ASW_SENTRY_CANNON )
	{
		CTraceFilterSimple traceFilter( GetSentryBase(), COLLISION_GROUP_NONE );
		UTIL_TraceLine( vFiringPos, pEnt->WorldSpaceCenter(), MASK_OPAQUE_AND_NPCS, &traceFilter, &tr );
	}

	m_iCanSeeError = 2;
	bool bClear = tr.fraction == 1.0;
	if ( !bClear )
	{
		if ( tr.m_pEnt == pEnt )
			return true;
	}
	return bClear;
}

float CASW_Sentry_Top::GetYawTo( CBaseEntity *pEnt )
{
	if ( !pEnt )
		return m_fCenterAimYaw;

	Vector diff = pEnt->WorldSpaceCenter() - GetAbsOrigin();
	if ( diff.x == 0 && diff.y == 0 && diff.z == 0 )
		return m_fCenterAimYaw;

	return UTIL_VecToYaw( diff );
}

float CASW_Sentry_Top::GetPitchTo( CBaseEntity *pEnt )
{
	if ( !pEnt )
		return 0;

	Vector diff = pEnt->WorldSpaceCenter() - GetFiringPosition();
	if ( diff.x == 0 && diff.y == 0 && diff.z == 0 )
		return 0;

	return UTIL_VecToPitch( diff );
}

bool CASW_Sentry_Top::IsValidEnemy( CAI_BaseNPC *pNPC )
{
	if ( !pNPC )
		return false;

	if ( pNPC->GetFlags() & FL_NOTARGET )
		return false;

	return true;
}

void CASW_Sentry_Top::CheckFiring()
{
	if ( gpGlobals->curtime > m_fNextFireTime && HasAmmo() && ( m_bHasHysteresis || m_hEnemy.Get() ) )
	{
		float flDist = fabs( m_fGoalYaw - m_fCurrentYaw );
		flDist = fsel( flDist - 180, 360 - flDist, flDist );

		if ( ( flDist < ASW_SENTRY_FIRE_ANGLE_THRESHOLD ) || ( m_bHasHysteresis && !m_hEnemy ) )
		{
			Fire();
		}
	}
}

Vector CASW_Sentry_Top::GetFiringPosition()
{
	Vector vMuzzlePos;
	GetAttachment( "muzzle", vMuzzlePos );

	return vMuzzlePos;
}

CASW_Sentry_Base* CASW_Sentry_Top::GetSentryBase()
{	
	return m_hSentryBase.Get();
}

int CASW_Sentry_Top::GetSentryDamage()
{
	float flDamage = 10.0f;

	return flDamage * ( GetSentryBase() ? GetSentryBase()->m_fDamageScale : 1.0f );
}

bool CASW_Sentry_Top::HasAmmo()
{
	if ( !GetSentryBase() )
	{
		return true;
	}

	return ( GetSentryBase() && GetSentryBase()->GetAmmo() > 0 );
}

int CASW_Sentry_Top::GetAmmo()
{
	return ( GetSentryBase() ? GetSentryBase()->GetAmmo() : 0 );
}

void CASW_Sentry_Top::MakeTracer( const Vector &vecTracerSrc, const trace_t &tr, int iTracerType )
{
	CBroadcastRecipientFilter filter;
	
	UserMessageBegin( filter, "ASWSentryTracer" );
		WRITE_SHORT( entindex() );
		WRITE_FLOAT( tr.endpos.x );
		WRITE_FLOAT( tr.endpos.y );
		WRITE_FLOAT( tr.endpos.z );
	MessageEnd();
}
