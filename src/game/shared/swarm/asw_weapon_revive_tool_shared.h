#pragma once

#ifdef CLIENT_DLL
#include "c_asw_weapon.h"
#define CASW_Weapon_Revive_Tool C_ASW_Weapon_Revive_Tool
#else
#include "asw_weapon.h"
#endif

#ifdef RD_7A_WEAPONS
class CASW_Weapon_Revive_Tool : public CASW_Weapon
{
public:
	DECLARE_CLASS( CASW_Weapon_Revive_Tool, CASW_Weapon );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

#ifdef CLIENT_DLL
#else
	DECLARE_DATADESC();
#endif

	void SecondaryAttack() override {}
	bool IsOffensiveWeapon() override { return false; }

	Class_T Classify() override { return ( Class_T )CLASS_ASW_REVIVE_TOOL; }
};
#endif
