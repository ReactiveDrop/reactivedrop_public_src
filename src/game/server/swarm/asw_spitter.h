#pragma once

#include "asw_alien.h"

#ifdef RD_7A_ENEMIES
class CASW_Spitter : public CASW_Alien
{
public:
	DECLARE_CLASS( CASW_Spitter, CASW_Alien );
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

	Class_T Classify() override { return ( Class_T )CLASS_ASW_SPITTER; }
};
#endif
