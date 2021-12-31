#pragma once

#include "asw_hudelement.h"
#include "stringpool.h"
#include <vgui/VGUI.h>
#include <vgui_controls/Panel.h>

class C_RD_Boss_Bar;
class CRD_Hud_Boss_Bar_Container;

class CRD_Hud_Boss_Bars : public CASW_HudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CRD_Hud_Boss_Bars, vgui::Panel );
public:
	CStringPool m_IdentifierPool;
	vgui::Dar<vgui::DHANDLE<CRD_Hud_Boss_Bar_Container>> m_Containers;

	CRD_Hud_Boss_Bars( const char *pElementName );
	virtual ~CRD_Hud_Boss_Bars();

	virtual void LevelInit();
	virtual void LevelShutdown();

	virtual void PerformLayout();

	void ClearAllBossBars();

	void OnBossBarEntityChanged( C_RD_Boss_Bar *pBar, bool bCreated );

	CRD_Hud_Boss_Bar_Container *FindBarContainer( const char *pszIdentifier, bool bAllowCreate );
};

class CRD_Hud_Boss_Bar_Container : public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CRD_Hud_Boss_Bar_Container, vgui::Panel );
public:
	struct Layout_t
	{
		int m_iColumnIndex;
		int m_iColumnCount;
		float m_flRowTop;
		float m_flRowHeight;
	};

	// string is pooled in CRD_Hud_Boss_Bars and may be invalid if the panel is marked for deletion
	const char *m_pIdentifier;
	vgui::HFont m_hFont;
	vgui::Label *m_pLabel;
	vgui::Dar<CHandle<C_RD_Boss_Bar>> m_BarEntities;
	vgui::Dar<Layout_t> m_BarLayout;
	float m_flBarAreaHeight;

	CRD_Hud_Boss_Bar_Container( CRD_Hud_Boss_Bars *pParent, const char *pIdentifier );
	virtual ~CRD_Hud_Boss_Bar_Container();

	void OnBarDataChanged();
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void Paint();
	virtual void PerformLayout();
};
