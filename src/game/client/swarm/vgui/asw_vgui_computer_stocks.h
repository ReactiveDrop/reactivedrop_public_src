#ifndef _INCLUDED_ASW_VGUI_COMPUTER_STOCKS_H
#define _INCLUDED_ASW_VGUI_COMPUTER_STOCKS_H

#include <vgui_controls/Frame.h>
#include <vgui_controls/PropertyPage.h>
#include <vgui_controls/Slider.h>
#include <vgui/IScheme.h>
#include "vgui_controls/PanelListPanel.h"
#include "vgui_controls/ComboBox.h"
#include "vgui/IScheme.h"
#include "asw_vgui_ingame_panel.h"

class C_ASW_Hack_Computer;
class ImageButton;

#define ASW_STOCK_ENTRIES 16

// computer page showing randomly generated stock values

class CASW_VGUI_Computer_Stocks : public vgui::Panel, public CASW_VGUI_Ingame_Panel
{
	DECLARE_CLASS_SIMPLE( CASW_VGUI_Computer_Stocks, vgui::Panel );

	CASW_VGUI_Computer_Stocks( vgui::Panel *pParent, const char *pElementName, C_ASW_Hack_Computer *pHackDoor );
	virtual ~CASW_VGUI_Computer_Stocks();

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void OnThink(); // called every frame before painting, but only if panel is visible	
	virtual void PerformLayout();
	void ASWInit();
	void ApplySettingAndFadeLabelIn( vgui::Label *pLabel );
	virtual bool MouseClick( int x, int y, bool bRightClick, bool bDown );
	virtual void OnCommand( char const *command );
	void SetStockLabels();

	// current computer hack
	C_ASW_Hack_Computer *m_pHackComputer;
	ImageButton *m_pBackButton;
	vgui::Label *m_pTitleLabel;
	vgui::ImagePanel *m_pTitleIcon;
	vgui::ImagePanel *m_pTitleIconShadow;

	vgui::Label *m_pSymbol[ASW_STOCK_ENTRIES];
	vgui::Label *m_pCorp[ASW_STOCK_ENTRIES];
	vgui::Label *m_pValue[ASW_STOCK_ENTRIES];
	vgui::Label *m_pChange[ASW_STOCK_ENTRIES];
	vgui::Label *m_pVolume[ASW_STOCK_ENTRIES];

	bool m_bMouseOverBackButton;

	// overall scale of this window
	float m_fScale;
	float m_fLastThinkTime;
	bool m_bSetAlpha;
};

#endif /* _INCLUDED_ASW_VGUI_COMPUTER_STOCKS_H */
