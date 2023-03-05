#ifndef _DEFINED_ASW_RIFLE_GRENADE_H
#define _DEFINED_ASW_RIFLE_GRENADE_H
#pragma once

#include "asw_shareddefs.h"
#include "rd_inventory_shared.h"

#ifdef CLIENT_DLL
#define CBaseEntity C_BaseEntity
#endif

class CSprite;
class CSpriteTrail;

class CASW_Rifle_Grenade : public CBaseCombatCharacter, IRD_Has_Projectile_Data
{
public:
	DECLARE_CLASS( CASW_Rifle_Grenade, CBaseCombatCharacter );
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

	CASW_Rifle_Grenade();
	virtual ~CASW_Rifle_Grenade( void );

	void	Spawn( void );
	void	OnRestore( void );
	void	Precache( void );
	bool	CreateVPhysics( void );
	virtual void	CreateEffects( void );
	virtual void	KillEffects();

	virtual void Detonate();
	
	unsigned int PhysicsSolidMaskForEntity() const;
	void	GrenadeTouch( CBaseEntity *pOther );

	static CASW_Rifle_Grenade *Rifle_Grenade_Create( float flDamage, float fRadius, const Vector &position, const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, CBaseEntity *pOwner , CBaseEntity *pCreator);	

	float	m_flDamage;
	float	m_DmgRadius;
	bool	m_bSilent;
	bool	m_inSolid;
	CHandle<CSprite>		m_pMainGlow;
	CHandle<CSpriteTrail>	m_pGlowTrail;

	EHANDLE m_hCreatorWeapon;

	CNetworkVarEmbedded( CRD_ProjectileData, m_ProjectileData );
	const CRD_ProjectileData *GetProjectileData() const override
	{
		return &m_ProjectileData;
	}

	// Classification
	virtual Class_T Classify( void ) { return (Class_T)CLASS_ASW_RIFLE_GRENADE; }
};


#endif // _DEFINED_ASW_RIFLE_GRENADE_H
