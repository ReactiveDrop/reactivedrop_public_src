#pragma once

#include <vgui_controls/EditablePanel.h>

class CRD_VGUI_Stock_Ticker : public vgui::EditablePanel
{
public:
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Stock_Ticker, vgui::EditablePanel );

	CRD_VGUI_Stock_Ticker( vgui::Panel *parent, const char *panelName );
	~CRD_VGUI_Stock_Ticker();

	void PerformLayout() override;
	void OnThink() override;
	void Paint() override;
	void ManageTextBuffer();
	void GenerateNextTickerText( wchar_t * &wszText, int &iIconTexture );
	void GenerateTickerText( KeyValues *pDef, wchar_t *&wszText, int &iIconTexture );

	bool m_bLastReduceMotion;
	float m_flLastThink;
	int m_iTitleX;
	int m_iTextStartX;
	int m_iBackgroundStartX;
	KeyValues::AutoDelete m_pKVTickerDefs;
	CUtlQueue<KeyValues *> m_TickerDefCooldown;
	float m_flTotalWeight;
	CUtlDict<int> m_TickerDefTextures;
	wchar_t m_wszTitle[128];
	CUtlQueue<wchar_t *> m_TextBuffer;
	CUtlQueue<int> m_IconBuffer;
	int m_iFirstTextWidth;
	int m_iTextTotalWidth;
	CUniformRandomStream m_RandomStream;
	int m_iLastRandomSeed;

	CPanelAnimationVar( vgui::HFont, m_hTickerFont, "ticker_font", "DefaultSmall" );
	CPanelAnimationVar( vgui::HFont, m_hTickerBlurFont, "ticker_blur_font", "DefaultSmallBlur" );
	CPanelAnimationVarAliasType( int, m_iTextStartXOffset, "text_start_xpos", "0", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iTextY, "text_ypos", "0", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iTitlePadding, "title_padding", "5", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iTitleAfterWidth, "title_after_wide", "10", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iIconXPos, "icon_xpos", "0", "proportional_int");
	CPanelAnimationVarAliasType( int, m_iIconSize, "icon_size", "10", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iSeparatorWidth, "separator_wide", "10", "proportional_int" );
	CPanelAnimationVarAliasType( float, m_flTickerSpeed, "ticker_speed", "10", "proportional_float" );
	CPanelAnimationVarAliasType( int, m_iBackgroundTexture, "background_tex", "vgui/white", "textureid" );
	CPanelAnimationVarAliasType( int, m_iTitleBeforeTexture, "title_before_tex", "vgui/white", "textureid" );
	CPanelAnimationVarAliasType( int, m_iTitleBackgroundTexture, "title_background_tex", "vgui/white", "textureid" );
	CPanelAnimationVarAliasType( int, m_iTitleAfterTexture, "title_after_tex", "vgui/white", "textureid" );
	CPanelAnimationVarAliasType( int, m_iSeparatorTexture, "separator_tex", "vgui/white", "textureid" );
	CPanelAnimationVarAliasType( int, m_iStockUpTexture, "stock_up_tex", "vgui/white", "textureid" );
	CPanelAnimationVarAliasType( int, m_iStockDownTexture, "stock_down_tex", "vgui/white", "textureid" );
};
