#ifndef _INCLUDED_ASW_WEAPON_MINIGUN_H
#define _INCLUDED_ASW_WEAPON_MINIGUN_H
#pragma once

#ifdef CLIENT_DLL
	#define CASW_Weapon_Minigun C_ASW_Weapon_Minigun
	#define CASW_Weapon_Rifle C_ASW_Weapon_Rifle
	#define CASW_Marine C_ASW_Marine
	#include "c_asw_weapon_rifle.h"

class C_ASW_Gun_Smoke_Emitter;
#else
	#include "npc_combine.h"
	#include "asw_weapon_rifle.h"
#endif

extern ConVar rd_ground_shooting;

class CASW_Weapon_Minigun : public CASW_Weapon_Rifle
{
public:
	DECLARE_CLASS( CASW_Weapon_Minigun, CASW_Weapon_Rifle );
	DECLARE_NETWORKCLASS();
	DECLARE_ENT_SCRIPTDESC();

	CASW_Weapon_Minigun();
	virtual ~CASW_Weapon_Minigun();
	void Precache();

	virtual void PrimaryAttack();
	//float	GetFireRate( void ) { return 0.07f; }
	virtual void FireBullets( CASW_Marine* pMarine, const FireBulletsInfo_t& info );

	virtual const float GetAutoAimAmount() { return 0.0f; }
	virtual const float GetAutoAimRadiusScale() { return 1.0f; }
	virtual const Vector& GetBulletSpread( void );
	virtual void ItemPostFrame();
	virtual void ItemBusyFrame();
	bool Holster( CBaseCombatWeapon *pSwitchingTo );
	virtual void UpdateSpinRate();

	#ifndef CLIENT_DLL
		DECLARE_DATADESC();

		virtual void Spawn();
		virtual void SecondaryAttack();
		virtual float GetMadFiringBias() { return 1.0f; }	// scales the rate at which the mad firing counter goes up when we shoot aliens with this weapon
		virtual void Drop( const Vector &vecVelocity );

	#else
		DECLARE_PREDICTABLE();

		virtual bool ShouldMarineMinigunShoot();
		virtual bool HasSecondaryExplosive( void ) const { return false; }
		virtual float GetMuzzleFlashScale();
		virtual Vector GetMuzzleFlashTint();
		virtual void OnMuzzleFlashed();
		virtual void ReachedEndOfSequence();
		float m_flLastMuzzleFlashTime;
		virtual const char* GetPartialReloadSound(int iPart);

		virtual void ClientThink();
		void UpdateSpinningBarrel();

		virtual const char* GetTracerEffectName() { return "tracer_minigun"; }

		// gunsmoke
		virtual void OnDataChanged( DataUpdateType_t updateType );
		virtual void UpdateOnRemove();
		virtual void SetDormant( bool bDormant );
		void CreateGunSmoke();
		CHandle<C_ASW_Gun_Smoke_Emitter> m_hGunSmoke;

		CSoundPatch		*m_pBarrelSpinSound;
	#endif
	virtual const char *GetMagazineGibModelName() const override { return "models/weapons/empty_clips/autogun_empty_clip.mdl"; }
	virtual float GetWeaponDamage();
	virtual float GetMovementScale();
	virtual bool ShouldMarineMoveSlow();
	virtual bool SupportsBayonet() { return false; }
	virtual void OnStoppedFiring();
	float m_flTimeFireStarted;

	virtual bool SupportsGroundShooting() { return rd_ground_shooting.GetBool(); }

	float GetSpinRate() { return m_flSpinRate.Get(); }

	CNetworkVar( bool, m_bHalfShot );
	CNetworkVar( float, m_flSpinRate );		// barrel spin speed

	// Classification
	virtual Class_T		Classify( void ) { return (Class_T) CLASS_ASW_MINIGUN; }

	virtual int DisplayClip1() override;
	virtual int DisplayMaxClip1() override;
	int ScriptClip1();
	int ScriptGetMaxClip1();
	int ScriptGetDefaultClip1();
	int ScriptGetMaxAmmo1();
#ifdef GAME_DLL
	void ScriptSetClip1( int iAmmo );
#endif
	virtual void FinishReload() override;
private:
#ifdef CLIENT_DLL
	bool m_bShouldUpdateActivityClient;
#endif
};


#endif /* _INCLUDED_ASW_WEAPON_MINIGUN_H */
