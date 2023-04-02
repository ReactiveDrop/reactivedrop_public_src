#ifndef _INCLUDED_ASW_WEAPON_MEDRIFLE_H
#define _INCLUDED_ASW_WEAPON_MEDRIFLE_H
#pragma once

#ifdef CLIENT_DLL
	#define CASW_Weapon_MedRifle C_ASW_Weapon_MedRifle
	#define CASW_Weapon_Rifle C_ASW_Weapon_Rifle
	#include "c_asw_weapon_rifle.h"
#else
	#include "npc_combine.h"
	#include "asw_weapon_rifle.h"
#endif

#include "asw_weapon_heal_gun_shared.h"

/*enum ASW_Weapon_HealGunFireState_t { 
	ASW_HG_FIRE_OFF,
	ASW_HG_FIRE_DISCHARGE,
	ASW_HG_FIRE_ATTACHED,
	ASW_HG_FIRE_HEALSELF,
	ASW_HG_NUM_FIRE_STATES
};*/

class CASW_Weapon_MedRifle : public CASW_Weapon_Rifle, public IASW_Medical_Weapon
{
public:
	DECLARE_CLASS( CASW_Weapon_MedRifle, CASW_Weapon_Rifle );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CASW_Weapon_MedRifle();
	virtual ~CASW_Weapon_MedRifle();
	void Precache();

	virtual bool Reload( void );
	virtual float GetWeaponDamage();
	virtual void SecondaryAttack();
	virtual void HealAttack() { SecondaryAttack(); }
	virtual bool HasMedicalAmmo() { return HasSecondaryAmmo(); }
	virtual void WeaponIdle( void );
	virtual const Vector& GetBulletSpread( void );
	virtual void ItemPostFrame();
	virtual void ItemBusyFrame();
	virtual void CheckEndFiringState();
	virtual void UpdateOnRemove();
	virtual bool Holster( CBaseCombatWeapon *pSwitchingTo = NULL );
	virtual void Drop( const Vector &vecVelocity );
	virtual bool ShouldMarineMoveSlow();
	float GetSecondaryFireRate( void );

	#ifndef CLIENT_DLL
		DECLARE_DATADESC();
		virtual void GetButtons(bool& bAttack1, bool& bAttack2, bool& bReload, bool& bOldReload, bool& bOldAttack1 );
	#else
		virtual void MouseOverEntity(C_BaseEntity *pEnt, Vector vecWorldCursor);
		virtual void ClientThink( void );
		//virtual const char* GetPartialReloadSound( int iPart );
		virtual void UpdateEffects();
		virtual bool ShouldShowLaserPointer();
		virtual float GetLaserPointerRange( void ) { return 240; }// Give a chance for non-local weapons to update their effects on the client
	#endif
	virtual bool ShouldHealSelfOnInvalidTarget( CBaseEntity *pTarget );
	virtual const char *GetMagazineGibModelName() const override { return "models/weapons/empty_clips/medrifle_empty_clip.mdl"; }

	// Classification
	virtual Class_T		Classify( void ) { return (Class_T) CLASS_ASW_MEDRIFLE; }

	bool	HealAttach( CBaseEntity *pEntity );
	bool	HasHealAttachTarget( void )	{ return (m_hHealEntity.Get()) ? true : false; }

protected:
	void	HealDetach();
	bool	TargetCanBeHealed( CBaseEntity* pTargetMarine );
	void	HealSelf();
	void	HealEntity();
	virtual int		GetHealAmount();
	virtual float	GetInfestationCureAmount();

	void	StartHealSound();
	void	StartSearchSound();
	void	StopHealSound();

	CSoundPatch *m_pSearchSound;
	CSoundPatch *m_pHealSound;

	void	EndAttack( void );
	void	Fire( const Vector &vecOrigSrc, const Vector &vecDir );
	virtual void	SetFiringState( ASW_Weapon_HealGunFireState_t state );

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


#endif /* _INCLUDED_ASW_WEAPON_MEDRIFLE_H */
