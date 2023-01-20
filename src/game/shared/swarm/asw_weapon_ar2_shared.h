#ifndef _INCLUDED_ASW_WEAPON_AR2_H
#define _INCLUDED_ASW_WEAPON_AR2_H
#pragma once

#ifdef CLIENT_DLL
	#define CASW_Weapon_AR2 C_ASW_Weapon_AR2
	#define CASW_Weapon_Rifle C_ASW_Weapon_Rifle
	#include "c_asw_weapon_rifle.h"
#else
	#include "asw_weapon_rifle.h"
#endif

class CASW_Weapon_AR2 : public CASW_Weapon_Rifle
{
public:
	DECLARE_CLASS( CASW_Weapon_AR2, CASW_Weapon_Rifle );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CASW_Weapon_AR2();
	~CASW_Weapon_AR2();

	void Precache() override;
	float GetWeaponDamage() override;
	void ItemPostFrame() override;
	void SecondaryAttack() override;
	void DelayedAttack() override;
	bool CanHolster() override;

	#ifndef CLIENT_DLL
		DECLARE_DATADESC();
		DECLARE_ACTTABLE();

		void	FireNPCSecondaryAttack( CBaseCombatCharacter *pOperator, bool bUseWeaponAngles );
		void	Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );
	#else
		bool HasSecondaryExplosive( void ) const override { return false; }

		const char *GetTracerEffectName() override { return "tracer_ar2"; }	// particle effect name
		const char *GetMuzzleEffectName() override { return "muzzle_ar2"; }	// particle effect name
	#endif

	// Classification
	Class_T Classify( void ) override { return ( Class_T )CLASS_ASW_AR2; }
};

#endif /* _INCLUDED_ASW_WEAPON_AR2_H */
