#ifndef _INCLUDED_ASW_HARVESTER_H
#define _INCLUDED_ASW_HARVESTER_H
#ifdef _WIN32
#pragma once
#endif

#include "asw_alien.h"
#include "ai_blended_movement.h"
#include "util_shared.h"
#include "ai_speech.h"

class CASW_Door;
class CASW_Drone_Movement;
class CASW_Parasite;

class CASW_Harvester : public CASW_Alien
{
public:

	DECLARE_CLASS( CASW_Harvester, CASW_Alien );
	DECLARE_DATADESC();
	CASW_Harvester( void );
	virtual ~CASW_Harvester( void );

	void Spawn();
	void Precache();
	float GetIdealSpeed() const;
	float GetIdealAccel( ) const;
	float MaxYawSpeed( void );

	virtual int GetBaseHealth() override;
	//virtual float GetSequenceGroundSpeed( int iSequence );	
	//virtual void NPCThink();
	Class_T		Classify( void ) { return (Class_T) CLASS_ASW_HARVESTER; }
	//void HandleAnimEvent( animevent_t *pEvent );
	//bool CorpseGib( const CTakeDamageInfo &info );
	
	//bool ShouldGib( const CTakeDamageInfo &info );
	virtual bool OverrideMoveFacing( const AILocalMoveGoal_t &move, float flInterval );
	virtual int SelectSchedule();
	virtual int SelectHarvesterCombatSchedule();
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
	virtual void HandleAnimEvent( animevent_t *pEvent );
	CAI_BaseNPC* SpawnAlien();	
	virtual void ChildAlienKilled(CASW_Alien* pAlien);

	virtual void StartTouch( CBaseEntity *pOther );
	
	// sounds
	virtual void PainSound( const CTakeDamageInfo &info );
	virtual void AlertSound();
	virtual void DeathSound( const CTakeDamageInfo &info );
	virtual void AttackSound();
	virtual void IdleSound();
	virtual bool ShouldPlayIdleSound();
	
	virtual bool IsHeavyDamage( const CTakeDamageInfo &info );
	virtual bool CanBePushedAway();
	virtual void SetInhabitedAlienAttackSchedule();

	float m_fLastLayTime;		// last time we laid a critter
	int m_iCrittersAlive;		// how many spawned critters we currently have
	float m_fLastTouchHurtTime;
	float m_fGibTime;
	float m_flIdleDelay;
	static float s_fNextSpawnSoundTime;
	static float s_fNextPainSoundTime;

	enum
	{
		SCHED_ASW_HARVESTER_LAY_CRITTER = BaseClass::NEXT_SCHEDULE,
		SCHED_ASW_HARVESTER_LAY_CRITTER_INHABITED,
		NEXT_SCHEDULE,
	};

	enum
	{
		TASK_LAY_CRITTER = BaseClass::NEXT_TASK,
		NEXT_TASK,
	};

private:
	DEFINE_CUSTOM_AI;
};


#endif	//_INCLUDED_ASW_HARVESTER_H
