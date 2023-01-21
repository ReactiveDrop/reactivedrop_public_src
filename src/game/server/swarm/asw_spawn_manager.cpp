#include "cbase.h"
#include "asw_spawn_manager.h"
#include "asw_marine.h"
#include "asw_marine_resource.h"
#include "asw_game_resource.h"
#include "iasw_spawnable_npc.h"
#include "asw_player.h"
#include "ai_network.h"
#include "ai_waypoint.h"
#include "ai_node.h"
#include "asw_director.h"
#include "asw_util_shared.h"
#include "asw_path_utils.h"
#include "asw_trace_filter_doors.h"
#include "asw_objective_escape.h"
#include "triggers.h"
#include "datacache/imdlcache.h"
#include "ai_link.h"
#include "asw_alien.h"
#include "asw_buzzer.h"
#include "asw_door.h"
#include "asw_spawn_selection.h"
#include "rd_director_triggers.h"
#include "asw_spawner.h"
#include "asw_alien_goo_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CASW_Spawn_Manager g_Spawn_Manager;
CASW_Spawn_Manager* ASWSpawnManager() { return &g_Spawn_Manager; }

#define CANDIDATE_ALIEN_HULL HULL_MEDIUMBIG		// TODO: have this use the hull of the alien type we're spawning a horde of?
#define MARINE_NEAR_DISTANCE 740.0f

extern ConVar asw_director_debug;
ConVar asw_horde_min_distance("asw_horde_min_distance", "800", FCVAR_CHEAT, "Minimum distance away from the marines the horde can spawn" );
ConVar asw_horde_max_distance("asw_horde_max_distance", "1500", FCVAR_CHEAT, "Maximum distance away from the marines the horde can spawn" );
ConVar asw_max_alien_batch("asw_max_alien_batch", "10", FCVAR_CHEAT, "Max number of aliens spawned in a horde batch" );
ConVar asw_batch_interval("asw_batch_interval", "5", FCVAR_CHEAT, "Time between successive batches spawning in the same spot");
ConVar asw_candidate_interval("asw_candidate_interval", "1.0", FCVAR_CHEAT, "Interval between updating candidate spawning nodes");

ConVar rd_prespawn_antlionguard( "rd_prespawn_antlionguard", "0", FCVAR_CHEAT, "If 1 and Onslaught is enabled an npc_antlionguard will be prespawned somewhere on map");
ConVar rd_horde_two_sided( "rd_horde_two_sided", "0", FCVAR_CHEAT, "If 1 and Onslaught is enabled a 2nd horde will come from opposite side, e.g. north and south" );
ConVar rd_horde_retry_on_fail( "rd_horde_retry_on_fail", "1", FCVAR_CHEAT, "When set to 1 will retry to spawn horde from opposite direction if previous direction spawn failed." );
ConVar rd_horde_ignore_north_door( "rd_horde_ignore_north_door", "0", FCVAR_CHEAT, "If 1 hordes can spawn behind sealed and locked doors to the north from marines. Excluding indestructible doors." );
ConVar rd_director_spawner_range( "rd_director_spawner_range", "600", FCVAR_CHEAT, "Radius around expected spawn point that the director can look for spawners");
ConVar rd_director_spawner_bias( "rd_director_spawner_bias", "0.9", FCVAR_CHEAT, "0 (search from the node) to 1 (search from the nearest marine)", true, 0, true, 1);
ConVar rd_horde_from_exit( "rd_horde_from_exit", "1", FCVAR_CHEAT, "If 0 hordes and wanderers cannot spawn in map exit zone. 1 by default" );

CASW_Spawn_Manager::CASW_Spawn_Manager()
{
	m_nAwakeAliens = 0;
	m_nAwakeDrones = 0;
}

CASW_Spawn_Manager::~CASW_Spawn_Manager()
{

}

int CASW_Spawn_Manager::GetNumAlienClasses()
{
	return NELEMS( g_Aliens );
}

const ASW_Alien_Class_Entry* CASW_Spawn_Manager::GetAlienClass( int i )
{
	Assert( i >= 0 && i < NELEMS( g_Aliens ) );
	return &g_Aliens[ i ];
}

void CASW_Spawn_Manager::LevelInitPreEntity()
{
	m_nAwakeAliens = 0;
	m_nAwakeDrones = 0;
}

void CASW_Spawn_Manager::LevelInitPostEntity()
{
	m_vecHordePosition = vec3_origin;
	m_vecHordePosition2 = vec3_origin;
	m_angHordeAngle = vec3_angle;
	m_angHordeAngle2 = vec3_angle;
	m_batchInterval.Invalidate();
	m_CandidateUpdateTimer.Invalidate();
	m_iHordeToSpawn = 0;
	m_pHordeDefinition = NULL;
	m_pHordeWandererDefinition.Purge();
	m_pAliensToSpawn.Purge();

	m_northCandidateNodes.Purge();
	m_southCandidateNodes.Purge();
	m_bWarnedAboutMissingNodes = false;

	FindEscapeTriggers();
}

void CASW_Spawn_Manager::OnAlienWokeUp( CASW_Alien *pAlien )
{
	m_nAwakeAliens++;
	if ( pAlien && pAlien->Classify() == CLASS_ASW_DRONE )
	{
		m_nAwakeDrones++;
	}
}

void CASW_Spawn_Manager::OnAlienSleeping( CASW_Alien *pAlien )
{
	m_nAwakeAliens--;
	if ( pAlien && pAlien->Classify() == CLASS_ASW_DRONE )
	{
		m_nAwakeDrones--;
	}
}

// finds all trigger_multiples linked to asw_objective_escape entities
void CASW_Spawn_Manager::FindEscapeTriggers()
{
	m_EscapeTriggers.Purge();

	// go through all escape objectives
	CBaseEntity* pEntity = NULL;
	while ( (pEntity = gEntList.FindEntityByClassname( pEntity, "asw_objective_escape" )) != NULL )
	{
		CASW_Objective_Escape* pObjective = dynamic_cast<CASW_Objective_Escape*>(pEntity);
		if ( !pObjective )
			continue;

		const char *pszEscapeTargetName = STRING( pObjective->GetEntityName() );

		CBaseEntity* pOtherEntity = NULL;
		while ( (pOtherEntity = gEntList.FindEntityByClassname( pOtherEntity, "trigger_multiple" )) != NULL )
		{
			CTriggerMultiple *pTrigger = dynamic_cast<CTriggerMultiple*>( pOtherEntity );
			if ( !pTrigger )
				continue;

			bool bAdded = false;
			CBaseEntityOutput *pOutput = pTrigger->FindNamedOutput( "OnTrigger" );
			if ( pOutput )
			{
				CEventAction *pAction = pOutput->GetFirstAction();
				while ( pAction )
				{
					if ( !Q_stricmp( STRING( pAction->m_iTarget ), pszEscapeTargetName ) )
					{
						bAdded = true;
						m_EscapeTriggers.AddToTail( pTrigger );
						break;
					}
					pAction = pAction->m_pNext;
				}
			}

			if ( !bAdded )
			{
				pOutput = pTrigger->FindNamedOutput( "OnStartTouch" );
				if ( pOutput )
				{
					CEventAction *pAction = pOutput->GetFirstAction();
					while ( pAction )
					{
						if ( !Q_stricmp( STRING( pAction->m_iTarget ), pszEscapeTargetName ) )
						{
							bAdded = true;
							m_EscapeTriggers.AddToTail( pTrigger );
							break;
						}
						pAction = pAction->m_pNext;
					}
				}
			}
			
		}
	}
	if ( asw_director_debug.GetBool() )
	{
		Msg( "Spawn manager found %d escape triggers\n", m_EscapeTriggers.Count() );
	}
}


void CASW_Spawn_Manager::Update()
{
	if ( GetHordeToSpawn() > 0 )
	{
		if ( m_vecHordePosition != vec3_origin && ( !m_batchInterval.HasStarted() || m_batchInterval.IsElapsed() ) )
		{
			int iToSpawn = MIN( m_iHordeToSpawn, asw_max_alien_batch.GetInt() );
			int iSpawned = iToSpawn > 0 ? SpawnAlienBatch( m_pHordeDefinition, iToSpawn, m_vecHordePosition, m_angHordeAngle, 0 ) : 0;
			if ( rd_horde_two_sided.GetBool() && iToSpawn > 0 && m_vecHordePosition2 != vec3_origin )
			{
				SpawnAlienBatch( m_pHordeDefinition, iToSpawn, m_vecHordePosition2, m_angHordeAngle2, 0 );
			}
			m_iHordeToSpawn -= iSpawned;
			if ( m_pHordeWandererDefinition.Count() > 0 )
			{
				if ( SpawnAlienAt( m_pHordeWandererDefinition[0], m_vecHordePosition, m_angHordeAngle, true ) )
				{
					iSpawned++;
					m_pHordeWandererDefinition.Remove( 0 );
				}
			}
			if ( m_iHordeToSpawn <= 0 && m_pHordeWandererDefinition.Count() == 0 )
			{
				m_iHordeToSpawn = 0;
				m_pHordeDefinition = NULL;
				m_pHordeWandererDefinition.Purge();
				ASWDirector()->OnHordeFinishedSpawning();
				m_vecHordePosition = vec3_origin;
			}
			else if ( iSpawned == 0 )			// if we failed to spawn any aliens, then try to find a new horde location
			{
				if ( asw_director_debug.GetBool() )
				{
					Msg( "Horde failed to spawn any aliens, trying new horde position.\n" );
				}
				if ( !FindHordePosition() )		// if we failed to find a new location, just abort this horde
				{
					m_iHordeToSpawn = 0;
					m_pHordeDefinition = NULL;
					m_pHordeWandererDefinition.Purge();
					ASWDirector()->OnHordeFinishedSpawning();
					m_vecHordePosition = vec3_origin;
				}
			}
			m_batchInterval.Start( asw_batch_interval.GetFloat() );
		}
		else if ( m_vecHordePosition == vec3_origin )
		{
			Warning( "Warning: Had horde to spawn but no position, clearing.\n" );
			m_iHordeToSpawn = 0;
			m_pHordeDefinition = NULL;
			m_pHordeWandererDefinition.Purge();
			ASWDirector()->OnHordeFinishedSpawning();
		}
	}

	if ( asw_director_debug.GetBool() )
	{
		engine->Con_NPrintf( 14, "SM: Batch interval: %f pos = %f %f %f\n", m_batchInterval.HasStarted() ? m_batchInterval.GetRemainingTime() : -1, VectorExpand( m_vecHordePosition ) );		
	}

	if ( m_pAliensToSpawn.Count() > 0 )
	{
		CASW_Spawn_Definition *pSpawn = m_pAliensToSpawn[0];
		//DevMsg("Spawning at %f\n", gpGlobals->curtime);
		if (!SpawnAlientAtRandomNode( pSpawn ))
		{
			char szSpawn[256];
			Q_snprintf( szSpawn, sizeof( szSpawn ), "NPC { AlienClass %s }", "asw_drone" );
			CASW_Spawn_Definition spawn( KeyValues::AutoDeleteInline( KeyValues::FromString( "WANDERER", szSpawn ) ) );

			SpawnAlientAtRandomNode( &spawn );
		}
		m_pAliensToSpawn.Remove( 0 );
	}
}

void CASW_Spawn_Manager::AddWanderer( CASW_Spawn_Definition *pSpawn )
{
	// don't stock up more than 10 wanderers at once
	if ( m_pAliensToSpawn.Count() > 10 )
		return;

	m_pAliensToSpawn.AddToTail( pSpawn );
}

void CASW_Spawn_Manager::AddHordeWanderer( CASW_Spawn_Definition *pSpawn )
{
	// don't stock up more than 10 wanderers at once
	if ( m_pHordeWandererDefinition.Count() > 10 )
		return;

	m_pHordeWandererDefinition.AddToTail( pSpawn );
}

bool CASW_Spawn_Manager::SpawnAlientAtRandomNode( CASW_Spawn_Definition *pSpawn )
{
	if ( pSpawn->m_NPCs.Count() < 1 )
		return false;

	UpdateCandidateNodes();

	// decide if the alien is going to come from behind or in front
	bool bNorth = RandomFloat() < 0.7f;
	if ( m_northCandidateNodes.Count() <= 0 )
	{
		bNorth = false;
	}
	else if ( m_southCandidateNodes.Count() <= 0 )
	{
		bNorth = true;
	}

	CUtlVector<int> *candidateNodes = bNorth ? &m_northCandidateNodes : &m_southCandidateNodes;

	if ( candidateNodes->Count() <= 0 )
		return false;

	int iMaxTries = 2;
	for ( int i = 0 ; i < iMaxTries ; i++ )
	{
		// try to spawn from another direction
		if ( 1 == i )
		{
			candidateNodes = bNorth ? &m_southCandidateNodes : &m_northCandidateNodes;
			if ( candidateNodes->Count() <= 0 )
				return false;
			bNorth = !bNorth;
		}

		int iChosen = RandomInt( 0, candidateNodes->Count() - 1);
		CAI_Node *pNode = GetNetwork()->GetNode( (*candidateNodes)[iChosen] );
		if ( !pNode )
			continue;

		float flDistance = 0;
		const int WANDERER_HULL = pSpawn->m_NPCs[0]->m_pAlienClass->m_nHullType;
		CASW_Marine *pMarine = UTIL_ASW_NearestMarine( pNode->GetPosition( WANDERER_HULL ), flDistance );
		if ( !pMarine )
			return false;

		// check if there's a route from this node to the marine(s)
		AI_Waypoint_t *pRoute = ASWPathUtils()->BuildRouteForHull( pNode->GetPosition( WANDERER_HULL ), pMarine->GetAbsOrigin(), NULL, 100, WANDERER_HULL );
		if ( !pRoute )
		{
			if ( asw_director_debug.GetBool() )
			{
				NDebugOverlay::Cross3D( pNode->GetOrigin(), 10.0f, 255, 128, 0, true, 20.0f );
			}
			continue;
		}

		// reactivedrop: preventing wanderers from spawning behind indestructible doors
		CASW_Door *pDoor = UTIL_ASW_DoorBlockingRoute(pRoute, true);
		if ( ( bNorth && pDoor ) || ( pDoor && pDoor->GetDoorType() == 2 ) ) // 2 - indestructible door
		{
			DeleteRoute( pRoute );
			continue;
		}

		// reactivedrop: preventing wanderers from spawning behind closed airlocks
		if ( UTIL_ASW_BrushBlockingRoute( pRoute, MASK_NPCSOLID_BRUSHONLY, ASW_COLLISION_GROUP_ALIEN ) )
		{
			DeleteRoute( pRoute );
			continue;
		}

		// reactivedrop: preventing wanderers from spawning behind closed airlocks x2
		if ( UTIL_ASW_AirlockBlockingRoute( pRoute, CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_MONSTER, ASW_COLLISION_GROUP_ALIEN ) )
		{
			DeleteRoute( pRoute );
			continue;
		}

		bool bAny = false;
		bool bTried = false;
		// BenLubar: TODO: spawn aliens in grid, not to be inside each other
		FOR_EACH_VEC( pSpawn->m_NPCs, j )
		{
			CASW_Spawn_NPC *pNPC = pSpawn->m_NPCs[j];
			if ( !pNPC->m_Requirement.CanSpawn() || ( pNPC->m_flSpawnChance < 1 && random->RandomFloat() > pNPC->m_flSpawnChance ) )
			{
				continue;
			}
			bTried = true;

			// additional route check 
			const int CURRENT_WANDERER_HULL = pNPC->m_pAlienClass->m_nHullType;
			if ( WANDERER_HULL != CURRENT_WANDERER_HULL )
			{
				AI_Waypoint_t *pRoute2 = ASWPathUtils()->BuildRouteForHull( pNode->GetPosition( CURRENT_WANDERER_HULL ), pMarine->GetAbsOrigin(), NULL, 100, CURRENT_WANDERER_HULL );
				if ( !pRoute2 )
				{
					if ( asw_director_debug.GetBool() )
					{
						NDebugOverlay::Cross3D( pNode->GetOrigin(), 10.0f, 255, 128, 0, true, 20.0f );
					}
					continue;
				}
				DeleteRoute( pRoute2 );
			}

			const Vector & vecMins = NAI_Hull::Mins( pNPC->m_pAlienClass->m_nHullType );
			const Vector & vecMaxs = NAI_Hull::Maxs( pNPC->m_pAlienClass->m_nHullType );

			Vector vecSpawnPos = pNode->GetPosition( pNPC->m_pAlienClass->m_nHullType ) + Vector( 0, 0, 23 );
			if ( ValidSpawnPoint( vecSpawnPos, vecMins, vecMaxs, true, MARINE_NEAR_DISTANCE ) )
			{
				if ( SpawnAlienAt( pNPC, vecSpawnPos, QAngle( 0, RandomFloat( 0.0f, 360.0f ), 0 ), true) )
				{
					if ( asw_director_debug.GetBool() )
					{
						NDebugOverlay::Cross3D( vecSpawnPos, 25.0f, 255, 255, 255, true, 20.0f );
						float flDist;
						CASW_Marine *pDebugMarine = UTIL_ASW_NearestMarine( vecSpawnPos, flDist );
						if ( pDebugMarine )
						{
							NDebugOverlay::Line( pDebugMarine->GetAbsOrigin(), vecSpawnPos, 64, 64, 64, true, 60.0f );
						}
					}
					bAny = true;
				}
			}
			else
			{
				if ( asw_director_debug.GetBool() )
				{
					NDebugOverlay::Cross3D( vecSpawnPos, 25.0f, 255, 0, 0, true, 20.0f );
				}
			}
		}

		DeleteRoute( pRoute );

		if ( bAny || !bTried )
		{
			return true;
		}
	}
	return false;
}

bool CASW_Spawn_Manager::ScriptSpawnAlienAtRandomNode( const char *szAlienClass )
{
	char szSpawn[256];
	Q_snprintf( szSpawn, sizeof(szSpawn), "NPC { AlienClass %s }", szAlienClass );
	CASW_Spawn_Definition spawn( KeyValues::AutoDeleteInline( KeyValues::FromString( "WANDERER", szSpawn ) ) );

	return SpawnAlientAtRandomNode( &spawn );
}

bool CASW_Spawn_Manager::AddHorde( int iHordeSize, CASW_Spawn_Definition *pSpawn )
{
	m_iHordeToSpawn = iHordeSize;
	m_pHordeDefinition = pSpawn;

	if ( m_vecHordePosition == vec3_origin )
	{
		if ( !FindHordePosition() )
		{
			if ( !m_bWarnedAboutMissingNodes )
			{
				Warning( "Error: Failed to find horde position\n" );
			}

			return false;
		}
		else
		{
			if ( asw_director_debug.GetBool() )
			{
				NDebugOverlay::Cross3D( m_vecHordePosition, 50.0f, 255, 128, 0, true, 60.0f );
				float flDist;
				CASW_Marine *pMarine = UTIL_ASW_NearestMarine( m_vecHordePosition, flDist );
				if ( pMarine )
				{
					NDebugOverlay::Line( pMarine->GetAbsOrigin(), m_vecHordePosition, 255, 128, 0, true, 60.0f );
				}
			}
		}
	}
	return true;
}

CAI_Network* CASW_Spawn_Manager::GetNetwork()
{
	return g_pBigAINet;
}

void CASW_Spawn_Manager::UpdateCandidateNodes()
{
	// don't update too frequently
	if ( m_CandidateUpdateTimer.HasStarted() && !m_CandidateUpdateTimer.IsElapsed() )
		return;

	m_CandidateUpdateTimer.Start( asw_candidate_interval.GetFloat() );

	if ( !GetNetwork() || !GetNetwork()->NumNodes() )
	{
		m_vecHordePosition = vec3_origin;
		if ( !m_bWarnedAboutMissingNodes )
		{
			m_bWarnedAboutMissingNodes = true;
			Warning( "Error: Can't spawn hordes as this map has no node network\n" );
		}
		return;
	}

	CASW_Game_Resource *pGameResource = ASWGameResource();
	if ( !pGameResource )
		return;

	Vector vecSouthMarine = vec3_origin;
	Vector vecNorthMarine = vec3_origin;
	for ( int i=0;i<pGameResource->GetMaxMarineResources();i++ )
	{
		CASW_Marine_Resource *pMR = pGameResource->GetMarineResource(i);
		if ( !pMR )
			continue;

		CASW_Marine *pMarine = pMR->GetMarineEntity();
		if ( !pMarine || pMarine->GetHealth() <= 0 )
			continue;

		if ( vecSouthMarine == vec3_origin || vecSouthMarine.y > pMarine->GetAbsOrigin().y )
		{
			vecSouthMarine = pMarine->GetAbsOrigin();
		}
		if ( vecNorthMarine == vec3_origin || vecNorthMarine.y < pMarine->GetAbsOrigin().y )
		{
			vecNorthMarine = pMarine->GetAbsOrigin();
		}
	}
	if ( vecSouthMarine == vec3_origin || vecNorthMarine == vec3_origin )		// no live marines
		return;
	
	int iNumNodes = GetNetwork()->NumNodes();
	m_northCandidateNodes.Purge();
	m_southCandidateNodes.Purge();
	for ( int i=0 ; i<iNumNodes; i++ )
	{
		CAI_Node *pNode = GetNetwork()->GetNode( i );
		if ( !pNode || pNode->GetType() != NODE_GROUND )
			continue;

		Vector vecPos = pNode->GetPosition( CANDIDATE_ALIEN_HULL );
		
		// find the nearest marine to this node
		float flDistance = 0;
		CASW_Marine *pMarine = UTIL_ASW_NearestMarine( vecPos, flDistance );
		if ( !pMarine )
			return;

		if ( flDistance > asw_horde_max_distance.GetFloat() || flDistance < asw_horde_min_distance.GetFloat() )
			continue;

		// check node isn't in an exit trigger
		if ( !rd_horde_from_exit.GetBool() )
		{
			bool bInsideEscapeArea = false;
			for ( int d = 0; d < m_EscapeTriggers.Count(); d++ )
			{
				if ( !m_EscapeTriggers[d]->m_bDisabled && m_EscapeTriggers[d]->CollisionProp()->IsPointInBounds( vecPos ) )
				{
					bInsideEscapeArea = true;
					break;
				}
			}
			if ( bInsideEscapeArea )
				continue;
		}

		if ( vecPos.y >= vecSouthMarine.y )
		{
			if ( asw_director_debug.GetInt() == 3 )
			{
				NDebugOverlay::Box( vecPos, -Vector( 5, 5, 5 ), Vector( 5, 5, 5 ), 32, 32, 128, 10, 60.0f );
			}
			m_northCandidateNodes.AddToTail( i );
		}
		if ( vecPos.y <= vecNorthMarine.y )
		{
			m_southCandidateNodes.AddToTail( i );
			if ( asw_director_debug.GetInt() == 3 )
			{
				NDebugOverlay::Box( vecPos, -Vector( 5, 5, 5 ), Vector( 5, 5, 5 ), 128, 32, 32, 10, 60.0f );
			}
		}
	}
}

bool CASW_Spawn_Manager::FindHordePos( bool bNorth, const CUtlVector<int> &candidateNodes, Vector  &vecHordePosition, QAngle &angHordeAngle )
{
	if ( candidateNodes.Count() <= 0 )
	{
		if ( asw_director_debug.GetBool() )
		{
			DevMsg( "  Failed to find horde pos as there are no candidate nodes\n" );
		}
		return false;
	}

	int iMaxTries = 3;
	for ( int i = 0; i<iMaxTries; i++ )
	{
		int iChosen = RandomInt( 0, candidateNodes.Count() - 1 );
		CAI_Node *pNode = GetNetwork()->GetNode( candidateNodes[iChosen] );
		if ( !pNode )
			continue;

		float flDistance = 0;
		CASW_Marine *pMarine = UTIL_ASW_NearestMarine( pNode->GetPosition( CANDIDATE_ALIEN_HULL ), flDistance );
		if ( !pMarine )
		{
			if ( asw_director_debug.GetBool() )
			{
				DevMsg( "  Failed to find horde pos as there is no nearest marine\n" );
			}
			return false;
		}

		// check if there's a route from this node to the marine(s)
		AI_Waypoint_t *pRoute = ASWPathUtils()->BuildRoute( pNode->GetPosition( CANDIDATE_ALIEN_HULL ), pMarine->GetAbsOrigin(), NULL, 100 );
		if ( !pRoute )
		{
			if ( asw_director_debug.GetInt() >= 2 )
			{
				DevMsg( "  Discarding horde node %d as there's no route.\n", iChosen );
			}
			continue;
		}

		// reactivedrop: preventing hordes from spawning behind indestructible doors
		CASW_Door *pDoor = UTIL_ASW_DoorBlockingRoute( pRoute, true );
		if ( ( bNorth && pDoor && !rd_horde_ignore_north_door.GetBool() ) || ( pDoor && pDoor->GetDoorType() == 2 ) ) // 2 - indestructible door
		{
			if ( asw_director_debug.GetInt() >= 2 )
			{
				DevMsg( "  Discarding horde node %d as there's a door in the way.\n", iChosen );
			}
			DeleteRoute( pRoute );
			continue;
		}

		// reactivedrop: preventing hordes from spawning behind closed airlocks
		if ( UTIL_ASW_BrushBlockingRoute( pRoute, MASK_NPCSOLID_BRUSHONLY, ASW_COLLISION_GROUP_ALIEN ) )
		{
			if ( asw_director_debug.GetInt() >= 2 )
			{
				DevMsg( "  Discarding horde node %d as there's a brush in the way.\n", iChosen );
			}
			DeleteRoute( pRoute );
			continue;
		}

		// reactivedrop: since airlock has collision now and brushes were removed from certain maps
		if ( UTIL_ASW_AirlockBlockingRoute( pRoute, CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_MONSTER, ASW_COLLISION_GROUP_ALIEN ) )
		{
			if ( asw_director_debug.GetInt() >= 2 )
			{
				DevMsg( "  Discarding horde node %d as there's an airlock in the way.\n", iChosen );
			}
			DeleteRoute( pRoute );
			continue;
		}

		vecHordePosition = pNode->GetPosition( CANDIDATE_ALIEN_HULL ) + Vector( 0, 0, 32 );

		// spawn facing the nearest marine
		Vector vecDir = pMarine->GetAbsOrigin() - vecHordePosition;
		vecDir.z = 0;
		vecDir.NormalizeInPlace();
		VectorAngles( vecDir, angHordeAngle );

		if ( asw_director_debug.GetInt() >= 2 )
		{
			DevMsg( "  Accepting horde node %d.\n", iChosen );
		}
		DeleteRoute( pRoute );
		return true;
	}

	if ( asw_director_debug.GetBool() )
	{
		DevMsg( "  Failed to find horde pos as we tried 3 times to build routes to possible locations, but failed\n" );
	}

	return false;
}

bool CASW_Spawn_Manager::FindHordePosition()
{
	// need to find a suitable place from which to spawn a horde
	// this place should:
	//   - be far enough away from the marines so the whole horde can spawn before the marines get there
	//   - should have a clear path to the marines
	
	UpdateCandidateNodes();

	// decide if the horde is going to come from behind or in front
	bool bNorth = RandomFloat() < 0.7f;
	if ( m_northCandidateNodes.Count() <= 0 )
	{
		bNorth = false;
	}
	else if ( m_southCandidateNodes.Count() <= 0 )
	{
		bNorth = true;
	}

	CUtlVector<int> &candidateNodes = bNorth ? m_northCandidateNodes : m_southCandidateNodes;
	bool bResult = FindHordePos( bNorth, candidateNodes, m_vecHordePosition, m_angHordeAngle );
	if ( !bResult && rd_horde_retry_on_fail.GetBool() )
	{
		CUtlVector<int> &candidateNodes2 = bNorth ? m_southCandidateNodes : m_northCandidateNodes;
		bResult = candidateNodes2.Count() > 0 && FindHordePos( !bNorth, candidateNodes2, m_vecHordePosition, m_angHordeAngle );
	}
	if ( rd_horde_two_sided.GetBool() )
	{
		CUtlVector<int> &candidateNodes2 = bNorth ? m_southCandidateNodes : m_northCandidateNodes;
		bool bResult2 = FindHordePos( !bNorth, candidateNodes2, m_vecHordePosition2, m_angHordeAngle2 );
		bResult = bResult || bResult2;
	}
	return bResult;
}

bool CASW_Spawn_Manager::LineBlockedByGeometry( const Vector &vecSrc, const Vector &vecEnd )
{
	trace_t tr;
	UTIL_TraceLine( vecSrc,
		vecEnd, MASK_SOLID_BRUSHONLY, 
		NULL, COLLISION_GROUP_NONE, &tr );

	return ( tr.fraction != 1.0f );
}

CASW_Spawner *CASW_Spawn_Manager::FindAvailableSpawner( CASW_Spawn_NPC *pNPC, const Vector & vecPos )
{
	if ( rd_director_spawner_range.GetFloat() < 0 )
	{
		return NULL;
	}

	float flDist;
	CASW_Marine *pMarine = UTIL_ASW_NearestMarine( vecPos, flDist );
	Vector vecBiased = vecPos;
	if ( pMarine )
	{
		VectorLerp( vecPos, pMarine->GetAbsOrigin(), rd_director_spawner_bias.GetFloat(), vecBiased );
	}

	float flRangeSqr = Square( rd_director_spawner_range.GetFloat() );

	const Vector & vecMins = NAI_Hull::Mins( pNPC->m_pAlienClass->m_nHullType );
	const Vector & vecMaxs = NAI_Hull::Maxs( pNPC->m_pAlienClass->m_nHullType );
	int iSeenSpawners = -1;
	CASW_Spawner *pSelectedSpawner = NULL;
	CASW_Spawner *pSpawner = NULL;
	while ( ( pSpawner = assert_cast<CASW_Spawner *>( gEntList.FindEntityByClassname( pSpawner, "asw_spawner" ) ) ) != NULL )
	{
		if ( pSpawner->GetAbsOrigin().DistToSqr( vecBiased ) > flRangeSqr )
		{
			continue;
		}
		if ( !pSpawner->IsEnabled() || pSpawner->m_iAllowDirectorSpawns != 1 || pSpawner->m_AlienClassName != pNPC->m_pAlienClass->m_iszAlienClass )
		{
			continue;
		}
		if ( pSpawner->m_flLastDirectorSpawn + pSpawner->m_flDirectorLockTime > gpGlobals->curtime )
		{
			continue;
		}
		pSpawner->m_vecCurrentSpawnPosition = pSpawner->GetAbsOrigin();
		if ( !pSpawner->CanSpawn( vecMins, vecMaxs, pNPC ) )
		{
			continue;
		}
		iSeenSpawners++;

		// allow any spawner that fits the criteria with equal probability.
		if ( RandomInt( 0, iSeenSpawners ) == 0 )
		{
			pSelectedSpawner = pSpawner;
		}
	}
	return pSelectedSpawner;
}

bool CASW_Spawn_Manager::GetAlienBounds( const char *szAlienClass, Vector &vecMins, Vector &vecMaxs )
{
	int nCount = GetNumAlienClasses();
	for ( int i = 0 ; i < nCount; i++ )
	{
		if ( !Q_stricmp( szAlienClass, GetAlienClass( i )->m_pszAlienClass ) )
		{
			vecMins = NAI_Hull::Mins( GetAlienClass( i )->m_nHullType );
			vecMaxs = NAI_Hull::Maxs (GetAlienClass( i )->m_nHullType );
			return true;
		}
	}
	return false;
}

bool CASW_Spawn_Manager::GetAlienBounds( string_t iszAlienClass, Vector &vecMins, Vector &vecMaxs )
{
	int nCount = GetNumAlienClasses();
	for ( int i = 0 ; i < nCount; i++ )
	{
		if ( iszAlienClass == GetAlienClass( i )->m_iszAlienClass )
		{
			vecMins = NAI_Hull::Mins( GetAlienClass( i )->m_nHullType );
			vecMaxs = NAI_Hull::Maxs (GetAlienClass( i )->m_nHullType );
			return true;
		}
	}
	return false;
}

// spawn a group of aliens at the target point
template <typename Alien>
int CASW_Spawn_Manager::SpawnAlienBatch( Alien szAlienClass, int iNumAliens, const Vector &vecPosition, const QAngle &angFacing, float flMarinesBeyondDist, const Vector & vecMins, const Vector & vecMaxs )
{
	int iSpawned = 0;
	bool bCheckGround = true;

	float flAlienWidth = vecMaxs.x - vecMins.x;
	float flAlienDepth = vecMaxs.y - vecMins.y;

	// spawn one in the middle
	if ( ValidSpawnPoint( vecPosition, vecMins, vecMaxs, bCheckGround, flMarinesBeyondDist ) )
	{
		if ( SpawnAlienAt( szAlienClass, vecPosition, angFacing ) )
			iSpawned++;
	}

	// try to spawn a 5x5 grid of aliens, starting at the centre and expanding outwards
	Vector vecNewPos = vecPosition;
	for ( int i=1; i<=5 && iSpawned < iNumAliens; i++ )
	{
		QAngle angle = angFacing;
		angle[YAW] += RandomFloat( -20, 20 );
		// spawn aliens along top of box
		for ( int x=-i; x<=i && iSpawned < iNumAliens; x++ )
		{
			vecNewPos = vecPosition;
			vecNewPos.x += x * flAlienWidth;
			vecNewPos.y -= i * flAlienDepth;

			if ( ValidHordeShiftedSpawnPoint( vecPosition, vecNewPos, vecMins, vecMaxs, true ) )
			{
				if ( SpawnAlienAt( szAlienClass, vecNewPos, angle ) )
					iSpawned++;
			}
		}

		// spawn aliens along bottom of box
		for ( int x=-i; x<=i && iSpawned < iNumAliens; x++ )
		{
			vecNewPos = vecPosition;
			vecNewPos.x += x * flAlienWidth;
			vecNewPos.y += i * flAlienDepth;
			if ( ValidHordeShiftedSpawnPoint( vecPosition, vecNewPos, vecMins, vecMaxs, true ) )
			{
				if ( SpawnAlienAt( szAlienClass, vecNewPos, angle ) )
					iSpawned++;
			}
		}

		// spawn aliens along left of box
		for ( int y=-i + 1; y<i && iSpawned < iNumAliens; y++ )
		{
			vecNewPos = vecPosition;
			vecNewPos.x -= i * flAlienWidth;
			vecNewPos.y += y * flAlienDepth;
			if ( ValidHordeShiftedSpawnPoint( vecPosition, vecNewPos, vecMins, vecMaxs, true ) )
			{
				if ( SpawnAlienAt( szAlienClass, vecNewPos, angle ) )
					iSpawned++;
			}
		}

		// spawn aliens along right of box
		for ( int y=-i + 1; y<i && iSpawned < iNumAliens; y++ )
		{
			vecNewPos = vecPosition;
			vecNewPos.x += i * flAlienWidth;
			vecNewPos.y += y * flAlienDepth;
			if ( ValidHordeShiftedSpawnPoint( vecPosition, vecNewPos, vecMins, vecMaxs, true ) )
			{
				if ( SpawnAlienAt( szAlienClass, vecNewPos, angle ) )
					iSpawned++;
			}
		}
	}

	return iSpawned;
}

int CASW_Spawn_Manager::SpawnAlienBatch( const char *szAlienClass, int iNumAliens, const Vector &vecPosition, const QAngle &angle, float flMarinesBeyondDist )
{
	Vector vecMins = NAI_Hull::Mins( HULL_MEDIUMBIG );
	Vector vecMaxs = NAI_Hull::Maxs( HULL_MEDIUMBIG );
	GetAlienBounds( szAlienClass, vecMins, vecMaxs );

	CASW_Spawn_NPC npc( szAlienClass );

	return SpawnAlienBatch( &npc, iNumAliens, vecPosition, angle, flMarinesBeyondDist, vecMins, vecMaxs );
}

int CASW_Spawn_Manager::SpawnAlienBatch( CASW_Spawn_Definition *pSpawn, int iNumAliens, const Vector &vecPosition, const QAngle &angle, float flMarinesBeyondDist )
{
	int iHull = pSpawn->m_NPCs.Count() == 0 ? HULL_MEDIUMBIG : pSpawn->m_NPCs[0]->m_pAlienClass->m_nHullType;
	const Vector & vecMins = NAI_Hull::Mins( iHull );
	const Vector & vecMaxs = NAI_Hull::Maxs( iHull );

	return SpawnAlienBatch( pSpawn, iNumAliens, vecPosition, angle, flMarinesBeyondDist, vecMins, vecMaxs );
}

bool CASW_Spawn_Manager::SpawnAlienAt( CASW_Spawn_Definition *pSpawn, const Vector & vecPos, const QAngle & angle, bool bAllowSpawner )
{
	bool bAny = false;
	bool bTried = false;

	FOR_EACH_VEC( pSpawn->m_NPCs, i )
	{
		CASW_Spawn_NPC *pNPC = pSpawn->m_NPCs[i];
		if ( !pNPC->m_Requirement.CanSpawn() || ( pNPC->m_flSpawnChance < 1 && random->RandomFloat() > pNPC->m_flSpawnChance ) )
		{
			continue;
		}
		bTried = true;

		const Vector & vecMins = NAI_Hull::Mins( pNPC->m_pAlienClass->m_nHullType );
		const Vector & vecMaxs = NAI_Hull::Maxs( pNPC->m_pAlienClass->m_nHullType );

		if ( ValidSpawnPoint( vecPos, vecMins, vecMaxs, true, MARINE_NEAR_DISTANCE ) )
		{
			if ( SpawnAlienAt( pNPC, vecPos, vec3_angle, bAllowSpawner ) )
			{
				if ( asw_director_debug.GetBool() )
				{
					NDebugOverlay::Cross3D( vecPos, 25.0f, 255, 255, 255, true, 20.0f );
					float flDist;
					CASW_Marine *pMarine = UTIL_ASW_NearestMarine( vecPos, flDist );
					if ( pMarine )
					{
						NDebugOverlay::Line( pMarine->GetAbsOrigin(), vecPos, 64, 64, 64, true, 60.0f );
					}
				}
				bAny = true;
			}
		}
		else
		{
			if ( asw_director_debug.GetBool() )
			{
				NDebugOverlay::Cross3D( vecPos, 25.0f, 255, 0, 0, true, 20.0f );
			}
		}
	}

	return bAny || !bTried;
}

CBaseEntity *CASW_Spawn_Manager::SpawnAlienAt( CASW_Spawn_NPC *pNPC, const Vector &vecPos, const QAngle &angle, bool bAllowSpawner )
{
	if ( bAllowSpawner )
	{
		CASW_Spawner *pSpawner = FindAvailableSpawner( pNPC, vecPos );
		if ( pSpawner )
		{
			CBaseEntity *pSpawned = dynamic_cast< CBaseEntity * >( pSpawner->SpawnAlien( pNPC->m_pAlienClass->m_pszAlienClass, NAI_Hull::Mins( pNPC->m_pAlienClass->m_nHullType ), NAI_Hull::Maxs( pNPC->m_pAlienClass->m_nHullType ), pNPC ) );
			if ( pSpawned )
			{
				return pSpawned;
			}
		}
	}

	CBaseEntity *pEntity = CreateEntityByName( pNPC->m_pAlienClass->m_pszAlienClass );
	if ( !pEntity )
	{
		return NULL;
	}

	if ( pEntity->IsInhabitableNPC() )
	{
		CASW_Inhabitable_NPC *pAlien = assert_cast< CASW_Inhabitable_NPC * >( pEntity );

		pAlien->m_bFlammable = pNPC->m_bFlammable;
		pAlien->m_bTeslable = pNPC->m_bTeslable;
		pAlien->m_bFreezable = pNPC->m_bFreezable;
		pAlien->m_bFlinchable = pNPC->m_bFlinches;
		pAlien->m_bGrenadeReflector = pNPC->m_bGrenadeReflector;
		pAlien->m_iHealthBonus = pNPC->m_iHealthBonus;
		pAlien->m_fSizeScale = pNPC->m_flSizeScale;
		pAlien->m_fSpeedScale = pNPC->m_flSpeedScale;
	}

	CAI_BaseNPC *pBaseNPC = pEntity->MyNPCPointer();
	if ( pBaseNPC )
	{
		pBaseNPC->AddSpawnFlags( SF_NPC_FALL_TO_GROUND );
	}

	// Strip pitch and roll from the spawner's angles. Pass only yaw to the spawned NPC.
	QAngle angles = angle;
	angles.x = 0.0;
	angles.z = 0.0;
	pEntity->SetAbsOrigin( vecPos );
	pEntity->SetAbsAngles( angles );

	IASW_Spawnable_NPC *pSpawnable = dynamic_cast< IASW_Spawnable_NPC * >( pEntity );
	Assert( pSpawnable );
	if ( !pSpawnable )
	{
		Warning( "NULL Spawnable Ent in CASW_Spawn_Manager::SpawnAlienAt! AlienClass = %s\n", pNPC->m_pAlienClass->m_pszAlienClass );
		UTIL_Remove( pEntity );
		return NULL;
	}

	// have drones and rangers unburrow by default, so we don't worry so much about them spawning onscreen
	if ( pNPC->m_pAlienClass->m_nHullType == HULL_MEDIUMBIG )
	{
		pSpawnable->StartBurrowed();
		pSpawnable->SetUnburrowIdleActivity( NULL_STRING );
		pSpawnable->SetUnburrowActivity( NULL_STRING );
	}

	DispatchSpawn( pEntity );
	pEntity->Activate();

	UTIL_DropToFloor( pEntity, pEntity->PhysicsSolidMaskForEntity() );

	// give our aliens the orders
	pSpawnable->SetAlienOrders( AOT_MoveToNearestMarine, vec3_origin, NULL );

	if ( pNPC->m_iszVScript != NULL_STRING )
	{
		pEntity->RunScriptFile( STRING( pNPC->m_iszVScript ) );
	}

	return pEntity;
}

bool CASW_Spawn_Manager::ValidSpawnPoint( const Vector &vecPosition, const Vector &vecMins, const Vector &vecMaxs, bool bCheckGround, float flMarineNearDistance )
{
	// check if there's a brush telling us to not spawn there
	FOR_EACH_VEC( IRD_No_Director_Aliens::AutoList(), d )
	{
		CRD_No_Director_Aliens *pNoDirectorAliens = assert_cast< CRD_No_Director_Aliens * >( IRD_No_Director_Aliens::AutoList()[d] );
		if ( !pNoDirectorAliens->m_bDisabled && pNoDirectorAliens->CollisionProp()->IsPointInBounds( vecPosition ) )
		{
			return false;
		}
	}

	// check if we can fit there
	trace_t tr;
	UTIL_TraceHull( vecPosition,
		vecPosition + Vector( 0, 0, 1 ),
		vecMins,
		vecMaxs,
		MASK_NPCSOLID,
		NULL,
		COLLISION_GROUP_NONE,
		&tr );

	if( tr.fraction != 1.0 )
		return false;

	if ( tr.startsolid )
		return false;

	// check there's ground underneath this point
	if ( bCheckGround )
	{
		UTIL_TraceHull( vecPosition + Vector( 0, 0, 1 ),
			vecPosition - Vector( 0, 0, 64 ),
			vecMins,
			vecMaxs,
			MASK_NPCSOLID,
			NULL,
			COLLISION_GROUP_NONE,
			&tr );

		if( tr.fraction == 1.0 )
			return false;
	}

	if ( flMarineNearDistance > 0 )
	{
		CASW_Game_Resource* pGameResource = ASWGameResource();
		float distance = 0.0f;
		for ( int i=0 ; i < pGameResource->GetMaxMarineResources() ; i++ )
		{
			CASW_Marine_Resource* pMR = pGameResource->GetMarineResource(i);
			if ( pMR && pMR->GetMarineEntity() && pMR->GetMarineEntity()->GetHealth() > 0 )
			{
				distance = pMR->GetMarineEntity()->GetAbsOrigin().DistTo( vecPosition );
				if ( distance < flMarineNearDistance )
				{
					return false;
				}
			}
		}
	}

	return true;
}

bool CASW_Spawn_Manager::ValidHordeShiftedSpawnPoint( const Vector &vecOrigPos, const Vector &vecPosition, const Vector &vecMins, const Vector &vecMaxs, bool bCheckGround )
{
	// check if we can fit there
	trace_t tr;
	UTIL_TraceHull( vecOrigPos,
		vecPosition,
		vecMins,
		vecMaxs,
		MASK_NPCSOLID,
		NULL,
		COLLISION_GROUP_NONE,
		&tr );

	if( tr.fraction != 1.0 )
		return false;

	if ( tr.startsolid )
		return false;

	// check there's ground underneath this point
	if ( bCheckGround )
	{
		UTIL_TraceHull( vecPosition + Vector( 0, 0, 1 ),
			vecPosition - Vector( 0, 0, 64 ),
			vecMins,
			vecMaxs,
			MASK_NPCSOLID,
			NULL,
			COLLISION_GROUP_NONE,
			&tr );

		if( tr.fraction == 1.0 )
			return false;
	}

	return true;
}

void CASW_Spawn_Manager::DeleteRoute( AI_Waypoint_t *pWaypointList )
{
	while ( pWaypointList )
	{
		AI_Waypoint_t *pPrevWaypoint = pWaypointList;
		pWaypointList = pWaypointList->GetNext();
		delete pPrevWaypoint;
	}
}

bool CASW_Spawn_Manager::PrespawnAliens( CASW_Spawn_Definition *pSpawn )
{
	int iNumNodes = GetNetwork()->NumNodes();
	if ( iNumNodes < 6 )
		return false;

	int nHull = pSpawn->m_NPCs.Count() == 0 ? HULL_WIDE_SHORT : pSpawn->m_NPCs[0]->m_pAlienClass->m_nHullType;
	CUtlVectorAutoPurge<CASW_Open_Area *> aAreas;
	for ( int i = 0; i < pSpawn->m_NPCs.Count() * 2; i++ )
	{
		CAI_Node *pNode = NULL;
		int nTries = 0;
		while ( nTries < 5 && ( !pNode || pNode->GetType() != NODE_GROUND ) )
		{
			pNode = GetNetwork()->GetNode( RandomInt( 0, iNumNodes - 1 ) );
			nTries++;
		}
		
		if ( pNode && pNode->GetType() == NODE_GROUND )
		{
			CASW_Open_Area *pArea = FindNearbyOpenArea( pNode->GetOrigin(), HULL_MEDIUMBIG );
			if ( pArea )
			{
				if ( pArea->m_nTotalLinks > 30 )
				{ 
					// test if there's room to spawn a shieldbug at that spot
					if ( ValidSpawnPoint( pArea->m_pNode->GetPosition( nHull ), NAI_Hull::Mins( nHull ), NAI_Hull::Maxs( nHull ), true, 0.0f ) )
					{
						aAreas.AddToTail( pArea );
					}
					else
					{
						delete pArea;
					}
				}
				else
				{
					delete pArea;
				}
			}
		}
		// stop searching once we have enough acceptable candidates
		if ( aAreas.Count() >= pSpawn->m_NPCs.Count() )
			break;
	}

	bool bAny = false;
	bool bTried = false;
	int iArea = 0;
	FOR_EACH_VEC( pSpawn->m_NPCs, i )
	{
		if ( iArea >= aAreas.Count() )
		{
			break;
		}

		CASW_Spawn_NPC *pNPC = pSpawn->m_NPCs[i];
		if ( !pNPC->m_Requirement.CanSpawn() || ( pNPC->m_flSpawnChance < 1 && random->RandomFloat() > pNPC->m_flSpawnChance ) )
		{
			continue;
		}
		bTried = true;

		CBaseEntity *pAlien = SpawnAlienAt( pNPC, aAreas[iArea]->m_pNode->GetPosition( pNPC->m_pAlienClass->m_nHullType ), RandomAngle( 0, 360 ) );
		if ( pAlien )
		{
			bAny = true;
			if ( asw_director_debug.GetBool() && pAlien )
			{
				Msg( "Spawned %s at %f %f %f\n", pNPC->m_pAlienClass->m_pszAlienClass, VectorExpand( pAlien->GetAbsOrigin() ) );
				NDebugOverlay::Cross3D( pAlien->GetAbsOrigin(), 8.0f, 255, 0, 0, true, 20.0f );
			}
		}
		IASW_Spawnable_NPC *pSpawnable = dynamic_cast<IASW_Spawnable_NPC *>( pAlien );
		if ( pSpawnable )
		{
			pSpawnable->SetAlienOrders( AOT_SpreadThenHibernate, vec3_origin, NULL );
		}

		iArea++;
	}

	return bAny || !bTried;
}

Vector TraceToGround( const Vector &vecPos )
{
	trace_t tr;
	UTIL_TraceLine( vecPos + Vector( 0, 0, 100 ), vecPos, MASK_NPCSOLID, NULL, ASW_COLLISION_GROUP_PARASITE, &tr );
	return tr.endpos;
}

bool CASW_Spawn_Manager::SpawnAlienPack( CASW_Spawn_Definition *pSpawn )
{
	int iNumNodes = GetNetwork()->NumNodes();
	if ( iNumNodes < 6 )
		return false;

	int nHull = pSpawn->m_NPCs.Count() == 0 ? HULL_TINY : pSpawn->m_NPCs[0]->m_pAlienClass->m_nHullType;
	CUtlVectorAutoPurge<CASW_Open_Area *> aAreas;
	for ( int i = 0; i < 6; i++ )
	{
		CAI_Node *pNode = NULL;
		int nTries = 0;
		while ( nTries < 5 && ( !pNode || pNode->GetType() != NODE_GROUND ) )
		{
			pNode = GetNetwork()->GetNode( RandomInt( 0, iNumNodes ) );
			nTries++;
		}

		if ( pNode )
		{
			CASW_Open_Area *pArea = FindNearbyOpenArea( pNode->GetOrigin(), HULL_MEDIUMBIG );
			if ( pArea )
			{
				if ( pArea->m_nTotalLinks > 30 )
				{ 
					// test if there's room to spawn a shieldbug at that spot
					if ( ValidSpawnPoint( pArea->m_pNode->GetPosition( nHull ), NAI_Hull::Mins( nHull ), NAI_Hull::Maxs( nHull ), true, 0.0f ) )
					{
						aAreas.AddToTail( pArea );
					}
					else
					{
						delete pArea;
					}
				}
				else
				{
					delete pArea;
				}
			}
		}
		// stop searching once we have 3 acceptable candidates
		if ( aAreas.Count() >= 3 )
			break;
	}

	// find area with the highest connectivity
	CASW_Open_Area *pBestArea = NULL;
	for ( int i = 0; i < aAreas.Count(); i++ )
	{
		CASW_Open_Area *pArea = aAreas[i];
		if ( !pBestArea || pArea->m_nTotalLinks > pBestArea->m_nTotalLinks )
		{
			pBestArea = pArea;
		}
	}

	if ( pBestArea )
	{
		FOR_EACH_VEC( pSpawn->m_NPCs, i )
		{
			CASW_Spawn_NPC *pNPC = pSpawn->m_NPCs[i];

			// reactivedrop: 
			// raise the position by 12 units, a workaround for parasites
			// falling through displacements
			CBaseEntity *pAlien = SpawnAlienAt( pNPC, pBestArea->m_pNode->GetPosition( nHull )  + Vector( 0, 0, 12 ), RandomAngle( 0, 360 ) );
			if ( asw_director_debug.GetBool() && pAlien )
			{
				Msg( "Spawned %s at %f %f %f\n", pNPC->m_pAlienClass->m_pszAlienClass, VectorExpand( pAlien->GetAbsOrigin() ) );
				NDebugOverlay::Cross3D( pAlien->GetAbsOrigin(), 8.0f, 255, 0, 0, true, 20.0f );
			}
			IASW_Spawnable_NPC *pSpawnable = dynamic_cast<IASW_Spawnable_NPC *>( pAlien );
			if ( pSpawnable )
			{
				pSpawnable->SetAlienOrders( AOT_SpreadThenHibernate, vec3_origin, NULL );
			}
		}
		return true;
	}

	return false;
}

ConVar rm_prespawn_num_parasites("rm_prespawn_num_parasites", "7", FCVAR_CHEAT, "Num aliens to randomly spawn if rd_prespawn_scale 1");
ConVar rm_prespawn_num_boomers("rm_prespawn_num_boomers", "3", FCVAR_CHEAT, "Num aliens to randomly spawn if rd_prespawn_scale 1");
ConVar rm_prespawn_num_mortars("rm_prespawn_num_mortars", "2", FCVAR_CHEAT, "Num aliens to randomly spawn if rd_prespawn_scale 1");
ConVar rm_prespawn_num_harvesters("rm_prespawn_num_harvesters", "4", FCVAR_CHEAT, "Num aliens to randomly spawn if rd_prespawn_scale 1");
ConVar rm_prespawn_num_drones("rm_prespawn_num_drones", "15", FCVAR_CHEAT, "Num aliens to randomly spawn if rd_prespawn_scale 1");
ConVar rm_prespawn_num_uber_drones("rm_prespawn_num_uber_drones", "2", FCVAR_CHEAT, "Num aliens to randomly spawn if rd_prespawn_scale 1");
ConVar rm_prespawn_num_shieldbugs("rm_prespawn_num_shieldbugs", "1", FCVAR_CHEAT, "Num aliens to randomly spawn if rd_prespawn_scale 1");
ConVar rm_prespawn_num_shamans("rm_prespawn_num_shamans", "5", FCVAR_CHEAT, "Num aliens to randomly spawn if rd_prespawn_scale 1");
ConVar rm_prespawn_num_buzzers("rm_prespawn_num_buzzers", "1", FCVAR_CHEAT, "Num aliens to randomly spawn if rd_prespawn_scale 1");
ConVar rm_prespawn_num_rangers("rm_prespawn_num_rangers", "5", FCVAR_CHEAT, "Num aliens to randomly spawn if rd_prespawn_scale 1");
ConVar rm_prespawn_num_biomass( "rm_prespawn_num_biomass", "3", FCVAR_CHEAT, "Num biomass to randomly spawn if rd_prespawn_scale 1" );

void CASW_Spawn_Manager::PrespawnAliens(int multiplier)
{
	// spawn random parasites 7, boomers 3, mortars 3, harvesters 4, 
	// drones 15, uber drones 2, shieldbug 1, shaman 7, flies 1
	
	// get num nodes
	// get random node
	// check if it's far enough from player start, 1000 units
	// check if valid spawn point for an alien
	// spawn alien

	const int NUM_PARASITES		= rm_prespawn_num_parasites.GetInt();
	const int NUM_BOOMERS		= rm_prespawn_num_boomers.GetInt();
	const int NUM_MORTARS		= rm_prespawn_num_mortars.GetInt();
	const int NUM_HARVESTERS	= rm_prespawn_num_harvesters.GetInt();
	const int NUM_DRONES		= rm_prespawn_num_drones.GetInt();
	const int NUM_UBER_DRONES	= rm_prespawn_num_uber_drones.GetInt();
	const int NUM_SHIELDBUGS	= rm_prespawn_num_shieldbugs.GetInt();
	const int NUM_SHAMANS		= rm_prespawn_num_shamans.GetInt();
	//const int NUM_FLIES			= rm_prespawn_num_buzzers.GetInt();
	const int NUM_RANGERS		= rm_prespawn_num_rangers.GetInt();
	const int NUM_BIOMASS		= rm_prespawn_num_biomass.GetInt();


	int iNumNodes = g_pBigAINet->NumNodes();
	if (iNumNodes < 50)
		return;

	CBaseEntity *pPlayerStart = gEntList.FindEntityByClassname(NULL, "info_player_start");
	if (!pPlayerStart)
		return;
	Vector playerStartPos = pPlayerStart->GetAbsOrigin();

	PrespawnAlienAtRandomNode("asw_parasite",	NUM_PARASITES * multiplier, HULL_TINY, playerStartPos, iNumNodes);
	PrespawnAlienAtRandomNode("asw_boomer",		NUM_BOOMERS * multiplier, HULL_LARGE, playerStartPos, iNumNodes);
	PrespawnAlienAtRandomNode("asw_mortarbug",	NUM_MORTARS * multiplier, HULL_WIDE_SHORT, playerStartPos, iNumNodes);
	PrespawnAlienAtRandomNode("asw_harvester",	NUM_HARVESTERS * multiplier, HULL_WIDE_SHORT, playerStartPos, iNumNodes);
	PrespawnAlienAtRandomNode("asw_drone",		NUM_DRONES * multiplier, HULL_MEDIUMBIG, playerStartPos, iNumNodes);
	PrespawnAlienAtRandomNode("asw_drone_uber", NUM_UBER_DRONES * multiplier, HULL_MEDIUMBIG, playerStartPos, iNumNodes);
	PrespawnAlienAtRandomNode("asw_shieldbug",	NUM_SHIELDBUGS * multiplier, HULL_WIDE_SHORT, playerStartPos, iNumNodes);
	PrespawnAlienAtRandomNode("asw_shaman",		NUM_SHAMANS * multiplier, HULL_MEDIUM, playerStartPos, iNumNodes);
	//PrespawnAlienAtRandomNode("asw_buzzer",		NUM_FLIES * multiplier, HULL_LARGE, playerStartPos, iNumNodes);	
	PrespawnAlienAtRandomNode("asw_ranger",		NUM_RANGERS * multiplier, HULL_MEDIUMBIG, playerStartPos, iNumNodes);

	PrespawnEntityAtRandomNode( "asw_alien_goo", NUM_BIOMASS * multiplier, playerStartPos, iNumNodes );
}


void CASW_Spawn_Manager::PrespawnAlienAtRandomNode(const char *szAlienClass, const int iNumAliens, const int iHull, const Vector &playerStartPos, const int iNumNodes)
{
	CASW_Spawn_NPC npc( szAlienClass );
	for (int i = 0; i < iNumAliens; ++i)
	{
		CAI_Node *pNode = NULL;
		for (int k = 0; k < 30; ++k)
		{
			int node_id = RandomInt(0, iNumNodes - 1);
			pNode = g_pBigAINet->GetNode(node_id);
			if (!pNode || pNode->GetType() != NODE_GROUND)
				continue;
			else if (pNode->GetOrigin().DistToSqr(playerStartPos) < 1000 * 1000)
			{
				continue;
			}

			if (ValidSpawnPoint(pNode->GetPosition(iHull), NAI_Hull::Mins(iHull), NAI_Hull::Maxs(iHull), true, 0.0f))
			{
				// Raise the end position a little up off the floor, place the npc and drop him down
				CBaseEntity *pAlien = SpawnAlienAt( &npc, pNode->GetPosition( iHull ) + Vector( 0.f, 0.f, 12.f ), RandomAngle( 0, 360 ) );
				IASW_Spawnable_NPC *pSpawnable = dynamic_cast<IASW_Spawnable_NPC*>(pAlien);
				if (pSpawnable)
				{
					pSpawnable->SetAlienOrders(AOT_SpreadThenHibernate, vec3_origin, NULL);
				}
				if (asw_director_debug.GetBool() && pAlien)
				{
					Msg("Spawned alien at %f %f %f\n", pAlien->GetAbsOrigin().x, pAlien->GetAbsOrigin().y, pAlien->GetAbsOrigin().z);
					NDebugOverlay::Cross3D(pAlien->GetAbsOrigin(), 8.0f, 255, 0, 0, true, 20.0f);
				}
				if (pAlien)
					break;
			}
		}
	}
}

void CASW_Spawn_Manager::PrespawnEntityAtRandomNode( const char *szEntityClass, const int iNumEntitiesToSpawn, const Vector &playerStartPos, const int iNumNodes )
{
	for ( int i = 0; i < iNumEntitiesToSpawn; ++i )
	{
		CAI_Node *pNode = NULL;
		for ( int k = 0; k < 30; ++k )	// 30 tries to find a node
		{
			int node_id = RandomInt( 0, iNumNodes - 1 );
			pNode = g_pBigAINet->GetNode( node_id );
			if ( !pNode || pNode->GetType() != NODE_GROUND )
				continue;
			else if ( pNode->GetOrigin().DistToSqr( playerStartPos ) < 1000 * 1000 )
			{
				continue;
			}

			MDLCACHE_CRITICAL_SECTION();

			bool allowPrecache = CBaseEntity::IsPrecacheAllowed();
			CBaseEntity::SetAllowPrecache( true );

			// Try to create entity
			CBaseEntity *entity = dynamic_cast< CBaseEntity * >( CreateEntityByName( szEntityClass ) );
			if ( entity )
			{
				Vector vecPositionOffset(0, 0, 0);
				QAngle ang(0, 0, 0);
				if ( entity->ClassMatches( "asw_alien_goo" ) )
				{
					CASW_Alien_Goo *pBiomass = static_cast< CASW_Alien_Goo* >( entity );
					const int nBiomassModelId = RandomInt( 1, 4 );
					switch ( nBiomassModelId )
					{
					case 1: 
					default:
						pBiomass->SetModelName( AllocPooledString( "models/aliens/biomass/biomasshelix.mdl" ) );
						// each biomass model needs their own offset and angle not to float into the air
						vecPositionOffset.Init( 0, 0, -32.f );
						ang.Init( 0, RandomFloat( 0.0f, 360.0f ), 0.0f );
						break;
					case 2: 
						pBiomass->SetModelName( AllocPooledString( "models/aliens/biomass/biomassl.mdl" ) ); 
						vecPositionOffset.Init( 0, 0, -32.f );
						ang.Init( 40.f, RandomFloat( 0.0f, 360.0f ), 0.0f );
						break;
					case 3: 
						pBiomass->SetModelName( AllocPooledString( "models/aliens/biomass/biomasss.mdl" ) ); 
						vecPositionOffset.Init( 0, 0, -32.f );
						ang.Init( 0, RandomFloat( 0.0f, 360.0f ), 0.0f );
						break;
					case 4: 
						pBiomass->SetModelName( AllocPooledString( "models/aliens/biomass/biomassu.mdl" ) ); 
						vecPositionOffset.Init( 0, 0, 32.f );
						ang.Init( -81.5, RandomFloat( 0.0f, 360.0f ), 0.0f );
						break;
					}
				}
				entity->Precache();
				Vector pos = pNode->GetOrigin() + vecPositionOffset;
				entity->Teleport( &pos, &ang, NULL );

				DispatchSpawn( entity );
			}
			CBaseEntity::SetAllowPrecache( allowPrecache );

			if ( entity )
				break;			// exit from 30 tries
		}
	}
}

// heuristic to find reasonably open space - searches for areas with high node connectivity
CASW_Open_Area* CASW_Spawn_Manager::FindNearbyOpenArea( const Vector &vecSearchOrigin, int nSearchHull )
{
	CBaseEntity *pStartEntity = gEntList.FindEntityByClassname( NULL, "info_player_start" );
	int iNumNodes = g_pBigAINet->NumNodes();
	CAI_Node *pHighestConnectivity = NULL;
	int nHighestLinks = 0;
	for ( int i=0 ; i<iNumNodes; i++ )
	{
		CAI_Node *pNode = g_pBigAINet->GetNode( i );
		if ( !pNode || pNode->GetType() != NODE_GROUND )
			continue;

		Vector vecPos = pNode->GetOrigin();
		float flDist = vecPos.DistTo( vecSearchOrigin );
		if ( flDist > 400.0f )
			continue;

		// discard if node is too near start location
		if ( pStartEntity && vecPos.DistTo( pStartEntity->GetAbsOrigin() ) < 1400.0f )  // NOTE: assumes all start points are clustered near one another
			continue;

		// discard if node is inside an escape area
		bool bInsideEscapeArea = false;
		for ( int d=0; d<m_EscapeTriggers.Count(); d++ )
		{
			if ( !m_EscapeTriggers[d]->m_bDisabled && m_EscapeTriggers[d]->CollisionProp()->IsPointInBounds( vecPos ) )
			{
				bInsideEscapeArea = true;
				break;
			}
		}
		if ( bInsideEscapeArea )
			continue;

		FOR_EACH_VEC( IRD_No_Director_Aliens::AutoList(), d )
		{
			CRD_No_Director_Aliens *pNoDirectorAliens = assert_cast<CRD_No_Director_Aliens *>( IRD_No_Director_Aliens::AutoList()[d] );
			if ( !pNoDirectorAliens->m_bDisabled && pNoDirectorAliens->CollisionProp()->IsPointInBounds( vecPos ) )
			{
				bInsideEscapeArea = true;
				break;
			}
		}
		if ( bInsideEscapeArea )
			continue;

		// count links that drones could follow
		int nLinks = pNode->NumLinks();
		int nValidLinks = 0;
		for ( int k = 0; k < nLinks; k++ )
		{
			CAI_Link *pLink = pNode->GetLinkByIndex( k );
			if ( !pLink )
				continue;

			if ( !( pLink->m_iAcceptedMoveTypes[nSearchHull] & bits_CAP_MOVE_GROUND ) )
				continue;

			nValidLinks++;
		}
		if ( nValidLinks > nHighestLinks )
		{
			nHighestLinks = nValidLinks;
			pHighestConnectivity = pNode;
		}
		if ( asw_director_debug.GetBool() )
		{
			NDebugOverlay::Text( vecPos, UTIL_VarArgs( "%d", nValidLinks ), false, 10.0f );
		}
	}

	if ( !pHighestConnectivity )
		return NULL;

	// now, starting at the new node, find all nearby nodes with a minimum connectivity
	CASW_Open_Area *pArea = new CASW_Open_Area();
	pArea->m_vecOrigin = pHighestConnectivity->GetOrigin();
	pArea->m_pNode = pHighestConnectivity;
	int nMinLinks = nHighestLinks * 0.3f;
	nMinLinks = MAX( nMinLinks, 4 );

	pArea->m_aAreaNodes.AddToTail( pHighestConnectivity );
	if ( asw_director_debug.GetBool() )
	{
		Msg( "minLinks = %d\n", nMinLinks );
	}
	pArea->m_nTotalLinks = 0;
	for ( int i=0 ; i<iNumNodes; i++ )
	{
		CAI_Node *pNode = g_pBigAINet->GetNode( i );
		if ( !pNode || pNode->GetType() != NODE_GROUND )
			continue;

		Vector vecPos = pNode->GetOrigin();
		float flDist = vecPos.DistTo( pArea->m_vecOrigin );
		if ( flDist > 400.0f )
			continue;

		// discard if node is inside an escape area
		bool bInsideEscapeArea = false;
		for ( int d=0; d<m_EscapeTriggers.Count(); d++ )
		{
			if ( !m_EscapeTriggers[d]->m_bDisabled && m_EscapeTriggers[d]->CollisionProp()->IsPointInBounds( vecPos ) )
			{
				bInsideEscapeArea = true;
				break;
			}
		}
		if ( bInsideEscapeArea )
			continue;

		// count links that drones could follow
		int nLinks = pNode->NumLinks();
		int nValidLinks = 0;
		for ( int k = 0; k < nLinks; k++ )
		{
			CAI_Link *pLink = pNode->GetLinkByIndex( k );
			if ( !pLink )
				continue;

			if ( !( pLink->m_iAcceptedMoveTypes[nSearchHull] & bits_CAP_MOVE_GROUND ) )
				continue;

			nValidLinks++;
		}
		if ( nValidLinks >= nMinLinks )
		{
			pArea->m_aAreaNodes.AddToTail( pNode );
			pArea->m_nTotalLinks += nValidLinks;
		}
	}
	// highlight and measure bounds
	Vector vecAreaMins = Vector( FLT_MAX, FLT_MAX, FLT_MAX );
	Vector vecAreaMaxs = Vector( -FLT_MAX, -FLT_MAX, -FLT_MAX );
	
	for ( int i = 0; i < pArea->m_aAreaNodes.Count(); i++ )
	{
		vecAreaMins = VectorMin( vecAreaMins, pArea->m_aAreaNodes[i]->GetOrigin() );
		vecAreaMaxs = VectorMax( vecAreaMaxs, pArea->m_aAreaNodes[i]->GetOrigin() );
		
		if ( asw_director_debug.GetBool() )
		{
			if ( i == 0 )
			{
				NDebugOverlay::Cross3D( pArea->m_aAreaNodes[i]->GetOrigin(), 20.0f, 255, 255, 64, true, 10.0f );
			}
			else
			{
				NDebugOverlay::Cross3D( pArea->m_aAreaNodes[i]->GetOrigin(), 10.0f, 255, 128, 0, true, 10.0f );
			}
		}
	}

	Vector vecArea = ( vecAreaMaxs - vecAreaMins );
	float flArea = vecArea.x * vecArea.y;

	if ( asw_director_debug.GetBool() )
	{
		Msg( "area mins = %f %f %f\n", VectorExpand( vecAreaMins ) );
		Msg( "area maxs = %f %f %f\n", VectorExpand( vecAreaMaxs ) );
		NDebugOverlay::Box( vec3_origin, vecAreaMins, vecAreaMaxs, 255, 128, 128, 10, 10.0f );	
		Msg( "Total links = %d Area = %f\n", pArea->m_nTotalLinks, flArea );
	}

	return pArea;
}

// creates a batch of aliens at the mouse cursor
void asw_alien_batch_f( const CCommand& args )
{
	MDLCACHE_CRITICAL_SECTION();

	bool allowPrecache = CBaseEntity::IsPrecacheAllowed();
	CBaseEntity::SetAllowPrecache( true );

	// find spawn point
	CASW_Player* pPlayer = ToASW_Player(UTIL_GetCommandClient());
	if (!pPlayer)
		return;
	CASW_Inhabitable_NPC *pNPC = pPlayer->GetNPC();
	if ( !pNPC )
		return;
	trace_t tr;
	Vector forward;

	AngleVectors( pNPC->EyeAngles(), &forward );
	UTIL_TraceLine( pNPC->EyePosition(),
		pNPC->EyePosition() + forward * 300.0f, MASK_SOLID,
		pNPC, COLLISION_GROUP_NONE, &tr );
	if ( tr.fraction != 0.0 )
	{
		// trace to the floor from this spot
		Vector vecSrc = tr.endpos;
		tr.endpos.z += 12;
		UTIL_TraceLine( vecSrc + Vector( 0, 0, 12 ),
			vecSrc - Vector( 0, 0, 512 ), MASK_SOLID,
			pNPC, COLLISION_GROUP_NONE, &tr );
		
		ASWSpawnManager()->SpawnAlienBatch( "asw_parasite", 25, tr.endpos, vec3_angle );
	}
	
	CBaseEntity::SetAllowPrecache( allowPrecache );
}
static ConCommand asw_alien_batch("asw_alien_batch", asw_alien_batch_f, "Creates a batch of aliens at the cursor", FCVAR_GAMEDLL | FCVAR_CHEAT);


CON_COMMAND_F( asw_alien_horde, "Creates a horde of aliens somewhere nearby", FCVAR_CHEAT )
{
	if ( args.ArgC() < 2 )
	{
		Msg("supply horde size!\n");
		return;
	}

	int iHordeSize = atoi( args[1] );
	if ( iHordeSize <= 0 )
	{
		Msg("invalid horde size\n");
		return;
	}

	CASW_Spawn_Definition *pSpawn = ASWSpawnSelection()->RandomHordeDefinition();
	if ( !pSpawn )
	{
		Msg("no HORDE definitions!\n");
		return;
	}

	if ( ASWSpawnManager()->AddHorde( iHordeSize, pSpawn ) )
	{
		Msg("queued horde of %d aliens\n", iHordeSize);
	}
}

CON_COMMAND_F( asw_spawn_shieldbug, "Spawns a shieldbug somewhere randomly in the map", FCVAR_CHEAT )
{
	CASW_Spawn_Definition spawn( KeyValues::AutoDeleteInline( KeyValues::FromString( "PRESPAWN", "NPC { AlienClass asw_shieldbug }" ) ) );

	ASWSpawnManager()->PrespawnAliens( &spawn );
}

CON_COMMAND_F( asw_spawn_parasite_pack, "Spawns a group of parasites somewhere randomly in the map", FCVAR_CHEAT )
{
	CASW_Spawn_Definition spawn( KeyValues::AutoDeleteInline( KeyValues::FromString( "PACK",
		"NPC { AlienClass asw_parasite } "
		"NPC { AlienClass asw_parasite } "
		"NPC { AlienClass asw_parasite } "
		"NPC { AlienClass asw_parasite SpawnChance 0.75 } "
		"NPC { AlienClass asw_parasite SpawnChance 0.5 } "
		"NPC { AlienClass asw_parasite SpawnChance 0.25 }" ) ) );

	ASWSpawnManager()->SpawnAlienPack( &spawn );
}
