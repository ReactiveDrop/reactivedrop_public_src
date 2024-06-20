//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//

#include "cbase.h"
#include "VHybridButton.h"
#include "basemodpanel.h"
#include "VFooterPanel.h"
#include "VFlyoutMenu.h"
#include "EngineInterface.h"
#include "vgui/ISurface.h"
#include "vgui_controls/TextImage.h"
#include "vgui_controls/Tooltip.h"
#include "vgui/IVgui.h"
#include "tier1/KeyValues.h"
#include "vgui/ilocalize.h"
#include "VDropDownMenu.h"
#include "VSliderControl.h"
#include "gamemodes.h"

#ifndef _X360
#include <ctype.h>
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace BaseModUI;
using namespace vgui;

#ifdef INFESTED_DLL
extern ConVar rd_reduce_motion;
#endif

DECLARE_BUILD_FACTORY_DEFAULT_TEXT( BaseModHybridButton, HybridButton );

void Demo_DisableButton( Button *pButton )
{
	BaseModHybridButton *pHybridButton = dynamic_cast<BaseModHybridButton *>(pButton);

	if (pHybridButton)
	{
		pHybridButton->SetEnabled( false );

		char szTooltip[512];
		wchar_t *wUnicode = g_pVGuiLocalize->Find( "#L4D360UI_MainMenu_DemoVersion" );
		if ( !wUnicode )
			wUnicode = L"";

		g_pVGuiLocalize->ConvertUnicodeToANSI( wUnicode, szTooltip, sizeof( szTooltip ) );

		pHybridButton->SetHelpText( szTooltip , false );
	}
}

void Dlc1_DisableButton( Button *pButton )
{
	BaseModHybridButton *pHybridButton = dynamic_cast<BaseModHybridButton *>(pButton);

	if(pHybridButton)
	{
		pHybridButton->SetEnabled( false );

		char szTooltip[512];
		wchar_t *wUnicode = g_pVGuiLocalize->Find( "#L4D360UI_DLC1_NotInstalled" );

		if ( !wUnicode )
			wUnicode = L"";

		g_pVGuiLocalize->ConvertUnicodeToANSI( wUnicode, szTooltip, sizeof( szTooltip ) );

		pHybridButton->SetHelpText( szTooltip , false );
	}
}

struct HybridEnableStates
{
	BaseModUI::BaseModHybridButton::EnableCondition mCondition;
	char mConditionName[64];
};

HybridEnableStates sHybridStates[] = 
{
	{ BaseModUI::BaseModHybridButton::EC_LIVE_REQUIRED,	"LiveRequired" },
	{ BaseModUI::BaseModHybridButton::EC_NOTFORDEMO,		"Never" }
};

//=============================================================================
// Constructor / Destructor
//=============================================================================

BaseModUI::BaseModHybridButton::BaseModHybridButton( Panel *parent, const char *panelName, const char *text, Panel *pActionSignalTarget, const char *pCmd )
	: BaseClass( parent, panelName, text, pActionSignalTarget, pCmd )
{
	SetPaintBorderEnabled( false );
	SetPaintBackgroundEnabled( false );
	SetContentAlignment( a_northwest );
	SetClosed();
	SetButtonActivationType( ACTIVATE_ONRELEASED );
	SetConsoleStylePanel( true );

	m_isNavigateTo = false;
	m_bOnlyActiveUser = false;
	m_bIgnoreButtonA = false;

	mEnableCondition = EC_ALWAYS;

	m_nStyle = BUTTON_SIMPLE;
	m_bSetStyle = false;
	m_hTextFont = 0;
	m_hTextBlurFont = 0;
	m_hHintTextFont = 0;
	m_hSelectionFont = 0;
	m_hSelectionBlurFont = 0;

	m_originalTall = 0;
	m_textInsetX = 0;
	m_textInsetY = 0;

	m_iSelectedArrow = -1;
	m_iUnselectedArrow = -1;

	m_nWideAtOpen = 0;
}

BaseModUI::BaseModHybridButton::BaseModHybridButton( Panel *parent, const char *panelName, const wchar_t *text, Panel *pActionSignalTarget, const char *pCmd )
	: BaseClass( parent, panelName, text, pActionSignalTarget, pCmd )
{
	SetPaintBorderEnabled( false );
	SetPaintBackgroundEnabled( false );
	SetContentAlignment( a_northwest );
	SetClosed();
	SetButtonActivationType( ACTIVATE_ONRELEASED );

	m_isNavigateTo = false;
	m_iUsablePlayerIndex = -1;

	mEnableCondition = EC_ALWAYS;

	m_nStyle = BUTTON_SIMPLE;
	m_bSetStyle = false;
	m_hTextFont = 0;
	m_hTextBlurFont = 0;
	m_hHintTextFont = 0;
	m_hSelectionFont = 0;
	m_hSelectionBlurFont = 0;

	m_originalTall = 0;
	m_textInsetX = 0;
	m_textInsetY = 0;

	m_iSelectedArrow = -1;
	m_iUnselectedArrow = -1;
}

BaseModUI::BaseModHybridButton::~BaseModHybridButton()
{
	// purposely not destroying our textures
	// otherwise i/o constantly on these
}

BaseModHybridButton::State BaseModHybridButton::GetCurrentState( bool bIgnoreOpen )
{
	State curState = Enabled;
	if ( IsPC() )
	{
		if ( HasFocus() )
		{
			curState = IsEnabled() ? Focus : FocusDisabled;
		}
	}
	if ( !bIgnoreOpen && m_isOpen )
	{
		curState = Open;
	}
	else if ( IsArmed() || m_isNavigateTo )	//NavigateTo doesn't instantly give focus to the control
	{										//so this little boolean value is meant to let us know we should have focus for the "focus state"
		if ( IsEnabled() )					//until vgui catches up and registers us as having focus.
		{
			curState = Focus;
		}
		else
		{
			curState = FocusDisabled;
		}

		if ( IsArmed() )
		{
			m_isNavigateTo = false;
		}
	}
	else if ( !IsEnabled() )
	{
		curState = Disabled;
	}

	return curState;
}

void BaseModHybridButton::SetStyle( ButtonStyle_t nStyle )
{
	m_bSetStyle = true;
	m_nStyle = nStyle;
	ApplyStyle();
}

void BaseModHybridButton::SetOpen()
{
	if ( m_isOpen )
		return;
	m_isOpen = true;
	if ( IsPC() )
	{
		PostMessageToAllSiblingsOfType< BaseModHybridButton >( new KeyValues( "OnSiblingHybridButtonOpened" ) );
	}
}

void BaseModHybridButton::SetClosed()
{
	if ( !m_isOpen )
		return;

	m_isOpen = false;
}

void BaseModHybridButton::OnSiblingHybridButtonOpened()
{
	if ( !IsPC() )
		return;

	bool bClosed = false;

	FlyoutMenu *pActiveFlyout = FlyoutMenu::GetActiveMenu();

	if ( pActiveFlyout )
	{
		BaseModHybridButton *button = dynamic_cast< BaseModHybridButton* >( pActiveFlyout->GetNavFrom() );
		if ( button && button == this )
		{
			// We need to close the flyout attached to this button
			FlyoutMenu::CloseActiveMenu();
			bClosed = true;
		}
	}

	if ( !bClosed )
	{
		SetClosed();
	}

	m_isNavigateTo = false;
}

char const *BaseModHybridButton::GetHelpText( bool bEnabled ) const
{
	return bEnabled ? m_enabledToolText.String() : m_disabledToolText.String();
}

void BaseModHybridButton::UpdateFooterHelpText()
{
	CBaseModFooterPanel *footer = BaseModUI::CBaseModPanel::GetSingleton().GetFooterPanel();
	if ( footer )
	{
		//Directly set the new tooltips
		footer->SetHelpText( IsEnabled() ? m_enabledToolText.String() : m_disabledToolText.String() );
	}
}

void BaseModHybridButton::OnMousePressed( vgui::MouseCode code )
{
	BaseClass::OnMousePressed( code );
	if ( IsPC() )
	{
		if( code == MOUSE_RIGHT )
		{
			FlyoutMenu::CloseActiveMenu( this );
		}
		else
		{
			if( (code == MOUSE_LEFT) && (IsEnabled() == false) && (dynamic_cast<FlyoutMenu *>( GetParent() ) == NULL) )
			{
				//when trying to use an inactive item that isn't part of a flyout. Close any open flyouts.
				FlyoutMenu::CloseActiveMenu( this );
			}
			RequestFocus( 0 );			
		}
	}
}

void BaseModHybridButton::NavigateTo( )
{
	BaseClass::NavigateTo( );

	UpdateFooterHelpText();

	FlyoutMenu* parentMenu = dynamic_cast< FlyoutMenu* >( GetParent() );
	if( parentMenu )
	{
		parentMenu->NotifyChildFocus( this );
	}

	if (GetVParent())
	{
		KeyValues *msg = new KeyValues("OnHybridButtonNavigatedTo");
		msg->SetInt("button", ToHandle() );

		ivgui()->PostMessage(GetVParent(), msg, GetVPanel());
	}

	m_isNavigateTo = true;
	if ( IsPC() )
	{
		RequestFocus( 0 );
	}
}

void BaseModHybridButton::NavigateFrom( )
{
	BaseClass::NavigateFrom( );

	if ( IsPC() && CBaseModPanel::GetSingleton().GetFooterPanel() )
	{
		// Show no help text if they left the button
		CBaseModPanel::GetSingleton().GetFooterPanel()->FadeHelpText();
	}

	m_isNavigateTo = false;
}

void BaseModHybridButton::SetHelpText( const char* tooltip, bool enabled )
{
	if ( enabled )
	{
		m_enabledToolText = tooltip;
	}
	else
	{
		m_disabledToolText = tooltip;
	}

	// if we have the focus update the footer
	if ( HasFocus() )
	{
		UpdateFooterHelpText();
	}
}

// 0 = Deprecated CA buttons
// 1 = Normal Button
// 2 = MainMenu Button or InGameMenu Butto
// 3 = Flyout button
void BaseModHybridButton::PaintButtonEx()
{
	int x, y;
	int wide, tall;
	GetSize( wide, tall );

	x = 0;

	// due to vertical resizing, center within the control
	y = ( tall - m_originalTall ) / 2;
	tall = m_originalTall;

	if ( m_nStyle == BUTTON_GAMEMODE )
	{
		y = 0;
	}

	if ( ( m_nStyle == BUTTON_DROPDOWN || m_nStyle == BUTTON_GAMEMODE ) && GetCurrentState() == Open && m_nWideAtOpen )
	{
		wide = m_nWideAtOpen;
	}

	bool bAnimateGlow = false;
	bool bDrawGlow = false;
	bool bDrawCursor = false;
	bool bDrawText = true;
	Color col;
	State curState = GetCurrentState();
	switch ( curState )
	{
		case Enabled:
			// selectable, just not highlighted
			col = m_ColorEnabled;
			break;
		case Disabled:
			col = m_ColorDisabled;
			bDrawText = true;
			bDrawGlow = false;
			break;
		case FocusDisabled:
			col = m_ColorFocusDisabled;
			bDrawText = false;
			bDrawGlow = true;
			break;
		case Open:
			// flyout menu is attached
			col = m_ColorOpen;
			// ...or top level tab is active
			if ( m_nStyle != BUTTON_REACTIVEDROPMAINMENUTOP )
			{
				bDrawGlow = true;
				bDrawCursor = true;
			}
			break;
		case Focus:
			// active item
			col = m_ColorFocus;
			bDrawGlow = true;
			bAnimateGlow = true;
			if ( m_nStyle == BUTTON_SIMPLE ||
				 m_nStyle == BUTTON_DROPDOWN ||
				 m_nStyle == BUTTON_DIALOG ||
				 m_nStyle == BUTTON_RED )
			{
				bDrawCursor = true;
			}
			break;
	}

	SetFgColor( col );

	wchar_t szUnicode[512];
	GetText( szUnicode, sizeof( szUnicode ) );
	int len = V_wcslen( szUnicode );

	int textWide, textTall;
	GetTextImage()->GetContentSize( textWide, textTall );

	textWide = clamp( textWide, 0, wide - m_textInsetX * 2 );
	textTall = clamp( textTall, 0, tall - m_textInsetY * 2 );

	int textInsetX = m_textInsetX;
	if ( m_nStyle == BUTTON_DIALOG )
	{
		// dialog buttons are centered
		textInsetX = ( wide - textWide ) / 2;
	}
	if ( m_nStyle == BUTTON_REACTIVEDROPMAINMENU || m_nStyle == BUTTON_REACTIVEDROPMAINMENUBIG || m_nStyle == BUTTON_REACTIVEDROPMAINMENUTOP || m_nStyle == BUTTON_REACTIVEDROPMAINMENUTIMER )
	{
		// main menu buttons are centered in both directions
		textInsetX = ( wide - textWide ) / 2;
		y = ( tall - textTall ) / 2 - m_textInsetY * 2;
	}
	if ( m_nStyle == BUTTON_REACTIVEDROPMAINMENUHOIAF )
	{
		// only centered vertically
		y = ( tall - textTall ) / 2 - m_textInsetY * 2;
	}
	if ( m_nStyle == BUTTON_REACTIVEDROPMAINMENUSHOWCASE )
	{
		// showcase text is at the bottom
		y = tall - textTall - m_textInsetY * 2;

		if ( szUnicode[0] != L'\0' )
		{
			// black background so we can read the text on top of the image
			surface()->DrawSetColor( m_BlotchColor.r() * m_BlotchColor.a() / 255, m_BlotchColor.g() * m_BlotchColor.a() / 255, m_BlotchColor.b() * m_BlotchColor.a() / 255, 224 );
			surface()->DrawFilledRectFastFade( YRES( 1 ), y, wide - YRES( 1 ), tall - YRES( 1 ), y, y + YRES( 2 ), 0, 224, false );
		}
	}

	if ( FlyoutMenu::GetActiveMenu() && FlyoutMenu::GetActiveMenu()->GetNavFrom() != this )
	{
		bDrawCursor = false;
	}

	if ( bDrawCursor )
	{
		// draw backing rectangle
		if ( curState == Open )
		{
			surface()->DrawSetColor( Color( 0, 0, 0, 255 ) );
			surface()->DrawFilledRectFade( x, y, x+wide, y+tall, 0, 255, true );
		}

		// draw blotch
		surface()->DrawSetColor( m_BlotchColor );
		if ( m_nStyle == BUTTON_DIALOG )
		{
			int blotchWide = textWide;
			int blotchX = x + textInsetX;
			surface()->DrawFilledRectFade( blotchX, y, blotchX + 0.50f * blotchWide, y+tall, 0, 150, true );
			surface()->DrawFilledRectFade( blotchX + 0.50f * blotchWide, y, blotchX + blotchWide, y+tall, 150, 0, true );
		}
		else
		{
			int blotchWide = textWide + vgui::scheme()->GetProportionalScaledValueEx( GetScheme(), 40 );
			int blotchX = x + textInsetX;
			surface()->DrawFilledRectFade( blotchX, y, blotchX + 0.25f * blotchWide, y+tall, 0, 150, true );
			surface()->DrawFilledRectFade( blotchX + 0.25f * blotchWide, y, blotchX + blotchWide, y+tall, 150, 0, true );
		}

		// draw border lines
		surface()->DrawSetColor( m_BorderColor );
		if ( curState == Open )
		{
			FlyoutMenu *pActiveFlyout = FlyoutMenu::GetActiveMenu();
			BaseModHybridButton *button = dynamic_cast< BaseModHybridButton* >( pActiveFlyout ? pActiveFlyout->GetNavFrom() : NULL );
			if ( pActiveFlyout && pActiveFlyout->GetOriginalTall() == 0 && button && button == this )
			{
				surface()->DrawFilledRectFade( x, y, x + wide, y+2, 255, 0, true );
			}
			else
			{
				// the border lines end at the beginning of the flyout
				// the flyout will draw to complete the look
				surface()->DrawFilledRectFade( x, y, x + wide, y+2, 0, 255, true );
				surface()->DrawFilledRectFade( x, y+tall-2, x + wide, y+tall, 0, 255, true );
			}
		}
		else
		{
			// top and bottom border lines
			surface()->DrawFilledRectFade( x, y, x + 0.5f * wide, y+2, 0, 255, true );
			surface()->DrawFilledRectFade( x + 0.5f * wide, y, x + wide, y+2, 255, 0, true );
			surface()->DrawFilledRectFade( x, y+tall-2, x + 0.5f * wide, y+tall, 0, 255, true );
			surface()->DrawFilledRectFade( x + 0.5f * wide, y+tall-2, x + wide, y+tall, 255, 0, true );
		}
	}

#ifndef INFESTED_DLL
	// assume drawn, unless otherwise shortened with ellipsis
	int iLabelCharsDrawn = len;
#endif

	// draw the text
	if ( bDrawText )
	{
		int availableWidth = GetWide() - x - textInsetX;

		if ( m_bShowDropDownIndicator )
		{
			textTall = vgui::surface()->GetFontTall( m_hTextFont );
			int textLen = 0;

			len = wcslen( szUnicode );
			for ( int i = 0; i < len; i++ )
			{
				textLen += vgui::surface()->GetCharacterWidth( m_hTextFont, szUnicode[i] );
			}

			int imageX = x + textInsetX + textLen + m_iSelectedArrowSize / 2;
			int imageY = y + ( textTall - m_iSelectedArrowSize ) / 2;

			if ( ( imageX + m_iSelectedArrowSize ) > GetWide() )
			{
				imageX = GetWide() - m_iSelectedArrowSize;
			}

			if ( curState == Open || m_bOverrideDropDownIndicator )
			{
				vgui::surface()->DrawSetTexture( m_iSelectedArrow );
			}
			else
			{
				vgui::surface()->DrawSetTexture( m_iUnselectedArrow );
			}

			vgui::surface()->DrawSetColor( col );
			vgui::surface()->DrawTexturedRect( imageX, imageY, imageX + m_iSelectedArrowSize, imageY + m_iSelectedArrowSize );
			vgui::surface()->DrawSetTexture( 0 );

			availableWidth -= m_iSelectedArrowSize * 2;
		}

#ifdef INFESTED_DLL
		vgui::surface()->DrawSetColor( col );
		GetTextImage()->SetPos( x + textInsetX, y + m_textInsetY );
		GetTextImage()->Paint();
#else
		vgui::surface()->DrawSetTextFont( m_hTextFont );
		vgui::surface()->DrawSetTextPos( x + textInsetX, y + m_textInsetY );
		vgui::surface()->DrawSetTextColor( col );
		if ( textWide > availableWidth )
		{
			// length of 3 dots
			int ellipsesLen = 3 * vgui::surface()->GetCharacterWidth( m_hTextFont, L'.' );

			availableWidth -= ellipsesLen;

			iLabelCharsDrawn = 0;
			int charX = x + textInsetX;
			for ( int i = 0; i < len; i++ )
			{
				vgui::surface()->DrawUnicodeChar( szUnicode[i] );
				iLabelCharsDrawn++;

				charX += vgui::surface()->GetCharacterWidth( m_hTextFont, szUnicode[i] );
				if ( charX >= ( x + textInsetX + availableWidth ) )
					break;
			}

			vgui::surface()->DrawPrintText( L"...", 3 );
		}
		else
		{
			vgui::surface()->DrawUnicodeString( szUnicode );
		}
#endif
	}
	else if ( GetCurrentState() == Disabled || GetCurrentState() == FocusDisabled )
	{
		Color textcol = col;
		textcol[ 3 ] = 64;

#ifdef INFESTED_DLL
		vgui::surface()->DrawSetColor( textcol );
		GetTextImage()->SetPos( x + textInsetX, y + m_textInsetY );
		GetTextImage()->Paint();
#else
		vgui::surface()->DrawSetTextFont( m_hTextFont );
		vgui::surface()->DrawSetTextPos( x + textInsetX, y + m_textInsetY  );
		vgui::surface()->DrawSetTextColor( textcol );
		vgui::surface()->DrawPrintText( szUnicode, len );
#endif
	}

	// draw the help text
	if ( m_nStyle == BUTTON_GAMEMODE && curState != Open )
	{
		const char *pHelpText = GetHelpText( IsEnabled() );
		wchar_t szHelpUnicode[512];
		wchar_t *pUnicodeString;
		if ( pHelpText && pHelpText[0] == '#' )
		{
			pUnicodeString = g_pVGuiLocalize->Find( pHelpText );
		}
		else
		{
			g_pVGuiLocalize->ConvertANSIToUnicode( pHelpText, szHelpUnicode, sizeof( szHelpUnicode ) );
			pUnicodeString = szHelpUnicode;
		}	
		vgui::surface()->DrawSetTextFont( m_hHintTextFont );
		vgui::surface()->DrawSetTextPos( x + textInsetX, y + m_textInsetY + m_nTextFontHeight );
		vgui::surface()->DrawSetTextColor( col );
		vgui::surface()->DrawPrintText( pUnicodeString, wcslen( pUnicodeString ) );
	}

	// draw the pulsing glow
	if ( bDrawGlow )
	{
		if ( !bDrawText )
		{
#ifdef INFESTED_DLL
			vgui::surface()->DrawSetColor( col );
#else
			vgui::surface()->DrawSetTextColor( col );
#endif
		}
		else
		{
			int alpha = bAnimateGlow ? 60.0f + 30.0f * sin( Plat_FloatTime() * 4.0f ) : 30;
#ifdef INFESTED_DLL
			if ( bAnimateGlow && rd_reduce_motion.GetBool() )
				alpha = 60;
#endif
			Color glowColor( 255, 255, 255, alpha );
#ifdef INFESTED_DLL
			SetFgColor( glowColor );
			vgui::surface()->DrawSetColor( glowColor );
#else
			vgui::surface()->DrawSetTextColor( glowColor );
#endif
		}
#ifdef INFESTED_DLL
		GetTextImage()->SetPos( x + textInsetX, y + m_textInsetY );
		GetTextImage()->SetFont( m_hTextBlurFont );
		GetTextImage()->Paint();
		GetTextImage()->SetFont( m_hTextFont );
		SetFgColor( col );
#else
		vgui::surface()->DrawSetTextFont( m_hTextBlurFont );
		vgui::surface()->DrawSetTextPos( x + textInsetX, y + m_textInsetY );

		for ( int i=0; i<len && i < iLabelCharsDrawn; i++ )
		{
			vgui::surface()->DrawUnicodeChar( szUnicode[i] );
		}
#endif
	}

	if ( m_nStyle == BUTTON_DROPDOWN && curState != Open )
	{
		// draw right justified item
		wchar_t *pUnicodeString = g_pVGuiLocalize->Find( m_DropDownSelection.String() );
		if ( !pUnicodeString )
		{
			g_pVGuiLocalize->ConvertANSIToUnicode( m_DropDownSelection.String(), szUnicode, sizeof( szUnicode ) );
			pUnicodeString = szUnicode;
		}
		len = V_wcslen( pUnicodeString );

		surface()->GetTextSize( m_hSelectionFont, pUnicodeString, textWide, textTall );

		// horizontal right justify
		int xx = wide - textWide - textInsetX;
		// vertical center within
		int yy = ( tall - textTall )/2;

		// draw the drop down selection text
		vgui::surface()->DrawSetTextFont( m_hSelectionFont );
		vgui::surface()->DrawSetTextPos( xx, yy );
		vgui::surface()->DrawSetTextColor( col );
		vgui::surface()->DrawPrintText( pUnicodeString, len );

		if ( bDrawGlow )
		{
			int alpha = bAnimateGlow ? 60.0f + 30.0f * sin( Plat_FloatTime() * 4.0f ) : 30;
#ifdef INFESTED_DLL
			if ( bAnimateGlow && rd_reduce_motion.GetBool() )
				alpha = 60;
#endif
			vgui::surface()->DrawSetTextColor( Color( 255, 255, 255, alpha ) );
			vgui::surface()->DrawSetTextFont( m_hSelectionBlurFont );
			vgui::surface()->DrawSetTextPos( xx, yy );
			vgui::surface()->DrawPrintText( pUnicodeString, len );
		}
	}
}

void BaseModHybridButton::Paint()
{
	// bypass ALL of CA's inept broken drawing code
	// not using VGUI controls, to much draw state is misconfigured by CA to salvage
	PaintButtonEx();

	// only do the forced selection for a single frame.
	m_isNavigateTo = false;
}

void BaseModHybridButton::OnThink()
{
	switch( mEnableCondition )
	{
	case EC_LIVE_REQUIRED:
		{
#ifdef _X360 
			SetEnabled( CUIGameData::Get()->SignedInToLive() );
#else
			SetEnabled( true );
#endif
		}	
		break;
	case EC_NOTFORDEMO:
		{
			if ( IsEnabled() )
			{
				Demo_DisableButton( this );
			}
		}
		break;

	}

	BaseClass::OnThink();
}

void BaseModHybridButton::ApplySettings( KeyValues * inResourceData )
{
	BaseClass::ApplySettings( inResourceData );

	vgui::IScheme *scheme = vgui::scheme()->GetIScheme( GetScheme() );
	if( !scheme )
		return;

	char keyString[128];

	// if a style is specified attempt to load values in from the SCHEME file
	const char *style = inResourceData->GetString( "Style", NULL );
	if ( !style )
	{
		style = "DefaultButton";
	}

	// this is a total bypass of the CA look
	if ( !m_bSetStyle )
	{
		m_nStyle = BUTTON_SIMPLE;
		V_snprintf( keyString, sizeof( keyString ), "%s.Style", style );
		const char *pFormatString = scheme->GetResourceString( keyString );
		if ( pFormatString && pFormatString[0] )
		{
			m_nStyle = ( ButtonStyle_t )atoi( pFormatString );
		}
	}

	ApplyStyle();

	m_nWideAtOpen = vgui::scheme()->GetProportionalScaledValue( inResourceData->GetInt( "wideatopen", 0 ) );

	// tooltips
	const char* enabledToolTipText = inResourceData->GetString( "tooltiptext" , NULL );
	if ( enabledToolTipText )
	{
		SetHelpText( enabledToolTipText, true );
	}

	const char* disabledToolTipText = inResourceData->GetString( "disabled_tooltiptext", NULL );
	if ( disabledToolTipText )
	{
		SetHelpText( disabledToolTipText, false );
	}

	//vgui's standard handling of the tabPosition tag doesn't properly navigate for the X360
	if ( inResourceData->GetInt( "tabPosition", 0 ) == 1 )
	{
		NavigateTo();
	}

	//Get the x text inset
	V_snprintf( keyString, sizeof( keyString ), "%s.%s", style, "TextInsetX" );
	const char *result = scheme->GetResourceString( keyString );
	if ( *result != 0 )
	{
		m_textInsetX = atoi( result );
		m_textInsetX = vgui::scheme()->GetProportionalScaledValueEx( GetScheme(), m_textInsetX );
	}

	//Get the y text inset
	V_snprintf( keyString, sizeof( keyString ), "%s.%s", style, "TextInsetY" );
	result = scheme->GetResourceString( keyString );
	if( *result != 0 )
	{
		m_textInsetY = atoi( result );
		m_textInsetY = vgui::scheme()->GetProportionalScaledValueEx( GetScheme(), m_textInsetY );
	}

	// tell Label double the inset so it adjusts the width for word wrap/ellipsis
	SetTextInset( m_textInsetX * 2, m_textInsetY * 2 );

	//0 = press and release
	//1 = press
	//2 = release
	int activationType = inResourceData->GetInt( "ActivationType" );
	activationType = clamp( activationType, 0, 2 );
	SetButtonActivationType( static_cast< vgui::Button::ActivationType_t >( activationType ) );

	m_iUsablePlayerIndex = USE_EVERYBODY;
	if ( const char *pszValue = inResourceData->GetString( "usablePlayerIndex", "" ) )
	{
		if ( !stricmp( "primary", pszValue ) )
		{
			m_iUsablePlayerIndex = USE_PRIMARY;
		}
		else if ( !stricmp( "nobody", pszValue ) )
		{
			m_iUsablePlayerIndex = USE_NOBODY;
		}
		else if ( V_isdigit( pszValue[0] ) )
		{
			m_iUsablePlayerIndex = atoi( pszValue );
		}
	}

	//handle different conditions to allow the control to be enabled and disabled automatically
	const char * condition = inResourceData->GetString( "EnableCondition" );
	for ( int index = 0; index < ( sizeof( sHybridStates ) / sizeof( HybridEnableStates ) ); ++index )
	{
		if ( Q_stricmp( condition, sHybridStates[ index ].mConditionName ) == 0 )
		{
			mEnableCondition = sHybridStates[ index ].mCondition;
			break;
		}
	}

	if ( mEnableCondition == EC_NOTFORDEMO )
	{
		if ( IsEnabled() )
		{
			Demo_DisableButton( this );
		}
	}

	m_bOnlyActiveUser = ( inResourceData->GetInt( "OnlyActiveUser", 0 ) != 0 );
	m_bIgnoreButtonA = ( inResourceData->GetInt( "IgnoreButtonA", 0 ) != 0 );

	m_bShowDropDownIndicator = ( inResourceData->GetInt( "ShowDropDownIndicator", 0 ) != 0 );
	m_bOverrideDropDownIndicator = false;

	m_iSelectedArrowSize = vgui::scheme()->GetProportionalScaledValue( inResourceData->GetInt( "DropDownIndicatorSize", 8 ) );
}

void BaseModHybridButton::ApplyStyle()
{
	vgui::IScheme *scheme = vgui::scheme()->GetIScheme( GetScheme() );
	if ( !scheme )
		return;

	const char *szStyle = NULL;

#define GET_FONT( var, def ) scheme->GetFont( scheme->GetResourceString( var ) ? scheme->GetResourceString( var ) : def, true )

	if ( m_nStyle == BUTTON_MAINMENU )
	{
		szStyle = "MainMenuButton";
		m_hTextFont = GET_FONT( "MainMenuButton.Font", "MainBold" );
		m_hTextBlurFont = GET_FONT( "MainMenuButton.FontBlur", "MainBoldBlur" );
	}
	else if ( m_nStyle == BUTTON_FLYOUTITEM )
	{
		szStyle = "FlyoutMenuButton";
		m_hTextFont = GET_FONT( "FlyoutMenuButton.Font", "DefaultMedium" );
		m_hTextBlurFont = GET_FONT( "FlyoutMenuButton.FontBlur", "DefaultMediumBlur" );
	}
	else if ( m_nStyle == BUTTON_DROPDOWN )
	{
		szStyle = "DropDownButton";
		m_hTextFont = GET_FONT( "DropDownButton.Font", "DefaultBold" );
		m_hTextBlurFont = GET_FONT( "DropDownButton.FontBlur", "DefaultBoldBlur" );
		m_hSelectionFont = GET_FONT( "DropDownButton.FontSelected", "DefaultMedium" );
		m_hSelectionBlurFont = GET_FONT( "DropDownButton.FontSelectedBlur", "DefaultMediumBlur" );
	}
	else if ( m_nStyle == BUTTON_DIALOG )
	{
		szStyle = "DialogButton";
		m_hTextFont = GET_FONT( "DialogButton.Font", "DefaultBold" );
		m_hTextBlurFont = GET_FONT( "DialogButton.FontBlur", "DefaultBoldBlur" );
	}
	else if ( m_nStyle == BUTTON_RED )
	{
		szStyle = "RedButton";
		m_hTextFont = GET_FONT( "RedButton.Font", "DefaultBold" );
		m_hTextBlurFont = GET_FONT( "RedButton.FontBlur", "DefaultBoldBlur" );
	}
	else if ( m_nStyle == BUTTON_REDMAIN )
	{
		szStyle = "RedMainButton";
		m_hTextFont = GET_FONT( "RedMainButton.Font", "DefaultBold" );
		m_hTextBlurFont = GET_FONT( "RedMainButton.FontBlur", "DefaultBoldBlur" );
	}
	else if ( m_nStyle == BUTTON_SMALL )
	{
		szStyle = "SmallButton";
		m_hTextFont = GET_FONT( "SmallButton.Font", "DefaultVerySmall" );
		m_hTextBlurFont = GET_FONT( "SmallButton.FontBlur", "DefaultVerySmall" );
	}
	else if ( m_nStyle == BUTTON_MEDIUM )
	{
		szStyle = "MediumButton";
		m_hTextFont = GET_FONT( "MediumButton.Font", "DefaultMedium" );
		m_hTextBlurFont = GET_FONT( "MediumButton.FontBlur", "DefaultMediumBlur" );
	}
	else if ( m_nStyle == BUTTON_GAMEMODE )
	{
		szStyle = "GameModeButton";
		m_hTextFont = GET_FONT( "GameModeButton.Font", "MainBold" );
		m_hTextBlurFont = GET_FONT( "GameModeButton.FontBlur", "MainBoldBlur" );
		m_hHintTextFont = GET_FONT( "GameModeButton.FontHint", "Default" );
	}
	else if ( m_nStyle == BUTTON_MAINMENUSMALL )
	{
		szStyle = "MainMenuSmallButton";
		m_hTextFont = GET_FONT( "MainMenuSmallButton.Font", "DefaultBold" );
		m_hTextBlurFont = GET_FONT( "MainMenuSmallButton.FontBlur", "DefaultBoldBlur" );
	}
	else if ( m_nStyle == BUTTON_ALIENSWARMMENUBUTTON )
	{
		szStyle = "AlienSwarmMenuButton";
		m_hTextFont = GET_FONT( "AlienSwarmMenuButton.Font", "DefaultBold" );
		m_hTextBlurFont = GET_FONT( "AlienSwarmMenuButton.FontBlur", "DefaultBoldBlur" );
	}
	else if ( m_nStyle == BUTTON_ALIENSWARMMENUBUTTONSMALL )
	{
		szStyle = "AlienSwarmMenuButtonSmall";
		m_hTextFont = GET_FONT( "AlienSwarmMenuButtonSmall.Font", "DefaultMedium" );
		m_hTextBlurFont = GET_FONT( "AlienSwarmMenuButtonSmall.FontBlur", "DefaultMediumBlur" );
	}
	else if ( m_nStyle == BUTTON_ALIENSWARMDEFAULT )
	{
		szStyle = "AlienSwarmDefault";
		m_hTextFont = GET_FONT( "AlienSwarmDefault.Font", "Default" );
		m_hTextBlurFont = GET_FONT( "AlienSwarmDefault.FontBlur", "DefaultBlur" );
	}
	else if ( m_nStyle == BUTTON_REACTIVEDROPMAINMENU )
	{
		szStyle = "ReactiveDropMainMenu";
		m_hTextFont = GET_FONT( "ReactiveDropMainMenu.Font", "DefaultLarge" );
		m_hTextBlurFont = GET_FONT( "ReactiveDropMainMenu.FontBlur", "DefaultLargeBlur" );
	}
	else if ( m_nStyle == BUTTON_REACTIVEDROPMAINMENUBIG )
	{
		szStyle = "ReactiveDropMainMenuBig";
		m_hTextFont = GET_FONT( "ReactiveDropMainMenuBig.Font", "DefaultExtraLarge" );
		m_hTextBlurFont = GET_FONT( "ReactiveDropMainMenuBig.FontBlur", "DefaultExtraLargeBlur" );
	}
	else if ( m_nStyle == BUTTON_REACTIVEDROPMAINMENUTOP )
	{
		szStyle = "ReactiveDropMainMenuTop";
		m_hTextFont = GET_FONT( "ReactiveDropMainMenuTop.Font", "DefaultMedium" );
		m_hTextBlurFont = GET_FONT( "ReactiveDropMainMenuTop.FontBlur", "DefaultMediumBlur" );
	}
	else if ( m_nStyle == BUTTON_REACTIVEDROPMAINMENUSHOWCASE )
	{
		szStyle = "ReactiveDropMainMenuShowcase";
		m_hTextFont = GET_FONT( "ReactiveDropMainMenuShowcase.Font", "Default" );
		m_hTextBlurFont = GET_FONT( "ReactiveDropMainMenuShowcase.FontBlur", "DefaultBlur" );
	}
	else if ( m_nStyle == BUTTON_REACTIVEDROPMAINMENUTIMER )
	{
		szStyle = "ReactiveDropMainMenuTimer";
		m_hTextFont = GET_FONT( "ReactiveDropMainMenuTimer.Font", "Default" );
		m_hTextBlurFont = GET_FONT( "ReactiveDropMainMenuTimer.FontBlur", "DefaultBlur" );
		SetWrap( true );
	}
	else if ( m_nStyle == BUTTON_REACTIVEDROPMAINMENUHOIAF )
	{
		szStyle = "ReactiveDropMainMenuHoIAF";
		// fonts handled by subclass
	}
	else
	{
		Assert( m_nStyle == BUTTON_SIMPLE );
		szStyle = "DefaultButton";
		m_nStyle = BUTTON_SIMPLE;
		m_hTextFont = GET_FONT( "DefaultButton.Font", "DefaultBold" );
		m_hTextBlurFont = GET_FONT( "DefaultButton.FontBlur", "DefaultBoldBlur" );
	}

#undef GET_FONT

	Assert( szStyle );

	m_nTextFontHeight = vgui::surface()->GetFontTall( m_hTextFont );
	m_nHintTextFontHeight = vgui::surface()->GetFontTall( m_hHintTextFont );

	SetFont( m_hTextFont );

	int x, y, wide, tall;
	GetBounds( x, y, wide, tall );
	m_originalTall = tall;

	if ( m_nStyle == BUTTON_GAMEMODE )
	{
		// needs to be exact height sized
		tall = m_nTextFontHeight + m_nHintTextFontHeight;
		SetSize( wide, tall );
		m_originalTall = m_nTextFontHeight;
	}

	char szKey[128];
	V_snprintf( szKey, sizeof( szKey ), "%s.ColorEnabled", szStyle );
	m_ColorEnabled = scheme->GetColor( szKey, Color( 83, 148, 192, 255 ) );
	V_snprintf( szKey, sizeof( szKey ), "%s.ColorDisabled", szStyle );
	m_ColorDisabled = scheme->GetColor( szKey, Color( 32, 59, 82, 255 ) );
	V_snprintf( szKey, sizeof( szKey ), "%s.ColorFocusDisabled", szStyle );
	m_ColorFocusDisabled = scheme->GetColor( szKey, Color( 182, 189, 194, 255 ) );
	V_snprintf( szKey, sizeof( szKey ), "%s.ColorOpen", szStyle );
	m_ColorOpen = scheme->GetColor( szKey, Color( 169, 213, 255, 255 ) );
	V_snprintf( szKey, sizeof( szKey ), "%s.ColorFocus", szStyle );
	m_ColorFocus = scheme->GetColor( szKey, Color( 255, 255, 255, 255 ) );
}

void BaseModHybridButton::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	SetReleasedSound( CBaseModPanel::GetSingleton().GetUISoundName( UISOUND_ACCEPT ) );

	const char *pImageName;

	// use find or create pattern, avoid pointless redundant i/o
	pImageName = "vgui/icon_arrow_down";
	m_iSelectedArrow = vgui::surface()->DrawGetTextureId( pImageName );
	if ( m_iSelectedArrow == -1 )
	{
		m_iSelectedArrow = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile( m_iSelectedArrow, pImageName, true, false );	
	}

	// use find or create pattern, avoid pointles redundant i/o
	pImageName = "vgui/icon_arrow";
	m_iUnselectedArrow = vgui::surface()->DrawGetTextureId( pImageName );
	if ( m_iUnselectedArrow == -1 )
	{
		m_iUnselectedArrow = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile( m_iUnselectedArrow, pImageName, true, false );	
	}

	// cache some colors so we don't do string lookups on every frame
	m_BlotchColor = pScheme->GetColor( "HybridButton.BlotchColor", Color( 0, 0, 0, 255 ) );
	m_BorderColor = pScheme->GetColor( "HybridButton.BorderColor", Color( 0, 0, 0, 255 ) );
}

void BaseModHybridButton::OnKeyCodePressed( vgui::KeyCode code )
{
	int iJoystick = GetJoystickForCode( code );

	if ( m_bOnlyActiveUser )
	{
		// Only allow input from the active userid
		int userId = CBaseModPanel::GetSingleton().GetLastActiveUserId();

		if( iJoystick != userId || iJoystick < 0 )
		{	
			return;
		}
	}

	BaseModUI::CBaseModPanel::GetSingleton().SetLastActiveUserId( iJoystick );

	int iController = XBX_GetUserId( iJoystick );
	bool bIsPrimaryUser = ( iController >= 0 && XBX_GetActiveUserId() == iController );

	KeyCode localCode = GetBaseButtonCode( code );

	if ( ( localCode == KEY_XBUTTON_A ) )
	{
		if ( m_bIgnoreButtonA )
		{
			// Don't swallow the a key... our parent wants it
			CallParentFunction(new KeyValues("KeyCodePressed", "code", code));
			return;
		}

		bool bEnabled = true;
		if ( !IsEnabled() )
		{
			bEnabled = false;
		}

		switch( m_iUsablePlayerIndex )
		{
		case USE_EVERYBODY:
			break;

		case USE_PRIMARY:
			if ( !bIsPrimaryUser )
				bEnabled = false;
			break;

		case USE_SLOT0:
		case USE_SLOT1:
		case USE_SLOT2:
		case USE_SLOT3:
			if ( iJoystick != m_iUsablePlayerIndex )
				bEnabled = false;
			break;

		default:
			bEnabled = false;
			break;
		}

		if ( !bEnabled )
		{
			CBaseModPanel::GetSingleton().PlayUISound( UISOUND_INVALID );
			return;
		}
	}

	if ( IsX360() && m_nStyle == BUTTON_GAMEMODE )
	{
		GameModes *pGameModes = dynamic_cast< GameModes * >( GetParent() );
		if ( pGameModes )
		{
			switch ( localCode )
			{
			case KEY_XBUTTON_A:
				if ( pGameModes->IsScrollBusy() )
				{
					// swallow it
					return;
				}
				break;

			case KEY_XBUTTON_LEFT:
			case KEY_XSTICK1_LEFT:
			case KEY_XSTICK2_LEFT:
				pGameModes->ScrollLeft();
				break;

			case KEY_XBUTTON_RIGHT:
			case KEY_XSTICK1_RIGHT:
			case KEY_XSTICK2_RIGHT:
				pGameModes->ScrollRight();
				break;
			}
		}
	}

	BaseClass::OnKeyCodePressed( code );
}

void BaseModHybridButton::OnKeyCodeReleased( KeyCode keycode )
{
	//at some point vgui_controls/button.cpp got a 360 only change to never set the armed state to false ever. Too late to fix the logic behind that, just roll with it on PC
	//fixes bug where menu items de-highlight after letting go of the arrow key used to navigate to it
	bool bOldArmedState = IsArmed();

	BaseClass::OnKeyCodeReleased( keycode );

	if( bOldArmedState && !IsArmed() )
		SetArmed( true );
}

void BaseModHybridButton::SetDropdownSelection( const char *pText )
{
	m_DropDownSelection = pText;
}

void BaseModHybridButton::EnableDropdownSelection( bool bEnable )
{
	m_bDropDownSelection = bEnable;
}

void BaseModHybridButton::OnCursorEntered()
{
	BaseClass::OnCursorEntered();
	if ( IsPC() )
	{
		if ( !m_isOpen )
		{
			if ( IsEnabled() && !HasFocus() )
			{
				CBaseModPanel::GetSingleton().PlayUISound( UISOUND_FOCUS );
			}

			if( GetParent() )
			{
				GetParent()->NavigateToChild( this );
			}
			else
			{
				NavigateTo();
			}
		}
	}
}

void BaseModHybridButton::OnCursorExited()
{
	// This is a hack for now, we shouldn't close if the cursor goes to the flyout of this item...
	// Maybe have VFloutMenu check the m_navFrom and it's one of these, keep the SetClosedState...
	BaseClass::OnCursorExited();
	if ( IsPC() )
	{
//		SetClosed();
	}
}

// Message targets that the button has been pressed
void BaseModHybridButton::FireActionSignal( void )
{
	BaseClass::FireActionSignal();

	if ( IsPC() )
	{
		PostMessageToAllSiblingsOfType< BaseModHybridButton >( new KeyValues( "OnSiblingHybridButtonOpened" ) );
	}
}


Panel* BaseModHybridButton::NavigateUp()
{
	Panel *target = BaseClass::NavigateUp();
	if ( IsPC() && !target && 
		(dynamic_cast< DropDownMenu * >( GetParent() ) || dynamic_cast< SliderControl * >( GetParent() )) )
	{
		target = GetParent()->NavigateUp();
	}

	return target;
}

Panel* BaseModHybridButton::NavigateDown()
{
	Panel *target = BaseClass::NavigateDown();
	if ( IsPC() && !target && 
		(dynamic_cast< DropDownMenu * >( GetParent() ) || dynamic_cast< SliderControl * >( GetParent() )) )
	{
		target = GetParent()->NavigateDown();
	}
	return target;
}

Panel* BaseModHybridButton::NavigateLeft()
{
	Panel *target = BaseClass::NavigateLeft();
	if ( IsPC() && !target && 
		dynamic_cast< DropDownMenu * >( GetParent() ) )
	{
		target = GetParent()->NavigateLeft();
	}
	return target;
}

Panel* BaseModHybridButton::NavigateRight()
{
	Panel *target = BaseClass::NavigateRight();
	if ( IsPC() && !target && 
		dynamic_cast< DropDownMenu * >( GetParent() ) )
	{
		target = GetParent()->NavigateRight();
	}
	return target;
}
