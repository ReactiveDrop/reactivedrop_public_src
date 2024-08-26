#pragma once

#include "asw_buffgrenade_projectile.h"

class CASW_ShieldGrenade_Projectile : public CASW_AOEGrenade_Projectile
{
public:
	DECLARE_CLASS( CASW_ShieldGrenade_Projectile, CASW_AOEGrenade_Projectile );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	CASW_ShieldGrenade_Projectile();

	void Precache() override;
};
