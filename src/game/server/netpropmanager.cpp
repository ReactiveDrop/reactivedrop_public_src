//========= Copyright Valve Corporation, All rights reserved. ==============================//
//
// Purpose: Gets and sets SendTable/DataMap networked properties and caches results.
//
// Code contributions by and used with the permission of L4D2 modders:
// Neil Rao (neilrao42@gmail.com)
// Raymond Nondorf (rayman1103@aol.com)
//==========================================================================================//


#include "cbase.h"
//#include "../../engine/server.h"
#include "netpropmanager.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


extern void SendProxy_StringT_To_String( const SendProp *pProp, const void *pStruct, const void *pVarData, DVariant *pOut, int iElement, int objectID );

//-----------------------------------------------------------------------------
CNetPropManager::~CNetPropManager()
{
	m_PropCache.PurgeAndDeleteElements();
}

//-----------------------------------------------------------------------------
SendProp *CNetPropManager::SearchSendTable( SendTable *pSendTable, const char *pszProperty ) const
{
	// Iterate through the send table and find the prop that we are looking for
	for ( int nPropIdx = 0; nPropIdx < pSendTable->GetNumProps(); nPropIdx++ )
	{
		SendProp *pSendProp = pSendTable->GetProp( nPropIdx );
		const char *pszPropName = pSendProp->GetName();

		// If we found the property, return the prop
		if ( pszPropName && V_strcmp( pszPropName, pszProperty ) == 0 )
			return pSendProp;

		// Search nested tables
		SendTable *pInternalSendTable = pSendProp->GetDataTable();
		if ( pInternalSendTable )
		{
			pSendProp = SearchSendTable( pInternalSendTable, pszProperty );
			if ( pSendProp )
				return pSendProp;
		}
	}

	return NULL;
}


//-----------------------------------------------------------------------------
inline typedescription_t *CNetPropManager::SearchDataMap( datamap_t *pMap, const char *pszProperty ) const
{
	while ( pMap )
	{
		for ( int field = 0; field < pMap->dataNumFields; field++ )
		{
			const char *fieldName = pMap->dataDesc[field].fieldName;
			if ( !fieldName )
				continue;
			
			if ( V_strcmp( pszProperty, fieldName ) == 0 )
				return &pMap->dataDesc[field];
			
			if ( pMap->dataDesc[field].td )
			{
				typedescription_t *td = SearchDataMap( pMap->dataDesc[field].td, pszProperty );
				if ( td )
					return td;
			}
		}

		pMap = pMap->baseMap;
	}
	
	return NULL; 
}


//-----------------------------------------------------------------------------
inline CNetPropManager::PropInfo_t CNetPropManager::GetEntityPropInfo( CBaseEntity* pBaseEntity, const char *pszProperty, int element )
{
	ServerClass *pServerClass       = pBaseEntity->GetServerClass();
	const char  *pszServerClassName = pServerClass->GetName();
	SendTable   *pSendTable         = pServerClass->m_pTable;
	datamap_t   *pDataMap           = pBaseEntity->GetDataDescMap();

	// First, search the cache and see if the property was looked up before
	int classIdx = m_PropCache.Find( pszServerClassName );
	if ( m_PropCache.IsValidIndex( classIdx ) )
	{
		const PropInfoDict_t &properties = *(m_PropCache[ classIdx ]);
		int propIdx = properties.Find( pszProperty );
		if ( element > 0 )
		{
			char pProperty[256];
			V_snprintf( pProperty, sizeof(pProperty), "%s%d", pszProperty, element );
			propIdx = properties.Find( pProperty );
		}
		if ( properties.IsValidIndex( propIdx ) )
			return properties[ propIdx ];
	}

	CUtlStringList szPropertyList;
	char pProperty[256];
	int offset = 0;
	
	typedescription_t *pTypeDesc = NULL;
	SendProp *pSendProp = NULL;

	V_SplitString( pszProperty, ".", szPropertyList );
	
	int nPropertyCount = szPropertyList.Count();
	if ( nPropertyCount )
	{
		// Search the SendTable for the prop, and if not found, search the datamap
		int iProperty = 0;
		int nRootStringLength = 0;

		while ( iProperty < nPropertyCount )
		{
			char *pszSearchProperty = szPropertyList[ iProperty ];

			pSendProp = SearchSendTable( pSendTable, pszSearchProperty );
			if ( !pSendProp )
			{
				// Try the full string remainder as a single property name
				const char *pszPropertyRemainder = pszProperty + nRootStringLength;
				pSendProp = SearchSendTable( pSendTable, pszPropertyRemainder );
				if ( pSendProp )
					offset += pSendProp->GetOffset();
				break;
			}

			// Handle nested properties
			++iProperty;
			offset += pSendProp->GetOffset();
			nRootStringLength += V_strlen( pszSearchProperty ) + iProperty;
		}

		if ( !pSendProp )
		{
			offset = 0;
			iProperty = 0;
			nRootStringLength = 0;

			while ( iProperty < nPropertyCount )
			{
				char *pszSearchProperty = szPropertyList[ iProperty ];

				pTypeDesc = SearchDataMap( pDataMap, pszSearchProperty );
				if ( !pTypeDesc )
				{
					// Try the full string remainder as a single property name
					const char *pszPropertyRemainder = pszProperty + nRootStringLength;
					pTypeDesc = SearchDataMap( pDataMap, pszPropertyRemainder );
					if ( pTypeDesc )
						offset += pTypeDesc->fieldOffset;
					break;
				}

				// handle nested properties
				++iProperty;
				offset += pTypeDesc->fieldOffset;
				nRootStringLength += V_strlen( pszSearchProperty ) + iProperty;
			}
		}
	}

	PropInfo_t  propInfo;
	if ( pSendProp )
	{
		if ( element < 0 )
		{
			propInfo.m_eType = Type_InvalidOrMax;
			propInfo.m_IsPropValid = false;
			return propInfo;
		}

		propInfo.m_nOffset	= offset;
		propInfo.m_nProps	= 0;
			
		if ( (NetPropType)pSendProp->GetType() == Type_DataTable )
		{
			SendTable pArrayTable = *pSendProp->GetDataTable();
			propInfo.m_nProps = pArrayTable.GetNumProps();

			if ( element >= pArrayTable.GetNumProps() )
			{
				propInfo.m_eType = Type_InvalidOrMax;
				propInfo.m_IsPropValid = false;
				return propInfo;
			}
			pSendProp = pArrayTable.GetProp( element );
			propInfo.m_nOffset += pSendProp->GetOffset();
		}

		NetPropType ePropType = (NetPropType)pSendProp->GetType();
		if ( ePropType == Type_String )
		{
			if ( pSendProp->GetProxyFn() != NULL )
			{
				Assert( pSendProp->GetProxyFn() == &SendProxy_StringT_To_String );
				ePropType = Type_String_t;
			}
		}
		propInfo.m_bIsSendProp = true;
		propInfo.m_eType       = ePropType;
		propInfo.m_nBitCount   = pSendProp->m_nBits;
		propInfo.m_nElements   = pSendProp->GetNumElements();
		propInfo.m_nTransFlags = pSendProp->GetFlags();
		propInfo.m_IsPropValid = true;

		if ( propInfo.m_eType == Type_String )
			propInfo.m_nPropLen = DT_MAX_STRING_BUFFERSIZE;
	}
	else if ( pTypeDesc && pTypeDesc->fieldSizeInBytes > 0 )
	{
		if ( element < 0 || element >= pTypeDesc->fieldSize )
		{
			propInfo.m_eType = Type_InvalidOrMax;
			propInfo.m_IsPropValid = false;
			return propInfo;
		}

		propInfo.m_bIsSendProp		= false;
		propInfo.m_IsPropValid		= true;
		propInfo.m_nOffset			= offset + ( element * ( pTypeDesc->fieldSizeInBytes / pTypeDesc->fieldSize ) );
		propInfo.m_nElements		= pTypeDesc->fieldSize;
		propInfo.m_nTransFlags		= pTypeDesc->flags;
		propInfo.m_nProps			= propInfo.m_nElements;

		switch (pTypeDesc->fieldType)
		{
		case FIELD_TICK:
		case FIELD_MODELINDEX:
		case FIELD_MATERIALINDEX:
		case FIELD_INTEGER:
		case FIELD_COLOR32:
			{
				propInfo.m_nBitCount = 32;
				propInfo.m_eType = Type_Int;
				break;
			}
		case FIELD_VECTOR:
		case FIELD_POSITION_VECTOR:
			{
				propInfo.m_nBitCount = 12;
				propInfo.m_eType = Type_Vector;
				break;
			}
		case FIELD_SHORT:
			{
				propInfo.m_nBitCount = 16;
				propInfo.m_eType = Type_Int;
				break;
			}
		case FIELD_BOOLEAN:
			{
				propInfo.m_nBitCount = 1;
				propInfo.m_eType = Type_Int;
				break;
			}
		case FIELD_CHARACTER:
			{
				if (pTypeDesc->fieldSize == 1)
				{
					propInfo.m_nBitCount = 8;
					propInfo.m_eType = Type_Int;
				}
				else
				{
					propInfo.m_nBitCount = 8 * pTypeDesc->fieldSize;
					propInfo.m_eType = Type_String;
				}

				break;
			}
		case FIELD_MODELNAME:
		case FIELD_SOUNDNAME:
		case FIELD_STRING:
			{
				propInfo.m_nBitCount = sizeof(string_t);
				propInfo.m_eType = Type_String_t;
				break;
			}
		case FIELD_FLOAT:
		case FIELD_TIME:
			{
				propInfo.m_nBitCount = 32;
				propInfo.m_eType = Type_Float;
				break;
			}
		case FIELD_EHANDLE:
			{
				propInfo.m_nBitCount = 32;
				propInfo.m_eType = Type_Int;
				break;
			}
		default:
			{
				propInfo.m_IsPropValid = false;
				propInfo.m_eType = Type_InvalidOrMax;
			}
		}

		propInfo.m_nPropLen = pTypeDesc->fieldSize;
	}
	else
	{
		propInfo.m_eType = Type_InvalidOrMax;
		propInfo.m_IsPropValid = false;
		return propInfo;
	}

	// Cache the property
 	if ( !m_PropCache.IsValidIndex( classIdx ) )
	{
		classIdx = m_PropCache.Insert( pszServerClassName, new PropInfoDict_t );
	}
	PropInfoDict_t &properties = *(m_PropCache[ classIdx ]);
	if ( element > 0 )
	{
		V_snprintf( pProperty, sizeof( pProperty ), "%s%d", pszProperty, element );
		properties.Insert( pProperty, propInfo );
	}
	else
	{
		properties.Insert( pszProperty, propInfo );
	}

	return propInfo;
}


//-----------------------------------------------------------------------------
int CNetPropManager::GetPropInt( HSCRIPT hEnt, const char *pszProperty )
{
	return GetPropIntArray( hEnt, pszProperty, 0 );
}


//-----------------------------------------------------------------------------
int CNetPropManager::GetPropIntArray( HSCRIPT hEnt, const char *pszProperty, int element )
{
	// Get the base entity of the specified index
	CBaseEntity *pBaseEntity = ToEnt( hEnt );
	if ( !pBaseEntity )
		return -1;

	// Find the requested property info (this will throw if the entity is
	// invalid, which is exactly what we want)
	PropInfo_t propInfo = GetEntityPropInfo( pBaseEntity, pszProperty, element );

	// Property must be valid
	if ( (!propInfo.m_IsPropValid) || (propInfo.m_eType != Type_Int) )
		return -1;

	void *pBaseEntityOrGameRules = pBaseEntity;
	if ( dynamic_cast<CGameRulesProxy*>(pBaseEntity) && propInfo.m_bIsSendProp )
	{
		pBaseEntityOrGameRules = GameRules();
		if ( !pBaseEntityOrGameRules )
			return -1;
	}

	// All sendprops store an offset from the pointer to the base entity to
	// where the prop data actually is; the reason is because the engine needs
	// to relay data very quickly to all the clients, so it works with
	// offsets to make the ordeal faster
	uint8 *pEntityPropData = (uint8 *)pBaseEntityOrGameRules + propInfo.m_nOffset;
	bool bUnsigned = propInfo.m_nTransFlags & SPROP_UNSIGNED;

	// Thanks to SM for figuring out the types to use for bit counts.
	// All we are doing below is looking at how many bits are in the prop.
	// Since some values can be shorts, longs, ints, signed/unsigned,
	// boolean, etc., so we need to decipher exactly what the SendProp info
	// tells us in order to properly retrieve the right number of bytes.
	if (propInfo.m_nBitCount >= 17)
	{
		return *(int32 *)pEntityPropData;
	}
	else if (propInfo.m_nBitCount >= 9)
	{
		if (bUnsigned)
			return *(uint16 *)pEntityPropData;
		else
			return *(int16 *)pEntityPropData;
	}
	else if (propInfo.m_nBitCount >= 2)
	{
		if (bUnsigned)
			return *(uint8 *)pEntityPropData;
		else
			return *(int8 *)pEntityPropData;
	}
	else
	{
		return *(bool *)(pEntityPropData) ? 1 : 0;
	}

	return 0;
}


//-----------------------------------------------------------------------------
void CNetPropManager::SetPropInt( HSCRIPT hEnt, const char *pszProperty, int value )
{
	SetPropIntArray( hEnt, pszProperty, value, 0 );
}


//-----------------------------------------------------------------------------
void CNetPropManager::SetPropIntArray( HSCRIPT hEnt, const char *pszProperty, int value, int element )
{
	CBaseEntity *pBaseEntity = ToEnt( hEnt );
	if ( !pBaseEntity )
		return;

	PropInfo_t propInfo = GetEntityPropInfo( pBaseEntity, pszProperty, element );

	if ( (!propInfo.m_IsPropValid) || (propInfo.m_eType != Type_Int) )
		return;

	void *pBaseEntityOrGameRules = pBaseEntity;
	if ( dynamic_cast<CGameRulesProxy*>(pBaseEntity) && propInfo.m_bIsSendProp )
	{
		pBaseEntityOrGameRules = GameRules();
		if ( !pBaseEntityOrGameRules )
			return;
	}

	uint8 *pEntityPropData = (uint8 *)pBaseEntityOrGameRules + propInfo.m_nOffset;
	bool bUnsigned = propInfo.m_nTransFlags & SPROP_UNSIGNED;

	if (propInfo.m_nBitCount >= 17)
	{
		*(int32 *)pEntityPropData = (int32)value;
	}
	else if (propInfo.m_nBitCount >= 9)
	{
		if (bUnsigned)
			*(uint16 *)pEntityPropData = (uint16)value;
		else
			*(int16 *)pEntityPropData = (int16)value;
	}
	else if (propInfo.m_nBitCount >= 2)
	{
		if (bUnsigned)
			*(uint8 *)pEntityPropData = (uint8)value;
		else
			*(int8 *)pEntityPropData = (int8)value;
	}
	else
	{
		*(bool *)pEntityPropData = value ? true : false;
	}

	// Network the prop change to connected clients (otherwise the network state won't
	// be updated until the engine re-transmits the entire table)
	if ( propInfo.m_bIsSendProp )
	{
		pBaseEntity->edict()->StateChanged( propInfo.m_nOffset );
	}
}


//-----------------------------------------------------------------------------
float CNetPropManager::GetPropFloat( HSCRIPT hEnt, const char *pszProperty )
{
	return GetPropFloatArray( hEnt, pszProperty, 0 );
}


//-----------------------------------------------------------------------------
float CNetPropManager::GetPropFloatArray( HSCRIPT hEnt, const char *pszProperty, int element )
{
	CBaseEntity *pBaseEntity = ToEnt( hEnt );
	if ( !pBaseEntity )
		return -1.0f;

	PropInfo_t propInfo = GetEntityPropInfo( pBaseEntity, pszProperty, element );

	if ( (!propInfo.m_IsPropValid) || (propInfo.m_eType != Type_Float) )
		return -1.0f;

	void *pBaseEntityOrGameRules = pBaseEntity;
	if ( dynamic_cast<CGameRulesProxy*>(pBaseEntity) && propInfo.m_bIsSendProp )
	{
		pBaseEntityOrGameRules = GameRules();
		if ( !pBaseEntityOrGameRules )
			return -1.0f;
	}

	return *(float *)((uint8 *)pBaseEntityOrGameRules + propInfo.m_nOffset);
}


//-----------------------------------------------------------------------------
void CNetPropManager::SetPropFloat( HSCRIPT hEnt, const char *pszProperty, float value )
{
	SetPropFloatArray( hEnt, pszProperty, value, 0 );
}


//-----------------------------------------------------------------------------
void CNetPropManager::SetPropFloatArray( HSCRIPT hEnt, const char *pszProperty, float value, int element )
{
	CBaseEntity *pBaseEntity = ToEnt( hEnt );
	if ( !pBaseEntity )
		return;

	PropInfo_t propInfo = GetEntityPropInfo( pBaseEntity, pszProperty, element );

	if ( (!propInfo.m_IsPropValid) || (propInfo.m_eType != Type_Float) )
		return;

	void *pBaseEntityOrGameRules = pBaseEntity;
	if ( dynamic_cast<CGameRulesProxy*>(pBaseEntity) && propInfo.m_bIsSendProp )
	{
		pBaseEntityOrGameRules = GameRules();
		if ( !pBaseEntityOrGameRules )
			return;
	}

	*(float *)((uint8 *)pBaseEntityOrGameRules + propInfo.m_nOffset) = value;

	if ( propInfo.m_bIsSendProp )
	{
		pBaseEntity->edict()->StateChanged( propInfo.m_nOffset );
	}
}


//-----------------------------------------------------------------------------
HSCRIPT CNetPropManager::GetPropEntity( HSCRIPT hEnt, const char *pszProperty )
{
	return GetPropEntityArray( hEnt, pszProperty, 0 );
}


//-----------------------------------------------------------------------------
HSCRIPT CNetPropManager::GetPropEntityArray( HSCRIPT hEnt, const char *pszProperty, int element )
{
	CBaseEntity *pBaseEntity = ToEnt( hEnt );
	if ( !pBaseEntity )
		return NULL;

	PropInfo_t propInfo = GetEntityPropInfo( pBaseEntity, pszProperty, element );

	if ( (!propInfo.m_IsPropValid) || (propInfo.m_eType != Type_Int) )
		return NULL;

	void *pBaseEntityOrGameRules = pBaseEntity;
	if ( dynamic_cast<CGameRulesProxy*>(pBaseEntity) && propInfo.m_bIsSendProp )
	{
		pBaseEntityOrGameRules = GameRules();
		if ( !pBaseEntityOrGameRules )
			return NULL;
	}

	CBaseHandle &baseHandle = *(CBaseHandle *)((uint8 *)pBaseEntityOrGameRules + propInfo.m_nOffset);
	CBaseEntity *pPropEntity = CBaseEntity::Instance( baseHandle );

	return ToHScript( pPropEntity );
}


//-----------------------------------------------------------------------------
void CNetPropManager::SetPropEntity( HSCRIPT hEnt, const char *pszProperty, HSCRIPT hPropEnt )
{
	SetPropEntityArray( hEnt, pszProperty, hPropEnt, 0 );
}


//-----------------------------------------------------------------------------
void CNetPropManager::SetPropEntityArray( HSCRIPT hEnt, const char *pszProperty, HSCRIPT hPropEnt, int element )
{
	CBaseEntity *pBaseEntity = ToEnt( hEnt );
	if ( !pBaseEntity )
		return;

	PropInfo_t propInfo = GetEntityPropInfo( pBaseEntity, pszProperty, element );

	if ( (!propInfo.m_IsPropValid) || (propInfo.m_eType != Type_Int) )
		return;

	void *pBaseEntityOrGameRules = pBaseEntity;
	if ( dynamic_cast<CGameRulesProxy*>(pBaseEntity) && propInfo.m_bIsSendProp )
	{
		pBaseEntityOrGameRules = GameRules();
		if ( !pBaseEntityOrGameRules )
			return;
	}

	CBaseHandle &baseHandle = *(CBaseHandle *)((uint8 *)pBaseEntityOrGameRules + propInfo.m_nOffset);

	CBaseEntity *pOtherEntity = ToEnt( hPropEnt );
	if ( !pOtherEntity )
	{
		baseHandle.Set( NULL );
	}
	else
	{
		baseHandle.Set( (IHandleEntity *)pOtherEntity );
	}

	if ( propInfo.m_bIsSendProp )
	{
		pBaseEntity->edict()->StateChanged( propInfo.m_nOffset );
	}
}


//-----------------------------------------------------------------------------
const Vector& CNetPropManager::GetPropVector( HSCRIPT hEnt, const char *pszProperty )
{
	return GetPropVectorArray( hEnt, pszProperty, 0 );
}


//-----------------------------------------------------------------------------
const Vector& CNetPropManager::GetPropVectorArray( HSCRIPT hEnt, const char *pszProperty, int element )
{
	static Vector vAng = Vector(0, 0, 0);
	CBaseEntity *pBaseEntity = ToEnt( hEnt );
	if ( !pBaseEntity )
		return vAng;

	PropInfo_t propInfo = GetEntityPropInfo( pBaseEntity, pszProperty, element );

	if ( (!propInfo.m_IsPropValid) || (propInfo.m_eType != Type_Vector) )
		return vAng;

	void *pBaseEntityOrGameRules = pBaseEntity;
	if ( dynamic_cast<CGameRulesProxy*>(pBaseEntity) && propInfo.m_bIsSendProp )
	{
		pBaseEntityOrGameRules = GameRules();
		if ( !pBaseEntityOrGameRules )
			return vAng;
	}

	vAng = *(Vector *)((uint8 *)pBaseEntityOrGameRules + propInfo.m_nOffset);
	return vAng;
}


//-----------------------------------------------------------------------------
void CNetPropManager::SetPropVector( HSCRIPT hEnt, const char *pszProperty, Vector value )
{
	SetPropVectorArray( hEnt, pszProperty, value, 0 );
}


//-----------------------------------------------------------------------------
void CNetPropManager::SetPropVectorArray( HSCRIPT hEnt, const char *pszProperty, Vector value, int element )
{
	CBaseEntity *pBaseEntity = ToEnt( hEnt );
	if ( !pBaseEntity )
		return;

	PropInfo_t propInfo = GetEntityPropInfo( pBaseEntity, pszProperty, element );

	if ( (!propInfo.m_IsPropValid) || (propInfo.m_eType != Type_Vector) )
		return;

	void *pBaseEntityOrGameRules = pBaseEntity;
	if ( dynamic_cast<CGameRulesProxy*>(pBaseEntity) && propInfo.m_bIsSendProp )
	{
		pBaseEntityOrGameRules = GameRules();
		if ( !pBaseEntityOrGameRules )
			return;
	}

	Vector *pVec = (Vector *)((uint8 *)pBaseEntityOrGameRules + propInfo.m_nOffset);
	pVec->x = value.x;
	pVec->y = value.y;
	pVec->z = value.z;

	if ( propInfo.m_bIsSendProp )
	{
		pBaseEntity->edict()->StateChanged( propInfo.m_nOffset );
	}
}


//-----------------------------------------------------------------------------
const char *CNetPropManager::GetPropString( HSCRIPT hEnt, const char *pszProperty )
{
	return GetPropStringArray( hEnt, pszProperty, 0 );
}


//-----------------------------------------------------------------------------
const char *CNetPropManager::GetPropStringArray( HSCRIPT hEnt, const char *pszProperty, int element )
{
	CBaseEntity *pBaseEntity = ToEnt( hEnt );
	if ( !pBaseEntity )
		return "";

	PropInfo_t propInfo = GetEntityPropInfo( pBaseEntity, pszProperty, element );

	if ( (!propInfo.m_IsPropValid) || (propInfo.m_eType != Type_String && propInfo.m_eType != Type_String_t && propInfo.m_eType != Type_Int) )
		return "";

	void *pBaseEntityOrGameRules = pBaseEntity;
	if ( dynamic_cast<CGameRulesProxy*>(pBaseEntity) && propInfo.m_bIsSendProp )
	{
		pBaseEntityOrGameRules = GameRules();
		if ( !pBaseEntityOrGameRules )
			return "";
	}

	if ( propInfo.m_eType == Type_String_t )
	{
		string_t propString = *(string_t *)((uint8 *)pBaseEntityOrGameRules + propInfo.m_nOffset);
		return (propString == NULL_STRING) ? "" : STRING(propString);
	}
	else
	{
		return (const char *)((uint8 *)pBaseEntityOrGameRules + propInfo.m_nOffset);
	}
}


//-----------------------------------------------------------------------------
void CNetPropManager::SetPropString( HSCRIPT hEnt, const char *pszProperty, const char *value )
{
	SetPropStringArray( hEnt, pszProperty, value, 0 );
}


//-----------------------------------------------------------------------------
void CNetPropManager::SetPropStringArray( HSCRIPT hEnt, const char *pszProperty, const char *value, int element )
{
	CBaseEntity *pBaseEntity = ToEnt( hEnt );
	if ( !pBaseEntity )
		return;

	PropInfo_t propInfo = GetEntityPropInfo( pBaseEntity, pszProperty, element );

	if ( (!propInfo.m_IsPropValid) || (propInfo.m_eType != Type_String && propInfo.m_eType != Type_String_t) )
		return;

	void *pBaseEntityOrGameRules = pBaseEntity;
	if ( dynamic_cast<CGameRulesProxy*>(pBaseEntity) && propInfo.m_bIsSendProp )
	{
		pBaseEntityOrGameRules = GameRules();
		if ( !pBaseEntityOrGameRules )
			return;
	}

	if ( propInfo.m_eType == Type_String_t )
	{
		*(string_t *)((uint8 *)pBaseEntityOrGameRules + propInfo.m_nOffset) = AllocPooledString(value);
	}
	else
	{
		char* strDest = (char *)((uint8 *)pBaseEntityOrGameRules + propInfo.m_nOffset);
		V_strncpy( strDest, value, propInfo.m_nPropLen );
	}

	if ( propInfo.m_bIsSendProp )
	{
		pBaseEntity->edict()->StateChanged( propInfo.m_nOffset );
	}
}


//-----------------------------------------------------------------------------
int CNetPropManager::GetPropArraySize( HSCRIPT hEnt, const char *pszProperty )
{
	CBaseEntity *pBaseEntity = ToEnt( hEnt );
	if (!pBaseEntity)
		return -1;

	PropInfo_t propInfo = GetEntityPropInfo( pBaseEntity, pszProperty, 0 );

	if ( (!propInfo.m_IsPropValid) )
		return -1;

	return propInfo.m_nProps;
}


//-----------------------------------------------------------------------------
bool CNetPropManager::HasProp( HSCRIPT hEnt, const char *pszProperty )
{
	CBaseEntity *pBaseEntity = ToEnt( hEnt );
	if ( !pBaseEntity )
		return false;

	PropInfo_t propInfo = GetEntityPropInfo( pBaseEntity, pszProperty, 0 );

	return ( propInfo.m_IsPropValid ) ? true : false;
}


//-----------------------------------------------------------------------------
const char *CNetPropManager::GetPropType( HSCRIPT hEnt, const char *pszProperty )
{
	CBaseEntity *pBaseEntity = ToEnt( hEnt );
	if ( !pBaseEntity )
		return NULL;

	PropInfo_t propInfo = GetEntityPropInfo( pBaseEntity, pszProperty, 0 );

	if ( !propInfo.m_IsPropValid )
		return NULL;

	if ( propInfo.m_eType == Type_Int )
	{
		return "integer";
	}
	else if ( propInfo.m_eType == Type_Float )
	{
		return "float";
	}
	else if ( propInfo.m_eType == Type_Vector )
	{
		return "Vector";
	}
	else if ( propInfo.m_eType == Type_VectorXY )
	{
		return "VectorXY";
	}
	else if ( propInfo.m_eType == Type_String || propInfo.m_eType == Type_String_t )
	{
		return "string";
	}
	else if ( propInfo.m_eType == Type_Array )
	{
		return "array";
	}
	else if ( propInfo.m_eType == Type_DataTable )
	{
		return "table";
	}
	#ifdef SUPPORTS_INT64
		else if ( propInfo.m_eType == Type_Int64 )
		{
			return "integer64";
		}
	#endif

	return NULL;
}