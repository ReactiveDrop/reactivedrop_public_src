#ifndef _DEFINED_ASW_EXTINGUISHER_PROJECTILE_H
#define _DEFINED_ASW_EXTINGUISHER_PROJECTILE_H
#pragma once

#include "rd_inventory_shared.h"

class CASW_Extinguisher_Projectile : public CBaseCombatCharacter
{
	DECLARE_CLASS( CASW_Extinguisher_Projectile, CBaseCombatCharacter );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	virtual ~CASW_Extinguisher_Projectile( void );

public:
	void	Spawn( void );
	bool	CreateVPhysics( void );
	unsigned int PhysicsSolidMaskForEntity() const;
	void	ProjectileTouch( CBaseEntity *pOther );
	virtual void PhysicsSimulate( void );
	virtual int ShouldTransmit( const CCheckTransmitInfo *pInfo );
	void TouchedEnvFire();
	void SetFreezeAmount( float flFreeze ) { m_flFreezeAmount = flFreeze; }

	static CASW_Extinguisher_Projectile *Extinguisher_Projectile_Create( const Vector &position, const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, CBaseEntity *pOwner, CBaseEntity *pWeapon );

	CBaseEntity* GetFirer();
	EHANDLE m_hFirer;
	EHANDLE m_hFirerWeapon;

	CNetworkVarEmbedded( CRD_ProjectileData, m_ProjectileData );

protected:

	float		m_flDamage;
	bool	m_inSolid;	
	float m_flFreezeAmount;
};


#endif // _DEFINED_ASW_EXTINGUISHER_PROJECTILE_H
