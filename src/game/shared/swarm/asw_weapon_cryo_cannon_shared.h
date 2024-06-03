#pragma once

#ifdef CLIENT_DLL
#include "c_asw_weapon.h"
#define CASW_Weapon_Cryo_Cannon C_ASW_Weapon_Cryo_Cannon
#else
#include "asw_weapon.h"
#endif

#ifdef RD_7A_WEAPONS
class CASW_Weapon_Cryo_Cannon : public CASW_Weapon
{
public:
	DECLARE_CLASS( CASW_Weapon_Cryo_Cannon, CASW_Weapon );
	DECLARE_NETWORKCLASS();

	CASW_Weapon_Cryo_Cannon();

	void Precache() override;
	void Spawn() override;

#ifdef CLIENT_DLL
	DECLARE_PREDICTABLE();

	void ClientThink() override;
	void UpdateSpinningBarrel();
	void UpdateFiringEffects();

	void OnDataChanged( DataUpdateType_t updateType ) override;
	void SetDormant( bool bDormant ) override;
	void UpdateOnRemove() override;
	bool HasSecondaryExplosive() const override { return false; }
#else
	DECLARE_DATADESC();
#endif

	const Vector &GetBulletSpread() override
	{
		static const Vector cone = VECTOR_CONE_PRECALCULATED;

		return cone;
	}

	enum
	{
		EXTINGUISHER_PROJECTILE_AIR_VELOCITY = 400,
	};

	void PrimaryAttack() override;
	void SecondaryAttack() override;
	void ItemPostFrame() override;
	void ItemBusyFrame() override;
	void UpdateSpinRate();
	bool Holster( CBaseCombatWeapon *pSwitchingTo ) override;
	void Drop( const Vector &vecVelocity ) override;
	float GetWeaponBaseDamageOverride() override;
	int GetWeaponSkillId() override;
	int GetWeaponSubSkillId() override;
	float GetMovementScale() override;
	bool ShouldMarineMoveSlow() override;

	float GetSpinRate() { return m_flSpinRate.Get(); }

	bool ShouldPlayFiringAnimations() override { return false; }
	bool HasSecondaryAttack() override { return false; } // secondary fire is spin-up
	Class_T Classify() override { return ( Class_T )CLASS_ASW_CRYO_CANNON; }

	CNetworkVar( float, m_flSpinRate );
#ifdef CLIENT_DLL
	CSoundPatch *m_pBarrelSpinSound;
	CSoundPatch *m_pFireSound;
	bool m_bShouldUpdateActivityClient;
	CUtlReference<CNewParticleEffect> m_hEffect;
#endif
};
#endif
