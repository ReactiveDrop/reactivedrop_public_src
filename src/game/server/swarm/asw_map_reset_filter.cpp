#include "cbase.h"
#include "asw_map_reset_filter.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CASW_Map_Reset_Filter::CASW_Map_Reset_Filter()
{
}

CASW_Map_Reset_Filter::~CASW_Map_Reset_Filter()
{
}
 
bool CASW_Map_Reset_Filter::ShouldCreateEntity( const char *pszClassname )
{
	// Returns true if the entity isn't in our list of entities to keep around
	return !m_pKeepEntities.Find( pszClassname ).IsValid();
}

CBaseEntity *CASW_Map_Reset_Filter::CreateNextEntity( const char *pszClassname )
{
	return CreateEntityByName( pszClassname );
}

void CASW_Map_Reset_Filter::AddKeepEntity( const char *pszClassname )
{
	m_pKeepEntities.AddString( pszClassname );
}
