#ifndef NPC_ZOMBINE_H
#define NPC_ZOMBINE_H
#pragma once

#include "npc_BaseZombie.h"
#include "player_pickup.h"

class CNPC_Zombine : public CNPC_BaseZombie, public CDefaultPlayerPickupVPhysics
{
	DECLARE_CLASS( CNPC_Zombine, CNPC_BaseZombie );

public:
	CNPC_Zombine();

	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

	void Spawn( void );
	void Precache( void );

	void SetZombieModel( void );
	int GetBaseHealth( void ) override;

	virtual void PrescheduleThink( void );
	virtual int SelectSchedule( void );
	virtual void BuildScheduleTestBits( void );

	virtual void HandleAnimEvent( animevent_t *pEvent );

	virtual const char *GetLegsModel( void );
	virtual const char *GetTorsoModel( void );
	virtual const char *GetHeadcrabClassname( void );
	virtual const char *GetHeadcrabModel( void );

	virtual void PainSound( const CTakeDamageInfo &info );
	virtual void DeathSound( const CTakeDamageInfo &info );
	virtual void AlertSound( void );
	virtual void IdleSound( void );
	virtual void AttackSound( void );
	virtual void AttackHitSound( void );
	virtual void AttackMissSound( void );
	virtual void FootstepSound( bool fRightFoot );
	virtual void FootscuffSound( bool fRightFoot );
	virtual void MoanSound( envelopePoint_t *pEnvelope, int iEnvelopeSize );

	virtual void Event_Killed( const CTakeDamageInfo &info );
	virtual void TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr );
	virtual void RunTask( const Task_t *pTask );
	virtual int  MeleeAttack1Conditions ( float flDot, float flDist );

	virtual bool ShouldBecomeTorso( const CTakeDamageInfo &info, float flDamageThreshold );

	virtual void OnScheduleChange ( void );
	virtual bool CanRunAScriptedNPCInteraction( bool bForced );

	void GatherGrenadeConditions( void );

	virtual Activity NPC_TranslateActivity( Activity baseAct );

	const char *GetMoanSound( int nSound );

	bool AllowedToSprint( void );
	void Sprint( bool bMadSprint = false );
	void StopSprint( void );

	void DropGrenade( Vector vDir );

	bool IsSprinting( void ) { return m_flSprintTime > gpGlobals->curtime;	}
	bool HasGrenade( void ) { return m_hGrenade != NULL; }

	int TranslateSchedule( int scheduleType );

	void InputStartSprint ( inputdata_t &inputdata );
	void InputPullGrenade ( inputdata_t &inputdata );

	virtual CBaseEntity *OnFailedPhysGunPickup ( Vector vPhysgunPos );

	//Called when we want to let go of a grenade and let the physcannon pick it up.
	void ReleaseGrenade( Vector vPhysgunPos );

	virtual bool HandleInteraction( int interactionType, void *data, CBaseCombatCharacter *sourceEnt );

	enum
	{
		COND_ZOMBINE_GRENADE = LAST_BASE_ZOMBIE_CONDITION,
	};

	enum
	{
		SCHED_ZOMBINE_PULL_GRENADE = BaseClass::NEXT_SCHEDULE,
		NEXT_SCHEDULE,
	};

public:
	DEFINE_CUSTOM_AI;

private:

	float	m_flSprintTime;
	float	m_flSprintRestTime;

	float	m_flSuperFastAttackTime;
	float   m_flGrenadePullTime;
	
	int		m_iGrenadeCount;
	string_t m_iszGrenadeClass;
	float m_flGrenadeFuseLength;
	int m_iGrenadeClusters;

	CNetworkHandle( CBaseEntity, m_hGrenade );

protected:
	static const char *pMoanSounds[];

};

#endif /* NPC_ZOMBINE_H */
