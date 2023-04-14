#include "cbase.h"
#include "asw_simple_grub.h"
#include "asw_trace_filter_melee.h"
#include "te_effect_dispatch.h"
#include "asw_util_shared.h"
#include "asw_marine.h"
#include "asw_marine_resource.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define	SWARM_GRUB_MODEL	"models/swarm/Grubs/Grub.mdl"

LINK_ENTITY_TO_CLASS( asw_grub, CASW_Simple_Grub );
PRECACHE_REGISTER( asw_grub );

BEGIN_DATADESC( CASW_Simple_Grub )
	DEFINE_ENTITYFUNC( GrubTouch ),
END_DATADESC()

extern ConVar asw_debug_simple_alien;
extern ConVar asw_debug_alien_damage;
ConVar rd_grub_health( "rd_grub_health", "1", FCVAR_CHEAT );

// =========================================
// Creation
// =========================================

CASW_Simple_Grub::CASW_Simple_Grub()
{
	m_bSkipGravity = false;
}

CASW_Simple_Grub::~CASW_Simple_Grub()
{
}

void CASW_Simple_Grub::Spawn(void)
{
	BaseClass::Spawn();
	SetCollisionGroup( ASW_COLLISION_GROUP_GRUBS );
	SetModel( STRING( GetModelName() ) );

	SetHullType( HULL_TINY );
	AddSolidFlags( FSOLID_TRIGGER );

	SetTouch( &CASW_Simple_Grub::GrubTouch );

	m_iHealth	= rd_grub_health.GetInt();
	SetBlocksLOS(false);
}

void CASW_Simple_Grub::GrubTouch( CBaseEntity *pOther )
{
	if ( !pOther )
		return;

	if ( pOther->Classify() == CLASS_ASW_MARINE && pOther->GetAbsVelocity().Length() > 100 )
	{
		Vector xydiff = GetAbsOrigin() - pOther->GetAbsOrigin();
		if ( xydiff.Length2D() < 20 )
		{
			CTakeDamageInfo dmg( pOther, pOther, Vector( 0, 0, -1 ), GetAbsOrigin() + Vector( 0, 0, 1 ), 20, DMG_CRUSH );
			TakeDamage( dmg );
		}
	}
}

void CASW_Simple_Grub::Precache()
{
	if ( GetModelName() == NULL_STRING )
	{
		SetModelName( AllocPooledString( SWARM_GRUB_MODEL ) );
	}

	BaseClass::Precache();

	PrecacheModel( "Models/Swarm/Grubs/GrubGib4.mdl" );

	PrecacheEffect( "GrubGib" );
	PrecacheScriptSound( "ASW_Grub.Pain" );
	PrecacheParticleSystem( "grub_death" );
	PrecacheParticleSystem( "grub_death_fire" );
}

// =========================================
// Anims
// =========================================

void CASW_Simple_Grub::PlayRunningAnimation()
{
	ResetSequence( LookupSequence( "CrawlA" ) );
	m_flPlaybackRate = 1.25f;
}

void CASW_Simple_Grub::PlayFallingAnimation()
{
	ResetSequence( LookupSequence( "Jump" ) );
	m_flPlaybackRate = 1.25f;
}

void CASW_Simple_Grub::PlayAttackingAnimation()
{
	ResetSequence( LookupSequence( "CrawlA" ) );
	m_flPlaybackRate = 1.25f;
}

// =========================================
// Sounds
// =========================================

void CASW_Simple_Grub::PainSound( const CTakeDamageInfo &info )
{
	if ( gpGlobals->curtime > m_fNextPainSound )
	{
		EmitSound( "ASW_Grub.Pain" );
		m_fNextPainSound = gpGlobals->curtime + 0.5f;
	}
}

void CASW_Simple_Grub::AlertSound()
{
}

void CASW_Simple_Grub::DeathSound( const CTakeDamageInfo &info )
{
}

void CASW_Simple_Grub::AttackSound()
{
}

// =========================================
// Gibbing
// =========================================

bool CASW_Simple_Grub::ShouldGib( const CTakeDamageInfo &info )
{
	return true;
}

bool CASW_Simple_Grub::CorpseGib( const CTakeDamageInfo &info )
{
	CEffectData	data;
	
	data.m_vOrigin = WorldSpaceCenter();
	data.m_vNormal = data.m_vOrigin - info.GetDamagePosition();
	VectorNormalize( data.m_vNormal );
	data.m_flScale = RemapVal( m_iHealth, 0, -500, 1, 3 );
	data.m_flScale = clamp( data.m_flScale, 1, 3 );
	data.m_fFlags = (IsOnFire() || (info.GetDamageType() & DMG_BURN)) ? ASW_GIBFLAG_ON_FIRE : 0;

	CPASFilter filter( data.m_vOrigin );
	filter.SetIgnorePredictionCull(true);
	DispatchEffect( filter, 0.0, "GrubGib", data );

	return true;
}

// =========================================
// Movement
// =========================================

float CASW_Simple_Grub::GetIdealSpeed() const
{
	return 100.0f;
}

void CASW_Simple_Grub::AlienThink()
{
	float delta = gpGlobals->curtime - m_fLastThinkTime;

	BaseClass::AlienThink();

	UpdatePitch(delta);
}

void CASW_Simple_Grub::UpdatePitch(float delta)
{
	float current = UTIL_AngleMod( GetLocalAngles()[PITCH] );
	float ideal = UTIL_AngleMod( GetIdealPitch() );
	
	float dt = MIN( 0.2, delta );	
	float newPitch = ASW_ClampYaw( GetPitchSpeed(), current, ideal, dt );
		
	if (newPitch != current)
	{
		QAngle angles = GetLocalAngles();
		angles[PITCH] = newPitch;
		SetLocalAngles( angles );
	}
}

float CASW_Simple_Grub::GetPitchSpeed() const
{
	return 2000.0f;
}

float CASW_Simple_Grub::GetIdealPitch()
{
	if (m_bSkipGravity)	// we're climbing
		return -90;
	return 0;
}

float CASW_Simple_Grub::GetFaceEnemyDistance() const
{
	return 50.0f;
}

float CASW_Simple_Grub::GetZigZagChaseDistance() const
{
	return 50.0f;
}

bool CASW_Simple_Grub::ApplyGravity( Vector &vecTarget, float deltatime )
{
	if ( m_bSkipGravity )
	{
		if ( asw_debug_simple_alien.GetBool() )
			Msg( "Skipping grav\n" );

		return false;
	}

	return BaseClass::ApplyGravity( vecTarget, deltatime );
}

// run towards the target like a normal simple alien, but if he hit some geometry, try to climb up over it
//  todo: perform move could check if gravity was skipped and switch us to a climbing anim instead of a running
bool CASW_Simple_Grub::TryMove( const Vector &vecSrc, Vector &vecTarget, float deltatime, bool bStepMove )
{
	m_bSkipGravity = false;

	// do a trace to the dest
	Ray_t ray;
	trace_t trace;
	CTraceFilterSimple traceFilter( this, GetCollisionGroup() );
	ray.Init( vecSrc, vecTarget, GetHullMins(), GetHullMaxs() );
	enginetrace->TraceRay( ray, MASK_NPCSOLID, &traceFilter, &trace );
	if ( trace.startsolid )
	{
		// doh, we're stuck in something!
		//  todo: move us to a safe spot? wait for push out phys props?
		if ( asw_debug_simple_alien.GetBool() )
			Msg( "CASW_Simple_Grub stuck!\n" );
		m_MoveFailure.trace = trace;
		m_MoveFailure.vecStartPos = vecSrc;
		m_MoveFailure.vecTargetPos = vecTarget;
		return false;
	}
	if ( trace.fraction < 0.1f )	 // barely/didn't move
	{
		// try and do a 'stepped up' move to the target
		if ( !bStepMove )
		{
			Vector vecStepSrc = vecSrc;
			vecStepSrc.z += 24;
			Vector vecStepTarget = vecTarget;
			vecTarget.z += 24;
			if ( TryMove( vecStepSrc, vecStepTarget, deltatime, true ) )
			{
				vecTarget = vecStepTarget;
				return true;
			}
		}

		// if we collide with geometry, try to climb up over it		
		if ( trace.m_pEnt && trace.m_pEnt->IsWorld() && GetEnemy() && !bStepMove )
		{
			// check our enemy is on the other side of this obstruction
			Vector vecEnemyDiff = GetEnemy()->GetAbsOrigin() - GetAbsOrigin();
			vecEnemyDiff.NormalizeInPlace();
			if ( vecEnemyDiff.Dot( trace.plane.normal ) < 0 )
			{
				Vector vecClimbSrc = vecSrc;
				Vector vecClimbTarget = vecSrc;
				vecClimbTarget.z += GetIdealSpeed() * deltatime * 0.7f;	// 70% speed when climbing...
				if ( TryMove( vecClimbSrc, vecClimbTarget, deltatime, true ) )
				{
					m_bSkipGravity = true;
					if ( asw_debug_simple_alien.GetBool() )
						Msg( "Setting skip grav, moved %f z=%f\n", GetIdealSpeed() * deltatime * 0.7f, vecClimbTarget.z );	// 70% speed when climbing...
					vecTarget = vecClimbTarget;
					return true;
				}
			}
		}

		m_MoveFailure.trace = trace;
		m_MoveFailure.vecStartPos = vecSrc;
		m_MoveFailure.vecTargetPos = vecTarget;

		return false;
	}
	else if ( trace.fraction < 1 )  // we hit something early, but we did move
	{
		// we hit something early
		m_MoveFailure.trace = trace;
		m_MoveFailure.vecStartPos = vecSrc;
		m_MoveFailure.vecTargetPos = vecTarget;

		vecTarget = trace.endpos;
	}

	return true;
}

void CASW_Simple_Grub::Event_Killed( const CTakeDamageInfo &info )
{
	CBaseEntity *pAttacker = info.GetAttacker();
	if ( pAttacker && pAttacker->Classify() == CLASS_ASW_MARINE )
	{
		CASW_Marine *pMarine = assert_cast< CASW_Marine * >( pAttacker );
		if ( pMarine->GetMarineResource() )
		{
			pMarine->GetMarineResource()->m_iGrubKills++;
		}
	}
	BaseClass::Event_Killed( info );
}

void CASW_Simple_Grub::SetHealthByDifficultyLevel()
{
	if ( asw_debug_alien_damage.GetBool() )
		Msg( "Setting grub's initial health to %d\n", rd_grub_health.GetInt() );
	SetHealth( rd_grub_health.GetInt() );
	SetMaxHealth( rd_grub_health.GetInt() );
}
