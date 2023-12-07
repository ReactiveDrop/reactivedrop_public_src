#include "cbase.h"
#include "asw_vgui_computer_custom_hack.h"
#include "asw_vgui_computer_frame.h"
#include "clientmode.h"
#include "rd_computer_vscript_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CASW_VGUI_Computer_Custom_Hack::CASW_VGUI_Computer_Custom_Hack( vgui::Panel *pParent, const char *pElementName, C_ASW_Hack_Computer *pHackComputer, C_RD_Computer_VScript *pCustom ) :
	BaseClass( pParent, pElementName, pHackComputer, pCustom )
{
}

CASW_VGUI_Computer_Custom_Hack::~CASW_VGUI_Computer_Custom_Hack()
{
}
