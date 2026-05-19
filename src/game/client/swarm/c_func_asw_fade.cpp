#include "cbase.h"
#include "c_func_asw_fade.h"
#include "c_asw_marine.h"
#include "c_asw_marine_resource.h"
#include "c_asw_game_resource.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_CLIENTCLASS_DT( C_Func_ASW_Fade, DT_Func_ASW_Fade, CFunc_ASW_Fade )
	RecvPropInt( RECVINFO( m_nFadeOpacity ) ),
	RecvPropBool( RECVINFO( m_bAllowFade ) ),
	RecvPropBool( RECVINFO( m_bHasProxies ) ),
	RecvPropBool( RECVINFO( m_bCollideWithMarines ) ),
	RecvPropInt( RECVINFO( m_fVisibleToMarine ), SPROP_UNSIGNED ),
	RecvPropBool( RECVINFO( m_bSolidWhenInvisible ) ),
END_RECV_TABLE()

C_Func_ASW_Fade::C_Func_ASW_Fade()
{
}

int C_Func_ASW_Fade::GetMarineIndex( C_BaseEntity *pEnt, const char *szContext )
{
	Assert( !szContext ); // we shouldn't be called from a script or entity input on the client

	C_ASW_Marine *pMarine = C_ASW_Marine::AsMarine( pEnt );
	if ( !pMarine )
	{
		return -1;
	}

	C_ASW_Marine_Resource *pMR = pMarine->GetMarineResource();
	if ( !pMR )
	{
		return -1;
	}

	C_ASW_Game_Resource *pGameResource = ASWGameResource();
	if ( !pGameResource )
	{
		return -1;
	}

	return pGameResource->GetMarineResourceIndex( pMR );
}
