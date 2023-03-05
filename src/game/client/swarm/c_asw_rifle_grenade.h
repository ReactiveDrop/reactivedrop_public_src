#pragma once

#include "c_basecombatcharacter.h"
#include "rd_inventory_shared.h"

class C_ASW_Rifle_Grenade : public C_BaseCombatCharacter, public IRD_Has_Projectile_Data
{
	DECLARE_CLASS( C_ASW_Rifle_Grenade, C_BaseCombatCharacter );
public:
	DECLARE_CLIENTCLASS();

	CNetworkVarEmbedded( CRD_ProjectileData, m_ProjectileData );
	const CRD_ProjectileData *GetProjectileData() const override
	{
		return &m_ProjectileData;
	}
};
