#include "cbase.h"
#include "env_sprite_asw_fade.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( env_sprite_asw_fade, CEnv_Sprite_ASW_Fade );

BEGIN_DATADESC( CEnv_Sprite_ASW_Fade )
	DEFINE_KEYFIELD( m_nFadeOpacity, FIELD_CHARACTER, "fade_opacity" ),
	DEFINE_INPUT( m_bAllowFade, FIELD_BOOLEAN, "AllowFade" ),
	DEFINE_KEYFIELD( m_vecFadeOrigin, FIELD_VECTOR, "fade_origin" ),
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CEnv_Sprite_ASW_Fade, DT_Env_Sprite_ASW_Fade )
	SendPropInt( SENDINFO( m_nFadeOpacity ), 8, SPROP_UNSIGNED ),
	SendPropBool( SENDINFO( m_bAllowFade ) ),
	SendPropVector( SENDINFO( m_vecFadeOrigin ) ),
END_SEND_TABLE()

CEnv_Sprite_ASW_Fade::CEnv_Sprite_ASW_Fade()
{
	m_nFadeOpacity = 0;
	m_bAllowFade = true;
	m_vecFadeOrigin = vec3_origin;
}
