#pragma once

#include "c_asw_aoegrenade_projectile.h"

class C_ASW_ShieldGrenade_Projectile : public C_ASW_AOEGrenade_Projectile
{
public:
	DECLARE_CLASS( C_ASW_ShieldGrenade_Projectile, C_ASW_AOEGrenade_Projectile );
	DECLARE_CLIENTCLASS();

	void Precache() override;

	void UpdatePingEffects() override;
	bool ShouldSpawnSphere() override { return false; } // we spawn our own
};
