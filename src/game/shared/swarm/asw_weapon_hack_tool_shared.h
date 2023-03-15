#pragma once

#ifdef CLIENT_DLL
#include "c_asw_weapon.h"
#define CASW_Weapon_Hack_Tool C_ASW_Weapon_Hack_Tool
#else
#include "asw_weapon.h"
#endif

#ifdef RD_7A_WEAPONS
class CASW_Weapon_Hack_Tool : public CASW_Weapon
{
public:
	DECLARE_CLASS( CASW_Weapon_Hack_Tool, CASW_Weapon );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

#ifdef CLIENT_DLL
#else
	DECLARE_DATADESC();
#endif

	// attacking with this does nothing
	void PrimaryAttack() override {}
	void SecondaryAttack() override {}
	bool IsOffensiveWeapon() override { return false; }

	Class_T Classify() override { return ( Class_T )CLASS_ASW_HACK_TOOL; }
};
#endif
