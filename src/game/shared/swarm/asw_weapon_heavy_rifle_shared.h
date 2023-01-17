#ifndef _INCLUDED_ASW_WEAPON_HEAVY_RIFLE_H
#define _INCLUDED_ASW_WEAPON_HEAVY_RIFLE_H
#pragma once

#ifdef CLIENT_DLL
	#define CASW_Weapon_Heavy_Rifle C_ASW_Weapon_Heavy_Rifle
	#define CASW_Weapon_Rifle C_ASW_Weapon_Rifle
	#include "c_asw_weapon_rifle.h"
#else
	#include "npc_combine.h"
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
	virtual const Vector& GetBulletSpread( void );
	float GetFireRate( void );
	void StopFastFire();

	#ifndef CLIENT_DLL
		DECLARE_DATADESC();
	#endif

	// Classification
	virtual Class_T		Classify( void ) { return (Class_T) CLASS_ASW_HEAVY_RIFLE; }

protected:
	CNetworkVar( bool, m_bFastFire );
};


#endif /* _INCLUDED_ASW_WEAPON_HEAVY_RIFLE_H */
