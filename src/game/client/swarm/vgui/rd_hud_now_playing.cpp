#include "cbase.h"
#include "rd_hud_now_playing.h"
#include "clientmode_asw.h"
#include "c_asw_jukebox.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

DECLARE_HUDELEMENT_FLAGS( CRD_HUD_Now_Playing, HUDELEMENT_SS_FULLSCREEN_ONLY );

CRD_HUD_Now_Playing::CRD_HUD_Now_Playing( const char *pElementName) :
	CASW_HudElement( pElementName ),
	BaseClass( NULL, pElementName )
{
	SetParent( GetFullscreenClientMode()->GetViewport() );
	SetHiddenBits( HIDEHUD_MISCSTATUS );

	vgui::HScheme scheme = vgui::scheme()->LoadSchemeFromFile( "resource/SwarmSchemeNew.res", "SwarmSchemeNew" );
	SetScheme( scheme );

	m_pLblTrackName = new vgui::Label( this, "LblTrackName", L"" );
	m_pLblAlbumName = new vgui::Label( this, "LblAlbumName", L"" );
	m_pLblArtistName = new vgui::Label( this, "LblArtistName", L"" );
}

void CRD_HUD_Now_Playing::ShowAfterDelay( float flDelay )
{
	HideEarly();

	m_flFadeInStart = MAX( m_flFadeOutEnd, gpGlobals->realtime + m_flDelayTime + flDelay );
	m_flFadeInEnd = m_flFadeInStart + m_flFadeInTime;

	m_flUpdateLabelsAfter = m_flFadeOutEnd;
	if ( m_flUpdateLabelsAfter < 0 )
		UpdateLabels();
}

void CRD_HUD_Now_Playing::HideEarly()
{
	m_flFadeInStart = -1;
	m_flFadeInEnd = -1;

	if ( GetAlpha() == 0 )
	{
		m_flFadeOutStart = -1;
		m_flFadeOutEnd = -1;
	}
	else
	{
		m_flFadeOutEnd = gpGlobals->realtime + RemapVal( GetAlpha(), 0, 255, 0, m_flFadeOutTime );
		m_flFadeOutStart = m_flFadeOutEnd - m_flFadeOutTime;
	}
}

void CRD_HUD_Now_Playing::HideImmediately()
{
	m_flFadeInStart = -1;
	m_flFadeInEnd = -1;
	m_flFadeOutStart = -1;
	m_flFadeOutEnd = -1;
	m_flUpdateLabelsAfter = -1;

	SetAlpha( 0 );
}

void CRD_HUD_Now_Playing::UpdateLabels()
{
	if ( g_ASWJukebox.m_wszTrackName[0] == L'\0' )
	{
		HideEarly();

		return;
	}

	m_pLblTrackName->SetText( g_ASWJukebox.m_wszTrackName );
	m_pLblAlbumName->SetText( g_ASWJukebox.m_wszAlbumName );
	m_pLblArtistName->SetText( g_ASWJukebox.m_wszArtistName );

	Assert( m_flFadeInEnd >= 0 );

	m_flFadeOutStart = m_flFadeInEnd + m_flSustainTime;
	m_flFadeOutEnd = m_flFadeOutStart + m_flFadeOutTime;
}

void CRD_HUD_Now_Playing::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	LoadControlSettings( "Resource/UI/CRD_HUD_Now_Playing.res" );

	HideImmediately();
}

void CRD_HUD_Now_Playing::OnThink()
{
	BaseClass::OnThink();

	if ( m_flUpdateLabelsAfter >= 0 && m_flUpdateLabelsAfter <= gpGlobals->realtime )
	{
		m_flUpdateLabelsAfter = -1;
		UpdateLabels();
	}

	if ( m_flFadeInStart >= 0 && m_flFadeInStart <= gpGlobals->realtime )
	{
		Assert( m_flFadeInEnd >= m_flFadeInStart );
		SetAlpha( RemapValClamped( gpGlobals->realtime, m_flFadeInStart, m_flFadeInEnd, 0, 255 ) );

		if ( gpGlobals->realtime >= m_flFadeInEnd )
		{
			m_flFadeInStart = -1;
			m_flFadeInEnd = -1;
		}
	}

	if ( m_flFadeOutStart >= 0 && m_flFadeOutStart <= gpGlobals->realtime )
	{
		Assert( m_flFadeOutEnd >= m_flFadeOutStart );
		SetAlpha( RemapValClamped( gpGlobals->realtime, m_flFadeOutStart, m_flFadeOutEnd, 255, 0 ) );

		if ( gpGlobals->realtime >= m_flFadeOutEnd )
		{
			m_flFadeOutStart = -1;
			m_flFadeOutEnd = -1;
		}
	}
}
