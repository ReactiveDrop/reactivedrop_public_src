#include "cbase.h"
#include "info_player_start.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

BEGIN_DATADESC( CBaseStart )

DEFINE_KEYFIELD( m_nMarineProfile, FIELD_INTEGER, "marineprofile" ),

END_DATADESC()

LINK_ENTITY_TO_CLASS( info_player_start, CBaseStart );

CBaseStart::CBaseStart()
{
	m_nMarineProfile = -1;
	m_bUsed = false;
}

bool CBaseStart::KeyValue( const char* szKeyName, const char* szValue )
{
	if ( FStrEq( szKeyName, "marineprofile" ) )
	{
		m_nMarineProfile = atoi( szValue );
	}
	else
		return BaseClass::KeyValue( szKeyName, szValue );

	return true;
}