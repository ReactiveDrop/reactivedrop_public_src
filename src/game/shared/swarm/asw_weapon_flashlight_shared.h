#ifndef _INCLUDED_ASW_WEAPON_FLASHLIGHT_H
#define _INCLUDED_ASW_WEAPON_FLASHLIGHT_H
#pragma once

#ifdef CLIENT_DLL
#include "c_asw_weapon.h"
#define CASW_Weapon C_ASW_Weapon
#define CASW_Weapon_Flashlight C_ASW_Weapon_Flashlight
#define CASW_Marine C_ASW_Marine
#else
#include "asw_weapon.h"
#include "npc_combine.h"
#endif

#include "basegrenade_shared.h"
#include "asw_shareddefs.h"

class CASW_Weapon_Flashlight : public CASW_Weapon
{
public:
	DECLARE_CLASS( CASW_Weapon_Flashlight, CASW_Weapon );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CASW_Weapon_Flashlight();
	virtual ~CASW_Weapon_Flashlight();
	void Precache();

	virtual float	GetFireRate(void) { return 1.4f; }
	virtual bool	Reload() { return false;  }
	virtual bool	ShouldMarineMoveSlow() { return false; }	// firing doesn't slow the marine down
	
	virtual bool	OffhandActivate();
	virtual bool	IsOffensiveWeapon() { return false; }

//	virtual bool	WantsOffhandPostFrame() { return true; }
	// Classification
	virtual Class_T		Classify( void ) { return (Class_T) CLASS_ASW_FLASHLIGHT; }
	virtual void	HandleFireOnEmpty() { return PrimaryAttack();  }

	virtual bool ViewModelIsMarineAttachment() const { return true; }
	Activity		GetPrimaryAttackActivity( void ) { return ACT_VM_PRIMARYATTACK; }
//	virtual int		ASW_SelectWeaponActivity(int idealActivity) { return idealActivity; }
	virtual int		AmmoClickPoint() { return 0; }
	virtual void	PrimaryAttack();

	virtual const Vector& GetBulletSpread( void )
	{
		static Vector cone;
		cone = Vector(0,0,0);
		return cone;
	}

	#ifndef CLIENT_DLL
		DECLARE_DATADESC();

		// todo: stop AI attacking with this?
		int		CapabilitiesGet( void ) { return bits_CAP_WEAPON_RANGE_ATTACK1; }

		// for toggling the flashlight effect when we take/drop this weapon
		const char *GetEquipSound() override { return "ASW_Weapon.AttachmentEquipSmall"; }
		virtual void MarineDropped(CASW_Marine* pMarine);
		virtual void Equip( CBaseCombatCharacter *pOwner );
	#else
		virtual void ClientThink() override;
	#endif
};


#endif /* _INCLUDED_ASW_WEAPON_FLASHLIGHT_H */
