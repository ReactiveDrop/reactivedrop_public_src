#include "cbase.h"
#include "mission_complete_message.h"
#include "vgui_controls/Label.h"
#include "vgui_controls/ImagePanel.h"
#include "vgui_controls/TextEntry.h"
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include "asw_scalable_text.h"
#include "asw_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

CMission_Complete_Message::CMission_Complete_Message( vgui::Panel *parent, const char *name ) : BaseClass( parent, name )
{
	m_flMessageBackgroundStartTime = 0.0f;	
	m_flMessageBackgroundFadeDuration = 0.0f;
}

CMission_Complete_Message::~CMission_Complete_Message()
{
}

void CMission_Complete_Message::PerformLayout()
{
	BaseClass::PerformLayout();

	m_flStartWidth = m_flStartHeight = YRES( 400 );
	m_flEndWidth = m_flEndHeight = YRES( 105 );

	SetBounds( 0, 0, ScreenWidth(), ScreenHeight() );
}

void CMission_Complete_Message::Paint()
{
	BaseClass::Paint();

	PaintMessageBackground();
	PaintLetters();
}

void CMission_Complete_Message::PaintMessageBackground()
{
	float flLerpAmount = ( gpGlobals->curtime - m_flMessageBackgroundStartTime ) / ( m_flMessageBackgroundFadeDuration );
	flLerpAmount = clamp<float>( flLerpAmount, 0.0f, 1.0f );

	if ( m_bSuccess )
	{
		surface()->DrawSetColor( Color( 35, 41, 57, flLerpAmount * 128.0f ) );
		surface()->DrawSetColor( Color( 0, 0, 0, flLerpAmount * 128.0f ) );
	}
	else
	{
		surface()->DrawSetColor( Color( 64, 0, 0, flLerpAmount * 192.0f ) );
	}
	int top = ScreenHeight() * 0.22f;
	int bottom = ScreenHeight() * 0.62f;
	surface()->DrawFilledRect( 0, top, ScreenWidth() + 1, bottom );

	if ( !m_bSuccess )
	{
		// fail hint bg
		surface()->DrawSetColor( Color( 64, 0, 0, flLerpAmount * 192.0f ) );
		top = ScreenHeight() * 0.72f;
		bottom = ScreenHeight() * 0.82f;
		surface()->DrawFilledRect( 0, top, ScreenWidth() + 1, bottom );
	}
}

void CMission_Complete_Message::StartMessage( bool bSuccess )
{
	m_bSuccess = bSuccess;

	m_aAnimatingLetters.PurgeAndDeleteElements();

	m_flMessageBackgroundStartTime = gpGlobals->curtime + 0.4f;
	m_flMessageBackgroundFadeDuration = 0.6f;

	// MISSION
	float flStartTime = gpGlobals->curtime + 0.2f;
	if ( m_bSuccess && ASWGameRules() && ASWGameRules()->IsCampaignGame() && ASWGameRules()->CampaignMissionsLeft() <= 1 )
	{
		AddWord( g_pVGuiLocalize->FindSafe( "#asw_mission_complete_CAMPAIGN" ), ScreenWidth() * 0.5f, ScreenHeight() * 0.35f, flStartTime, 0.075f );
	}
	else
	{
		AddWord( g_pVGuiLocalize->FindSafe( "#asw_mission_complete_MISSION" ), ScreenWidth() * 0.5f, ScreenHeight() * 0.35f, flStartTime, 0.075f );
	}

	flStartTime += 0.2f;

	if ( m_bSuccess )
	{
		AddWord( g_pVGuiLocalize->FindSafe( "#asw_mission_complete_COMPLETE" ), ScreenWidth() * 0.5f, ScreenHeight() * 0.5f, flStartTime, 0.075f );
	}
	else
	{
		AddWord( g_pVGuiLocalize->FindSafe( "#asw_mission_complete_FAILED" ), ScreenWidth() * 0.5f, ScreenHeight() * 0.5f, flStartTime, 0.075f );
	}
}

void CMission_Complete_Message::AddWord( const wchar_t *wszWord, int row_middle_x, int row_middle_y, float &flStartTime, float flLetterTimeInterval )
{
	float flTotalWidth = 0;
	for ( const wchar_t *pLetter = wszWord; *pLetter; pLetter++ )
	{
		flTotalWidth += ASWScalableText()->GetLetterWidth( *pLetter );
	}

	float flLetterOffset = -flTotalWidth / 2.0f;
	for ( const wchar_t *pLetter = wszWord; *pLetter; pLetter++ )
	{
		float flHalfWidth = ASWScalableText()->GetLetterWidth( *pLetter ) / 2.0f;
		flLetterOffset += flHalfWidth;
		AddLetter( *pLetter, row_middle_x, row_middle_y, flLetterOffset, flStartTime );
		flLetterOffset += flHalfWidth;
		flStartTime += flLetterTimeInterval;
	}
}

void CMission_Complete_Message::AddLetter( wchar_t letter, int x, int y, float letter_offset, float flStartTime )
{
	const float letter_end_spacing = YRES( 50 );
	const float letter_start_spacing = YRES( 100 );
	const float flAnimTime = 0.35f;

	CAnimating_Letter *pLetter = new CAnimating_Letter;
	pLetter->m_chLetter = letter;
	pLetter->m_flStartTime = flStartTime;
	pLetter->m_flEndTime = flStartTime + flAnimTime;

	pLetter->m_flStartX = x + letter_offset * letter_start_spacing;
	pLetter->m_flStartY = y;
	pLetter->m_flEndX = x + letter_offset * letter_end_spacing;
	pLetter->m_flEndY = y;

	m_aAnimatingLetters.AddToTail( pLetter );
}

void CMission_Complete_Message::PaintLetters()
{
	int nCount = m_aAnimatingLetters.Count();
	for ( int i = 0; i < nCount; i++ )
	{
		PaintLetter( m_aAnimatingLetters[i], true );
	}
	for ( int i = 0; i < nCount; i++ )
	{
		PaintLetter( m_aAnimatingLetters[i], false );
	}
}

void CMission_Complete_Message::PaintLetter( CAnimating_Letter *pLetter, bool bGlow )
{
	float flLerpAmount = ( gpGlobals->curtime - pLetter->m_flStartTime ) / ( pLetter->m_flEndTime - pLetter->m_flStartTime );
	flLerpAmount = clamp<float>( flLerpAmount, 0.0f, 1.0f );
	flLerpAmount *= flLerpAmount;
	flLerpAmount *= flLerpAmount;

	float flX = pLetter->m_flStartX + ( pLetter->m_flEndX - pLetter->m_flStartX ) * flLerpAmount;
	float flY = pLetter->m_flStartY + ( pLetter->m_flEndY - pLetter->m_flStartY ) * flLerpAmount;
	float flWidth = m_flStartWidth + ( m_flEndWidth - m_flStartWidth ) * flLerpAmount;
	float flHeight = m_flStartHeight + ( m_flEndHeight - m_flStartHeight ) * flLerpAmount;
	flWidth *= 0.5f;
	flHeight *= 0.5f;

	if ( bGlow )
	{
		if ( m_bSuccess )
		{
			surface()->DrawSetColor( Color( 35, 214, 250, flLerpAmount * 112.0f ) );
		}
		else
		{
			surface()->DrawSetColor( Color( 250, 0, 0, flLerpAmount * 192.0f ) );
		}
	}
	else
	{
		surface()->DrawSetColor( Color( 255, 255, 255, flLerpAmount * 255.0f ) );
	}
	int iTexture = ASWScalableText()->GetLetterTexture( pLetter->m_chLetter, bGlow );
	surface()->DrawSetTexture( iTexture );

	Vertex_t points[4] =
	{
		Vertex_t( Vector2D( flX - flWidth, flY - flHeight ), Vector2D( 0,0 ) ),
		Vertex_t( Vector2D( flX + flWidth, flY - flHeight ), Vector2D( 1,0 ) ),
		Vertex_t( Vector2D( flX + flWidth, flY + flHeight ), Vector2D( 1,1 ) ),
		Vertex_t( Vector2D( flX - flWidth, flY + flHeight ), Vector2D( 0,1 ) )
	};
	surface()->DrawTexturedPolygon( 4, points );
}
