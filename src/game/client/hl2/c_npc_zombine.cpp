#include "cbase.h"
#include "c_asw_alien.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class C_NPC_Zombine : public C_ASW_Alien
{
	DECLARE_CLASS( C_NPC_Zombine, C_ASW_Alien );
public:
	DECLARE_CLIENTCLASS();

	const Vector &GetAimTargetPos( const Vector &vecFiringSrc, bool bWeaponPrefersFlatAiming ) override
	{
		// marines aim at the grenade so they can knock it out of the zombine's hand more often
		if ( !m_hGrenade || bWeaponPrefersFlatAiming )
			return BaseClass::GetAimTargetPos( vecFiringSrc, bWeaponPrefersFlatAiming );

		return m_hGrenade->WorldSpaceCenter();
	}

	CNetworkHandle( C_BaseEntity, m_hGrenade );
};

IMPLEMENT_CLIENTCLASS_DT( C_NPC_Zombine, DT_NPC_Zombine, CNPC_Zombine )
	RecvPropEHandle( RECVINFO( m_hGrenade ) ),
END_RECV_TABLE()
