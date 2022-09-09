#include "cbase.h"
#include "vadvancedsettings.h"
#include "nb_header_footer.h"
#include "filesystem.h"
#include "vgui/ILocalize.h"
#include "vdropdownmenu.h"
#include "vhybridbutton.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;
using namespace BaseModUI;

extern ConVar ui_gameui_modal;

AdvancedSettings::AdvancedSettings( Panel *parent, const char *panelName )
	: BaseClass( parent, panelName )
{
	if ( ui_gameui_modal.GetBool() )
	{
		GameUI().PreventEngineHideGameUI();
	}
	SetDeleteSelfOnClose( true );

	SetProportional( true );

	SetUpperGarnishEnabled( true );
	SetLowerGarnishEnabled( true );

	m_pHeaderFooter = new CNB_Header_Footer( this, "HeaderFooter" );
	m_pHeaderFooter->SetTitle( L"" );
	m_pHeaderFooter->SetHeaderEnabled( false );
	m_pHeaderFooter->SetFooterEnabled( true );
	m_pHeaderFooter->SetGradientBarEnabled( true );
	m_pHeaderFooter->SetGradientBarPos( 80, 340 );

	m_pSectionName = new BaseModHybridButton( this, "SectionName", L"" );

	m_iCurrentSection = 0;
	m_pFlyoutButton = NULL;

	LoadSettingDefinitions();
	CreateSettingControls();
}

AdvancedSettings::~AdvancedSettings()
{
	GameUI().AllowEngineHideGameUI();
}

void AdvancedSettings::PerformLayout()
{
	int x = ScreenWidth() / 2 - scheme()->GetProportionalScaledValue( 180 );
	int y = scheme()->GetProportionalScaledValue( 150 );

	vgui::Panel *pPrevSetting = m_pSectionName;

	FOR_EACH_VEC( m_SectionDefs, i )
	{
		Section_t *pSectionDef = m_SectionDefs[i];

		if ( i == m_iCurrentSection )
		{
			m_pSectionName->SetText( pSectionDef->m_Name );
		}

		FOR_EACH_VEC( pSectionDef->m_Settings, j )
		{
			Setting_t *pSettingDef = pSectionDef->m_Settings[j];
			if ( pSettingDef->m_Type == SETTING_INVALID || !pSettingDef->m_ConVar.IsValid() )
			{
				continue;
			}

			Assert( pSettingDef->m_Type == SETTING_SELECT );

			const char *szConVar = pSettingDef->m_ConVar.GetName();
			const char *pConVarValue = pSettingDef->m_ConVar.GetString();
			const char *pConVarDefValue = pSettingDef->m_ConVar.GetDefault();

			DropDownMenu *pSetting = assert_cast<DropDownMenu *>( FindChildByName( VarArgs( "Drp%s", szConVar ) ) );
			Assert( pSetting );
			FlyoutMenu *pOptions = assert_cast<FlyoutMenu *>( FindChildByName( VarArgs( "Flm%s", szConVar ) ) );
			Assert( pOptions );

			if ( !pSetting || !pOptions )
			{
				continue;
			}

			bool bHidden = false;

			if ( i != m_iCurrentSection )
			{
				bHidden = true;
			}
			else
			{
				FOR_EACH_VEC( pSettingDef->m_HideUnless, k )
				{
					if ( !pSettingDef->m_HideUnless[k]->IsValid() )
					{
						continue;
					}

					const char *szHideUnless = pSettingDef->m_HideUnless[k]->GetString();
					if ( !V_strcmp( szHideUnless, "" ) || !V_strcmp( szHideUnless, "0" ) )
					{
						bHidden = true;
						break;
					}
				}
			}

			if ( bHidden )
			{
				pSetting->SetVisible( false );
				continue;
			}

			pSetting->SetVisible( true );
			pSetting->SetPos( x, y );
			y += scheme()->GetProportionalScaledValue( 20 );

			int iDefault = -1;

			FOR_EACH_VEC( pSettingDef->m_Options, k )
			{
				if ( !V_strcmp( pSettingDef->m_Options[k].m_Value, pConVarValue ) )
				{
					char szName[256];
					V_UnicodeToUTF8( pSettingDef->m_Options[k].m_Name, szName, sizeof( szName ) );
					pSetting->GetButton()->SetDropdownSelection( szName );
					pOptions->SetInitialSelection( VarArgs( "Btn%d", k ) );
					iDefault = -1;
					break;
				}

				if ( !V_strcmp( pSettingDef->m_Options[k].m_Value, pConVarDefValue ) )
				{
					iDefault = k;
				}
			}

			if ( iDefault != -1 )
			{
				pOptions->SetInitialSelection( VarArgs( "Btn%d", iDefault ) );
			}

			pPrevSetting->SetNavDown( pSetting );
			pSetting->SetNavUp( pPrevSetting );
			pSetting->SetNavDown( m_pSectionName );
			SetNavUp( pSetting );
			if ( j == 0 )
				SetNavDown( pSetting );
			pPrevSetting = pSetting;
		}
	}

	BaseClass::PerformLayout();

	SetBounds( 0, 0, ScreenWidth(), ScreenHeight() );
}

void AdvancedSettings::LoadSettingDefinitions()
{
	KeyValues::AutoDelete pKV( "AdvancedSettings" );
	if ( !pKV->LoadFromFile( g_pFullFileSystem, "resource/rd_advanced_settings.txt", "GAME" ) )
	{
		Warning( "Could not load advanced settings list!\n" );
		return;
	}

	if ( pKV->GetFirstValue() )
	{
		Warning( "Advanced settings list contains non-SECTION key: %s\n", pKV->GetFirstValue()->GetName() );
	}

	FOR_EACH_TRUE_SUBKEY( pKV, pKVSection )
	{
		if ( V_strcmp( pKVSection->GetName(), "SECTION" ) )
		{
			Warning( "Advanced settings list contains non-SECTION key: %s\n", pKVSection->GetName() );
			continue;
		}

		Section_t *pSection = new Section_t();
		pSection->m_Name = g_pVGuiLocalize->FindSafe( pKVSection->GetString( "title" ) );
		m_SectionDefs.AddToTail( pSection );

		FOR_EACH_TRUE_SUBKEY( pKVSection, pKVSetting )
		{
			const char *szCVarName = pKVSetting->GetString( "cvar" );
			Setting_t *pSetting = new Setting_t( szCVarName );
			pSection->m_Settings.AddToTail( pSetting );

			const char *szTypeName = pKVSetting->GetName();
			if ( !V_strcmp( szTypeName, "SELECT" ) )
			{
				pSetting->m_Type = SETTING_SELECT;
			}
			else if ( !V_strcmp( szTypeName, "TOGGLE" ) )
			{
				if ( pKVSetting->FindKey( "options" ) )
				{
					Warning( "Advanced settings TOGGLE with options: %s\n", szCVarName );
				}

				// TODO: should we add checkboxes instead of making this yet another dropdown?
				pSetting->m_Type = SETTING_SELECT;
				pKVSetting->AddSubKey( new KeyValues( "options", "0", "#RD_AdvancedSettings_Toggle_Off", "1", "#RD_AdvancedSettings_Toggle_On" ) );
			}
			else
			{
				Warning( "Advanced setting contains unknown type %s for %s\n", szTypeName, szCVarName );
				pSetting->m_Type = SETTING_INVALID;
			}

			pSetting->m_Name = g_pVGuiLocalize->FindSafe( pKVSetting->GetString( "title" ) );

			switch ( pSetting->m_Type )
			{
			case SETTING_SELECT:
				if ( KeyValues *pOptions = pKVSetting->FindKey( "options" ) )
				{
					FOR_EACH_VALUE( pOptions, pOption )
					{
						int iOption = pSetting->m_Options.AddToTail();
						V_strncpy( pSetting->m_Options[iOption].m_Value, pOption->GetName(), sizeof( pSetting->m_Options[iOption].m_Value ) );
						pSetting->m_Options[iOption].m_Name = g_pVGuiLocalize->FindSafe( pOption->GetString() );
					}
				}
				else
				{
					Warning( "Advanced settings SELECT with no options: %s\n", szCVarName );
				}
				break;
			default:
				break;
			}

			FOR_EACH_VALUE( pKVSetting, pValue )
			{
				if ( FStrEq( pValue->GetName(), "hide_unless" ) )
				{
					pSetting->m_HideUnless.AddToTail( new ConVarRef( pValue->GetString() ) );
				}
			}
		}
	}
}

void AdvancedSettings::CreateSettingControls()
{
	KeyValues::AutoDelete kvButtonSettings( "BtnDropButton" );
	kvButtonSettings->SetString( "wide", "360" );
	kvButtonSettings->SetString( "wideatopen", "260" );
	kvButtonSettings->SetString( "tall", "15" );
	kvButtonSettings->SetString( "autoResize", "1" );
	kvButtonSettings->SetString( "style", "DropDownButton" );
	kvButtonSettings->SetString( "ActivationType", "1" );

	KeyValues::AutoDelete kvOptionSettings( "Btn" );
	kvOptionSettings->SetString( "wide", "150" );
	kvOptionSettings->SetString( "tall", "20" );
	kvOptionSettings->SetString( "autoResize", "1" );
	kvOptionSettings->SetString( "wrap", "1" );
	kvOptionSettings->SetString( "style", "FlyoutMenuButton" );

	FOR_EACH_VEC( m_SectionDefs, i )
	{
		Section_t *pSectionDef = m_SectionDefs[i];

		FOR_EACH_VEC( pSectionDef->m_Settings, j )
		{
			Setting_t *pSettingDef = pSectionDef->m_Settings[j];
			if ( pSettingDef->m_Type == SETTING_INVALID || !pSettingDef->m_ConVar.IsValid() )
			{
				continue;
			}

			Assert( pSettingDef->m_Type == SETTING_SELECT );

			const char *szConVar = pSettingDef->m_ConVar.GetName();

			DropDownMenu *pSetting = new DropDownMenu( this, VarArgs( "Drp%s", szConVar ) );
			pSetting->SetZPos( 3 );
			pSetting->SetSize( scheme()->GetProportionalScaledValue( 360 ), scheme()->GetProportionalScaledValue( 15 ) );

			BaseModHybridButton *pButton = new BaseModHybridButton( pSetting, "BtnDropButton", pSettingDef->m_Name );
			pButton->SetCommand( VarArgs( "Flm%s", szConVar ) );
			pButton->ApplySettings( kvButtonSettings );

			FlyoutMenu *pOptions = new FlyoutMenu( this, VarArgs( "Flm%s", szConVar ) );
			pSetting->SetFlyout( VarArgs( "Flm%s", szConVar ) );
			pOptions->SetZPos( 4 );
			pOptions->SetListener( this );

			Panel *pBackground = new Panel( pOptions, "PnlBackground" );
			pBackground->SetZPos( -1 );
			pBackground->SetSize( scheme()->GetProportionalScaledValue( 156 ), scheme()->GetProportionalScaledValue( 20 * pSettingDef->m_Options.Count() + 5 ) );
			pBackground->SetPaintBackgroundEnabled( true );
			pBackground->SetPaintBorderEnabled( true );

			const char *pConVarValue = pSettingDef->m_ConVar.GetString();
			char szDropdownSelection[256];
			V_strncpy( szDropdownSelection, pConVarValue, sizeof( szDropdownSelection ) );

			BaseModHybridButton *pFirstButton = NULL, *pLastButton = NULL;
			FOR_EACH_VEC( pSettingDef->m_Options, k )
			{
				BaseModHybridButton *pOption = new BaseModHybridButton( pOptions, VarArgs( "Btn%d", k ), pSettingDef->m_Options[k].m_Name );
				pOption->ApplySettings( kvOptionSettings );
				pOption->SetPos( 0, scheme()->GetProportionalScaledValue( 20 * k ) );
				pOption->SetCommand( VarArgs( "RunCmd%s %s", szConVar, pSettingDef->m_Options[k].m_Value ) );
				pOption->AddActionSignalTarget( this );

				if ( !V_strcmp( pConVarValue, pSettingDef->m_Options[k].m_Value ) )
				{
					V_UnicodeToUTF8( pSettingDef->m_Options[k].m_Name, szDropdownSelection, sizeof( szDropdownSelection ) );
				}

				if ( !pFirstButton )
				{
					pFirstButton = pOption;
				}

				if ( pLastButton )
				{
					pLastButton->SetNavDown( pOption );
					pOption->SetNavUp( pLastButton );
					pOption->PinToSibling( VarArgs( "Btn%d", k - 1 ), PIN_TOPLEFT, PIN_BOTTOMLEFT );
				}

				pLastButton = pOption;
			}

			pButton->SetDropdownSelection( szDropdownSelection );

			if ( pFirstButton && pLastButton )
			{
				pFirstButton->SetNavUp( pLastButton );
				pLastButton->SetNavDown( pFirstButton );
			}

			pOptions->SetVisible( false );
		}
	}
}

void AdvancedSettings::OnCommand( const char *command )
{
	if ( const char *szConCommand = StringAfterPrefix( command, "RunCmd" ) )
	{
		engine->ExecuteClientCmd( szConCommand );
		engine->ExecuteClientCmd( "host_writeconfig" );
		InvalidateLayout();
	}
	else if ( !V_strcmp( command, "Back" ) )
	{
		OnKeyCodePressed( ButtonCodeToJoystickButtonCode( KEY_XBUTTON_B, CBaseModPanel::GetSingleton().GetLastActiveUserId() ) );
	}
	else if ( !V_strcmp( command, "NextSection" ) )
	{
		m_iCurrentSection++;
		if ( m_iCurrentSection >= m_SectionDefs.Count() )
		{
			m_iCurrentSection = 0;
		}

		FlyoutMenu::CloseActiveMenu();

		InvalidateLayout();
	}
	else
	{
		BaseClass::OnCommand( command );
	}
}

//=============================================================================
void AdvancedSettings::OnThink()
{
	if ( FlyoutMenu::GetActiveMenu() && !HasFocus() )
		RequestFocus();

	BaseClass::OnThink();
}

//=============================================================================
void AdvancedSettings::OnKeyCodePressed( KeyCode code )
{
	FlyoutMenu *pFlyout = FlyoutMenu::GetActiveMenu();
	if ( pFlyout && m_pFlyoutButton )
	{
		switch( GetBaseButtonCode( code ) )
		{
		case KEY_XSTICK1_DOWN:
		case KEY_XSTICK2_DOWN:
		case KEY_XBUTTON_DOWN:
		case KEY_DOWN:
			m_pFlyoutButton->NavigateDown();
			break;
		case KEY_XSTICK1_UP:
		case KEY_XSTICK2_UP:
		case KEY_XBUTTON_UP:
		case KEY_UP:
			m_pFlyoutButton->NavigateUp();
			break;
		case KEY_ENTER:
		case KEY_SPACE:
		case KEY_XBUTTON_A:
			m_pFlyoutButton->DoClick();
			break;
		case KEY_XBUTTON_B:
			pFlyout->CloseMenu( pFlyout->GetNavFrom() );
			break;

		default:
			BaseClass::OnKeyCodePressed( code );
			break;
		}
	}
	else
		BaseClass::OnKeyCodePressed( code );
}

void AdvancedSettings::OnKeyCodeTyped( KeyCode code )
{
	if ( FlyoutMenu::GetActiveMenu() && code == KEY_SPACE )
		return;
	else
		BaseClass::OnKeyCodeTyped( code );
}

void AdvancedSettings::OnNotifyChildFocus( vgui::Panel* child )
{
	if ( !child )
		return;

	m_pFlyoutButton = dynamic_cast<vgui::Button *>( child );
}

void AdvancedSettings::OnFlyoutMenuClose( vgui::Panel* flyTo )
{
	m_pFlyoutButton = NULL;
}

void AdvancedSettings::OnFlyoutMenuCancelled()
{

}
