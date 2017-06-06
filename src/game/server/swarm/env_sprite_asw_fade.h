#ifndef _INCLUDED_ENV_SPRITE_ASW_FADE_H
#define _INCLUDED_ENV_SPRITE_ASW_FADE_H
#ifdef _WIN32
#pragma once
#endif

#include "Sprite.h"

class CEnv_Sprite_ASW_Fade : public CSprite
{
public:
	DECLARE_CLASS( CEnv_Sprite_ASW_Fade, CSprite );

	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

	CEnv_Sprite_ASW_Fade();

	CNetworkVector( m_vecFadeOrigin );
protected:
	CNetworkVar( byte, m_nFadeOpacity );
	CNetworkVar( bool, m_bAllowFade );
};


#endif // _INCLUDED_ENV_SPRITE_ASW_FADE_H
