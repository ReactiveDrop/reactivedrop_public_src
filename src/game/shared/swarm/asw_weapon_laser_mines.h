#ifndef _INCLUDED_ASW_WEAPON_LASER_MINES_H
#define _INCLUDED_ASW_WEAPON_LASER_MINES_H
#pragma once

#ifdef CLIENT_DLL
#include "c_asw_weapon.h"
#define CASW_Weapon C_ASW_Weapon
#define CASW_Weapon_Laser_Mines C_ASW_Weapon_Laser_Mines
#define CASW_Marine C_ASW_Marine
#else
#include "asw_weapon.h"
#include "npc_combine.h"
#endif

#include "basegrenade_shared.h"
#include "asw_shareddefs.h"

class CASW_Weapon_Laser_Mines : public CASW_Weapon
{
public:
	DECLARE_CLASS( CASW_Weapon_Laser_Mines, CASW_Weapon );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CASW_Weapon_Laser_Mines();
	virtual ~CASW_Weapon_Laser_Mines();
	void Precache();

	float	GetFireRate( void ) { return 1.4f; }
	bool	Reload();
	void	ItemPostFrame();
	virtual bool ShouldMarineMoveSlow() { return false; }	// firing mines doesn't slow the marine down
	bool	ValidateThrow( int nMineNumber, int nMinesPerShot, Vector vecSrc, Vector vecAiming, float flSpread, bool& bOnGround, CBaseEntity* pParent, trace_t& tr, Vector& vecMineAiming );
	int		GetThrownMineCount( int nMinesPerShot, Vector vecSrc, Vector vecAiming, float flSpread );
	
	Activity	GetPrimaryAttackActivity( void );

	void	PrimaryAttack();
	bool	OffhandActivate();
	virtual void DelayedAttack();

	virtual int				GetMaxClip1(void) const;
	virtual int				GetDefaultClip1(void) const;

	virtual const Vector& GetBulletSpread( void )
	{
		static Vector cone;
		cone = Vector(0,0,0);
		return cone;
	}

	#ifndef CLIENT_DLL
		DECLARE_DATADESC();

		int		CapabilitiesGet( void ) { return bits_CAP_WEAPON_RANGE_ATTACK1; }
	#else
	
	#endif

	virtual bool IsOffensiveWeapon() { return false; }

	float	m_flSoonestPrimaryAttack;

	// Classification
	virtual Class_T		Classify( void ) { return (Class_T) CLASS_ASW_LASER_MINES; }
};


#endif /* _INCLUDED_ASW_WEAPON_LASER_MINES_H */
