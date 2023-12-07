#ifndef _INCLUDED_ASW_VGUI_COMPUTER_CUSTOM_HACK_H
#define _INCLUDED_ASW_VGUI_COMPUTER_CUSTOM_HACK_H

#include "asw_vgui_ingame_panel.h"

class C_RD_Computer_VScript;
class C_ASW_Hack_Computer;

class CASW_VGUI_Computer_Custom_Hack : public vgui::Panel, public CASW_VGUI_Ingame_Panel
{
	DECLARE_CLASS_SIMPLE( CASW_VGUI_Computer_Custom_Hack, vgui::Panel );
public:
	CASW_VGUI_Computer_Custom_Hack( vgui::Panel *pParent, const char *pElementName, C_ASW_Hack_Computer *pHackComputer, C_RD_Computer_VScript *pCustom );
	virtual ~CASW_VGUI_Computer_Custom_Hack();

	void PerformLayout() override;
	void Paint() override;

	// current computer hack
	CHandle<C_ASW_Hack_Computer> m_hHackComputer;
	CHandle<C_RD_Computer_VScript> m_hCustom;

	// overall scale of this window
	float m_fScale;
};

#endif /* _INCLUDED_ASW_VGUI_COMPUTER_CUSTOM_HACK_H */
