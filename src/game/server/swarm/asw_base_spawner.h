#ifndef _INCLUDED_ASW_BASE_SPAWNER_H
#define _INCLUDED_ASW_BASE_SPAWNER_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "iasw_spawnable_npc.h"
#include "asw_shareddefs.h"

class CASW_Alien;
class CASW_Buzzer;
class IASW_Spawnable_NPC;
class CASW_Spawn_NPC;

//===============================================
//  Base alien spawner
//===============================================
class CASW_Base_Spawner : public CServerOnlyPointEntity
{
public:
	DECLARE_CLASS( CASW_Base_Spawner, CServerOnlyPointEntity );
	DECLARE_DATADESC();

	CASW_Base_Spawner();
	virtual ~CASW_Base_Spawner();

	virtual void	Spawn();
	virtual void	Precache();

	virtual IASW_Spawnable_NPC*	SpawnAlien( const char *szAlienClassName, const Vector &vecHullMins, const Vector &vecHullMaxs, CASW_Spawn_NPC *pDirectorNPC = NULL );
	void					RemoveObstructingProps( CBaseEntity *pChild );	
	virtual bool			CanSpawn( const Vector &vecHullMins, const Vector &vecHullMaxs, CASW_Spawn_NPC *pDirectorNPC = NULL );
	CBaseEntity*			GetOrderTarget();
	bool					MoveSpawnableTo(IASW_Spawnable_NPC* pAlien, CBaseEntity* pGoalEnt, bool bIgnoreMarines);

	virtual void			AlienKilled( CBaseEntity *pVictim ) { }

	CBaseEntity*			GetAlienOrderTarget();

	bool					IsValidOnThisSkillLevel();
	bool					HasRecentlySpawned( float flRecent = 1.0f );
	bool					IsEnabled() { return m_bEnabled; }

	// inputs
	virtual void InputEnable( inputdata_t &inputdata );
	virtual void InputDisable( inputdata_t &inputdata );
	virtual void InputToggleEnabled( inputdata_t &inputdata );

protected:
	// spawn if a marine is inside this radius or not?
	bool	m_bSpawnIfMarinesAreNear;
	float	m_flNearDistance;
	
	// burrowing
	string_t m_UnburrowIdleActivity;
	string_t m_UnburrowActivity;
	bool m_bStartBurrowed;

	// skill level
	int m_iMinSkillLevel;
	int m_iMaxSkillLevel;

	// give the alien this order when it spawns	
	AlienOrder_t m_AlienOrders;
	string_t m_AlienOrderTargetName;		// name of our moveto target
	string_t m_AlienName;	// name given to spawned aliens
	EHANDLE m_hAlienOrderTarget;

	// outputs
	COutputEvent m_OnSpawned;
	
	bool m_bLongRangeNPC;	// NPC can see twice as far?
	bool m_bCheckSpawnIsClear;
	int m_iMoveAsideCount;
	float m_flLastSpawnTime;

	bool m_bEnabled;

	// reactivedrop: adding more options for spawner to make harder aliens
	// Sp suffix is used to distinguish from equal vars in asw_alien.h
	// Sp stands for spawner
	bool m_bFlammableSp;
	bool m_bTeslableSp;
	bool m_bFreezableSp;
	bool m_bFlinchableSp;
	bool m_bGrenadeReflectorSp;
	int m_iHealthBonusSp;
	float m_fSizeScaleSp;
	float m_fSpeedScaleSp;

	string_t m_iszAlienVScripts;
	string_t m_iszAlienScriptThinkFunction;

	// BenLubar(director-spawner-improvements)
	Vector m_vecCurrentSpawnPosition;
	QAngle m_angCurrentSpawnAngles;
	Vector m_vecMinOffset;
	Vector m_vecMaxOffset;
	float m_flYawOffset;
	//
};

#define ASW_SF_ALWAYS_INFINITE 1
#define ASW_SF_NO_UBER 2
#define ASW_SF_NEVER_SLEEP 4
#define ASW_SF_NO_CARNAGE 8

#endif /* _INCLUDED_ASW_BASE_SPAWNER_H */