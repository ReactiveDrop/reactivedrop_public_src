#pragma once

#include "vgui_controls/EditablePanel.h"
#include "gameui/swarm/vhybridbutton.h"

class CRD_VGUI_Main_Menu_Top_Bar : public vgui::EditablePanel
{
public:
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Main_Menu_Top_Bar, vgui::EditablePanel );

	CRD_VGUI_Main_Menu_Top_Bar( vgui::Panel *parent, const char *panelName );
	~CRD_VGUI_Main_Menu_Top_Bar();

	void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	void OnCommand( const char *command ) override;
	void PaintBackground() override;
	void NavigateTo() override;

	void DismissMainMenuScreens();

	BaseModUI::BaseModHybridButton *m_pBtnSettings;
	BaseModUI::BaseModHybridButton *m_pBtnLogo;
	BaseModUI::BaseModHybridButton *m_pTopButton[6];
	BaseModUI::BaseModHybridButton *m_pBtnQuit;

	bool m_bLeftGlow;
	bool m_bRightGlow;
};
