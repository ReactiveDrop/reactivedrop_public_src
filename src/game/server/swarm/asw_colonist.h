#ifndef	_INCLUDED_ASW_COLONIST_H
#define	_INCLUDED_ASW_COLONIST_H
#ifdef _WIN32
#pragma once
#endif
#include "ai_playerally.h"
#include "asw_shareddefs.h"
#include "iasw_server_usable_entity.h"

class CASW_Marine;
class CASW_Alien;

class CASW_Colonist : public CAI_PlayerAlly, public IASW_Server_Usable_Entity
{
	DECLARE_CLASS( CASW_Colonist, CAI_PlayerAlly );
	DECLARE_SERVERCLASS();
public:
	CASW_Colonist();
	virtual ~CASW_Colonist();

	void			Precache();	
	void			Spawn();
	
	Class_T 		Classify() { return (Class_T) CLASS_ASW_COLONIST; }
	Activity		NPC_TranslateActivity( Activity eNewActivity );
	int 			OnTakeDamage_Alive( const CTakeDamageInfo &info );
	virtual void MeleeBleed(CTakeDamageInfo* info);
	bool			IsPlayerAlly( CBasePlayer *pPlayer = NULL );
	void DeathSound( const CTakeDamageInfo &info );
	void TaskFail( AI_TaskFailureCode_t code );

	void NPCThink();
	void ASWThinkEffects();
	float m_fLastASWThink;

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
	Activity GetFlinchActivity( bool bHeavyDamage, bool bGesture );

	bool					m_bNotifyNavFailBlocked;
	COutputEvent		m_OnNavFailBlocked;


	void CASW_Colonist::ScriptIgnite( float flFlameLifetime );
	void CASW_Colonist::ASW_Ignite( float flFlameLifetime, CBaseEntity *pAttacker, CBaseEntity *pDamagingWeapon );

	int selectedBy;
	bool isSelectedBy(CASW_Marine* marine);
	static void ASW_Colonist_GoTo(CASW_Player *pPlayer, const Vector &targetPos, const Vector &traceDir);
	void Extinguish();
	const Vector CASW_Colonist::GetFollowPos();

	bool isFemale;
	int m_Gender;

	void ScriptGiveWeapon( const char *pszName );
	bool ScriptDropWeapon();
	bool ScriptRemoveWeapon();
	void CASW_Colonist::InputGiveWeapon( inputdata_t &inputdata );
	void CASW_Colonist::OnRangeAttack1();
	Vector CASW_Colonist::Weapon_ShootPosition();


	// IASW_Server_Usable_Entity implementation
	virtual CBaseEntity* GetEntity() { return this; }
	virtual bool IsUsable(CBaseEntity *pUser);
	virtual bool RequirementsMet( CBaseEntity *pUser ) { return true; }
	virtual void ActivateUseIcon( CASW_Marine* pMarine, int nHoldType );
	virtual void MarineUsing(CASW_Marine* pMarine, float deltatime) {}
	virtual void MarineStartedUsing(CASW_Marine* pMarine) {}
	virtual void MarineStoppedUsing(CASW_Marine* pMarine) {}
	virtual bool NeedsLOSCheck() { return true; }
private:
	DECLARE_DATADESC();
	DECLARE_ENT_SCRIPTDESC();
#ifdef _XBOX
protected:
#endif

	enum {
		SCHED_SA_FOLLOW_MOVE = BaseClass::NEXT_SCHEDULE,
		SCHED_SA_FOLLOW_WAIT,
		NEXT_SCHEDULE,
		
		TASK_SA_GET_PATH_TO_FOLLOW_TARGET= BaseClass::NEXT_TASK,
		TASK_SA_WAIT_FOR_FOLLOW_MOVEMENT,
		TASK_SA_FACE_FOLLOW_WAIT,
		NEXT_TASK
	};


	DEFINE_CUSTOM_AI;
};

//---------------------
#endif	// _INCLUDED_ASW_COLONIST_H
