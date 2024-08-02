#ifndef _DEFINED_ASW_EXTINGUISHER_PROJECTILE_H
#define _DEFINED_ASW_EXTINGUISHER_PROJECTILE_H
#pragma once

#include "rd_inventory_shared.h"

class CASW_Extinguisher_Projectile : public CBaseCombatCharacter, public IRD_Has_Projectile_Data
{
	DECLARE_CLASS( CASW_Extinguisher_Projectile, CBaseCombatCharacter );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	CASW_Extinguisher_Projectile();
	virtual ~CASW_Extinguisher_Projectile();

public:
	void Spawn( void ) override;
	bool CreateVPhysics( void ) override;
	unsigned int PhysicsSolidMaskForEntity() const override;
	void ProjectileTouch( CBaseEntity *pOther );
	void Explode( CBaseEntity *pExcept );
	void OnProjectileTouch( CBaseEntity *pOther );
	void PhysicsSimulate( void ) override;
	int ShouldTransmit( const CCheckTransmitInfo *pInfo ) override;
	void TouchedEnvFire();
	void SetFreezeAmount( float flFreeze ) { m_flFreezeAmount = flFreeze; }

	static CASW_Extinguisher_Projectile *Extinguisher_Projectile_Create( const Vector &position, const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, CBaseCombatCharacter *pOwner, CBaseEntity *pWeapon );

	CBaseEntity *GetFirer();
	CHandle<CBaseCombatCharacter> m_hFirer;
	EHANDLE m_hFirerWeapon;

	CNetworkVarEmbedded( CRD_ProjectileData, m_ProjectileData );
	const CRD_ProjectileData *GetProjectileData() const override
	{
		return &m_ProjectileData;
	}

	float m_flDamage;
	float m_flFreezeAmount;
	float m_flExplosionRadius;
	bool m_bAllowFriendlyFire;
	bool m_bUseHullFreezeScale;
	Vector m_vecSpawnOrigin;

	Class_T Classify() override { return (Class_T)CLASS_ASW_EXTINGUISHER_PROJECTILE; }
};


#endif // _DEFINED_ASW_EXTINGUISHER_PROJECTILE_H
