//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Defines the headcrab, a tiny, jumpy alien parasite.
//
//=============================================================================//

#ifndef NPC_HEADCRAB_H
#define NPC_HEADCRAB_H
#ifdef _WIN32
#pragma once
#endif

#include "ai_squadslot.h"
#include "asw_alien_jumper.h"
#include "soundent.h"



abstract_class CBaseHeadcrab : public CASW_Alien_Jumper
{
	DECLARE_CLASS( CBaseHeadcrab, CASW_Alien_Jumper );

public:
	void Spawn( void );
	void RunTask( const Task_t *pTask );
	void StartTask( const Task_t *pTask );

	void OnChangeActivity( Activity NewActivity );

	bool IsFirmlyOnGround();
	void MoveOrigin( const Vector &vecDelta );
	void ThrowAt( const Vector &vecPos );
	void ThrowThink( void );
	virtual void JumpAttack( bool bRandomJump, const Vector &vecPos = vec3_origin, bool bThrown = false );
	void JumpToBurrowHint( CAI_Hint *pHint );

	bool	HasHeadroom();
	void	LeapTouch ( CBaseEntity *pOther );
	virtual void TouchDamage( CBaseEntity *pOther );
	virtual bool ShouldGib( const CTakeDamageInfo &info ) { return false; }
	bool	CorpseGib( const CTakeDamageInfo &info );
	void	Touch( CBaseEntity *pOther );
	Vector	BodyTarget( const Vector &posSrc, bool bNoisy = true );
	float	GetAutoAimRadius();
	void	TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr );
	void	Ignite( float flFlameLifetime, bool bNPCOnly = true, float flSize = 0.0f, bool bCalledByLevelDesigner = false );

	float	MaxYawSpeed( void );
	void	GatherConditions( void );
	void	PrescheduleThink( void );
	Class_T Classify( void );
	void	HandleAnimEvent( animevent_t *pEvent );
	int		RangeAttack1Conditions ( float flDot, float flDist );
	int		OnTakeDamage_Alive( const CTakeDamageInfo &info );
	void	ClampRagdollForce( const Vector &vecForceIn, Vector *vecForceOut );
	void	Event_Killed( const CTakeDamageInfo &info );
	void	BuildScheduleTestBits( void );
	bool	FValidateHintType( CAI_Hint *pHint );

	bool	IsJumping( void ) { return m_bMidJump; }

	virtual void BiteSound( void ) = 0;
	virtual void AttackSound( void ) {};
	virtual void ImpactSound( void ) {};
	virtual void TelegraphSound( void ) {};

	virtual int		SelectSchedule( void );
	virtual int		SelectFailSchedule( int failedSchedule, int failedTask, AI_TaskFailureCode_t taskFailCode );
	virtual int		TranslateSchedule( int scheduleType );

	virtual float	GetReactionDelay( CBaseEntity *pEnemy ) { return 0.0; }

	bool			HandleInteraction(int interactionType, void *data, CBaseCombatCharacter* sourceEnt);

	void	CrawlFromCanister();

	virtual	bool		AllowedToIgnite( void ) { return m_bFlammable; }

	virtual bool CanBeAnEnemyOf( CBaseEntity *pEnemy );

	bool	IsHangingFromCeiling( void ) 
	{ 
#ifdef HL2_EPISODIC
		return m_bHangingFromCeiling;	
#else
		return false;
#endif
	}

	virtual void PlayerHasIlluminatedNPC( CBasePlayer *pPlayer, float flDot );
	virtual int GetBaseHealth( void ) override;

	void DropFromCeiling( void );

	DEFINE_CUSTOM_AI;
	DECLARE_DATADESC();

	enum
	{
		SCHED_HEADCRAB_RANGE_ATTACK1 = BaseClass::NEXT_SCHEDULE,
		SCHED_HEADCRAB_WAKE_ANGRY,
		SCHED_HEADCRAB_WAKE_ANGRY_NO_DISPLAY,
		SCHED_HEADCRAB_DROWN,
		SCHED_HEADCRAB_FAIL_DROWN,
		SCHED_HEADCRAB_AMBUSH,
		SCHED_HEADCRAB_HOP_RANDOMLY, // get off something you're not supposed to be on.
		SCHED_HEADCRAB_BARNACLED,
		SCHED_HEADCRAB_UNHIDE,
		SCHED_HEADCRAB_HARASS_ENEMY,
		SCHED_HEADCRAB_FALL_TO_GROUND,
		SCHED_HEADCRAB_RUN_TO_BURROW_IN,
		SCHED_HEADCRAB_RUN_TO_SPECIFIC_BURROW,
		SCHED_HEADCRAB_BURROW_IN,
		SCHED_HEADCRAB_BURROW_WAIT,
		SCHED_HEADCRAB_BURROW_OUT,
		SCHED_HEADCRAB_WAIT_FOR_CLEAR_UNBURROW,
		SCHED_HEADCRAB_CRAWL_FROM_CANISTER,

		SCHED_FAST_HEADCRAB_RANGE_ATTACK1,

		SCHED_HEADCRAB_CEILING_WAIT,
		SCHED_HEADCRAB_CEILING_DROP,

		NEXT_SCHEDULE,
	};

	enum
	{
		TASK_HEADCRAB_HOP_ASIDE = BaseClass::NEXT_TASK,
		TASK_HEADCRAB_HOP_OFF_NPC,
		TASK_HEADCRAB_DROWN,
		TASK_HEADCRAB_WAIT_FOR_BARNACLE_KILL,
		TASK_HEADCRAB_UNHIDE,
		TASK_HEADCRAB_HARASS_HOP,
		TASK_HEADCRAB_FIND_BURROW_IN_POINT,
		TASK_HEADCRAB_BURROW,
		TASK_HEADCRAB_UNBURROW,
		TASK_HEADCRAB_BURROW_WAIT,
		TASK_HEADCRAB_CHECK_FOR_UNBURROW,
		TASK_HEADCRAB_JUMP_FROM_CANISTER,
		TASK_HEADCRAB_CLIMB_FROM_CANISTER,

		TASK_HEADCRAB_CEILING_WAIT,
		TASK_HEADCRAB_CEILING_POSITION,
		TASK_HEADCRAB_CEILING_DETACH,
		TASK_HEADCRAB_CEILING_FALL,
		TASK_HEADCRAB_CEILING_LAND,

		NEXT_TASK,
	};

	enum
	{
		COND_HEADCRAB_IN_WATER = BaseClass::NEXT_CONDITION,
		COND_HEADCRAB_ILLEGAL_GROUNDENT,
		COND_HEADCRAB_BARNACLED,
		COND_HEADCRAB_UNHIDE,

		NEXT_CONDITION,
	};

protected:
	void Leap( const Vector &vecVel );

	void GrabHintNode( CAI_Hint *pHint );
	bool FindBurrow( const Vector &origin, float distance, bool excludeNear );
	bool ValidBurrowPoint( const Vector &point );
	void ClearBurrowPoint( const Vector &origin );
	void Burrow( void );
	void Unburrow( void );
	void SetBurrowed( bool bBurrowed );
	void JumpFromCanister();

	// Begins the climb from the canister
	void BeginClimbFromCanister();

	void InputBurrow( inputdata_t &inputdata );
	void InputBurrowImmediate( inputdata_t &inputdata );
	void InputUnburrow( inputdata_t &inputdata );

	void InputStartHangingFromCeiling( inputdata_t &inputdata );
	void InputDropFromCeiling( inputdata_t &inputdata );

	int CalcDamageInfo( CTakeDamageInfo *pInfo );
	void CreateDust( bool placeDecal = true );

	// Eliminates roll + pitch potentially in the headcrab at canister jump time
	void EliminateRollAndPitch();

	float InnateRange1MinRange( void );
	float InnateRange1MaxRange( void );

protected:
	int		m_nGibCount;
	float	m_flTimeDrown;
	Vector	m_vecCommittedJumpPos;	// The position of our enemy when we locked in our jump attack.

	float	m_flNextNPCThink;
	float	m_flIgnoreWorldCollisionTime;

	bool	m_bCommittedToJump;		// Whether we have 'locked in' to jump at our enemy.
	bool	m_bCrawlFromCanister;
	bool	m_bStartBurrowed;
	bool	m_bBurrowed;
	bool	m_bHidden;
	bool	m_bMidJump;
	bool	m_bAttackFailed;		// whether we ran into a wall during a jump.

	float	m_flBurrowTime;
	int		m_nContext;			// for FValidateHintType context
	int		m_nJumpFromCanisterDir;

	bool	m_bHangingFromCeiling;
	float	m_flIlluminatedTime;
};


//=========================================================
//=========================================================
// The ever popular chubby classic headcrab
//=========================================================
//=========================================================
class CHeadcrab : public CBaseHeadcrab
{
	DECLARE_CLASS( CHeadcrab, CBaseHeadcrab );

public:
	CHeadcrab();

	void Precache( void );

	float	MaxYawSpeed( void );
	Activity NPC_TranslateActivity( Activity eNewActivity );

	void	BiteSound( void );
	void	PainSound( const CTakeDamageInfo &info );
	void	DeathSound( const CTakeDamageInfo &info );
	void	IdleSound( void );
	void	AlertSound( void );
	void	AttackSound( void );
	void	TelegraphSound( void );
};

//=========================================================
//=========================================================
// The spindly, fast headcrab
//=========================================================
//=========================================================
class CFastHeadcrab : public CBaseHeadcrab
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS( CFastHeadcrab, CBaseHeadcrab );

	CFastHeadcrab();

	void	Precache( void );
	bool	QuerySeeEntity(CBaseEntity *pSightEnt, bool bOnlyHateOrFearIfNPC = false);

	float	MaxYawSpeed( void );

	void	PrescheduleThink( void );
	void	RunTask( const Task_t *pTask );
	void	StartTask( const Task_t *pTask );

	int		SelectSchedule( void );
	int		TranslateSchedule( int scheduleType );

	int		m_iRunMode;
	float	m_flRealGroundSpeed;
	float	m_flSlowRunTime;
	float	m_flPauseTime;
	Vector	m_vecJumpVel;

	void	BiteSound( void );
	void	PainSound( const CTakeDamageInfo &info );
	void	DeathSound( const CTakeDamageInfo &info );
	void	IdleSound( void );
	void	AlertSound( void );
	void	AttackSound( void );

	enum SquadSlot_t
	{	
		SQUAD_SLOT_ENGAGE1 = LAST_SHARED_SQUADSLOT + 1,
		SQUAD_SLOT_ENGAGE2,
		SQUAD_SLOT_ENGAGE3,
		SQUAD_SLOT_ENGAGE4,
	};

	DEFINE_CUSTOM_AI;
};


//=========================================================
//=========================================================
// Treacherous black headcrab
//=========================================================
//=========================================================
class CBlackHeadcrab : public CBaseHeadcrab
{
	DECLARE_CLASS( CBlackHeadcrab, CBaseHeadcrab );

public:
	CBlackHeadcrab();

	void Eject( const QAngle &vecAngles, float flVelocityScale, CBaseEntity *pEnemy );
	void EjectTouch( CBaseEntity *pOther );

	//
	// CBaseHeadcrab implementation.
	//
	void TouchDamage( CBaseEntity *pOther );
	void BiteSound( void );
	void AttackSound( void );

	//
	// CAI_BaseNPC implementation.
	//
	virtual void PrescheduleThink( void );
	virtual void BuildScheduleTestBits( void );
	virtual int SelectSchedule( void );
	virtual int TranslateSchedule( int scheduleType );

	virtual Activity NPC_TranslateActivity( Activity eNewActivity );
	virtual void HandleAnimEvent( animevent_t *pEvent );
	virtual float MaxYawSpeed( void );

	virtual int	GetSoundInterests( void ) { return (BaseClass::GetSoundInterests() | SOUND_DANGER | SOUND_BULLET_IMPACT); }

	bool IsHeavyDamage( const CTakeDamageInfo &info );

	virtual void PainSound( const CTakeDamageInfo &info );
	virtual void DeathSound( const CTakeDamageInfo &info );
	virtual void IdleSound( void );
	virtual void AlertSound( void );
	virtual void ImpactSound( void );
	virtual void TelegraphSound( void );
#if HL2_EPISODIC
	virtual bool FInViewCone( CBaseEntity *pEntity );
#endif

	//
	// CBaseEntity implementation.
	//
	virtual void Spawn( void );
	virtual void Precache( void );

	DEFINE_CUSTOM_AI;
	DECLARE_DATADESC();

private:


	void JumpFlinch( const Vector *pvecAwayFromPos );
	void Panic( float flDuration );

	bool m_bPanicState;
	float m_flPanicStopTime;
	float m_flNextHopTime;		// Keeps us from hopping too often due to damage.
};


#endif // NPC_HEADCRAB_H
