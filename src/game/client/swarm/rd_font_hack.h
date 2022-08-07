#pragma once

#include <vgui/VGUI.h>

class CRD_Font_Hack : public CAutoGameSystem
{
public:
	CRD_Font_Hack() : CAutoGameSystem( "CRD_Font_Hack" ) {}
	virtual bool Init();
};

extern CRD_Font_Hack g_ReactiveDropFontHack;
