#ifndef _INCLUDED_C_FUNC_ASW_FADE_H
#define _INCLUDED_C_FUNC_ASW_FADE_H
#ifdef _WIN32
#pragma once
#endif

#include "c_func_brush.h"
#include "iasw_fade_list.h"

class C_Func_ASW_Fade : public C_FuncBrush, public IASW_Fade_List
{
public:
	DECLARE_CLASS( C_Func_ASW_Fade, C_FuncBrush );
	DECLARE_CLIENTCLASS();

	C_Func_ASW_Fade();

	IMPLEMENT_ASW_FADE_LIST( GetAbsOrigin() );
};

#endif	// _INCLUDED_C_FUNC_ASW_FADE_H
