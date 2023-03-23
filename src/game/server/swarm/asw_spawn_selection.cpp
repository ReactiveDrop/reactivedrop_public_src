#include "cbase.h"
#include "asw_spawn_selection.h"
#include "filesystem.h"
#include "convar.h"
#include "asw_gamerules.h"
#include "asw_objective.h"
#include "asw_spawner.h"
#include "asw_util_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar rd_challenge;
ConVar rd_override_alien_selection_challenge( "rd_override_alien_selection_challenge", "", FCVAR_CHEAT | FCVAR_NOTIFY );

static CASW_Spawn_Selection g_ASW_Spawn_Selection;
CASW_Spawn_Selection *ASWSpawnSelection() { return &g_ASW_Spawn_Selection; }

void CASW_Spawn_Selection::LevelInitPreEntity()
{
	KeyValues *pKV = new KeyValues( "SpawnSet" );
	if ( UTIL_RD_LoadKeyValuesFromFile( pKV, filesystem, "resource/alien_selection.txt", "GAME" ) )
	{
		DevMsg( "Loading global spawn selection data\n" );
		PopulateSpawnSets( pKV );
	}
	else
	{
		DevMsg( "No global spawn selection data\n" );
	}
	pKV->deleteThis();

	char szFilename[MAX_PATH];
	Q_snprintf( szFilename, sizeof( szFilename ), "resource/alien_selection_%s.txt", STRING( gpGlobals->mapname ) );

	pKV = new KeyValues( "SpawnSet" );
	if ( UTIL_RD_LoadKeyValuesFromFile( pKV, filesystem, szFilename, "GAME" ) )
	{
		DevMsg( "Loading map-specific spawn selection data for %s\n", STRING( gpGlobals->mapname ) );
		PopulateSpawnSets( pKV );
	}
	else
	{
		DevMsg( "No map-specific spawn selection data for %s\n", STRING( gpGlobals->mapname ) );
	}
	pKV->deleteThis();
}

void CASW_Spawn_Selection::LevelShutdownPostEntity()
{
	m_SpawnSets.PurgeAndDeleteElements();
	m_pSelectedSpawnSet = NULL;
}

void CASW_Spawn_Selection::OnMissionStarted()
{
	m_pSelectedSpawnSet = NULL;

	// Which challenge is active is only known once the mission starts, so load the spawn set here.
	const char *pszChallenge = rd_override_alien_selection_challenge.GetString();
	if ( !Q_stricmp( pszChallenge, "" ) )
	{
		pszChallenge = rd_challenge.GetString();
	}
	if ( Q_stricmp( pszChallenge, "" ) )
	{
		char szFilename[MAX_PATH];
		Q_snprintf( szFilename, sizeof( szFilename ), "resource/alien_selection_%s.txt", pszChallenge );

		KeyValues *pKV = new KeyValues( "SpawnSet" );
		if ( UTIL_RD_LoadKeyValuesFromFile( pKV, filesystem, szFilename, "GAME" ) )
		{
			DevMsg( "Loading challenge-specific spawn selection data for %s\n", pszChallenge );
			PopulateSpawnSets( pKV );
		}
		else
		{
			DevMsg( "No challenge-specific spawn selection data for %s\n", pszChallenge );
		}
		pKV->deleteThis();

		Q_snprintf( szFilename, sizeof( szFilename ), "resource/alien_selection_%s_%s.txt", STRING( gpGlobals->mapname ), pszChallenge );

		pKV = new KeyValues( "SpawnSet" );
		if ( UTIL_RD_LoadKeyValuesFromFile( pKV, filesystem, szFilename, "GAME" ) )
		{
			DevMsg( "Loading map+challenge-specific spawn selection data for %s+%s\n", STRING( gpGlobals->mapname ), pszChallenge );
			PopulateSpawnSets( pKV );
		}
		else
		{
			DevMsg( "No map+challenge-specific spawn selection data for %s+%s\n", STRING( gpGlobals->mapname ), pszChallenge );
		}
		pKV->deleteThis();
	}

	Assert( ASWGameRules() );
	CUtlVector<CASW_Spawn_Set *> overlays;
	FOR_EACH_VEC_BACK( m_SpawnSets, i )
	{
		CASW_Spawn_Set *pSpawnSet = m_SpawnSets[i];
		if ( pSpawnSet->m_iMinSkill > ASWGameRules()->GetSkillLevel() )
		{
			continue;
		}
		if ( pSpawnSet->m_iMaxSkill < ASWGameRules()->GetSkillLevel() )
		{
			continue;
		}
		if ( pSpawnSet->m_bOverlay )
		{
			Msg( "Selected spawn set overlay: %s\n", STRING( pSpawnSet->m_iszName ) );
			overlays.AddToTail( pSpawnSet );
			continue;
		}
		Msg( "Selected spawn set: %s\n", STRING( pSpawnSet->m_iszName ) );
		m_pSelectedSpawnSet = pSpawnSet;
		FOR_EACH_VEC_BACK( overlays, j )
		{
			overlays[j]->ApplyOverlay( pSpawnSet );
		}
		return;
	}

	Warning( "No applicable spawn sets out of %d candidate sets! (picked %d overlays)\n", m_SpawnSets.Count(), overlays.Count() );
}

void CASW_Spawn_Selection::Dump()
{
	if ( !m_pSelectedSpawnSet )
	{
		if ( ASWGameRules() && ASWGameRules()->GetGameState() >= ASW_GS_INGAME )
		{
			CmdMsg( "No selected spawn set. Director will not spawn aliens.\n" );
		}
		else
		{
			CmdMsg( "Spawn selection does not occur until mission start.\n" );
		}
		return;
	}

	m_pSelectedSpawnSet->Dump();
}

int CASW_Spawn_Selection::RandomHordeSize()
{
	if ( !m_pSelectedSpawnSet )
	{
		return 0;
	}

	return random->RandomInt( m_pSelectedSpawnSet->m_iMinHordeSize, m_pSelectedSpawnSet->m_iMaxHordeSize );
}

int CASW_Spawn_Selection::RandomWandererCount()
{
	if ( !m_pSelectedSpawnSet )
	{
		return 0;
	}

	return random->RandomInt( m_pSelectedSpawnSet->m_iMinWanderers, m_pSelectedSpawnSet->m_iMaxWanderers );
}

int CASW_Spawn_Selection::RandomHordeWandererCount()
{
	if ( !m_pSelectedSpawnSet )
	{
		return 0;
	}

	return random->RandomInt( m_pSelectedSpawnSet->m_iMinHordeWanderers, m_pSelectedSpawnSet->m_iMaxHordeWanderers );
}

int CASW_Spawn_Selection::RandomPrespawnCount()
{
	if ( !m_pSelectedSpawnSet )
	{
		return 0;
	}

	return random->RandomInt( m_pSelectedSpawnSet->m_iMinPrespawn, m_pSelectedSpawnSet->m_iMaxPrespawn );
}

int CASW_Spawn_Selection::RandomPackCount()
{
	if ( !m_pSelectedSpawnSet )
	{
		return 0;
	}

	return random->RandomInt( m_pSelectedSpawnSet->m_iMinPacks, m_pSelectedSpawnSet->m_iMaxPacks );
}

CASW_Spawn_Definition *CASW_Spawn_Selection::RandomHordeDefinition()
{
	return RandomDefinition( ASW_SPAWN_TYPE_HORDE );
}

CASW_Spawn_Definition *CASW_Spawn_Selection::RandomWandererDefinition()
{
	return RandomDefinition( ASW_SPAWN_TYPE_WANDERER );
}

CASW_Spawn_Definition *CASW_Spawn_Selection::RandomHordeWandererDefinition()
{
	return RandomDefinition( ASW_SPAWN_TYPE_HORDE_WANDERER );
}

CASW_Spawn_Definition *CASW_Spawn_Selection::RandomPrespawnDefinition()
{
	return RandomDefinition( ASW_SPAWN_TYPE_PRESPAWN );
}

CASW_Spawn_Definition *CASW_Spawn_Selection::RandomPackDefinition()
{
	return RandomDefinition( ASW_SPAWN_TYPE_PACK );
}

CASW_Spawn_Definition *CASW_Spawn_Selection::RandomDefinition( ASW_Spawn_Type iSpawnType )
{
	static CASW_Spawn_Definition *s_pNoSpawnDefinition = new CASW_Spawn_Definition( NULL );

	Assert( m_pSelectedSpawnSet );

	CASW_Spawn_Definition *pLast = s_pNoSpawnDefinition;

	float flSelectedSpawn = random->RandomFloat( 0, m_pSelectedSpawnSet->ComputeTotalWeight( iSpawnType ) );
	FOR_EACH_VEC( m_pSelectedSpawnSet->m_SpawnDefinitions[iSpawnType], i )
	{
		CASW_Spawn_Definition *pSpawn = m_pSelectedSpawnSet->m_SpawnDefinitions[iSpawnType][i];
		if ( !pSpawn->m_Requirement.CanSpawn() )
		{
			continue;
		}

		pLast = pSpawn;

		flSelectedSpawn -= pSpawn->m_flSelectionWeight;
		if ( flSelectedSpawn <= 0 )
		{
			return pSpawn;
		}
	}

	return pLast;
}

float CASW_Spawn_Selection::ComputeTotalWeight( ASW_Spawn_Type iSpawnType )
{
	Assert( m_pSelectedSpawnSet );

	return m_pSelectedSpawnSet->ComputeTotalWeight( iSpawnType );
}

void CASW_Spawn_Selection::PopulateSpawnSets( KeyValues *pKV )
{
	for ( KeyValues *pSpawnSetKV = pKV; pSpawnSetKV; pSpawnSetKV = pSpawnSetKV->GetNextTrueSubKey() )
	{
		bool bOverlay = false;

		if ( !Q_stricmp( pSpawnSetKV->GetName(), "SpawnSetOverlay" ) )
		{
			bOverlay = true;
		}
		else if ( Q_stricmp( pSpawnSetKV->GetName(), "SpawnSet" ) )
		{
			continue;
		}

		CASW_Spawn_Set *pSpawnSet = new CASW_Spawn_Set( pSpawnSetKV, bOverlay );
		if ( !pSpawnSet->MatchesLevel( gpGlobals->mapname ) )
		{
			delete pSpawnSet;
			continue;
		}

		m_SpawnSets.AddToTail( pSpawnSet );
	}
}

static const char *s_SpawnTypeNames[NUM_ASW_SPAWN_TYPES] =
{
	"HORDE",
	"WANDERER",
	"HORDE_WANDERER",
	"PRESPAWN",
	"PACK"
};

CASW_Spawn_Set::CASW_Spawn_Set( KeyValues * pKV, bool bOverlay )
{
	m_bOverlay = bOverlay;
	m_iszName = AllocPooledString( pKV->GetString( "Name", "(missing name)" ) );
	m_iszMap = AllocPooledString( pKV->GetString( "Map", "*" ) );
	m_iMinSkill = pKV->GetInt( "MinSkill", 1 );
	m_iMaxSkill = pKV->GetInt( "MaxSkill", 5 );

	FOR_EACH_TRUE_SUBKEY( pKV, pSpawnKV )
	{
		CSplitString szTypes( pSpawnKV->GetName(), "+" );

		FOR_EACH_VEC( szTypes, i )
		{
			bool bFound = false;
			for ( int j = 0; j < NUM_ASW_SPAWN_TYPES; j++ )
			{
				if ( !Q_stricmp( s_SpawnTypeNames[j], szTypes[i] ) )
				{
					bFound = true;

					CASW_Spawn_Definition *pSpawn = new CASW_Spawn_Definition( pSpawnKV );
					if ( pSpawn->m_flSelectionWeight <= 0 )
					{
						DevWarning( "Spawn set %s contains a %s spawn with an invalid weight %f!\n", STRING( m_iszName ), szTypes[i], pSpawn->m_flSelectionWeight );
						delete pSpawn;
						break;
					}

					m_SpawnDefinitions[j].AddToTail( pSpawn );
					break;
				}
			}
			if ( !bFound )
			{
				DevWarning( "Unknown spawn type %s in spawn set %s\n", szTypes[i], STRING( m_iszName ) );
			}
		}
	}

#define READ_MIN_MAX(name, type) \
	m_iMin ## name = pKV->GetInt( "Min" #name ); \
	m_iMax ## name = pKV->GetInt( "Max" #name ); \
	m_bHas ## name = !bOverlay || m_iMax ## name != 0 || pKV->GetInt( "Max" #name, -1 ) != -1; \
	if ( bOverlay && ( ( m_bHas ## name && pKV->GetInt( "Min" #name, -1 ) != m_iMin ## name ) || ( !m_bHas ## name && pKV->GetInt( "Min" #name, -1 ) == m_iMin ## name ) ) ) \
	{ \
		Warning( "SpawnSetOverlay %s must specify both Min" #name " and Max" #name " or neither\n", STRING( m_iszName ) ); \
		m_bHas ## name = false; \
		m_iMin ## name = m_iMax ## name = 0; \
	} \
	if ( m_iMin ## name < 0 ) \
	{ \
		m_iMin ## name = 0; \
	} \
	if ( m_iMax ## name < m_iMin ## name ) \
	{ \
		m_iMax ## name = m_iMin ## name; \
	} \
	if ( !bOverlay && m_SpawnDefinitions[ ASW_SPAWN_TYPE_ ## type ].Count() == 0 && m_iMax ## name != 0 ) \
	{ \
		Warning( "Missing " #type " spawn definitions in %s\n", STRING( m_iszName ) ); \
		m_iMin ## name = m_iMax ## name = 0; \
	}

	READ_MIN_MAX( HordeSize, HORDE );
	READ_MIN_MAX( Wanderers, WANDERER );
	READ_MIN_MAX( HordeWanderers, HORDE_WANDERER );
	READ_MIN_MAX( Prespawn, PRESPAWN );
	READ_MIN_MAX( Packs, PACK );

#undef READ_MIN_MAX
}

bool CASW_Spawn_Set::MatchesLevel( string_t iszLevelName ) const
{
	return EntityNamesMatch( STRING( m_iszMap ), iszLevelName );
}

void CASW_Spawn_Set::ApplyOverlay( CASW_Spawn_Set *pTarget )
{
	Assert( m_bOverlay );
	Assert( !pTarget->m_bOverlay );

	pTarget->m_iszAppliedOverlays.AddToTail( m_iszName );

#define COPY_OVER( name, type ) \
	if ( m_bHas ## name ) \
	{ \
		if ( m_iMax ## name != 0 && pTarget->m_SpawnDefinitions[type].Count() == 0 && m_SpawnDefinitions[type].Count() == 0 ) \
		{ \
			Warning( "Spawn set overlay %s cannot set " #name " on spawn set %s because neither have any %s definitions!\n", STRING( m_iszName ), STRING( pTarget->m_iszName ), s_SpawnTypeNames[type] ); \
		} \
		else \
		{ \
			pTarget->m_iMin ## name = m_iMin ## name; \
			pTarget->m_iMax ## name = m_iMax ## name; \
		} \
	} \
	FOR_EACH_VEC( m_SpawnDefinitions[type], i ) \
	{ \
		pTarget->m_SpawnDefinitions[type].AddToTail( new CASW_Spawn_Definition( *m_SpawnDefinitions[type][i] ) ); \
	}
	
	COPY_OVER( HordeSize, ASW_SPAWN_TYPE_HORDE );
	COPY_OVER( Wanderers, ASW_SPAWN_TYPE_WANDERER );
	COPY_OVER( HordeWanderers, ASW_SPAWN_TYPE_HORDE_WANDERER );
	COPY_OVER( Prespawn, ASW_SPAWN_TYPE_PRESPAWN );
	COPY_OVER( Packs, ASW_SPAWN_TYPE_PACK );

#undef COPY_OVER
}

void CASW_Spawn_Set::Dump()
{
	CmdMsg( "SpawnSet name: %s\n", STRING( m_iszName ) );
	FOR_EACH_VEC( m_iszAppliedOverlays, i )
	{
		CmdMsg( "SpawnSetOverlay name: %s\n", STRING( m_iszAppliedOverlays[i] ) );
	}
	CmdMsg( "Map filter: %s\n", STRING( m_iszMap ) );
	CmdMsg( "Minimum skill level: %d\n", m_iMinSkill );
	CmdMsg( "Maximum skill level: %d\n", m_iMaxSkill );

#define DUMP_DEFINITIONS(name, type) \
	if ( m_iMax ## name > 0 ) \
	{ \
		CmdMsg( "\n" ); \
		CmdMsg( #type ":\n" ); \
		CmdMsg( "  Minimum " #name ": %d\n", m_iMin ## name ); \
		CmdMsg( "  Maximum " #name ": %d\n", m_iMax ## name ); \
		FOR_EACH_VEC( m_SpawnDefinitions[ ASW_SPAWN_TYPE_ ## type ], i ) \
		{ \
			m_SpawnDefinitions[ ASW_SPAWN_TYPE_ ## type ][i]->Dump( ComputeTotalWeight( ASW_SPAWN_TYPE_ ## type ) );  \
		} \
	}

	DUMP_DEFINITIONS( HordeSize, HORDE );
	DUMP_DEFINITIONS( Wanderers, WANDERER );
	DUMP_DEFINITIONS( HordeWanderers, HORDE_WANDERER );
	DUMP_DEFINITIONS( Prespawn, PRESPAWN );
	DUMP_DEFINITIONS( Packs, PACK );

#undef DUMP_DEFINITIONS
}

float CASW_Spawn_Set::ComputeTotalWeight( ASW_Spawn_Type iSpawnType ) const
{
	float flTotal = 0.0f;
	FOR_EACH_VEC( m_SpawnDefinitions[iSpawnType], i )
	{
		CASW_Spawn_Definition *pSpawn = m_SpawnDefinitions[iSpawnType][i];
		if ( pSpawn->m_Requirement.CanSpawn() )
		{
			flTotal += pSpawn->m_flSelectionWeight;
		}
	}
	return flTotal;
}

CASW_Spawn_Requirement::CASW_Spawn_Requirement( KeyValues *pKV )
{
	if ( !pKV )
		return;

	for ( KeyValues *pChildKV = pKV->GetFirstValue(); pChildKV; pChildKV = pChildKV->GetNextValue() )
	{
		if ( !Q_stricmp( pChildKV->GetName(), "RequireCVar" ) )
		{
			ConVar *pCVar = g_pCVar->FindVar( pChildKV->GetString() );
			if ( !pCVar )
			{
				Warning( "Unknown convar %s in spawn selection\n", pChildKV->GetString() );
				continue;
			}
			m_pRequireCVar.AddToTail( pCVar );
		}
		else if ( !Q_stricmp( pChildKV->GetName(), "RequireGlobal" ) )
		{
			if ( !Q_strnicmp( pChildKV->GetString(), "OFF:", strlen( "OFF:" ) ) )
			{
				const char *pszGlobalName = pChildKV->GetString() + strlen( "OFF:" );
				if ( m_RequireGlobalState.Defined( pszGlobalName ) )
				{
					Warning( "Ignoring extra global state %s in spawn selection\n", pChildKV->GetString() );
					continue;
				}
				m_RequireGlobalState[pszGlobalName] = GLOBAL_OFF;
			}
			else if ( !Q_strnicmp( pChildKV->GetString(), "ON:", strlen( "ON:" ) ) )
			{
				const char *pszGlobalName = pChildKV->GetString() + strlen( "ON:" );
				if ( m_RequireGlobalState.Defined( pszGlobalName ) )
				{
					Warning( "Ignoring extra global state %s in spawn selection\n", pChildKV->GetString() );
					continue;
				}
				m_RequireGlobalState[pszGlobalName] = GLOBAL_ON;
			}
			else if ( !Q_strnicmp( pChildKV->GetString(), "DEAD:", strlen( "DEAD:" ) ) )
			{
				const char *pszGlobalName = pChildKV->GetString() + strlen( "DEAD:" );
				if ( m_RequireGlobalState.Defined( pszGlobalName ) )
				{
					Warning( "Ignoring extra global state %s in spawn selection\n", pChildKV->GetString() );
					continue;
				}
				m_RequireGlobalState[pszGlobalName] = GLOBAL_DEAD;
			}
			else if ( !Q_strnicmp( pChildKV->GetString(), "MIN:", strlen( "MIN:" ) ) )
			{
				char *pszEnd = NULL;
				errno = 0;
				long int iValue = strtol( pChildKV->GetString() + strlen( "MIN:" ), &pszEnd, 10 );
				if ( errno != 0 || iValue < INT_MIN || iValue > INT_MAX || pszEnd == NULL || *pszEnd != ':' )
				{
					Warning( "Invalid RequireGlobal: expecting the format MIN:[number]:[name], but got %s\n", pChildKV->GetString() );
					continue;
				}
				const char *pszGlobalName = pszEnd + 1;
				if ( m_RequireGlobalMin.Defined( pszGlobalName ) )
				{
					Warning( "Ignoring extra global minimum %s in spawn selection\n", pChildKV->GetString() );
					continue;
				}
				if ( m_RequireGlobalMax.Defined( pszGlobalName ) && m_RequireGlobalMax[pszGlobalName] < (int) iValue )
				{
					Warning( "RequireGlobal for %s has a range of [%d, %d], which means this NPC will never spawn\n", pszGlobalName, (int) iValue, m_RequireGlobalMax[pszGlobalName] );
				}
				m_RequireGlobalMin[pszGlobalName] = (int) iValue;
			}
			else if ( !Q_strnicmp( pChildKV->GetString(), "MAX:", strlen( "MAX:" ) ) )
			{
				char *pszEnd = NULL;
				errno = 0;
				long int iValue = strtol( pChildKV->GetString() + strlen( "MAX:" ), &pszEnd, 10 );
				if ( errno != 0 || iValue < INT_MIN || iValue > INT_MAX || pszEnd == NULL || *pszEnd != ':' )
				{
					Warning( "Invalid RequireGlobal: expecting the format MAX:[number]:[name], but got %s\n", pChildKV->GetString() );
					continue;
				}
				const char *pszGlobalName = pszEnd + 1;
				if ( m_RequireGlobalMax.Defined( pszGlobalName ) )
				{
					Warning( "Ignoring extra global maximum %s in spawn selection\n", pChildKV->GetString() );
					continue;
				}
				if ( m_RequireGlobalMin.Defined( pszGlobalName ) && m_RequireGlobalMin[pszGlobalName] > (int) iValue )
				{
					Warning( "RequireGlobal for %s has a range of [%d, %d], which means this NPC will never spawn\n", pszGlobalName, m_RequireGlobalMin[pszGlobalName], (int) iValue );
				}
				m_RequireGlobalMax[pszGlobalName] = (int) iValue;
			}
			else
			{
				Warning( "Invalid RequireGlobal format: %s\n", pChildKV->GetString() );
			}
		}
		else if ( !Q_stricmp( pChildKV->GetName(), "BeforeObjective" ) )
		{
			if ( m_WantObjective.Defined( pChildKV->GetString() ) )
			{
				if ( m_WantObjective[pChildKV->GetString()] )
				{
					Warning( "Ignoring BeforeObjective %s because we already have AfterObjective %s\n", pChildKV->GetString(), pChildKV->GetString() );
				}
				else
				{
					Warning( "Ignoring BeforeObjective %s because we already have BeforeObjective %s\n", pChildKV->GetString(), pChildKV->GetString() );
				}
				continue;
			}

			m_WantObjective[pChildKV->GetString()] = false;
		}
		else if ( !Q_stricmp( pChildKV->GetName(), "AfterObjective" ) )
		{
			if ( m_WantObjective.Defined( pChildKV->GetString() ) )
			{
				if ( m_WantObjective[pChildKV->GetString()] )
				{
					Warning( "Ignoring AfterObjective %s because we already have AfterObjective %s\n", pChildKV->GetString(), pChildKV->GetString() );
				}
				else
				{
					Warning( "Ignoring AfterObjective %s because we already have BeforeObjective %s\n", pChildKV->GetString(), pChildKV->GetString() );
				}
				continue;
			}

			m_WantObjective[pChildKV->GetString()] = true;
		}
		else if ( !Q_stricmp( pChildKV->GetName(), "SpawnerSpawning" ) )
		{
			if ( m_WantSpawner.Defined( pChildKV->GetString() ) )
			{
				if ( m_WantSpawner[pChildKV->GetString()] )
				{
					Warning( "Ignoring SpawnerSpawning %s because we already have SpawnerSpawning %s\n", pChildKV->GetString(), pChildKV->GetString() );
				}
				else
				{
					Warning( "Ignoring SpawnerSpawning %s because we already have SpawnerWaiting %s\n", pChildKV->GetString(), pChildKV->GetString() );
				}
				continue;
			}

			m_WantSpawner[pChildKV->GetString()] = true;
		}
		else if ( !Q_stricmp( pChildKV->GetName(), "SpawnerWaiting" ) )
		{
			if ( m_WantSpawner.Defined( pChildKV->GetString() ) )
			{
				if ( m_WantSpawner[pChildKV->GetString()] )
				{
					Warning( "Ignoring SpawnerWaiting %s because we already have SpawnerSpawning %s\n", pChildKV->GetString(), pChildKV->GetString() );
				}
				else
				{
					Warning( "Ignoring SpawnerWaiting %s because we already have SpawnerWaiting %s\n", pChildKV->GetString(), pChildKV->GetString() );
				}
				continue;
			}

			m_WantSpawner[pChildKV->GetString()] = false;
		}
	}
}

CASW_Spawn_Requirement::CASW_Spawn_Requirement(const CASW_Spawn_Requirement & req)
{
	m_pRequireCVar.AddVectorToTail( req.m_pRequireCVar );
	for ( int i = 0; i < req.m_RequireGlobalState.GetNumStrings(); i++ )
	{
		m_RequireGlobalState[req.m_RequireGlobalState.String( i )] = req.m_RequireGlobalState[i];
	}
	for ( int i = 0; i < req.m_RequireGlobalMin.GetNumStrings(); i++ )
	{
		m_RequireGlobalMin[req.m_RequireGlobalMin.String( i )] = req.m_RequireGlobalMin[i];
	}
	for ( int i = 0; i < req.m_RequireGlobalMax.GetNumStrings(); i++ )
	{
		m_RequireGlobalMax[req.m_RequireGlobalMax.String( i )] = req.m_RequireGlobalMax[i];
	}
	for ( int i = 0; i < req.m_WantObjective.GetNumStrings(); i++ )
	{
		m_WantObjective[req.m_WantObjective.String( i )] = req.m_WantObjective[i];
	}
	for ( int i = 0; i < req.m_WantSpawner.GetNumStrings(); i++ )
	{
		m_WantSpawner[req.m_WantSpawner.String( i )] = req.m_WantSpawner[i];
	}
}

static const char *s_pszGlobalStates[] = {
	"OFF", // GLOBAL_OFF
	"ON",  // GLOBAL_ON
	"DEAD" // GLOBAL_DEAD
};

void CASW_Spawn_Requirement::Dump( const char *szIndent )
{
	FOR_EACH_VEC( m_pRequireCVar, i )
	{
		CmdMsg( "      Required cvar: %s (currently %s)\n", m_pRequireCVar[i]->GetName(), m_pRequireCVar[i]->GetBool() ? "true" : "false" );
	}
	for ( int i = 0; i < m_RequireGlobalState.GetNumStrings(); i++ )
	{
		CmdMsg( "      Required global state: %s %s (currently %s)\n", m_RequireGlobalState.String( i ), s_pszGlobalStates[m_RequireGlobalState[i]], s_pszGlobalStates[GlobalEntity_GetState( m_RequireGlobalState.String( i ) )] );
	}
	for ( int i = 0; i < m_RequireGlobalMin.GetNumStrings(); i++ )
	{
		UtlSymId_t j = m_RequireGlobalMax.Find( m_RequireGlobalMin.String( i ) );
		if ( j != UTL_INVAL_SYMBOL )
		{
			CmdMsg( "      Required global counter: %s between %d and %d (currently %d)\n", m_RequireGlobalMin.String( i ), m_RequireGlobalMin[i], m_RequireGlobalMax[j], GlobalEntity_GetCounter( m_RequireGlobalMin.String( i ) ) );
		}
		else
		{
			CmdMsg( "      Required global counter: %s at least %d (currently %d)\n", m_RequireGlobalMin.String( i ), m_RequireGlobalMin[i], GlobalEntity_GetCounter( m_RequireGlobalMin.String( i ) ) );
		}
	}
	for ( int i = 0; i < m_RequireGlobalMax.GetNumStrings(); i++ )
	{
		if ( !m_RequireGlobalMin.Defined( m_RequireGlobalMax.String( i ) ) )
		{
			CmdMsg( "      Required global counter: %s no more than %d (currently %d)\n", m_RequireGlobalMax.String( i ), m_RequireGlobalMax[i], GlobalEntity_GetCounter( m_RequireGlobalMax.String( i ) ) );
		}
	}
	for ( int i = 0; i < m_WantObjective.GetNumStrings(); i++ )
	{
		const char *pszCurrentState = "unknown, assuming incomplete";
		CASW_Objective *pObj = dynamic_cast<CASW_Objective *>( gEntList.FindEntityByName( NULL, m_WantObjective.String( i ) ) );
		if ( pObj )
		{
			pszCurrentState = pObj->IsObjectiveComplete() ? "complete" : "incomplete";
		}
		CmdMsg( "      Required objective state: %s must be %s (currently %s)\n", m_WantObjective.String( i ), m_WantObjective[i] ? "complete" : "incomplete", pszCurrentState );
	}
	for ( int i = 0; i < m_WantSpawner.GetNumStrings(); i++ )
	{
		const char *pszCurrentState = "unknown, assuming waiting";
		CASW_Spawner *pSpawner = dynamic_cast<CASW_Spawner *>( gEntList.FindEntityByName( NULL, m_WantSpawner.String( i ) ) );
		if ( pSpawner )
		{
			pszCurrentState = pSpawner->WasAllowedToSpawn() ? "spawning" : "waiting";
		}
		CmdMsg( "      Required spawner state: %s must be %s (currently %s)\n", m_WantSpawner.String( i ), m_WantSpawner[i] ? "spawning" : "waiting", pszCurrentState );
	}
}

bool CASW_Spawn_Requirement::CanSpawn() const
{
	FOR_EACH_VEC( m_pRequireCVar, i )
	{
		if ( !m_pRequireCVar[i]->GetBool() )
		{
			return false;
		}
	}
	for ( int i = 0; i < m_RequireGlobalState.GetNumStrings(); i++ )
	{
		if ( GlobalEntity_GetState( m_RequireGlobalState.String( i ) ) != m_RequireGlobalState[i] )
		{
			return false;
		}
	}
	for ( int i = 0; i < m_RequireGlobalMin.GetNumStrings(); i++ )
	{
		if ( GlobalEntity_GetCounter( m_RequireGlobalMin.String( i ) ) < m_RequireGlobalMin[i] )
		{
			return false;
		}
	}
	for ( int i = 0; i < m_RequireGlobalMax.GetNumStrings(); i++ )
	{
		if ( GlobalEntity_GetCounter( m_RequireGlobalMax.String( i ) ) > m_RequireGlobalMax[i] )
		{
			return false;
		}
	}
	for ( int i = 0; i < m_WantObjective.GetNumStrings(); i++ )
	{
		CASW_Objective *pObj = dynamic_cast<CASW_Objective *>( gEntList.FindEntityByName( NULL, m_WantObjective.String( i ) ) );
		if ( !pObj && m_WantObjective[i] )
		{
			return false;
		}
		if ( pObj && pObj->IsObjectiveComplete() != m_WantObjective[i] )
		{
			return false;
		}
	}
	for ( int i = 0; i < m_WantSpawner.GetNumStrings(); i++ )
	{
		CASW_Spawner *pSpawner = dynamic_cast<CASW_Spawner *>( gEntList.FindEntityByName( NULL, m_WantSpawner.String( i ) ) );
		if ( !pSpawner && m_WantSpawner[i] )
		{
			return false;
		}
		if ( pSpawner && pSpawner->WasAllowedToSpawn() != m_WantSpawner[i] )
		{
			return false;
		}
	}
	return true;
}

CASW_Spawn_Definition::CASW_Spawn_Definition( KeyValues *pKV ) : m_Requirement( pKV )
{
	if ( !pKV )
	{
		m_flSelectionWeight = 1.0f;
		return;
	}

	m_flSelectionWeight = pKV->GetFloat( "SelectionWeight", 1.0f );

	for ( KeyValues *pNPCKV = pKV->GetFirstTrueSubKey(); pNPCKV; pNPCKV = pNPCKV->GetNextTrueSubKey() )
	{
		if ( !Q_stricmp( pNPCKV->GetName(), "NPC" ) )
		{
			CASW_Spawn_NPC *pNPC = new CASW_Spawn_NPC( pNPCKV );
			if ( !pNPC->m_pAlienClass )
			{
				delete pNPC;
				continue;
			}

			m_NPCs.AddToTail( pNPC );
		}
	}

	if ( m_NPCs.Count() == 0 )
	{
		Warning( "Spawn definition with no NPCs\n" );
	}
}

CASW_Spawn_Definition::CASW_Spawn_Definition( const CASW_Spawn_Definition & def ) : m_Requirement( def.m_Requirement )
{
	m_flSelectionWeight = def.m_flSelectionWeight;
	m_NPCs.EnsureCapacity( def.m_NPCs.Count() );

	FOR_EACH_VEC( def.m_NPCs, i )
	{
		m_NPCs.AddToTail( new CASW_Spawn_NPC( *def.m_NPCs[i] ) );
	}
}

void CASW_Spawn_Definition::Dump( float flTotalWeight )
{
	CmdMsg( "  %f%% chance (weight %f):\n", m_flSelectionWeight / flTotalWeight * 100, m_flSelectionWeight );

	m_Requirement.Dump( "    " );

	FOR_EACH_VEC( m_NPCs, i )
	{
		m_NPCs[i]->Dump();
	}
}

CASW_Spawn_NPC::CASW_Spawn_NPC( const char *szAlienClass ) : m_Requirement( NULL )
{
	m_pAlienClass = NULL;
	for ( int i = 0; i < ASWSpawnManager()->GetNumAlienClasses(); i++ )
	{
		const ASW_Alien_Class_Entry *pAlienClass = ASWSpawnManager()->GetAlienClass( i );
		if ( !Q_stricmp( szAlienClass, pAlienClass->m_pszAlienClass ) )
		{
			m_pAlienClass = pAlienClass;
			break;
		}
	}
	if ( !m_pAlienClass )
	{
		Warning( "Invalid alien class in spawn definitions: %s\n", szAlienClass );
	}

	m_iHealthBonus = 0;
	m_flSpeedScale = 1;
	m_flSizeScale = 1;
	m_bFlammable = true;
	m_bFreezable = true;
	m_flFreezeResistance = 0.0f;
	m_bTeslable = true;
	m_bFlinches = true;
	m_bGrenadeReflector = false;
	m_iszVScript = NULL_STRING;
	m_flSpawnChance = 1;
}

CASW_Spawn_NPC::CASW_Spawn_NPC( KeyValues *pKV ) : m_Requirement( pKV )
{
	m_pAlienClass = NULL;
	for ( int i = 0; i < ASWSpawnManager()->GetNumAlienClasses(); i++ )
	{
		const ASW_Alien_Class_Entry *pAlienClass = ASWSpawnManager()->GetAlienClass( i );
		if ( !Q_stricmp( pKV->GetString( "AlienClass" ), pAlienClass->m_pszAlienClass ) )
		{
			m_pAlienClass = pAlienClass;
			break;
		}
	}
	if ( !m_pAlienClass )
	{
		Warning( "Invalid alien class in spawn definitions: %s\n", pKV->GetString( "AlienClass" ) );
	}

	m_iHealthBonus = pKV->GetInt( "HealthBonus", 0 );
	if ( m_iHealthBonus < 0 )
	{
		m_iHealthBonus = 0;
	}
	m_flSpeedScale = pKV->GetFloat( "SpeedScale", 1.0f );
	if ( m_flSpeedScale <= 0 )
	{
		m_flSpeedScale = 1;
	}
	m_flSizeScale = pKV->GetFloat( "SizeScale", 1.0f );
	if ( m_flSizeScale <= 0 )
	{
		m_flSizeScale = 1;
	}
	m_bFlammable = pKV->GetBool( "Flammable", true );
	m_bFreezable = pKV->GetBool( "Freezable", true );
	m_flFreezeResistance = pKV->GetFloat( "FreezeResistance", 0.0f );
	m_bTeslable = pKV->GetBool( "Teslable", true );
	m_bFlinches = pKV->GetBool( "Flinches", true );
	m_bGrenadeReflector = pKV->GetBool("Reflector", false);

	if ( pKV->GetString( "VScript", NULL ) == NULL )
	{
		m_iszVScript = NULL_STRING;
	}
	else
	{
		m_iszVScript = AllocPooledString( pKV->GetString( "VScript" ) );
	}

	m_flSpawnChance = pKV->GetFloat( "SpawnChance", 1.0f );
	if ( m_flSpawnChance <= 0 )
	{
		m_flSpawnChance = 1;
	}
}

CASW_Spawn_NPC::CASW_Spawn_NPC( const CASW_Spawn_NPC & npc ) : m_Requirement( npc.m_Requirement )
{
	m_pAlienClass = npc.m_pAlienClass;
	m_iHealthBonus = npc.m_iHealthBonus;
	m_flSpeedScale = npc.m_flSpeedScale;
	m_flSizeScale = npc.m_flSizeScale;
	m_bFlammable = npc.m_bFlammable;
	m_bFreezable = npc.m_bFreezable;
	m_flFreezeResistance = npc.m_flFreezeResistance;
	m_bTeslable = npc.m_bTeslable;
	m_bFlinches = npc.m_bFlinches;
	m_bGrenadeReflector = npc.m_bGrenadeReflector;
	m_iszVScript = npc.m_iszVScript;
	m_flSpawnChance = npc.m_flSpawnChance;
}

void CASW_Spawn_NPC::Dump()
{
	CmdMsg( "    NPC: %s\n", m_pAlienClass->m_pszAlienClass );
	if ( m_flSpawnChance != 1 )
	{
		CmdMsg( "      Spawn chance: %f%%\n", m_flSpawnChance * 100 );
	}
	if ( m_iHealthBonus != 0 )
	{
		CmdMsg( "      Bonus health: %d\n", m_iHealthBonus );
	}
	if ( m_flSpeedScale != 1 )
	{
		CmdMsg( "      Speed: %+f%%", ( m_flSpeedScale - 1 ) * 100 );
	}
	if ( m_flSizeScale != 1 )
	{
		CmdMsg( "      Size: %+f%%", ( m_flSizeScale - 1 ) * 100 );
	}
	if ( m_flFreezeResistance != 0.0f )
	{
		CmdMsg( "      Freeze resistance: %f\n", m_flFreezeResistance );
	}
	if ( !m_bFlammable )
	{
		CmdMsg( "      Not flammable\n" );
	}
	if ( !m_bFreezable )
	{
		CmdMsg( "      Not freezable\n" );
	}
	if ( !m_bTeslable )
	{
		CmdMsg( "      Not teslable\n" );
	}
	if ( !m_bFlinches )
	{
		CmdMsg( "      Never flinches\n" );
	}
	if ( m_iszVScript != NULL_STRING )
	{
		CmdMsg( "      Run VScript: scripts/vscripts/%s.nut\n", STRING( m_iszVScript ) );
	}
	m_Requirement.Dump( "      " );
}

CON_COMMAND( rd_dump_alien_selection_set, "" )
{
	Assert( ASWSpawnSelection() );
	ASWSpawnSelection()->Dump();
}
