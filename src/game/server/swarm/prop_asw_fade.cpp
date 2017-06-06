#include "cbase.h"
#include "prop_asw_fade.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( prop_asw_fade, CProp_ASW_Fade );

BEGIN_DATADESC( CProp_ASW_Fade )
	DEFINE_KEYFIELD( m_nFadeOpacity, FIELD_CHARACTER, "fade_opacity" ),
	DEFINE_INPUT( m_bAllowFade, FIELD_BOOLEAN, "AllowFade" ),
	DEFINE_KEYFIELD( m_vecFadeOrigin, FIELD_VECTOR, "fade_origin" ),
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CProp_ASW_Fade, DT_Prop_ASW_Fade )
	SendPropInt( SENDINFO( m_nFadeOpacity ), 8, SPROP_UNSIGNED ),
	SendPropBool( SENDINFO( m_bAllowFade ) ),
	SendPropVector( SENDINFO( m_vecFadeOrigin ) ),
END_SEND_TABLE()

CProp_ASW_Fade::CProp_ASW_Fade()
{
	m_nFadeOpacity = 0;
	m_bAllowFade = true;
	m_vecFadeOrigin = vec3_origin;
}
