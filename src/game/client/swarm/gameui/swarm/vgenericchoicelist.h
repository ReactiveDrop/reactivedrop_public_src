#pragma once

#include "basemodframe.h"
#include "vhybridbutton.h"

class CNB_Header_Footer;

namespace BaseModUI
{
	typedef void ( *ChoiceCallback_t )( KeyValues *pAction );

	class ChoiceButton;

	class GenericChoiceList : public CBaseModFrame
	{
		DECLARE_CLASS_SIMPLE( GenericChoiceList, CBaseModFrame );
	public:
		GenericChoiceList( vgui::Panel *parent, const char *panelName );
		~GenericChoiceList();

		void SetDataSettings( KeyValues *pSettings ) override;
		void PerformLayout() override;
		void OnKeyCodePressed( vgui::KeyCode code ) override;

		CNB_Header_Footer *m_pHeaderFooter;
		KeyValues::AutoDelete m_pSettings{ ( KeyValues * )NULL };
		CUtlVector<ChoiceButton *> m_Buttons;

		CPanelAnimationVar( int, m_iBackgroundStyle, "background_style", "0" );
		CPanelAnimationVarAliasType( int, m_iButtonWide, "button_wide", "30", "proportional_int" );
		CPanelAnimationVarAliasType( int, m_iButtonTall, "button_tall", "10", "proportional_int" );
		CPanelAnimationVarAliasType( int, m_iButtonSpacing, "button_spacing", "0", "proportional_int" );
		CPanelAnimationVarAliasType( int, m_iPaddingX, "padding_x", "0", "proportional_int" );
		CPanelAnimationVarAliasType( int, m_iPaddingY, "padding_y", "0", "proportional_int" );
		CPanelAnimationVar( Color, m_InnerColor, "inner_color", "53 86 117 255" );
	};

	class ChoiceButton : public BaseModHybridButton
	{
		DECLARE_CLASS_SIMPLE( ChoiceButton, BaseModHybridButton );
	public:
		ChoiceButton( vgui::Panel *parent, const char *panelName, const wchar_t *wszLabel, ChoiceCallback_t callback = NULL, KeyValues *pAction = NULL );
		~ChoiceButton();

		void NavigateTo() override;
		void OnCommand( const char *command ) override;

		ChoiceCallback_t m_callback;
		KeyValues *m_pAction;
	};
}
