#ifndef _INCLUDED_NB_HSVSlider_PANEL_H
#define _INCLUDED_NB_HSVSlider_PANEL_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui/VGUI.h>
#include <vgui_controls/EditablePanel.h>
#include "asw_shareddefs.h"
#include "steam/steam_api.h"
#include "bitmapimagepanel.h"
#include <vgui_controls/ImagePanel.h>

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

class CHSVSliderPanel : public vgui::ImagePanel
{
	DECLARE_CLASS_SIMPLE(CHSVSliderPanel, vgui::ImagePanel);
public:
	CHSVSliderPanel(vgui::Panel* parent, const char* name);
	virtual ~CHSVSliderPanel();

	virtual void PerformLayout();

	virtual void OnMousePressed(vgui::MouseCode code);
	virtual void OnMouseReleased(vgui::MouseCode code);
	virtual void OnCursorMoved(int x, int y);

	int m_localMouseX = 0;
	int m_localMouseY = 0;

	bool m_bLMousePressed = false;

};
#endif

