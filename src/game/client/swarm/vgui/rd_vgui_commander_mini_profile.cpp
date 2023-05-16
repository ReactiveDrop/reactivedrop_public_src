#include "cbase.h"
#include "rd_vgui_commander_mini_profile.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

DECLARE_BUILD_FACTORY( CRD_VGUI_Commander_Mini_Profile );

CRD_VGUI_Commander_Mini_Profile::CRD_VGUI_Commander_Mini_Profile( vgui::Panel *parent, const char *panelName ) :
	BaseClass( parent, panelName )
{
}

CRD_VGUI_Commander_Mini_Profile::~CRD_VGUI_Commander_Mini_Profile()
{
}

void CRD_VGUI_Commander_Mini_Profile::ClearHoIAFData()
{
}
