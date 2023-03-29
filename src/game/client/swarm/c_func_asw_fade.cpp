#include "cbase.h"
#include "c_func_asw_fade.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_CLIENTCLASS_DT( C_Func_ASW_Fade, DT_Func_ASW_Fade, CFunc_ASW_Fade )
	RecvPropInt( RECVINFO( m_nFadeOpacity ) ),
	RecvPropBool( RECVINFO( m_bAllowFade ) ),
	RecvPropBool( RECVINFO( m_bHasProxies ) ),
END_RECV_TABLE()

C_Func_ASW_Fade::C_Func_ASW_Fade()
{
}
