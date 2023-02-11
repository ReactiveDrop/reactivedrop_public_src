#ifndef _INCLUDED_ASW_WEAPON_NORMAL_ARMOR_H
#define _INCLUDED_ASW_WEAPON_NORMAL_ARMOR_H
#pragma once

#ifdef CLIENT_DLL
#include "c_asw_weapon.h"
#define CASW_Weapon C_ASW_Weapon
#define CASW_Weapon_Normal_Armor C_ASW_Weapon_Normal_Armor
#define CASW_Marine C_ASW_Marine
#else
#include "asw_weapon.h"
#endif

#include "asw_shareddefs.h"
#include "basegrenade_shared.h"

class CASW_Weapon_Normal_Armor : public CASW_Weapon
{
public:
	DECLARE_CLASS( CASW_Weapon_Normal_Armor, CASW_Weapon );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CASW_Weapon_Normal_Armor();
	virtual ~CASW_Weapon_Normal_Armor();
	// void Precache();
	
	void	PrimaryAttack();
	
	// bool	OffhandActivate();
	virtual bool ShouldMarineMoveSlow() { return false; }	// firing doesn't slow the marine down

	#ifndef CLIENT_DLL
		DECLARE_DATADESC();

		int		CapabilitiesGet( void ) { return bits_CAP_WEAPON_RANGE_ATTACK1; }
	#endif

	virtual bool IsOffensiveWeapon() { return false; }
	virtual bool ViewModelIsMarineAttachment() const { return true; }

	// Classification
	virtual Class_T		Classify( void ) { return (Class_T) CLASS_ASW_NORMAL_ARMOR; }

#ifndef CLIENT_DLL
	virtual float GetDamageScaleFactor();
	virtual void LayerRemoveOnDamage();
	virtual void LayerRestoreThink();
#else
	virtual float GetBatteryCharge();
#endif
	CNetworkVar( int, m_iLayersMissing );
};


#endif /* _INCLUDED_ASW_WEAPON_NORMAL_ARMOR_H */
