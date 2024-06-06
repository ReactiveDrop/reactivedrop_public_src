#pragma once

#ifdef RD_7A_WEAPONS
#include "asw_weapon_rifle_burst_shared.h"
#ifdef CLIENT_DLL
#define CASW_Weapon_Energy_Shield C_ASW_Weapon_Energy_Shield
#define CASW_Energy_Shield C_ASW_Energy_Shield
#define CASW_Marine C_ASW_Marine
#endif

class CASW_Energy_Shield;
class CASW_Marine;

class CASW_Weapon_Energy_Shield : public CASW_Weapon_Rifle_Burst
{
public:
	DECLARE_CLASS( CASW_Weapon_Energy_Shield, CASW_Weapon_Rifle_Burst );
	DECLARE_NETWORKCLASS();

	CASW_Weapon_Energy_Shield();
	virtual ~CASW_Weapon_Energy_Shield();

#ifdef CLIENT_DLL
	DECLARE_PREDICTABLE();

	const char *GetTracerEffectName( int iShot ) override;
	const char *GetMuzzleEffectName( int iShot ) override;
	bool HasSecondaryExplosive() const override { return false; }
	void OnDataChanged( DataUpdateType_t updateType ) override;
	void ClientThink() override;

	int m_iShieldPoseParameter{ -2 };
#else
	DECLARE_DATADESC();

	void DestroyShield();
#endif

	void Precache() override;
	void SecondaryAttack() override;
	int GetBurstCount() const override;
	float GetBurstRestRatio() const override;
	bool CanHolster() override;
	bool HolsterCancelsBurstFire() const override;
	void UpdateOnRemove() override;
	bool Holster( CBaseCombatWeapon *pSwitchingTo ) override;
	void Drop( const Vector &vecVelocity ) override;
	float GetWeaponBaseDamageOverride() override;
	int GetWeaponSkillId() override;
	int GetWeaponSubSkillId() override;
	const char *GetMagazineGibModelName() const override { return "models/weapons/empty_clips/shieldrifle_empty_clip.mdl"; }
	const Vector &GetBulletSpread() override
	{
		static const Vector cone = VECTOR_CONE_1DEGREES;
		return cone;
	}

	CNetworkHandle( CASW_Energy_Shield, m_hShield );

	Class_T Classify() override { return ( Class_T )CLASS_ASW_ENERGY_SHIELD; }
};

class CASW_Energy_Shield : public CBaseAnimating
{
public:
	DECLARE_CLASS( CASW_Energy_Shield, CBaseAnimating );
	DECLARE_NETWORKCLASS();

	CASW_Energy_Shield();
	virtual ~CASW_Energy_Shield();

#ifdef CLIENT_DLL
	DECLARE_PREDICTABLE();

	void OnDataChanged( DataUpdateType_t updateType ) override;
	void ClientThink() override;
#else
	DECLARE_DATADESC();

	void Spawn() override;
	bool CreateVPhysics() override;
	void StartTouch( CBaseEntity *pOther ) override;
	void EndTouch( CBaseEntity *pOther ) override;
	void TouchThink();
	void OnProjectileHit( CBaseEntity *pProjectile );

	CUtlVector<EHANDLE> m_vecTouching;
#endif

	void Precache() override;

	Class_T Classify() override { return ( Class_T )CLASS_ASW_ENERGY_SHIELD_SHIELD; }

	CNetworkHandle( CASW_Marine, m_hCreatorMarine );
	CNetworkHandle( CASW_Weapon_Energy_Shield, m_hCreatorWeapon );
	CNetworkVar( float, m_flLastHitTime );
	CNetworkVar( float, m_flExpireTime );
};
#endif
