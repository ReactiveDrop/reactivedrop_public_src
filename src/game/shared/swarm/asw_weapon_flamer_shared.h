#ifndef _INCLUDED_ASW_WEAPON_FLAMER_H
#define _INCLUDED_ASW_WEAPON_FLAMER_H
#pragma once

#ifdef CLIENT_DLL
#include "c_asw_weapon.h"
#define CASW_Weapon C_ASW_Weapon
#define CASW_Weapon_Flamer C_ASW_Weapon_Flamer
#define CASW_Marine C_ASW_Marine
#else
#include "asw_weapon.h"
#include "npc_combine.h"
#endif

#include "basegrenade_shared.h"
#include "asw_shareddefs.h"

class C_ASW_Emitter;

class CASW_Weapon_Flamer : public CASW_Weapon
{
public:
	DECLARE_CLASS( CASW_Weapon_Flamer, CASW_Weapon );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CASW_Weapon_Flamer();
	virtual ~CASW_Weapon_Flamer();
	void Precache();

	virtual void Spawn();

	virtual bool SupportsBayonet();
	virtual float GetWeaponBaseDamageOverride();
	virtual int GetWeaponSkillId();
	virtual int GetWeaponSubSkillId();
	
	Activity	GetPrimaryAttackActivity( void );
	virtual void	SecondaryAttack();
	virtual void	PrimaryAttack();
	virtual void	ClearIsFiring();
	virtual void	ItemPostFrame();
	virtual bool SecondaryAttackUsesPrimaryAmmo() { return true; }
	virtual bool ShouldFireFromCameraInFirstPerson() const { return false; }

	virtual const Vector& GetBulletSpread( void )
	{
		static const Vector cone = VECTOR_CONE_PRECALCULATED;

		return cone;
	}

	#ifndef CLIENT_DLL
		DECLARE_DATADESC();

		int		CapabilitiesGet( void ) { return bits_CAP_WEAPON_RANGE_ATTACK1; }
	#else
		virtual bool ShouldMarineFlame(); // if true, the marine emits flames from his flame emitter
		virtual bool ShouldMarineFireExtinguish();
		virtual const char* GetPartialReloadSound(int iPart);
		virtual bool Simulate();

		virtual float GetLaserPointerRange( void ) { return 280; }

		virtual void OnDataChanged( DataUpdateType_t updateType );
		virtual void UpdateOnRemove();
		virtual void ClientThink();
	#endif
	virtual const char *GetMagazineGibModelName() const override { return "models/weapons/empty_clips/flamethrower_empty_clip.mdl"; }

	float m_flLastFireTime;
	CNetworkVar(bool, m_bIsSecondaryFiring);

	enum 
	{	// namespaced immediate constant:
		FLAMER_PROJECTILE_AIR_VELOCITY = 600,
		EXTINGUISHER_PROJECTILE_AIR_VELOCITY = 400,
	};

#ifdef CLIENT_DLL
	CUtlReference<CNewParticleEffect> pEffect;	
	CUtlReference<CNewParticleEffect> pExtinguishEffect;	
	CUtlReference<CNewParticleEffect> m_hPilotLight;
#endif

	// Classification
	virtual Class_T		Classify( void ) { return (Class_T) CLASS_ASW_FLAMER; }
};


#endif /* _INCLUDED_ASW_WEAPON_FLAMER_H */
