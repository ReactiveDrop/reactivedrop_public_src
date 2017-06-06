#ifndef _INCLUDED_C_ENV_SPRITE_ASW_FADE_H
#define _INCLUDED_C_ENV_SPRITE_ASW_FADE_H
#ifdef _WIN32
#pragma once
#endif

#include "Sprite.h"
#include "c_asw_marine.h"
#include "iasw_fade_list.h"

class C_Env_Sprite_ASW_Fade : public C_Sprite, public IASW_Fade_List
{
public:
	DECLARE_CLASS( C_Env_Sprite_ASW_Fade, C_Sprite );
	DECLARE_CLIENTCLASS();

	C_Env_Sprite_ASW_Fade();

	Vector m_vecFadeOrigin;

	IMPLEMENT_ASW_FADE_LIST( m_vecFadeOrigin );
};

#endif	// _INCLUDED_C_ENV_SPRITE_ASW_FADE_H
