#ifndef ASW_WEAPON_HEALAMP_GUN_SHARED_HEADER_INCLUDED__
#define ASW_WEAPON_HEALAMP_GUN_SHARED_HEADER_INCLUDED__

#include "asw_shareddefs.h"	// for CLASS_ASW_HEALAMP_GUN
#include "asw_weapon_heal_gun_shared.h"

#ifdef CLIENT_DLL
#define CASW_Weapon_HealAmp_Gun C_ASW_Weapon_HealAmp_Gun
#endif

class CASW_Weapon_HealAmp_Gun : public CASW_Weapon_Heal_Gun
{
public:
	DECLARE_CLASS( CASW_Weapon_HealAmp_Gun, CASW_Weapon_Heal_Gun );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CASW_Weapon_HealAmp_Gun(void);

	virtual void    Precache(void);

	virtual void	PrimaryAttack();
	virtual void	SecondaryAttack();

	virtual Class_T		Classify( void ) { return (Class_T)CLASS_ASW_HEALAMP_GUN; }
	virtual bool		SecondaryAttackUsesPrimaryAmmo() { return false; }
	virtual bool		UsesClipsForAmmo2() { return false; }

#ifdef CLIENT_DLL
	virtual void MouseOverEntity(C_BaseEntity *pEnt, Vector vecWorldCursor);
	virtual void UpdateEffects();
#else
#endif
	virtual bool ShouldHealSelfOnInvalidTarget( CBaseEntity *pTarget );
	virtual int GetMagazineGibModelSkin() const override { return 1; }

protected:
	void	SetFiringState( ASW_Weapon_HealGunFireState_t state );
	bool	TargetCanBeHealed( CBaseEntity* pTargetMarine );
	void	HealSelf();
	void	HealEntity();

	void	StartHealSound();
	void	StartSearchSound();

	CNetworkVar( bool, m_bIsBuffing );			// true if we are not healing but buffing (like damage amplifier)
};

#endif // !ASW_WEAPON_HEALAMP_GUN_SHARED_HEADER_INCLUDED__
