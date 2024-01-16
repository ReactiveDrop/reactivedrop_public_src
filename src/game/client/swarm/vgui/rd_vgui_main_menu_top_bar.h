#pragma once

#include "vgui_controls/EditablePanel.h"
#include "gameui/swarm/vhybridbutton.h"
#include "rd_hud_glow_helper.h"

class CRD_VGUI_Notifications_Button;

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

	enum
	{
		BTN_LOADOUTS,
#ifdef RD_7A_QUESTS
		BTN_CONTRACTS,
#endif
		BTN_RECORDINGS,
		BTN_SWARMOPEDIA,
		BTN_WORKSHOP,
		BTN_INVENTORY,
		NUM_TOP_BUTTONS,
	};

	vgui::DHANDLE<BaseModUI::BaseModHybridButton> m_hActiveButton;

	BaseModUI::BaseModHybridButton *m_pBtnSettings;
	BaseModUI::BaseModHybridButton *m_pBtnLogo;
	BaseModUI::BaseModHybridButton *m_pTopButton[NUM_TOP_BUTTONS];
	CRD_VGUI_Notifications_Button *m_pBtnNotifications;
	BaseModUI::BaseModHybridButton *m_pBtnQuit;

	HUDGlowHelper_t m_GlowSettings;
	HUDGlowHelper_t m_GlowLogo;
	HUDGlowHelper_t m_GlowTopButton[6];
	HUDGlowHelper_t m_GlowNotifications;
	HUDGlowHelper_t m_GlowQuit;

	uint8_t m_iLeftGlow;
	uint8_t m_iRightGlow;
};
