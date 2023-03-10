#pragma once

#ifdef CLIENT_DLL
#include "c_asw_weapon.h"
#define CASW_Weapon_Energy_Shield C_ASW_Weapon_Energy_Shield
#else
#include "asw_weapon.h"
#endif

#ifdef RD_7A_WEAPONS
class CASW_Weapon_Energy_Shield : public CASW_Weapon
{
public:
	DECLARE_CLASS( CASW_Weapon_Energy_Shield, CASW_Weapon );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

#ifdef CLIENT_DLL
#else
	DECLARE_DATADESC();
#endif

	Class_T Classify() override { return ( Class_T )CLASS_ASW_ENERGY_SHIELD; }
};
#endif
