#ifndef _INCLUDED_ASW_WEAPON_GRENADES_H
#define _INCLUDED_ASW_WEAPON_GRENADES_H
#pragma once

#include "asw_weapon_grenade_box_shared.h"
#ifdef CLIENT_DLL
#define CASW_Weapon_Grenades C_ASW_Weapon_Grenades
#endif

#include "basegrenade_shared.h"
#include "asw_shareddefs.h"

class CASW_Weapon_Grenades : public CASW_Weapon_Grenade_Box
{
public:
	DECLARE_CLASS( CASW_Weapon_Grenades, CASW_Weapon_Grenade_Box );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	void Precache();

	float	GetFireRate( void ) { return 1.4f; }
	
	virtual float GetThrowGravity() { return 0.8f; }

	virtual float GetWeaponBaseDamageOverride();
	virtual int GetWeaponSkillId();
	virtual int GetWeaponSubSkillId();

	virtual void DelayedAttack();

#ifndef CLIENT_DLL
	DECLARE_DATADESC();

	// Get the damage and radius that a grenade thrown by a given marine should 
	// explode with:
	static float GetBoomDamage( CASW_Marine *pMarine );
	static float GetBoomRadius( CASW_Marine *pMarine );
#endif

	// Classification
	virtual Class_T		Classify( void ) { return (Class_T) CLASS_ASW_GRENADES; }
};

#endif /* _INCLUDED_ASW_WEAPON_GRENADES_H */
