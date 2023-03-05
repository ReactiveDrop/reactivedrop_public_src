#include "cbase.h"
#include "c_asw_rifle_grenade.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_CLIENTCLASS_DT( C_ASW_Rifle_Grenade, DT_ASW_Rifle_Grenade, CASW_Rifle_Grenade )
	RecvPropDataTable( RECVINFO_DT( m_ProjectileData ), 0, &REFERENCE_RECV_TABLE( DT_RD_ProjectileData ) ),
END_RECV_TABLE();
