#ifndef _INCLUDED_NB_BUTTON_H
#define _INCLUDED_NB_BUTTON_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui/VGUI.h>
#include <vgui_controls/Button.h>

// == MANAGED_CLASS_DECLARATIONS_START: Do not edit by hand ==
// == MANAGED_CLASS_DECLARATIONS_END ==

class CNB_Button : public vgui::Button
{
	DECLARE_CLASS_SIMPLE( CNB_Button, vgui::Button );
public:
	CNB_Button( Panel *parent, const char *panelName, const char *text, Panel *pActionSignalTarget = NULL, const char *pCmd = NULL, bool bSuppressAddToFocusList = false );
	CNB_Button( Panel *parent, const char *panelName, const wchar_t *text, Panel *pActionSignalTarget = NULL, const char *pCmd = NULL, bool bSuppressAddToFocusList = false );
	virtual ~CNB_Button();
	
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void Paint();
	virtual void PaintBackground();
	virtual void OnCursorEntered();
	virtual void NavigateTo();

	void DrawRoundedBox( int x, int y, int wide, int tall, Color color, Color highlightCenterColor );
	void SetControllerButton( vgui::KeyCode code );

	// == MANAGED_MEMBER_POINTERS_START: Do not edit by hand ==
	// == MANAGED_MEMBER_POINTERS_END ==

	vgui::HFont m_hButtonFont;
	const char *m_szControllerButton;
	bool m_bAddedToControllerFocus;

	CPanelAnimationVar( bool, m_bAutoFocus, "autoFocus", "0" );
	CPanelAnimationVarAliasType( int, m_nNBBgTextureId1, "NBTexture1", "vgui/hud/800corner1", "textureid" );
	CPanelAnimationVarAliasType( int, m_nNBBgTextureId2, "NBTexture2", "vgui/hud/800corner2", "textureid" );
	CPanelAnimationVarAliasType( int, m_nNBBgTextureId3, "NBTexture3", "vgui/hud/800corner3", "textureid" );
	CPanelAnimationVarAliasType( int, m_nNBBgTextureId4, "NBTexture4", "vgui/hud/800corner4", "textureid" );
	CPanelAnimationVar( int, m_nBorderThickMin, "border_thickness_min", "1" );
	CPanelAnimationVarAliasType( int, m_nBorderThick, "border_thickness", "1", "proportional_int" );
	CPanelAnimationVar( Color, m_BorderColor, "border_color", "78 94 110 255" );
	CPanelAnimationVar( Color, m_PressedColor, "pressed_color", "20 59 96 255" );
	CPanelAnimationVar( Color, m_PressedHighlightColor, "pressed_highlight_color", "28 80 130 255" );
	CPanelAnimationVar( Color, m_EnabledColor, "enabled_color", "24 43 66 255" );
	CPanelAnimationVar( Color, m_EnabledHighlightColor, "enabled_highlight_color", "0 0 0 0" );
	CPanelAnimationVar( Color, m_DisabledColor, "disabled_color", "65 78 91 255" );
	CPanelAnimationVar( Color, m_DisabledHighlightColor, "disabled_highlight_color", "0 0 0 0" );
};

#endif // _INCLUDED_NB_BUTTON_H
