#ifndef _INCLUDED_BRIEFINGTOOLIP_H
#define _INCLUDED_BRIEFINGTOOLIP_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui/VGUI.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/PHandle.h>

namespace vgui
{
	class MultiFontRichText;
}

// this is the tooltip panel used throughout the briefing
class BriefingTooltip : public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( BriefingTooltip, vgui::Panel );
public:
	BriefingTooltip( Panel *parent, const char *panelName );
	virtual ~BriefingTooltip();

	void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void OnThink(); // called every frame before painting, but only if panel is visible

	vgui::Panel *GetTooltipPanel() { return m_pTooltipPanel; }
	void SetTooltip( vgui::Panel *pPanel, const char *szMainText, const char *szSubText,
		int iTooltipX, int iTooltipY, vgui::Label::Alignment iAlignment = vgui::Label::a_north, bool bZbalermorna = false );
	void SetTooltip( vgui::Panel *pPanel, const wchar_t *szMainText, const wchar_t *szSubText,
		int iTooltipX, int iTooltipY, vgui::Label::Alignment iAlignment = vgui::Label::a_north );

	virtual void PerformLayout();
	vgui::Panel *m_pTooltipPanel;
	vgui::MultiFontRichText *m_pMainLabel;
	vgui::MultiFontRichText *m_pSubLabel;
	vgui::HFont m_MainFont, m_SubFont;
	int m_iTooltipX, m_iTooltipY;
	vgui::Label::Alignment m_iTooltipAlignment;

	// allow disabling of tooltips
	void SetTooltipsEnabled( bool bEnabled ) { m_bTooltipsEnabled = bEnabled; }
	bool m_bTooltipsEnabled;
	void SetTooltipIgnoresCursor( bool bIgnore ) { m_bTooltipIgnoresCursor = bIgnore; }
	bool m_bTooltipIgnoresCursor;

	static void EnsureParent( Panel *parent );
	static void Free();
};

extern vgui::DHANDLE<BriefingTooltip> g_hBriefingTooltip;

#endif // _INCLUDED_BRIEFINGTOOLIP_H
