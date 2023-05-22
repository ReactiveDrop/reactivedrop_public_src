#include "cbase.h"
#include "rd_vgui_stock_ticker.h"
#include "asw_util_shared.h"
#include "steam/isteamuserstats.h"
#include <vgui/ILocalize.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <ctime>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

DECLARE_BUILD_FACTORY( CRD_VGUI_Stock_Ticker );

extern ConVar rd_reduce_motion;

static bool CheckTickerDefCorrectness( const char *szPath, KeyValues *pDef )
{
	float flWeight = pDef->GetFloat( "weight", 1 );
	if ( flWeight <= 0 )
	{
		Warning( "Invalid weight %f in %s\n", flWeight, szPath );
		return false;
	}

	if ( !V_strcmp( pDef->GetName(), "flavor" ) )
	{
		if ( pDef->GetString( "text" )[0] == '\0' )
		{
			Warning( "Empty flavor text in %s\n", szPath );
			return false;
		}

		return true;
	}

	if ( !V_strcmp( pDef->GetName(), "stock" ) )
	{
		if ( pDef->GetString( "name" )[0] == '\0' )
		{
			Warning( "Stock missing name in %s\n", szPath );
			return false;
		}

		if ( pDef->GetString( "symbol" )[0] == '\0' )
		{
			Warning( "Stock missing symbol in %s\n", szPath );
			return false;
		}

		return true;
	}

	if ( !V_strcmp( pDef->GetName(), "global_stat" ) )
	{
		int iDays = pDef->GetInt( "days", 30 );
		if ( iDays <= 0 || iDays > 60 )
		{
			Warning( "Global stat days %d is not between 1 and 60 in %s\n", iDays, szPath );
			return false;
		}

		if ( pDef->GetString( "stat_name" )[0] == '\0' )
		{
			Warning( "Missing global stat name in %s\n", szPath );
			return false;
		}

		if ( pDef->GetString( "text" )[0] == '\0' )
		{
			Warning( "Empty global stat text in %s\n", szPath );
			return false;
		}

		return true;
	}

	Warning( "Invalid ticker def type '%s' in %s\n", pDef->GetName(), szPath );
	return false;
}

static void AppendTickerDefsHelper( const char *szPath, KeyValues *pKV, void *pUserData )
{
	KeyValues *pDef;
	while ( ( pDef = pKV->GetFirstSubKey() ) != NULL )
	{
		pKV->RemoveSubKey( pDef );
		if ( CheckTickerDefCorrectness( szPath, pDef ) )
		{
			static_cast< KeyValues * >( pUserData )->AddSubKey( pDef );
		}
		else
		{
			KeyValuesDumpAsDevMsg( pDef );
			pDef->deleteThis();
		}
	}
}

CRD_VGUI_Stock_Ticker::CRD_VGUI_Stock_Ticker( vgui::Panel *parent, const char *panelName ) :
	BaseClass( parent, panelName ),
	m_pKVTickerDefs{ "TickerDefs" }
{
	m_flLastThink = 0;
	m_iFirstTextWidth = 0;
	m_iTextTotalWidth = 0;
	m_iLastRandomSeed = 0;
	m_wszTitle[0] = L'\0';
	m_hTickerFont = vgui::INVALID_FONT;
	m_hTickerBlurFont = vgui::INVALID_FONT;
	m_bLastReduceMotion = rd_reduce_motion.GetBool();

	KeyValues *pKVTickerDefs = m_pKVTickerDefs;
	UTIL_RD_LoadAllKeyValues( "resource/ticker.txt", "GAME", "TickerDefs", AppendTickerDefsHelper, pKVTickerDefs );

	m_flTotalWeight = 0;
	FOR_EACH_SUBKEY( pKVTickerDefs, pDef )
	{
		m_flTotalWeight += pDef->GetFloat( "weight", 1 );
	}
}

CRD_VGUI_Stock_Ticker::~CRD_VGUI_Stock_Ticker()
{
	FOR_EACH_VEC( m_TextBuffer, i )
	{
		delete[] m_TextBuffer[i];
	}
	m_TextBuffer.Purge();

	if ( vgui::surface() )
	{
		for ( unsigned i = 0; i < m_TickerDefTextures.Count(); i++ )
		{
			vgui::surface()->DestroyTextureID( m_TickerDefTextures[i] );
		}
		m_TickerDefTextures.Purge();
	}
}

void CRD_VGUI_Stock_Ticker::PerformLayout()
{
	BaseClass::PerformLayout();

	if ( m_flLastThink != 0 )
		return;

	if ( m_hTickerFont == vgui::INVALID_FONT )
		ApplySchemeSettings( vgui::scheme()->GetIScheme( GetScheme() ) );

	TryLocalize( "#rd_ticker_title", m_wszTitle, sizeof( m_wszTitle ) );
	int w, t;
	vgui::surface()->GetTextSize( m_hTickerFont, m_wszTitle, w, t );
	int iTitleWidth = m_iTitlePadding * 2 + m_iTitleAfterWidth + w + m_iTextStartXOffset;

	m_bLastReduceMotion = rd_reduce_motion.GetBool();
	if ( m_bLastReduceMotion )
	{
		m_flLastThink = -1;
		m_iTitleX = 0;
		m_iTextStartX = iTitleWidth;
		m_iBackgroundStartX = m_iTextStartX - m_iTitleAfterWidth;
	}
	else
	{
		m_flLastThink = Plat_FloatTime();
		m_iTitleX = GetWide() - iTitleWidth;
		m_iTextStartX = GetWide();
		m_iBackgroundStartX = m_iTextStartX - m_iTitleAfterWidth;
	}

	ManageTextBuffer();
}

void CRD_VGUI_Stock_Ticker::OnThink()
{
	if ( rd_reduce_motion.GetBool() != m_bLastReduceMotion )
	{
		// do a full reset if our initial state has become invalid
		m_flLastThink = 0;
		m_iFirstTextWidth = 0;
		m_iTextTotalWidth = 0;
		m_iLastRandomSeed = 0;
		FOR_EACH_VEC( m_TextBuffer, i )
		{
			delete[] m_TextBuffer[i];
		}
		m_TextBuffer.Purge();
		m_IconBuffer.Purge();

		InvalidateLayout( true );
		return;
	}

	if ( m_flLastThink == -1 )
	{
		int iCurrentSeed = std::time( NULL ) / 300;
		if ( m_iLastRandomSeed == iCurrentSeed )
			return;

		// for reduced motion mode, we update the entire ticker once every 5 minutes
		FOR_EACH_VEC( m_TextBuffer, i )
		{
			delete[] m_TextBuffer[i];
		}
		m_TextBuffer.Purge();
		m_IconBuffer.Purge();
		m_iFirstTextWidth = 0;
		m_iTextTotalWidth = 0;
		ManageTextBuffer();
		Repaint();

		return;
	}

	if ( m_flTickerSpeed <= 0 )
		return;

	float flDeltaTime = clamp<float>( Plat_FloatTime() - m_flLastThink, 0, 1 );
	int iShiftPixels = flDeltaTime * m_flTickerSpeed;
	if ( iShiftPixels == 0 )
		return;

	m_flLastThink += iShiftPixels / m_flTickerSpeed;

	m_iTitleX = MAX( m_iTitleX - iShiftPixels, 0 );
	m_iTextStartX -= iShiftPixels;
	m_iBackgroundStartX -= iShiftPixels;
	if ( m_iBackgroundStartX < -GetTall() )
		m_iBackgroundStartX += GetTall();

	ManageTextBuffer();
	Repaint();
}

void CRD_VGUI_Stock_Ticker::Paint()
{
	BaseClass::Paint();

	if ( !IsVisible() )
		return;

	vgui::surface()->DrawSetTextColor( GetFgColor() );
	vgui::surface()->DrawSetTextFont( m_hTickerFont );

	vgui::surface()->DrawSetColor( 255, 255, 255, 255 );
	vgui::surface()->DrawSetTexture( m_iBackgroundTexture );
	vgui::surface()->DrawTexturedSubRect( m_iBackgroundStartX, 0, m_iBackgroundStartX + GetWide() * 2, GetTall(), 0, 0, GetWide() * 2.0f / GetTall(), 1 );

	int x = m_iTextStartX;
	int iIconY = ( GetTall() - m_iIconSize ) / 2;
	FOR_EACH_VEC( m_TextBuffer, i )
	{
		if ( m_IconBuffer[i] != -1 )
		{
			vgui::surface()->DrawSetTexture( m_IconBuffer[i] );
			vgui::surface()->DrawTexturedRect( x + m_iIconXPos, iIconY, x + m_iIconSize + m_iIconXPos, iIconY + m_iIconSize );
			x += m_iIconSize;
		}
		vgui::surface()->DrawSetTextPos( x, m_iTextY );
		vgui::surface()->DrawUnicodeString( m_TextBuffer[i] );
		int w, t;
		vgui::surface()->GetTextSize( m_hTickerFont, m_TextBuffer[i], w, t );
		x += w;
		vgui::surface()->DrawSetTexture( m_iSeparatorTexture );
		vgui::surface()->DrawTexturedRect( x, 0, x + m_iSeparatorWidth, GetTall() );
		x += m_iSeparatorWidth;
	}

	int w, t;
	vgui::surface()->GetTextSize( m_hTickerFont, m_wszTitle, w, t );
	w += m_iTitlePadding * 2;

	if ( m_iTitleX )
	{
		vgui::surface()->DrawSetTexture( m_iTitleBeforeTexture );
		vgui::surface()->DrawTexturedSubRect( 0, 0, m_iTitleX, GetTall(), 1 - m_iTitleX / float( GetTall() ), 0, 1, 1 );
	}
	vgui::surface()->DrawSetTexture( m_iTitleBackgroundTexture );
	vgui::surface()->DrawTexturedSubRect( m_iTitleX, 0, m_iTitleX + w, GetTall(), 0, 0, w / float( GetTall() ), 1 );
	vgui::surface()->DrawSetTexture( m_iTitleAfterTexture );
	vgui::surface()->DrawTexturedRect( m_iTitleX + w, 0, m_iTitleX + w + m_iTitleAfterWidth, GetTall() );

	vgui::surface()->DrawSetTextColor( 255, 255, 255, 60 );
	vgui::surface()->DrawSetTextFont( m_hTickerBlurFont );
	vgui::surface()->DrawSetTextPos( m_iTitleX + m_iTitlePadding, m_iTextY );
	vgui::surface()->DrawUnicodeString( m_wszTitle );

	vgui::surface()->DrawSetTextColor( 255, 255, 255, 255 );
	vgui::surface()->DrawSetTextFont( m_hTickerFont );
	vgui::surface()->DrawSetTextPos( m_iTitleX + m_iTitlePadding, m_iTextY );
	vgui::surface()->DrawUnicodeString( m_wszTitle );
}

void CRD_VGUI_Stock_Ticker::ManageTextBuffer()
{
	while ( m_iTextStartX <= -m_iFirstTextWidth )
	{
		// safety
		if ( m_TextBuffer.Count() == 0 )
			break;

		delete[] m_TextBuffer.RemoveAtHead();
		m_IconBuffer.RemoveAtHead();
		Assert( m_TextBuffer.Count() == m_IconBuffer.Count() );

		m_iTextStartX += m_iFirstTextWidth;
		m_iTextTotalWidth -= m_iFirstTextWidth;
		m_iFirstTextWidth = 0;
		Assert( m_iTextTotalWidth >= 0 );

		if ( m_TextBuffer.Count() != 0 )
		{
			int w, t;
			vgui::surface()->GetTextSize( m_hTickerFont, m_TextBuffer.Head(), w, t );
			m_iFirstTextWidth = w + m_iSeparatorWidth;

			if ( m_IconBuffer.Head() != -1 )
				m_iFirstTextWidth += m_iIconSize;
		}
	}

	while ( m_iTextStartX + m_iTextTotalWidth < GetWide() )
	{
		wchar_t *wszText = NULL;
		int iIconTexture = -1;
		GenerateNextTickerText( wszText, iIconTexture );
		if ( !wszText )
			break;

		int w, t;
		vgui::surface()->GetTextSize( m_hTickerFont, wszText, w, t );
		w += m_iSeparatorWidth;
		if ( iIconTexture != -1 )
			w += m_iIconSize;
		m_iTextTotalWidth += w;

		if ( m_TextBuffer.Count() == 0 )
			m_iFirstTextWidth = w;

		m_TextBuffer.Insert( wszText );
		m_IconBuffer.Insert( iIconTexture );
	}
}

void CRD_VGUI_Stock_Ticker::GenerateNextTickerText( wchar_t *&wszText, int &iIconTexture )
{
	if ( m_flTotalWeight <= 0 )
	{
		wszText = NULL;
		iIconTexture = -1;

		return;
	}

	// use the current 5-minute interval as the seed so players (mostly) see the same messages
	int iCurrentSeed = std::time( NULL ) / 300;
	if ( m_iLastRandomSeed != iCurrentSeed )
	{
		m_RandomStream.SetSeed( iCurrentSeed );
		m_iLastRandomSeed = iCurrentSeed;
		m_TickerDefCooldown.Purge();

		if ( ISteamUserStats *pUserStats = SteamUserStats() )
		{
			( void )pUserStats->RequestGlobalStats( 60 );
		}
	}

	float flTotalWeight = m_flTotalWeight;
	FOR_EACH_VEC( m_TickerDefCooldown, i )
	{
		flTotalWeight -= m_TickerDefCooldown[i]->GetFloat( "weight", 1.0f );
	}

	if ( m_TickerDefCooldown.Count() >= 15 || ( m_TickerDefCooldown.Count() > 0 && flTotalWeight <= 1.0f ) )
	{
		flTotalWeight += m_TickerDefCooldown.RemoveAtHead()->GetFloat( "weight", 1.0f );
	}

	float flWeight = m_RandomStream.RandomFloat( 0, flTotalWeight );
	KeyValues *pChosenDef = NULL;
	FOR_EACH_SUBKEY( m_pKVTickerDefs, pDef )
	{
		bool bSkip = false;
		FOR_EACH_VEC( m_TickerDefCooldown, i )
		{
			if ( m_TickerDefCooldown[i] == pDef )
			{
				bSkip = true;
				break;
			}
		}

		if ( bSkip )
			continue;

		pChosenDef = pDef;
		flWeight -= pDef->GetFloat( "weight", 1.0f );
		if ( flWeight <= 0 )
			break;
	}

	// if we exited early, we probably hit a rounding error
	Assert( flWeight < 1.0f );

	m_TickerDefCooldown.Insert( pChosenDef );

	GenerateTickerText( pChosenDef, wszText, iIconTexture );
}

void CRD_VGUI_Stock_Ticker::GenerateTickerText( KeyValues *pDef, wchar_t *&wszText, int &iIconTexture )
{
	wszText = NULL;
	iIconTexture = -1;

	Assert( pDef );
	if ( !pDef )
		return;

	if ( const char *szIcon = pDef->GetString( "icon", NULL ) )
	{
		int i = m_TickerDefTextures.Find( szIcon );
		if ( !m_TickerDefTextures.IsValidIndex( i ) )
		{
			i = m_TickerDefTextures.Insert( szIcon );
			m_TickerDefTextures[i] = vgui::surface()->CreateNewTextureID();
			vgui::surface()->DrawSetTextureFile( m_TickerDefTextures[i], szIcon, 1, false );
		}

		iIconTexture = m_TickerDefTextures[i];
	}

	wszText = new wchar_t[1024];
#define SIZEOF_WSZTEXT sizeof( wchar_t[1024] )

	if ( !V_strcmp( pDef->GetName(), "flavor" ) )
	{
		TryLocalize( pDef->GetString( "text" ), wszText, SIZEOF_WSZTEXT );
		return;
	}

	if ( !V_strcmp( pDef->GetName(), "stock" ) )
	{
		float flCurrent = pDef->GetFloat( "base", 50 );
		int iVariance = pDef->GetInt( "variance", 500 );
		flCurrent += m_RandomStream.RandomInt( -iVariance, iVariance ) / 100.0f;
		int iMaxChange = pDef->GetInt( "maxchange", 400 );
		float flChange = m_RandomStream.RandomInt( -iMaxChange, iMaxChange ) / 100.0f;
		flCurrent += flChange;

		wchar_t wszCurrent[32];
		V_snwprintf( wszCurrent, NELEMS( wszCurrent ), L"%.2f", flCurrent );
		wchar_t wszChange[32];
		V_snwprintf( wszChange, NELEMS( wszChange ), L"%.2f", fabsf( flChange ) );
		wchar_t wszName[128];
		TryLocalize( pDef->GetString( "name" ), wszName, sizeof( wszName ) );
		wchar_t wszSymbol[32];
		TryLocalize( pDef->GetString( "symbol" ), wszSymbol, sizeof( wszSymbol ) );

		g_pVGuiLocalize->ConstructString( wszText, SIZEOF_WSZTEXT, g_pVGuiLocalize->Find( flChange < 0 ? "#rd_ticker_stock_down" : "#rd_ticker_stock_up" ), 4, wszName, wszSymbol, wszChange, wszCurrent );
		if ( iIconTexture == -1 )
			iIconTexture = flChange < 0 ? m_iStockDownTexture : m_iStockUpTexture;

		return;
	}

	if ( !V_strcmp( pDef->GetName(), "global_stat" ) )
	{
		int iDays = pDef->GetInt( "days", 30 );
		Assert( iDays <= 60 );

		ISteamUserStats *pUserStats = SteamUserStats();
		if ( !pUserStats )
		{
			delete[] wszText;
			wszText = NULL;

			return;
		}

		int64 iTotalStat = 0;
		FOR_EACH_VALUE( pDef, pValue )
		{
			if ( !FStrEq( pValue->GetName(), "stat_name" ) )
				continue;

			int64 iStatHistory[60];
			int iDaysRetrieved = pUserStats->GetGlobalStatHistory( pValue->GetString(), iStatHistory, sizeof( iStatHistory ) );
			if ( iDaysRetrieved < iDays )
			{
				// failed to retrieve stats; will try again next time this comes up
				delete[] wszText;
				wszText = NULL;

				return;
			}

			for ( int i = 0; i < iDays; i++ )
			{
				iTotalStat += iStatHistory[i];
			}
		}

		wchar_t wszTemplate[1024];
		TryLocalize( pDef->GetString( "text" ), wszTemplate, sizeof( wszTemplate ) );

		g_pVGuiLocalize->ConstructString( wszText, SIZEOF_WSZTEXT, wszTemplate, 1, UTIL_RD_CommaNumber( iTotalStat ) );

		return;
	}

	Assert( !"unhandled ticker def" );
	delete[] wszText;
	wszText = NULL;
}
