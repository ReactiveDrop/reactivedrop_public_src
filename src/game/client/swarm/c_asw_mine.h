#ifndef _INCLUDED_C_ASW_MINE_H
#define _INCLUDED_C_ASW_MINE_H

#include "c_basecombatcharacter.h"
#include "rd_inventory_shared.h"

struct dlight_t;

class C_ASW_Mine : public C_BaseCombatCharacter, public IRD_Has_Projectile_Data
{
public:
	DECLARE_CLASS( C_ASW_Mine, C_BaseCombatCharacter );
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();

					C_ASW_Mine();
	virtual			~C_ASW_Mine();
	void ClientThink(void);
	void OnDataChanged(DataUpdateType_t updateType);

	CNetworkVar( bool, m_bMineTriggered );

	CNetworkVarEmbedded( CRD_ProjectileData, m_ProjectileData );
	const CRD_ProjectileData *GetProjectileData() const override
	{
		return &m_ProjectileData;
	}

private:
	C_ASW_Mine( const C_ASW_Mine & ); // not defined, not accessible
};

#endif // _INCLUDED_C_ASW_MINE_H