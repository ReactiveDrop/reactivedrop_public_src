#include "cbase.h"
#include "HSVSliderPanel.h"
#include "HSVColorSquarePanel.h"
#include "vgui/ISurface.h"
#include "vgui_controls/AnimationController.h"
#include "nb_lobby_laser_rgb_menu.h"
#include <controller_focus.h>
#include "vgui/IInput.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar cl_asw_laser_sight_color;

class Cnb_lobby_laser_rgb_menu;

CHSVSliderPanel::CHSVSliderPanel(vgui::Panel* parent, const char* name) : vgui::ImagePanel(parent, name)
{

}

CHSVSliderPanel::~CHSVSliderPanel()
{

}

void CHSVSliderPanel::OnThink()
{
	BaseClass::OnThink();

	CheckHSVUpdate();
}

void CHSVSliderPanel::CheckHSVUpdate()
{
	if (m_LeftMousePressed)
	{
		int width, height;
		GetSize(width, height);

		int worldCursorX, worldCursorY;
		vgui::input()->GetCursorPos(worldCursorX, worldCursorY);

		int worldPosX, worldPosY = 0;
		//GetWorldPos(worldPosX, worldPosY);
		LocalToScreen(worldPosX, worldPosY);

		m_localMouseX = MIN(MAX(0, worldCursorX - worldPosX), width);
		m_localMouseY = MIN(MAX(0, worldCursorY - worldPosY), height);

		if (vgui::input()->IsMouseDown(MOUSE_LEFT))
		{
			Cnb_lobby_laser_rgb_menu* parentRGB = dynamic_cast<Cnb_lobby_laser_rgb_menu*>(GetParent());

			if (!GetControllerFocus()->IsControllerMode() || GetControllerFocus()->GetFocusPanel() == NULL)
			{
				parentRGB->m_CurrentFocusMode = LaserRGBMenuFocusMode::HSVSlider;
				GetControllerFocus()->SetIsRawOverride(true);
			}

			if (parentRGB->m_CurrentFocusMode == LaserRGBMenuFocusMode::Main || parentRGB->m_CurrentFocusMode == LaserRGBMenuFocusMode::HSVSquare)
			{
				parentRGB->m_CurrentFocusMode = LaserRGBMenuFocusMode::HSVSlider;
			}
			else if (parentRGB->m_CurrentFocusMode == LaserRGBMenuFocusMode::HSVSlider)
			{
				int Xa, Ya;
				GetPos(Xa, Ya);
				float offsetY = m_localMouseY;

				float YMod = offsetY / height;

				float outputVal = 1.0f - YMod;

				Cnb_lobby_laser_rgb_menu* parentRGB = dynamic_cast<Cnb_lobby_laser_rgb_menu*>(GetParent());

				if (parentRGB)
				{
					parentRGB->SetHSV_Val(outputVal);
					parentRGB->RecalculateHSVColor();
					parentRGB->UpdateHSVColor();
				}
			}
		}
		else
		{
			m_LeftMousePressed = false;
		}
	}
}

void CHSVSliderPanel::OnMousePressed(vgui::MouseCode code)
{
	if (code == MOUSE_LEFT)
	{
		m_LeftMousePressed = true;
	}
}

void CHSVSliderPanel::OnMouseReleased(vgui::MouseCode code)
{
	if (code == MOUSE_LEFT)
	{
		m_LeftMousePressed = false;
	}
}

void CHSVSliderPanel::OnCursorMoved(int x, int y)
{
	m_localMouseX = x;
	m_localMouseY = y;
}

void CHSVSliderPanel::PerformLayout()
{
	/*
	// Get the screen size
	int wide, tall;
	vgui::surface()->GetScreenSize(wide, tall);

	float LogoWidth = wide * 0.8f;
	SetBounds((wide - LogoWidth) * 0.5f, tall * 0.12f, LogoWidth, LogoWidth * 0.25f);
	*/
}


extern ConVar developer;

