#include "cbase.h"
#include "baseentity.h"
#include "asw_base_spawner.h"
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
#include "asw_spawn_selection.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar asw_debug_spawners("asw_debug_spawners", "0", FCVAR_CHEAT, "Displays debug messages for the asw_spawners");

BEGIN_DATADESC( CASW_Base_Spawner )
	DEFINE_KEYFIELD( m_bSpawnIfMarinesAreNear,	FIELD_BOOLEAN,	"SpawnIfMarinesAreNear" ),
	DEFINE_KEYFIELD( m_flNearDistance,			FIELD_FLOAT,	"NearDistance" ),
	DEFINE_KEYFIELD( m_AlienOrders,				FIELD_INTEGER,	"AlienOrders" ),	
	DEFINE_KEYFIELD( m_AlienOrderTargetName,	FIELD_STRING,	"AlienOrderTargetName" ),
	DEFINE_KEYFIELD( m_AlienName,				FIELD_STRING,	"AlienNameTag" ),
	DEFINE_KEYFIELD( m_bStartBurrowed,			FIELD_BOOLEAN,	"StartBurrowed" ),
	DEFINE_KEYFIELD( m_UnburrowIdleActivity,	FIELD_STRING,	"UnburrowIdleActivity" ),
	DEFINE_KEYFIELD( m_UnburrowActivity,		FIELD_STRING,	"UnburrowActivity" ),
	DEFINE_KEYFIELD( m_bCheckSpawnIsClear,		FIELD_BOOLEAN,	"ClearCheck" ),
	DEFINE_KEYFIELD( m_bLongRangeNPC,			FIELD_BOOLEAN,  "LongRange" ),
	DEFINE_KEYFIELD( m_iMinSkillLevel,	FIELD_INTEGER,	"MinSkillLevel" ),
	DEFINE_KEYFIELD( m_iMaxSkillLevel,	FIELD_INTEGER,	"MaxSkillLevel" ),

	DEFINE_KEYFIELD( m_bFlammableSp, FIELD_BOOLEAN, "flammablesp" ),
	DEFINE_KEYFIELD( m_bTeslableSp, FIELD_BOOLEAN, "teslablesp" ),
	DEFINE_KEYFIELD( m_bFreezableSp, FIELD_BOOLEAN, "freezablesp" ),
	DEFINE_KEYFIELD( m_flFreezeResistanceSp, FIELD_FLOAT, "freezeresistancesp" ),
	DEFINE_KEYFIELD( m_bFlinchableSp, FIELD_BOOLEAN, "flinchablesp" ),
	DEFINE_KEYFIELD( m_bGrenadeReflectorSp, FIELD_BOOLEAN, "reflectorsp"),
	DEFINE_KEYFIELD( m_iHealthBonusSp, FIELD_INTEGER, "healthbonussp"),
	DEFINE_KEYFIELD( m_fSizeScaleSp, FIELD_FLOAT, "sizescalesp" ),
	DEFINE_KEYFIELD( m_fSpeedScaleSp, FIELD_FLOAT, "speedscalesp" ),

	DEFINE_KEYFIELD( m_vecMinOffset, FIELD_VECTOR, "minoffset" ),
	DEFINE_KEYFIELD( m_vecMaxOffset, FIELD_VECTOR, "maxoffset" ),
	DEFINE_KEYFIELD( m_flYawOffset, FIELD_FLOAT, "yawoffset" ),

	DEFINE_KEYFIELD( m_iszAlienVScripts, FIELD_STRING, "alien_vscripts" ),
	DEFINE_KEYFIELD( m_iszAlienScriptThinkFunction, FIELD_STRING, "alien_thinkfunction" ),

	DEFINE_OUTPUT( m_OnSpawned,			"OnSpawned" ),

	DEFINE_FIELD( m_iMoveAsideCount, FIELD_INTEGER ),
	DEFINE_FIELD( m_hAlienOrderTarget, FIELD_EHANDLE ),
	DEFINE_FIELD( m_bEnabled, FIELD_BOOLEAN ),

	DEFINE_INPUTFUNC( FIELD_VOID,	"Enable",	InputEnable ),
	DEFINE_INPUTFUNC( FIELD_VOID,	"Disable",	InputDisable ),
	DEFINE_INPUTFUNC( FIELD_VOID,	"ToggleEnabled",		InputToggleEnabled ),
END_DATADESC()

CASW_Base_Spawner::CASW_Base_Spawner()
{
	m_hAlienOrderTarget = NULL;
	m_flLastSpawnTime = 0.0f;
	m_bEnabled = true;

	m_bFlammableSp = true;
	m_bTeslableSp = true;
	m_bFreezableSp = true;
	m_flFreezeResistanceSp = 0.0f;
	m_bFlinchableSp = true;
	m_bGrenadeReflectorSp = false;
	m_iHealthBonusSp = 0;
	m_fSizeScaleSp = 1.0f;
	m_fSpeedScaleSp = 1.0f;

	m_iszAlienVScripts = NULL_STRING;
	m_iszAlienScriptThinkFunction = NULL_STRING;

	m_vecMinOffset.Init();
	m_vecMaxOffset.Init();
	m_flYawOffset = 0.0f;
}

CASW_Base_Spawner::~CASW_Base_Spawner()
{

}

void CASW_Base_Spawner::Spawn()
{
	SetSolid( SOLID_NONE );
	m_iMoveAsideCount = 0;
	Precache();
	//SetModel( "models/editor/asw_spawner/asw_spawner.mdl" );
}

void CASW_Base_Spawner::Precache()
{
	BaseClass::Precache();

	PrecacheScriptSound( "Spawner.Horde" );
	PrecacheScriptSound( "Spawner.AreaClear" );
	//PrecacheModel( "models/editor/asw_spawner/asw_spawner.mdl" );
}

bool CASW_Base_Spawner::CanSpawn( const Vector &vecHullMins, const Vector &vecHullMaxs, CASW_Spawn_NPC *pDirectorNPC )
{
	if ( !m_bEnabled )
		return false;

	// is a marine too near?
	if ( !m_bSpawnIfMarinesAreNear && m_flNearDistance > 0 )
	{		
		CASW_Game_Resource* pGameResource = ASWGameResource();
		float distance = 0.0f;
		for ( int i = 0; i < ASW_MAX_MARINE_RESOURCES; i++ )
		{
			CASW_Marine_Resource* pMR = pGameResource->GetMarineResource(i);
			if ( pMR && pMR->GetMarineEntity() && pMR->GetMarineEntity()->GetHealth() > 0 )
			{
				distance = pMR->GetMarineEntity()->GetAbsOrigin().DistTo( m_vecCurrentSpawnPosition );
				if ( distance < m_flNearDistance )
				{
					if ( asw_debug_spawners.GetBool() )
					{
						Msg( "asw_spawner(%s): Alien can't spawn because a marine (%d) is %f away\n", GetEntityNameAsCStr(), i, distance );

						debugoverlay->AddLineOverlay( m_vecCurrentSpawnPosition, pMR->GetMarineEntity()->GetAbsOrigin(), 255, 0, 192, true, 1.0f );
					}
					return false;
				}
			}
		}
	}

	if ( asw_debug_spawners.GetBool() )
	{
		debugoverlay->AddBoxOverlay2( m_vecCurrentSpawnPosition, vecHullMins - Vector( 0, 0, 1 ), vecHullMaxs + Vector( 0, 0, 1 ), vec3_angle, Color( 0, 0, 0, 0 ), Color( 255, 255, 64, 255 ), 1.0f );
		debugoverlay->AddBoxOverlay2( m_vecCurrentSpawnPosition, Vector( -23, -23, 0 ), Vector( 23, 23, 0 ), vec3_angle, Color( 0, 0, 0, 0 ), Color( 192, 255, 0, 255 ), 1.0f );
	}

	Vector mins = m_vecCurrentSpawnPosition - Vector( 23, 23, 0 );
	Vector maxs = m_vecCurrentSpawnPosition + Vector( 23, 23, 0 );
	CBaseEntity *pList[128];
	int count = UTIL_EntitiesInBox( pList, 128, mins, maxs, FL_CLIENT|FL_NPC );
	if ( count )
	{
		//Iterate through the list and check the results
		for ( int i = 0; i < count; i++ )
		{
			//Don't build on top of another entity
			if ( pList[i] == NULL )
				continue;

			//If one of the entities is solid, then we may not be able to spawn now
			if ( ( pList[i]->GetSolidFlags() & FSOLID_NOT_SOLID ) == false )
			{
				// Since the outer method doesn't work well around striders on account of their huge bounding box.
				// Find the ground under me and see if a human hull would fit there.
				trace_t tr;
				UTIL_TraceHull( m_vecCurrentSpawnPosition + Vector( 0, 0, 1 ),
								m_vecCurrentSpawnPosition - Vector( 0, 0, 1 ),
								vecHullMins,
								vecHullMaxs,
								MASK_NPCSOLID,
								NULL,
								COLLISION_GROUP_NONE,
								&tr );

				if (tr.fraction < 1.0f && tr.DidHitNonWorldEntity())
				{
					// some non-world entity is blocking the spawn point, so don't spawn
					// riflemod: Cargo Elevator railing spawn fix for carnage
					// added && tr.m_pEnt->Classify() != CLASS_ASW_DRONE
					if (tr.m_pEnt && tr.m_pEnt->Classify() != CLASS_ASW_DRONE)
					{
						if ( m_iMoveAsideCount < 6 )	// don't send 'move aside' commands more than 5 times in a row, else you'll stop blocked NPCs going to sleep.
						{
							IASW_Spawnable_NPC *pSpawnable = dynamic_cast<IASW_Spawnable_NPC*>(tr.m_pEnt);
							if (pSpawnable)
							{
								pSpawnable->MoveAside();		// try and make him move aside
								m_iMoveAsideCount++;
							}
						}
						if (asw_debug_spawners.GetBool())
							Msg("asw_spawner(%s): Alien can't spawn because a non-world entity is blocking the spawn point: %s\n", GetEntityNameAsCStr(), tr.m_pEnt->GetClassname());
					}
					else
					{
						if (asw_debug_spawners.GetBool())
							Msg("asw_spawner(%s): Alien can't spawn because a non-world entity is blocking the spawn point.\n", GetEntityNameAsCStr());
					}

					if ( asw_debug_spawners.GetBool() && tr.m_pEnt && tr.m_pEnt->CollisionProp() )
					{
						debugoverlay->AddBoxOverlay2( tr.m_pEnt->CollisionProp()->GetCollisionOrigin(), tr.m_pEnt->CollisionProp()->OBBMins(), tr.m_pEnt->CollisionProp()->OBBMaxs(), tr.m_pEnt->CollisionProp()->GetCollisionAngles(), Color( 0, 0, 0, 0 ), Color( 255, 0, 0, 255 ), 1.0f );
					}

					return false;
				}

				if ( asw_debug_spawners.GetBool() && tr.m_pEnt && tr.m_pEnt->CollisionProp() )
				{
					debugoverlay->AddBoxOverlay2( tr.m_pEnt->CollisionProp()->GetCollisionOrigin(), tr.m_pEnt->CollisionProp()->OBBMins(), tr.m_pEnt->CollisionProp()->OBBMaxs(), tr.m_pEnt->CollisionProp()->GetCollisionAngles(), Color( 0, 0, 0, 0 ), Color( 0, 255, 0, 255 ), 1.0f );
				}
			}
		}
	}

	// is there something blocking the spawn point?
	if ( m_bCheckSpawnIsClear )
	{
		if ( asw_debug_spawners.GetBool() )
		{
			Msg("Checking spawn is clear...\n");
		}

		trace_t tr;
		UTIL_TraceHull( m_vecCurrentSpawnPosition,
					m_vecCurrentSpawnPosition + Vector( 0, 0, 1 ),
					vecHullMins,
					vecHullMaxs,
					MASK_NPCSOLID,
					this,
					COLLISION_GROUP_NONE,
					&tr );

		if( tr.fraction != 1.0 )
		{
			if ( asw_debug_spawners.GetBool() )
			{
				Msg( "asw_spawner(%s): Alien can't spawn because he wouldn't fit in the spawn point.\n", GetEntityNameAsCStr() );

				if ( tr.m_pEnt && tr.m_pEnt->CollisionProp() )
				{
					debugoverlay->AddBoxOverlay2( tr.m_pEnt->CollisionProp()->GetCollisionOrigin(), tr.m_pEnt->CollisionProp()->OBBMins(), tr.m_pEnt->CollisionProp()->OBBMaxs(), tr.m_pEnt->CollisionProp()->GetCollisionAngles(), Color( 0, 0, 0, 0 ), Color( 255, 0, 0, 255 ), 1.0f );
				}
			}
			// TODO: If we were trying to spawn an uber, change to spawning a regular instead
			return false;
		}
	}

	m_iMoveAsideCount = 0;
	return true;
}

void CASW_Base_Spawner::RemoveObstructingProps( CBaseEntity *pChild )
{
	// If I'm stuck inside any props, remove them
	bool bFound = true;
	while ( bFound )
	{
		trace_t tr;
		UTIL_TraceHull( pChild->GetAbsOrigin(), pChild->GetAbsOrigin(), pChild->WorldAlignMins(), pChild->WorldAlignMaxs(), MASK_NPCSOLID, pChild, COLLISION_GROUP_NONE, &tr );
		if (asw_debug_spawners.GetBool())
		{
			NDebugOverlay::Box( pChild->GetAbsOrigin(), pChild->WorldAlignMins(), pChild->WorldAlignMaxs(), 0, 255, 0, 32, 5.0 );
		}
		if ( tr.fraction != 1.0 && tr.m_pEnt )
		{
			if ( dynamic_cast<CBaseProp*>(tr.m_pEnt) )
			{
				// Set to non-solid so this loop doesn't keep finding it
				tr.m_pEnt->AddSolidFlags( FSOLID_NOT_SOLID );
				UTIL_RemoveImmediate( tr.m_pEnt );
				continue;
			}
		}

		bFound = false;
	}
}

CBaseEntity* CASW_Base_Spawner::GetOrderTarget()
{
	// find entity with name m_AlienOrderTargetName
	if (GetAlienOrderTarget() == NULL &&
		(m_AlienOrders == AOT_MoveTo || m_AlienOrders == AOT_MoveToIgnoringMarines )
		)
	{
		m_hAlienOrderTarget = gEntList.FindEntityByName( NULL, m_AlienOrderTargetName, NULL );

		if( !GetAlienOrderTarget() )
		{
			DevWarning("%s: asw_spawner can't find order object: %s\n", GetDebugName(), STRING(m_AlienOrderTargetName) );
			return NULL;
		}
	}
	return GetAlienOrderTarget();
}

IASW_Spawnable_NPC* CASW_Base_Spawner::SpawnAlien( const char *szAlienClassName, const Vector &vecHullMins, const Vector &vecHullMaxs, CASW_Spawn_NPC *pDirectorNPC )
{
	if ( !IsValidOnThisSkillLevel() )
	{
		UTIL_Remove(this);		// delete ourself if this spawner isn't valid on this difficulty level
		return NULL;
	}

	m_vecCurrentSpawnPosition = GetAbsOrigin();
	m_vecCurrentSpawnPosition.x += RandomFloat( m_vecMinOffset.x, m_vecMaxOffset.x );
	m_vecCurrentSpawnPosition.y += RandomFloat( m_vecMinOffset.y, m_vecMaxOffset.y );
	m_vecCurrentSpawnPosition.z += RandomFloat( m_vecMinOffset.z, m_vecMaxOffset.z );

	// reactivedrop: added + Vector(0, 0, vecHullMaxs.z - vecHullMins.z)
	// raise the position of spawned alien by it's hull because otherwise it
	// falls through displacement
	// make this workaround only for parasites, because other aliens seems to
	// spawn ok
	if ( !Q_strcmp( "asw_parasite", szAlienClassName ) )
	{
		m_vecCurrentSpawnPosition.z += vecHullMaxs.z - vecHullMins.z;
	}
	// Strip pitch and roll from the spawner's angles. Pass only yaw to the spawned NPC.
	m_angCurrentSpawnAngles.Init();
	m_angCurrentSpawnAngles.y = GetAbsAngles().y;
	m_angCurrentSpawnAngles.y += RandomFloat( -m_flYawOffset, m_flYawOffset );

	if ( !CanSpawn( vecHullMins, vecHullMaxs, pDirectorNPC ) )	// this may turn off m_bCurrentlySpawningUber if there's no room
		return NULL;

	CBaseEntity	*pEntity = CreateEntityByName( szAlienClassName );
	if ( !pEntity )
	{
		Msg( "Failed to spawn %s\n", szAlienClassName );
		return NULL;
	}

	if ( pEntity->IsAlienClassType() )
	{
		CASW_Alien* pAlien = assert_cast<CASW_Alien*>(pEntity);

		if (pDirectorNPC)
		{
			pAlien->m_bFlammable = pDirectorNPC->m_bFlammable;
			pAlien->m_bTeslable = pDirectorNPC->m_bTeslable;
			pAlien->m_bFreezable = pDirectorNPC->m_bFreezable;
			pAlien->m_flFreezeResistance = pDirectorNPC->m_flFreezeResistance;
			pAlien->m_bFlinchable = pDirectorNPC->m_bFlinches;
			pAlien->m_bGrenadeReflector = pDirectorNPC->m_bGrenadeReflector;
			pAlien->m_iHealthBonus = pDirectorNPC->m_iHealthBonus;
			pAlien->m_fSizeScale = pDirectorNPC->m_flSizeScale;
			pAlien->m_fSpeedScale = pDirectorNPC->m_flSpeedScale;
		}
		else
		{
			pAlien->m_bFlammable = m_bFlammableSp;
			pAlien->m_bTeslable = m_bTeslableSp;
			pAlien->m_bFreezable = m_bFreezableSp;
			pAlien->m_flFreezeResistance = m_flFreezeResistanceSp;
			pAlien->m_bFlinchable = m_bFlinchableSp;
			pAlien->m_bGrenadeReflector = m_bGrenadeReflectorSp;
			pAlien->m_iHealthBonus = m_iHealthBonusSp;
			pAlien->m_fSizeScale = m_fSizeScaleSp;
			pAlien->m_fSpeedScale = m_fSpeedScaleSp;
		}
	}

	if ( pEntity->Classify() == CLASS_ASW_BUZZER )
	{
		CASW_Buzzer* pBuzzer = assert_cast<CASW_Buzzer*>(pEntity);
		pBuzzer->m_bFlammable = m_bFlammableSp;
		pBuzzer->m_bTeslable = m_bTeslableSp;
		pBuzzer->m_bFreezable = m_bFreezableSp;
		pBuzzer->m_flFreezeResistance = m_flFreezeResistanceSp;
		pBuzzer->m_bFlinchable = m_bFlinchableSp;
		pBuzzer->m_bGrenadeReflector = m_bGrenadeReflectorSp;
		pBuzzer->m_iHealthBonus = m_iHealthBonusSp;
		pBuzzer->m_fSizeScale = m_fSizeScaleSp;
		pBuzzer->m_fSpeedScale = m_fSpeedScaleSp;
	}

	CAI_BaseNPC	*pNPC = pEntity->MyNPCPointer();
	if ( pNPC )
	{
		pNPC->AddSpawnFlags( SF_NPC_FALL_TO_GROUND );		
	}

	// check if he can see far
	if ( !pDirectorNPC && m_bLongRangeNPC )
		pEntity->AddSpawnFlags( SF_NPC_LONG_RANGE );

	if ( !pDirectorNPC && HasSpawnFlags( ASW_SF_NEVER_SLEEP ) )
		pEntity->AddSpawnFlags( SF_NPC_ALWAYSTHINK );

	pEntity->SetAbsOrigin( m_vecCurrentSpawnPosition );
	pEntity->SetAbsAngles( m_angCurrentSpawnAngles );

	IASW_Spawnable_NPC* pSpawnable = dynamic_cast<IASW_Spawnable_NPC*>(pEntity);
	Assert( pSpawnable );	
	if ( !pSpawnable )
	{
		Warning( "NULL Spawnable Ent in asw_spawner! AlienClass = %s\n", szAlienClassName );
		UTIL_Remove( pEntity );
		return NULL;
	}
	m_flLastSpawnTime = gpGlobals->curtime;

	if ( !pDirectorNPC )
	{
		pSpawnable->SetSpawner( this );
	}

	if ( m_bStartBurrowed )
	{
		pSpawnable->StartBurrowed();
	}

	if ( m_bStartBurrowed )
	{
		pSpawnable->SetUnburrowIdleActivity( m_UnburrowIdleActivity );
		pSpawnable->SetUnburrowActivity( m_UnburrowActivity );
	}

	pEntity->m_iszVScripts = m_iszAlienVScripts;
	pEntity->m_iszScriptThinkFunction = m_iszAlienScriptThinkFunction;

	DispatchSpawn( pEntity );

	if ( pDirectorNPC )
	{
		if ( pDirectorNPC->m_iszVScript != NULL_STRING )
		{
			pEntity->RunScriptFile( STRING( pDirectorNPC->m_iszVScript ) );
		}

		pEntity->Activate();

		pSpawnable->SetAlienOrders( AOT_MoveToNearestMarine, vec3_origin, NULL );
	}
	else
	{
		pEntity->SetOwnerEntity( this );
		pEntity->Activate();

		if ( m_AlienName != NULL_STRING )
		{
			pEntity->SetName( m_AlienName );
		}

		RemoveObstructingProps( pEntity );

		// give our aliens the orders
		pSpawnable->SetAlienOrders( m_AlienOrders, vec3_origin, GetOrderTarget() );

		m_OnSpawned.FireOutput(pEntity, this);
	}

	return pSpawnable;
}

CBaseEntity* CASW_Base_Spawner::GetAlienOrderTarget()
{
	return m_hAlienOrderTarget.Get();
}

bool CASW_Base_Spawner::IsValidOnThisSkillLevel()
{
	int nSkillLevel = ASWGameRules()->GetSkillLevel();

	if ( rd_difficulty_tier.GetInt() > 0 )
	{
		nSkillLevel = 5; // set to max value, means Brutal
	}

	if ( m_iMinSkillLevel > 0 && nSkillLevel < m_iMinSkillLevel )
		return false;
	if ( m_iMaxSkillLevel > 0 && m_iMaxSkillLevel < 10
			&& nSkillLevel > m_iMaxSkillLevel )
		return false;
	return true;
}

bool CASW_Base_Spawner::HasRecentlySpawned( float flRecent )
{
	return m_flLastSpawnTime > 0.0f && ( ( gpGlobals->curtime - m_flLastSpawnTime ) < flRecent );
}


//------------------------------------------------------------------------------
// Inputs
//------------------------------------------------------------------------------
void CASW_Base_Spawner::InputToggleEnabled( inputdata_t &inputdata )
{
	if ( !m_bEnabled )
	{
		InputEnable( inputdata );
	}
	else
	{
		InputDisable( inputdata );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CASW_Base_Spawner::InputEnable( inputdata_t &inputdata )
{
	if ( !m_bEnabled )
	{
		m_bEnabled = true;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CASW_Base_Spawner::InputDisable( inputdata_t &inputdata )
{
	if ( m_bEnabled )
	{
		m_bEnabled = false;
	}
}
