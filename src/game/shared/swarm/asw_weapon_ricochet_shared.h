#ifndef _INCLUDED_ASW_WEAPON_RICOCHET_H
#define _INCLUDED_ASW_WEAPON_RICOCHET_H
#pragma once

#ifdef CLIENT_DLL
	#define CASW_Weapon_Ricochet C_ASW_Weapon_Ricochet
	#define CASW_Weapon_Rifle C_ASW_Weapon_Rifle
	#define CASW_Marine C_ASW_Marine
	#include "c_asw_weapon_rifle.h"
#else
	#include "asw_weapon_rifle.h"
#endif

class CASW_Marine;

class CASW_Weapon_Ricochet : public CASW_Weapon_Rifle
{
public:
	DECLARE_CLASS( CASW_Weapon_Ricochet, CASW_Weapon_Rifle );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CASW_Weapon_Ricochet();
	virtual ~CASW_Weapon_Ricochet();
	void Precache();

	virtual void PrimaryAttack();
	virtual void SecondaryAttack();

	virtual const float GetAutoAimAmount() { return 0.26; }
	virtual bool ShouldFlareAutoaim() { return true; }

	#ifndef CLIENT_DLL
	#else
		virtual bool HasSecondaryExplosive( void ) const { return false; }
		virtual const char *GetLaserPointerEffectName() const { return "weapon_laser_sight_ricochet"; }

		void UpdateBounceLaser();
		virtual bool Simulate();
	#endif
	virtual const char *GetMagazineGibModelName() const override { return "models/weapons/empty_clips/autogun_empty_clip.mdl"; }

	// aiming grenades at the ground
	virtual bool SupportsGroundShooting() { return false; }

	// Classification
	virtual Class_T		Classify( void ) { return (Class_T) CLASS_ASW_RICOCHET; }
};

#endif /* _INCLUDED_ASW_WEAPON_RICOCHET_H */
