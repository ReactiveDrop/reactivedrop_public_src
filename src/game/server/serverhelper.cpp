#include "cbase.h"
#include "serverhelper.h"

// helper class to find the offset of a datamap for a specific edict
inline unsigned int FindPropertyOffsetInDataMap( datamap_t* pMap, const char* className, const char* property )
{
	while ( pMap )
	{
		for ( int i = 0; i < pMap->dataNumFields; i++ )
		{
			const typedescription_t info = pMap->dataDesc[i];
			if ( !info.fieldName ) continue;

			if ( Q_strcmp(className, pMap->dataClassName ) == 0 && Q_strcmp( property, info.fieldName) == 0 ) return info.fieldOffset;
			if ( info.td )
			{
				const unsigned int offset = FindPropertyOffsetInDataMap( info.td, className, property );
				if ( offset > 0 ) return offset;
			}
		}

		pMap = pMap->baseMap;
	}
	return 0;
}

unsigned int CServerHelper::GetDataMapOffsetForEdict( edict_t* pEdict, const char* className, const char* property )
{
	IServerUnknown* pUnknown = ( IServerUnknown* )pEdict->GetUnknown();
	if ( !pUnknown ) return 0;

	CBaseEntity* pEntity = pUnknown->GetBaseEntity();
	return FindPropertyOffsetInDataMap( pEntity->GetDataDescMap(), className, property );
}
