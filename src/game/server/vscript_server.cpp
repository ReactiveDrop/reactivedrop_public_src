//========== Copyright © 2008, Valve Corporation, All rights reserved. ========
//
// Purpose:
//
//=============================================================================

#include "cbase.h"
#include "vscript_server.h"
#include "icommandline.h"
#include "tier1/utlbuffer.h"
#include "tier1/fmtstr.h"
#include "filesystem.h"
#include "eventqueue.h"
#include "characterset.h"
#include "sceneentity.h"		// for exposing scene precache function
#include "isaverestore.h"
#include "gamerules.h"
#include "netpropmanager.h"
#include "ai_speech.h"
#include "ai_network.h"
#include "ai_node.h"
#include "inetchannelinfo.h"
#ifdef _WIN32
#include "vscript_server_nut.h"
#endif

extern ScriptClassDesc_t * GetScriptDesc( CBaseEntity * );

// #define VMPROFILE 1

#ifdef VMPROFILE

#define VMPROF_START float debugStartTime = Plat_FloatTime();
#define VMPROF_SHOW( funcname, funcdesc  ) DevMsg("***VSCRIPT PROFILE***: %s %s: %6.4f milliseconds\n", (##funcname), (##funcdesc), (Plat_FloatTime() - debugStartTime)*1000.0 );

#else // !VMPROFILE

#define VMPROF_START
#define VMPROF_SHOW

#endif // VMPROFILE

static ConVar sv_mapspawn_nut_exec( "sv_mapspawn_nut_exec", "0", FCVAR_NONE, "If set to 1, server will execute scripts/vscripts/mapspawn.nut file" );

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
class CScriptEntityIterator
{
public:
	HSCRIPT First() { return Next(NULL); }

	HSCRIPT Next( HSCRIPT hStartEntity )
	{
		return ToHScript( gEntList.NextEnt( ToEnt( hStartEntity ) ) );
	}

	HSCRIPT CreateByClassname( const char *className )
	{
		return ToHScript( CreateEntityByName( className ) );
	}

	HSCRIPT FindByClassname( HSCRIPT hStartEntity, const char *szName )
	{
		return ToHScript( gEntList.FindEntityByClassname( ToEnt( hStartEntity ), szName ) );
	}

	HSCRIPT FindByName( HSCRIPT hStartEntity, const char *szName )
	{
		return ToHScript( gEntList.FindEntityByName( ToEnt( hStartEntity ), szName ) );
	}

	HSCRIPT FindInSphere( HSCRIPT hStartEntity, const Vector &vecCenter, float flRadius )
	{
		return ToHScript( gEntList.FindEntityInSphere( ToEnt( hStartEntity ), vecCenter, flRadius ) );
	}

	HSCRIPT FindByTarget( HSCRIPT hStartEntity, const char *szName )
	{
		return ToHScript( gEntList.FindEntityByTarget( ToEnt( hStartEntity ), szName ) );
	}

	HSCRIPT FindByModel( HSCRIPT hStartEntity, const char *szModelName )
	{
		return ToHScript( gEntList.FindEntityByModel( ToEnt( hStartEntity ), szModelName ) );
	}

	HSCRIPT FindByNameNearest( const char *szName, const Vector &vecSrc, float flRadius )
	{
		return ToHScript( gEntList.FindEntityByNameNearest( szName, vecSrc, flRadius ) );
	}

	HSCRIPT FindByNameWithin( HSCRIPT hStartEntity, const char *szName, const Vector &vecSrc, float flRadius )
	{
		return ToHScript( gEntList.FindEntityByNameWithin( ToEnt( hStartEntity ), szName, vecSrc, flRadius ) );
	}

	HSCRIPT FindByClassnameNearest( const char *szName, const Vector &vecSrc, float flRadius )
	{
		return ToHScript( gEntList.FindEntityByClassnameNearest( szName, vecSrc, flRadius ) );
	}

	HSCRIPT FindByClassnameWithin( HSCRIPT hStartEntity , const char *szName, const Vector &vecSrc, float flRadius )
	{
		return ToHScript( gEntList.FindEntityByClassnameWithin( ToEnt( hStartEntity ), szName, vecSrc, flRadius ) );
	}

private:
} g_ScriptEntityIterator;

BEGIN_SCRIPTDESC_ROOT_NAMED( CScriptEntityIterator, "CEntities", SCRIPT_SINGLETON "The global list of entities" )
	DEFINE_SCRIPTFUNC( First, "Begin an iteration over the list of entities" )
	DEFINE_SCRIPTFUNC( Next, "Continue an iteration over the list of entities, providing reference to a previously found entity" )
	DEFINE_SCRIPTFUNC( CreateByClassname, "Creates an entity by classname" )
	DEFINE_SCRIPTFUNC( FindByClassname, "Find entities by class name. Pass 'null' to start an iteration, or reference to a previously found entity to continue a search"  )
	DEFINE_SCRIPTFUNC( FindByName, "Find entities by name. Pass 'null' to start an iteration, or reference to a previously found entity to continue a search"  )
	DEFINE_SCRIPTFUNC( FindInSphere, "Find entities within a radius. Pass 'null' to start an iteration, or reference to a previously found entity to continue a search"  )
	DEFINE_SCRIPTFUNC( FindByTarget, "Find entities by targetname. Pass 'null' to start an iteration, or reference to a previously found entity to continue a search"  )
	DEFINE_SCRIPTFUNC( FindByModel, "Find entities by model name. Pass 'null' to start an iteration, or reference to a previously found entity to continue a search"  )
	DEFINE_SCRIPTFUNC( FindByNameNearest, "Find entities by name nearest to a point."  )
	DEFINE_SCRIPTFUNC( FindByNameWithin, "Find entities by name within a radius. Pass 'null' to start an iteration, or reference to a previously found entity to continue a search"  )
	DEFINE_SCRIPTFUNC( FindByClassnameNearest, "Find entities by class name nearest to a point."  )
	DEFINE_SCRIPTFUNC( FindByClassnameWithin, "Find entities by class name within a radius. Pass 'null' to start an iteration, or reference to a previously found entity to continue a search"  )
END_SCRIPTDESC();

// ----------------------------------------------------------------------------
// KeyValues access - CBaseEntity::ScriptGetKeyFromModel returns root KeyValues
// ----------------------------------------------------------------------------

BEGIN_SCRIPTDESC_ROOT( CScriptKeyValues, "Wrapper class over KeyValues instance" )
	DEFINE_SCRIPT_CONSTRUCTOR()	
	DEFINE_SCRIPTFUNC_NAMED( ScriptFindKey, "FindKey", "Given a KeyValues object and a key name, find a KeyValues object associated with the key name" );
	DEFINE_SCRIPTFUNC_NAMED( ScriptGetFirstSubKey, "GetFirstSubKey", "Given a KeyValues object, return the first sub key object" );
	DEFINE_SCRIPTFUNC_NAMED( ScriptGetNextKey, "GetNextKey", "Given a KeyValues object, return the next key object in a sub key group" );
	DEFINE_SCRIPTFUNC_NAMED( ScriptGetKeyValueInt, "GetKeyInt", "Given a KeyValues object and a key name, return associated integer value" );
	DEFINE_SCRIPTFUNC_NAMED( ScriptGetKeyValueFloat, "GetKeyFloat", "Given a KeyValues object and a key name, return associated float value" );
	DEFINE_SCRIPTFUNC_NAMED( ScriptGetKeyValueBool, "GetKeyBool", "Given a KeyValues object and a key name, return associated bool value" );
	DEFINE_SCRIPTFUNC_NAMED( ScriptGetKeyValueString, "GetKeyString", "Given a KeyValues object and a key name, return associated string value" );
	DEFINE_SCRIPTFUNC_NAMED( ScriptIsKeyValueEmpty, "IsKeyEmpty", "Given a KeyValues object and a key name, return true if key name has no value" );
	DEFINE_SCRIPTFUNC_NAMED( ScriptReleaseKeyValues, "ReleaseKeyValues", "Given a root KeyValues object, release its contents" );
END_SCRIPTDESC();

HSCRIPT CScriptKeyValues::ScriptFindKey( const char *pszName )
{
	KeyValues *pKeyValues = m_pKeyValues->FindKey(pszName);
	if ( pKeyValues == NULL )
		return NULL;

	CScriptKeyValues *pScriptKey = new CScriptKeyValues( pKeyValues );

	// UNDONE: who calls ReleaseInstance on this??
	HSCRIPT hScriptInstance = g_pScriptVM->RegisterInstance( pScriptKey );
	return hScriptInstance;
}

HSCRIPT CScriptKeyValues::ScriptGetFirstSubKey( void )
{
	KeyValues *pKeyValues = m_pKeyValues->GetFirstSubKey();
	if ( pKeyValues == NULL )
		return NULL;

	CScriptKeyValues *pScriptKey = new CScriptKeyValues( pKeyValues );

	// UNDONE: who calls ReleaseInstance on this??
	HSCRIPT hScriptInstance = g_pScriptVM->RegisterInstance( pScriptKey );
	return hScriptInstance;
}

HSCRIPT CScriptKeyValues::ScriptGetNextKey( void )
{
	KeyValues *pKeyValues = m_pKeyValues->GetNextKey();
	if ( pKeyValues == NULL )
		return NULL;

	CScriptKeyValues *pScriptKey = new CScriptKeyValues( pKeyValues );

	// UNDONE: who calls ReleaseInstance on this??
	HSCRIPT hScriptInstance = g_pScriptVM->RegisterInstance( pScriptKey );
	return hScriptInstance;
}

int CScriptKeyValues::ScriptGetKeyValueInt( const char *pszName )
{
	int i = m_pKeyValues->GetInt( pszName );
	return i;
}

float CScriptKeyValues::ScriptGetKeyValueFloat( const char *pszName )
{
	float f = m_pKeyValues->GetFloat( pszName );
	return f;
}

const char *CScriptKeyValues::ScriptGetKeyValueString( const char *pszName )
{
	const char *psz = m_pKeyValues->GetString( pszName );
	return psz;
}

bool CScriptKeyValues::ScriptIsKeyValueEmpty( const char *pszName )
{
	bool b = m_pKeyValues->IsEmpty( pszName );
	return b;
}

bool CScriptKeyValues::ScriptGetKeyValueBool( const char *pszName )
{
	bool b = m_pKeyValues->GetBool( pszName );
	return b;
}

void CScriptKeyValues::ScriptReleaseKeyValues( )
{
	m_pKeyValues->deleteThis();
	m_pKeyValues = NULL;
}


// constructors
CScriptKeyValues::CScriptKeyValues( KeyValues *pKeyValues = NULL )
{
	m_pKeyValues = pKeyValues;
}

// destructor
CScriptKeyValues::~CScriptKeyValues( )
{
	if (m_pKeyValues)
	{
		m_pKeyValues->deleteThis();
	}
	m_pKeyValues = NULL;
}


class CScriptResponseCriteria
{
public:
	const char *GetValue( HSCRIPT hEntity, const char *pCriteria )
	{
		CBaseEntity *pBaseEntity = ToEnt(hEntity);
		if ( !pBaseEntity )
			return "";

		AI_CriteriaSet criteria;
		pBaseEntity->ModifyOrAppendCriteria( criteria );

		int index = criteria.FindCriterionIndex( pCriteria );
		if ( !criteria.IsValidIndex( index ) )
			return "";

		return criteria.GetValue( index );
	}

	void GetTable( HSCRIPT hEntity, HSCRIPT hTable )
	{
		CBaseEntity *pBaseEntity = ToEnt(hEntity);
		if ( !pBaseEntity || !hTable )
			return;

		AI_CriteriaSet criteria;
		pBaseEntity->ModifyOrAppendCriteria( criteria );

		for ( int nCriteriaIdx = 0; nCriteriaIdx < criteria.GetCount(); nCriteriaIdx++ )
		{
			if ( !criteria.IsValidIndex( nCriteriaIdx ) )
				continue;

			g_pScriptVM->SetValue( hTable, criteria.GetName( nCriteriaIdx ), criteria.GetValue( nCriteriaIdx ) );
		}
	}

	bool HasCriterion( HSCRIPT hEntity, const char *pCriteria )
	{
		CBaseEntity *pBaseEntity = ToEnt(hEntity);
		if ( !pBaseEntity )
			return false;

		AI_CriteriaSet criteria;
		pBaseEntity->ModifyOrAppendCriteria( criteria );

		int index = criteria.FindCriterionIndex( pCriteria );
		return criteria.IsValidIndex( index );
	}
} g_ScriptResponseCriteria;

BEGIN_SCRIPTDESC_ROOT_NAMED( CScriptResponseCriteria, "CScriptResponseCriteria", SCRIPT_SINGLETON "Used to get response criteria" )
	DEFINE_SCRIPTFUNC( GetValue, "Arguments: ( entity, criteriaName ) - returns a string" )
	DEFINE_SCRIPTFUNC( GetTable, "Arguments: ( entity ) - returns a table of all criteria" )
	DEFINE_SCRIPTFUNC( HasCriterion, "Arguments: ( entity, criteriaName ) - returns true if the criterion exists" )
END_SCRIPTDESC();


class CScriptEntityOutputs
{
public:
	int GetNumElements( HSCRIPT hEntity, const char *szOutputName )
	{
		CBaseEntity *pBaseEntity = ToEnt(hEntity);
		if ( !pBaseEntity )
			return -1;

		CBaseEntityOutput *pOutput = pBaseEntity->FindNamedOutput( szOutputName );
		if ( !pOutput )
			return -1;

		return pOutput->NumberOfElements();
	}

	void GetOutputTable( HSCRIPT hEntity, const char *szOutputName, HSCRIPT hOutputTable, int element )
	{
		CBaseEntity *pBaseEntity = ToEnt(hEntity);
		if ( !pBaseEntity || !hOutputTable || element < 0 )
			return;

		CBaseEntityOutput *pOutput = pBaseEntity->FindNamedOutput( szOutputName );
		if ( pOutput )
		{
			int iCount = 0;
			CEventAction *pAction = pOutput->GetFirstAction();
			while ( pAction )
			{
				if ( iCount == element )
				{
					g_pScriptVM->SetValue( hOutputTable, "target", STRING( pAction->m_iTarget ) );
					g_pScriptVM->SetValue( hOutputTable, "input", STRING( pAction->m_iTargetInput ) );
					g_pScriptVM->SetValue( hOutputTable, "parameter", STRING( pAction->m_iParameter ) );
					g_pScriptVM->SetValue( hOutputTable, "delay", pAction->m_flDelay );
					g_pScriptVM->SetValue( hOutputTable, "times_to_fire", pAction->m_nTimesToFire );
					break;
				}
				else
				{
					iCount++;
					pAction = pAction->m_pNext;
				}
			}
		}
	}

	bool HasOutput( HSCRIPT hEntity, const char *szOutputName )
	{
		CBaseEntity *pBaseEntity = ToEnt(hEntity);
		if ( !pBaseEntity )
			return false;

		CBaseEntityOutput *pOutput = pBaseEntity->FindNamedOutput( szOutputName );
		if ( !pOutput )
			return false;

		return true;
	}

	bool HasAction( HSCRIPT hEntity, const char *szOutputName )
	{
		CBaseEntity *pBaseEntity = ToEnt(hEntity);
		if ( !pBaseEntity )
			return false;

		CBaseEntityOutput *pOutput = pBaseEntity->FindNamedOutput( szOutputName );
		if ( pOutput )
		{
			CEventAction *pAction = pOutput->GetFirstAction();
			if ( pAction )
				return true;
		}

		return false;
	}
} g_ScriptEntityOutputs;

BEGIN_SCRIPTDESC_ROOT_NAMED( CScriptEntityOutputs, "CScriptEntityOutputs", SCRIPT_SINGLETON "Used to get entity output data" )
	DEFINE_SCRIPTFUNC( GetNumElements, "Arguments: ( entity, outputName ) - returns the number of array elements" )
	DEFINE_SCRIPTFUNC( GetOutputTable, "Arguments: ( entity, outputName, table, arrayElement ) - returns a table of output information" )
	DEFINE_SCRIPTFUNC( HasOutput, "Arguments: ( entity, outputName ) - returns true if the output exists" )
	DEFINE_SCRIPTFUNC( HasAction, "Arguments: ( entity, outputName ) - returns true if an action exists for the output" )
END_SCRIPTDESC();


class CScriptInfoNodes
{
public:
	int GetNumNodes()
	{
		return g_pBigAINet->NumNodes();
	}

	Vector GetNodeOrigin( int node_id )
	{
		CAI_Node *pNode = g_pBigAINet->GetNode( node_id );
		if ( !pNode )
			return Vector( 0, 0, 0 );

		return pNode->GetOrigin();
	}

	Vector GetNodePosition( int node_id, int hull )
	{
		CAI_Node *pNode = g_pBigAINet->GetNode( node_id );
		if ( !pNode || ( hull < HULL_HUMAN || hull >= NUM_HULLS ) )
			return Vector( 0, 0, 0 );

		return pNode->GetPosition( hull );
	}

	int GetNodeType( int node_id )
	{
		CAI_Node *pNode = g_pBigAINet->GetNode( node_id );
		if ( !pNode )
			return -1;

		return pNode->GetType();
	}
} g_ScriptInfoNodes;

BEGIN_SCRIPTDESC_ROOT_NAMED( CScriptInfoNodes, "CScriptInfoNodes", SCRIPT_SINGLETON "Used to get info_node data" )
	DEFINE_SCRIPTFUNC( GetNumNodes, "Returns the amount of info_nodes in the network array" )
	DEFINE_SCRIPTFUNC( GetNodeOrigin, "Arguments: ( id ) - returns the origin of the node" )
	DEFINE_SCRIPTFUNC( GetNodePosition, "Arguments: ( id, hull ) - returns the hull specific origin of the node" )
	DEFINE_SCRIPTFUNC( GetNodeType, "Arguments: ( id ) - returns the type of node" )
END_SCRIPTDESC();


// When a scripter wants to change a netprop value, they can use the
// CNetPropManager class; it checks for errors and such on its own.
CNetPropManager g_NetProps;

BEGIN_SCRIPTDESC_ROOT_NAMED( CNetPropManager, "CNetPropManager", SCRIPT_SINGLETON "Used to get/set entity network fields" )
	DEFINE_SCRIPTFUNC( GetPropInt, "Arguments: ( entity, propertyName )" )
	DEFINE_SCRIPTFUNC( GetPropFloat, "Arguments: ( entity, propertyName )" )
	DEFINE_SCRIPTFUNC( GetPropVector, "Arguments: ( entity, propertyName )" )
	DEFINE_SCRIPTFUNC( GetPropEntity, "Arguments: ( entity, propertyName ) - returns an entity" )
	DEFINE_SCRIPTFUNC( GetPropString, "Arguments: ( entity, propertyName )" )
	DEFINE_SCRIPTFUNC( SetPropInt, "Arguments: ( entity, propertyName, value )" )
	DEFINE_SCRIPTFUNC( SetPropFloat, "Arguments: ( entity, propertyName, value )" )
	DEFINE_SCRIPTFUNC( SetPropVector, "Arguments: ( entity, propertyName, value )" )
	DEFINE_SCRIPTFUNC( SetPropEntity, "Arguments: ( entity, propertyName, value )" )
	DEFINE_SCRIPTFUNC( SetPropString, "Arguments: ( entity, propertyName, value )" )
	DEFINE_SCRIPTFUNC( GetPropIntArray, "Arguments: ( entity, propertyName, arrayElement )" )
	DEFINE_SCRIPTFUNC( GetPropFloatArray, "Arguments: ( entity, propertyName, arrayElement )" )
	DEFINE_SCRIPTFUNC( GetPropVectorArray, "Arguments: ( entity, propertyName, arrayElement )" )
	DEFINE_SCRIPTFUNC( GetPropEntityArray, "Arguments: ( entity, propertyName, arrayElement ) - returns an entity" )
	DEFINE_SCRIPTFUNC( GetPropStringArray, "Arguments: ( entity, propertyName, arrayElement )" )
	DEFINE_SCRIPTFUNC( SetPropIntArray, "Arguments: ( entity, propertyName, value, arrayElement )" )
	DEFINE_SCRIPTFUNC( SetPropFloatArray, "Arguments: ( entity, propertyName, value, arrayElement )" )
	DEFINE_SCRIPTFUNC( SetPropVectorArray, "Arguments: ( entity, propertyName, value, arrayElement )" )
	DEFINE_SCRIPTFUNC( SetPropEntityArray, "Arguments: ( entity, propertyName, value, arrayElement )" )
	DEFINE_SCRIPTFUNC( SetPropStringArray, "Arguments: ( entity, propertyName, value, arrayElement )" )
	DEFINE_SCRIPTFUNC( GetPropArraySize, "Arguments: ( entity, propertyName )" )
	DEFINE_SCRIPTFUNC( HasProp, "Arguments: ( entity, propertyName )" )
	DEFINE_SCRIPTFUNC( GetPropType, "Arguments: ( entity, propertyName )" )
END_SCRIPTDESC()




//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
static float Time()
{
	return gpGlobals->curtime;
}

static float FrameTime()
{
	return gpGlobals->frametime;
}

static int Script_GetFrameCount()
{
	return gpGlobals->framecount;
}

static void SendToConsole( const char *pszCommand )
{
	CBasePlayer *pPlayer = UTIL_GetLocalPlayerOrListenServerHost();
	if ( !pPlayer )
	{
		DevMsg ("Cannot execute \"%s\", no player\n", pszCommand );
		return;
	}

	engine->ClientCommand( pPlayer->edict(), pszCommand );
}

static void SendToServerConsole( const char *pszCommand )
{
	char szCommand[512];
	Q_snprintf( szCommand, sizeof(szCommand), "%s\n", pszCommand );
	engine->ServerCommand( szCommand );
}

static const char *GetMapName()
{
	return STRING( gpGlobals->mapname );
}

static const char *DoUniqueString( const char *pszBase )
{
	static char szBuf[512];
	g_pScriptVM->GenerateUniqueKey( pszBase, szBuf, ARRAYSIZE(szBuf) );
	return szBuf;
}

static bool Script_IsDedicatedServer()
{
	return engine->IsDedicatedServer();
}

HSCRIPT Script_GetLocalPlayerOrListenServerHost()
{
	CBasePlayer *pPlayer = UTIL_GetLocalPlayerOrListenServerHost();
	return ToHScript( pPlayer );
}

static void Script_AddThinkToEnt( HSCRIPT entity, const char *funcName )
{
	CBaseEntity *pEntity = ToEnt( entity );

	if ( !pEntity )
		return;

	pEntity->m_iszScriptThinkFunction = AllocPooledString( funcName );
	pEntity->SetContextThink( &CBaseEntity::ScriptThink, gpGlobals->curtime, "ScriptThink" );
}

static void DoEntFire( const char *pszTarget, const char *pszAction, const char *pszValue, float delay, HSCRIPT hActivator, HSCRIPT hCaller )
{
	const char *target = "", *action = "Use";
	variant_t value;

	target = STRING( AllocPooledString( pszTarget ) );

	// Don't allow them to run anything on a point_servercommand unless they're the host player. Otherwise they can ent_fire
	// and run any command on the server. Admittedly, they can only do the ent_fire if sv_cheats is on, but 
	// people complained about users resetting the rcon password if the server briefly turned on cheats like this:
	//    give point_servercommand
	//    ent_fire point_servercommand command "rcon_password mynewpassword"
	if ( gpGlobals->maxClients > 1 && V_stricmp( target, "point_servercommand" ) == 0 )
	{
		return;
	}

	if ( *pszAction )
	{
		action = STRING( AllocPooledString( pszAction ) );
	}
	if ( *pszValue )
	{
		value.SetString( AllocPooledString( pszValue ) );
	}
	if ( delay < 0 )
	{
		delay = 0;
	}

	g_EventQueue.AddEvent( target, action, value, delay, ToEnt(hActivator), ToEnt(hCaller) );
}


bool DoIncludeScript( const char *pszScript, HSCRIPT hScope )
{
	if ( !VScriptRunScript( pszScript, hScope, true ) )
	{
		g_pScriptVM->RaiseException( CFmtStr( "Failed to include script \"%s\"", ( pszScript ) ? pszScript : "unknown" ) );
		return false;
	}
	return true;
}

HSCRIPT CreateProp( const char *pszEntityName, const Vector &vOrigin, const char *pszModelName, int iAnim )
{
	CBaseAnimating *pBaseEntity = (CBaseAnimating *)CreateEntityByName( pszEntityName );
	pBaseEntity->SetAbsOrigin( vOrigin );
	pBaseEntity->SetModel( pszModelName );
	pBaseEntity->SetPlaybackRate( 1.0f );

	int iSequence = pBaseEntity->SelectWeightedSequence( (Activity)iAnim );

	if ( iSequence != -1 )
	{
		pBaseEntity->SetSequence( iSequence );
	}

	// BenLubar(key-values-director)
	DispatchSpawn( pBaseEntity );
	pBaseEntity->Activate();
	//

	return ToHScript( pBaseEntity );
}

//--------------------------------------------------------------------------------------------------
// Use an entity's script instance to add an entity IO event (used for firing events on unnamed entities from vscript)
//--------------------------------------------------------------------------------------------------
static void DoEntFireByInstanceHandle( HSCRIPT hTarget, const char *pszAction, const char *pszValue, float delay, HSCRIPT hActivator, HSCRIPT hCaller )
{
	const char *action = "Use";
	variant_t value;

	if ( *pszAction )
	{
		action = STRING( AllocPooledString( pszAction ) );
	}
	if ( *pszValue )
	{
		value.SetString( AllocPooledString( pszValue ) );
	}
	if ( delay < 0 )
	{
		delay = 0;
	}

	CBaseEntity* pTarget = ToEnt(hTarget);

	if ( !pTarget )
	{
		Warning( "VScript error: DoEntFire was passed an invalid entity instance.\n" );
		return;
	}

	g_EventQueue.AddEvent( pTarget, action, value, delay, ToEnt(hActivator), ToEnt(hCaller) );
}

static float ScriptTraceLine( const Vector &vecStart, const Vector &vecEnd, HSCRIPT entIgnore )
{
	// UTIL_TraceLine( vecAbsStart, vecAbsEnd, MASK_BLOCKLOS, pLooker, COLLISION_GROUP_NONE, ptr );
	trace_t tr;
	CBaseEntity *pLooker = ToEnt(entIgnore);
	UTIL_TraceLine( vecStart, vecEnd, MASK_NPCWORLDSTATIC, pLooker, COLLISION_GROUP_NONE, &tr);
	if (tr.fractionleftsolid && tr.startsolid)
	{
		return 1.0 - tr.fractionleftsolid;
	}
	else
	{
		return tr.fraction;
	}
}

HSCRIPT Script_PlayerInstanceFromIndex( int playerIndex )
{
	CBasePlayer *pPlayer = UTIL_PlayerByIndex( playerIndex );
	return ToHScript( pPlayer );
}

HSCRIPT Script_GetPlayerFromUserID( int userID )
{
	CBasePlayer *pPlayer = UTIL_PlayerByUserId( userID );
	return ToHScript( pPlayer );
}

HSCRIPT Script_EntIndexToHScript( int entIndex )
{
	CBaseEntity *pBaseEntity = UTIL_EntityByIndex( entIndex );
	return ToHScript( pBaseEntity );
}

static void Script_Say( HSCRIPT hPlayer, const char *pText )
{
	CBaseEntity *pBaseEntity = ToEnt(hPlayer);
	CBasePlayer *pPlayer = NULL;
	char szText[256];

	if ( pBaseEntity )
		pPlayer = dynamic_cast<CBasePlayer*>(pBaseEntity);

	if ( pPlayer )
	{
		Q_snprintf( szText, sizeof(szText), "say %s", pText );
		engine->ClientCommand( pPlayer->edict(), szText );
	}
	else
	{
		Q_snprintf( szText, sizeof(szText), "Console: %s", pText );
		UTIL_SayTextAll( szText, pPlayer );
	}
}

static void Script_ClientPrint( HSCRIPT hPlayer, int iDest, const char *pText )
{
	CBaseEntity *pBaseEntity = ToEnt(hPlayer);
	CBasePlayer *pPlayer = NULL;

	if ( pBaseEntity )
		pPlayer = dynamic_cast<CBasePlayer*>(pBaseEntity);

	if ( pPlayer )
		ClientPrint( pPlayer, iDest, pText );
	else
		UTIL_ClientPrintAll( iDest, pText );
}

static void Script_StringToFile( const char *pszFileName, const char *pszString )
{
	if ( V_strstr( pszFileName, "..") )
	{
		Log_Warning( LOG_VScript, "StringToFile() file name cannot contain '..'\n" );
		return;
	}

	char szFullFileName[256];
	Q_snprintf( szFullFileName, sizeof(szFullFileName), "save/vscripts/%s", pszFileName );

	static char szFolders[256];
	bool bHasFolders = false;
	const char *pszDelimiter = V_strrchr( pszFileName, '/' );
	if ( pszDelimiter )
	{
		bHasFolders = true;
		V_strncpy( szFolders, pszFileName, MIN( ARRAYSIZE(szFolders), pszDelimiter - pszFileName + 1 ) );
	}

	if ( bHasFolders )
		g_pFullFileSystem->CreateDirHierarchy( CFmtStr( "save/vscripts/%s", szFolders ), "GAME" );
	else
		g_pFullFileSystem->CreateDirHierarchy( "save/vscripts", "GAME" );
	CUtlBuffer buf;
	buf.PutString( pszString );
	g_pFullFileSystem->WriteFile( szFullFileName, "GAME", buf );
}

static const char *Script_FileToString( const char *pszFileName )
{
	if ( V_strstr( pszFileName, "..") )
	{
		Log_Warning( LOG_VScript, "FileToString() file name cannot contain '..'\n" );
		return NULL;
	}

	char szFullFileName[256];
	Q_snprintf( szFullFileName, sizeof(szFullFileName), "save/vscripts/%s", pszFileName );

	FileHandle_t fh = g_pFullFileSystem->Open( szFullFileName, "rb" );
	if ( fh == FILESYSTEM_INVALID_HANDLE )
		return NULL;

	char szString[16384];
	int size = g_pFullFileSystem->Size(fh) + 1;
	if ( size > sizeof(szString) )
	{
		Log_Warning( LOG_VScript, "File %s (from %s) is len %i too long for a ScriptFileRead\n", szFullFileName, pszFileName, size );
		return NULL;
	}

	CUtlBuffer buf( 0, size, CUtlBuffer::TEXT_BUFFER );
	g_pFullFileSystem->Read( szString, size, fh );
	g_pFullFileSystem->Close(fh);

	const char *pszString = (const char*)szString;
	return pszString;
}

static void Script_ScreenShake( const Vector &center, float amplitude, float frequency, float duration, float radius, int eCommand, bool bAirShake )
{
	UTIL_ScreenShake( center, amplitude, frequency, duration, radius, (ShakeCommand_t)eCommand, bAirShake );
}

static void Script_ScreenFade( HSCRIPT hEntity, int r, int g, int b, int a, float fadeTime, float fadeHold, int flags )
{
	CBaseEntity *pEntity = ToEnt(hEntity);
	color32 color = { r, g, b, a };

	if ( pEntity )
		UTIL_ScreenFade( pEntity, color, fadeTime, fadeHold, flags );
	else
		UTIL_ScreenFadeAll( color, fadeTime, fadeHold, flags );
}

static void Script_ChangeLevel( const char *mapname )
{
	if ( !engine->IsMapValid( mapname ) )
		return;

	engine->ChangeLevel( mapname, NULL );
}

bool Script_IsModelPrecached( const char *modelname )
{
	return engine->IsModelPrecached( VScriptCutDownString( modelname ) );
}

static void Script_GetPlayerConnectionInfo( HSCRIPT hPlayer, HSCRIPT hTable )
{
	CBaseEntity *pBaseEntity = ToEnt(hPlayer);
	CBasePlayer *pPlayer = NULL;

	if ( pBaseEntity )
		pPlayer = dynamic_cast<CBasePlayer*>(pBaseEntity);

	if ( !pPlayer || !hTable )
		return;

	INetChannelInfo *nci = engine->GetPlayerNetInfo( pPlayer->entindex() );
	if ( nci )
	{
		g_pScriptVM->SetValue( hTable, "name", nci->GetName() );
		g_pScriptVM->SetValue( hTable, "address", nci->GetAddress() );
		g_pScriptVM->SetValue( hTable, "time", nci->GetTime() );
		g_pScriptVM->SetValue( hTable, "time_connected", nci->GetTimeConnected() );
		g_pScriptVM->SetValue( hTable, "latency", nci->GetLatency( FLOW_OUTGOING ) );
		g_pScriptVM->SetValue( hTable, "is_loopback", nci->IsLoopback() );
		g_pScriptVM->SetValue( hTable, "is_timing_out", nci->IsTimingOut() );
		g_pScriptVM->SetValue( hTable, "is_playback", nci->IsPlayback() );
	}

	int ping, packetloss;
	UTIL_GetPlayerConnectionInfo( pPlayer->entindex(), ping, packetloss );
	g_pScriptVM->SetValue( hTable, "ping", ping );
	g_pScriptVM->SetValue( hTable, "packetloss", packetloss );
}

static const char *Script_GetClientXUID( HSCRIPT hPlayer )
{
	CBaseEntity *pBaseEntity = ToEnt(hPlayer);
	CBasePlayer *pPlayer = NULL;

	if ( pBaseEntity )
		pPlayer = dynamic_cast<CBasePlayer*>(pBaseEntity);

	if ( !pPlayer )
		return "";

	uint64 xuid = engine->GetClientXUID( pPlayer->edict() );
	return CFmtStr( "%I64u", xuid );
}

static void Script_FadeClientVolume( HSCRIPT hPlayer, float fadePercent, float fadeOutSeconds, float holdTime, float fadeInSeconds )
{
	CBaseEntity *pBaseEntity = ToEnt(hPlayer);
	CBasePlayer *pPlayer = NULL;

	if ( pBaseEntity )
		pPlayer = dynamic_cast<CBasePlayer*>(pBaseEntity);

	if ( pPlayer )
		engine->FadeClientVolume( pPlayer->edict(), fadePercent, fadeOutSeconds, holdTime, fadeInSeconds );
}

static void Script_LocalTime( HSCRIPT hTable )
{
	if ( !hTable )
		return;

	struct tm timeinfo;
	Plat_GetLocalTime( &timeinfo );
	g_pScriptVM->SetValue( hTable, "year", (timeinfo.tm_year + 1900) );
	g_pScriptVM->SetValue( hTable, "month", (timeinfo.tm_mon + 1) );
	g_pScriptVM->SetValue( hTable, "dayofweek", timeinfo.tm_wday );
	g_pScriptVM->SetValue( hTable, "day", timeinfo.tm_mday );
	g_pScriptVM->SetValue( hTable, "hour", timeinfo.tm_hour );
	g_pScriptVM->SetValue( hTable, "minute", timeinfo.tm_min );
	g_pScriptVM->SetValue( hTable, "second", timeinfo.tm_sec );
	g_pScriptVM->SetValue( hTable, "dayofyear", timeinfo.tm_yday );
	g_pScriptVM->SetValue( hTable, "daylightsavings", timeinfo.tm_isdst );
}

bool VScriptServerInit()
{
	VMPROF_START

	if( scriptmanager != NULL )
	{
		ScriptLanguage_t scriptLanguage = SL_DEFAULT;

		char const *pszScriptLanguage;
		if ( CommandLine()->CheckParm( "-scriptlang", &pszScriptLanguage ) )
		{
			if( !Q_stricmp(pszScriptLanguage, "gamemonkey") )
			{
				scriptLanguage = SL_GAMEMONKEY;
			}
			else if( !Q_stricmp(pszScriptLanguage, "squirrel") )
			{
				scriptLanguage = SL_SQUIRREL;
			}
			else if( !Q_stricmp(pszScriptLanguage, "python") )
			{
				scriptLanguage = SL_PYTHON;
			}
			else
			{
				DevWarning("-server_script does not recognize a language named '%s'. virtual machine did NOT start.\n", pszScriptLanguage );
				scriptLanguage = SL_NONE;
			}

		}
		if( scriptLanguage != SL_NONE )
		{
			if ( g_pScriptVM == NULL )
				g_pScriptVM = scriptmanager->CreateVM( scriptLanguage );

			if( g_pScriptVM )
			{
				Log_Msg( LOG_VScript, "VSCRIPT: Started VScript virtual machine using script language '%s'\n", g_pScriptVM->GetLanguageName() );
				ScriptRegisterFunctionNamed( g_pScriptVM, UTIL_ShowMessageAll, "ShowMessage", "Print a hud message on all clients" );

				ScriptRegisterFunction( g_pScriptVM, SendToConsole, "Send a string to the console as a command" );
				ScriptRegisterFunction( g_pScriptVM, SendToServerConsole, "Send a string to the server console as a command" );
				ScriptRegisterFunction( g_pScriptVM, GetMapName, "Get the name of the map.");
				ScriptRegisterFunctionNamed( g_pScriptVM, ScriptTraceLine, "TraceLine", "given 2 points & ent to ignore, return fraction along line that hits world or models" );

				ScriptRegisterFunction( g_pScriptVM, Time, "Get the current server time" );
				ScriptRegisterFunction( g_pScriptVM, FrameTime, "Get the time spent on the server in the last frame" );
				ScriptRegisterFunctionNamed( g_pScriptVM, Script_GetFrameCount, "GetFrameCount", "Returns the engines current frame count" );
				ScriptRegisterFunctionNamed( g_pScriptVM, Script_IsDedicatedServer, "IsDedicatedServer", "Returns true if this is a dedicated server.");
				ScriptRegisterFunctionNamed( g_pScriptVM, Script_GetLocalPlayerOrListenServerHost, "GetListenServerHost", "Get the host player on a listen server.");
				ScriptRegisterFunction( g_pScriptVM, DoEntFire, SCRIPT_ALIAS( "EntFire", "Generate and entity i/o event" ) );
				ScriptRegisterFunctionNamed( g_pScriptVM, DoEntFireByInstanceHandle, "EntFireByHandle", "Generate and entity i/o event. First parameter is an entity instance." );
				ScriptRegisterFunction( g_pScriptVM, DoUniqueString, SCRIPT_ALIAS( "UniqueString", "Generate a string guaranteed to be unique across the life of the script VM, with an optional root string. Useful for adding data to tables when not sure what keys are already in use in that table." ) );
				ScriptRegisterFunctionNamed( g_pScriptVM, ScriptCreateSceneEntity, "CreateSceneEntity", "Create a scene entity to play the specified scene." );
				ScriptRegisterFunctionNamed( g_pScriptVM, NDebugOverlay::Box, "DebugDrawBox", "Draw a debug overlay box" );
				ScriptRegisterFunctionNamed( g_pScriptVM, NDebugOverlay::Line, "DebugDrawLine", "Draw a debug overlay box" );
				ScriptRegisterFunction( g_pScriptVM, DoIncludeScript, "Execute a script (internal)" );
				ScriptRegisterFunction( g_pScriptVM, CreateProp, "Create a physics prop" );
				ScriptRegisterFunctionNamed( g_pScriptVM, Script_Say, "Say", "Have player say string" );
				ScriptRegisterFunctionNamed( g_pScriptVM, Script_ClientPrint, "ClientPrint", "Print a client message" );
				ScriptRegisterFunctionNamed( g_pScriptVM, Script_StringToFile, "StringToFile", "Stores the string into the file." );
				ScriptRegisterFunctionNamed( g_pScriptVM, Script_FileToString, "FileToString", "Reads a string from file. Returns the string from the file, null if no file or file is too big." );
				ScriptRegisterFunctionNamed( g_pScriptVM, Script_AddThinkToEnt, "AddThinkToEnt", "Adds a late bound think function to the C++ think tables for the obj" );
				ScriptRegisterFunctionNamed( g_pScriptVM, Script_ScreenShake, "ScreenShake", "Start a screenshake with the following parameters. vecCenter, flAmplitude, flFrequency, flDuration, flRadius, eCommand( SHAKE_START = 0, SHAKE_STOP = 1 ), bAirShake" );
				ScriptRegisterFunctionNamed( g_pScriptVM, Script_ScreenFade, "ScreenFade", "Start a screenfade with the following parameters. player, red, green, blue, alpha, flFadeTime, flFadeHold, flags" );
				ScriptRegisterFunctionNamed( g_pScriptVM, Script_ChangeLevel, "ChangeLevel", "Tell engine to change level." );
				ScriptRegisterFunctionNamed( g_pScriptVM, Script_IsModelPrecached, "IsModelPrecached", "Checks if the modelname is precached." );
				ScriptRegisterFunctionNamed( g_pScriptVM, Script_GetPlayerConnectionInfo, "GetPlayerConnectionInfo", "Returns a table containing the player's connection info." );
				ScriptRegisterFunctionNamed( g_pScriptVM, Script_GetClientXUID, "GetClientXUID", "Get the player's xuid (i.e. SteamID64)." );
				ScriptRegisterFunctionNamed( g_pScriptVM, Script_FadeClientVolume, "FadeClientVolume", "Fade out the client's volume level toward silence (or fadePercent)" );
				ScriptRegisterFunctionNamed( g_pScriptVM, Script_LocalTime, "LocalTime", "Fills in the passed table with the local system time." );

				ScriptRegisterFunctionNamed( g_pScriptVM, Script_PlayerInstanceFromIndex, "PlayerInstanceFromIndex", "Get a script handle of a player using the player index." );
				ScriptRegisterFunctionNamed( g_pScriptVM, Script_GetPlayerFromUserID, "GetPlayerFromUserID", "Given a user id, return the entity, or null." );
				ScriptRegisterFunctionNamed( g_pScriptVM, Script_EntIndexToHScript, "EntIndexToHScript", "Returns the script handle for the given entity index." );

				
				if ( GameRules() )
				{
					GameRules()->RegisterScriptFunctions();
				}

				g_pScriptVM->RegisterInstance( &g_ScriptEntityIterator, "Entities" );
				g_pScriptVM->RegisterInstance( &g_NetProps, "NetProps" );
				g_pScriptVM->RegisterInstance( &g_ScriptResponseCriteria, "ResponseCriteria" );
				g_pScriptVM->RegisterInstance( &g_ScriptEntityOutputs, "EntityOutputs" );
				g_pScriptVM->RegisterInstance( &g_ScriptInfoNodes, "InfoNodes" );

				// To be used with Script_ClientPrint
				g_pScriptVM->SetValue( "HUD_PRINTNOTIFY", HUD_PRINTNOTIFY );
				g_pScriptVM->SetValue( "HUD_PRINTCONSOLE", HUD_PRINTCONSOLE );
				g_pScriptVM->SetValue( "HUD_PRINTTALK", HUD_PRINTTALK );
				g_pScriptVM->SetValue( "HUD_PRINTCENTER", HUD_PRINTCENTER );
				g_pScriptVM->SetValue( "ASW_HUD_PRINTTALKANDCONSOLE", 5 );

				// To be used with CScriptInfoNode::GetNodeType
				g_pScriptVM->SetValue( "NODE_ANY", 0 );
				g_pScriptVM->SetValue( "NODE_DELETED", 1 );
				g_pScriptVM->SetValue( "NODE_GROUND", 2 );
				g_pScriptVM->SetValue( "NODE_AIR", 3 );
				g_pScriptVM->SetValue( "NODE_CLIMB", 4 );
				g_pScriptVM->SetValue( "NODE_WATER", 5 );

				// Types of hulls
				g_pScriptVM->SetValue( "HULL_HUMAN", 0 );
				g_pScriptVM->SetValue( "HULL_SMALL_CENTERED", 1 );
				g_pScriptVM->SetValue( "HULL_WIDE_HUMAN", 2 );
				g_pScriptVM->SetValue( "HULL_TINY", 3 );
				g_pScriptVM->SetValue( "HULL_WIDE_SHORT", 4 );
				g_pScriptVM->SetValue( "HULL_MEDIUM", 5 );
				g_pScriptVM->SetValue( "HULL_TINY_CENTERED", 6 );
				g_pScriptVM->SetValue( "HULL_LARGE", 7 );
				g_pScriptVM->SetValue( "HULL_LARGE_CENTERED", 8 );
				g_pScriptVM->SetValue( "HULL_MEDIUM_TALL", 9 );
				g_pScriptVM->SetValue( "HULL_TINY_FLUID", 10 );
				g_pScriptVM->SetValue( "HULL_MEDIUMBIG", 11 );

				if ( scriptLanguage == SL_SQUIRREL )
				{
					g_pScriptVM->Run( g_Script_vscript_server );
				}

				if ( sv_mapspawn_nut_exec.GetBool() )
				{
					VScriptRunScript( "mapspawn", false );
				}

				VMPROF_SHOW( pszScriptLanguage, "virtual machine startup" );

				return true;
			}
			else
			{
				DevWarning("VM Did not start!\n");
			}
		}
	}
	else
	{
		Log_Msg( LOG_VScript, "\nVSCRIPT: Scripting is disabled.\n" );
	}
	g_pScriptVM = NULL;
	return false;
}

void VScriptServerTerm()
{
	if( g_pScriptVM != NULL )
	{
		if( g_pScriptVM )
		{
			scriptmanager->DestroyVM( g_pScriptVM );
			g_pScriptVM = NULL;
		}
	}
}


bool VScriptServerReplaceClosures( const char *pszScriptName, HSCRIPT hScope, bool bWarnMissing )
{
	if ( !g_pScriptVM )
	{
		return false;
	}

	HSCRIPT hReplaceClosuresFunc = g_pScriptVM->LookupFunction( "__ReplaceClosures" );
	if ( !hReplaceClosuresFunc )
	{
		return false;
	}
	HSCRIPT hNewScript =  VScriptCompileScript( pszScriptName, bWarnMissing );
	if ( !hNewScript )
	{
		return false;
	}

	g_pScriptVM->Call( hReplaceClosuresFunc, NULL, true, NULL, hNewScript, hScope );
	return true;
}

CON_COMMAND_F( script_reload_code, "Execute a vscript file, replacing existing functions with the functions in the run script", FCVAR_CHEAT )
{
	if ( !*args[1] )
	{
		Log_Warning( LOG_VScript, "No script specified\n" );
		return;
	}

	if ( !g_pScriptVM )
	{
		Log_Warning( LOG_VScript, "Scripting disabled or no server running\n" );
		return;
	}

	VScriptServerReplaceClosures( args[1], NULL, true );
}

CON_COMMAND_F( script_reload_entity_code, "Execute all of this entity's VScripts, replacing existing functions with the functions in the run scripts", FCVAR_CHEAT )
{
	extern CBaseEntity *GetNextCommandEntity( CBasePlayer *pPlayer, const char *name, CBaseEntity *ent );

	const char *pszTarget = "";
	if ( *args[1] )
	{
		pszTarget = args[1];
	}

	if ( !g_pScriptVM )
	{
		Log_Warning( LOG_VScript, "Scripting disabled or no server running\n" );
		return;
	}

	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	if ( !pPlayer )
		return;

	CBaseEntity *pEntity = NULL;
	while ( (pEntity = GetNextCommandEntity( pPlayer, pszTarget, pEntity )) != NULL )
	{
		if ( pEntity->m_ScriptScope.IsInitialized() && pEntity->m_iszVScripts != NULL_STRING )
		{
			char szScriptsList[255];
			Q_strcpy( szScriptsList, STRING(pEntity->m_iszVScripts) );
			CUtlStringList szScripts;
			V_SplitString( szScriptsList, " ", szScripts);

			for( int i = 0 ; i < szScripts.Count() ; i++ )
			{
				VScriptServerReplaceClosures( szScripts[i], pEntity->m_ScriptScope, true );
			}
		}
	}
}

CON_COMMAND_F( script_reload_think, "Execute an activation script, replacing existing functions with the functions in the run script", FCVAR_CHEAT )
{
	extern CBaseEntity *GetNextCommandEntity( CBasePlayer *pPlayer, const char *name, CBaseEntity *ent );

	const char *pszTarget = "";
	if ( *args[1] )
	{
		pszTarget = args[1];
	}

	if ( !g_pScriptVM )
	{
		Log_Warning( LOG_VScript, "Scripting disabled or no server running\n" );
		return;
	}

	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	if ( !pPlayer )
		return;

	CBaseEntity *pEntity = NULL;
	while ( (pEntity = GetNextCommandEntity( pPlayer, pszTarget, pEntity )) != NULL )
	{
		if ( pEntity->m_ScriptScope.IsInitialized() && pEntity->m_iszScriptThinkFunction != NULL_STRING )
		{
			VScriptServerReplaceClosures( STRING(pEntity->m_iszScriptThinkFunction), pEntity->m_ScriptScope, true );
		}
	}
}

CON_COMMAND( scripted_user_func, "Call script from this user, with the value" )
{
	if ( !g_pScriptVM )
	{
		Log_Warning( LOG_VScript, "Scripting disabled or no server running\n" );
		return;
	}
	
	CBasePlayer* pPlayer = UTIL_GetCommandClient();
	const char *pszValue = args[1];

	HSCRIPT hUserCommandFunc = g_pScriptVM->LookupFunction( "UserConsoleCommand" );
	if ( hUserCommandFunc )
	{
		ScriptStatus_t nStatus = g_pScriptVM->Call( hUserCommandFunc, NULL, false, NULL, ToHScript( pPlayer ), pszValue );
		if ( nStatus != SCRIPT_DONE )
		{
			DevWarning( "UserConsoleCommand VScript function did not finish!\n" );
		}
		g_pScriptVM->ReleaseFunction( hUserCommandFunc );
	}

	if ( g_pScriptVM->ValueExists( "g_ModeScript" ) )
	{
		ScriptVariant_t hModeScript;
		if ( g_pScriptVM->GetValue( "g_ModeScript", &hModeScript ) )
		{
			if ( HSCRIPT hFunction = g_pScriptVM->LookupFunction( "UserConsoleCommand", hModeScript ) )
			{
				ScriptStatus_t nStatus = g_pScriptVM->Call( hFunction, hModeScript, false, NULL, ToHScript( pPlayer ), pszValue );
				if ( nStatus != SCRIPT_DONE )
				{
					DevWarning( "UserConsoleCommand VScript function did not finish!\n" );
				}

				g_pScriptVM->ReleaseFunction( hFunction );
			}
			g_pScriptVM->ReleaseValue( hModeScript );
		}
	}
}

class CVScriptGameSystem : public CAutoGameSystemPerFrame
{
public:
	// Inherited from IAutoServerSystem
	virtual void LevelInitPreEntity( void )
	{
		m_bAllowEntityCreationInScripts = true;
		VScriptServerInit();
	}

	virtual void LevelInitPostEntity( void )
	{
		m_bAllowEntityCreationInScripts = false;
	}

	virtual void LevelShutdownPostEntity( void )
	{
		VScriptServerTerm();
	}

	virtual void FrameUpdatePostEntityThink() 
	{ 
		if ( g_pScriptVM )
			g_pScriptVM->Frame( gpGlobals->frametime );
	}

	bool m_bAllowEntityCreationInScripts;
};

CVScriptGameSystem g_VScriptGameSystem;

bool IsEntityCreationAllowedInScripts( void )
{
	return g_VScriptGameSystem.m_bAllowEntityCreationInScripts;
}

static short VSCRIPT_SERVER_SAVE_RESTORE_VERSION = 2;


//-----------------------------------------------------------------------------

class CVScriptSaveRestoreBlockHandler : public CDefSaveRestoreBlockHandler
{
public:
	CVScriptSaveRestoreBlockHandler() :
		m_InstanceMap( DefLessFunc(const char *) )
	{
	}
	const char *GetBlockName()
	{
		return "VScriptServer";
	}

	//---------------------------------

	void Save( ISave *pSave )
	{
		pSave->StartBlock();

		int temp = g_pScriptVM != NULL;
		pSave->WriteInt( &temp );
		if ( g_pScriptVM )
		{
			temp = g_pScriptVM->GetLanguage();
			pSave->WriteInt( &temp );
			CUtlBuffer buffer;
			g_pScriptVM->WriteState( &buffer );
			temp = buffer.TellPut();
			pSave->WriteInt( &temp );
			if ( temp > 0 )
			{
				pSave->WriteData( (const char *)buffer.Base(), temp );
			}
		}

		pSave->EndBlock();
	}

	//---------------------------------

	void WriteSaveHeaders( ISave *pSave )
	{
		pSave->WriteShort( &VSCRIPT_SERVER_SAVE_RESTORE_VERSION );
	}

	//---------------------------------

	void ReadRestoreHeaders( IRestore *pRestore )
	{
		// No reason why any future version shouldn't try to retain backward compatability. The default here is to not do so.
		short version;
		pRestore->ReadShort( &version );
		m_fDoLoad = ( version == VSCRIPT_SERVER_SAVE_RESTORE_VERSION );
	}

	//---------------------------------

	void Restore( IRestore *pRestore, bool createPlayers )
	{
		if ( !m_fDoLoad && g_pScriptVM )
		{
			return;
		}
		CBaseEntity *pEnt = gEntList.FirstEnt();
		while ( pEnt )
		{
			if ( pEnt->m_iszScriptId != NULL_STRING )
			{
				g_pScriptVM->RegisterClass( pEnt->GetScriptDesc() );
				m_InstanceMap.Insert( STRING( pEnt->m_iszScriptId ), pEnt );
			}
			pEnt = gEntList.NextEnt( pEnt );
		}

		pRestore->StartBlock();
		if ( pRestore->ReadInt() && pRestore->ReadInt() == g_pScriptVM->GetLanguage() )
		{
			int nBytes = pRestore->ReadInt();
			if ( nBytes > 0 )
			{
				CUtlBuffer buffer;
				buffer.EnsureCapacity( nBytes );
				pRestore->ReadData( (char *)buffer.AccessForDirectRead( nBytes ), nBytes, 0 );
				g_pScriptVM->ReadState( &buffer );
			}
		}
		pRestore->EndBlock();
	}

	void PostRestore( void )
	{
		for ( int i = m_InstanceMap.FirstInorder(); i != m_InstanceMap.InvalidIndex(); i = m_InstanceMap.NextInorder( i ) )
		{
			CBaseEntity *pEnt = m_InstanceMap[i];
			if ( pEnt->m_hScriptInstance )
			{
				ScriptVariant_t variant;
				if ( g_pScriptVM->GetValue( STRING(pEnt->m_iszScriptId), &variant ) && variant.m_type == FIELD_HSCRIPT )
				{
					pEnt->m_ScriptScope.Init( variant.m_hScript, false );
					pEnt->RunPrecacheScripts();
				}
			}
			else
			{
				// Script system probably has no internal references
				pEnt->m_iszScriptId = NULL_STRING;
			}
		}
		m_InstanceMap.Purge();
	}


	CUtlMap<const char *, CBaseEntity *> m_InstanceMap;

private:
	bool m_fDoLoad;
};

//-----------------------------------------------------------------------------

CVScriptSaveRestoreBlockHandler g_VScriptSaveRestoreBlockHandler;

//-------------------------------------

ISaveRestoreBlockHandler *GetVScriptSaveRestoreBlockHandler()
{
	return &g_VScriptSaveRestoreBlockHandler;
}

//-----------------------------------------------------------------------------

bool CBaseEntityScriptInstanceHelper::ToString( void *p, char *pBuf, int bufSize )	
{
	CBaseEntity *pEntity = (CBaseEntity *)p;
	if ( pEntity->GetEntityName() != NULL_STRING )
	{
		V_snprintf( pBuf, bufSize, "([%d] %s: %s)", pEntity->entindex(), STRING(pEntity->m_iClassname), STRING( pEntity->GetEntityName() ) );
	}
	else
	{
		V_snprintf( pBuf, bufSize, "([%d] %s)", pEntity->entindex(), STRING(pEntity->m_iClassname) );
	}
	return true; 
}

void *CBaseEntityScriptInstanceHelper::BindOnRead( HSCRIPT hInstance, void *pOld, const char *pszId )
{
	int iEntity = g_VScriptSaveRestoreBlockHandler.m_InstanceMap.Find( pszId );
	if ( iEntity != g_VScriptSaveRestoreBlockHandler.m_InstanceMap.InvalidIndex() )
	{
		CBaseEntity *pEnt = g_VScriptSaveRestoreBlockHandler.m_InstanceMap[iEntity];
		pEnt->m_hScriptInstance = hInstance;
		return pEnt;
	}
	return NULL;
}


CBaseEntityScriptInstanceHelper g_BaseEntityScriptInstanceHelper;


