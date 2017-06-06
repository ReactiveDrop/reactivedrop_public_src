//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//

#include "cbase.h"
#include "VGamepad.h"
#include "VFooterPanel.h"
#include "VDropDownMenu.h"
#include "VSliderControl.h"
#include "gameui_util.h"
#include "vgui/ISurface.h"
#include "EngineInterface.h"
#include "filesystem.h"
#include "fmtstr.h"
#include "cdll_util.h"
#include "nb_button.h"
#include "nb_header_footer.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;
using namespace BaseModUI;



//=============================================================================
Gamepad::Gamepad(Panel *parent, const char *panelName):
BaseClass(parent, panelName)
{
	GameUI().PreventEngineHideGameUI();

	SetDeleteSelfOnClose(true);

	SetProportional( true );

	SetUpperGarnishEnabled(true);
	SetLowerGarnishEnabled(true);

	m_pHeaderFooter = new CNB_Header_Footer( this, "HeaderFooter" );
	m_pHeaderFooter->SetTitle( "" );
	m_pHeaderFooter->SetHeaderEnabled( false );
	m_pHeaderFooter->SetFooterEnabled( true );
	m_pHeaderFooter->SetGradientBarEnabled( true );
	m_pHeaderFooter->SetGradientBarPos( 150, 120 );

	m_drpGamepadEnable = NULL;
	m_sldGamepadHSensitivity = NULL;
	m_sldGamepadVSensitivity = NULL;
	m_drpGamepadYInvert = NULL;
	m_drpGamepadSwapSticks = NULL;

	m_btnDone = NULL;
}

//=============================================================================
Gamepad::~Gamepad()
{
	GameUI().AllowEngineHideGameUI();

	UpdateFooter( false );
}

void Gamepad::PerformLayout()
{
	BaseClass::PerformLayout();

	SetBounds( 0, 0, ScreenWidth(), ScreenHeight() );
}

//=============================================================================
void Gamepad::Activate()
{
	BaseClass::Activate();

	CGameUIConVarRef joystick("joystick");

	if ( m_drpGamepadEnable )
	{
		if ( !joystick.GetBool() )
		{
			m_drpGamepadEnable->SetCurrentSelection( "GamepadDisabled" );
		}
		else
		{
			m_drpGamepadEnable->SetCurrentSelection( "GamepadEnabled" );
		}

		FlyoutMenu *pFlyout = m_drpGamepadEnable->GetCurrentFlyout();
		if ( pFlyout )
		{
			pFlyout->SetListener( this );
		}
	}

	if ( m_sldGamepadHSensitivity )
	{
		m_sldGamepadHSensitivity->Reset();

		m_sldGamepadHSensitivity->SetEnabled( joystick.GetBool() );
	}

	if ( m_sldGamepadVSensitivity )
	{
		m_sldGamepadVSensitivity->Reset();

		m_sldGamepadVSensitivity->SetEnabled( joystick.GetBool() );
	}

	if ( m_drpGamepadYInvert )
	{
		CGameUIConVarRef joy_inverty("joy_inverty");

		if ( !joy_inverty.GetBool() )
		{
			m_drpGamepadYInvert->SetCurrentSelection( "GamepadYInvertDisabled" );
		}
		else
		{
			m_drpGamepadYInvert->SetCurrentSelection( "GamepadYInvertEnabled" );
		}

		m_drpGamepadYInvert->SetEnabled( joystick.GetBool() );

		FlyoutMenu *pFlyout = m_drpGamepadYInvert->GetCurrentFlyout();
		if ( pFlyout )
		{
			pFlyout->SetListener( this );
		}
	}

	if ( m_drpGamepadSwapSticks )
	{
		CGameUIConVarRef joy_movement_stick("joy_movement_stick");

		if ( !joy_movement_stick.GetBool() )
		{
			m_drpGamepadSwapSticks->SetCurrentSelection( "GamepadSwapSticksDisabled" );
		}
		else
		{
			m_drpGamepadSwapSticks->SetCurrentSelection( "GamepadSwapSticksEnabled" );
		}

		m_drpGamepadSwapSticks->SetEnabled( joystick.GetBool() );

		FlyoutMenu *pFlyout = m_drpGamepadSwapSticks->GetCurrentFlyout();
		if ( pFlyout )
		{
			pFlyout->SetListener( this );
		}
	}

	UpdateFooter( true );
}


void Gamepad::UpdateFooter( bool bEnableCloud )
{
	if ( !BaseModUI::CBaseModPanel::GetSingletonPtr() )
		return;

	CBaseModFooterPanel *footer = BaseModUI::CBaseModPanel::GetSingleton().GetFooterPanel();
	if ( footer )
	{
		footer->SetButtons( FB_ABUTTON | FB_BBUTTON, FF_AB_ONLY, false );
		footer->SetButtonText( FB_ABUTTON, "#L4D360UI_Select" );
		footer->SetButtonText( FB_BBUTTON, "#L4D360UI_Controller_Done" );

		footer->SetShowCloud( bEnableCloud );
	}
}

void Gamepad::OnThink()
{
	BaseClass::OnThink();

	bool needsActivate = false;

	if( !m_drpGamepadEnable )
	{
		m_drpGamepadEnable = dynamic_cast< DropDownMenu* >( FindChildByName( "DrpGamepadEnable" ) );
		needsActivate = true;
	}

	if( !m_sldGamepadHSensitivity )
	{
		m_sldGamepadHSensitivity = dynamic_cast< SliderControl* >( FindChildByName( "SldGamepadHSensitivity" ) );
		needsActivate = true;
	}

	if( !m_sldGamepadVSensitivity )
	{
		m_sldGamepadVSensitivity = dynamic_cast< SliderControl* >( FindChildByName( "SldGamepadVSensitivity" ) );
		needsActivate = true;
	}

	if( !m_drpGamepadYInvert )
	{
		m_drpGamepadYInvert = dynamic_cast< DropDownMenu* >( FindChildByName( "DrpGamepadYInvert" ) );
		needsActivate = true;
	}

	if( !m_drpGamepadSwapSticks )
	{
		m_drpGamepadSwapSticks = dynamic_cast< DropDownMenu* >( FindChildByName( "DrpGamepadSwapSticks" ) );
		needsActivate = true;
	}

	if( !m_btnDone )
	{
		m_btnDone = dynamic_cast< CNB_Button* >( FindChildByName( "BtnDone" ) );
		needsActivate = true;
	}

	if ( needsActivate )
	{
		Activate();
	}
}

void Gamepad::OnKeyCodePressed(KeyCode code)
{
	int joystick = GetJoystickForCode( code );
	int userId = CBaseModPanel::GetSingleton().GetLastActiveUserId();
	if ( joystick != userId || joystick < 0 )
	{	
		return;
	}

	switch ( GetBaseButtonCode( code ) )
	{
	case KEY_XBUTTON_B:
		// nav back
		BaseClass::OnKeyCodePressed(code);
		break;

	default:
		BaseClass::OnKeyCodePressed(code);
		break;
	}
}

//=============================================================================
void Gamepad::OnCommand(const char *command)
{
	if( Q_stricmp( "GamepadEnabled", command ) == 0 )
	{
		CGameUIConVarRef joystick("joystick");

		if( joystick.GetBool() == false )
		{
			// The joystick is being enabled, and this is a state change
			// rather than a redundant execution of this code. Enable
			// the gamepad controls by execing a config file.
			if ( IsPC() )
			{
				engine->ClientCmd_Unrestricted( "exec 360controller_pc.cfg" );
			}
			else if ( IsX360() )
			{
				engine->ClientCmd_Unrestricted( "exec 360controller_xbox.cfg" );
			}
		}

		joystick.SetValue( true );

		SetControlEnabled( "SldGamepadHSensitivity", true );
		SetControlEnabled( "SldGamepadVSensitivity", true );
		SetControlEnabled( "DrpGamepadYInvert", true );
		SetControlEnabled( "DrpGamepadSwapSticks", true );
	}
	else if( Q_stricmp( "GamepadDisabled", command ) == 0 )
	{
		CGameUIConVarRef joystick("joystick");

		if( joystick.GetBool() == true )
		{
			// The gamepad is being disabled, and this is a state change
			// rather than a redundant execution of this code. Disable
			// the gamepad by execing a config file.
			char const *szConfigFile = "exec undo360controller.cfg";
			engine->ClientCmd_Unrestricted(szConfigFile);
		}

		joystick.SetValue( false );

		SetControlEnabled( "SldGamepadHSensitivity", false );
		SetControlEnabled( "SldGamepadVSensitivity", false );

		if ( m_drpGamepadYInvert )
		{
			m_drpGamepadYInvert->CloseDropDown();
			m_drpGamepadYInvert->SetEnabled( false );
		}

		if ( m_drpGamepadSwapSticks )
		{
			m_drpGamepadSwapSticks->CloseDropDown();
			m_drpGamepadSwapSticks->SetEnabled( false );
		}
	}
	else if( Q_stricmp( "GamepadYInvertEnabled", command ) == 0 )
	{
		CGameUIConVarRef joy_inverty("joy_inverty");
		joy_inverty.SetValue( true );
	}
	else if( Q_stricmp( "GamepadYInvertDisabled", command ) == 0 )
	{
		CGameUIConVarRef joy_inverty("joy_inverty");
		joy_inverty.SetValue( false );
	}
	else if( Q_stricmp( "GamepadSwapSticksEnabled", command ) == 0 )
	{
		CGameUIConVarRef joy_movement_stick("joy_movement_stick");
		joy_movement_stick.SetValue( true );
	}
	else if( Q_stricmp( "GamepadSwapSticksDisabled", command ) == 0 )
	{
		CGameUIConVarRef joy_movement_stick("joy_movement_stick");
		joy_movement_stick.SetValue( false );
	}
	else if( Q_stricmp( "Back", command ) == 0 )
	{
		OnKeyCodePressed( KEY_XBUTTON_B );
	}
	else
	{
		BaseClass::OnCommand( command );
	}
}

void Gamepad::OnNotifyChildFocus( vgui::Panel* child )
{
}

void Gamepad::OnFlyoutMenuClose( vgui::Panel* flyTo )
{
	UpdateFooter( true );
}

void Gamepad::OnFlyoutMenuCancelled()
{
}

//=============================================================================
Panel* Gamepad::NavigateBack()
{
	engine->ClientCmd_Unrestricted( VarArgs( "host_writeconfig_ss %d", XBX_GetActiveUserId() ) );

	return BaseClass::NavigateBack();
}

void Gamepad::PaintBackground()
{
	//BaseClass::DrawDialogBackground( "#GameUI_Multiplayer", NULL, "#L4D360UI_Multiplayer_Desc", NULL, NULL, true );
}

void Gamepad::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	// required for new style
	SetPaintBackgroundEnabled( true );
	SetupAsDialogStyle();
}
