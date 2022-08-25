#ifndef _INCLUDED_ASW_MAP_RESET_FILTER_H
#define _INCLUDED_ASW_MAP_RESET_FILTER_H

#include "mapentities.h"
#include "utlsymbol.h"

class CASW_Map_Reset_Filter : public IMapEntityFilter
{
public:
	CASW_Map_Reset_Filter();
	virtual ~CASW_Map_Reset_Filter();

	virtual bool ShouldCreateEntity( const char *pClassname );
	virtual CBaseEntity *CreateNextEntity( const char *pClassname );
	void AddKeepEntity( const char * );

private:
	CUtlSymbolTable m_pKeepEntities;
};

#endif // _INCLUDED_ASW_MAP_RESET_FILTER_H
