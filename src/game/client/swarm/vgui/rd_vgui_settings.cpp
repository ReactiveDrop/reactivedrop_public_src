#include "cbase.h"
#include "rd_vgui_settings.h"
#include "gameui/swarm/vhybridbutton.h"
#include "gameui/swarm/basemodpanel.h"
#include <vgui/IInput.h>
#include <vgui/ISurface.h>
#include <vgui_controls/TextEntry.h>
#include "asw_util_shared.h"
#include "nb_header_footer.h"
#include "rd_vgui_main_menu_top_bar.h"
#include "rd_hud_sheet.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace BaseModUI;

DECLARE_BUILD_FACTORY( CRD_VGUI_Settings );
DECLARE_BUILD_FACTORY_DEFAULT_TEXT( CRD_VGUI_Option, OptionNameMissing );

ConVar rd_settings_last_tab( "rd_settings_last_tab", "controls", FCVAR_ARCHIVE, "Remembers the last-used tab on the settings window." );
ConVar rd_option_slider_recommended_magnetism( "rd_option_slider_recommended_magnetism", "3", FCVAR_NONE, "How sticky the recommended value for a slider option is." );
ConVar rd_option_radio_repeat_speed( "rd_option_radio_repeat_speed", "0.75", FCVAR_NONE, "Time between left/right key repeats on radio button settings." );
ConVar rd_option_radio_repeat_accel( "rd_option_radio_repeat_accel", "0.5", FCVAR_NONE, "For each repeat, subtract this amount from the time between future repeats." );
ConVar rd_option_radio_repeat_max_accel( "rd_option_radio_repeat_max_accel", "1", FCVAR_NONE, "Limit to the number of times rd_option_radio_repeat_accel can be applied." );
ConVar rd_option_dropdown_repeat_speed( "rd_option_dropdown_repeat_speed", "0.75", FCVAR_NONE, "Time between up/down key repeats on dropdown settings." );
ConVar rd_option_dropdown_repeat_accel( "rd_option_dropdown_repeat_accel", "0.5", FCVAR_NONE, "For each repeat, subtract this amount from the time between future repeats." );
ConVar rd_option_dropdown_repeat_max_accel( "rd_option_dropdown_repeat_max_accel", "1", FCVAR_NONE, "Limit to the number of times rd_option_dropdown_repeat_accel can be applied." );
ConVar rd_option_slider_repeat_speed( "rd_option_slider_repeat_speed", "0.5", FCVAR_NONE, "Time between left/right key repeats on slider settings." );
ConVar rd_option_slider_repeat_accel( "rd_option_slider_repeat_accel", "0.0625", FCVAR_NONE, "For each repeat, subtract this amount from the time between future repeats." );
ConVar rd_option_slider_repeat_max_accel( "rd_option_slider_repeat_max_accel", "7", FCVAR_NONE, "Limit to the number of times rd_option_slider_repeat_accel can be applied." );

bool CRD_VGUI_Settings::s_bWantSave = false;

static CUtlVector<CRD_VGUI_Option *> s_OptionControls;
static bool s_bSuppressTextEntryChange = false;

static void SettingsConVarChangedCallback( IConVar *var, const char *pOldValue, float flOldValue )
{
	if ( s_bSuppressTextEntryChange )
		return;

	Assert( s_OptionControls.Count() );
	FOR_EACH_VEC( s_OptionControls, i )
	{
		s_OptionControls[i]->InvalidateLayout();
	}
}

CRD_VGUI_Settings::CRD_VGUI_Settings( vgui::Panel *parent, const char *panelName ) :
	BaseClass( parent, panelName )
{
	SetProportional( true );

	m_pHeaderFooter = new CNB_Header_Footer( this, "HeaderFooter" );
	m_pHeaderFooter->SetTitle( "" );
	m_pHeaderFooter->SetHeaderEnabled( false );
	m_pHeaderFooter->SetFooterEnabled( false );
	m_pTopBar = new CRD_VGUI_Main_Menu_Top_Bar( this, "TopBar" );
	m_pTopBar->m_hActiveButton = m_pTopBar->m_pBtnSettings;

	m_pBtnControls = new BaseModHybridButton( this, "BtnControls", "#rd_settings_controls", this, "Controls" );
	m_pBtnOptions = new BaseModHybridButton( this, "BtnOptions", "#rd_settings_options", this, "Options" );
	m_pBtnAudio = new BaseModHybridButton( this, "BtnAudio", "#rd_settings_audio", this, "Audio" );
	m_pBtnVideo = new BaseModHybridButton( this, "BtnVideo", "#rd_settings_video", this, "Video" );
	m_pBtnAbout = new BaseModHybridButton( this, "BtnAbout", "#rd_settings_about", this, "About" );

	m_pPnlControls = new CRD_VGUI_Settings_Controls( this, "PnlControls" );
	m_pPnlOptions = new CRD_VGUI_Settings_Options( this, "PnlOptions" );
	m_pPnlAudio = new CRD_VGUI_Settings_Audio( this, "PnlAudio" );
	m_pPnlVideo = new CRD_VGUI_Settings_Video( this, "PnlVideo" );
	m_pPnlAbout = new CRD_VGUI_Settings_About( this, "PnlAbout" );

	SetTitle( "", false );
	SetDeleteSelfOnClose( true );
	SetLowerGarnishEnabled( false );
	SetMoveable( false );
}

CRD_VGUI_Settings::~CRD_VGUI_Settings()
{
	CRD_VGUI_Option::WriteConfig( s_bWantSave );
	s_bWantSave = false;
}

void CRD_VGUI_Settings::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	g_RD_HUD_Sheets.VidInit();

	// I don't know why these aren't working from the .res file.
	m_pBtnControls->SetNavUp( m_pTopBar );
	m_pBtnOptions->SetNavUp( m_pTopBar );
	m_pBtnAudio->SetNavUp( m_pTopBar );
	m_pBtnVideo->SetNavUp( m_pTopBar );
	m_pBtnAbout->SetNavUp( m_pTopBar );

	if ( !V_stricmp( rd_settings_last_tab.GetString(), "controls" ) )
	{
		NavigateToTab( m_pBtnControls, m_pPnlControls, NULL );
	}
	else if ( !V_stricmp( rd_settings_last_tab.GetString(), "options" ) )
	{
		NavigateToTab( m_pBtnOptions, m_pPnlOptions, NULL );
	}
	else if ( !V_stricmp( rd_settings_last_tab.GetString(), "audio" ) )
	{
		NavigateToTab( m_pBtnAudio, m_pPnlAudio, NULL );
	}
	else if ( !V_stricmp( rd_settings_last_tab.GetString(), "video" ) )
	{
		NavigateToTab( m_pBtnVideo, m_pPnlVideo, NULL );
	}
	else
	{
		NavigateToTab( m_pBtnAbout, m_pPnlAbout, NULL );
	}
}

void CRD_VGUI_Settings::OnCommand( const char *command )
{
	if ( !V_stricmp( command, "Controls" ) )
	{
		NavigateToTab( m_pBtnControls, m_pPnlControls, "controls" );
	}
	else if ( !V_stricmp( command, "Options" ) )
	{
		NavigateToTab( m_pBtnOptions, m_pPnlOptions, "options" );
	}
	else if ( !V_stricmp( command, "Audio" ) )
	{
		NavigateToTab( m_pBtnAudio, m_pPnlAudio, "audio" );
	}
	else if ( !V_stricmp( command, "Video" ) )
	{
		NavigateToTab( m_pBtnVideo, m_pPnlVideo, "video" );
	}
	else if ( !V_stricmp( command, "About" ) )
	{
		NavigateToTab( m_pBtnAbout, m_pPnlAbout, "about" );
	}
	else
	{
		BaseClass::OnCommand( command );
	}
}

void CRD_VGUI_Settings::NavigateToTab( BaseModHybridButton *pButton, CRD_VGUI_Settings_Panel_Base *pPanel, const char *szTabID )
{
	pButton->SetOpen();
	m_pPnlControls->SetVisible( m_pPnlControls == pPanel );
	m_pPnlOptions->SetVisible( m_pPnlOptions == pPanel );
	m_pPnlAudio->SetVisible( m_pPnlAudio == pPanel );
	m_pPnlVideo->SetVisible( m_pPnlVideo == pPanel );
	m_pPnlAbout->SetVisible( m_pPnlAbout == pPanel );
	m_pBtnControls->SetNavDown( pPanel );
	m_pBtnOptions->SetNavDown( pPanel );
	m_pBtnAudio->SetNavDown( pPanel );
	m_pBtnVideo->SetNavDown( pPanel );
	m_pBtnAbout->SetNavDown( pPanel );
	NavigateToChild( pPanel );
	pPanel->Activate();
	pPanel->InvalidateLayout();

	if ( szTabID )
	{
		rd_settings_last_tab.SetValue( szTabID );
		s_bWantSave = true;
	}
}

CRD_VGUI_Settings_Panel_Base::CRD_VGUI_Settings_Panel_Base( vgui::Panel *parent, const char *panelName ) :
	BaseClass( parent, panelName )
{
	SetConsoleStylePanel( true );
}

void CRD_VGUI_Settings_Panel_Base::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	char szResourceFile[MAX_PATH];
	V_snprintf( szResourceFile, sizeof( szResourceFile ), "resource/ui/basemodui/%s.res", GetClassName() );
	LoadControlSettings( szResourceFile, "GAME" );
}

void CRD_VGUI_Settings_Panel_Base::NavigateTo()
{
	// navigate twice so we end up on a button
	if ( GetLastNavDirection() == ND_DOWN && NavigateDown() )
	{
		return;
	}

	if ( GetLastNavDirection() == ND_UP && NavigateUp() )
	{
		return;
	}

	BaseClass::NavigateTo();
}

class CRD_VGUI_Option_Dropdown : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Option_Dropdown, vgui::EditablePanel );
public:
	CRD_VGUI_Option_Dropdown( CRD_VGUI_Option *parent, const char *panelName ) :
		BaseClass{ parent, panelName }
	{
		m_hFont = vgui::INVALID_FONT;
		MakePopup( false );
	}

	void PerformLayout() override
	{
		BaseClass::PerformLayout();

		CRD_VGUI_Option *parent = assert_cast< CRD_VGUI_Option * >( GetParent() );

		m_hFont = parent->m_pLblFieldName->GetFont();
		SetFgColor( parent->m_pInteractiveArea->GetFgColor() );
		SetBgColor( parent->m_pInteractiveArea->GetBgColor() );
		SetPaintBackgroundEnabled( true );

		int iMaxWidth = parent->m_pInteractiveArea->GetWide() - vgui::Label::Content * 2;
		FOR_EACH_VEC( parent->m_Options, i )
		{
			iMaxWidth = MAX( iMaxWidth, parent->m_Options[i]->m_iWidth );
		}

		int x = 0, y = 0;
		parent->m_pInteractiveArea->LocalToScreen( x, y );
		SetPos( x, y );

		SetWide( vgui::Label::Content * 2 + iMaxWidth );
		SetTall( vgui::Label::Content + parent->m_Options.Count() * vgui::surface()->GetFontTall( m_hFont ) );

		vgui::input()->GetCursorPos( x, y );
		ScreenToLocal( x, y );
		OnCursorMoved( x, y );
	}

	void Paint() override
	{
		CRD_VGUI_Option *parent = assert_cast< CRD_VGUI_Option * >( GetParent() );
		Assert( parent && parent->m_iActiveOption != -1 );
		if ( !parent || parent->m_iActiveOption == -1 )
		{
			SetVisible( false );
			return;
		}

		BaseClass::Paint();

		int x = vgui::Label::Content;
		int y = vgui::Label::Content / 2;
		int iLineHeight = vgui::surface()->GetFontTall( m_hFont );

		vgui::surface()->DrawSetTextFont( m_hFont );

		FOR_EACH_VEC( parent->m_Options, i )
		{
			if ( parent->m_Options[i]->m_iFlags & CRD_VGUI_Option::FLAG_HIDDEN )
			{
				y += iLineHeight;
				continue;
			}

			const wchar_t *wszLabel = parent->m_Options[i]->m_wszLabel;
			bool bRecommended = parent->m_bHaveRecommended && i == parent->m_Recommended.m_iOption;
			bool bCurrent = parent->m_bHaveCurrent && i == parent->m_Current.m_iOption;
			bool bActive = parent->m_iActiveOption == i;

			if ( bActive )
			{
				vgui::surface()->DrawSetColor( 255, 255, 255, 32 );
				vgui::surface()->DrawFilledRect( 0, y, GetWide(), y + iLineHeight );
			}

			vgui::surface()->DrawSetTextPos( x, y );
			if ( parent->m_Options[i]->m_iFlags & CRD_VGUI_Option::FLAG_DISABLED )
				vgui::surface()->DrawSetTextColor( 64, 64, 64, 255 );
			else if ( bRecommended )
				vgui::surface()->DrawSetTextColor( 192, 224, 255, 255 );
			else if ( bCurrent )
				vgui::surface()->DrawSetTextColor( 255, 255, 255, 255 );
			else
				vgui::surface()->DrawSetTextColor( GetFgColor() );
			vgui::surface()->DrawUnicodeString( wszLabel );

			y += iLineHeight;
		}
	}

	void OnMousePressed( vgui::MouseCode code ) override
	{
		// forward mouse presses to parent; don't process here
		GetParent()->OnMousePressed( code );
	}

	void OnMouseReleased( vgui::MouseCode code ) override
	{
		GetParent()->OnMouseReleased( code );
	}

	void OnCursorMoved( int x, int y ) override
	{
		BaseClass::OnCursorMoved( x, y );

		CRD_VGUI_Option *parent = assert_cast< CRD_VGUI_Option * >( GetParent() );
		Assert( parent );
		if ( !parent )
			return;

		int iLineHeight = vgui::surface()->GetFontTall( m_hFont );
		int iOption = ( y - vgui::Label::Content / 2 ) / iLineHeight;
		if ( iOption >= 0 && iOption < parent->m_Options.Count() && parent->m_iActiveOption != iOption && !( parent->m_Options[iOption]->m_iFlags & CRD_VGUI_Option::FLAG_DISABLED ) )
		{
			parent->m_iActiveOption = iOption;
			Repaint();

			CBaseModPanel::GetSingleton().PlayUISound( UISOUND_FOCUS );
		}
	}

	vgui::HFont m_hFont;
};

CRD_VGUI_Option::CRD_VGUI_Option( vgui::Panel *parent, const char *panelName, const char *szLabel, Mode_t eMode ) :
	BaseClass{ parent, panelName },
	m_eMode{ eMode }
{
	SetConsoleStylePanel( true );

	m_pInteractiveArea = new vgui::Panel( this, "InteractiveArea" );
	m_pInteractiveArea->SetMouseInputEnabled( false );
	m_pLblFieldName = new vgui::Label( this, "LblFieldName", szLabel );
	m_pLblFieldName->SetMouseInputEnabled( false );
	m_pLblHint = new vgui::Label( this, "LblHint", "" );
	m_pLblHint->SetMouseInputEnabled( false );
	m_pTextEntry = eMode == MODE_SLIDER ? new vgui::TextEntry( this, "TextEntry" ) : NULL;
	if ( m_pTextEntry )
	{
		m_pTextEntry->AddActionSignalTarget( this );
		m_pTextEntry->SetAllowNumericInputOnly( true );
	}
	m_pDropdown = NULL;

	m_bHaveCurrent = false;
	m_bHaveRecommended = false;
	m_bSetUsingConVars = false;
	m_bReverseSlider = false;
	m_bSliderActive = false;
	m_bSliderActiveMouse = false;
	m_bStartedSliderActiveAtRecommended = false;
	m_iActiveOption = -1;

	if ( !s_OptionControls.Count() )
		g_pCVar->InstallGlobalChangeCallback( &SettingsConVarChangedCallback );

	s_OptionControls.AddToTail( this );
}

CRD_VGUI_Option::~CRD_VGUI_Option()
{
	bool bRemoved = s_OptionControls.FindAndFastRemove( this );
	Assert( bRemoved );

	if ( bRemoved && s_OptionControls.Count() == 0 )
		g_pCVar->RemoveGlobalChangeCallback( &SettingsConVarChangedCallback );
}

void CRD_VGUI_Option::OnCursorEntered()
{
	BaseClass::OnCursorEntered();

	if ( GetParent() )
		NavigateToChild( this );
}

void CRD_VGUI_Option::OnCursorMoved( int x, int y )
{
	BaseClass::OnCursorMoved( x, y );

	int x0, y0, x1, y1;
	m_pInteractiveArea->GetBounds( x0, y0, x1, y1 );

	if ( m_eMode == MODE_SLIDER && m_bSliderActiveMouse )
	{
		float flMin = m_flMinValue, flMax = m_flMaxValue;
		if ( m_bReverseSlider )
			V_swap( flMin, flMax );

		float flNewValue = RemapValClamped( x, x0 + y1 / 2, x0 + x1 - y1 / 2, flMin, flMax );
		if ( !m_bStartedSliderActiveAtRecommended && m_bHaveRecommended && fabsf( x - RemapValClamped( m_Recommended.m_flValue, flMin, flMax, x0 + y1 / 2, x0 + x1 - y1 / 2 ) ) < YRES( rd_option_slider_recommended_magnetism.GetFloat() ) )
			flNewValue = m_Recommended.m_flValue;

		SetCurrentSliderValue( flNewValue );
		if ( m_pSliderLink )
		{
			m_pSliderLink->SetValue( flNewValue );
			s_bCVarChanged = true;
		}

		CBaseModPanel::GetSingleton().PlayUISound( UISOUND_FOCUS );

		return;
	}

	if ( m_pInteractiveArea->IsCursorOver() )
	{
		if ( m_eMode == MODE_RADIO )
		{
			x0 -= vgui::Label::Content;

			int iNewActive = -1;

			FOR_EACH_VEC( m_Options, i )
			{
				if ( x0 > x )
					break;

				iNewActive = i;

				x0 += y1 + vgui::Label::Content * 2 + m_Options[i]->m_iWidth;
			}

			if ( m_iActiveOption != iNewActive && !( m_Options[iNewActive]->m_iFlags & FLAG_DISABLED ) )
				ChangeActiveRadioButton( iNewActive );
		}
	}
}

void CRD_VGUI_Option::NavigateTo()
{
	BaseClass::NavigateTo();

	if ( m_eMode == MODE_RADIO )
	{
		if ( m_bHaveCurrent && m_Current.m_iOption >= 0 && m_Current.m_iOption < m_Options.Count() )
			m_iActiveOption = m_Current.m_iOption;
		else if ( m_bHaveRecommended )
			m_iActiveOption = m_Recommended.m_iOption;
		else
			m_iActiveOption = 0;
	}

	if ( m_eMode == MODE_SLIDER )
	{
		int iCurrent = -1, iRecommended = -1;
		FOR_EACH_VEC( m_Options, i )
		{
			if ( m_bHaveCurrent && m_Options[i]->m_iValue == m_Current.m_flValue )
				iCurrent = i;
			if ( m_bHaveRecommended && m_Options[i]->m_iValue == m_Recommended.m_flValue )
				iRecommended = i;
			if ( iCurrent != -1 && iRecommended != -1 )
				break;
		}

		if ( iCurrent != -1 )
			m_iActiveOption = iCurrent;
		else if ( m_bHaveCurrent && m_Current.m_flValue >= m_flMinValue && m_Current.m_flValue <= m_flMaxValue )
			m_iActiveOption = m_Options.Count();
		else if ( iRecommended != -1 )
			m_iActiveOption = iRecommended;
		else
			m_iActiveOption = m_Options.Count();
	}

	RequestFocus();

	CBaseModPanel::GetSingleton().PlayUISound( UISOUND_FOCUS );
}

void CRD_VGUI_Option::OnKillFocus()
{
	vgui::Panel *pNewFocus = vgui::ipanel()->GetPanel( vgui::input()->GetCalculatedFocus(), GetModuleName() );
	if ( m_pDropdown && pNewFocus == m_pDropdown )
	{
		RequestFocus();
		return;
	}

	BaseClass::OnKillFocus();

	if ( m_bSliderActive )
	{
		ToggleSliderActive( false );
	}

	if ( m_pDropdown )
	{
		m_pDropdown->SetVisible( false );
	}

	m_iActiveOption = -1;
}

void CRD_VGUI_Option::ApplySettings( KeyValues *pSettings )
{
	BaseClass::ApplySettings( pSettings );

	const char *szResFile = pSettings->GetString( "ResourceFile", NULL );
	if ( szResFile )
	{
		LoadControlSettings( szResFile );
	}
}

void CRD_VGUI_Option::PerformLayout()
{
	BaseClass::PerformLayout();

	if ( m_bSetUsingConVars )
		SetCurrentUsingConVars();

	vgui::HFont hFont = m_pLblFieldName->GetFont();
	int iTotalWidth = 0;
	FOR_EACH_VEC( m_Options, i )
	{
		int discard;
		vgui::surface()->GetTextSize( hFont, m_Options[i]->m_wszLabel, m_Options[i]->m_iWidth, discard );
		iTotalWidth += m_Options[i]->m_iWidth;
	}

	if ( m_eMode == MODE_RADIO && m_Options.Count() > 1 )
	{
		int iAvailableWidth = m_pInteractiveArea->GetWide() - ( m_pInteractiveArea->GetTall() + vgui::Label::Content * 2 ) * m_Options.Count();
		int iRemainingWidth = iAvailableWidth - iTotalWidth;
		int iAddWidth = iRemainingWidth / ( m_Options.Count() - 1 );
		FOR_EACH_VEC( m_Options, i )
		{
			if ( i != m_Options.Count() - 1 )
				m_Options[i]->m_iWidth += iAddWidth;
		}
	}

	if ( m_eMode == MODE_RADIO || m_eMode == MODE_DROPDOWN )
	{
		Assert( !m_bHaveCurrent || ( m_Current.m_iOption >= 0 && m_Current.m_iOption < m_Options.Count() ) );
		if ( m_bHaveCurrent && m_Current.m_iOption >= 0 && m_Current.m_iOption < m_Options.Count() )
		{
			m_pLblHint->SetText( m_Options[m_Current.m_iOption]->m_wszHint );
		}
		else
		{
			m_pLblHint->SetText( m_wszDefaultHint );
		}
	}
	else if ( m_eMode == MODE_SLIDER )
	{
		bool bFound = false;
		if ( m_bHaveCurrent )
		{
			FOR_EACH_VEC( m_Options, i )
			{
				if ( m_Options[i]->m_iValue == m_Current.m_flValue )
				{
					m_pLblHint->SetText( m_Options[i]->m_wszHint );
					bFound = true;
					break;
				}
			}
		}

		if ( !bFound )
		{
			m_pLblHint->SetText( m_wszDefaultHint );
		}
	}
	else
	{
		m_pLblHint->SetText( m_wszDefaultHint );
	}
}

void CRD_VGUI_Option::OnThink()
{
	BaseClass::OnThink();

	if ( !HasFocus() )
		return;

	bool bHorizontal = false, bVertical = false;
	float flSpeed = 0.0f, flAccel = 0.0f;
	int iMaxAccel = 0;

	switch ( m_eMode )
	{
	case MODE_RADIO:
		bHorizontal = true;
		flSpeed = rd_option_radio_repeat_speed.GetFloat();
		flAccel = rd_option_radio_repeat_accel.GetFloat();
		iMaxAccel = rd_option_radio_repeat_max_accel.GetInt();
		break;
	case MODE_DROPDOWN:
		if ( m_bSliderActive )
		{
			bVertical = true;
			flSpeed = rd_option_dropdown_repeat_speed.GetFloat();
			flAccel = rd_option_dropdown_repeat_accel.GetFloat();
			iMaxAccel = rd_option_dropdown_repeat_max_accel.GetInt();
		}
		break;
	case MODE_SLIDER:
		if ( m_bSliderActive )
		{
			bHorizontal = true;
			flSpeed = rd_option_slider_repeat_speed.GetFloat();
			flAccel = rd_option_slider_repeat_accel.GetFloat();
			iMaxAccel = rd_option_slider_repeat_max_accel.GetInt();
		}
		else
		{
			bHorizontal = true;
			flSpeed = rd_option_radio_repeat_speed.GetFloat();
			flAccel = rd_option_radio_repeat_accel.GetFloat();
			iMaxAccel = rd_option_radio_repeat_max_accel.GetInt();
		}
		break;
	case MODE_COLOR:
		if ( m_bSliderActive )
		{
			Assert( !"TODO: button repeat for color" );
		}
		break;
	default:
		Assert( m_eMode == MODE_CHECKBOX || m_eMode == MODE_CUSTOM );
		break;
	}

#define CHECK_BUTTON_REPEAT( DIR, arg1, arg2 ) \
	if ( vgui::input()->IsKeyDown( KEY_##DIR ) || \
		vgui::input()->IsKeyDown( ButtonCodeToJoystickButtonCode( KEY_XBUTTON_##DIR, CBaseModPanel::GetSingleton().GetLastActiveUserId() ) ) || \
		vgui::input()->IsKeyDown( ButtonCodeToJoystickButtonCode( KEY_XSTICK1_##DIR, CBaseModPanel::GetSingleton().GetLastActiveUserId() ) ) || \
		vgui::input()->IsKeyDown( ButtonCodeToJoystickButtonCode( KEY_XSTICK2_##DIR, CBaseModPanel::GetSingleton().GetLastActiveUserId() ) ) ) \
	{ \
		CheckButtonRepeat( s_flLastRepeat##DIR, s_iRepeatCount##DIR, arg1, arg2, flSpeed, flAccel, iMaxAccel ); \
	} \
	else \
	{ \
		s_flLastRepeat##DIR = 0.0f; \
		s_iRepeatCount##DIR = 0; \
	}

	if ( bHorizontal )
	{
		CHECK_BUTTON_REPEAT( LEFT, -1, false );
		CHECK_BUTTON_REPEAT( RIGHT, 1, false );
	}

	if ( bVertical )
	{
		CHECK_BUTTON_REPEAT( UP, -1, true );
		CHECK_BUTTON_REPEAT( DOWN, 1, true );
	}
}

void CRD_VGUI_Option::Paint()
{
	BaseClass::Paint();

	float flMultiplier = IsEnabled() ? 1.0f : 0.25f;
#define DRAW_CONTROL( x0, y0, x1, y1, full_texture_name, red, green, blue ) \
	HUD_SHEET_DRAW_RECT_COLOR( ( x0 ), ( y0 ), ( x1 ), ( y1 ), Controls, full_texture_name, ( red ) * flMultiplier, ( green ) * flMultiplier, ( blue ) * flMultiplier, 255 )

	bool bIsFocused = HasFocus() && IsEnabled();

	int x0, y0, x1, y1;
	m_pInteractiveArea->GetBounds( x0, y0, x1, y1 );
	if ( m_eMode == MODE_RADIO )
	{
		bool bIsIndeterminate = !m_bHaveCurrent || m_Current.m_iOption < 0 || m_Current.m_iOption >= m_Options.Count();
		FOR_EACH_VEC( m_Options, i )
		{
			if ( m_Options[i]->m_iFlags & FLAG_HIDDEN )
			{
				x0 += y1 + vgui::Label::Content * 2 + m_Options[i]->m_iWidth;
				continue;
			}

			flMultiplier = IsEnabled() && !( m_Options[i]->m_iFlags & FLAG_DISABLED ) ? 1.0f : 0.25f;

			bool bIsActive = bIsFocused && m_iActiveOption == i;
			bool bIsRecommended = bIsFocused && m_bHaveRecommended && m_Recommended.m_iOption == i;
			bool bIsCurrent = m_bHaveCurrent && m_Current.m_iOption == i;
			int r = 255, g = 255, b = 255;
			if ( bIsRecommended )
			{
				r = 192;
				g = 224;
			}

			if ( bIsActive )
				DRAW_CONTROL( x0, y0, x0 + y1, y0 + y1, UV_radio_hover, r, g, b );
			else if ( bIsFocused )
				DRAW_CONTROL( x0, y0, x0 + y1, y0 + y1, UV_radio_focus, r, g, b );
			else
				DRAW_CONTROL( x0, y0, x0 + y1, y0 + y1, UV_radio, r, g, b );

			if ( bIsActive && bIsIndeterminate )
				DRAW_CONTROL( x0, y0, x0 + y1, y0 + y1, UV_radio_ind_hover, 255, 255, 255 );
			else if ( bIsIndeterminate )
				DRAW_CONTROL( x0, y0, x0 + y1, y0 + y1, UV_radio_ind, 255, 255, 255 );
			else if ( bIsActive && bIsCurrent )
				DRAW_CONTROL( x0, y0, x0 + y1, y0 + y1, UV_radio_checked_hover, 255, 255, 255 );
			else if ( bIsCurrent )
				DRAW_CONTROL( x0, y0, x0 + y1, y0 + y1, UV_radio_checked, 255, 255, 255 );
			
			vgui::surface()->DrawSetTextColor( m_pInteractiveArea->GetFgColor() );
			vgui::surface()->DrawSetTextFont( m_pLblFieldName->GetFont() );
			vgui::surface()->DrawSetTextPos( x0 + y1 + vgui::Label::Content, y0 + ( y1 - vgui::surface()->GetFontTall( m_pLblFieldName->GetFont() ) ) / 2 );
			vgui::surface()->DrawUnicodeString( m_Options[i]->m_wszLabel );
			x0 += y1 + vgui::Label::Content * 2 + m_Options[i]->m_iWidth;
		}
	}
	else if ( m_eMode == MODE_DROPDOWN )
	{
		if ( m_bHaveCurrent )
		{
			Assert( m_Current.m_iOption >= 0 && m_Current.m_iOption < m_Options.Count() );
			if ( m_Current.m_iOption >= 0 && m_Current.m_iOption < m_Options.Count() )
			{
				if ( HasFocus() )
					vgui::surface()->DrawSetTextColor( 255, 255, 255, 255 );
				else
					vgui::surface()->DrawSetTextColor( m_pInteractiveArea->GetFgColor() );
				vgui::surface()->DrawSetTextFont( m_pLblFieldName->GetFont() );
				vgui::surface()->DrawSetTextPos( x0 + vgui::Label::Content, y0 + ( y1 - vgui::surface()->GetFontTall( m_pLblFieldName->GetFont() ) ) / 2 );
				vgui::surface()->DrawUnicodeString( m_Options[m_Current.m_iOption]->m_wszLabel );
			}
		}
	}
	else if ( m_eMode == MODE_CHECKBOX )
	{
		if ( HasFocus() )
			DRAW_CONTROL( x0, y0, x0 + x1, y0 + y1, UV_checkbox_hover, 255, 255, 255 );
		else if ( m_bHaveRecommended && ( !m_bHaveCurrent || m_Current.m_flValue != m_Recommended.m_flValue ) )
			DRAW_CONTROL( x0, y0, x0 + x1, y0 + y1, UV_checkbox_focus, 255, 255, 255 );
		else
			DRAW_CONTROL( x0, y0, x0 + x1, y0 + y1, UV_checkbox, 255, 255, 255 );

		if ( !m_bHaveCurrent )
		{
			if ( HasFocus() )
				HUD_SHEET_DRAW_BOUNDS( Controls, UV_checkbox_ind_hover );
			else
				HUD_SHEET_DRAW_BOUNDS( Controls, UV_checkbox_ind );
		}
		else if ( m_Current.m_flValue == m_flMaxValue )
		{
			if ( HasFocus() )
				HUD_SHEET_DRAW_BOUNDS( Controls, UV_checkbox_checked_hover );
			else
				HUD_SHEET_DRAW_BOUNDS( Controls, UV_checkbox_checked );
		}
		else if ( m_Current.m_flValue != m_flMinValue )
		{
			if ( HasFocus() )
				HUD_SHEET_DRAW_BOUNDS( Controls, UV_checkbox_ind_hover );
			else
				HUD_SHEET_DRAW_BOUNDS( Controls, UV_checkbox_ind );
		}
	}
	else if ( m_eMode == MODE_SLIDER )
	{
		FOR_EACH_VEC( m_Options, i )
		{
			HUD_SHEET_DRAW_RECT( x0, y0, x0 + y1, y0 + y1, Controls, UV_radio );

			Assert( !"TODO: slider with options" );

			x0 += y1;
		}

		float flMin = m_flMinValue, flMax = m_flMaxValue;
		if ( m_bReverseSlider )
			V_swap( flMin, flMax );
		int x = m_bHaveCurrent ? RemapValClamped( m_Current.m_flValue, flMin, flMax, x0 + y1 / 2, x0 + x1 - y1 / 2 ) : x0 + x1 / 2;
		const HudSheetTexture_t &Bar = g_RD_HUD_Sheets.m_Controls[CRD_HUD_Sheets::UV_slider_bar];
		const HudSheetTexture_t &BarEnd = g_RD_HUD_Sheets.m_Controls[CRD_HUD_Sheets::UV_slider_bar_end];
		vgui::surface()->DrawSetTexture( g_RD_HUD_Sheets.m_nControlsID );
		vgui::surface()->DrawSetColor( 255 * flMultiplier, 255 * flMultiplier, 255 * flMultiplier, 255 );
		if ( !m_bHaveCurrent )
			vgui::surface()->DrawSetColor( 128 * flMultiplier, 128 * flMultiplier, 128 * flMultiplier, 255 );
		vgui::surface()->DrawTexturedSubRect( x0, y0, x0 + y1 / 2, y0 + y1, BarEnd.u, BarEnd.v, BarEnd.s, BarEnd.t );
		vgui::surface()->DrawTexturedSubRect( x0 + y1 / 2, y0, x, y0 + y1, ( Bar.u + Bar.s ) / 2, Bar.v, ( Bar.u + Bar.s ) / 2, Bar.t );
		vgui::surface()->DrawSetColor( 128 * flMultiplier, 128 * flMultiplier, 128 * flMultiplier, 255 );
		vgui::surface()->DrawTexturedSubRect( x, y0, x0 + x1 - y1 / 2, y0 + y1, ( Bar.u + Bar.s ) / 2, Bar.v, ( Bar.u + Bar.s ) / 2, Bar.t );
		vgui::surface()->DrawTexturedSubRect( x0 + x1 - y1 / 2, y0, x0 + x1, y0 + y1, BarEnd.s, BarEnd.v, BarEnd.u, BarEnd.t );
		if ( m_bHaveRecommended && m_Recommended.m_flValue >= m_flMinValue && m_Recommended.m_flValue <= m_flMaxValue )
		{
			vgui::surface()->DrawSetColor( 192 * flMultiplier, 224 * flMultiplier, 255 * flMultiplier, 255 );
			int recommendedX = RemapValClamped( m_Recommended.m_flValue, flMin, flMax, x0 + y1 / 2, x0 + x1 - y1 / 2 );
			vgui::surface()->DrawTexturedSubRect( recommendedX - YRES( 0.5f ), y0, recommendedX + YRES( 0.5f ), y0 + y1, ( Bar.u + Bar.s ) / 2, Bar.v, ( Bar.u + Bar.s ) / 2, Bar.t );
		}

		if ( bIsFocused && m_bSliderActive )
			HUD_SHEET_DRAW_RECT( x - y1 / 4, y0, x + y1 / 4, y0 + y1, Controls, UV_slider_handle_hover );
		else if ( bIsFocused && m_iActiveOption == m_Options.Count() )
			HUD_SHEET_DRAW_RECT( x - y1 / 4, y0, x + y1 / 4, y0 + y1, Controls, UV_slider_handle_focus );
		else
			HUD_SHEET_DRAW_RECT( x - y1 / 4, y0, x + y1 / 4, y0 + y1, Controls, UV_slider_handle );
	}
	else if ( m_eMode == MODE_COLOR )
	{
		Assert( m_pSliderLink );
		if ( m_pSliderLink )
		{
			Color c = m_pSliderLink->GetColor();
			vgui::surface()->DrawSetColor( c.r(), c.g(), c.b(), 255 );
			vgui::surface()->DrawSetTexture( g_RD_HUD_Sheets.m_nControlsID );
			vgui::surface()->DrawTexturedSubRect( x0, y0, x0 + x1, y0 + y1, HUD_UV_COORDS( Controls, UV_color_swatch ) );
		}

		if ( HasFocus() )
			HUD_SHEET_DRAW_BOUNDS( Controls, UV_color_swatch_border_hover );
		else
			HUD_SHEET_DRAW_BOUNDS( Controls, UV_color_swatch_border );
	}
	else
	{
		Assert( m_eMode == MODE_CUSTOM );
	}
}

void CRD_VGUI_Option::OnKeyCodeTyped( vgui::KeyCode code )
{
	if ( code == KEY_ESCAPE )
	{
		if ( m_eMode == MODE_DROPDOWN && m_iActiveOption != -1 )
		{
			Assert( m_pDropdown );
			m_iActiveOption = -1;
			m_pDropdown->SetVisible( false );
			return;
		}

		if ( m_bSliderActive )
		{
			if ( OnActivateButton( false ) )
				return;
		}
	}

	BaseClass::OnKeyCodeTyped( code );
}

void CRD_VGUI_Option::OnKeyCodePressed( vgui::KeyCode code )
{
	if ( m_pTextEntry && m_pTextEntry->HasFocus() )
		return;

	int joystick = GetJoystickForCode( code );
	if ( joystick != CBaseModPanel::GetSingleton().GetLastActiveUserId() )
		return;

	switch ( GetBaseButtonCode( code ) )
	{
	case KEY_SPACE:
	case KEY_ENTER:
	case KEY_PAD_ENTER:
	case KEY_XBUTTON_A:
		if ( OnActivateButton( false ) )
			return;

		break;

	case KEY_LEFT:
	case KEY_XBUTTON_LEFT:
	case KEY_XSTICK1_LEFT:
	case KEY_XSTICK2_LEFT:
		if ( OnMovementButton( -1, false ) )
			return;

		break;

	case KEY_RIGHT:
	case KEY_XBUTTON_RIGHT:
	case KEY_XSTICK1_RIGHT:
	case KEY_XSTICK2_RIGHT:
		if ( OnMovementButton( 1, false ) )
			return;

		break;

	case KEY_UP:
	case KEY_XBUTTON_UP:
	case KEY_XSTICK1_UP:
	case KEY_XSTICK2_UP:
		if ( OnMovementButton( -1, true ) )
			return;

		break;

	case KEY_DOWN:
	case KEY_XBUTTON_DOWN:
	case KEY_XSTICK1_DOWN:
	case KEY_XSTICK2_DOWN:
		if ( OnMovementButton( 1, true ) )
			return;

		break;
	}

	BaseClass::OnKeyCodePressed( code );
}

void CRD_VGUI_Option::OnMousePressed( vgui::MouseCode code )
{
	if ( m_pTextEntry && m_pTextEntry->HasFocus() )
	{
		BaseClass::OnMouseReleased( code );

		return;
	}

	switch ( code )
	{
	case MOUSE_LEFT:
		if ( OnActivateButton( true ) )
			return;

		break;
	}

	BaseClass::OnMousePressed( code );
}

void CRD_VGUI_Option::OnMouseReleased( vgui::MouseCode code )
{
	if ( m_pTextEntry && m_pTextEntry->HasFocus() )
	{
		BaseClass::OnMouseReleased( code );

		return;
	}

	switch ( code )
	{
	case MOUSE_LEFT:
		if ( OnDeactivateButton( true ) )
			return;

		break;
	}

	BaseClass::OnMouseReleased( code );
}

void CRD_VGUI_Option::OnTextChanged()
{
	Assert( m_pTextEntry );
	Assert( !s_bSuppressTextEntryChange );

	float flNewValue = m_pTextEntry->GetValueAsFloat();
	flNewValue = clamp( flNewValue, m_flMinValue, m_flMaxValue );

	s_bSuppressTextEntryChange = true;
	SetCurrentSliderValue( flNewValue );

	Assert( m_bHaveCurrent && m_pSliderLink );
	if ( m_bHaveCurrent && m_pSliderLink )
	{
		m_pSliderLink->SetValue( flNewValue );
		s_bCVarChanged = true;
	}
	s_bSuppressTextEntryChange = false;
}

void CRD_VGUI_Option::OnTextKillFocus()
{
	Assert( m_pTextEntry );
	Assert( !s_bSuppressTextEntryChange );

	if ( m_bHaveCurrent )
	{
		// re-set slider value so the text box gets the correct (clamped) value
		SetCurrentSliderValue( m_Current.m_flValue );
	}
}

void CRD_VGUI_Option::RemoveAllOptions()
{
	Assert( m_eMode == MODE_RADIO || m_eMode == MODE_DROPDOWN || m_eMode == MODE_SLIDER );
	if ( m_eMode != MODE_SLIDER )
	{
		m_bHaveCurrent = false;
		m_bHaveRecommended = false;
	}

	m_Options.PurgeAndDeleteElements();
	InvalidateLayout();
}

void CRD_VGUI_Option::AddOption( int iOption, const char *szLabel, const char *szHint, int iFlags )
{
#ifdef DBGFLAG_ASSERT
	Assert( m_eMode == MODE_RADIO || m_eMode == MODE_DROPDOWN || m_eMode == MODE_SLIDER );
	Assert( m_eMode != MODE_SLIDER || iOption < m_flMinValue || iOption > m_flMaxValue );
	FOR_EACH_VEC( m_Options, i )
	{
		Assert( m_Options[i]->m_iValue != iOption );
	}
#endif

	m_Options.AddToTail( new Option_t{ iOption, szLabel, szHint, iFlags } );
	InvalidateLayout();
}

void CRD_VGUI_Option::SetOptionFlags( int iOption, int iFlags )
{
	Assert( m_eMode == MODE_RADIO || m_eMode == MODE_DROPDOWN || m_eMode == MODE_SLIDER );
	Assert( m_eMode != MODE_SLIDER || iOption < m_flMinValue || iOption > m_flMaxValue );

	FOR_EACH_VEC( m_Options, i )
	{
		if ( m_Options[i]->m_iValue == iOption )
		{
			m_Options[i]->m_iFlags = iFlags;
			InvalidateLayout();

			return;
		}
	}

	Assert( !"option not found" );
}

void CRD_VGUI_Option::SetCurrentAndRecommended( int iCurrent, int iRecommended )
{
	SetCurrentOption( iCurrent );
	SetRecommendedOption( iRecommended );
}

int CRD_VGUI_Option::GetCurrentOption()
{
	Assert( m_eMode == MODE_RADIO || m_eMode == MODE_DROPDOWN );

	if ( !m_bHaveCurrent )
		return -1;

	Assert( m_Current.m_iOption >= 0 );
	Assert( m_Current.m_iOption < m_Options.Count() );

	return m_Options[m_Current.m_iOption]->m_iValue;
}

void CRD_VGUI_Option::SetCurrentOption( int iCurrent )
{
	Assert( m_eMode == MODE_RADIO || m_eMode == MODE_DROPDOWN );

	KeyValues *pKV = new KeyValues( "CurrentOptionChanged" );
	pKV->SetPtr( "panel", this );

#ifdef DBGFLAG_ASSERT
	bool bHaveNegativeOne = false;
#endif
	FOR_EACH_VEC( m_Options, i )
	{
		if ( m_Options[i]->m_iValue == iCurrent )
		{
			m_bHaveCurrent = true;
			m_Current.m_iOption = i;

			PostActionSignal( pKV );

			return;
		}

#ifdef DBGFLAG_ASSERT
		if ( m_Options[i]->m_iValue == -1 )
		{
			bHaveNegativeOne = true;
		}
#endif
	}

	Assert( iCurrent == ( bHaveNegativeOne ? -2 : -1 ) );
	m_bHaveCurrent = false;

	PostActionSignal( pKV );
}

void CRD_VGUI_Option::SetRecommendedOption( int iRecommended )
{
	Assert( m_eMode == MODE_RADIO || m_eMode == MODE_DROPDOWN );

#ifdef DBGFLAG_ASSERT
	bool bHaveNegativeOne = false;
#endif
	FOR_EACH_VEC( m_Options, i )
	{
		if ( m_Options[i]->m_iValue == iRecommended )
		{
			m_bHaveRecommended = true;
			m_Recommended.m_iOption = i;

			return;
		}

#ifdef DBGFLAG_ASSERT
		if ( m_Options[i]->m_iValue == -1 )
		{
			bHaveNegativeOne = true;
		}
#endif
	}

	Assert( iRecommended == ( bHaveNegativeOne ? -2 : -1 ) );
	m_bHaveRecommended = false;
}

void CRD_VGUI_Option::SetSliderMinMax( float flMin, float flMax )
{
	m_bReverseSlider = flMax < flMin;
	if ( m_bReverseSlider )
		V_swap( flMin, flMax );

#ifdef DBGFLAG_ASSERT
	Assert( m_eMode == MODE_SLIDER || m_eMode == MODE_CHECKBOX );
	Assert( m_eMode != MODE_CHECKBOX || flMin != flMax );
	FOR_EACH_VEC( m_Options, i )
	{
		Assert( m_Options[i]->m_iValue < flMin || m_Options[i]->m_iValue > flMax );
	}
#endif

	m_flMinValue = flMin;
	m_flMaxValue = flMax;
}

void CRD_VGUI_Option::SetCurrentSliderValue( float flValue )
{
#ifdef DBGFLAG_ASSERT
	Assert( m_eMode == MODE_SLIDER || m_eMode == MODE_CHECKBOX );
	bool bMatchesOption = false;
	FOR_EACH_VEC( m_Options, i )
	{
		if ( m_Options[i]->m_iValue == flValue )
		{
			bMatchesOption = true;
			break;
		}
	}
	Assert( m_eMode != MODE_SLIDER || ( m_flMinValue <= flValue && flValue <= m_flMaxValue ) || bMatchesOption );
	Assert( m_eMode != MODE_CHECKBOX || m_flMinValue == flValue || m_flMaxValue == flValue );
#endif

	m_bHaveCurrent = true;
	m_Current.m_flValue = flValue;

	if ( m_pTextEntry && !s_bSuppressTextEntryChange && !IsLayoutInvalid() )
	{
		if ( IsEnabled() )
		{
			m_pTextEntry->SetEnabled( true );
			m_pTextEntry->SetCursor( vgui::dc_ibeam );
			m_pTextEntry->SetAlpha( 255 );
		}
		else
		{
			m_pTextEntry->SetEnabled( false );
			m_pTextEntry->SetCursor( vgui::dc_arrow );
			m_pTextEntry->SetAlpha( 64 );
		}

		int iDigits = 1 - MIN( log10f( ( m_flMaxValue - m_flMinValue ) * YRES( 1 ) / m_pInteractiveArea->GetWide() ), 0 );

		m_pTextEntry->SetText( VarArgs( "%.*f", iDigits, flValue ) );
	}

	KeyValues *pKV = new KeyValues( "CurrentOptionChanged" );
	pKV->SetPtr( "panel", this );
	PostActionSignal( pKV );
}

void CRD_VGUI_Option::SetRecommendedSliderValue( float flValue )
{
#ifdef DBGFLAG_ASSERT
	Assert( m_eMode == MODE_SLIDER || m_eMode == MODE_CHECKBOX );
	bool bMatchesOption = false;
	FOR_EACH_VEC( m_Options, i )
	{
		if ( m_Options[i]->m_iValue == flValue )
		{
			bMatchesOption = true;
			break;
		}
	}
	Assert( m_eMode != MODE_SLIDER || ( m_flMinValue <= flValue && flValue <= m_flMaxValue ) || bMatchesOption );
	Assert( m_eMode != MODE_CHECKBOX || m_flMinValue == flValue || m_flMaxValue == flValue );
#endif
	m_bHaveRecommended = true;
	m_Recommended.m_flValue = flValue;
}

void CRD_VGUI_Option::ClearRecommendedSliderValue()
{
	Assert( m_eMode == MODE_SLIDER || m_eMode == MODE_CHECKBOX );
	m_bHaveRecommended = false;
}

float CRD_VGUI_Option::GetCurrentSliderValue()
{
	Assert( m_eMode == MODE_SLIDER || m_eMode == MODE_CHECKBOX );
	return m_Current.m_flValue;
}

void CRD_VGUI_Option::SetDefaultHint( const char *szHint )
{
	TryLocalize( szHint, m_wszDefaultHint, sizeof( m_wszDefaultHint ) );
	InvalidateLayout();
}

void CRD_VGUI_Option::LinkToConVar( const char *szName, bool bSetRecommendedToDefaultValue )
{
	Assert( m_eMode == MODE_CHECKBOX || m_eMode == MODE_RADIO || m_eMode == MODE_DROPDOWN || m_eMode == MODE_SLIDER || m_eMode == MODE_COLOR );
	ConVar *pVar = g_pCVar->FindVar( szName );
	Assert( pVar );
	if ( !pVar )
		return;

	Assert( m_pSliderLink == NULL );
	if ( m_eMode == MODE_SLIDER || m_eMode == MODE_CHECKBOX || m_eMode == MODE_COLOR )
	{
		m_pSliderLink = pVar;
	}

	FOR_EACH_VEC( m_Options, i )
	{
		Assert( m_Options[i]->m_ConVars.Count() == 0 );
		m_Options[i]->m_ConVars.AddToTail( ConVarLink_t{ pVar, m_Options[i]->m_iValue } );
	}

	if ( m_eMode != MODE_COLOR )
	{
		SetCurrentUsingConVars();
		if ( bSetRecommendedToDefaultValue )
			SetRecommendedUsingConVars();
	}
}

void CRD_VGUI_Option::LinkToConVarAdvanced( int iOption, const char *szName, const char *szValue )
{
	Assert( m_eMode == MODE_RADIO || m_eMode == MODE_DROPDOWN || m_eMode == MODE_SLIDER );
	ConVar *pVar = g_pCVar->FindVar( szName );
	Assert( pVar );
	if ( !pVar )
		return;

	FOR_EACH_VEC( m_Options, i )
	{
		if ( m_Options[i]->m_iValue != iOption )
			continue;

#ifdef DBGFLAG_ASSERT
		FOR_EACH_VEC( m_Options[i]->m_ConVars, j )
		{
			Assert( m_Options[i]->m_ConVars[j].m_pConVar != pVar );
		}
#endif

		m_Options[i]->m_ConVars.AddToTail( ConVarLink_t{ pVar, szValue } );

		return;
	}

	Assert( !"option was not present in list" );
}

void CRD_VGUI_Option::LinkToConVarAdvanced( int iOption, const char *szName, int iValue )
{
	Assert( m_eMode == MODE_RADIO || m_eMode == MODE_DROPDOWN || m_eMode == MODE_SLIDER );
	ConVar *pVar = g_pCVar->FindVar( szName );
	Assert( pVar );
	if ( !pVar )
		return;

	FOR_EACH_VEC( m_Options, i )
	{
		if ( m_Options[i]->m_iValue != iOption )
			continue;

#ifdef DBGFLAG_ASSERT
		FOR_EACH_VEC( m_Options[i]->m_ConVars, j )
		{
			Assert( m_Options[i]->m_ConVars[j].m_pConVar != pVar );
		}
#endif

		m_Options[i]->m_ConVars.AddToTail( ConVarLink_t{ pVar, iValue } );

		return;
	}

	Assert( !"option was not present in list" );
}

void CRD_VGUI_Option::SetCurrentUsingConVars()
{
	m_bSetUsingConVars = true;

	Assert( m_eMode == MODE_CHECKBOX || m_eMode == MODE_RADIO || m_eMode == MODE_DROPDOWN || m_eMode == MODE_SLIDER );
	if ( m_eMode == MODE_CHECKBOX )
	{
		Assert( m_pSliderLink );
		if ( m_pSliderLink )
		{
			float flValue = m_pSliderLink->GetFloat();

			if ( m_flMinValue == flValue || m_flMaxValue == flValue )
			{
				SetCurrentSliderValue( flValue );

				return;
			}
		}

		m_bHaveCurrent = false;

		return;
	}

	if ( m_eMode == MODE_SLIDER )
	{
		Assert( m_pSliderLink );
		if ( m_pSliderLink )
		{
			float flValue = m_pSliderLink->GetFloat();
			if ( flValue >= m_flMinValue && flValue <= m_flMaxValue )
			{
				SetCurrentSliderValue( flValue );

				return;
			}
		}
	}

	bool bFoundCompleteMatch = false;
	FOR_EACH_VEC( m_Options, i )
	{
		Assert( m_Options[i]->m_ConVars.Count() > 0 );
		bool bOptionMatches = true;
		FOR_EACH_VEC( m_Options[i]->m_ConVars, j )
		{
			bool bVarMatches;
			if ( m_Options[i]->m_ConVars[j].m_szValue )
				bVarMatches = !V_strcmp( m_Options[i]->m_ConVars[j].m_pConVar->GetString(), m_Options[i]->m_ConVars[j].m_szValue );
			else
				bVarMatches = m_Options[i]->m_ConVars[j].m_pConVar->GetInt() == m_Options[i]->m_ConVars[j].m_iValue;

			if ( !bVarMatches )
			{
				bOptionMatches = false;
				break;
			}
		}

		if ( bOptionMatches )
		{
			Assert( !bFoundCompleteMatch );
			bFoundCompleteMatch = true;

			m_bHaveCurrent = true;
			if ( m_eMode == MODE_SLIDER )
				m_Current.m_flValue = m_Options[i]->m_iValue;
			else
				m_Current.m_iOption = i;

#ifndef DBGFLAG_ASSERT
			return;
#endif
		}
	}

	if ( !bFoundCompleteMatch )
	{
		m_bHaveCurrent = false;
	}
}

void CRD_VGUI_Option::SetRecommendedUsingConVars()
{
	Assert( m_eMode == MODE_CHECKBOX || m_eMode == MODE_RADIO || m_eMode == MODE_DROPDOWN || m_eMode == MODE_SLIDER );
	if ( m_eMode == MODE_CHECKBOX )
	{
		Assert( m_pSliderLink );
		if ( m_pSliderLink )
		{
			float flValue = V_atof( m_pSliderLink->GetDefault() );

			if ( m_flMinValue == flValue || m_flMaxValue == flValue )
			{
				m_bHaveRecommended = true;
				m_Recommended.m_flValue = flValue;

				return;
			}
		}

		m_bHaveRecommended = false;

		return;
	}

	if ( m_eMode == MODE_SLIDER )
	{
		Assert( m_pSliderLink );
		if ( m_pSliderLink )
		{
			float flValue = V_atof( m_pSliderLink->GetDefault() );
			if ( flValue >= m_flMinValue && flValue <= m_flMaxValue )
			{
				m_bHaveRecommended = true;
				m_Recommended.m_flValue = flValue;
				return;
			}
		}
	}

	bool bFoundCompleteMatch = false;
	FOR_EACH_VEC( m_Options, i )
	{
		Assert( m_Options[i]->m_ConVars.Count() > 0 );
		bool bOptionMatches = true;
		FOR_EACH_VEC( m_Options[i]->m_ConVars, j )
		{
			bool bVarMatches;
			if ( m_Options[i]->m_ConVars[j].m_szValue )
				bVarMatches = !V_strcmp( m_Options[i]->m_ConVars[j].m_pConVar->GetDefault(), m_Options[i]->m_ConVars[j].m_szValue );
			else
				bVarMatches = V_atoi( m_Options[i]->m_ConVars[j].m_pConVar->GetDefault() ) == m_Options[i]->m_ConVars[j].m_iValue;

			if ( !bVarMatches )
			{
				bOptionMatches = false;
				break;
			}
		}

		if ( bOptionMatches )
		{
			Assert( !bFoundCompleteMatch );
			bFoundCompleteMatch = true;

			m_bHaveRecommended = true;
			if ( m_eMode == MODE_SLIDER )
				m_Recommended.m_flValue = m_Options[i]->m_iValue;
			else
				m_Recommended.m_iOption = i;

#ifndef DBGFLAG_ASSERT
			return;
#endif
		}
	}

	if ( !bFoundCompleteMatch )
	{
		m_bHaveRecommended = false;
	}
}

void CRD_VGUI_Option::WriteConfig( bool bForce )
{
	if ( s_bCVarChanged || bForce )
	{
		engine->ClientCmd_Unrestricted( "mat_savechanges" );
		engine->ClientCmd_Unrestricted( "host_writeconfig" );
		s_bCVarChanged = false;
	}
}

CRD_VGUI_Option::ConVarLink_t::ConVarLink_t( ConVar *pConVar, const char *szValue )
{
	Assert( pConVar );
	Assert( szValue );

	m_pConVar = pConVar;
	m_szValue = szValue;
	m_iValue = 0;
}

CRD_VGUI_Option::ConVarLink_t::ConVarLink_t( ConVar *pConVar, int iValue )
{
	Assert( pConVar );

	m_pConVar = pConVar;
	m_szValue = NULL;
	m_iValue = iValue;
}

CRD_VGUI_Option::Option_t::Option_t( int iValue, const char *szLabel, const char *szHint, int iFlags )
{
	m_iValue = iValue;
	TryLocalize( szLabel, m_wszLabel, sizeof( m_wszLabel ) );
	TryLocalize( szHint, m_wszHint, sizeof( m_wszHint ) );
	m_iFlags = iFlags;
	m_iWidth = 30;
}

bool CRD_VGUI_Option::s_bCVarChanged = false;
float CRD_VGUI_Option::s_flLastRepeatLEFT = 0.0f, CRD_VGUI_Option::s_flLastRepeatRIGHT = 0.0f;
float CRD_VGUI_Option::s_flLastRepeatUP = 0.0f, CRD_VGUI_Option::s_flLastRepeatDOWN = 0.0f;
int CRD_VGUI_Option::s_iRepeatCountLEFT = 0, CRD_VGUI_Option::s_iRepeatCountRIGHT = 0;
int CRD_VGUI_Option::s_iRepeatCountUP = 0, CRD_VGUI_Option::s_iRepeatCountDOWN = 0;

bool CRD_VGUI_Option::OnActivateButton( bool bMouse )
{
	if ( !IsEnabled() )
	{
		return false;
	}

	// reset repeat timer on activate
	s_flLastRepeatLEFT = s_flLastRepeatRIGHT = 0.0f;
	s_flLastRepeatUP = s_flLastRepeatDOWN = 0.0f;
	s_iRepeatCountLEFT = s_iRepeatCountRIGHT = 0;
	s_iRepeatCountUP = s_iRepeatCountDOWN = 0;

	if ( m_eMode == MODE_CHECKBOX )
	{
		ToggleCheckbox();

		return true;
	}

	if ( ( m_eMode == MODE_RADIO || m_eMode == MODE_SLIDER || m_eMode == MODE_DROPDOWN ) && m_iActiveOption >= 0 && m_iActiveOption < m_Options.Count() )
	{
		SelectActiveRadioButton();

		return true;
	}

	if ( m_eMode == MODE_SLIDER && m_iActiveOption == m_Options.Count() )
	{
		ToggleSliderActive( bMouse );

		return true;
	}

	if ( m_eMode == MODE_COLOR )
	{
		ToggleColorActive();

		return true;
	}

	if ( m_eMode == MODE_DROPDOWN )
	{
		ToggleDropdownActive();

		return true;
	}

	return false;
}

bool CRD_VGUI_Option::OnDeactivateButton( bool bMouse )
{
	Assert( bMouse );

	if ( !IsEnabled() )
	{
		return false;
	}

	if ( m_eMode == MODE_SLIDER && m_iActiveOption == m_Options.Count() && m_bSliderActive )
	{
		ToggleSliderActive( bMouse );

		return true;
	}

	return false;
}

bool CRD_VGUI_Option::OnMovementButton( int iDirection, bool bVertical )
{
	if ( !IsEnabled() )
		return false;

	if ( !bVertical && ( m_eMode == MODE_RADIO || ( m_eMode == MODE_SLIDER && !m_bSliderActive ) ) )
	{
		for ( int i = m_iActiveOption + iDirection; i >= 0 && i < m_Options.Count() + ( m_eMode == MODE_SLIDER ? 1 : 0 ); i += iDirection )
		{
			if ( i == m_Options.Count() )
			{
				Assert( m_eMode == MODE_SLIDER );
				ChangeActiveRadioButton( i );

				return true;
			}

			if ( m_Options[i]->m_iFlags & FLAG_DISABLED )
			{
				continue;
			}

			ChangeActiveRadioButton( i );

			return true;
		}
	}

	// FIXME: these next two don't properly handle disabled options, but it isn't an issue yet.
	if ( !bVertical && m_pTextEntry && m_iActiveOption == 0 && iDirection == -1 && !m_pTextEntry->HasFocus() && m_pInteractiveArea->GetNavLeft() == m_pTextEntry )
	{
		NavigateToChild( m_pTextEntry );

		return true;
	}

	if ( !bVertical && m_pTextEntry && m_iActiveOption == m_Options.Count() + ( m_eMode == MODE_SLIDER ? 1 : 0 ) && iDirection == 1 && !m_pTextEntry->HasFocus() && m_pInteractiveArea->GetNavRight() == m_pTextEntry )
	{
		NavigateToChild( m_pTextEntry );

		return true;
	}

	if ( !bVertical && m_eMode == MODE_SLIDER && m_bSliderActive )
	{
		float flMin = m_flMinValue, flMax = m_flMaxValue;
		if ( m_bReverseSlider )
			V_swap( flMin, flMax );

		float flDelta = iDirection * ( flMax - flMin ) / m_pInteractiveArea->GetWide() / 480.0f * ScreenWidth();
		float flAbsDelta = fabsf( flDelta );

		float flNewValue = clamp( m_Current.m_flValue + flDelta, m_flMinValue, m_flMaxValue );
		if ( m_bHaveRecommended && m_Recommended.m_flValue >= m_flMinValue && m_Recommended.m_flValue <= m_flMaxValue && !m_bStartedSliderActiveAtRecommended &&
			!( m_Current.m_flValue >= m_Recommended.m_flValue - flAbsDelta * rd_option_slider_recommended_magnetism.GetFloat() && m_Current.m_flValue <= m_Recommended.m_flValue + flAbsDelta * rd_option_slider_recommended_magnetism.GetFloat() ) &&
			flNewValue >= m_Recommended.m_flValue - flAbsDelta * rd_option_slider_recommended_magnetism.GetFloat() && flNewValue <= m_Recommended.m_flValue + flAbsDelta * rd_option_slider_recommended_magnetism.GetFloat() )
			flNewValue = m_Recommended.m_flValue;

		SetCurrentSliderValue( flNewValue );
		m_bStartedSliderActiveAtRecommended = m_bHaveRecommended && flNewValue == m_Recommended.m_flValue;

		Assert( m_bHaveCurrent && m_pSliderLink );
		if ( m_bHaveCurrent && m_pSliderLink )
		{
			m_pSliderLink->SetValue( flNewValue );
			s_bCVarChanged = true;
		}

		CBaseModPanel::GetSingleton().PlayUISound( UISOUND_FOCUS );

		return true;
	}

	if ( bVertical && m_pDropdown && m_pDropdown->IsVisible() )
	{
		for ( int i = m_iActiveOption + iDirection; i >= 0 && i < m_Options.Count(); i += iDirection )
		{
			if ( m_Options[i]->m_iFlags & FLAG_DISABLED )
				continue;

			m_iActiveOption = i;
			m_pDropdown->Repaint();

			CBaseModPanel::GetSingleton().PlayUISound( UISOUND_FOCUS );

			return true;
		}

		// don't leave the dropdown menu until it is closed
		return true;
	}

	return false;
}

void CRD_VGUI_Option::CheckButtonRepeat( float &flLastRepeat, int &iRepeatCount, int iDirection, bool bVertical, float flSpeed, float flAccel, int iMaxAccel )
{
	float flNow = Plat_FloatTime();
	if ( iRepeatCount <= 0 )
	{
		flLastRepeat = flNow;
		iRepeatCount = 1;
		return;
	}

	for ( ;; )
	{
		// limit repeat speed to once every 10ms
		float flRepeatAfter = MAX( flSpeed - flAccel * MIN( iRepeatCount - 1, iMaxAccel ), 0.01f );
		if ( flLastRepeat + flRepeatAfter > flNow )
			break;

		OnMovementButton( iDirection, bVertical );
		flLastRepeat += flRepeatAfter;
		iRepeatCount++;
	}
}

void CRD_VGUI_Option::ToggleCheckbox()
{
	if ( m_bHaveCurrent && m_Current.m_flValue == m_flMinValue )
		SetCurrentSliderValue( m_flMaxValue );
	else
		SetCurrentSliderValue( m_flMinValue );

	Assert( m_bHaveCurrent && m_pSliderLink );
	if ( m_bHaveCurrent && m_pSliderLink )
	{
		m_pSliderLink->SetValue( m_Current.m_flValue );
		s_bCVarChanged = true;
	}

	CBaseModPanel::GetSingleton().PlayUISound( UISOUND_ACCEPT );

	InvalidateLayout();
}

void CRD_VGUI_Option::SelectActiveRadioButton()
{
	Assert( m_eMode == MODE_RADIO || m_eMode == MODE_SLIDER || m_eMode == MODE_DROPDOWN );

	Assert( m_iActiveOption >= 0 && m_iActiveOption < m_Options.Count() );
	if ( m_iActiveOption < 0 || m_iActiveOption >= m_Options.Count() )
		return;

	if ( m_Options[m_iActiveOption]->m_iFlags & FLAG_DISABLED )
	{
		CBaseModPanel::GetSingleton().PlayUISound( UISOUND_DENY );

		return;
	}

	const CUtlVector<ConVarLink_t> &convars = m_Options[m_iActiveOption]->m_ConVars;
	FOR_EACH_VEC( convars, i )
	{
		if ( convars[i].m_szValue )
			convars[i].m_pConVar->SetValue( convars[i].m_szValue );
		else
			convars[i].m_pConVar->SetValue( convars[i].m_iValue );

		s_bCVarChanged = true;
	}

	m_bHaveCurrent = true;
	if ( m_eMode == MODE_SLIDER )
		SetCurrentSliderValue( m_Options[m_iActiveOption]->m_iValue );
	else
		SetCurrentOption( m_Options[m_iActiveOption]->m_iValue );

	if ( m_eMode == MODE_DROPDOWN )
	{
		m_pDropdown->SetVisible( false );
		m_iActiveOption = -1;
	}

	CBaseModPanel::GetSingleton().PlayUISound( UISOUND_ACCEPT );

	InvalidateLayout();
}

void CRD_VGUI_Option::ChangeActiveRadioButton( int iActive )
{
	Assert( m_eMode == MODE_RADIO || m_eMode == MODE_SLIDER );
	Assert( iActive >= 0 && iActive < m_Options.Count() + ( m_eMode == MODE_SLIDER ? 1 : 0 ) );
	Assert( !( m_Options[iActive]->m_iFlags & FLAG_DISABLED ) );

	m_iActiveOption = iActive;

	CBaseModPanel::GetSingleton().PlayUISound( UISOUND_FOCUS );
}

void CRD_VGUI_Option::ToggleSliderActive( bool bMouse )
{
	Assert( m_eMode == MODE_SLIDER );
	Assert( m_iActiveOption == m_Options.Count() );

	if ( m_bSliderActive )
	{
		m_bSliderActive = false;
		m_bSliderActiveMouse = false;

		if ( vgui::input()->GetMouseCapture() == GetVPanel() )
			vgui::input()->SetMouseCapture( NULL );
	}
	else
	{
		m_bSliderActive = true;
		m_bSliderActiveMouse = bMouse;
		m_bStartedSliderActiveAtRecommended = m_bHaveCurrent && m_bHaveRecommended && m_Current.m_flValue == m_Recommended.m_flValue;

		if ( bMouse )
		{
			vgui::input()->SetMouseCapture( GetVPanel() );

			int x, y;
			vgui::input()->GetCursorPos( x, y );
			ScreenToLocal( x, y );
			OnCursorMoved( x, y );
		}
	}
}

void CRD_VGUI_Option::ToggleColorActive()
{
	Assert( m_eMode == MODE_COLOR );

	Assert( !"TODO: ToggleColorActive" );
}

void CRD_VGUI_Option::ToggleDropdownActive()
{
	Assert( m_eMode == MODE_DROPDOWN );
	Assert( m_iActiveOption == -1 );

	if ( !m_pDropdown )
	{
		m_pDropdown = new CRD_VGUI_Option_Dropdown( this, "Dropdown" );
		m_pDropdown->MakeReadyForUse();
	}

	if ( m_bHaveCurrent )
	{
		Assert( m_Current.m_iOption >= 0 && m_Current.m_iOption < m_Options.Count() );
		m_iActiveOption = m_Current.m_iOption;
	}
	else if ( m_bHaveRecommended )
	{
		Assert( m_Recommended.m_iOption >= 0 && m_Recommended.m_iOption < m_Options.Count() );
		m_iActiveOption = m_Recommended.m_iOption;
	}
	else
	{
		Assert( m_Options.Count() );
		m_iActiveOption = 0;
	}

	m_pDropdown->SetVisible( true );
	m_pDropdown->MoveToFront();
	m_pDropdown->InvalidateLayout();
	m_pDropdown->Repaint();
}

CON_COMMAND( rd_settings, "Opens the settings screen." )
{
	CBaseModPanel::GetSingleton().OpenWindow( WT_SETTINGS, CBaseModPanel::GetSingleton().GetWindow( CBaseModPanel::GetSingleton().GetActiveWindowType() ) );
}
