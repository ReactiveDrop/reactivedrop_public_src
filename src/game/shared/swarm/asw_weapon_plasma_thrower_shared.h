#pragma once

#ifdef CLIENT_DLL
#include "c_asw_weapon.h"
#define CASW_Weapon_Plasma_Thrower C_ASW_Weapon_Plasma_Thrower
#else
#include "asw_weapon.h"
#endif

#ifdef RD_7A_WEAPONS
class CASW_Weapon_Plasma_Thrower : public CASW_Weapon
{
public:
	DECLARE_CLASS( CASW_Weapon_Plasma_Thrower, CASW_Weapon );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

#ifdef CLIENT_DLL
#else
	DECLARE_DATADESC();
#endif

	bool SecondaryAttackUsesPrimaryAmmo() override { return true; }

	Class_T Classify() override { return ( Class_T )CLASS_ASW_PLASMA_THROWER; }
};
#endif
