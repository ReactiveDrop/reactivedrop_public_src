#include "cbase.h"
#include "nb_button_hold.h"
#include "vgui/ISurface.h"
#include "inputsystem/iinputsystem.h"
#include "rd_steam_input.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

DECLARE_BUILD_FACTORY_DEFAULT_TEXT( CNB_Button_Hold, CNB_Button_Hold );

CNB_Button_Hold::CNB_Button_Hold( vgui::Panel *parent, const char *panelName, const char *text, vgui::Panel *pActionSignalTarget, const char *pCmd, bool bSuppressAddToFocusList ) :
	BaseClass{ parent, panelName, text, pActionSignalTarget, pCmd, bSuppressAddToFocusList }
{
}

CNB_Button_Hold::CNB_Button_Hold( vgui::Panel *parent, const char *panelName, const wchar_t *text, vgui::Panel *pActionSignalTarget, const char *pCmd, bool bSuppressAddToFocusList ) :
	BaseClass{ parent, panelName, text, pActionSignalTarget, pCmd, bSuppressAddToFocusList }
{
}

void CNB_Button_Hold::PaintBackground()
{
	BaseClass::PaintBackground();

	if ( m_flHoldProgress <= -1.0f )
	{
		return;
	}

	if ( m_nNBBgTextureId1 == -1 || m_nNBBgTextureId2 == -1 || m_nNBBgTextureId3 == -1 || m_nNBBgTextureId4 == -1 )
	{
		return;
	}

	float flHoldProgress = clamp( m_flHoldProgress, 0.0f, 1.0f );

	int wide, tall;
	GetSize( wide, tall );

	int nBorder = MAX( m_nBorderThick, m_nBorderThickMin );
	int x = nBorder, y = nBorder;
	wide -= nBorder * 2;
	tall -= nBorder * 2;

	int wide2 = wide * ( m_flHoldProgress >= 0.0f ? Lerp( m_flHoldHighlightLip, flHoldProgress, 1.0f ) : m_flHoldHighlightLip * ( 1.0f + m_flHoldProgress ) );

	int cornerWide, cornerTall;
	GetCornerTextureSize( cornerWide, cornerTall );

	int r1, g1, b1, a1, r2, g2, b2, a2, r3, g3, b3, a3;
	m_HoldHighlightColorStart.GetColor( r1, g1, b1, a1 );
	m_HoldHighlightColorFlash1.GetColor( r2, g2, b2, a2 );
	m_HoldHighlightColorFlash2.GetColor( r3, g3, b3, a3 );
	float flFlashInterp = sinf( m_flHoldProgress * m_flHoldHighlightColorSpeed ) * 0.5f + 0.5f;

#define INTERP( channel ) Lerp( Bias( flHoldProgress, m_flHoldHighlightColorBias ), channel##1, Lerp( flFlashInterp, channel##2, channel##3 ) )
	vgui::surface()->DrawSetColor( INTERP( r ), INTERP( g ), INTERP( b ), INTERP( a ) );
#undef INTERP
	if ( wide2 >= cornerWide )
		vgui::surface()->DrawFilledRect( x + cornerWide, y, x + MIN( wide - cornerWide, wide2 ), y + cornerTall );
	vgui::surface()->DrawFilledRect( x, y + cornerTall, x + wide2, y + tall - cornerTall );
	if ( wide2 >= cornerWide )
		vgui::surface()->DrawFilledRect( x + cornerWide, y + tall - cornerTall, x + MIN( wide - cornerWide, wide2 ), y + tall );

	float s1 = RemapValClamped( wide2, 0.0f, cornerWide, 0.0f, 1.0f );
	float s2 = RemapValClamped( wide2, wide - cornerWide, wide, 0.0f, 1.0f );
	vgui::surface()->DrawSetTexture( m_nNBBgTextureId1 );
	vgui::surface()->DrawTexturedSubRect( x, y, x + MIN( cornerWide, wide2 ), y + cornerTall, 0.0f, 0.0f, s1, 1.0f );
	if ( wide2 >= wide - cornerWide )
	{
		vgui::surface()->DrawSetTexture( m_nNBBgTextureId2 );
		vgui::surface()->DrawTexturedSubRect( x + wide - cornerWide, y, x + wide2, y + cornerTall, 0.0f, 0.0f, s2, 1.0f );
		vgui::surface()->DrawSetTexture( m_nNBBgTextureId3 );
		vgui::surface()->DrawTexturedSubRect( x + wide - cornerWide, y + tall - cornerTall, x + wide2, y + tall, 0.0f, 0.0f, s2, 1.0f );
	}
	vgui::surface()->DrawSetTexture( m_nNBBgTextureId4 );
	vgui::surface()->DrawTexturedSubRect( x + 0, y + tall - cornerTall, x + MIN( cornerWide, wide2 ), y + tall, 0.0f, 0.0f, s1, 1.0f );

	// re-draw the controller button icon so it's on top of the wipe
	if ( m_szControllerButton && g_RD_Steam_Input.GetJoystickCount() )
	{
		int padding = ( tall - vgui::surface()->GetFontTall( m_hButtonFont ) ) / 2;
		g_RD_Steam_Input.DrawLegacyControllerGlyph( m_szControllerButton, nBorder + padding, nBorder + padding, false, false, m_hButtonFont );
	}
}

void CNB_Button_Hold::PaintTraverse( bool Repaint, bool allowForce )
{
	bool bBeingHeld = m_flHoldProgress > 0.0f;
	int origX, origY;
	GetPos( origX, origY );

	if ( bBeingHeld )
	{
		float flHoldProgress = MIN( m_flHoldProgress, 1.0f );

		int offX = 0, offY = 0;
		offX = cosf( Bias( flHoldProgress, m_flShakeSpeedBiasX ) * m_flShakeSpeedX ) * Bias( flHoldProgress, m_flShakeAmountBiasX ) * m_flShakeAmountX;
		offY = sinf( Bias( flHoldProgress, m_flShakeSpeedBiasY ) * m_flShakeSpeedY ) * Bias( flHoldProgress, m_flShakeAmountBiasY ) * m_flShakeAmountY;

		SetPos( origX + offX, origY + offY );
		vgui::ipanel()->Solve( GetVPanel() );
	}

	BaseClass::PaintTraverse( Repaint, allowForce );

	if ( bBeingHeld )
	{
		SetPos( origX, origY );
		vgui::ipanel()->Solve( GetVPanel() );
	}
}

void CNB_Button_Hold::DoClick()
{
	SetSelected( false );
}

void CNB_Button_Hold::OnThink()
{
	BaseClass::OnThink();

	if ( m_iControllerButton != KEY_NONE )
	{
		ForceDepressed( g_pInputSystem->IsButtonDown( m_iControllerButton ) );
	}

	double now = Plat_FloatTime();
	double dt = clamp( now - m_flLastThink, 0.0f, 0.1f );
	m_flLastThink = now;

	if ( IsDepressed() && !m_bClickReset )
	{
		float flNextProgress = MAX( m_flHoldProgress, 0.0f ) + dt / m_flHoldTime;
		bool bFinishedHold = m_flHoldProgress < 1.0f && flNextProgress >= 1.0f;
		m_flHoldProgress = flNextProgress;
		Repaint();

		if ( bFinishedHold )
		{
			FireActionSignal();
		}
	}
	else
	{
		if ( m_flHoldProgress >= 1.0f )
		{
			m_bClickReset = true;
			m_flHoldProgress = 1.0f;
		}

		if ( m_flHoldProgress > -1.0f )
		{
			m_flHoldProgress = MAX( m_flHoldProgress - dt / m_flResetTime, -1.0f );
			Repaint();
		}

		if ( m_bClickReset && m_flHoldProgress <= 0.0f )
		{
			m_flHoldProgress = -1.0f;
			m_bClickReset = false;
		}
	}
}
