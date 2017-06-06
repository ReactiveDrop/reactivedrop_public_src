#include "cbase.h"
#include "c_env_sprite_asw_fade.h"
#include "c_asw_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_CLIENTCLASS_DT( C_Env_Sprite_ASW_Fade, DT_Env_Sprite_ASW_Fade, CEnv_Sprite_ASW_Fade )
	RecvPropInt( RECVINFO( m_nFadeOpacity ) ),
	RecvPropBool( RECVINFO( m_bAllowFade ) ),
	RecvPropVector( RECVINFO( m_vecFadeOrigin ) ),
END_RECV_TABLE()

extern ConVar asw_fade_duration;

C_Env_Sprite_ASW_Fade::C_Env_Sprite_ASW_Fade()
{
}
