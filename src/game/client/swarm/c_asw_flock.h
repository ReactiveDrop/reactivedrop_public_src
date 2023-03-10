#pragma once

#include "c_asw_alien.h"

#ifdef RD_7A_ENEMIES
class C_ASW_Flock : public C_ASW_Alien
{
public:
	DECLARE_CLASS( C_ASW_Flock, C_ASW_Alien );
	DECLARE_CLIENTCLASS();

	Class_T Classify() override { return ( Class_T )CLASS_ASW_FLOCK; }
};
#endif
