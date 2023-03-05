#ifndef _INCLUDED_C_ASW_GAS_GRENADE_PROJECTILE_H
#define _INCLUDED_C_ASW_GAS_GRENADE_PROJECTILE_H
#pragma once

#include "c_pixel_visibility.h"
#include "rd_inventory_shared.h"

class C_ASW_Gas_Grenade_Projectile : public C_BaseCombatCharacter, public IRD_Has_Projectile_Data
{
public:
	DECLARE_CLASS( C_ASW_Gas_Grenade_Projectile, C_BaseCombatCharacter );
	DECLARE_CLIENTCLASS();

	C_ASW_Gas_Grenade_Projectile();
	virtual ~C_ASW_Gas_Grenade_Projectile();

	virtual Class_T	Classify() { return Class_T(CLASS_ASW_GAS_GRENADE); }

	void	OnDataChanged( DataUpdateType_t updateType );
	//void	Update( float timeDelta );
	virtual void	ClientThink( void );
	void	NotifyShouldTransmit( ShouldTransmitState_t state );
	const Vector& GetEffectOrigin();

	float	m_flTimeBurnOut;
	float	m_flScale;
	float	m_flTimeCreated;
	bool	m_bSmoke;
	bool	m_bStopped;
	pixelvis_handle_t m_queryHandle;

	CNetworkVarEmbedded( CRD_ProjectileData, m_ProjectileData );
	const CRD_ProjectileData *GetProjectileData() const override
	{
		return &m_ProjectileData;
	}

private:
	C_ASW_Gas_Grenade_Projectile( const C_ASW_Gas_Grenade_Projectile & );

	CUtlReference<CNewParticleEffect> m_pTrailEffect;
};

#endif // _INCLUDED_C_ASW_GAS_GRENADE_PROJECTILE_H