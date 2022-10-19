#include "cbase.h"
#include "HSVColorSquarePanel.h"
#include "vgui/ISurface.h"
#include "vgui_controls/AnimationController.h"
#include "nb_lobby_laser_rgb_menu.h"
#include <controller_focus.h>
#include "vgui/IInput.h"

extern ConVar cl_asw_laser_sight_color;

class Cnb_lobby_laser_rgb_menu;

CHSVColorSquarePanel::CHSVColorSquarePanel(vgui::Panel* parent, const char* name) : vgui::ImagePanel(parent, name)
{

}

CHSVColorSquarePanel::~CHSVColorSquarePanel()
{

}

void CHSVColorSquarePanel::OnThink()
{
	BaseClass::OnThink();

	CheckHSVUpdate();
}

void CHSVColorSquarePanel::CheckHSVUpdate()
{
	if (m_LeftMousePressed)
	{
		int width, height;
		GetSize(width, height);

		int worldCursorX, worldCursorY;
		vgui::input()->GetCursorPos(worldCursorX, worldCursorY);

		int worldPosX = 0;
		int worldPosY = 0;

		LocalToScreen(worldPosX, worldPosY);

		m_localMouseX = MIN(MAX(0, worldCursorX - worldPosX), width);
		m_localMouseY = MIN(MAX(0, worldCursorY - worldPosY), height);


		if (vgui::input()->IsMouseDown(MOUSE_LEFT))
		{
			Cnb_lobby_laser_rgb_menu* parentRGB = dynamic_cast<Cnb_lobby_laser_rgb_menu*>(GetParent());

			if (!GetControllerFocus()->IsControllerMode() || GetControllerFocus()->GetFocusPanel() == NULL)
			{
				parentRGB->m_CurrentFocusMode = LaserRGBMenuFocusMode::HSVSquare;
				GetControllerFocus()->SetIsRawOverride(true);
			}

			if (parentRGB->m_CurrentFocusMode == LaserRGBMenuFocusMode::Main || parentRGB->m_CurrentFocusMode == LaserRGBMenuFocusMode::HSVSlider)
			{
				parentRGB->m_CurrentFocusMode = LaserRGBMenuFocusMode::HSVSquare;
			}
			else if (parentRGB->m_CurrentFocusMode == LaserRGBMenuFocusMode::HSVSquare)
			{
				int Xa, Ya;
				GetPos(Xa, Ya);
				float offsetX = m_localMouseX; //No offset needed for this calculation
				float offsetY = m_localMouseY;

				float XMod = offsetX / width;
				float YMod = offsetY / height;

				float outputHue = XMod * 360.0f;
				float outputSat = 1.0f - YMod; // *255.0f;



				if (parentRGB)
				{
					parentRGB->SetHSV_Hue(outputHue);
					parentRGB->SetHSV_Sat(outputSat);
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

void CHSVColorSquarePanel::OnMousePressed(vgui::MouseCode code)
{
	if (code == MOUSE_LEFT)
	{
		m_LeftMousePressed = true;
	}
}

void CHSVColorSquarePanel::OnMouseReleased(vgui::MouseCode code)
{
	if (code == MOUSE_LEFT)
	{
		m_LeftMousePressed = false;
	}
}

void CHSVColorSquarePanel::OnCursorMoved(int x, int y)
{
	m_localMouseX = x;
	m_localMouseY = y;
}

void CHSVColorSquarePanel::OnKeyCodeTyped(vgui::KeyCode code)
{
	BaseClass::OnKeyCodeTyped(code);
}

void CHSVColorSquarePanel::OnKeyCodePressed(vgui::KeyCode code)
{
	GetParent()->OnKeyCodePressed(code);
}

void CHSVColorSquarePanel::PerformLayout()
{
}


extern ConVar developer;

