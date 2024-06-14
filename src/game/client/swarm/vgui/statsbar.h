#ifndef _INCLUDED_STATSBAR_H
#define _INCLUDED_STATSBAR_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui/VGUI.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/PHandle.h>

namespace vgui
{
	class Label;
};

// this class shows a number over a bar
//   both number and bar tick up from zero at the specified rate

class StatsBar : public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( StatsBar, vgui::Panel );
public:
	StatsBar( vgui::Panel *parent, const char *name );

	virtual void OnThink();
	virtual void PerformLayout();
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void Init( float fCurrent, float fTarget, float fIncreaseRate, bool bDisplayInteger, bool bDisplayPercentage );
	virtual void SetStartCountingTime( float fTime ) { m_fStartTime = fTime; }
	void AddMinMax( float flBarMin, float flBarMax );
	void ClearMinMax();
	void UseExternalCounter( vgui::Label *pCounter );
	void SetShowMaxOnCounter( bool bShowMax ) { m_bShowMaxOnCounter = bShowMax; }

	vgui::Label *m_pLabel;
	vgui::DHANDLE<vgui::Label> m_hExternalCounter;
	vgui::Panel *m_pBar;
	vgui::Panel *m_pIncreaseBar;
	vgui::Panel *m_pUnusedBar;
	CSoundPatch *m_pLoopingSound;

	// find the min/max which contains the current value
	float GetBarMin();
	float GetBarMax();

	bool IsDoneAnimating( void ) { return m_fCurrent == m_fTarget; }
	void SetUpdateInterval( float flUpdateInterval ) { m_flUpdateInterval = m_flNextUpdateTime; }

	bool m_bShowCumulativeTotal;

	float m_fStart;
	float m_fTarget;
	float m_fCurrent;
	float m_fIncreaseRate;
	bool m_bDisplayInteger;
	bool m_bDisplayPercentage;
	bool m_bDisplayTime;
	float m_flBorder;

	bool m_bInit;
	bool m_bShowMaxOnCounter;
	float m_fStartTime;
	float m_flNextUpdateTime;
	float m_flUpdateInterval;

	// colors
	CPanelAnimationVar( Color, m_TextColor, "text_color", "255 255 255 0" );
	CPanelAnimationVar( Color, m_BarColor, "bar_color", "93 148 192 255" );
	CPanelAnimationVar( Color, m_IncreaseBarColor, "increase_color", "255 255 255 255" );
	CPanelAnimationVar( Color, m_UnusedColor, "unused_color", "17 37 57 255" );

	CPanelAnimationVar( vgui::HFont, m_hLabelFont, "label_font", "Default" );

	// list of future bar max's for looping bars (e.g. XP bar)
	struct StatsBarMinMax
	{
		float flMin;
		float flMax;
	};
	CUtlVector<StatsBarMinMax> m_Bounds;
};

#endif // _INCLUDED_STATSBAR_H