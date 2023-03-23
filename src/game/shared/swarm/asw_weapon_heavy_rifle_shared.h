#ifndef _INCLUDED_ASW_WEAPON_HEAVY_RIFLE_H
#define _INCLUDED_ASW_WEAPON_HEAVY_RIFLE_H
#pragma once

#ifdef CLIENT_DLL
	#define CASW_Weapon_Heavy_Rifle C_ASW_Weapon_Heavy_Rifle
	#define CASW_Weapon_Rifle C_ASW_Weapon_Rifle
	#include "c_asw_weapon_rifle.h"
#else
	#include "asw_weapon_rifle.h"
#endif

class CASW_Weapon_Heavy_Rifle : public CASW_Weapon_Rifle
{
public:
	DECLARE_CLASS( CASW_Weapon_Heavy_Rifle, CASW_Weapon_Rifle );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CASW_Weapon_Heavy_Rifle();
	virtual ~CASW_Weapon_Heavy_Rifle();
	void Precache();

	virtual float GetWeaponDamage();
	virtual void SecondaryAttack();
	virtual const Vector &GetBulletSpread( void );
	float GetFireRate( void );
	void StopFastFire();

#ifdef CLIENT_DLL
	void ClientThink() override;
#else
	DECLARE_DATADESC();
#endif
	virtual const char *GetMagazineGibModelName() const override { return "models/weapons/empty_clips/heavyrifle_empty_clip.mdl"; }

	// Classification
	virtual Class_T		Classify( void ) { return ( Class_T )CLASS_ASW_HEAVY_RIFLE; }

protected:
	CNetworkVar( bool, m_bFastFire );
#ifdef CLIENT_DLL
	HPARTICLEFFECT m_hFastFireParticle;
#endif
};


#endif /* _INCLUDED_ASW_WEAPON_HEAVY_RIFLE_H */
