#ifndef RD_DIRECTOR_TRIGGERS_H
#define RD_DIRECTOR_TRIGGERS_H
#ifdef _WIN32
#pragma once
#endif

#include "util_shared.h"

DECLARE_AUTO_LIST( IRD_No_Director_Aliens );

class CRD_No_Director_Aliens : public CServerOnlyEntity, public IRD_No_Director_Aliens
{
public:
	DECLARE_CLASS( CRD_No_Director_Aliens, CServerOnlyEntity );
	DECLARE_DATADESC();
	IMPLEMENT_AUTO_LIST_GET();

	void Spawn() override;

	void InputEnable( inputdata_t & inputdata );
	void InputDisable( inputdata_t & inputdata );
	void InputToggle( inputdata_t & inputdata );

	bool m_bDisabled;
};

#endif // RD_DIRECTOR_TRIGGERS_H