#pragma once

#ifdef CLIENT_DLL
#include "c_asw_weapon_rifle.h"
#define CASW_Weapon_Rifle C_ASW_Weapon_Rifle
#define CASW_Weapon_Energy_Shield C_ASW_Weapon_Energy_Shield
#else
#include "asw_weapon_rifle.h"
#endif

#ifdef RD_7A_WEAPONS
class CASW_Weapon_Energy_Shield : public CASW_Weapon_Rifle
{
public:
	DECLARE_CLASS( CASW_Weapon_Energy_Shield, CASW_Weapon );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

#ifdef CLIENT_DLL
	const char *GetTracerEffectName() override { return "tracer_shieldrifle"; }	// particle effect name
	const char *GetMuzzleEffectName() override { return "muzzle_shieldrifle"; }	// particle effect name
	bool HasSecondaryExplosive( void ) const override { return false; }
#else
	DECLARE_DATADESC();
#endif

	void Precache() override;
	void SecondaryAttack() override;
	const char *GetMagazineGibModelName() const override { return "models/weapons/empty_clips/shieldrifle_empty_clip.mdl"; }
	const Vector &GetBulletSpread( void ) override
	{
		static const Vector cone = VECTOR_CONE_1DEGREES;
		return cone;
	}

	Class_T Classify() override { return ( Class_T )CLASS_ASW_ENERGY_SHIELD; }
};
#endif
