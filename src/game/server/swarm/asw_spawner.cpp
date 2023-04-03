#include "cbase.h"
#include "baseentity.h"
#include "asw_spawner.h"
#include "asw_marine.h"
#include "asw_gamerules.h"
#include "asw_marine_resource.h"
#include "asw_game_resource.h"
#include "entityapi.h"
#include "entityoutput.h"
#include "props.h"
#include "asw_alien.h"
#include "asw_buzzer.h"
#include "asw_director.h"
#include "asw_fail_advice.h"
#include "asw_spawn_manager.h"
#include "asw_spawn_selection.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( asw_spawner, CASW_Spawner );

//ConVar asw_uber_drone_chance("asw_uber_drone_chance", "0.25f", FCVAR_CHEAT, "Chance of an uber drone spawning when playing in uber mode");
extern ConVar asw_debug_spawners;
extern ConVar asw_drone_health;
ConVar asw_spawning_enabled( "asw_spawning_enabled", "1", FCVAR_CHEAT, "If set to 0, asw_spawners won't spawn aliens" );
ConVar rd_spawning_start_at_randomized_intervals( "rd_spawning_start_at_randomized_intervals", "0", FCVAR_CHEAT, "If set to 1, asw_spawners that start spawning aliens have a small random delay each before starting to spawn" );

BEGIN_DATADESC( CASW_Spawner )
	DEFINE_KEYFIELD( m_nMaxLiveAliens,			FIELD_INTEGER,	"MaxLiveAliens" ),
	DEFINE_KEYFIELD( m_nNumAliens,				FIELD_INTEGER,	"NumAliens" ),	
	DEFINE_KEYFIELD( m_bInfiniteAliens,			FIELD_BOOLEAN,	"InfiniteAliens" ),
	DEFINE_KEYFIELD( m_flSpawnInterval,			FIELD_FLOAT,	"SpawnInterval" ),
	DEFINE_KEYFIELD( m_flSpawnIntervalJitter,	FIELD_FLOAT,	"SpawnIntervalJitter" ),
	DEFINE_KEYFIELD( m_AlienClassNum,			FIELD_INTEGER,	"AlienClass" ),
	DEFINE_KEYFIELD( m_szAlienModelOverride,	FIELD_MODELNAME, "AlienModelOverride" ),
	DEFINE_KEYFIELD( m_SpawnerState,			FIELD_INTEGER,	"SpawnerState" ),
	DEFINE_KEYFIELD( m_flDirectorLockTime,		FIELD_FLOAT,	"DirectorLockTime" ),
	DEFINE_INPUT(    m_iAllowDirectorSpawns,	FIELD_INTEGER,	"AllowDirectorSpawns" ),

	DEFINE_INPUTFUNC( FIELD_VOID,	"SpawnOneAlien",	InputSpawnAlien ),
	DEFINE_INPUTFUNC( FIELD_VOID,	"StartSpawning",	InputStartSpawning ),
	DEFINE_INPUTFUNC( FIELD_VOID,	"StopSpawning",		InputStopSpawning ),
	DEFINE_INPUTFUNC( FIELD_VOID,	"ToggleSpawning",	InputToggleSpawning ),

	DEFINE_OUTPUT( m_OnDirectorSpawned, "OnDirectorSpawned" ),
	DEFINE_OUTPUT( m_OnAllSpawned,		"OnAllSpawned" ),
	DEFINE_OUTPUT( m_OnAllSpawnedDead,	"OnAllSpawnedDead" ),

	DEFINE_FIELD( m_nCurrentLiveAliens, FIELD_INTEGER ),
	DEFINE_FIELD( m_AlienClassName,		FIELD_STRING ),

	DEFINE_THINKFUNC( SpawnerThink ),
END_DATADESC()

CASW_Spawner::CASW_Spawner()
{
	m_hAlienOrderTarget = NULL;
	m_iAllowDirectorSpawns = 0;
	m_flDirectorLockTime = 4;
	m_flLastDirectorSpawn = -FLT_MAX;
	m_szAlienModelOverride = NULL_STRING;
}

CASW_Spawner::~CASW_Spawner()
{

}

void CASW_Spawner::InitAlienClassName()
{
	if ( m_AlienClassNum < 0 || m_AlienClassNum >= ASWSpawnManager()->GetNumAlienClasses() )
	{
		m_AlienClassNum = 0;
	}

	m_AlienClassName = ASWSpawnManager()->GetAlienClass( m_AlienClassNum )->m_iszAlienClass;
}

void CASW_Spawner::Spawn()
{
	InitAlienClassName();

	BaseClass::Spawn();
	
	m_flSpawnIntervalJitter /= 100.0f;
	m_flSpawnIntervalJitter = clamp<float>( m_flSpawnIntervalJitter, 0, 1 );

	SetSolid( SOLID_NONE );
	m_nCurrentLiveAliens = 0;

	// trigger any begin state stuff
	SetSpawnerState(m_SpawnerState);
}

void CASW_Spawner::Precache()
{
	BaseClass::Precache();

	InitAlienClassName();
	const char *pszNPCName = STRING( m_AlienClassName );
	if ( !pszNPCName || !pszNPCName[0] )
	{
		Warning("asw_spawner %s has no specified alien-to-spawn classname.\n", STRING(GetEntityName()) );
	}
	else
	{
		UTIL_PrecacheOther( pszNPCName );
	}

	if ( m_szAlienModelOverride != NULL_STRING )
	{
		PrecacheModel( STRING( m_szAlienModelOverride ) );
	}
}

IASW_Spawnable_NPC* CASW_Spawner::SpawnAlien( const char *szAlienClassName, const Vector &vecHullMins, const Vector &vecHullMaxs, CASW_Spawn_NPC *pDirectorNPC )
{
	IASW_Spawnable_NPC *pSpawnable = BaseClass::SpawnAlien( szAlienClassName, vecHullMins, vecHullMaxs, pDirectorNPC );
	if ( pSpawnable && pDirectorNPC )
	{
		m_OnDirectorSpawned.FireOutput( dynamic_cast<CBaseEntity *>( pSpawnable ), this );
		m_flLastDirectorSpawn = gpGlobals->curtime;
	}
	else if ( pSpawnable )
	{
		m_nCurrentLiveAliens++;

		if ( m_iAllowDirectorSpawns == 2 )
		{
			m_iAllowDirectorSpawns = 1;
		}

		if (!m_bInfiniteAliens)
		{
			m_nNumAliens--;
			if (m_nNumAliens <= 0)
			{
				SpawnedAllAliens();
			}
		}
		else
		{
			ASWFailAdvice()->OnAlienSpawnedInfinite();
		}
	}
	return pSpawnable;
}

bool CASW_Spawner::CanSpawn( const Vector &vecHullMins, const Vector &vecHullMaxs, CASW_Spawn_NPC *pDirectorNPC )
{
	if ( pDirectorNPC )
		return BaseClass::CanSpawn( vecHullMins, vecHullMaxs, pDirectorNPC );

	if ( !asw_spawning_enabled.GetBool() )
		return false;

	// too many alive already?
	if (m_nMaxLiveAliens>0 && m_nCurrentLiveAliens>=m_nMaxLiveAliens)
		return false;

	// have we run out?
	if (!m_bInfiniteAliens && m_nNumAliens<=0)
		return false;

	return BaseClass::CanSpawn( vecHullMins, vecHullMaxs );
}

void CASW_Spawner::DoDispatchSpawn( CBaseEntity *pEntity, CASW_Spawn_NPC *pDirectorNPC )
{
	if ( pDirectorNPC )
	{
		if ( pDirectorNPC->m_iszModelOverride != NULL_STRING )
		{
			pEntity->SetModelName( pDirectorNPC->m_iszModelOverride );
		}
	}
	else if ( m_szAlienModelOverride != NULL_STRING )
	{
		pEntity->SetModelName( m_szAlienModelOverride );
	}

	BaseClass::DoDispatchSpawn( pEntity, pDirectorNPC );
}

// called when we've spawned all the aliens we can,
//  spawner should go to sleep
void CASW_Spawner::SpawnedAllAliens()
{
	m_OnAllSpawned.FireOutput( this, this );

	SetSpawnerState(SST_Finished); // disables think functions and so on
}

void CASW_Spawner::AlienKilled( CBaseEntity *pVictim )
{
	BaseClass::AlienKilled( pVictim );

	m_nCurrentLiveAliens--;

	if (asw_debug_spawners.GetBool())
		Msg("%d AlienKilled NumLive = %d\n", entindex(), m_nCurrentLiveAliens );

	// If we're here, we're getting erroneous death messages from children we haven't created
	AssertMsg( m_nCurrentLiveAliens >= 0, "asw_spawner receiving child death notice but thinks has no children\n" );

	if ( m_nCurrentLiveAliens <= 0 )
	{
		// See if we've exhausted our supply of NPCs
		if (!m_bInfiniteAliens && m_nNumAliens <= 0 )
		{
			if (asw_debug_spawners.GetBool())
				Msg("%d OnAllSpawnedDead (%s)\n", entindex(), STRING(GetEntityName()));
			// Signal that all our children have been spawned and are now dead
			m_OnAllSpawnedDead.FireOutput( this, this );
		}
	}
}

static bool IgnoreBasePropHelper( IHandleEntity *pHandleEntity, int contentsMask )
{
	if ( dynamic_cast< CBaseProp * >( pHandleEntity ) )
	{
		return false;
	}

	return true;
}

// mission started
void CASW_Spawner::MissionStart()
{
	if (asw_debug_spawners.GetBool())
		Msg("Spawner mission start, always inf=%d infinitealiens=%d\n", HasSpawnFlags( ASW_SF_ALWAYS_INFINITE ), m_bInfiniteAliens );
	// remove infinite spawns on easy mode
	if ( !HasSpawnFlags( ASW_SF_ALWAYS_INFINITE ) && ASWGameRules() && ASWGameRules()->GetSkillLevel() == 1
			&& m_bInfiniteAliens)
	{
		m_bInfiniteAliens = false;
		if (m_nNumAliens < 8)
			m_nNumAliens = 8;

		if (asw_debug_spawners.GetBool())
			Msg("  removed infinite and set num aliens to %d\n", m_nNumAliens);
	}

	if (m_SpawnerState == SST_StartSpawningWhenMissionStart)
		SetSpawnerState(SST_Spawning);

	if ( developer.GetBool() )
	{
		const Vector &hullMins = g_Aliens[m_AlienClassNum].m_vecRealHullMins;
		const Vector &hullMaxs = g_Aliens[m_AlienClassNum].m_vecRealHullMaxs;
		Vector traceStart = GetAbsOrigin() + Vector( 0, 0, 16.0f );
		Vector traceEnd = GetAbsOrigin() + Vector( 0, 0, 1.0f / 16.0f );
		CTraceFilterSimple filter( this, ASW_COLLISION_GROUP_ALIEN, &IgnoreBasePropHelper );
		trace_t tr;
		UTIL_TraceHull( traceStart, traceEnd, hullMins, hullMaxs, MASK_NPCSOLID, &filter, &tr );
		if ( tr.fraction < 1.0f )
		{
			if ( developer.GetInt() >= 3 )
			{
				debugoverlay->AddBoxOverlay2( GetAbsOrigin(), hullMins, hullMaxs, vec3_angle, Color{32, 64, 32, 64}, Color{128, 255, 128, 255}, 300.0f);
				debugoverlay->AddBoxOverlay2( tr.endpos, hullMins, hullMaxs, vec3_angle, Color{ 64, 32, 32, 64 }, Color{ 255, 128, 128, 255 }, 300.0f );
			}
			DevWarning( "Spawner %s at %f %f %f will spawn %s inside floor (%s). Recommendation: Raise spawner by %d units.\n", GetEntityName() == NULL_STRING ? "(unnamed)" : GetEntityNameAsCStr(), VectorExpand( GetAbsOrigin() ), STRING( m_AlienClassName ), tr.m_pEnt ? tr.m_pEnt->GetDebugName() : "no ent", Ceil2Int( tr.endpos.z - traceEnd.z ) );
		}
	}
}

void CASW_Spawner::SetSpawnerState(SpawnerState_t newState)
{
	m_SpawnerState = newState;

	// begin state stuff
	if (m_SpawnerState == SST_Spawning)
	{
		SetThink ( &CASW_Spawner::SpawnerThink );
		if ( rd_spawning_start_at_randomized_intervals.GetInt() )
		{
			// calculate jitter
			const float fInterval = random->RandomFloat( 0.0f, 1.0f );
			SetNextThink( gpGlobals->curtime + fInterval );
		}
		else
		{
			SetNextThink( gpGlobals->curtime );
		}
	}
	else if (m_SpawnerState == SST_Finished)
	{
		// Disable this forever.  Don't kill it because it still gets death notices
		SetThink( NULL );
		SetUse( NULL );
	}
	else if (m_SpawnerState == SST_WaitForInputs)
	{
		SetThink( NULL );	// stop thinking
	}
}

void CASW_Spawner::SpawnerThink()
{	
	// calculate jitter
	float fInterval = random->RandomFloat(1.0f - m_flSpawnIntervalJitter, 1.0f + m_flSpawnIntervalJitter) * m_flSpawnInterval;
	SetNextThink( gpGlobals->curtime + fInterval );

	if ( ASWDirector() && ASWDirector()->CanSpawnAlien( this ) )
	{
		SpawnAlien( STRING( m_AlienClassName ), GetAlienMins(), GetAlienMaxs() );
	}
}

// =====================
// Inputs
// =====================

void CASW_Spawner::SpawnOneAlien()
{
	SpawnAlien( STRING( m_AlienClassName ), GetAlienMins(), GetAlienMaxs() );
}

void CASW_Spawner::InputSpawnAlien( inputdata_t &inputdata )
{
	if (m_SpawnerState == SST_WaitForInputs)
	{
		if ( ASWDirector() && ASWDirector()->CanSpawnAlien( this ) )
		{
			SpawnOneAlien();
		}
	}
}

void CASW_Spawner::InputStartSpawning( inputdata_t &inputdata )
{
	if (m_SpawnerState == SST_WaitForInputs)
		SetSpawnerState(SST_Spawning);
}

void CASW_Spawner::InputStopSpawning( inputdata_t &inputdata )
{
	if (m_SpawnerState == SST_Spawning)
		SetSpawnerState(SST_WaitForInputs);
}

void CASW_Spawner::InputToggleSpawning( inputdata_t &inputdata )
{
	if (m_SpawnerState == SST_Spawning)
		SetSpawnerState(SST_WaitForInputs);
	else if (m_SpawnerState == SST_WaitForInputs)
		SetSpawnerState(SST_Spawning);
}

const Vector& CASW_Spawner::GetAlienMins()
{
	return NAI_Hull::Mins( ASWSpawnManager()->GetAlienClass( m_AlienClassNum )->m_nHullType );
}

const Vector& CASW_Spawner::GetAlienMaxs()
{
	return NAI_Hull::Maxs( ASWSpawnManager()->GetAlienClass( m_AlienClassNum )->m_nHullType );
}

bool CASW_Spawner::ApplyCarnageMode( float fScaler, float fInvScaler )
{
	if ( m_AlienClassNum == g_nDroneClassEntry ||  m_AlienClassNum == g_nDroneJumperClassEntry )
	{
		float flNumAliens = m_nNumAliens * fScaler;
		float flMaxLiveAliens = m_nMaxLiveAliens * fScaler;

		// fractional part is randomly rounded - 1.25 rounds down to 1 75% of the time and up to 2 25% of the time.
		float flNumAliensPartial = fmodf( flNumAliens, 1 );
		if ( flNumAliensPartial != 0 )
		{
			flNumAliens += RandomFloat() < flNumAliensPartial ? 1 : 0;
		}
		float flMaxLiveAliensPartial = fmodf( flMaxLiveAliens, 1 );
		if ( flMaxLiveAliensPartial != 0 )
		{
			flMaxLiveAliens += RandomFloat() < flMaxLiveAliensPartial ? 1 : 0;
		}

		m_nNumAliens = flNumAliens;
		m_nMaxLiveAliens = flMaxLiveAliens;
		m_flSpawnInterval *= fInvScaler;

		return true;
	}	

	return false;
}

void CASW_Spawner::SetInfinitelySpawnAliens(bool spawn_infinitely /*= true */)
{
	m_bInfiniteAliens = spawn_infinitely;
}


int	CASW_Spawner::DrawDebugTextOverlays()
{
	int text_offset = BaseClass::DrawDebugTextOverlays();

	if (m_debugOverlays & OVERLAY_TEXT_BIT)
	{
		NDebugOverlay::EntityText( entindex(),text_offset,CFmtStr( "Num Live Aliens: %d", m_nCurrentLiveAliens ),0 );
		text_offset++;
		NDebugOverlay::EntityText( entindex(),text_offset,CFmtStr( "Max Live Aliens: %d", m_nMaxLiveAliens ),0 );
		text_offset++;
		NDebugOverlay::EntityText( entindex(),text_offset,CFmtStr( "Alien supply: %d", m_bInfiniteAliens ? -1 : m_nNumAliens ),0 );
		text_offset++;
	}
	return text_offset;
}


void ASW_ApplyCarnage_f(float fScaler)
{
	if ( fScaler <= 0 )
		fScaler = 1.0f;

	float fInvScaler = 1.0f / fScaler;

	//int iNewHealth = fInvScaler * 80.0f;	// note: boosted health a bit here so this mode is harder than normal
	//asw_drone_health.SetValue(iNewHealth);

	CBaseEntity* pEntity = NULL;
	int iSpawnersChanged = 0;
	while ((pEntity = gEntList.FindEntityByClassname( pEntity, "asw_spawner" )) != NULL)
	{
		CASW_Spawner* pSpawner = dynamic_cast<CASW_Spawner*>(pEntity);			
		if ( pSpawner && !pSpawner->HasSpawnFlags( ASW_SF_NO_CARNAGE ) )
		{
			if ( pSpawner->ApplyCarnageMode( fScaler, fInvScaler ) )
			{
				iSpawnersChanged++;
			}
		}
	}
}

void ASW_ApplyInfiniteSpawners_f(void)
{
	CBaseEntity* pEntity = NULL;
	int iSpawnersChanged = 0;
	while ((pEntity = gEntList.FindEntityByClassname(pEntity, "asw_spawner")) != NULL)
	{
		CASW_Spawner* pSpawner = dynamic_cast<CASW_Spawner*>(pEntity);
		if (pSpawner)
		{
			pSpawner->SetInfinitelySpawnAliens();
			iSpawnersChanged++;
		}
	}
}


void asw_carnage_f(const CCommand &args)
{
	if ( args.ArgC() < 2 )
	{
		Msg( "Please supply a scale\n" );
	}
	ASW_ApplyCarnage_f( atof( args[1] ) );
}

ConCommand asw_carnage( "asw_carnage", asw_carnage_f, "Scales the number of aliens each spawner will put out", FCVAR_CHEAT );
