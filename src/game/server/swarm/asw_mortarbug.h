#ifndef _INCLUDED_ASW_MORTARBUG_H
#define _INCLUDED_ASW_MORTARBUG_H
#ifdef _WIN32
#pragma once
#endif

#include "asw_alien.h"
#include "ai_blended_movement.h"
#include "util_shared.h"

class CASW_Door;
class CASW_Drone_Movement;

class CASW_Mortarbug : public CASW_Alien
{
public:

	DECLARE_CLASS( CASW_Mortarbug, CASW_Alien  );
	DECLARE_DATADESC();
	CASW_Mortarbug( void );
	virtual ~CASW_Mortarbug( void );

	void Spawn();
	void Precache();
	float GetIdealSpeed() const;
	float GetIdealAccel( ) const;
	float MaxYawSpeed( void );

	virtual int GetBaseHealth() override;

	Class_T		Classify( void ) { return (Class_T) CLASS_ASW_MORTAR_BUG; }
	virtual bool OverrideMoveFacing( const AILocalMoveGoal_t &move, float flInterval );
	virtual int SelectSchedule();
	virtual int SelectMortarbugCombatSchedule();
	virtual int SelectDeadSchedule();
	virtual void BuildScheduleTestBits();
	virtual int SelectFlinchSchedule_ASW();
	virtual bool ShouldGib( const CTakeDamageInfo &info );
	virtual int TranslateSchedule( int scheduleType );
	virtual void StartTask( const Task_t *pTask );
	virtual void RunTask( const Task_t *pTask );
	virtual void NPCThink();
	virtual void Event_Killed( const CTakeDamageInfo &info );
	virtual int OnTakeDamage_Alive( const CTakeDamageInfo &info );

	virtual float InnateRange1MinRange( void );
	virtual float InnateRange1MaxRange( void );
	virtual int RangeAttack1Conditions( float flDot, float flDist );
	virtual bool FCanCheckAttacks();
	virtual bool WeaponLOSCondition(const Vector &ownerPos, const Vector &targetPos, bool bSetConditions);
	virtual bool InnateWeaponLOSCondition( const Vector &ownerPos, const Vector &targetPos, bool bSetConditions );
	virtual void HandleAnimEvent( animevent_t *pEvent );
	CBaseEntity* SpawnProjectile();	
	virtual bool GetSpitVector( const Vector &vecStartPos, const Vector &vecTarget, Vector *vecOut );
	virtual bool SeenEnemyWithinTime( float flTime );

	virtual void StartTouch( CBaseEntity *pOther );
	
	// sounds
	virtual void PainSound( const CTakeDamageInfo &info );
	virtual void AlertSound();
	virtual void DeathSound( const CTakeDamageInfo &info );
	virtual void AttackSound();
	virtual void IdleSound();
	virtual bool ShouldPlayIdleSound();
	
	virtual bool IsHeavyDamage( const CTakeDamageInfo &info );

	bool CanFireMortar( const Vector &vecSrc, const Vector &vecDest, bool bDrawArc );
	int FindMortarNode( const Vector &vThreatPos, float flMinThreatDist, float flMaxThreatDist, float flBlockTime );
	Vector GetMortarFireSource( const Vector *vecStandingPos );

	float m_fLastFireTime;
	float m_fLastTouchHurtTime;
	float m_fGibTime;
	float m_flIdleDelay;
	Vector m_vecSaveSpitVelocity;
	static float s_fNextSpawnSoundTime;
	static float s_fNextPainSoundTime;

	enum
	{
		SCHED_ASW_MORTARBUG_SPIT = BaseClass::NEXT_SCHEDULE,
		SCHED_ESTABLISH_LINE_OF_MORTAR_FIRE,
		NEXT_SCHEDULE,
	};

	enum
	{
		TASK_MORTARBUG_SPIT = BaseClass::NEXT_TASK,
		TASK_GET_PATH_TO_MORTAR_ENEMY,
		NEXT_TASK,
	};

private:
	DEFINE_CUSTOM_AI;
};

#endif	//_INCLUDED_ASW_MORTARBUG_H
