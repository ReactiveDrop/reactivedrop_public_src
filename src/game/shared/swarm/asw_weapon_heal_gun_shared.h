#ifndef _INCLUDED_ASW_WEAPON_HEAL_GUN_SHARED_H
#define _INCLUDED_ASW_WEAPON_HEAL_GUN_SHARED_H
#ifdef _WIN32
#pragma once
#endif

// Mining Laser
//   "Weapon" that causes heat damage at short range.  Can be used to blast through certain kinds of rocks.

#ifdef CLIENT_DLL
#include "c_asw_weapon.h"
#define CASW_Weapon_Heal_Gun C_ASW_Weapon_Heal_Gun
#else
#include "asw_weapon.h"
#endif

enum ASW_Weapon_HealGunFireState_t { 
	ASW_HG_FIRE_OFF, 
	ASW_HG_FIRE_DISCHARGE, 
	ASW_HG_FIRE_ATTACHED,
	ASW_HG_FIRE_HEALSELF,
	ASW_HG_NUM_FIRE_STATES
};

abstract_class IASW_Medical_Weapon
{
public:
	// for now, every medical weapon has the same range
	static float GetWeaponRange(void) { return 240; }

	virtual bool HasMedicalAmmo() = 0;
	virtual bool HasHealAttachTarget() = 0;
	virtual void HealSelf() = 0;
	virtual bool HealAttach( CBaseEntity *pEntity ) = 0;
	virtual void HealDetach() = 0;
	virtual void HealAttack() = 0;
};

class CASW_Weapon_Heal_Gun : public CASW_Weapon, public IASW_Medical_Weapon
{
public:
	DECLARE_CLASS( CASW_Weapon_Heal_Gun, CASW_Weapon );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

    CASW_Weapon_Heal_Gun(void);

    virtual void    Precache( void );

	virtual void	PrimaryAttack( void );
	virtual void	SecondaryAttack();
	virtual void	HealAttack() { PrimaryAttack(); }
	virtual bool	HasMedicalAmmo() { return HasPrimaryAmmo(); }
	virtual bool	SecondaryAttackUsesPrimaryAmmo() { return true; }
	virtual bool	Reload( void );
	virtual bool	HasAmmo();
	virtual void	WeaponIdle( void );
	virtual void	ItemPostFrame();
	virtual void	ItemBusyFrame();
	virtual void	CheckEndFiringState();

	virtual void	UpdateOnRemove();

	virtual const float GetAutoAimAmount() { return AUTOAIM_2DEGREES; };
	virtual bool		Holster( CBaseCombatWeapon *pSwitchingTo = NULL );
	virtual void		Drop( const Vector &vecVelocity );
	virtual bool		ShouldMarineMoveSlow();
	virtual bool		IsOffensiveWeapon() { return false; }
	virtual Class_T		Classify( void ) { return (Class_T)CLASS_ASW_HEAL_GUN; }

	virtual bool	DestroyOnButtonRelease() { return true; }  // whether this item should destroy on release of the fire button or not

#ifdef CLIENT_DLL
	virtual void MouseOverEntity(C_BaseEntity *pEnt, Vector vecWorldCursor);
	virtual void ClientThink( void );
	virtual const char* GetPartialReloadSound( int iPart );
	virtual void UpdateEffects();	
	virtual bool ShouldShowLaserPointer();
	virtual float GetLaserPointerRange( void ) { return 240; }// Give a chance for non-local weapons to update their effects on the client
#else
	int		CapabilitiesGet( void ) { return bits_CAP_WEAPON_RANGE_ATTACK1; }
	virtual void GetButtons(bool& bAttack1, bool& bAttack2, bool& bReload, bool& bOldReload, bool& bOldAttack1 );
	float m_fLastForcedFireTime;
#endif

protected:
	void	Attack( void );
	void	EndAttack( void );
	void	Fire( const Vector &vecOrigSrc, const Vector &vecDir );

	virtual void	SetFiringState( ASW_Weapon_HealGunFireState_t state );
	
	virtual bool	TargetCanBeHealed( CBaseEntity* pTargetMarine );
	virtual void	HealSelf();
	virtual void	HealEntity();
	virtual int		GetHealAmount();
	virtual float	GetInfestationCureAmount();
#ifdef GAME_DLL
	//void	DoArcingHeal( float flBaseDamage, CBaseEntity *pLastHealed );
#endif

public:
	bool	HealAttach( CBaseEntity *pEntity );
	bool	HasHealAttachTarget( void )	{ return (m_hHealEntity.Get()) ? true : false; }
protected:
	void	HealDetach();

	void StartSearchSound();
	void StartHealSound();
	void StopHealSound();

	CSoundPatch *m_pSearchSound;
	CSoundPatch *m_pHealSound;

#ifdef CLIENT_DLL
	CUtlReference<CNewParticleEffect>	m_pDischargeEffect;
#endif

	CNetworkVar(unsigned char, m_FireState);	// one of the ASW_Weapon_HealGunFireState_t enums
	CNetworkVar(float, m_fSlowTime);			// marine moves slow until this moment
	CNetworkVar(EHANDLE, m_hHealEntity);		// entity we're attached too and healing
	CNetworkVar(Vector, m_vecHealPos);			// place we're zapping

	//float	m_flLastDischargeTime;
	trace_t m_AttackTrace;
	float	m_flLastHealTime;
	float	m_flNextHealMessageTime;
	float	m_flTotalAmountHealed;
};

#endif // _INCLUDED_ASW_WEAPON_HEAL_GUN_SHARED_H