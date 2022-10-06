#include "cbase.h"
#include "HSVColorSquarePanel.h"
#include "vgui/ISurface.h"
#include "vgui_controls/AnimationController.h"
#include "nb_lobby_laser_rgb_menu.h"
#include <controller_focus.h>

extern ConVar cl_asw_laser_sight_color;

class Cnb_lobby_laser_rgb_menu;

CHSVColorSquarePanel::CHSVColorSquarePanel(vgui::Panel* parent, const char* name) : vgui::ImagePanel(parent, name)
{

}

CHSVColorSquarePanel::~CHSVColorSquarePanel()
{

}

void CHSVColorSquarePanel::OnMousePressed(vgui::MouseCode code)
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
		if (code == MOUSE_LEFT)
		{
			int width, height;
			GetSize(width, height);

			int Xa, Ya;
			GetPos(Xa, Ya);
			float offsetX = m_localMouseX; //No offset needed for this calculation
			float offsetY = m_localMouseY;

			float XMod = offsetX / width;
			float YMod = offsetY / height;

			float outputHue = XMod * 360.0f;
			float outputSat = 1.0f - YMod; // *255.0f;

			/*
			float outputVal = 1.0f; //255.0f;

			Vector hsvVec = Vector(outputHue, outputSat, outputVal);
			Vector colVec = Vector(0, 0, 0);

			HSVtoRGB(hsvVec, colVec);
			*/



			if (parentRGB)
			{
				parentRGB->SetHSV_Hue(outputHue);
				parentRGB->SetHSV_Sat(outputSat);
				parentRGB->RecalculateHSVColor();

				parentRGB->UpdateHSVColor();
			}

			/*
			char sLaserColor[16]{};
			snprintf(sLaserColor, sizeof(sLaserColor), "%d %d %d", (int)(colVec.x*255.0f), (int)(colVec.y * 255.0f), (int)(colVec.z * 255.0f));
			cl_asw_laser_sight_color.SetValue(sLaserColor);
			*/
		}
	}
}

void CHSVColorSquarePanel::OnCursorMoved(int x, int y)
{
	m_localMouseX = x;
	m_localMouseY = y;
}

void CHSVColorSquarePanel::OnKeyCodeTyped(vgui::KeyCode code)
{
	/*
	// force ourselves to be closed if the escape key it pressed
	if (code == KEY_ESCAPE)
	{
		Close();
	}
	else
	{
		BaseClass::OnKeyCodeTyped(code);
	}
	*/



	BaseClass::OnKeyCodeTyped(code);
}

void CHSVColorSquarePanel::OnKeyCodePressed(vgui::KeyCode code)
{
	GetParent()->OnKeyCodePressed(code);
}

void CHSVColorSquarePanel::PerformLayout()
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

