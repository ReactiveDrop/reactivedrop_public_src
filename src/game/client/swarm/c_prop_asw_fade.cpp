#include "cbase.h"
#include "c_prop_asw_fade.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_CLIENTCLASS_DT( C_Prop_ASW_Fade, DT_Prop_ASW_Fade, CProp_ASW_Fade )
	RecvPropInt( RECVINFO( m_nFadeOpacity ) ),
	RecvPropBool( RECVINFO( m_bAllowFade ) ),
	RecvPropFloat( RECVINFO( m_flFadeOriginOffset ) ),
	RecvPropBool( RECVINFO( m_bHasProxies ) ),
END_RECV_TABLE()

C_Prop_ASW_Fade::C_Prop_ASW_Fade()
{
}
