#include "cbase.h"
#include "c_asw_colonist.h"
#include "c_asw_marine.h"
#include "asw_util_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_CLIENTCLASS_DT( C_ASW_Colonist, DT_ASW_Colonist, CASW_Colonist )
	RecvPropBool( RECVINFO( m_bAimTarget ) ),
	RecvPropBool( RECVINFO( m_bFollowOnUse ) ),
	RecvPropEHandle( RECVINFO( m_hFollowingMarine ) ),
END_RECV_TABLE()

C_ASW_Colonist::C_ASW_Colonist()
{
}

bool C_ASW_Colonist::IsUsable( C_BaseEntity *pUser )
{
	C_ASW_Marine *pMarine = C_ASW_Marine::AsMarine( pUser );

	return m_bFollowOnUse && pMarine && pMarine->GetAbsOrigin().DistToSqr( GetAbsOrigin() ) < Square( ASW_MARINE_USE_RADIUS );
}

bool C_ASW_Colonist::GetUseAction( ASWUseAction &action, C_ASW_Inhabitable_NPC *pUser )
{
	if ( !m_bFollowOnUse )
	{
		return false;
	}

	action.UseTarget = this;

	// reusing these strings because next to no maps use colonists anyway.
	if ( m_hFollowingMarine.Get() == pUser )
		TryLocalize( "#asw_order_marines_hold", action.wszText, sizeof( action.wszText ) );
	else
		TryLocalize( "#asw_order_marines_follow", action.wszText, sizeof( action.wszText ) );

	return true;
}
