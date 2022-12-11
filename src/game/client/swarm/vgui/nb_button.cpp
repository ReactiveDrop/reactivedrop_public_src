#include "cbase.h"
#include "nb_button.h"
#include "vgui/ISurface.h"
#include "vgui/ILocalize.h"
#include "inputsystem/iinputsystem.h"
#include "controller_focus.h"
#include "rd_steam_input.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

DECLARE_BUILD_FACTORY_DEFAULT_TEXT( CNB_Button, CNB_Button );

CNB_Button::CNB_Button(Panel *parent, const char *panelName, const char *text, Panel *pActionSignalTarget, const char *pCmd)
: BaseClass( parent, panelName, text, pActionSignalTarget, pCmd )
{
	// == MANAGED_MEMBER_CREATION_START: Do not edit by hand ==
	// == MANAGED_MEMBER_CREATION_END ==

	GetControllerFocus()->AddToFocusList( this );

	m_hButtonFont = INVALID_FONT;
	m_szControllerButton = NULL;
}
CNB_Button::CNB_Button(Panel *parent, const char *panelName, const wchar_t *text, Panel *pActionSignalTarget, const char *pCmd)
: BaseClass( parent, panelName, text, pActionSignalTarget, pCmd )
{
	GetControllerFocus()->AddToFocusList( this );

	m_hButtonFont = INVALID_FONT;
	m_szControllerButton = NULL;
}

CNB_Button::~CNB_Button()
{
	GetControllerFocus()->RemoveFromFocusList( this );
}

void CNB_Button::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	SetButtonBorderEnabled( false );

	SetReleasedSound( "UI/menu_accept.wav" );

	const char *pCmd = GetCommand() ? GetCommand()->GetString( "command" ) : "";
	if ( !engine->IsConnected() && ( !V_strcmp( GetName(), "BtnBack" ) || !V_strcmp( GetName(), "BtnDone" ) || !V_strcmp( GetName(), "BtnCancel" ) || !V_strcmp( GetName(), "BackButton" ) ) && ( !V_strcmp( pCmd, "Back" ) || !V_strcmp( pCmd, "BackButton" ) ) )
	{
		SetControllerButton( KEY_XBUTTON_B );
	}

	m_hButtonFont = pScheme->GetFont( "GameUIButtonsTiny", true );
}

void CNB_Button::Paint()
{
	if ( !ShouldPaint() )
		return; 

	BaseClass::BaseClass::Paint();  // skip drawing regular vgui::Button's focus border
}

void CNB_Button::PaintBackground()
{
	// draw gray outline background
	DrawRoundedBox( 0, 0, GetWide(), GetTall(), Color( 78, 94, 110, 255 ), 1.0f, false, Color( 0, 0, 0, 0 ) );

	int nBorder = MAX( YRES( 1 ), 1 );
	if ( IsArmed() || IsDepressed() )
	{
		DrawRoundedBox( nBorder, nBorder, GetWide() - nBorder * 2, GetTall() - nBorder * 2, Color( 20, 59, 96, 255 ), 1.0f, true, Color( 28, 80, 130, 255 ) );
	}
	else if ( IsEnabled() )
	{
		DrawRoundedBox( nBorder, nBorder, GetWide() - nBorder * 2, GetTall() - nBorder * 2, Color( 24, 43, 66, 255 ), 1.0f, false, Color( 0, 0, 0, 0 ) );
	}
	else
	{
		DrawRoundedBox( nBorder, nBorder, GetWide() - nBorder * 2, GetTall() - nBorder * 2, Color( 65, 78, 91, 255 ), 1.0f, false, Color( 0, 0, 0, 0 ) );
	}
}

void CNB_Button::OnCursorEntered()
{
	if ( IsPC() )
	{
		if ( IsEnabled() && !HasFocus() )
		{
			vgui::surface()->PlaySound( "UI/menu_focus.wav" );
		}
	}
	BaseClass::OnCursorEntered();
}

void CNB_Button::DrawRoundedBox( int x, int y, int wide, int tall, Color color, float normalizedAlpha, bool bHighlightGradient, Color highlightCenterColor )
{
	if ( m_nNBBgTextureId1 == -1 ||
		m_nNBBgTextureId2 == -1 ||
		m_nNBBgTextureId3 == -1 ||
		m_nNBBgTextureId4 == -1 )
	{
		return;
	}

	color[3] *= normalizedAlpha;

	// work out our bounds
	int cornerWide, cornerTall;
	GetCornerTextureSize( cornerWide, cornerTall );

	// draw the background in the areas not occupied by the corners
	// draw it in three horizontal strips
	surface()->DrawSetColor(color);
	surface()->DrawFilledRect(x + cornerWide, y, x + wide - cornerWide,	y + cornerTall);
	surface()->DrawFilledRect(x, y + cornerTall, x + wide, y + tall - cornerTall);
	surface()->DrawFilledRect(x + cornerWide, y + tall - cornerTall, x + wide - cornerWide, y + tall);

	// draw the corners
	surface()->DrawSetTexture(m_nNBBgTextureId1);
	surface()->DrawTexturedRect(x, y, x + cornerWide, y + cornerTall);
	surface()->DrawSetTexture(m_nNBBgTextureId2);
	surface()->DrawTexturedRect(x + wide - cornerWide, y, x + wide, y + cornerTall);
	surface()->DrawSetTexture(m_nNBBgTextureId3);
	surface()->DrawTexturedRect(x + wide - cornerWide, y + tall - cornerTall, x + wide, y + tall);
	surface()->DrawSetTexture(m_nNBBgTextureId4);
	surface()->DrawTexturedRect(x + 0, y + tall - cornerTall, x + cornerWide, y + tall);

	if ( bHighlightGradient )
	{
		surface()->DrawSetColor(highlightCenterColor);
		surface()->DrawFilledRectFade( x + cornerWide, y, x + wide * 0.5f, y + tall, 0, 255, true );
		surface()->DrawFilledRectFade( x + wide * 0.5f, y, x + wide - cornerWide, y + tall, 255, 0, true );
	}

	// TODO: is there a better way of knowing out-of-game whether a controller is active than just whether it's plugged in?
	if ( m_szControllerButton && g_RD_Steam_Input.GetJoystickCount() )
	{
		int padding = ( tall - surface()->GetFontTall( m_hButtonFont ) ) / 2;
		g_RD_Steam_Input.DrawLegacyControllerGlyph( m_szControllerButton, x + padding, y + padding, false, false, m_hButtonFont );
	}
}

void CNB_Button::SetControllerButton( KeyCode code )
{
	switch ( code )
	{
	case KEY_XBUTTON_A:
		m_szControllerButton = "A_BUTTON";
		break;
	case KEY_XBUTTON_B:
		m_szControllerButton = "B_BUTTON";
		break;
	case KEY_XBUTTON_X:
		m_szControllerButton = "X_BUTTON";
		break;
	case KEY_XBUTTON_Y:
		m_szControllerButton = "Y_BUTTON";
		break;
	case KEY_XBUTTON_UP:
		m_szControllerButton = "UP_DPAD";
		break;
	case KEY_XBUTTON_DOWN:
		m_szControllerButton = "DOWN_DPAD";
		break;
	case KEY_XBUTTON_LEFT:
		m_szControllerButton = "LEFT_DPAD";
		break;
	case KEY_XBUTTON_RIGHT:
		m_szControllerButton = "RIGHT_DPAD";
		break;
	case KEY_XBUTTON_LTRIGGER:
		m_szControllerButton = "LEFT_TRIGGER";
		break;
	case KEY_XBUTTON_RTRIGGER:
		m_szControllerButton = "RIGHT_TRIGGER";
		break;
	case KEY_XBUTTON_LEFT_SHOULDER:
		m_szControllerButton = "LEFT_BUMPER";
		break;
	case KEY_XBUTTON_RIGHT_SHOULDER:
		m_szControllerButton = "RIGHT_BUMPER";
		break;
	case KEY_XBUTTON_BACK:
		m_szControllerButton = "BACK_BUTTON";
		break;
	case KEY_XBUTTON_START:
		m_szControllerButton = "START_BUTTON";
		break;
	case KEY_XBUTTON_STICK1:
		m_szControllerButton = "LEFT_STICK";
		break;
	case KEY_XBUTTON_STICK2:
		m_szControllerButton = "RIGHT_STICK";
		break;
	default:
		Warning( "CNB_Button: Unhandled controller button code %d\n", code );
		m_szControllerButton = NULL;
		break;
	}
}
