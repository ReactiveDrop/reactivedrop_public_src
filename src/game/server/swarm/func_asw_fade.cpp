#include "cbase.h"
#include "func_asw_fade.h"
#include "asw_shareddefs.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( func_asw_fade, CFunc_ASW_Fade );

BEGIN_DATADESC( CFunc_ASW_Fade )
	DEFINE_KEYFIELD( m_bCollideWithGrenades, FIELD_BOOLEAN, "CollideWithGrenades" ),
	DEFINE_KEYFIELD( m_nFadeOpacity, FIELD_CHARACTER, "fade_opacity" ),
	DEFINE_INPUT( m_bAllowFade, FIELD_BOOLEAN, "AllowFade" ),
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CFunc_ASW_Fade, DT_Func_ASW_Fade )
	SendPropInt( SENDINFO( m_nFadeOpacity ), 8, SPROP_UNSIGNED ),
	SendPropBool( SENDINFO( m_bAllowFade ) ),
END_SEND_TABLE()

CFunc_ASW_Fade::CFunc_ASW_Fade()
{
	m_bCollideWithGrenades = false;
	m_nFadeOpacity = 0;
	m_bAllowFade = true;
}

void CFunc_ASW_Fade::Spawn()
{
	BaseClass::Spawn();

	SetCollisionGroup( m_bCollideWithGrenades ? COLLISION_GROUP_NONE : ASW_COLLISION_GROUP_CEILINGS );
}
