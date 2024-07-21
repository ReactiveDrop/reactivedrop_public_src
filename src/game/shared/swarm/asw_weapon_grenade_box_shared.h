#pragma once

#ifdef CLIENT_DLL
#include "c_asw_weapon.h"
#define CASW_Weapon C_ASW_Weapon
#define CASW_Weapon_Grenade_Box C_ASW_Weapon_Grenade_Box
#define CASW_Marine C_ASW_Marine
#else
#include "asw_weapon.h"
#endif

class CASW_Weapon_Grenade_Box : public CASW_Weapon
{
public:
	DECLARE_CLASS( CASW_Weapon_Grenade_Box, CASW_Weapon );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CASW_Weapon_Grenade_Box();
	virtual ~CASW_Weapon_Grenade_Box();

#ifdef CLIENT_DLL
#else
	DECLARE_DATADESC();

	int CapabilitiesGet() override { return bits_CAP_WEAPON_RANGE_ATTACK1; }
#endif

	void Precache() override;
	bool Reload() override { return false; } // you can't reload a box
	bool OffhandActivate() override;
	void PrimaryAttack() override;
	void Equip( CBaseCombatCharacter *pOwner ) override;
	void ItemPostFrame() override;
	bool ShouldMarineMoveSlow() override { return false; }
	bool IsOffensiveWeapon() override { return false; }
	const Vector &GetBulletSpread() override { return VECTOR_CONE_PRECALCULATED; }
	int AmmoClickPoint() override { return 0; }

	float m_flSoonestPrimaryAttack;
};
