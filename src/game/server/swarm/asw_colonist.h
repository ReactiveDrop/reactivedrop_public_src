#ifndef	_INCLUDED_ASW_COLONIST_H
#define	_INCLUDED_ASW_COLONIST_H
#ifdef _WIN32
#pragma once
#endif
#include "asw_inhabitable_npc.h"
#include "asw_shareddefs.h"
#include "iasw_server_usable_entity.h"

class CASW_Alien;
class CASW_Marine;

class CASW_Colonist : public CASW_Inhabitable_NPC, public IASW_Server_Usable_Entity
{
	DECLARE_CLASS( CASW_Colonist, CASW_Inhabitable_NPC );
	DECLARE_SERVERCLASS();
public:
	CASW_Colonist();
	virtual ~CASW_Colonist();

	void			Precache();
	void			Spawn();
	void			SetHealthByDifficultyLevel();
	int				GetBaseHealth();
	
	Class_T 		Classify() { return (Class_T) CLASS_ASW_COLONIST; }
	Activity		NPC_TranslateActivity( Activity eNewActivity );
	int 			OnTakeDamage_Alive( const CTakeDamageInfo &info );
	void ASW_Ignite( float flFlameLifetime, float flSize, CBaseEntity *pAttacker, CBaseEntity *pDamagingWeapon );
	virtual void MeleeBleed(CTakeDamageInfo* info);
	bool			IsPlayerAlly( CBasePlayer *pPlayer = NULL );
	void PainSound( const CTakeDamageInfo &info );
	void DeathSound( const CTakeDamageInfo &info );
	void TaskFail( AI_TaskFailureCode_t code );

	void NPCThink();
	void ASWThinkEffects();
	float m_fLastASWThink;
	void HandleAnimEvent( animevent_t *pEvent );

	// healing
	void AddSlowHeal(int iHealAmount, CASW_Marine *pMedic);
	bool m_bSlowHeal;
	int m_iSlowHealAmount;
	float m_fNextSlowHealTick;

	// infestation
	bool Event_Gibbed( const CTakeDamageInfo &info );
	bool ShouldGib( const CTakeDamageInfo &info );
	void BecomeInfested(CASW_Alien* pAlien);
	void CureInfestation(CASW_Marine *pHealer, float fCureFraction);
	void ScriptBecomeInfested();
	void ScriptCureInfestation();
	bool IsInfested() { return m_bInfested; }
	bool IsHeavyDamage( const CTakeDamageInfo &info );
	virtual bool HasHumanGibs() { return true; }
	float m_fInfestedTime;	// how much time left on the infestation
	int m_iInfestCycle;
	bool m_bInfested;
	CHandle<CASW_Marine> m_hInfestationCurer;	// the last medic to cure us of some infestation
	virtual int SelectSchedule();
	virtual void RunTask( const Task_t *pTask );
	virtual void StartTask(const Task_t *pTask);
	virtual int SelectFlinchSchedule_ASW();

	bool					m_bNotifyNavFailBlocked;
	COutputEvent		m_OnNavFailBlocked;

	int selectedBy;
	bool isSelectedBy(CASW_Marine* marine);
	static void ASW_Colonist_GoTo(CASW_Player *pPlayer, CASW_Marine *pMarine, const Vector &targetPos, const Vector &traceDir);
	const Vector GetFollowPos();

	int m_Gender;

	void ScriptGiveWeapon( const char *pszName );
	bool ScriptDropWeapon();
	bool ScriptRemoveWeapon();
	void InputGiveWeapon( inputdata_t &inputdata );
	void OnRangeAttack1();
	Vector Weapon_ShootPosition();

	// IASW_Server_Usable_Entity implementation
	virtual CBaseEntity *GetEntity() { return this; }
	virtual bool IsUsable( CBaseEntity *pUser );
	virtual bool RequirementsMet( CBaseEntity *pUser ) { return true; }
	virtual void ActivateUseIcon( CASW_Inhabitable_NPC *pNPC, int nHoldType );
	virtual void NPCUsing( CASW_Inhabitable_NPC *pNPC, float deltatime ) {}
	virtual void NPCStartedUsing( CASW_Inhabitable_NPC *pNPC ) {}
	virtual void NPCStoppedUsing( CASW_Inhabitable_NPC *pNPC ) {}
	virtual bool NeedsLOSCheck() { return true; }
private:
	DECLARE_DATADESC();
	DECLARE_ENT_SCRIPTDESC();

	enum
	{
		SCHED_SA_FOLLOW_MOVE = BaseClass::NEXT_SCHEDULE,
		SCHED_SA_FOLLOW_WAIT,
		NEXT_SCHEDULE,
		
		TASK_SA_GET_PATH_TO_FOLLOW_TARGET= BaseClass::NEXT_TASK,
		TASK_SA_WAIT_FOR_FOLLOW_MOVEMENT,
		TASK_SA_FACE_FOLLOW_WAIT,
		NEXT_TASK,
	};

	DEFINE_CUSTOM_AI;
};

//---------------------
#endif	// _INCLUDED_ASW_COLONIST_H
