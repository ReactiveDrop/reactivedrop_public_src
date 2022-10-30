#include "cbase.h"
#include "nb_lobby_laser_rgb_menu.h"
#include "vgui_controls/ImagePanel.h"
#include "vgui_controls/Label.h"
#include "vgui_controls/Panel.h"
#include "StatsBar.h"
#include "vgui_bitmapbutton.h"
#include <vgui/ILocalize.h>
#include "asw_marine_profile.h"
#include "asw_briefing.h"
#include "asw_equipment_list.h"
#include "asw_weapon_parse.h"
#include "asw_player_shared.h"
#include "nb_lobby_tooltip.h"
#include "controller_focus.h"
#include "nb_main_panel.h"
#include "vgui_avatarimage.h"
#include "voice_status.h"
#include "gameui/swarm/vflyoutmenu.h"
#include "gameui/swarm/vdropdownmenu.h"
#include "gameui/swarm/vhybridbutton.h"
#include "rd_inventory_shared.h"
#include <vgui/IInput.h>
#include "HSVColorSquarePanel.h"
#include "HSVSliderPanel.h"
#include <vgui_controls/Controls.h>
#include <vgui/ISystem.h>
#include "LaserHelperFunctions_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace BaseModUI;

extern ConVar cl_asw_laser_sight_color;

extern ConVar cl_asw_archived_lsc1;
extern ConVar cl_asw_archived_lsc2;
extern ConVar cl_asw_archived_lsc3;
extern ConVar cl_asw_archived_lsc4;
extern ConVar cl_asw_archived_lsc5;
extern ConVar cl_asw_archived_lsc6;
extern ConVar cl_asw_archived_lsc7;
extern ConVar cl_asw_archived_lsc8;
extern ConVar cl_asw_archived_lsc9;

Cnb_lobby_laser_rgb_menu::Cnb_lobby_laser_rgb_menu(vgui::Panel* parent, const char* name) : BaseClass(parent, name)
{
	int outLaserR = 0, outLaserG = 0, outLaserB = 0, outLaserStyle = 0, outLaserSize = 0;
	LaserHelper::SplitLaserConvar(&cl_asw_laser_sight_color, outLaserR, outLaserG, outLaserB, outLaserStyle, outLaserSize);
	Color laserCol = Color(outLaserR, outLaserG, outLaserB);
	SetColorData(laserCol, outLaserStyle, outLaserSize);

	m_pBackground = new vgui::ImagePanel(this, "Background");
	m_pHSVSquare = new CHSVColorSquarePanel(this, "HSVSquare");

	m_pLaserButton = new CBitmapButton(this, "LaserPreviewButton", "");
	m_pLaserOverlayButton = new CBitmapButton(this, "LaserPreviewOverlayButton", "");

	m_pHSVSlider = new CHSVSliderPanel(this, "HSVSlider");
	m_pHSVSlider->SetSize(20, 75);

	m_pHSVSquareMarker = new vgui::ImagePanel(this, "HSVSquareMarker");
	m_pHSVSliderMarker = new vgui::ImagePanel(this, "HSVSquareSliderMarker");


	m_pCustomColor1 = new CBitmapButton(this, "Color1Button", "");
	m_pCustomColor2 = new CBitmapButton(this, "Color2Button", "");
	m_pCustomColor3 = new CBitmapButton(this, "Color3Button", "");
	m_pCustomColor4 = new CBitmapButton(this, "Color4Button", "");
	m_pCustomColor5 = new CBitmapButton(this, "Color5Button", "");
	m_pCustomColor6 = new CBitmapButton(this, "Color6Button", "");
	m_pCustomColor7 = new CBitmapButton(this, "Color7Button", "");
	m_pCustomColor8 = new CBitmapButton(this, "Color8Button", "");
	m_pCustomColor9 = new CBitmapButton(this, "Color9Button", "");


	m_pHighlightColor1 = new vgui::ImagePanel(this, "Color1Highlight");
	m_pHighlightColor2 = new vgui::ImagePanel(this, "Color2Highlight");
	m_pHighlightColor3 = new vgui::ImagePanel(this, "Color3Highlight");
	m_pHighlightColor4 = new vgui::ImagePanel(this, "Color4Highlight");
	m_pHighlightColor5 = new vgui::ImagePanel(this, "Color5Highlight");
	m_pHighlightColor6 = new vgui::ImagePanel(this, "Color6Highlight");
	m_pHighlightColor7 = new vgui::ImagePanel(this, "Color7Highlight");
	m_pHighlightColor8 = new vgui::ImagePanel(this, "Color8Highlight");
	m_pHighlightColor9 = new vgui::ImagePanel(this, "Color9Highlight");

	m_pHighlightHSVSquare = new vgui::ImagePanel(this, "HSVSquareHighlight");
	m_pHightlightHSVPreview = new vgui::ImagePanel(this, "HSVPreviewHighlight");

	m_pHighlightHSVSlider = new vgui::ImagePanel(this, "HSVSliderHighlight");

	m_pHighlightReplaceColor = new vgui::ImagePanel(this, "ActivateReplaceColorHighlight");



	m_pReplaceColorButton = new CBitmapButton(this, "ActivateReplaceColorButton", "");


	m_pCustomColor1->SetVisible(true);
	m_pCustomColor2->SetVisible(true);
	m_pCustomColor3->SetVisible(true);
	m_pCustomColor4->SetVisible(true);
	m_pCustomColor5->SetVisible(true);
	m_pCustomColor6->SetVisible(true);
	m_pCustomColor7->SetVisible(true);
	m_pCustomColor8->SetVisible(true);
	m_pCustomColor9->SetVisible(true);

	m_pReplaceColorButton->SetVisible(true);

	m_pCustomColor1->SetCommand("SetCustLaser1");
	m_pCustomColor2->SetCommand("SetCustLaser2");
	m_pCustomColor3->SetCommand("SetCustLaser3");
	m_pCustomColor4->SetCommand("SetCustLaser4");
	m_pCustomColor5->SetCommand("SetCustLaser5");
	m_pCustomColor6->SetCommand("SetCustLaser6");
	m_pCustomColor7->SetCommand("SetCustLaser7");
	m_pCustomColor8->SetCommand("SetCustLaser8");
	m_pCustomColor9->SetCommand("SetCustLaser9");

	m_pReplaceColorButton->SetCommand("ActivateReplaceColor");

	GetControllerFocus()->AddToFocusList(m_pHSVSquare);
	GetControllerFocus()->AddToFocusList(m_pHSVSlider);

	GetControllerFocus()->AddToFocusList(m_pCustomColor1);
	GetControllerFocus()->AddToFocusList(m_pCustomColor2);
	GetControllerFocus()->AddToFocusList(m_pCustomColor3);
	GetControllerFocus()->AddToFocusList(m_pCustomColor4);
	GetControllerFocus()->AddToFocusList(m_pCustomColor5);
	GetControllerFocus()->AddToFocusList(m_pCustomColor6);
	GetControllerFocus()->AddToFocusList(m_pCustomColor7);
	GetControllerFocus()->AddToFocusList(m_pCustomColor8);
	GetControllerFocus()->AddToFocusList(m_pCustomColor9);

	GetControllerFocus()->AddToFocusList(m_pReplaceColorButton);

	InvalidateLayout(true, true);
}

Cnb_lobby_laser_rgb_menu::~Cnb_lobby_laser_rgb_menu()
{
	GetControllerFocus()->SetIsRawOverride(false);
}

void Cnb_lobby_laser_rgb_menu::ApplySchemeSettings(vgui::IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	LoadControlSettings("resource/ui/basemodui/nb_lobby_laser_rgb_menu.res");
}

void Cnb_lobby_laser_rgb_menu::PerformLayout()
{
	BaseClass::PerformLayout();

}

void Cnb_lobby_laser_rgb_menu::OnThink()
{
	BaseClass::OnThink();

	UpdateDetails();
}

void Cnb_lobby_laser_rgb_menu::UpdateDetails()
{
	int outLaserR = 0, outLaserG = 0, outLaserB = 0, outLaserStyle = 0, outLaserSize = 0;
	LaserHelper::SplitLaserConvar(&cl_asw_laser_sight_color, outLaserR, outLaserG, outLaserB, outLaserStyle, outLaserSize);
	Color _inputRGBColor = Color(outLaserR, outLaserG, outLaserB);
	color32 rgbLaserColor = color32();
	rgbLaserColor.r = _inputRGBColor.r();
	rgbLaserColor.g = _inputRGBColor.g();
	rgbLaserColor.b = _inputRGBColor.b();
	rgbLaserColor.a = 255;

	color32 invisible;
	invisible.r = 0;
	invisible.g = 0;
	invisible.b = 0;
	invisible.a = 0;

	color32 white;
	white.r = 255;
	white.g = 255;
	white.b = 255;
	white.a = 255;

	color32 lightgrey;
	lightgrey.r = 255;
	lightgrey.g = 255;
	lightgrey.b = 255;
	lightgrey.a = 255;

	int outRed = 0, outGreen = 0, outBlue = 0, outStyle = 0, outSize = 0;
	LaserHelper::SplitLaserConvar(&cl_asw_archived_lsc1, outRed, outGreen, outBlue, outStyle, outSize);
	Color _inputRGBCustColor1 = Color(outRed, outGreen, outBlue);
	color32 rgbLaserCustColor1 = color32();
	rgbLaserCustColor1.r = _inputRGBCustColor1.r();
	rgbLaserCustColor1.g = _inputRGBCustColor1.g();
	rgbLaserCustColor1.b = _inputRGBCustColor1.b();
	rgbLaserCustColor1.a = 255;
	m_pCustomColor1->SetImage(CBitmapButton::BUTTON_ENABLED, "vgui/swarm/color/color_highlight", rgbLaserCustColor1);
	m_pCustomColor1->SetImage(CBitmapButton::BUTTON_ENABLED_MOUSE_OVER, "vgui/swarm/color/color_highlight", rgbLaserCustColor1);
	m_pCustomColor1->SetImage(CBitmapButton::BUTTON_PRESSED, "vgui/white", rgbLaserCustColor1);

	LaserHelper::SplitLaserConvar(&cl_asw_archived_lsc2, outRed, outGreen, outBlue, outStyle, outSize);
	Color _inputRGBCustColor2 = Color(outRed, outGreen, outBlue);
	color32 rgbLaserCustColor2 = color32();
	rgbLaserCustColor2.r = _inputRGBCustColor2.r();
	rgbLaserCustColor2.g = _inputRGBCustColor2.g();
	rgbLaserCustColor2.b = _inputRGBCustColor2.b();
	rgbLaserCustColor2.a = 255;
	m_pCustomColor2->SetImage(CBitmapButton::BUTTON_ENABLED, "vgui/swarm/color/color_highlight", rgbLaserCustColor2);
	m_pCustomColor2->SetImage(CBitmapButton::BUTTON_ENABLED_MOUSE_OVER, "vgui/swarm/color/color_highlight", rgbLaserCustColor2);
	m_pCustomColor2->SetImage(CBitmapButton::BUTTON_PRESSED, "vgui/white", rgbLaserCustColor2);

	LaserHelper::SplitLaserConvar(&cl_asw_archived_lsc3, outRed, outGreen, outBlue, outStyle, outSize);
	Color _inputRGBCustColor3 = Color(outRed, outGreen, outBlue);
	color32 rgbLaserCustColor3 = color32();
	rgbLaserCustColor3.r = _inputRGBCustColor3.r();
	rgbLaserCustColor3.g = _inputRGBCustColor3.g();
	rgbLaserCustColor3.b = _inputRGBCustColor3.b();
	rgbLaserCustColor3.a = 255;
	m_pCustomColor3->SetImage(CBitmapButton::BUTTON_ENABLED, "vgui/swarm/color/color_highlight", rgbLaserCustColor3);
	m_pCustomColor3->SetImage(CBitmapButton::BUTTON_ENABLED_MOUSE_OVER, "vgui/swarm/color/color_highlight", rgbLaserCustColor3);
	m_pCustomColor3->SetImage(CBitmapButton::BUTTON_PRESSED, "vgui/white", rgbLaserCustColor3);

	LaserHelper::SplitLaserConvar(&cl_asw_archived_lsc4, outRed, outGreen, outBlue, outStyle, outSize);
	Color _inputRGBCustColor4 = Color(outRed, outGreen, outBlue);
	color32 rgbLaserCustColor4 = color32();
	rgbLaserCustColor4.r = _inputRGBCustColor4.r();
	rgbLaserCustColor4.g = _inputRGBCustColor4.g();
	rgbLaserCustColor4.b = _inputRGBCustColor4.b();
	rgbLaserCustColor4.a = 255;
	m_pCustomColor4->SetImage(CBitmapButton::BUTTON_ENABLED, "vgui/swarm/color/color_highlight", rgbLaserCustColor4);
	m_pCustomColor4->SetImage(CBitmapButton::BUTTON_ENABLED_MOUSE_OVER, "vgui/swarm/color/color_highlight", rgbLaserCustColor4);
	m_pCustomColor4->SetImage(CBitmapButton::BUTTON_PRESSED, "vgui/white", rgbLaserCustColor4);

	LaserHelper::SplitLaserConvar(&cl_asw_archived_lsc5, outRed, outGreen, outBlue, outStyle, outSize);
	Color _inputRGBCustColor5 = Color(outRed, outGreen, outBlue);
	color32 rgbLaserCustColor5 = color32();
	rgbLaserCustColor5.r = _inputRGBCustColor5.r();
	rgbLaserCustColor5.g = _inputRGBCustColor5.g();
	rgbLaserCustColor5.b = _inputRGBCustColor5.b();
	rgbLaserCustColor5.a = 255;
	m_pCustomColor5->SetImage(CBitmapButton::BUTTON_ENABLED, "vgui/swarm/color/color_highlight", rgbLaserCustColor5);
	m_pCustomColor5->SetImage(CBitmapButton::BUTTON_ENABLED_MOUSE_OVER, "vgui/swarm/color/color_highlight", rgbLaserCustColor5);
	m_pCustomColor5->SetImage(CBitmapButton::BUTTON_PRESSED, "vgui/white", rgbLaserCustColor5);


	LaserHelper::SplitLaserConvar(&cl_asw_archived_lsc6, outRed, outGreen, outBlue, outStyle, outSize);
	Color _inputRGBCustColor6 = Color(outRed, outGreen, outBlue);
	color32 rgbLaserCustColor6 = color32();
	rgbLaserCustColor6.r = _inputRGBCustColor6.r();
	rgbLaserCustColor6.g = _inputRGBCustColor6.g();
	rgbLaserCustColor6.b = _inputRGBCustColor6.b();
	rgbLaserCustColor6.a = 255;
	m_pCustomColor6->SetImage(CBitmapButton::BUTTON_ENABLED, "vgui/swarm/color/color_highlight", rgbLaserCustColor6);
	m_pCustomColor6->SetImage(CBitmapButton::BUTTON_ENABLED_MOUSE_OVER, "vgui/swarm/color/color_highlight", rgbLaserCustColor6);
	m_pCustomColor6->SetImage(CBitmapButton::BUTTON_PRESSED, "vgui/white", rgbLaserCustColor6);

	LaserHelper::SplitLaserConvar(&cl_asw_archived_lsc7, outRed, outGreen, outBlue, outStyle, outSize);
	Color _inputRGBCustColor7 = Color(outRed, outGreen, outBlue);
	color32 rgbLaserCustColor7 = color32();
	rgbLaserCustColor7.r = _inputRGBCustColor7.r();
	rgbLaserCustColor7.g = _inputRGBCustColor7.g();
	rgbLaserCustColor7.b = _inputRGBCustColor7.b();
	rgbLaserCustColor7.a = 255;
	m_pCustomColor7->SetImage(CBitmapButton::BUTTON_ENABLED, "vgui/swarm/color/color_highlight", rgbLaserCustColor7);
	m_pCustomColor7->SetImage(CBitmapButton::BUTTON_ENABLED_MOUSE_OVER, "vgui/swarm/color/color_highlight", rgbLaserCustColor7);
	m_pCustomColor7->SetImage(CBitmapButton::BUTTON_PRESSED, "vgui/white", rgbLaserCustColor7);

	LaserHelper::SplitLaserConvar(&cl_asw_archived_lsc8, outRed, outGreen, outBlue, outStyle, outSize);
	Color _inputRGBCustColor8 = Color(outRed, outGreen, outBlue);
	color32 rgbLaserCustColor8 = color32();
	rgbLaserCustColor8.r = _inputRGBCustColor8.r();
	rgbLaserCustColor8.g = _inputRGBCustColor8.g();
	rgbLaserCustColor8.b = _inputRGBCustColor8.b();
	rgbLaserCustColor8.a = 255;
	m_pCustomColor8->SetImage(CBitmapButton::BUTTON_ENABLED, "vgui/swarm/color/color_highlight", rgbLaserCustColor8);
	m_pCustomColor8->SetImage(CBitmapButton::BUTTON_ENABLED_MOUSE_OVER, "vgui/swarm/color/color_highlight", rgbLaserCustColor8);
	m_pCustomColor8->SetImage(CBitmapButton::BUTTON_PRESSED, "vgui/white", rgbLaserCustColor8);

	LaserHelper::SplitLaserConvar(&cl_asw_archived_lsc9, outRed, outGreen, outBlue, outStyle, outSize);
	Color _inputRGBCustColor9 = Color(outRed, outGreen, outBlue);
	color32 rgbLaserCustColor9 = color32();
	rgbLaserCustColor9.r = _inputRGBCustColor9.r();
	rgbLaserCustColor9.g = _inputRGBCustColor9.g();
	rgbLaserCustColor9.b = _inputRGBCustColor9.b();
	rgbLaserCustColor9.a = 255;
	m_pCustomColor9->SetImage(CBitmapButton::BUTTON_ENABLED, "vgui/swarm/color/color_highlight", rgbLaserCustColor9);
	m_pCustomColor9->SetImage(CBitmapButton::BUTTON_ENABLED_MOUSE_OVER, "vgui/swarm/color/color_highlight", rgbLaserCustColor9);
	m_pCustomColor9->SetImage(CBitmapButton::BUTTON_PRESSED, "vgui/white", rgbLaserCustColor9);


	m_pReplaceColorButton->SetImage(CBitmapButton::BUTTON_ENABLED, "vgui/swarm/color/hsv_replace_color_button", rgbLaserColor);
	m_pReplaceColorButton->SetImage(CBitmapButton::BUTTON_ENABLED_MOUSE_OVER, "vgui/swarm/color/hsv_replace_color_button", white);
	m_pReplaceColorButton->SetImage(CBitmapButton::BUTTON_PRESSED, "vgui/swarm/color/hsv_replace_color_button", rgbLaserColor);


	
	//float baseWidth = 15.0f, baseHeight = 15.0f;

	int custColX, custColY, custColWidth, custColHeight;

	m_pHSVSquare->GetPos(custColX, custColY);
	m_pHighlightHSVSquare->SetPos(custColX - 1, custColY - 1);

	m_pHSVSquare->GetSize(custColWidth, custColHeight);
	m_pHighlightHSVSquare->SetSize(custColWidth+2, custColHeight+2);

	m_pLaserButton->GetPos(custColX, custColY);
	m_pHightlightHSVPreview->SetPos(custColX - 1, custColY - 1);

	m_pLaserButton->GetSize(custColWidth, custColHeight);
	m_pHightlightHSVPreview->SetSize(custColWidth + 2, custColHeight + 2);

	m_pHSVSlider->GetPos(custColX, custColY);
	m_pHighlightHSVSlider->SetPos(custColX - 1, custColY - 1);

	m_pHSVSlider->GetSize(custColWidth, custColHeight);
	m_pHighlightHSVSlider->SetSize(custColWidth + 2, custColHeight + 2);


	m_pReplaceColorButton-> GetPos(custColX, custColY);
	m_pHighlightReplaceColor->SetPos(custColX - 1, custColY - 1);

	m_pReplaceColorButton->GetSize(custColWidth, custColHeight);
	m_pHighlightReplaceColor->SetSize(custColWidth + 2, custColHeight + 2);





	Panel* _focusedPanel = GetControllerFocus()->GetFocusPanel();

	if (_focusedPanel == m_pCustomColor1 || m_bReplaceColor)
	{
		m_pHighlightColor1->SetAlpha(55);
	}
	else
	{
		m_pHighlightColor1->SetAlpha(255);
	}

	if (_focusedPanel == m_pCustomColor2 || m_bReplaceColor)
	{
		m_pHighlightColor2->SetAlpha(55);
	}
	else
	{
		m_pHighlightColor2->SetAlpha(255);
	}

	if (_focusedPanel == m_pCustomColor3 || m_bReplaceColor)
	{
		m_pHighlightColor3->SetAlpha(55);
	}
	else
	{
		m_pHighlightColor3->SetAlpha(255);
	}

	if (_focusedPanel == m_pCustomColor4 || m_bReplaceColor)
	{
		m_pHighlightColor4->SetAlpha(55);
	}
	else
	{
		m_pHighlightColor4->SetAlpha(255);
	}

	if (_focusedPanel == m_pCustomColor5 || m_bReplaceColor)
	{
		m_pHighlightColor5->SetAlpha(55);
	}
	else
	{
		m_pHighlightColor5->SetAlpha(255);
	}

	if (_focusedPanel == m_pCustomColor6 || m_bReplaceColor)
	{
		m_pHighlightColor6->SetAlpha(55);
	}
	else
	{
		m_pHighlightColor6->SetAlpha(255);
	}

	if (_focusedPanel == m_pCustomColor7 || m_bReplaceColor)
	{
		m_pHighlightColor7->SetAlpha(55);
	}
	else
	{
		m_pHighlightColor7->SetAlpha(255);
	}

	if (_focusedPanel == m_pCustomColor8 || m_bReplaceColor)
	{
		m_pHighlightColor8->SetAlpha(55);
	}
	else
	{
		m_pHighlightColor8->SetAlpha(255);
	}

	if (_focusedPanel == m_pCustomColor9 || m_bReplaceColor)
	{
		m_pHighlightColor9->SetAlpha(55);
	}
	else
	{
		m_pHighlightColor9->SetAlpha(255);
	}

	m_pBackground->SetImage("swarm/color/HSVbackground");

	m_pLaserButton->SetImage(CBitmapButton::BUTTON_ENABLED, "vgui/swarm/color/laser_glow", rgbLaserColor);

	m_pHSVSlider->SetVisible(true);

	if (m_CurrentFocusMode == LaserRGBMenuFocusMode::HSVSquare || m_CurrentFocusMode == LaserRGBMenuFocusMode::HSVSlider)
	{
		if (GetControllerFocus()->m_bKeyDown[GetControllerFocus()->JF_KEY_CANCEL])
		{
			m_CurrentFocusMode = LaserRGBMenuFocusMode::Main;
		}
	}

	if (m_CurrentFocusMode == LaserRGBMenuFocusMode::Main)
	{
		GetControllerFocus()->SetIsRawOverride(false);
	}
	else if (m_CurrentFocusMode == LaserRGBMenuFocusMode::HSVSquare)
	{
		GetControllerFocus()->SetIsRawOverride(true);
		bool bControllerInputted = false;

		if (GetControllerFocus()->m_bKeyDown[GetControllerFocus()->JF_KEY_UP])
		{
			bControllerInputted = true;
			m_fHSV_Sat += 0.01;
			if (m_fHSV_Sat > 1.0f)
			{
				m_fHSV_Sat = 1.0f;
			}
		}
		else if (GetControllerFocus()->m_bKeyDown[GetControllerFocus()->JF_KEY_DOWN])
		{
			bControllerInputted = true;
			m_fHSV_Sat -= 0.01;
			if (m_fHSV_Sat < 0.0f)
			{
				m_fHSV_Sat = 0.0f;
			}
		}
		if (GetControllerFocus()->m_bKeyDown[GetControllerFocus()->JF_KEY_RIGHT])
		{
			bControllerInputted = true;
			m_fHSV_Hue += 3.0f;
			if (m_fHSV_Hue > 360.0f)
			{
				m_fHSV_Hue -= 360.0;
			}
		}
		else if (GetControllerFocus()->m_bKeyDown[GetControllerFocus()->JF_KEY_LEFT])
		{
			bControllerInputted = true;
			m_fHSV_Hue -= 3.0f;
			if (m_fHSV_Hue < 0.0f)
			{
				m_fHSV_Hue += 360.0;
			}
		}

		if (bControllerInputted)
		{
			SetHSVMarkerPos();
			RecalculateHSVColor();
			UpdateHSVColor();
		}
	}
	else if (m_CurrentFocusMode == LaserRGBMenuFocusMode::HSVSlider)
	{
		GetControllerFocus()->SetIsRawOverride(true);

		bool bControllerInputted = false;

		if (GetControllerFocus()->m_bKeyDown[GetControllerFocus()->JF_KEY_UP])
		{
			bControllerInputted = true;
			m_fHSV_Val += 0.01;
			if (m_fHSV_Val > 1.0f)
			{
				m_fHSV_Val = 1.0f;
			}
		}
		else if (GetControllerFocus()->m_bKeyDown[GetControllerFocus()->JF_KEY_DOWN])
		{
			bControllerInputted = true;
			m_fHSV_Val -= 0.01;
			if (m_fHSV_Val < 0.0f)
			{
				m_fHSV_Val = 0.0f;
			}
		}

		if (bControllerInputted)
		{
			SetHSVMarkerPos();
			RecalculateHSVColor();
			UpdateHSVColor();
		}
	}


	SetHSVMarkerPos();
}

void Cnb_lobby_laser_rgb_menu::CheckTooltip(CNB_Lobby_Tooltip* pTooltip)
{

}

extern ConVar developer;

void Cnb_lobby_laser_rgb_menu::SetColorData(Color datCol, int style, int size)
{
	m_currentLSColor.r = datCol.r();
	m_currentLSColor.g = datCol.g();
	m_currentLSColor.b = datCol.b();
	m_currentLSColor.a = 255;

	Vector vecCol = Vector(((float)m_currentLSColor.r) / 255.0f, ((float)m_currentLSColor.g) / 255.0f, ((float)m_currentLSColor.b) / 255.0f);
	Vector vecHSV = Vector(0, 0, 0);
	RGBtoHSV(vecCol, vecHSV);

	m_fHSV_Hue = vecHSV.x;
	m_fHSV_Sat = vecHSV.y;
	m_fHSV_Val = vecHSV.z;

	m_currentLaserStyle = style;
	m_currentLaserSize = size;
}

void Cnb_lobby_laser_rgb_menu::OnCommand(const char* command)
{
	CNB_Main_Panel* pMainPanel = GetMainPanel();
	if (!pMainPanel)
		return;

	if (!Q_stricmp(command, "SetLaserColor"))
	{
		LaserHelper::SetLaserConvar(&cl_asw_laser_sight_color, m_currentLSColor.r, m_currentLSColor.g, m_currentLSColor.b, m_currentLaserStyle, m_currentLaserSize);
	}
	else if (!Q_stricmp(command, "SetCustLaser1"))
	{
		if (m_bReplaceColor)
		{
			LaserHelper::SetLaserConvar(&cl_asw_archived_lsc1, m_currentLSColor.r, m_currentLSColor.g, m_currentLSColor.b, m_currentLaserStyle, m_currentLaserSize);
			m_bReplaceColor = false;
		}
		else
		{
			int outLaserR, outLaserG, outLaserB, outLaserStyle, outLaserSize;
			LaserHelper::SplitLaserConvar(&cl_asw_archived_lsc1, outLaserR, outLaserG, outLaserB, outLaserStyle, outLaserSize);
			Color laserCol = Color(outLaserR, outLaserG, outLaserB);
			SetColorData(laserCol, outLaserStyle, outLaserSize);

			SetHSVMarkerPos();
			RecalculateHSVColor();
			UpdateHSVColor();
		}
	}
	else if (!Q_stricmp(command, "SetCustLaser2"))
	{
		if (m_bReplaceColor)
		{
			LaserHelper::SetLaserConvar(&cl_asw_archived_lsc2, m_currentLSColor.r, m_currentLSColor.g, m_currentLSColor.b, m_currentLaserStyle, m_currentLaserSize);
			m_bReplaceColor = false;
		}
		else
		{
			int outLaserR, outLaserG, outLaserB, outLaserStyle, outLaserSize;
			LaserHelper::SplitLaserConvar(&cl_asw_archived_lsc2, outLaserR, outLaserG, outLaserB, outLaserStyle, outLaserSize);
			Color laserCol = Color(outLaserR, outLaserG, outLaserB);
			SetColorData(laserCol, outLaserStyle, outLaserSize);

			SetHSVMarkerPos();
			RecalculateHSVColor();
			UpdateHSVColor();
		}
	}
	else if (!Q_stricmp(command, "SetCustLaser3"))
	{
		if (m_bReplaceColor)
		{
			LaserHelper::SetLaserConvar(&cl_asw_archived_lsc3, m_currentLSColor.r, m_currentLSColor.g, m_currentLSColor.b, m_currentLaserStyle, m_currentLaserSize);
			m_bReplaceColor = false;
		}
		else
		{
			int outLaserR, outLaserG, outLaserB, outLaserStyle, outLaserSize;
			LaserHelper::SplitLaserConvar(&cl_asw_archived_lsc3, outLaserR, outLaserG, outLaserB, outLaserStyle, outLaserSize);
			Color laserCol = Color(outLaserR, outLaserG, outLaserB);
			SetColorData(laserCol, outLaserStyle, outLaserSize);

			SetHSVMarkerPos();
			RecalculateHSVColor();
			UpdateHSVColor();
		}
	}
	else if (!Q_stricmp(command, "SetCustLaser4"))
	{
		if (m_bReplaceColor)
		{
			LaserHelper::SetLaserConvar(&cl_asw_archived_lsc4, m_currentLSColor.r, m_currentLSColor.g, m_currentLSColor.b, m_currentLaserStyle, m_currentLaserSize);
			m_bReplaceColor = false;
		}
		else
		{
			int outLaserR, outLaserG, outLaserB, outLaserStyle, outLaserSize;
			LaserHelper::SplitLaserConvar(&cl_asw_archived_lsc4, outLaserR, outLaserG, outLaserB, outLaserStyle, outLaserSize);
			Color laserCol = Color(outLaserR, outLaserG, outLaserB);
			SetColorData(laserCol, outLaserStyle, outLaserSize);

			SetHSVMarkerPos();
			RecalculateHSVColor();
			UpdateHSVColor();
		}
	}
	else if (!Q_stricmp(command, "SetCustLaser5"))
	{
		if (m_bReplaceColor)
		{
			LaserHelper::SetLaserConvar(&cl_asw_archived_lsc5, m_currentLSColor.r, m_currentLSColor.g, m_currentLSColor.b, m_currentLaserStyle, m_currentLaserSize);
			m_bReplaceColor = false;
		}
		else
		{
			int outLaserR, outLaserG, outLaserB, outLaserStyle, outLaserSize;
			LaserHelper::SplitLaserConvar(&cl_asw_archived_lsc5, outLaserR, outLaserG, outLaserB, outLaserStyle, outLaserSize);
			Color laserCol = Color(outLaserR, outLaserG, outLaserB);
			SetColorData(laserCol, outLaserStyle, outLaserSize);

			SetHSVMarkerPos();
			RecalculateHSVColor();
			UpdateHSVColor();
		}
	}
	else if (!Q_stricmp(command, "SetCustLaser6"))
	{
	if (m_bReplaceColor)
	{
		LaserHelper::SetLaserConvar(&cl_asw_archived_lsc6, m_currentLSColor.r, m_currentLSColor.g, m_currentLSColor.b, m_currentLaserStyle, m_currentLaserSize);
		m_bReplaceColor = false;
	}
	else
	{
		int outLaserR, outLaserG, outLaserB, outLaserStyle, outLaserSize;
		LaserHelper::SplitLaserConvar(&cl_asw_archived_lsc6, outLaserR, outLaserG, outLaserB, outLaserStyle, outLaserSize);
		Color laserCol = Color(outLaserR, outLaserG, outLaserB);
		SetColorData(laserCol, outLaserStyle, outLaserSize);

		SetHSVMarkerPos();
		RecalculateHSVColor();
		UpdateHSVColor();
	}
	}
	else if (!Q_stricmp(command, "SetCustLaser7"))
	{
	if (m_bReplaceColor)
	{
		LaserHelper::SetLaserConvar(&cl_asw_archived_lsc7, m_currentLSColor.r, m_currentLSColor.g, m_currentLSColor.b, m_currentLaserStyle, m_currentLaserSize);
		m_bReplaceColor = false;
	}
	else
	{
		int outLaserR, outLaserG, outLaserB, outLaserStyle, outLaserSize;
		LaserHelper::SplitLaserConvar(&cl_asw_archived_lsc7, outLaserR, outLaserG, outLaserB, outLaserStyle, outLaserSize);
		Color laserCol = Color(outLaserR, outLaserG, outLaserB);
		SetColorData(laserCol, outLaserStyle, outLaserSize);

		SetHSVMarkerPos();
		RecalculateHSVColor();
		UpdateHSVColor();
	}
	}
	else if (!Q_stricmp(command, "SetCustLaser8"))
	{
	if (m_bReplaceColor)
	{
		LaserHelper::SetLaserConvar(&cl_asw_archived_lsc8, m_currentLSColor.r, m_currentLSColor.g, m_currentLSColor.b, m_currentLaserStyle, m_currentLaserSize);
		m_bReplaceColor = false;
	}
	else
	{
		int outLaserR, outLaserG, outLaserB, outLaserStyle, outLaserSize;
		LaserHelper::SplitLaserConvar(&cl_asw_archived_lsc8, outLaserR, outLaserG, outLaserB, outLaserStyle, outLaserSize);
		Color laserCol = Color(outLaserR, outLaserG, outLaserB);
		SetColorData(laserCol, outLaserStyle, outLaserSize);

		SetHSVMarkerPos();
		RecalculateHSVColor();
		UpdateHSVColor();
	}
	}
	else if (!Q_stricmp(command, "SetCustLaser9"))
	{
	if (m_bReplaceColor)
	{
		LaserHelper::SetLaserConvar(&cl_asw_archived_lsc9, m_currentLSColor.r, m_currentLSColor.g, m_currentLSColor.b, m_currentLaserStyle, m_currentLaserSize);
		m_bReplaceColor = false;
	}
	else
	{
		int outLaserR, outLaserG, outLaserB, outLaserStyle, outLaserSize;
		LaserHelper::SplitLaserConvar(&cl_asw_archived_lsc9, outLaserR, outLaserG, outLaserB, outLaserStyle, outLaserSize);
		Color laserCol = Color(outLaserR, outLaserG, outLaserB);
		SetColorData(laserCol, outLaserStyle, outLaserSize);

		SetHSVMarkerPos();
		RecalculateHSVColor();
		UpdateHSVColor();
	}
	}
	else if (!Q_stricmp(command, "ActivateReplaceColor"))
	{
		m_bReplaceColor = !m_bReplaceColor;
	}
}

void Cnb_lobby_laser_rgb_menu::OpenPlayerFlyout()
{

}

CNB_Main_Panel* Cnb_lobby_laser_rgb_menu::GetMainPanel()
{
	CNB_Main_Panel* pMainPanel = NULL;
	for (vgui::Panel* pPanel = GetParent(); pPanel; pPanel = pPanel->GetParent())
	{
		pMainPanel = dynamic_cast<CNB_Main_Panel*>(pPanel);
		if (pMainPanel)
		{
			return pMainPanel;
		}
	}

	Warning("Error, parent of lobby row is not the main panel\n");
	return NULL;
}

void Cnb_lobby_laser_rgb_menu::SetHSV_Hue(float outputHue)
{
	m_fHSV_Hue = outputHue;
}

void Cnb_lobby_laser_rgb_menu::SetHSV_Sat(float outputSat)
{
	m_fHSV_Sat = outputSat;
}

void Cnb_lobby_laser_rgb_menu::SetHSV_Val(float outputVal)
{
	m_fHSV_Val = outputVal;
}

void Cnb_lobby_laser_rgb_menu::RecalculateHSVColor()
{
	Vector vecHSV = Vector(m_fHSV_Hue, m_fHSV_Sat, m_fHSV_Val);
	Vector vecCol = Vector(0, 0, 0);
	HSVtoRGB(vecHSV, vecCol);
	m_currentLSColor.r = vecCol.x * 255.0f;
	m_currentLSColor.g = vecCol.y * 255.0f;
	m_currentLSColor.b = vecCol.z * 255.0f;

	SetHSVMarkerPos();
}

void Cnb_lobby_laser_rgb_menu::UpdateHSVColor()
{
	LaserHelper::SetLaserConvar(&cl_asw_laser_sight_color, m_currentLSColor.r, m_currentLSColor.g, m_currentLSColor.b, m_currentLaserStyle, m_currentLaserSize);
}

void Cnb_lobby_laser_rgb_menu::SetHSVMarkerPos()
{
	float baseMarkerWidth = 25.0f, baseMarkerHeight = 25.0f;
	//HSV SQUARE MARKER
	int widtha, heighta;
	m_pHSVSquare->GetSize(widtha, heighta);
	int xa, ya;
	m_pHSVSquare->GetPos(xa, ya);

	int widthd, heightd;

	m_pHSVSquareMarker->GetSize(widthd, heightd);

	float widthMod = widthd / baseMarkerWidth;
	float heightMod = heightd / baseMarkerHeight;

	int baseX = xa - (10.0f * widthMod), baseY = ya - (10.0f * heightMod);

	float horMod = m_fHSV_Hue / 360.0;
	float verMod = 1.0f-(m_fHSV_Sat / 1.0);

	float newX = baseX + (horMod * widtha);
	float newY = baseY + (verMod * heighta);


	m_pHSVSquareMarker->SetPos(newX, newY);

	m_pHSVSquareMarker->SetMouseInputEnabled(false);

	//HSV VAL SLIDER
	int xb, yb, widthb, heightb;
	m_pHSVSlider->GetPos(xb, yb);
	m_pHSVSlider->GetSize(widthb, heightb);

	baseX = xb - (11.0f * widthMod);
	baseY = yb - (10.0f * heightMod);

	verMod = 1.0f - (m_fHSV_Val / 1.0);

	newY = baseY + (verMod * heightb);

	newX = baseX + ((widthb != 0) ? widthb : 1)/2;


	m_pHSVSliderMarker->SetPos(newX, newY);

	m_pHSVSliderMarker->SetMouseInputEnabled(false);


}

void Cnb_lobby_laser_rgb_menu::OnMousePressed(vgui::MouseCode code)
{
}

void Cnb_lobby_laser_rgb_menu::OnCursorMoved(int x, int y)
{
	m_localMouseX = x;
	m_localMouseY = y;
}

void Cnb_lobby_laser_rgb_menu::SetChangingLaserColor(int laser)
{
	GetMainPanel()->SetChangingLaserColor(laser);
}
