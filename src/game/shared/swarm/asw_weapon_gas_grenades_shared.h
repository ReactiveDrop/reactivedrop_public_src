#ifndef _INCLUDED_ASW_WEAPON_GAS_GRENADES_H
#define _INCLUDED_ASW_WEAPON_GAS_GRENADES_H
#pragma once
#include "asw_weapon_grenade_box_shared.h"
#ifdef CLIENT_DLL
#define CASW_Weapon_Gas_Grenades C_ASW_Weapon_Gas_Grenades
#endif

#include "basegrenade_shared.h"
#include "asw_shareddefs.h"

class CASW_Weapon_Gas_Grenades : public CASW_Weapon_Grenade_Box
{
public:
	DECLARE_CLASS( CASW_Weapon_Gas_Grenades, CASW_Weapon_Grenade_Box );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CASW_Weapon_Gas_Grenades();
	virtual ~CASW_Weapon_Gas_Grenades();
	void Precache();

	float	GetFireRate( void ) { return 1.4f; }

	virtual float GetThrowGravity() { return 1.15f; }

	void	PrimaryAttack();
	void	DelayedAttack();

	#ifndef CLIENT_DLL
		DECLARE_DATADESC();
	#endif

	// Classification
	virtual Class_T	Classify( void ) { return (Class_T) CLASS_ASW_GAS_GRENADES; }
};


#endif /* _INCLUDED_ASW_WEAPON_GAS_GRENADES_H */

