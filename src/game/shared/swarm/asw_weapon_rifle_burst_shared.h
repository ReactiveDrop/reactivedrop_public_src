#pragma once

#ifdef CLIENT_DLL
#include "c_asw_weapon_rifle.h"
#define CASW_Weapon_Rifle C_ASW_Weapon_Rifle
#define CASW_Weapon_Rifle_Burst C_ASW_Weapon_Rifle_Burst
#else
#include "asw_weapon_rifle.h"
#endif

#ifdef RD_7A_WEAPONS
class CASW_Weapon_Rifle_Burst : public CASW_Weapon_Rifle
{
public:
	DECLARE_CLASS( CASW_Weapon_Rifle_Burst, CASW_Weapon );
	DECLARE_NETWORKCLASS();

	CASW_Weapon_Rifle_Burst();
	virtual ~CASW_Weapon_Rifle_Burst();

#ifdef CLIENT_DLL
	DECLARE_PREDICTABLE();

	virtual const char *GetTracerEffectName( int iShot ) { return BaseClass::GetTracerEffectName(); }
	virtual const char *GetMuzzleEffectName( int iShot ) { return BaseClass::GetMuzzleEffectName(); }
	const char *GetTracerEffectName() override final;
	const char *GetMuzzleEffectName() override final;

	int m_iTracerShot;
	int m_iMuzzleShot;
#else
	DECLARE_DATADESC();
#endif

	void PrimaryAttack() override;
	void DelayedAttack() override;
	void ItemPostFrame() override;
	float GetFireRate() override;
	virtual float GetBurstFireRate();
	virtual int GetBurstCount() const { return 1; }
	virtual float GetBurstRestRatio() const { return 0.0f; }
	virtual bool HolsterCancelsBurstFire() const { return false; }
	virtual void OnStartedBurst() {}
	virtual void OnEndedBurst() {}
	bool CanHolster() override;
	void ClearIsFiring() override;

	CNetworkVar( int, m_iBurstShots );
};
#endif
