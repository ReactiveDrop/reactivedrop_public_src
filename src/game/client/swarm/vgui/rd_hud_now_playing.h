#pragma once

#include <vgui_controls/EditablePanel.h>
#include "asw_hudelement.h"

class CRD_HUD_Now_Playing : public CASW_HudElement, public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CRD_HUD_Now_Playing, vgui::EditablePanel );
public:

	CRD_HUD_Now_Playing( const char *pElementName );

	void ShowAfterDelay( float flDelay );
	void HideEarly();
	void HideImmediately();
	void UpdateLabels();

	void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	void OnThink() override;

	vgui::Label *m_pLblTrackName;
	vgui::Label *m_pLblAlbumName;
	vgui::Label *m_pLblArtistName;

	float m_flFadeInStart{ -1 };
	float m_flFadeInEnd{ -1 };
	float m_flFadeOutStart{ -1 };
	float m_flFadeOutEnd{ -1 };
	float m_flUpdateLabelsAfter{ -1 };

	CPanelAnimationVar( float, m_flDelayTime, "delay_time", "0.0" );
	CPanelAnimationVar( float, m_flFadeInTime, "fade_in_time", "0.25" );
	CPanelAnimationVar( float, m_flSustainTime, "sustain_time", "5.0" );
	CPanelAnimationVar( float, m_flFadeOutTime, "fade_out_time", "0.75" );
};
