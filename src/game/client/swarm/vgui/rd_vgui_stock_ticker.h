#pragma once

#include <vgui_controls/EditablePanel.h>

class CRD_VGUI_Stock_Ticker : public vgui::EditablePanel
{
public:
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Stock_Ticker, vgui::EditablePanel );

	CRD_VGUI_Stock_Ticker( vgui::Panel *parent, const char *panelName );
	~CRD_VGUI_Stock_Ticker();

	void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	void PerformLayout() override;
	void OnThink() override;
	void Paint() override;
	void ManageTextBuffer();
	void GenerateNextTickerText( wchar_t * &wszText, int &iIconTexture );
	void GenerateTickerText( KeyValues *pDef, wchar_t *&wszText, int &iIconTexture );

	float m_flLastThink;
	int m_iTitleX;
	int m_iTextStartX;
	KeyValues::AutoDelete m_pKVTickerDefs;
	float m_flTotalWeight;
	CUtlDict<int> m_TickerDefTextures;
	wchar_t m_wszTitle[128];
	CUtlQueue<wchar_t *> m_TextBuffer;
	CUtlQueue<int> m_IconBuffer;
	int m_iFirstTextWidth;
	int m_iTextTotalWidth;
	vgui::HFont m_hTickerFont;
	vgui::HFont m_hTickerBlurFont;
	CUniformRandomStream m_RandomStream;
	int m_iLastRandomSeed;

	CPanelAnimationVarAliasType( int, m_iTextY, "text_ypos", "0", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iTitlePadding, "title_padding", "5", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iTitleAfterWidth, "title_after_wide", "10", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iIconSize, "icon_size", "10", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iSeparatorWidth, "separator_wide", "10", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iBackgroundTexture, "background_tex", "vgui/white", "textureid" );
	CPanelAnimationVarAliasType( int, m_iTitleBeforeTexture, "title_before_tex", "vgui/white", "textureid" );
	CPanelAnimationVarAliasType( int, m_iTitleBackgroundTexture, "title_background_tex", "vgui/white", "textureid" );
	CPanelAnimationVarAliasType( int, m_iTitleAfterTexture, "title_after_tex", "vgui/white", "textureid" );
	CPanelAnimationVarAliasType( int, m_iSeparatorTexture, "separator_tex", "vgui/white", "textureid" );
	CPanelAnimationVarAliasType( int, m_iStockUpTexture, "stock_down_tex", "vgui/white", "textureid" );
	CPanelAnimationVarAliasType( int, m_iStockDownTexture, "stock_up_tex", "vgui/white", "textureid" );
};
