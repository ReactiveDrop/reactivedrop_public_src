#ifndef _INCLUDED_NB_LOBBY_LASER_RGB_MENU_H
#define _INCLUDED_NB_LOBBY_LASER_RGB_MENU_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui/VGUI.h>
#include <vgui_controls/EditablePanel.h>
#include "asw_shareddefs.h"
#include "steam/steam_api.h"
#include "HSVSliderPanel.h"

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
class HSVColorSquarePanel;
class CHSVSliderPanel;

namespace BaseModUI
{
	class DropDownMenu;
}

enum LaserRGBMenuFocusMode
{
	Main,
	HSVSquare,
	HSVSlider
};

class Cnb_lobby_laser_rgb_menu : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE(Cnb_lobby_laser_rgb_menu, vgui::EditablePanel);
public:
	Cnb_lobby_laser_rgb_menu(vgui::Panel* parent, const char* name);
	~Cnb_lobby_laser_rgb_menu();

	virtual void ApplySchemeSettings(vgui::IScheme* pScheme);
	virtual void PerformLayout();
	virtual void OnThink();
	virtual void OnCommand(const char* command);
	void OpenPlayerFlyout();
	CNB_Main_Panel* GetMainPanel();

	virtual void UpdateDetails();
	virtual void CheckTooltip(CNB_Lobby_Tooltip* pTooltip);

	virtual void OnMousePressed(vgui::MouseCode code);
	virtual void OnCursorMoved(int x, int y);

	//virtual bool OnControllerButtonPressed(ButtonCode_t keynum);
	//virtual void OnKeyCodePressed(vgui::KeyCode code);
	//virtual void OnKeyCodeTyped(vgui::KeyCode code);

	void SetHSV_Hue(float outputHue);
	void SetHSV_Sat(float outputSat);
	void SetHSV_Val(float outputVal);
	void RecalculateHSVColor();
	void UpdateHSVColor();
	void SetHSVMarkerPos();
	void SetColorData(Color datCol, int style, int size);

	void SetChangingLaserColor(Color rgbColor);

	color32 m_currentLSColor = color32();


	vgui::ImagePanel* m_pBackground;
	vgui::ImagePanel*  m_pHSVSquare;
	vgui::ImagePanel* m_pHSVSquareMarker;

	CBitmapButton* m_pLaserButton;
	CBitmapButton* m_pLaserOverlayButton;

	CHSVSliderPanel* m_pHSVSlider;
	vgui::ImagePanel* m_pHSVSliderMarker;

	CBitmapButton* m_pCustomColor1;
	CBitmapButton* m_pCustomColor2;
	CBitmapButton* m_pCustomColor3;
	CBitmapButton* m_pCustomColor4;
	CBitmapButton* m_pCustomColor5;
	CBitmapButton* m_pCustomColor6;
	CBitmapButton* m_pCustomColor7;
	CBitmapButton* m_pCustomColor8;
	CBitmapButton* m_pCustomColor9;

	vgui::ImagePanel* m_pHighlightColor1;
	vgui::ImagePanel* m_pHighlightColor2;
	vgui::ImagePanel* m_pHighlightColor3;
	vgui::ImagePanel* m_pHighlightColor4;
	vgui::ImagePanel* m_pHighlightColor5;
	vgui::ImagePanel* m_pHighlightColor6;
	vgui::ImagePanel* m_pHighlightColor7;
	vgui::ImagePanel* m_pHighlightColor8;
	vgui::ImagePanel* m_pHighlightColor9;


	vgui::ImagePanel* m_pHighlightHSVSquare;
	vgui::ImagePanel* m_pHightlightHSVPreview;
	vgui::ImagePanel* m_pHighlightHSVSlider;
	vgui::ImagePanel* m_pHighlightReplaceColor;

	CBitmapButton* m_pReplaceColorButton;

	int m_localMouseX = 0;
	int m_localMouseY = 0;

	float m_fHSV_Hue = 0.0f;
	float m_fHSV_Val = 1.0f;
	float m_fHSV_Sat = 1.0f;

	bool m_bReplaceColor = false;
	
	LaserRGBMenuFocusMode m_CurrentFocusMode = LaserRGBMenuFocusMode::Main;
};
#endif
