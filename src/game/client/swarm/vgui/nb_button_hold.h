#pragma once

#include "nb_button.h"

class CNB_Button_Hold : public CNB_Button
{
	DECLARE_CLASS_SIMPLE( CNB_Button_Hold, CNB_Button );
public:
	CNB_Button_Hold( vgui::Panel *parent, const char *panelName, const char *text, vgui::Panel *pActionSignalTarget = NULL, const char *pCmd = NULL, bool bSuppressAddToFocusList = false );
	CNB_Button_Hold( vgui::Panel *parent, const char *panelName, const wchar_t *text, vgui::Panel *pActionSignalTarget = NULL, const char *pCmd = NULL, bool bSuppressAddToFocusList = false );

	void PaintBackground() override;
	void PaintTraverse( bool Repaint, bool allowForce = true ) override;
	void DoClick() override;
	void OnThink() override;

	float m_flHoldProgress{ -1.0f };
	double m_flLastThink{ Plat_FloatTime() };
	bool m_bClickReset{ false };

	CPanelAnimationVar( float, m_flHoldTime, "hold_time", "2.30410" );
	CPanelAnimationVar( float, m_flResetTime, "reset_time", "0.548430" );
	CPanelAnimationVar( float, m_flShakeSpeedX, "shake_speed_x", "83.0" );
	CPanelAnimationVar( float, m_flShakeSpeedY, "shake_speed_y", "172.0" );
	CPanelAnimationVar( float, m_flShakeSpeedBiasX, "shake_speed_bias_x", "0.7" );
	CPanelAnimationVar( float, m_flShakeSpeedBiasY, "shake_speed_bias_y", "0.7" );
	CPanelAnimationVarAliasType( float, m_flShakeAmountX, "shake_amount_x", "2.5", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flShakeAmountY, "shake_amount_y", "2.0", "proportional_float" );
	CPanelAnimationVar( float, m_flShakeAmountBiasX, "shake_amount_bias_x", "0.5" );
	CPanelAnimationVar( float, m_flShakeAmountBiasY, "shake_amount_bias_y", "0.5" );
	CPanelAnimationVar( float, m_flHoldHighlightLip, "hold_highlight_lip", "0.1" );
	CPanelAnimationVar( Color, m_HoldHighlightColorStart, "hold_highlight_color_start", "40 80 130 192" );
	CPanelAnimationVar( Color, m_HoldHighlightColorFlash1, "hold_highlight_color_flash1", "254 100 1 192" );
	CPanelAnimationVar( Color, m_HoldHighlightColorFlash2, "hold_highlight_color_flash2", "255 196 0 192" );
	CPanelAnimationVar( float, m_flHoldHighlightColorBias, "hold_highlight_color_bias", "0.2" );
	CPanelAnimationVar( float, m_flHoldHighlightColorSpeed, "hold_highlight_color_speed", "25" );
};
