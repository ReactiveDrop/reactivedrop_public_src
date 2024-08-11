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

	CASW_Weapon_Plasma_Thrower();

#ifdef CLIENT_DLL
	DECLARE_PREDICTABLE();

	const char *GetTracerEffectName() override { return "tracer_egon2"; }
	const char *GetMuzzleEffectName() override { return "muzzle_egon2"; }
	bool ShouldShowLaserPointer() override;
	void ModifyCrosshairPos( int &x, int &y ) override;
#else
	DECLARE_DATADESC();
#endif

	const char *GetMagazineGibModelName() const override { return "models/weapons/empty_clips/flamethrower_empty_clip.mdl"; }
	int GetMagazineGibModelSkin() const override { return 1; }

	// flMagazineProgress (from 0 to 1) is the portion of the magazine fired in the current burst.
	static Vector ComputeRecoil( float flMagazineProgress, bool bMoving, bool bCrouching );

	void Precache() override;
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	void OnStoppedFiring() override;
	bool SecondaryAttackUsesPrimaryAmmo() override { return true; }
	float GetWeaponBaseDamageOverride() override;
	int GetWeaponSkillId() override;
	int GetWeaponSubSkillId() override;

	Class_T Classify() override { return ( Class_T )CLASS_ASW_PLASMA_THROWER; }

	CNetworkVar( int, m_iShotsInCurrentBurst );
};
#endif
