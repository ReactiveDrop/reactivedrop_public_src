#ifndef _INCLUDED_NB_HSVCOLOR_PANEL_H
#define _INCLUDED_NB_HSVCOLOR_PANEL_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui/VGUI.h>
#include <vgui_controls/EditablePanel.h>
#include "asw_shareddefs.h"
#include "steam/steam_api.h"
#include "bitmapimagepanel.h"
#include <vgui_controls/ImagePanel.h>
#include "nb_lobby_laser_rgb_menu.h"

class ChatEchoPanel;

// == MANAGED_CLASS_DECLARATIONS_START: Do not edit by hand ==
class vgui::ImagePanel;
class vgui::Label;
class StatsBar;
class vgui::Panel;
class CAvatarImagePanel;
// == MANAGED_CLASS_DECLARATIONS_END ==
class CBitmapButton;
class CNB_Lobby_Tooltip;
class CNB_Main_Panel;

namespace BaseModUI
{
	class DropDownMenu;
}

class CHSVColorSquarePanel : public vgui::ImagePanel
{
	DECLARE_CLASS_SIMPLE(CHSVColorSquarePanel, vgui::ImagePanel);
public:
	CHSVColorSquarePanel(vgui::Panel* parent, const char* name);
	virtual ~CHSVColorSquarePanel();

	virtual void PerformLayout();

	virtual void OnMousePressed(vgui::MouseCode code);
	virtual void OnKeyCodePressed(vgui::KeyCode code);
	void OnMouseReleased(vgui::MouseCode code);
	virtual void OnKeyCodeTyped(vgui::KeyCode code);
	virtual void OnCursorMoved(int x, int y);
	virtual void OnThink();

	void CheckHSVUpdate();

	bool m_LeftMousePressed = false;

	int m_localMouseX = 0;
	int m_localMouseY = 0;

};
#endif

