#ifndef _INCLUDED_ASW_VGUI_COMPUTER_CUSTOM_HACK_H
#define _INCLUDED_ASW_VGUI_COMPUTER_CUSTOM_HACK_H

#include "asw_vgui_computer_custom.h"

class CASW_VGUI_Computer_Custom_Hack : public CASW_VGUI_Computer_Custom
{
	DECLARE_CLASS_SIMPLE( CASW_VGUI_Computer_Custom_Hack, CASW_VGUI_Computer_Custom );
public:
	CASW_VGUI_Computer_Custom_Hack( vgui::Panel *pParent, const char *pElementName, C_ASW_Hack_Computer *pHackComputer, C_RD_Computer_VScript *pCustom );
	virtual ~CASW_VGUI_Computer_Custom_Hack();
};

#endif /* _INCLUDED_ASW_VGUI_COMPUTER_CUSTOM_HACK_H */
