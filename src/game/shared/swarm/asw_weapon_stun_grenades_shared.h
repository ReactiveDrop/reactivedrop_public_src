#pragma once

#include "asw_weapon_grenade_box_shared.h"
#ifdef CLIENT_DLL
#define CASW_Weapon_Stun_Grenades C_ASW_Weapon_Stun_Grenades
#endif

#ifdef RD_7A_WEAPONS
class CASW_Weapon_Stun_Grenades : public CASW_Weapon_Grenade_Box
{
public:
	DECLARE_CLASS( CASW_Weapon_Stun_Grenades, CASW_Weapon_Grenade_Box );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

#ifdef CLIENT_DLL
#else
	DECLARE_DATADESC();
#endif

	void Precache() override;

	float GetFireRate() override { return 1.4f; }

	float GetThrowGravity() override { return 2.0f; }

	void DelayedAttack() override;

	Class_T Classify() override { return ( Class_T )CLASS_ASW_GRENADE_BOX_PRIFLE; }
};
#endif
