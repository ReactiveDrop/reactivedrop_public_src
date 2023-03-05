#ifndef _DEFINED_ASW_WEAPON_SNIPER_RIFLE_H
#define _DEFINED_ASW_WEAPON_SNIPER_RIFLE_H
#pragma once

#include "asw_shareddefs.h"
#ifdef CLIENT_DLL
#include "c_asw_weapon.h"
#define CASW_Weapon C_ASW_Weapon
#define CASW_Weapon_Sniper_Rifle C_ASW_Weapon_Sniper_Rifle
#define CASW_Marine C_ASW_Marine
#else
#include "asw_weapon.h"
#include "npc_combine.h"
#endif

#include "basegrenade_shared.h"

class CASW_Weapon_Sniper_Rifle : public CASW_Weapon
{
public:
	DECLARE_CLASS( CASW_Weapon_Sniper_Rifle, CASW_Weapon );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CASW_Weapon_Sniper_Rifle();
	virtual ~CASW_Weapon_Sniper_Rifle();
	void Precache();

	virtual void ItemPostFrame();
	virtual void ItemBusyFrame();
	virtual void UpdateZoomState();

	Activity	GetPrimaryAttackActivity( void ) { return ACT_VM_PRIMARYATTACK; }
	virtual bool ShouldMarineMoveSlow();
	virtual float GetMovementScale();
	virtual float GetFireRate();
	virtual bool IsZoomed() { return m_bZoomed.Get(); }

	void	PrimaryAttack();
	virtual float GetWeaponDamage();
	virtual float GetZoomedDamageBonus();
	virtual int AmmoClickPoint() { return 2; }

	virtual Class_T		Classify( void ) { return (Class_T) CLASS_ASW_SNIPER_RIFLE; }

	virtual const float GetAutoAimAmount() { return 0.26; }
	virtual bool ShouldFlareAutoaim() { return true; }

	virtual const Vector& GetBulletSpread( void )
	{
		static Vector cone;
		cone = vec3_origin;
		return cone;
	}

	#ifndef CLIENT_DLL
		DECLARE_DATADESC();

		int		CapabilitiesGet( void ) { return bits_CAP_WEAPON_RANGE_ATTACK1; }

		virtual bool IsRapidFire() { return false; }
		virtual float GetMadFiringBias() { return 0.2f; }	// scales the rate at which the mad firing counter goes up when we shoot aliens with this weapon
	#else
		virtual const char* GetTracerEffectName() { return "tracer_sniper_rifle"; }	// particle effect name
		virtual const char* GetMuzzleEffectName() { return "muzzle_sniper_rifle"; }	// particle effect name

		virtual void OnDataChanged( DataUpdateType_t updateType );
		virtual void ClientThink();
		//virtual void UpdateDynamicLight();
		virtual void OnMuzzleFlashed();
		virtual const char *GetPartialReloadSound( int iPart );

		//dlight_t* m_pSniperDynamicLight;
		float m_flEjectBrassTime;
		int m_nEjectBrassCount;
	#endif
	virtual const char *GetMagazineGibModelName() const override { return "models/weapons/empty_clips/sniperrifle_empty_clip.mdl"; }

	CNetworkVar( bool, m_bZoomed );
protected:
	CNetworkVar( float, m_fSlowTime );	// marine moves slow until this moment
};


#endif /* _DEFINED_ASW_WEAPON_SNIPER_RIFLE_H */
