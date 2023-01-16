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
#include "ai_link.h"
#include "ai_basenpc.h"
#include "inetchannelinfo.h"
#include "decals.h"
#include "player_voice_listener.h"
#include "ColorText_Shared.h"
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
extern char *s_ElementNames[MAX_ARRAY_ELEMENTS];

constexpr int CLAMP_COLOR(int value)
{
	return value < 0 ? 0 : value > 255 ? 255 : value;
}

//-----------------------------------------------------------------------------
// Iterate through keys in a table and assign KeyValues on entity for spawn
//-----------------------------------------------------------------------------
void ParseTable( CBaseEntity *pEntity, HSCRIPT hTable, const char *pszKey = "" )
{
	if ( !pEntity || !hTable )
		return;

	ScriptVariant_t key, value;
	int nIterator = g_pScriptVM->GetKeyValue( hTable, 0, &key, &value );
	while ( nIterator != -1 )
	{
		const char *szKeyName = key;

		if ( V_strcmp( pszKey, "" ) != 0 )
			szKeyName = pszKey;

		switch ( value.m_type )
		{
			case FIELD_FLOAT:
			{
				pEntity->KeyValue( szKeyName, value.m_float );
				break;
			}
			case FIELD_VECTOR:
			{
				pEntity->KeyValue( szKeyName, *value.m_pVector );
				break;
			}
			case FIELD_INTEGER:
			case FIELD_BOOLEAN:
			{
				pEntity->KeyValue( szKeyName, value.m_int );
				break;
			}
			case FIELD_CSTRING:
			{
				pEntity->KeyValue( szKeyName, value.m_pszString );
				break;
			}
			case FIELD_HSCRIPT:
			{
				ParseTable( pEntity, value.m_hScript, key );
				break;
			}
			default:
			{
				Warning( "Unsupported KeyValue type for key %s (type %s)\n", key, ScriptFieldTypeName( value.m_type ) );
			}
		}

		nIterator = g_pScriptVM->GetKeyValue( hTable, nIterator, &key, &value );
	}
}

//-----------------------------------------------------------------------------
// Sets value of a SendProp for Temp Entity from a table
//-----------------------------------------------------------------------------
void TE_SetSendProp( SendProp *pSendProp, CBaseTempEntity *pTempEntity, int iOffset, int iElement, HSCRIPT hTable )
{
	const char *pszPropName = pSendProp->GetName();
	if ( iElement > -1 )
		pszPropName = s_ElementNames[iElement];

	ScriptVariant_t value;
	bool bKeyExists = g_pScriptVM->GetValue( hTable, pszPropName, &value );

	uint8 *pEntityPropData = (uint8 *)pTempEntity + iOffset + pSendProp->GetOffset();
	switch ( pSendProp->GetType() )
	{
		case DPT_Int:
		{
			int nBits = pSendProp->m_nBits;
			if ( nBits == 21 )
			{
				CBaseEntity *pOtherEntity = ToEnt( value.m_hScript );
				CBaseHandle &baseHandle = *(CBaseHandle *)pEntityPropData;
				if ( !pOtherEntity )
					baseHandle.Set( NULL );
				else
					baseHandle.Set( (IHandleEntity *)pOtherEntity );
			}
			else if ( nBits >= 17 )
			{
				*(int32 *)pEntityPropData = (int32)value.m_int;
			}
			else if ( nBits >= 9 )
			{
				if (!pSendProp->IsSigned())
					*(uint16 *)pEntityPropData = (uint16)value.m_int;
				else
					*(int16 *)pEntityPropData = (int16)value.m_int;
			}
			else if ( nBits >= 2 )
			{
				if (!pSendProp->IsSigned())
					*(uint8 *)pEntityPropData = (uint8)value.m_int;
				else
					*(int8 *)pEntityPropData = (int8)value.m_int;
			}
			else
			{
				*(bool *)pEntityPropData = value.m_bool ? true : false;
			}
			break;
		}
		case DPT_Float:
		{
			*(float *)(uint8 *)pEntityPropData = value.m_float;
			break;
		}
		case DPT_Vector:
		{
			if ( !bKeyExists )
				value = Vector( 0, 0, 0 );

			Vector *pVec = (Vector *)(uint8 *)pEntityPropData;
			pVec->x = value.m_pVector->x;
			pVec->y = value.m_pVector->y;
			pVec->z = value.m_pVector->z;
			break;
		}
	}
}

//-----------------------------------------------------------------------------
// Iterate through SendTable setting SendProp data for Temp Entity from a table
//-----------------------------------------------------------------------------
void TE_ParseSendPropTable( SendTable *pSendTable, CBaseTempEntity *pTempEntity, int iOffset, HSCRIPT hTable )
{
	for ( int nPropIdx = 0; nPropIdx < pSendTable->GetNumProps(); nPropIdx++ )
	{
		SendProp *pSendProp = pSendTable->GetProp( nPropIdx );
		if ( pSendProp->IsExcludeProp() )
			continue;

		SendTable *pInternalSendTable = pSendProp->GetDataTable();
		if ( pInternalSendTable )
			TE_ParseSendPropTable( pInternalSendTable, pTempEntity, (iOffset + pSendProp->GetOffset()), hTable );
		else
		{
			if ( pSendProp->GetType() == DPT_Array )
			{
				SendProp *pArrayProp = pSendProp->GetArrayProp();
				ScriptVariant_t value;
				bool bSuccess = g_pScriptVM->GetValue( hTable, pArrayProp->GetName(), &value );
				if ( bSuccess && value.m_type == FIELD_HSCRIPT )
				{
					for ( int element = 0; element < pSendProp->GetNumElements(); element++ )
					{
						TE_SetSendProp( pArrayProp, pTempEntity, iOffset + ( element * pSendProp->GetElementStride() ), element, value.m_hScript );
					}
				}
			}
			else
				TE_SetSendProp( pSendProp, pTempEntity, iOffset, -1, hTable );
		}
	}
}

//-----------------------------------------------------------------------------
// Store SendProp type in a table for Temp Entity
//-----------------------------------------------------------------------------
void TE_StoreSendPropValue( SendProp *pSendProp, int iElement, HSCRIPT hTable )
{
	if ( !hTable )
		return;

	const char *pszPropType = "unknown";
	const char *pszPropName = pSendProp->GetName();
	if ( iElement > -1 )
		pszPropName = s_ElementNames[iElement];

	switch ( pSendProp->GetType() )
	{
		case DPT_Int:
		{
			int nBits = pSendProp->m_nBits;
			if ( nBits == 21 )
				pszPropType = "instance";
			else
				pszPropType = "integer";
			break;
		}
		case DPT_Float:
		{
			pszPropType = "float";
			break;
		}
		case DPT_Vector:
		{
			pszPropType = "Vector";
			break;
		}
	}

	g_pScriptVM->SetValue( hTable, pszPropName, pszPropType );
}

//-----------------------------------------------------------------------------
// Iterate through SendTable storing SendProp types in a table for Temp Entity
//-----------------------------------------------------------------------------
void TE_CollectNestedSendProps( SendTable *pSendTable, HSCRIPT hTable )
{
	if ( !hTable )
		return;

	for ( int nPropIdx = 0; nPropIdx < pSendTable->GetNumProps(); nPropIdx++ )
	{
		SendProp *pSendProp = pSendTable->GetProp( nPropIdx );
		if ( pSendProp->IsExcludeProp() )
			continue;

		const char *pszPropName = pSendProp->GetName();
		SendTable *pInternalSendTable = pSendProp->GetDataTable();
		if ( pInternalSendTable )
		{
			if ( V_strcmp( pSendProp->GetName(), "baseclass" ) == 0 )
				pszPropName = pInternalSendTable->m_pNetTableName;

			ScriptVariant_t hPropTable;
			g_pScriptVM->CreateTable( hPropTable );
			TE_CollectNestedSendProps( pInternalSendTable, hPropTable );
			g_pScriptVM->SetValue( hTable, pszPropName, hPropTable );
			g_pScriptVM->ReleaseValue( hPropTable );
		}
		else
		{
			if ( pSendProp->GetType() == DPT_Array )
			{
				SendProp *pArrayProp = pSendProp->GetArrayProp();
				ScriptVariant_t hPropTable;
				g_pScriptVM->CreateTable( hPropTable );

				for ( int element = 0; element < pSendProp->GetNumElements(); element++ )
				{
					TE_StoreSendPropValue( pArrayProp, element, hPropTable );
				}

				g_pScriptVM->SetValue( hTable, pszPropName, hPropTable );
				g_pScriptVM->ReleaseValue( hPropTable );
			}
			else
				TE_StoreSendPropValue( pSendProp, -1, hTable );
		}
	}
}

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
	if ( !g_pScriptVM ) return NULL;

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
	if ( !g_pScriptVM ) return NULL;

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
	if ( !g_pScriptVM ) return NULL;

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

		if ( !g_pScriptVM ) return;

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

		if ( !g_pScriptVM ) return;

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

	void AddOutput( HSCRIPT hEntity, const char *szOutputName, const char *szTarget, const char *szTargetInput, const char *szParameter, float flDelay, int iTimesToFire )
	{
		CBaseEntity *pBaseEntity = ToEnt(hEntity);
		if ( !pBaseEntity )
		{
			DevMsg ("Error: Entity is NULL in EntityOutputs.AddOutput\n" );
			return;
		}

		CBaseEntityOutput *pOutput = pBaseEntity->FindNamedOutput( szOutputName );
		if ( !pOutput )
		{
			DevMsg ("Error: Cannot find named output \"%s\" in EntityOutputs.AddOutput\n", szOutputName );
			return;
		}

		CEventAction *pAction = new CEventAction( NULL );
		pAction->m_iTarget = AllocPooledString( szTarget );
		pAction->m_iTargetInput = AllocPooledString( szTargetInput );
		pAction->m_iParameter = AllocPooledString( szParameter );
		pAction->m_flDelay = flDelay;
		pAction->m_nTimesToFire = iTimesToFire;
		pOutput->AddEventAction( pAction );
	}

	void RemoveOutput( HSCRIPT hEntity, const char *szOutputName, const char *szTarget, const char *szTargetInput, const char *szParameter )
	{
		CBaseEntity *pBaseEntity = ToEnt(hEntity);
		if ( !pBaseEntity )
		{
			DevMsg ("Error: Entity is NULL in EntityOutputs.RemoveOutput\n" );
			return;
		}

		CBaseEntityOutput *pOutput = pBaseEntity->FindNamedOutput( szOutputName );
		if ( !pOutput )
		{
			DevMsg ("Error: Cannot find named output \"%s\" in EntityOutputs.RemoveOutput\n", szOutputName );
			return;
		}

		if ( V_strcmp( szTarget, "" ) == 0 )
			pOutput->DeleteAllElements();
		else
		{
			CEventAction *pAction = pOutput->GetFirstAction();
			pOutput->ScriptRemoveEventAction( pAction, szTarget, szTargetInput, szParameter );
		}
	}

	ScriptVariant_t GetValue( HSCRIPT hEntity, const char *szOutputName )
	{
		CBaseEntity *pBaseEntity = ToEnt( hEntity );
		if ( !pBaseEntity )
		{
			DevMsg( "Error: Entity is NULL in EntityOutputs.GetValue\n" );
			return SCRIPT_VARIANT_NULL;
		}

		CBaseEntityOutput *pOutput = pBaseEntity->FindNamedOutput( szOutputName );
		if ( !pOutput )
		{
			DevMsg( "Error: Cannot find named output \"%s\" in EntityOutputs.GetValue\n", szOutputName );
			return SCRIPT_VARIANT_NULL;
		}

		switch ( pOutput->ValueFieldType() )
		{
		default:
			// COutputVariant is only used by one output (logic_case's OnDefault) so to avoid complicating this function, only the specific types supported by other typedefs are handled.
			return SCRIPT_VARIANT_NULL;
		case FIELD_INTEGER:
			return static_cast<COutputInt *>( pOutput )->Get();
		case FIELD_FLOAT:
			return static_cast<COutputFloat *>( pOutput )->Get();
		case FIELD_STRING:
			return ScriptVariant_t( STRING( static_cast<COutputString *>( pOutput )->Get() ), true );
		case FIELD_EHANDLE:
			return ToHScript( static_cast<COutputEHANDLE *>( pOutput )->Get() );
		case FIELD_VECTOR:
		case FIELD_POSITION_VECTOR:
		{
			Vector vec;
			static_cast<COutputVector *>( pOutput )->Get( vec );
			return ScriptVariant_t( vec, true );
		}
		case FIELD_COLOR32:
			return int( *static_cast<COutputColor32 *>( pOutput )->Get().asInt() );
		}
	}
} g_ScriptEntityOutputs;

BEGIN_SCRIPTDESC_ROOT_NAMED( CScriptEntityOutputs, "CScriptEntityOutputs", SCRIPT_SINGLETON "Used to get entity output data" )
	DEFINE_SCRIPTFUNC( GetNumElements, "Arguments: ( entity, outputName ) - returns the number of array elements" )
	DEFINE_SCRIPTFUNC( GetOutputTable, "Arguments: ( entity, outputName, table, arrayElement ) - returns a table of output information" )
	DEFINE_SCRIPTFUNC( HasOutput, "Arguments: ( entity, outputName ) - returns true if the output exists" )
	DEFINE_SCRIPTFUNC( HasAction, "Arguments: ( entity, outputName ) - returns true if an action exists for the output" )
	DEFINE_SCRIPTFUNC( AddOutput, "Arguments: ( entity, outputName, targetName, inputName, parameter, delay, timesToFire ) - add a new output to the entity" )
	DEFINE_SCRIPTFUNC( RemoveOutput, "Arguments: ( entity, outputName, targetName, inputName, parameter ) - remove an output from the entity" )
	DEFINE_SCRIPTFUNC( GetValue, "Arguments: ( entity, outputName ) - returns the value of the output if it has one" )
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

	HSCRIPT GetNodeByID( int node_id )
	{
		CAI_Node *pNode = g_pBigAINet->GetNode( node_id );
		if ( !pNode )
			return NULL;

		return ToHScript( pNode );
	}

	HSCRIPT GetNearestNodeToPoint( HSCRIPT hNPC, const Vector &vPosition )
	{
		CBaseEntity *pBaseEntity = ToEnt(hNPC);
		CAI_BaseNPC *pNPC = NULL;
		if ( pBaseEntity )
			pNPC = dynamic_cast<CAI_BaseNPC*>(pBaseEntity);

		int node = g_pBigAINet->NearestNodeToPoint( pNPC, vPosition, false );
		if ( node == NO_NODE )
			return NULL;

		return ToHScript( g_pBigAINet->GetNode( node ) );
	}

	int GetAllNearestNodes( HSCRIPT hNPC, const Vector &vPosition, int iMaxNodes, HSCRIPT hTable )
	{
		return g_pBigAINet->ScriptNearestNodesInBox( hNPC, vPosition, iMaxNodes, hTable );
	}

	void GetAllNodes( HSCRIPT hTable )
	{
		int nNodes = g_pBigAINet->NumNodes();
		CAI_Node **ppNodes = g_pBigAINet->AccessNodes();
		for ( int i = 0; i < nNodes; i++ )
		{
			CAI_Node *pNode = ppNodes[i];
			g_pScriptVM->SetValue( hTable, CFmtStr( "node%i", i ), ToHScript( pNode ) );
		}
	}

	HSCRIPT CreateLink( int srcID, int destID )
	{
		CAI_Link *pLink = g_pBigAINet->CreateLink( srcID, destID );
		if ( !pLink )
			return NULL;

		return ToHScript( pLink );
	}
} g_ScriptInfoNodes;

BEGIN_SCRIPTDESC_ROOT_NAMED( CScriptInfoNodes, "CScriptInfoNodes", SCRIPT_SINGLETON "Used to get info_node data" )
	DEFINE_SCRIPTFUNC( GetNumNodes, "Returns the amount of info_nodes in the network array" )
	DEFINE_SCRIPTFUNC( GetNodeOrigin, "Arguments: ( id ) - returns the origin of the node" )
	DEFINE_SCRIPTFUNC( GetNodePosition, "Arguments: ( id, hull ) - returns the hull specific origin of the node" )
	DEFINE_SCRIPTFUNC( GetNodeType, "Arguments: ( id ) - returns the type of node" )
	DEFINE_SCRIPTFUNC( GetNodeByID, "Arguments: ( id ) - returns the node by ID" )
	DEFINE_SCRIPTFUNC( GetNearestNodeToPoint, "Arguments: ( npc, origin ) - returns the node nearest to origin with optional npc parameter" )
	DEFINE_SCRIPTFUNC( GetAllNearestNodes, "Arguments: ( npc, origin, maxNodes, table ) - fills a passed in table of x nearest nodes to origin with optional npc parameter" )
	DEFINE_SCRIPTFUNC( GetAllNodes, "Arguments: ( table ) - fills a passed in table of all nodes" )
	DEFINE_SCRIPTFUNC( CreateLink, "Arguments: ( srcID, destID ) - creates a new link from srcID to destID and returns the link" )
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
	DEFINE_SCRIPTFUNC( GetPropBool, "Arguments: ( entity, propertyName )" )
	DEFINE_SCRIPTFUNC( GetPropBoolArray, "Arguments: ( entity, propertyName, arrayElement )" )
	DEFINE_SCRIPTFUNC( SetPropBool, "Arguments: ( entity, propertyName, value )" )
	DEFINE_SCRIPTFUNC( SetPropBoolArray, "Arguments: ( entity, propertyName, value, arrayElement )" )
	DEFINE_SCRIPTFUNC( GetPropInfo, "Arguments: ( entity, propertyName, arrayElement, table ) - Fills in a passed table with property info for the provided entity" )
	DEFINE_SCRIPTFUNC( GetTable, "Arguments: ( entity, iPropType, table ) - Fills in a passed table with all props of a specified type for the provided entity (set iPropType to 0 for SendTable or 1 for DataMap)" )
END_SCRIPTDESC()


class CScriptTempEnts
{
public:
	void Create( HSCRIPT hPlayer, const char *pName, float flDelay, HSCRIPT hTable )
	{
		if ( !hTable )
			return;

		CBaseEntity *pBaseEntity = ToEnt(hPlayer);
		CBasePlayer *pPlayer = NULL;
		CRecipientFilter filter;

		if ( pBaseEntity )
			pPlayer = dynamic_cast<CBasePlayer*>(pBaseEntity);

		if ( pPlayer )
		{
			CSingleUserRecipientFilter user( pPlayer );
			filter = user;
		}
		else
		{
			CBroadcastRecipientFilter broadcast;
			filter = broadcast;
		}

		CBaseTempEntity *tempEnt = CBaseTempEntity::GetList();
		while ( tempEnt )
		{
			if ( V_strcmp( tempEnt->GetName(), pName ) == 0 )
			{
				ServerClass *pServerClass = tempEnt->GetServerClass();
				SendTable   *pSendTable = pServerClass->m_pTable;
				TE_ParseSendPropTable( pSendTable, tempEnt, 0, hTable );
				tempEnt->Create( filter, flDelay );
				break;
			}
			tempEnt = tempEnt->GetNext();
		}
	}
	void GetPropTypes( const char *pName, HSCRIPT hTable )
	{
		if ( !hTable )
			return;

		CBaseTempEntity *tempEnt = CBaseTempEntity::GetList();
		while ( tempEnt )
		{
			if ( V_strcmp( tempEnt->GetName(), pName ) == 0 )
			{
				ServerClass *pServerClass = tempEnt->GetServerClass();
				SendTable   *pSendTable = pServerClass->m_pTable;
				TE_CollectNestedSendProps( pSendTable, hTable );
				break;
			}
			tempEnt = tempEnt->GetNext();
		}
	}
	void GetNames( HSCRIPT hTable )
	{
		if ( !hTable )
			return;

		int i = 0;
		CBaseTempEntity *tempEnt = CBaseTempEntity::GetList();
		while ( tempEnt )
		{
			g_pScriptVM->SetValue( hTable, CFmtStr( "name%i", i++ ), tempEnt->GetName() );
			tempEnt = tempEnt->GetNext();
		}
	}
} g_ScriptTempEnts;

BEGIN_SCRIPTDESC_ROOT_NAMED( CScriptTempEnts, "CScriptTempEnts", SCRIPT_SINGLETON "Used to create temp entities on clients" )
	DEFINE_SCRIPTFUNC( Create, "Arguments: ( player, tempEntName, flDelay, table ) - Queue a temp entity for transmission from a passed table of SendProp data" )
	DEFINE_SCRIPTFUNC( GetPropTypes, "Arguments: ( tempEntName, table ) - Fills in a passed table with all SendProps and their types for the temp entity" )
	DEFINE_SCRIPTFUNC( GetNames, "Arguments: ( table ) - Fills in a passed table with the names of all temp entities" )
END_SCRIPTDESC();



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
	if ( g_pScriptVM )
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
		if ( g_pScriptVM )
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

static void ScriptTraceLineTable( HSCRIPT hTable )
{
	if ( !hTable )
		return;

	if  (!g_pScriptVM ) return;

	// UTIL_TraceLine( vecAbsStart, vecAbsEnd, MASK_BLOCKLOS, pLooker, COLLISION_GROUP_NONE, ptr );
	trace_t tr;
	ScriptVariant_t start, end, mask, ignore;
	g_pScriptVM->GetValue( hTable, "start", &start );
	g_pScriptVM->GetValue( hTable, "end", &end );
	g_pScriptVM->GetValue( hTable, "mask", &mask );
	g_pScriptVM->GetValue( hTable, "ignore", &ignore );
	const Vector &vecStart = Vector( start.m_pVector->x, start.m_pVector->y, start.m_pVector->z );
	const Vector &vecEnd = Vector( end.m_pVector->x, end.m_pVector->y, end.m_pVector->z );
	UTIL_TraceLine( vecStart, vecEnd, mask.m_int, ToEnt(ignore.m_hScript), COLLISION_GROUP_NONE, &tr );

	g_pScriptVM->SetValue( hTable, "pos", tr.endpos );
	g_pScriptVM->SetValue( hTable, "fraction", tr.fraction );
	g_pScriptVM->SetValue( hTable, "hit", tr.DidHit() );
	g_pScriptVM->SetValue( hTable, "enthit", ToHScript(tr.m_pEnt) );
	g_pScriptVM->SetValue( hTable, "startsolid", tr.startsolid );
	
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

//This is a vscript function that converts RGB into a transmittable format for colored text
static ScriptVariant_t Script_TextColor(int R, int G, int B)
{
	//Force channel ranges between 0 - 255
	R = CLAMP_COLOR(R);
	G = CLAMP_COLOR(G);
	B = CLAMP_COLOR(B);

	//create float modifiers at range 0 - 255 for conversion output
	float outputMod_R = R / 255.0f;
	float outputMod_G = G / 255.0f;
	float outputMod_B = B / 255.0f;

	char outputChars[5]{};
	outputChars[0] = COLOR_INPUTCUSTOMCOL;
	// pass float modifiers multiplied by max ASCII translation base then increment by 32 which is float ASCII offset
	outputChars[1] = (char)( 32 + ( outputMod_R * 94 ) );
	outputChars[2] = (char)( 32 + ( outputMod_G * 94 ) );
	outputChars[3] = (char)( 32 + ( outputMod_B * 94 ) );

	return ScriptVariant_t(outputChars, true);
}

static ScriptVariant_t Script_TextColorBlend(int R1, int G1, int B1, int R2, int G2, int B2)
{
	//Force channel ranges between 0 - 255
	R1 = CLAMP_COLOR(R1);
	G1 = CLAMP_COLOR(G1);
	B1 = CLAMP_COLOR(B1);

	R2 = CLAMP_COLOR(R2);
	G2 = CLAMP_COLOR(G2);
	B2 = CLAMP_COLOR(B2);

	//create float modifiers at range 0 - 255 for conversion output
	float outputMod_R1 = R1 / 255.0f;
	float outputMod_G1 = G1 / 255.0f;
	float outputMod_B1 = B1 / 255.0f;

	float outputMod_R2 = R2 / 255.0f;
	float outputMod_G2 = G2 / 255.0f;
	float outputMod_B2 = B2 / 255.0f;

	char outputChars[10]{};
	outputChars[0] = COLOR_INPUTCUSTOMCOL;
	outputChars[1] = COLOR_INPUTCUSTOMCOL;
	outputChars[2] = BLEND_NORMAL;
	// pass float modifiers multiplied by max ASCII translation base then increment by 32 which is float ASCII offset
	outputChars[3] = (char)(32 + (outputMod_R1 * 94));
	outputChars[4] = (char)(32 + (outputMod_G1 * 94));
	outputChars[5] = (char)(32 + (outputMod_B1 * 94));

	outputChars[6] = (char)(32 + (outputMod_R2 * 94));
	outputChars[7] = (char)(32 + (outputMod_G2 * 94));
	outputChars[8] = (char)(32 + (outputMod_B2 * 94));

	return ScriptVariant_t(outputChars, true);
}

static ScriptVariant_t Script_TextColorBlendCycle(int iBlendLength, int R1, int G1, int B1, int R2, int G2, int B2)
{
	//Force channel ranges between 0 - 255
	R1 = CLAMP_COLOR(R1);
	G1 = CLAMP_COLOR(G1);
	B1 = CLAMP_COLOR(B1);

	R2 = CLAMP_COLOR(R2);
	G2 = CLAMP_COLOR(G2);
	B2 = CLAMP_COLOR(B2);

	if (iBlendLength > 96)
	{
		iBlendLength = 96;
	}
	else if (iBlendLength < 2)
	{
		iBlendLength = 2;
	}

	iBlendLength -= 2; //Align offset

	//create float modifiers at range 0 - 255 for conversion output
	float outputMod_R1 = R1 / 255.0f;
	float outputMod_G1 = G1 / 255.0f;
	float outputMod_B1 = B1 / 255.0f;

	float outputMod_R2 = R2 / 255.0f;
	float outputMod_G2 = G2 / 255.0f;
	float outputMod_B2 = B2 / 255.0f;

	char outputChars[11]{};
	outputChars[0] = COLOR_INPUTCUSTOMCOL;
	outputChars[1] = COLOR_INPUTCUSTOMCOL;
	outputChars[2] = BLEND_CYCLE;
	outputChars[3] = iBlendLength + 32;
	// pass float modifiers multiplied by max ASCII translation base then increment by 32 which is float ASCII offset
	outputChars[4] = (char)(32 + (outputMod_R1 * 94));
	outputChars[5] = (char)(32 + (outputMod_G1 * 94));
	outputChars[6] = (char)(32 + (outputMod_B1 * 94));

	outputChars[7] = (char)(32 + (outputMod_R2 * 94));
	outputChars[8] = (char)(32 + (outputMod_G2 * 94));
	outputChars[9] = (char)(32 + (outputMod_B2 * 94));

	return ScriptVariant_t(outputChars, true);
}

static ScriptVariant_t Script_TextColorBlendSmoothCycle(int iBlendLength, int R1, int G1, int B1, int R2, int G2, int B2)
{
	//Force channel ranges between 0 - 255
	R1 = CLAMP_COLOR(R1);
	G1 = CLAMP_COLOR(G1);
	B1 = CLAMP_COLOR(B1);

	R2 = CLAMP_COLOR(R2);
	G2 = CLAMP_COLOR(G2);
	B2 = CLAMP_COLOR(B2);

	if (iBlendLength > 96)
	{
		iBlendLength = 96;
	}
	else if (iBlendLength < 2)
	{
		iBlendLength = 2;
	}

	iBlendLength -= 2; //align offset

	//create float modifiers at range 0 - 255 for conversion output
	float outputMod_R1 = R1 / 255.0f;
	float outputMod_G1 = G1 / 255.0f;
	float outputMod_B1 = B1 / 255.0f;

	float outputMod_R2 = R2 / 255.0f;
	float outputMod_G2 = G2 / 255.0f;
	float outputMod_B2 = B2 / 255.0f;

	char outputChars[11]{};
	outputChars[0] = COLOR_INPUTCUSTOMCOL;
	outputChars[1] = COLOR_INPUTCUSTOMCOL;
	outputChars[2] = BLEND_SMOOTHCYCLE;
	outputChars[3] = iBlendLength + 32;
	// pass float modifiers multiplied by max ASCII translation base then increment by 32 which is float ASCII offset
	outputChars[4] = (char)(32 + (outputMod_R1 * 94));
	outputChars[5] = (char)(32 + (outputMod_G1 * 94));
	outputChars[6] = (char)(32 + (outputMod_B1 * 94));

	outputChars[7] = (char)(32 + (outputMod_R2 * 94));
	outputChars[8] = (char)(32 + (outputMod_G2 * 94));
	outputChars[9] = (char)(32 + (outputMod_B2 * 94));

	return ScriptVariant_t(outputChars, true);
}

static ScriptVariant_t Script_TextColorBlendInvert(int R1, int G1, int B1)
{
	//Force channel ranges between 0 - 255
	R1 = CLAMP_COLOR(R1);
	G1 = CLAMP_COLOR(G1);
	B1 = CLAMP_COLOR(B1);

	//create float modifiers at range 0 - 255 for conversion output
	float outputMod_R1 = R1 / 255.0f;
	float outputMod_G1 = G1 / 255.0f;
	float outputMod_B1 = B1 / 255.0f;

	char outputChars[7]{};
	outputChars[0] = COLOR_INPUTCUSTOMCOL;
	outputChars[1] = COLOR_INPUTCUSTOMCOL;
	outputChars[2] = BLEND_INVERT;
	// pass float modifiers multiplied by max ASCII translation base then increment by 32 which is float ASCII offset
	outputChars[3] = (char)(32 + (outputMod_R1 * 94));
	outputChars[4] = (char)(32 + (outputMod_G1 * 94));
	outputChars[5] = (char)(32 + (outputMod_B1 * 94));

	return ScriptVariant_t(outputChars, true);
}

static ScriptVariant_t Script_TextColorBlend3(int R1, int G1, int B1, int R2, int G2, int B2, int R3, int G3, int B3)
{
	//Force channel ranges between 0 - 255
	R1 = CLAMP_COLOR(R1);
	G1 = CLAMP_COLOR(G1);
	B1 = CLAMP_COLOR(B1);

	R2 = CLAMP_COLOR(R2);
	G2 = CLAMP_COLOR(G2);
	B2 = CLAMP_COLOR(B2);

	R3 = CLAMP_COLOR(R3);
	G3 = CLAMP_COLOR(G3);
	B3 = CLAMP_COLOR(B3);

	//create float modifiers at range 0 - 255 for conversion output
	float outputMod_R1 = R1 / 255.0f;
	float outputMod_G1 = G1 / 255.0f;
	float outputMod_B1 = B1 / 255.0f;

	float outputMod_R2 = R2 / 255.0f;
	float outputMod_G2 = G2 / 255.0f;
	float outputMod_B2 = B2 / 255.0f;

	float outputMod_R3 = R3 / 255.0f;
	float outputMod_G3 = G3 / 255.0f;
	float outputMod_B3 = B3 / 255.0f;

	char outputChars[13]{};
	outputChars[0] = COLOR_INPUTCUSTOMCOL;
	outputChars[1] = COLOR_INPUTCUSTOMCOL;
	outputChars[2] = BLEND_3COLOR;
	// pass float modifiers multiplied by max ASCII translation base then increment by 32 which is float ASCII offset
	outputChars[3] = (char)(32 + (outputMod_R1 * 94));
	outputChars[4] = (char)(32 + (outputMod_G1 * 94));
	outputChars[5] = (char)(32 + (outputMod_B1 * 94));

	outputChars[6] = (char)(32 + (outputMod_R2 * 94));
	outputChars[7] = (char)(32 + (outputMod_G2 * 94));
	outputChars[8] = (char)(32 + (outputMod_B2 * 94));

	outputChars[9] = (char)(32 + (outputMod_R3 * 94));
	outputChars[10] = (char)(32 + (outputMod_G3 * 94));
	outputChars[11] = (char)(32 + (outputMod_B3 * 94));

	return ScriptVariant_t(outputChars, true);
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
	if ( !pszFileName || !Q_strcmp( pszFileName, "" ) )
	{
		Log_Warning( LOG_VScript, "StringToFile() file name cannot be null or empty\n" );
		return;
	}

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
		g_pFullFileSystem->CreateDirHierarchy( CFmtStr( "save/vscripts/%s", szFolders ), "MOD" );
	else
		g_pFullFileSystem->CreateDirHierarchy( "save/vscripts", "MOD" );
	CUtlBuffer buf( pszString, V_strlen( pszString ), CUtlBuffer::READ_ONLY );
	g_pFullFileSystem->WriteFile( szFullFileName, "MOD", buf );
}

static const char *Script_FileToString( const char *pszFileName )
{
	if ( !pszFileName || !Q_strcmp( pszFileName, "" ) )
	{
		Log_Warning( LOG_VScript, "FileToString() file name cannot be null or empty\n" );
		return NULL;
	}

	if ( V_strstr( pszFileName, ".." ) )
	{
		Log_Warning( LOG_VScript, "FileToString() file name cannot contain '..'\n" );
		return NULL;
	}

	char szFullFileName[256];
	Q_snprintf( szFullFileName, sizeof(szFullFileName), "save/vscripts/%s", pszFileName );

	FileHandle_t fh = g_pFullFileSystem->Open( szFullFileName, "rb" );
	if ( fh == FILESYSTEM_INVALID_HANDLE )
		return NULL;

	static char szString[16384];
	int size = g_pFullFileSystem->Size(fh);
	if ( size + 1 >= sizeof(szString) )
	{
		Log_Warning( LOG_VScript, "File %s (from %s) is len %i too long for a ScriptFileRead\n", szFullFileName, pszFileName, size );
		return NULL;
	}

	g_pFullFileSystem->Read( szString, size, fh );
	g_pFullFileSystem->Close(fh);
	szString[size] = '\0';

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
	color32 color = { (byte)r, (byte)g, (byte)b, (byte)a };

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

static int Script_PrecacheModel( const char *modelname )
{
	return CBaseEntity::PrecacheModel( VScriptCutDownString( modelname ) );
}

static int Script_GetModelIndex( const char *modelname )
{
	return modelinfo->GetModelIndex( modelname );
}

static void Script_GetPlayerConnectionInfo( HSCRIPT hPlayer, HSCRIPT hTable )
{
	CBaseEntity *pBaseEntity = ToEnt(hPlayer);
	CBasePlayer *pPlayer = NULL;

	if ( pBaseEntity )
		pPlayer = dynamic_cast<CBasePlayer*>(pBaseEntity);

	if ( !pPlayer || !hTable )
		return;

	if ( !g_pScriptVM ) return;

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

static ScriptVariant_t Script_GetClientXUID( HSCRIPT hPlayer )
{
	CBaseEntity *pBaseEntity = ToEnt(hPlayer);
	CBasePlayer *pPlayer = NULL;

	if ( pBaseEntity )
		pPlayer = dynamic_cast<CBasePlayer*>(pBaseEntity);

	if ( !pPlayer )
		return "";

	uint64 xuid = engine->GetClientXUID( pPlayer->edict() );
	return ScriptVariant_t( CFmtStr( "%I64u", xuid ), true );
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

	if ( !g_pScriptVM ) return;

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

HSCRIPT Script_SpawnEntityFromTable( const char *pszEntityName, HSCRIPT hTable )
{
	CBaseEntity *pBaseEntity = (CBaseEntity *)CreateEntityByName( pszEntityName );
	if ( !pBaseEntity )
	{
		Warning( "Cannot spawn entity %s\n", pszEntityName );
		return NULL;
	}

	ParseTable( pBaseEntity, hTable );
	DispatchSpawn( pBaseEntity );
	pBaseEntity->Activate();
	return ToHScript( pBaseEntity );
}

static int Script_GetDecalIndexForName( const char *decalName )
{
	return decalsystem->GetDecalIndexForName( decalName );
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
				ScriptRegisterFunctionNamed( g_pScriptVM, ScriptTraceLineTable, "TraceLineTable", "Uses a configuration table to do a raytrace, puts return information into the table for return usage." );

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
				ScriptRegisterFunctionNamed( g_pScriptVM, Script_TextColor, "TextColor", "Gets the translated ASCII characters for an RGB input." );
				ScriptRegisterFunctionNamed( g_pScriptVM, Script_TextColorBlend, "TextColorBlend", "Gets the translated ASCII characters for an RGB blend input.");
				ScriptRegisterFunctionNamed( g_pScriptVM, Script_TextColorBlendCycle, "TextColorBlendCycle", "Gets the translated ASCII characters for an RGB blend cycle input.");
				ScriptRegisterFunctionNamed( g_pScriptVM, Script_TextColorBlendSmoothCycle, "TextColorBlendSmoothCycle", "Gets the translated ASCII characters for an RGB blend smooth cycle input.");
				ScriptRegisterFunctionNamed( g_pScriptVM, Script_TextColorBlend3, "TextColorBlend3", "Gets the translated ASCII characters for a 3 color RGB blend input.");
				ScriptRegisterFunctionNamed( g_pScriptVM, Script_TextColorBlendInvert, "TextColorBlendInvert", "Gets the translated ASCII characters for an RGB blend invert input.");
				ScriptRegisterFunctionNamed( g_pScriptVM, Script_StringToFile, "StringToFile", "Stores the string into the file." );
				ScriptRegisterFunctionNamed( g_pScriptVM, Script_FileToString, "FileToString", "Reads a string from file. Returns the string from the file, null if no file or file is too big." );
				ScriptRegisterFunctionNamed( g_pScriptVM, Script_AddThinkToEnt, "AddThinkToEnt", "Adds a late bound think function to the C++ think tables for the obj" );
				ScriptRegisterFunctionNamed( g_pScriptVM, Script_ScreenShake, "ScreenShake", "Start a screenshake with the following parameters. vecCenter, flAmplitude, flFrequency, flDuration, flRadius, eCommand( SHAKE_START = 0, SHAKE_STOP = 1 ), bAirShake" );
				ScriptRegisterFunctionNamed( g_pScriptVM, Script_ScreenFade, "ScreenFade", "Start a screenfade with the following parameters. player, red, green, blue, alpha, flFadeTime, flFadeHold, flags" );
				ScriptRegisterFunctionNamed( g_pScriptVM, Script_ChangeLevel, "ChangeLevel", "Tell engine to change level." );
				ScriptRegisterFunctionNamed( g_pScriptVM, Script_IsModelPrecached, "IsModelPrecached", "Checks if the modelname is precached." );
				ScriptRegisterFunctionNamed( g_pScriptVM, Script_PrecacheModel, "PrecacheModel", "Precache a model after the map has loaded and return index of the model" );
				ScriptRegisterFunctionNamed( g_pScriptVM, Script_GetModelIndex, "GetModelIndex", "Returns index of model by name." );
				ScriptRegisterFunctionNamed( g_pScriptVM, Script_GetPlayerConnectionInfo, "GetPlayerConnectionInfo", "Returns a table containing the player's connection info." );
				ScriptRegisterFunctionNamed( g_pScriptVM, Script_GetClientXUID, "GetClientXUID", "Get the player's xuid (i.e. SteamID64)." );
				ScriptRegisterFunctionNamed( g_pScriptVM, Script_FadeClientVolume, "FadeClientVolume", "Fade out the client's volume level toward silence (or fadePercent)" );
				ScriptRegisterFunctionNamed( g_pScriptVM, Script_LocalTime, "LocalTime", "Fills in the passed table with the local system time." );
				ScriptRegisterFunctionNamed( g_pScriptVM, Script_SpawnEntityFromTable, "SpawnEntityFromTable", "Spawn entity from KeyValues in table - 'name' is entity name, rest are KeyValues for spawn." );
				ScriptRegisterFunction( g_pScriptVM, PrecacheParticleSystem, "Precaches a particle material" );
				ScriptRegisterFunction( g_pScriptVM, GetParticleSystemIndex, "Converts a previously precached material into an index" );
				ScriptRegisterFunction( g_pScriptVM, GetParticleSystemNameFromIndex, "Converts a previously precached material index into a string" );
				ScriptRegisterFunction( g_pScriptVM, PrecacheEffect, "Precaches an effect" );
				ScriptRegisterFunction( g_pScriptVM, GetEffectIndex, "Converts a previously precached effect into an index" );
				ScriptRegisterFunction( g_pScriptVM, GetEffectNameFromIndex, "Converts a previously precached effect index into a string" );
				ScriptRegisterFunctionNamed( g_pScriptVM, Script_GetDecalIndexForName, "GetDecalIndexForName", "Get decal index from a string" );

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
				g_pScriptVM->RegisterInstance( &g_ScriptTempEnts, "TempEnts" );
				g_pScriptVM->RegisterInstance( &PlayerVoiceListener(), "PlayerVoiceListener" );

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
				g_pScriptVM->SetValue( "HULL_HUGE", 12 );

				// AI_ZoneIds_t enums for CAI_Node::GetZone and CAI_Node::SetZone
				g_pScriptVM->SetValue( "AI_NODE_ZONE_UNKNOWN", 0 );
				g_pScriptVM->SetValue( "AI_NODE_ZONE_SOLO", 1 );
				g_pScriptVM->SetValue( "AI_NODE_ZONE_UNIVERSAL", 3 );
				g_pScriptVM->SetValue( "AI_NODE_FIRST_ZONE", 4 );

				// Link_Info_t enums for CAI_Link::ScriptGetLinkInfo and CAI_Link::ScriptSetLinkInfo
				g_pScriptVM->SetValue( "bits_LINK_STALE_SUGGESTED", 0x01 );
				g_pScriptVM->SetValue( "bits_LINK_OFF", 0x02 );
				g_pScriptVM->SetValue( "bits_LINK_PRECISE_MOVEMENT", 0x04 );
				g_pScriptVM->SetValue( "bits_PREFER_AVOID", 0x08 );
				g_pScriptVM->SetValue( "bits_LINK_ASW_BASHABLE", 0x10 );

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
		g_pScriptVM->ReleaseFunction( hReplaceClosuresFunc );
		return false;
	}

	g_pScriptVM->Call( hReplaceClosuresFunc, NULL, true, NULL, hNewScript, hScope );
	g_pScriptVM->ReleaseFunction( hReplaceClosuresFunc );
	g_pScriptVM->ReleaseScript( hNewScript );
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

		if ( !g_pScriptVM ) return;

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
				if ( g_pScriptVM )
				{
					ScriptVariant_t variant;
					if ( g_pScriptVM->GetValue( STRING(pEnt->m_iszScriptId), &variant ) && variant.m_type == FIELD_HSCRIPT )
					{
						pEnt->m_ScriptScope.Init( variant.m_hScript, false );
						pEnt->RunPrecacheScripts();
					}
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


bool CAINodeScriptInstanceHelper::ToString( void *p, char *pBuf, int bufSize )	
{
	CAI_Node *pNode = (CAI_Node *)p;
	V_snprintf( pBuf, bufSize, "([%d] Node)", pNode->GetId() );
	return true; 
}

CAINodeScriptInstanceHelper g_AINodeScriptInstanceHelper;


bool CAILinkScriptInstanceHelper::ToString( void *p, char *pBuf, int bufSize )	
{
	CAI_Link *pLink = (CAI_Link *)p;
	V_snprintf( pBuf, bufSize, "([%d, %d] Link)", pLink->m_iSrcID, pLink->m_iDestID );
	return true; 
}

CAILinkScriptInstanceHelper g_AILinkScriptInstanceHelper;


