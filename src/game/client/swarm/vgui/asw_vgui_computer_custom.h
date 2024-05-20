#ifndef _INCLUDED_ASW_VGUI_COMPUTER_CUSTOM_H
#define _INCLUDED_ASW_VGUI_COMPUTER_CUSTOM_H

#include "asw_vgui_ingame_panel.h"

class C_RD_Computer_VScript;
class C_ASW_Hack_Computer;

class CASW_VGUI_Computer_Custom : public vgui::Panel, public CASW_VGUI_Ingame_Panel
{
	DECLARE_CLASS_SIMPLE( CASW_VGUI_Computer_Custom, vgui::Panel );
public:
	CASW_VGUI_Computer_Custom( vgui::Panel *pParent, const char *pElementName, C_ASW_Hack_Computer *pHackComputer, C_RD_Computer_VScript *pCustom );
	virtual ~CASW_VGUI_Computer_Custom();

	void ASWInit();
	void ASWClose();
	void PerformLayout() override;
	void OnThink() override;
	void Paint() override;
	bool MouseClick( int x, int y, bool bRightClick, bool bDown ) override;

	// current computer hack
	CHandle<C_ASW_Hack_Computer> m_hHackComputer;
	CHandle<C_RD_Computer_VScript> m_hCustom;

	// overall scale of this window
	float m_fScale;

	bool m_bInitialized;
};

#endif /* _INCLUDED_ASW_VGUI_COMPUTER_CUSTOM_H */
