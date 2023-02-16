//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Combine Zombie... Zombie Combine... its like a... Zombine... get it?
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "npc_zombine.h"
#include "ai_basenpc.h"
#include "ai_default.h"
#include "ai_schedule.h"
#include "ai_hull.h"
#include "ai_motor.h"
#include "ai_memory.h"
#include "ai_route.h"
#include "ai_squad.h"
#include "soundent.h"
#include "game.h"
#include "npcevent.h"
#include "entitylist.h"
#include "ai_task.h"
#include "activitylist.h"
#include "engine/IEngineSound.h"
#include "movevars_shared.h"
#include "IEffects.h"
#include "props.h"
#include "physics_npc_solver.h"
#include "asw_gamerules.h"
#include "asw_grenade_cluster.h"
#include "asw_util_shared.h"
#include "asw_marine.h"

#include "basecombatweapon.h"
#include "asw_grenade_vindicator.h"

#include "ai_interactions.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

enum
{	
	SQUAD_SLOT_ZOMBINE_SPRINT1 = LAST_SHARED_SQUADSLOT,
	SQUAD_SLOT_ZOMBINE_SPRINT2,
};

#define MIN_SPRINT_TIME 3.5f
#define MAX_SPRINT_TIME 5.5f

#define MIN_SPRINT_DISTANCE 64.0f
#define MAX_SPRINT_DISTANCE 1024.0f

#define SPRINT_CHANCE_VALUE 10
#define SPRINT_CHANCE_VALUE_DARKNESS 50

#define GRENADE_PULL_MAX_DISTANCE 256.0f

#define ZOMBINE_MAX_GRENADES 1

int ACT_ZOMBINE_GRENADE_PULL;
int ACT_ZOMBINE_GRENADE_WALK;
int ACT_ZOMBINE_GRENADE_RUN;
int ACT_ZOMBINE_GRENADE_IDLE;
int ACT_ZOMBINE_ATTACK_FAST;
int ACT_ZOMBINE_GRENADE_FLINCH_BACK;
int ACT_ZOMBINE_GRENADE_FLINCH_FRONT;
int ACT_ZOMBINE_GRENADE_FLINCH_WEST;
int ACT_ZOMBINE_GRENADE_FLINCH_EAST;

int AE_ZOMBINE_PULLPIN;

#ifdef HL2_DLL
extern bool IsAlyxInDarknessMode();
#endif

ConVar	sk_zombie_soldier_health( "sk_zombie_soldier_health", "100", FCVAR_CHEAT );

float g_flZombineGrenadeTimes = 0;

LINK_ENTITY_TO_CLASS( npc_zombine, CNPC_Zombine );

BEGIN_DATADESC( CNPC_Zombine )
	DEFINE_FIELD( m_flSprintTime, FIELD_TIME ),
	DEFINE_FIELD( m_flSprintRestTime, FIELD_TIME ),
	DEFINE_FIELD( m_flSuperFastAttackTime, FIELD_TIME ),
	DEFINE_FIELD( m_hGrenade, FIELD_EHANDLE ),
	DEFINE_FIELD( m_flGrenadePullTime, FIELD_TIME ),
	DEFINE_KEYFIELD( m_iGrenadeCount, FIELD_INTEGER, "NumGrenades" ),
	DEFINE_KEYFIELD( m_iszGrenadeClass, FIELD_STRING, "GrenadeClass" ),
	DEFINE_KEYFIELD( m_flGrenadeFuseLength, FIELD_FLOAT, "GrenadeFuseLength" ),
	DEFINE_KEYFIELD( m_iGrenadeClusters, FIELD_INTEGER, "GrenadeClusters" ),
	DEFINE_INPUTFUNC( FIELD_VOID,	"StartSprint", InputStartSprint ),
	DEFINE_INPUTFUNC( FIELD_VOID,	"PullGrenade", InputPullGrenade ),
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CNPC_Zombine, DT_NPC_Zombine )
	SendPropEHandle( SENDINFO( m_hGrenade ) ),
END_SEND_TABLE()

//---------------------------------------------------------
//---------------------------------------------------------
const char *CNPC_Zombine::pMoanSounds[] =
{
	"ATV_engine_null",
};

CNPC_Zombine::CNPC_Zombine()
{
	m_pszAlienModelName = "models/zombie/zombie_soldier.mdl";
	m_iGrenadeCount = ZOMBINE_MAX_GRENADES;
	m_iszGrenadeClass = AllocPooledString( "npc_grenade_frag" );
	m_flGrenadeFuseLength = 3.5f;
	m_iGrenadeClusters = 0;
}

void CNPC_Zombine::Spawn( void )
{
	Precache();

	m_fIsTorso = false;
	m_fIsHeadless = false;
	
#ifdef HL2_EPISODIC
	SetBloodColor( BLOOD_COLOR_ZOMBIE );
#else
	SetBloodColor( BLOOD_COLOR_GREEN );
#endif // HL2_EPISODIC

	m_iHealth			= sk_zombie_soldier_health.GetFloat();
	SetMaxHealth( m_iHealth );

	m_flFieldOfView		= 0.2;

	CapabilitiesClear();

	BaseClass::Spawn();

	m_flSprintTime = 0.0f;
	m_flSprintRestTime = 0.0f;

	m_flNextMoanSound = gpGlobals->curtime + random->RandomFloat( 1.0, 4.0 );

	g_flZombineGrenadeTimes = gpGlobals->curtime;
	m_flGrenadePullTime = gpGlobals->curtime;
}

void CNPC_Zombine::Precache( void )
{
	BaseClass::Precache();

	PrecacheModel( "models/zombie/zombie_soldier.mdl" );

	PrecacheScriptSound( "Zombie.FootstepRight" );
	PrecacheScriptSound( "Zombie.FootstepLeft" );
	PrecacheScriptSound( "Zombine.ScuffRight" );
	PrecacheScriptSound( "Zombine.ScuffLeft" );
	PrecacheScriptSound( "Zombie.AttackHit" );
	PrecacheScriptSound( "Zombie.AttackMiss" );
	PrecacheScriptSound( "Zombine.Pain" );
	PrecacheScriptSound( "Zombine.Die" );
	PrecacheScriptSound( "Zombine.Alert" );
	PrecacheScriptSound( "Zombine.Idle" );
	PrecacheScriptSound( "Zombine.ReadyGrenade" );

	PrecacheScriptSound( "ATV_engine_null" );
	PrecacheScriptSound( "Zombine.Charge" );
	PrecacheScriptSound( "Zombie.Attack" );

	UTIL_PrecacheOther( STRING( m_iszGrenadeClass ) );
}

void CNPC_Zombine::SetZombieModel( void )
{
	SetModel( "models/zombie/zombie_soldier.mdl" );
	SetHullType( HULL_HUMAN );

	SetBodygroup( ZOMBIE_BODYGROUP_HEADCRAB, !m_fIsHeadless );

	SetHullSizeNormal( true );
	SetDefaultEyeOffset();
	SetActivity( ACT_IDLE );
}

int CNPC_Zombine::GetBaseHealth()
{
	return sk_zombie_soldier_health.GetInt();
}

void CNPC_Zombine::PrescheduleThink( void )
{
	GatherGrenadeConditions();

	if( gpGlobals->curtime > m_flNextMoanSound )
	{
		if( CanPlayMoanSound() )
		{
			// Classic guy idles instead of moans.
			IdleSound();

			m_flNextMoanSound = gpGlobals->curtime + random->RandomFloat( 10.0, 15.0 );
		}
		else
		{
			m_flNextMoanSound = gpGlobals->curtime + random->RandomFloat( 2.5, 5.0 );
		}
	}

	if ( HasGrenade () )
	{
		CSoundEnt::InsertSound ( SOUND_DANGER, GetAbsOrigin() + GetSmoothedVelocity() * 0.5f , 256, 0.1, this, SOUNDENT_CHANNEL_ZOMBINE_GRENADE );

		if( IsSprinting() && GetEnemy() && GetEnemy()->Classify() == CLASS_PLAYER_ALLY_VITAL && HasCondition( COND_SEE_ENEMY ) )
		{
			if( GetAbsOrigin().DistToSqr(GetEnemy()->GetAbsOrigin()) < Square( 144 ) )
			{
				StopSprint();
			}
		}
	}

	BaseClass::PrescheduleThink();
}

void CNPC_Zombine::OnScheduleChange( void )
{
	if ( HasCondition( COND_CAN_MELEE_ATTACK1 ) && IsSprinting() == true )
	{
		m_flSuperFastAttackTime = gpGlobals->curtime + 1.0f;
	}

	BaseClass::OnScheduleChange();
}
bool CNPC_Zombine::CanRunAScriptedNPCInteraction( bool bForced )
{
	if ( HasGrenade() == true )
		return false;

	return BaseClass::CanRunAScriptedNPCInteraction( bForced );
}

int CNPC_Zombine::SelectSchedule( void )
{
	if ( GetHealth() <= 0 )
		return BaseClass::SelectSchedule();

	if ( HasCondition( COND_ZOMBINE_GRENADE ) )
	{
		ClearCondition( COND_ZOMBINE_GRENADE );
		
		return SCHED_ZOMBINE_PULL_GRENADE;
	}

	return BaseClass::SelectSchedule();
}

void CNPC_Zombine::BuildScheduleTestBits( void )
{
	BaseClass::BuildScheduleTestBits();

	SetCustomInterruptCondition( COND_ZOMBINE_GRENADE );
}

Activity CNPC_Zombine::NPC_TranslateActivity( Activity baseAct )
{
	if ( baseAct == ACT_MELEE_ATTACK1 )
	{
		if ( m_flSuperFastAttackTime > gpGlobals->curtime || HasGrenade() )
		{
			return (Activity)ACT_ZOMBINE_ATTACK_FAST;
		}
	}

	if ( baseAct == ACT_IDLE )
	{
		if ( HasGrenade() )
		{
			return (Activity)ACT_ZOMBINE_GRENADE_IDLE;
		}
	}

	return BaseClass::NPC_TranslateActivity( baseAct );
}

int CNPC_Zombine::MeleeAttack1Conditions ( float flDot, float flDist )
{
	int iBase = BaseClass::MeleeAttack1Conditions( flDot, flDist );

	if( HasGrenade() )
	{
		//Adrian: stop spriting if we get close enough to melee and we have a grenade
		//this gives NPCs time to move away from you (before it was almost impossible cause of the high sprint speed)
		if ( iBase == COND_CAN_MELEE_ATTACK1 )
		{
			StopSprint();
		}
	}

	return iBase;
}

void CNPC_Zombine::GatherGrenadeConditions( void )
{
	if ( m_iGrenadeCount <= 0 )
		return;

	if ( g_flZombineGrenadeTimes > gpGlobals->curtime )
		return;

	if ( m_flGrenadePullTime > gpGlobals->curtime )
		return;

	if ( m_flSuperFastAttackTime >= gpGlobals->curtime )
		return;
	
	if ( HasGrenade() )
		return;

	if ( GetEnemy() == NULL )
		return;

	if ( FVisible( GetEnemy() ) == false )
		return;

	if ( IsSprinting() )
		return;

	if ( IsOnFire() )
		return;
	
	if ( IsRunningDynamicInteraction() == true )
		return;

	if ( m_ActBusyBehavior.IsActive() )
		return;

	float flMarineDist;
	CASW_Marine *pMarine = UTIL_ASW_NearestMarine( GetAbsOrigin(), flMarineDist );

	if ( pMarine && pMarine->FVisible( this ) )
	{
		float flLengthToEnemy = flMarineDist;

		if ( pMarine != GetEnemy() )
		{
			flLengthToEnemy = ( GetEnemy()->GetAbsOrigin() - GetAbsOrigin()).Length();
		}

		if ( flMarineDist <= GRENADE_PULL_MAX_DISTANCE && flLengthToEnemy <= GRENADE_PULL_MAX_DISTANCE )
		{
			float flPullChance = 1.0f - ( flLengthToEnemy / GRENADE_PULL_MAX_DISTANCE );
			m_flGrenadePullTime = gpGlobals->curtime + 0.5f;

			if ( flPullChance >= random->RandomFloat( 0.0f, 1.0f ) )
			{
				g_flZombineGrenadeTimes = gpGlobals->curtime + 10.0f;
				SetCondition( COND_ZOMBINE_GRENADE );
			}
		}
	}
}

int CNPC_Zombine::TranslateSchedule( int scheduleType ) 
{
	return BaseClass::TranslateSchedule( scheduleType );
}

void CNPC_Zombine::DropGrenade( Vector vDir )
{
	if ( m_hGrenade == NULL )
		 return;

	m_hGrenade->SetParent( NULL );
	m_hGrenade->SetOwnerEntity( NULL );

	Vector vGunPos;
	QAngle angles;
	GetAttachment( "grenade_attachment", vGunPos, angles );

	IPhysicsObject *pPhysObj = m_hGrenade->VPhysicsGetObject();

	if ( pPhysObj == NULL )
	{
		m_hGrenade->SetMoveType( MOVETYPE_VPHYSICS );
		m_hGrenade->SetSolid( SOLID_VPHYSICS );
		m_hGrenade->SetCollisionGroup( ASW_COLLISION_GROUP_GRENADES );

		m_hGrenade->CreateVPhysics();
	}

	if ( pPhysObj )
	{
		pPhysObj->Wake();
		pPhysObj->SetPosition( vGunPos, angles, true );
		pPhysObj->ApplyForceCenter( vDir * 0.2f );

		pPhysObj->RecheckCollisionFilter();
	}

	m_hGrenade = NULL;
}

void CNPC_Zombine::Event_Killed( const CTakeDamageInfo &info )
{
	BaseClass::Event_Killed( info );

	if ( HasGrenade() )
	{
		DropGrenade( vec3_origin );
	}
}

//-----------------------------------------------------------------------------
// Purpose:  This is a generic function (to be implemented by sub-classes) to
//			 handle specific interactions between different types of characters
//			 (For example the barnacle grabbing an NPC)
// Input  :  Constant for the type of interaction
// Output :	 true  - if sub-class has a response for the interaction
//			 false - if sub-class has no response
//-----------------------------------------------------------------------------
bool CNPC_Zombine::HandleInteraction( int interactionType, void *data, CBaseCombatCharacter *sourceEnt )
{
#ifdef HL2_DLL
	if ( interactionType == g_interactionBarnacleVictimGrab )
	{
		if ( HasGrenade() )
		{
			DropGrenade( vec3_origin );
		}
	}
#endif

	return BaseClass::HandleInteraction( interactionType, data, sourceEnt );
}

void CNPC_Zombine::TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr )
{
	BaseClass::TraceAttack( info, vecDir, ptr );

	// Only knock grenades off their hands if it's a player or a bot doing the damage.
	if ( info.GetAttacker() && info.GetAttacker()->Classify() != CLASS_ASW_MARINE )
		return;

	if ( info.GetDamageType() & ( DMG_BULLET | DMG_CLUB ) )
	{
		if ( ptr->hitgroup == HITGROUP_LEFTARM )
		{
			if ( HasGrenade() )
			{
				DropGrenade( info.GetDamageForce() );
				StopSprint();
			}
		}
	}
}

void CNPC_Zombine::HandleAnimEvent( animevent_t *pEvent )
{
	if ( pEvent->Event() == AE_ZOMBINE_PULLPIN )
	{
		int iAttachment = LookupAttachment( "grenade_attachment" );

		Vector vecStart;
		QAngle angles;
		GetAttachment( iAttachment, vecStart, angles );

		CBaseEntity *pGrenade = CBaseEntity::Create( STRING( m_iszGrenadeClass ), vecStart, angles, this );

		if ( pGrenade )
		{
			if ( CASW_Grenade_Vindicator *pASWGrenade = dynamic_cast< CASW_Grenade_Vindicator * >( pGrenade ) )
			{
				pASWGrenade->m_hFirer = this;
				pASWGrenade->SetFuseLength( m_flGrenadeFuseLength );
				pASWGrenade->SetClusters( m_iGrenadeClusters, true );
				pASWGrenade->CreateEffects();
				pASWGrenade->m_flDamage = ASWGameRules()->ModifyAlienDamageBySkillLevel( 200.0f );

				// only change physics settings on grenades; if we pulled something weird out of our pocket just let it be
				if ( pGrenade->VPhysicsGetObject() )
					pGrenade->VPhysicsDestroyObject();

				pGrenade->SetMoveType( MOVETYPE_NONE );
				pGrenade->SetSolid( SOLID_NONE );
				pGrenade->SetCollisionGroup( COLLISION_GROUP_DEBRIS );

				pGrenade->SetParent( this, iAttachment );

				m_hGrenade = pGrenade;
			}

			if ( IASW_Spawnable_NPC *pNPC = dynamic_cast< IASW_Spawnable_NPC * >( pGrenade ) )
			{
				// special case for if we pulled out a buzzer or a parasite or something
				pNPC->SetHealthByDifficultyLevel();
				angles.x = 0;
				angles.z = 0;
				pGrenade->SetAbsAngles( angles );
			}
				
			EmitSound( "Zombine.ReadyGrenade" );

			m_iGrenadeCount--;
		}

		return;
	}

	if ( pEvent->Event() == AE_NPC_ATTACK_BROADCAST )
	{
		if ( HasGrenade() )
			return;
	}

	BaseClass::HandleAnimEvent( pEvent );
}

bool CNPC_Zombine::AllowedToSprint( void )
{
	if ( IsOnFire() )
		return false;
	
	//If you're sprinting then there's no reason to sprint again.
	if ( IsSprinting() )
		return false;

	int iChance = SPRINT_CHANCE_VALUE;

#ifdef HL2_DLL
	CHL2_Player *pPlayer = dynamic_cast <CHL2_Player*> ( AI_GetSinglePlayer() );

	if ( pPlayer )
	{
		if ( HL2GameRules()->IsAlyxInDarknessMode() && pPlayer->FlashlightIsOn() == false )
		{
			iChance = SPRINT_CHANCE_VALUE_DARKNESS;
		}

		//Bigger chance of this happening if the player is not looking at the zombie
		if ( pPlayer->FInViewCone( this ) == false )
		{
			iChance *= 2;
		}
	}
#endif

	if ( HasGrenade() ) 
	{
		iChance *= 4;
	}

	//Below 25% health they'll always sprint
	if ( ( GetHealth() > GetMaxHealth() * 0.5f ) )
	{
		if ( IsStrategySlotRangeOccupied( SQUAD_SLOT_ZOMBINE_SPRINT1, SQUAD_SLOT_ZOMBINE_SPRINT2 ) == true )
			return false;
		
		if ( random->RandomInt( 0, 100 ) > iChance )
			return false;
		
		if ( m_flSprintRestTime > gpGlobals->curtime )
			return false;
	}

	float flLength = ( GetEnemy()->WorldSpaceCenter() - WorldSpaceCenter() ).Length();

	if ( flLength > MAX_SPRINT_DISTANCE )
		return false;

	return true;
}

void CNPC_Zombine::StopSprint( void )
{
	GetNavigator()->SetMovementActivity( ACT_WALK );

	m_flSprintTime = gpGlobals->curtime;
	m_flSprintRestTime = m_flSprintTime + random->RandomFloat( 2.5f, 5.0f );
}

void CNPC_Zombine::Sprint( bool bMadSprint )
{
	if ( IsSprinting() )
		return;

	OccupyStrategySlotRange( SQUAD_SLOT_ZOMBINE_SPRINT1, SQUAD_SLOT_ZOMBINE_SPRINT2 );
	GetNavigator()->SetMovementActivity( ACT_RUN );

	float flSprintTime = random->RandomFloat( MIN_SPRINT_TIME, MAX_SPRINT_TIME );

	//If holding a grenade then sprint until it blows up.
	if ( HasGrenade() || bMadSprint == true )
	{
		flSprintTime = 9999;
	}

	m_flSprintTime = gpGlobals->curtime + flSprintTime;

	//Don't sprint for this long after I'm done with this sprint run.
	m_flSprintRestTime = m_flSprintTime + random->RandomFloat( 2.5f, 5.0f );

	EmitSound( "Zombine.Charge" );
}

void CNPC_Zombine::RunTask( const Task_t *pTask )
{
	switch ( pTask->iTask )
	{
		case TASK_WAIT_FOR_MOVEMENT_STEP:
		case TASK_WAIT_FOR_MOVEMENT:
		{
			BaseClass::RunTask( pTask );

			if ( IsOnFire() && IsSprinting() )
			{
				StopSprint();
			}

			//Only do this if I have an enemy
			if ( GetEnemy() )
			{
				if ( AllowedToSprint() == true )
				{
					Sprint( ( GetHealth() <= GetMaxHealth() * 0.5f ) );
					return;
				}

				if ( HasGrenade() )
				{
					if ( IsSprinting() )
					{
						GetNavigator()->SetMovementActivity( (Activity)ACT_ZOMBINE_GRENADE_RUN );
					}
					else
					{
						GetNavigator()->SetMovementActivity( (Activity)ACT_ZOMBINE_GRENADE_WALK );
					}

					return;
				}

				if ( GetNavigator()->GetMovementActivity() != ACT_WALK )
				{
					if ( IsSprinting() == false )
					{
						GetNavigator()->SetMovementActivity( ACT_WALK );
					}
				}
			}
			else
			{
				GetNavigator()->SetMovementActivity( ACT_WALK );
			}
		
			break;
		}
		default:
		{
			BaseClass::RunTask( pTask );
			break;
		}
	}
}

void CNPC_Zombine::InputStartSprint ( inputdata_t &inputdata )
{
	Sprint();
}

void CNPC_Zombine::InputPullGrenade ( inputdata_t &inputdata )
{
	g_flZombineGrenadeTimes = gpGlobals->curtime + 5.0f;
	SetCondition( COND_ZOMBINE_GRENADE );
}

//-----------------------------------------------------------------------------
// Purpose: Returns a moan sound for this class of zombie.
//-----------------------------------------------------------------------------
const char *CNPC_Zombine::GetMoanSound( int nSound )
{
	return pMoanSounds[ nSound % ARRAYSIZE( pMoanSounds ) ];
}

//-----------------------------------------------------------------------------
// Purpose: Sound of a footstep
//-----------------------------------------------------------------------------
void CNPC_Zombine::FootstepSound( bool fRightFoot )
{
	if( fRightFoot )
	{
		EmitSound( "Zombie.FootstepRight" );
	}
	else
	{
		EmitSound( "Zombie.FootstepLeft" );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Overloaded so that explosions don't split the zombine in twain.
//-----------------------------------------------------------------------------
bool CNPC_Zombine::ShouldBecomeTorso( const CTakeDamageInfo &info, float flDamageThreshold )
{
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Sound of a foot sliding/scraping
//-----------------------------------------------------------------------------
void CNPC_Zombine::FootscuffSound( bool fRightFoot )
{
	if( fRightFoot )
	{
		EmitSound( "Zombine.ScuffRight" );
	}
	else
	{
		EmitSound( "Zombine.ScuffLeft" );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Play a random attack hit sound
//-----------------------------------------------------------------------------
void CNPC_Zombine::AttackHitSound( void )
{
	EmitSound( "Zombie.AttackHit" );
}

//-----------------------------------------------------------------------------
// Purpose: Play a random attack miss sound
//-----------------------------------------------------------------------------
void CNPC_Zombine::AttackMissSound( void )
{
	// Play a random attack miss sound
	EmitSound( "Zombie.AttackMiss" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Zombine::PainSound( const CTakeDamageInfo &info )
{
	// We're constantly taking damage when we are on fire. Don't make all those noises!
	if ( IsOnFire() )
	{
		return;
	}

	EmitSound( "Zombine.Pain" );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CNPC_Zombine::DeathSound( const CTakeDamageInfo &info ) 
{
	EmitSound( "Zombine.Die" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Zombine::AlertSound( void )
{
	EmitSound( "Zombine.Alert" );

	// Don't let a moan sound cut off the alert sound.
	m_flNextMoanSound += random->RandomFloat( 2.0, 4.0 );
}

//-----------------------------------------------------------------------------
// Purpose: Play a random idle sound.
//-----------------------------------------------------------------------------
void CNPC_Zombine::IdleSound( void )
{
	if( GetState() == NPC_STATE_IDLE && random->RandomFloat( 0, 1 ) == 0 )
	{
		// Moan infrequently in IDLE state.
		return;
	}

	if( IsSlumped() )
	{
		// Sleeping zombies are quiet.
		return;
	}

	EmitSound( "Zombine.Idle" );
	MakeAISpookySound( 360.0f );
}

//-----------------------------------------------------------------------------
// Purpose: Play a random attack sound.
//-----------------------------------------------------------------------------
void CNPC_Zombine::AttackSound( void )
{
	
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
const char *CNPC_Zombine::GetHeadcrabModel( void )
{
	return "models/headcrabclassic.mdl";
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CNPC_Zombine::GetLegsModel( void )
{
	return "models/zombie/zombie_soldier_legs.mdl";
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
const char *CNPC_Zombine::GetTorsoModel( void )
{
	return "models/zombie/zombie_soldier_torso.mdl";
}

//---------------------------------------------------------
// Classic zombie only uses moan sound if on fire.
//---------------------------------------------------------
void CNPC_Zombine::MoanSound( envelopePoint_t *pEnvelope, int iEnvelopeSize )
{
	if( IsOnFire() )
	{
		BaseClass::MoanSound( pEnvelope, iEnvelopeSize );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Returns the classname (ie "npc_headcrab") to spawn when our headcrab bails.
//-----------------------------------------------------------------------------
const char *CNPC_Zombine::GetHeadcrabClassname( void )
{
	return "npc_headcrab";
}

void CNPC_Zombine::ReleaseGrenade( Vector vPhysgunPos )
{
	if ( HasGrenade() == false )
		return;

	Vector vDir = vPhysgunPos - m_hGrenade->GetAbsOrigin();
	VectorNormalize( vDir );

	Activity aActivity;

	Vector vForward, vRight;
	GetVectors( &vForward, &vRight, NULL );

	float flDotForward	= DotProduct( vForward, vDir );
	float flDotRight	= DotProduct( vRight, vDir );

	bool bNegativeForward = false;
	bool bNegativeRight = false;

	if ( flDotForward < 0.0f )
	{
		bNegativeForward = true;
		flDotForward = flDotForward * -1;
	}

	if ( flDotRight < 0.0f )
	{
		bNegativeRight = true;
		flDotRight = flDotRight * -1;
	}

	if ( flDotRight > flDotForward )
	{
		if ( bNegativeRight == true )
			aActivity = (Activity)ACT_ZOMBINE_GRENADE_FLINCH_WEST;
		else 
			aActivity = (Activity)ACT_ZOMBINE_GRENADE_FLINCH_EAST;
	}
	else
	{
		if ( bNegativeForward == true )
			aActivity = (Activity)ACT_ZOMBINE_GRENADE_FLINCH_BACK;
		else 
			aActivity = (Activity)ACT_ZOMBINE_GRENADE_FLINCH_FRONT;
	}

	AddGesture( aActivity );

	DropGrenade( vec3_origin );

	if ( IsSprinting() )
	{
		StopSprint();
	}
	else
	{
		Sprint();
	}
}

CBaseEntity *CNPC_Zombine::OnFailedPhysGunPickup( Vector vPhysgunPos )
{
	CBaseEntity *pGrenade = m_hGrenade;
	ReleaseGrenade( vPhysgunPos );
	return pGrenade;
}

//-----------------------------------------------------------------------------
//
// Schedules
//
//-----------------------------------------------------------------------------

AI_BEGIN_CUSTOM_NPC( npc_zombine, CNPC_Zombine )

	//Squad slots
	DECLARE_SQUADSLOT( SQUAD_SLOT_ZOMBINE_SPRINT1 )
	DECLARE_SQUADSLOT( SQUAD_SLOT_ZOMBINE_SPRINT2 )

	DECLARE_CONDITION( COND_ZOMBINE_GRENADE )

	DECLARE_ACTIVITY( ACT_ZOMBINE_GRENADE_PULL )
	DECLARE_ACTIVITY( ACT_ZOMBINE_GRENADE_WALK )
	DECLARE_ACTIVITY( ACT_ZOMBINE_GRENADE_RUN )
	DECLARE_ACTIVITY( ACT_ZOMBINE_GRENADE_IDLE )
	DECLARE_ACTIVITY( ACT_ZOMBINE_ATTACK_FAST )
	DECLARE_ACTIVITY( ACT_ZOMBINE_GRENADE_FLINCH_BACK )
	DECLARE_ACTIVITY( ACT_ZOMBINE_GRENADE_FLINCH_FRONT )
	DECLARE_ACTIVITY( ACT_ZOMBINE_GRENADE_FLINCH_WEST)
	DECLARE_ACTIVITY( ACT_ZOMBINE_GRENADE_FLINCH_EAST )

	DECLARE_ANIMEVENT( AE_ZOMBINE_PULLPIN )


	DEFINE_SCHEDULE
	(
	SCHED_ZOMBINE_PULL_GRENADE,

	"	Tasks"
	"		TASK_PLAY_SEQUENCE					ACTIVITY:ACT_ZOMBINE_GRENADE_PULL"


	"	Interrupts"

	)

AI_END_CUSTOM_NPC()
