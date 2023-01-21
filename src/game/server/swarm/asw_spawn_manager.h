#ifndef _INCLUDED_ASW_SPAWN_MANAGER_H
#define _INCLUDED_ASW_SPAWN_MANAGER_H
#ifdef _WIN32
#pragma once
#endif

#include "iasw_spawnable_npc.h"
#include "asw_alien_classes.h"

class CAI_Network;
class CTriggerMultiple;
struct AI_Waypoint_t;
class CAI_Node;
class CASW_Alien;
class CASW_Spawn_Definition;
class CASW_Spawn_NPC;
class CASW_Spawner;

class CASW_Open_Area
{
public:
	CASW_Open_Area()
	{
		m_flArea = 0.0f;
		m_nTotalLinks = 0;
		m_vecOrigin = vec3_origin;
		m_pNode = NULL;
	}
	float m_flArea;
	int m_nTotalLinks;
	Vector m_vecOrigin;
	CAI_Node *m_pNode;
	CUtlVector<CAI_Node*> m_aAreaNodes;
};

// The spawn manager can spawn aliens and groups of aliens
class CASW_Spawn_Manager
{
public:
	CASW_Spawn_Manager();
	~CASW_Spawn_Manager();

	void LevelInitPreEntity();
	void LevelInitPostEntity();
	void Update();
	bool AddHorde( int iHordeSize, CASW_Spawn_Definition *pSpawn ); // creates a large pack of aliens somewhere near the marines
	void AddHordeWanderer( CASW_Spawn_Definition *pSpawn );         // creates a single alien somewhere near the currently spawning horde
	void AddWanderer( CASW_Spawn_Definition *pSpawn );              // creates a single alien somewhere near the marines
	bool PrespawnAliens( CASW_Spawn_Definition *pSpawn );           // creates aliens in separate open spaces
	bool SpawnAlienPack( CASW_Spawn_Definition *pSpawn );           // creates aliens in a cluster
	bool ScriptSpawnAlienAtRandomNode( const char *szAlienClass );

	// ported from riflemod
	void PrespawnAliens(int multiplier);
	// ported from riflemod
	void PrespawnAlienAtRandomNode(const char *szAlienClass, const int iNumAliens, const int iHull, const Vector &playerStartPos, const int iNumNodes);
	void PrespawnEntityAtRandomNode( const char *szEntityClass, const int iNumEntitiesToSpawn, const Vector &playerStartPos, const int iNumNodes );
	int SpawnAlienBatch( const char *szAlienClass, int iNumAliens, const Vector &vecPosition, const QAngle &angle, float flMarinesBeyondDist = 0 );
	int SpawnAlienBatch( CASW_Spawn_Definition *pSpawn, int iNumAliens, const Vector &vecPosition, const QAngle &angle, float flMarinesBeyondDist = 0 );
	//CBaseEntity *SpawnAlienAt( const char *szAlienClass, const Vector & vecPos, const QAngle & angle );
	//CBaseEntity *SpawnAlienAtWithOrders( const char *szAlienClass, const Vector & vecPos, const QAngle & angle, AlienOrder_t orders );
	CBaseEntity *SpawnAlienAt( CASW_Spawn_NPC *pNPC, const Vector & vecPos, const QAngle & angle, bool bAllowSpawner = false );
	bool SpawnAlienAt( CASW_Spawn_Definition *pSpawn, const Vector & vecPos, const QAngle & angle, bool bAllowSpawner = false );

	bool ValidSpawnPoint( const Vector &vecPosition, const Vector &vecMins, const Vector &vecMaxs, bool bCheckGround = true, float flMarineNearDistance = 0 );
	bool ValidHordeShiftedSpawnPoint( const Vector &vecOrigPos, const Vector &vecPosition, const Vector &vecMins, const Vector &vecMaxs, bool bCheckGround = true );
	bool LineBlockedByGeometry( const Vector &vecSrc, const Vector &vecEnd );
	CASW_Spawner *FindAvailableSpawner( CASW_Spawn_NPC *pNPC, const Vector & vecPos );
	
	bool GetAlienBounds( const char *szAlienClass, Vector &vecMins, Vector &vecMaxs );
	bool GetAlienBounds( string_t iszAlienClass, Vector &vecMins, Vector &vecMaxs );

	int GetHordeToSpawn() { return m_iHordeToSpawn + m_pHordeWandererDefinition.Count(); }

	void OnAlienWokeUp( CASW_Alien *pAlien );
	void OnAlienSleeping( CASW_Alien *pAlien );
	int GetAwakeAliens() { return m_nAwakeAliens; }
	int GetAwakeDrones() { return m_nAwakeDrones; }

	int GetNumAlienClasses();
	const ASW_Alien_Class_Entry* GetAlienClass( int i );

private:
	template <typename Alien> int SpawnAlienBatch(Alien szAlienClass, int iNumAliens, const Vector &vecPosition, const QAngle &angFacing, float flMarinesBeyondDist, const Vector & vecMins, const Vector & vecMaxs);
	void UpdateCandidateNodes();
	bool FindHordePos( bool bNorth, const CUtlVector<int> &candidateNodes, Vector  &vecHordePosition, QAngle &angHordeAngle );
	bool FindHordePosition();
	CAI_Network* GetNetwork();
	bool SpawnAlientAtRandomNode( CASW_Spawn_Definition *pSpawn );
	void FindEscapeTriggers();
	void DeleteRoute( AI_Waypoint_t *pWaypointList );

	// finds an area with good node connectivity.  Caller should take ownership of the CASW_Open_Area instance.
	CASW_Open_Area* FindNearbyOpenArea( const Vector &vecSearchOrigin, int nSearchHull );

	CountdownTimer m_batchInterval;
	Vector m_vecHordePosition;
	Vector m_vecHordePosition2; // reactivedrop: for rd_horde_two_sided, this will be a north position for 2nd horde if 1st horde comes from south
	QAngle m_angHordeAngle;
	QAngle m_angHordeAngle2;	// reactivedrop: for rd_horde_two_sided
	int m_iHordeToSpawn;
	CASW_Spawn_Definition *m_pHordeDefinition;
	CUtlVector<CASW_Spawn_Definition *> m_pHordeWandererDefinition;
	CUtlVector<CASW_Spawn_Definition *> m_pAliensToSpawn;

	int m_nAwakeAliens;
	int m_nAwakeDrones;

	bool m_bWarnedAboutMissingNodes;

	// maintaining a list of possible nodes to spawn aliens from
	CUtlVector<int> m_northCandidateNodes;
	CUtlVector<int> m_southCandidateNodes;
	CountdownTimer m_CandidateUpdateTimer;

	typedef CHandle<CTriggerMultiple> TriggerMultiple_t;
	CUtlVector<TriggerMultiple_t> m_EscapeTriggers;

	friend class CASW_Director_VScript;
};

extern const int g_nDroneClassEntry;
extern const int g_nDroneJumperClassEntry;

CASW_Spawn_Manager* ASWSpawnManager();

#endif // _INCLUDED_ASW_SPAWN_MANAGER_H
