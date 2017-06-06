#ifndef _INCLUDED_C_PROP_ASW_FADE_H
#define _INCLUDED_C_PROP_ASW_FADE_H
#ifdef _WIN32
#pragma once
#endif

#include "c_props.h"
#include "iasw_fade_list.h"

class C_Prop_ASW_Fade : public C_DynamicProp, public IASW_Fade_List
{
public:
	DECLARE_CLASS( C_Prop_ASW_Fade, C_DynamicProp );
	DECLARE_CLIENTCLASS();

	C_Prop_ASW_Fade();

	Vector m_vecFadeOrigin;

	IMPLEMENT_ASW_FADE_LIST( m_vecFadeOrigin );
};

#endif	// _INCLUDED_C_PROP_ASW_FADE_H
