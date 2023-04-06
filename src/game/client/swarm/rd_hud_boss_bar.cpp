#include "cbase.h"
#include "rd_hud_boss_bar.h"
#include "c_rd_boss_bar.h"
#include "clientmode_asw.h"
#include "VGuiMatSurface/IMatSystemSurface.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


ConVar rd_paint_boss_bars( "rd_paint_boss_bars", "1", FCVAR_ARCHIVE, "Draw boss bars on the HUD when rd_boss_bar entities are active" );

#define BAR_HEIGHT_BASE 15
#define BAR_PADDING 2
#define BAR_TITLE_TALL 25
#define BAR_CONTAINER_WIDTH 500

DECLARE_HUDELEMENT( CRD_Hud_Boss_Bars );

CRD_Hud_Boss_Bars::CRD_Hud_Boss_Bars( const char *pElementName ) : CASW_HudElement( pElementName ), BaseClass( NULL, "RD_Hud_Boss_Bars" )
{
	SetParent( GetClientMode()->GetViewport() );
	SetScheme( vgui::scheme()->LoadSchemeFromFile( "resource/SwarmSchemeNew.res", "SwarmSchemeNew" ) );
}

CRD_Hud_Boss_Bars::~CRD_Hud_Boss_Bars()
{
}

void CRD_Hud_Boss_Bars::LevelInit() { ClearAllBossBars(); }
void CRD_Hud_Boss_Bars::LevelShutdown() { ClearAllBossBars(); }

void CRD_Hud_Boss_Bars::PerformLayout()
{
	SetPos( ( ScreenWidth() - scheme()->GetProportionalScaledValueEx( GetScheme(), BAR_CONTAINER_WIDTH ) ) / 2, scheme()->GetProportionalScaledValueEx( GetScheme(), 20 ) );
	SetSize( scheme()->GetProportionalScaledValueEx( GetScheme(), BAR_CONTAINER_WIDTH ), scheme()->GetProportionalScaledValueEx( GetScheme(), 250 ) );

	int iNumVisible = 0;
	FOR_EACH_VEC_BACK( m_Containers, i )
	{
		if ( !m_Containers[i] )
		{
			m_Containers.Remove( i );
			continue;
		}

		if ( m_Containers[i]->IsVisible() )
		{
			iNumVisible++;
		}
	}

	if ( iNumVisible != 0 )
	{
		int iVisibleIndex = 0;
		FOR_EACH_VEC( m_Containers, i )
		{
			if ( !m_Containers[i]->IsVisible() )
			{
				continue;
			}

			int wide, tall;
			m_Containers[i]->GetMinimumSize( wide, tall );
			m_Containers[i]->SetMinimumSize( GetWide() / iNumVisible, tall );
			m_Containers[i]->SetSize( GetWide() / iNumVisible, tall );
			m_Containers[i]->SetPos( GetWide() * iVisibleIndex / iNumVisible, 0);

			iVisibleIndex++;
		}
	}

	BaseClass::PerformLayout();
}

void CRD_Hud_Boss_Bars::ClearAllBossBars()
{
	FOR_EACH_VEC( m_Containers, i )
	{
		CRD_Hud_Boss_Bar_Container *pContainer = m_Containers[i];
		if ( pContainer )
		{
			pContainer->MarkForDeletion();
		}
	}

	m_Containers.Purge();
	m_IdentifierPool.FreeAll();
}

void CRD_Hud_Boss_Bars::OnBossBarEntityChanged( C_RD_Boss_Bar *pBar, bool bCreated )
{
	CRD_Hud_Boss_Bar_Container *pContainer = FindBarContainer( pBar->m_iszBarID, bCreated );
	Assert( pContainer );
	if ( !pContainer )
	{
		return;
	}

	if ( bCreated )
	{
		pContainer->m_BarEntities.AddToTail( pBar );
	}

	pContainer->OnBarDataChanged();

	InvalidateLayout();
}

CRD_Hud_Boss_Bar_Container *CRD_Hud_Boss_Bars::FindBarContainer( const char *pszIdentifier, bool bAllowCreate )
{
	const char *pIdentifier = m_IdentifierPool.Allocate( pszIdentifier );
	FOR_EACH_VEC( m_Containers, i )
	{
		CRD_Hud_Boss_Bar_Container *pContainer = m_Containers[i];
		if ( pContainer && pContainer->m_pIdentifier == pIdentifier )
		{
			return pContainer;
		}
	}

	if ( !bAllowCreate )
	{
		return NULL;
	}

	CRD_Hud_Boss_Bar_Container *pContainer = new CRD_Hud_Boss_Bar_Container( this, pIdentifier );
	m_Containers[m_Containers.AddToTail()] = pContainer;

	return pContainer;
}

CRD_Hud_Boss_Bar_Container::CRD_Hud_Boss_Bar_Container( CRD_Hud_Boss_Bars *pParent, const char *pIdentifier ) : BaseClass( pParent, "BossBarContainer" )
{
	m_pIdentifier = pIdentifier;
	m_pLabel = new vgui::Label( this, "Boss_Bar_Caption", "" );
}

CRD_Hud_Boss_Bar_Container::~CRD_Hud_Boss_Bar_Container()
{
}

static int __cdecl SortBarsByRowAndColumn(const CHandle<C_RD_Boss_Bar> *a, const CHandle<C_RD_Boss_Bar> *b)
{
	C_RD_Boss_Bar *pA = *a;
	C_RD_Boss_Bar *pB = *b;

	if ( pA->m_iBarRow != pB->m_iBarRow )
	{
		return pA->m_iBarRow < pB->m_iBarRow ? -1 : 1;
	}

	if ( pA->m_iBarColumn != pB->m_iBarColumn )
	{
		return pA->m_iBarColumn < pB->m_iBarColumn ? -1 : 1;
	}

	return pA->entindex() < pB->entindex() ? -1 : 1;
}

void CRD_Hud_Boss_Bar_Container::OnBarDataChanged()
{
	bool bAnyVisible = false;

	FOR_EACH_VEC_BACK( m_BarEntities, i )
	{
		C_RD_Boss_Bar *pBar = m_BarEntities[i];
		if ( !pBar )
		{
			m_BarEntities.Remove( i );

			continue;
		}

		bAnyVisible = bAnyVisible || ( pBar->m_bEnabled && !pBar->IsTooFarAway() );
	}

	if ( m_BarEntities.Count() == 0 )
	{
		MarkForDeletion();
	}

	if ( !bAnyVisible )
	{
		SetVisible( false );

		return;
	}

	SetVisible( true );

	m_BarEntities.Sort( &SortBarsByRowAndColumn );
	m_BarLayout.SetSize( m_BarEntities.Count() );

	int iRowStart = 0;
	while ( !m_BarEntities[iRowStart]->m_bEnabled )
	{
		iRowStart++;
	}

	m_pLabel->SetText( m_BarEntities[iRowStart]->m_iszBarTitle );

	float flRowTop = 0;
	int iCurrentRow = m_BarEntities[iRowStart]->m_iBarRow;
	int iCurrentColumn = m_BarEntities[iRowStart]->m_iBarColumn - 1;
	float flMaxHeightScale = MAX( m_BarEntities[iRowStart]->m_flBarHeightScale, 0 );
	int iColumnCount = 0;

	m_BarLayout[iRowStart].m_iColumnIndex = 0;

	for ( int i = iRowStart; ; i++ )
	{
		C_RD_Boss_Bar *pBar = i < m_BarEntities.Count() ? m_BarEntities[i] : NULL;
		if ( pBar && !pBar->m_bEnabled )
		{
			continue;
		}

		if ( !pBar || pBar->m_iBarRow != iCurrentRow )
		{
			for ( int j = iRowStart; j < i; j++ )
			{
				m_BarLayout[j].m_flRowHeight = flMaxHeightScale;
				m_BarLayout[j].m_iColumnCount = iColumnCount;
			}

			if ( !pBar )
			{
				break;
			}

			flRowTop += flMaxHeightScale + 0.1;
			iRowStart = i;
			iCurrentRow = pBar->m_iBarRow;
			iCurrentColumn = pBar->m_iBarColumn;
			flMaxHeightScale = MAX( pBar->m_flBarHeightScale, 0 );
			iColumnCount = 1;
		}

		if ( pBar->m_iBarColumn != iCurrentColumn )
		{
			iCurrentColumn = pBar->m_iBarColumn;
			iColumnCount++;
		}

		flMaxHeightScale = MAX( flMaxHeightScale, pBar->m_flBarHeightScale );

		m_BarLayout[i].m_flRowTop = flRowTop;
		m_BarLayout[i].m_iColumnIndex = iColumnCount - 1;
	}

	m_flBarAreaHeight = flRowTop + flMaxHeightScale;

	SetMinimumSize( scheme()->GetProportionalScaledValueEx( GetScheme(), BAR_CONTAINER_WIDTH ), scheme()->GetProportionalScaledValueEx( GetScheme(), int( m_flBarAreaHeight * BAR_HEIGHT_BASE ) + BAR_TITLE_TALL + BAR_PADDING ) );
	InvalidateLayout();
}

void CRD_Hud_Boss_Bar_Container::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	m_hFont = pScheme->GetFont( "DefaultLarge", true );

	SetBgColor( GetSchemeColor( "TransparentBlack", pScheme ) );
	SETUP_PANEL( m_pLabel );
	m_pLabel->SetFont( m_hFont );
	m_pLabel->SetTall( scheme()->GetProportionalScaledValueEx( GetScheme(), BAR_TITLE_TALL ) );
	m_pLabel->SetContentAlignment( vgui::Label::a_center );
}

void CRD_Hud_Boss_Bar_Container::Paint()
{
	if ( !rd_paint_boss_bars.GetBool() )
	{
		SetPaintBackgroundEnabled( false );
		m_pLabel->SetVisible( false );
		return;
	}

	SetPaintBackgroundEnabled( true );
	m_pLabel->SetVisible( true );

	BaseClass::Paint();

	int x0 = 0, y0 = 0, x1, y1;
	GetSize( x1, y1 );

	int iProportionalPadding = scheme()->GetProportionalScaledValueEx( GetScheme(), BAR_PADDING );
	
	x0 += iProportionalPadding;
	y0 += m_pLabel->GetTall();
	x1 -= iProportionalPadding;
	y1 -= iProportionalPadding;

	FOR_EACH_VEC( m_BarEntities, i )
	{
		C_RD_Boss_Bar *pBar = m_BarEntities[i];
		const Layout_t & layout = m_BarLayout[i];

		if ( !pBar || !pBar->m_bEnabled || pBar->IsTooFarAway() )
		{
			continue;
		}

		int iBarWidthPadded = ( x1 - x0 + iProportionalPadding ) / layout.m_iColumnCount;
		int iBarWidth = iBarWidthPadded - iProportionalPadding;

		float flHeight = MAX( pBar->m_flBarHeightScale * BAR_HEIGHT_BASE, 0 );
		int iBarHeight = scheme()->GetProportionalScaledValueEx( GetScheme(), int( flHeight ) );

		int x = x0 + iBarWidthPadded * layout.m_iColumnIndex;
		int y = y0 + scheme()->GetProportionalScaledValueEx( GetScheme(), int( layout.m_flRowTop * BAR_HEIGHT_BASE + ( layout.m_flRowHeight * BAR_HEIGHT_BASE - flHeight ) / 2 ) );

		int iPadAfter = layout.m_iColumnCount - ( x1 - x0 + iProportionalPadding ) % layout.m_iColumnCount;
		if ( layout.m_iColumnIndex >= iPadAfter )
		{
			x += layout.m_iColumnIndex - iPadAfter;
			iBarWidth++;
		}

		float flProgress = pBar->m_flBarValue > 0 ? pBar->m_flBarMaxValue > pBar->m_flBarValue ? pBar->m_flBarValue / pBar->m_flBarMaxValue : 1 : 0;
		float flLastProgress = pBar->m_flBarValuePrev > 0 ? pBar->m_flBarMaxValue > pBar->m_flBarValuePrev ? pBar->m_flBarValuePrev / pBar->m_flBarMaxValue : 1 : 0;
		float flSinceChange = gpGlobals->curtime - pBar->m_flBarValueLastChanged;
		float flSustain = MAX( pBar->m_flBarFlashSustain, 0 );
		float flInterpolate = MAX( pBar->m_flBarFlashInterpolate, 0 );
		float flFlashProgress = flSinceChange > flSustain ? flSinceChange < flSustain + flInterpolate ? ( flSinceChange - flSustain ) / flInterpolate : 1 : 0;
		int iScaledBarWidth = int( iBarWidth * flProgress );
		int iFlashBarWidth = int( ( 1 - flFlashProgress ) * ( flLastProgress - flProgress ) * iBarWidth );

		surface()->DrawSetColor( pBar->m_BarBGColor );
		surface()->DrawFilledRect( x, y, x + iBarWidth, y + iBarHeight );

		switch ( pBar->m_BarMode )
		{
		case BOSS_BAR_FILL_TO_RIGHT:
		{
			surface()->DrawSetColor( pBar->m_BarFGColor );
			surface()->DrawFilledRect( x, y, x + iScaledBarWidth, y + iBarHeight );

			surface()->DrawSetColor( pBar->m_BarFlashColor );
			if ( iFlashBarWidth > 0 )
			{
				surface()->DrawFilledRect( x + iScaledBarWidth, y, x + iScaledBarWidth + iFlashBarWidth, y + iBarHeight );
			}
			else if ( iFlashBarWidth < 0 )
			{
				surface()->DrawFilledRect( x + iScaledBarWidth + iFlashBarWidth, y, x + iScaledBarWidth, y + iBarHeight );
			}

			break;
		}
		case BOSS_BAR_FILL_TO_LEFT:
		{
			surface()->DrawSetColor( pBar->m_BarFGColor );
			surface()->DrawFilledRect( x + iBarWidth - iScaledBarWidth, y, x + iBarWidth, y + iBarHeight );

			surface()->DrawSetColor( pBar->m_BarFlashColor );
			if ( iFlashBarWidth > 0 )
			{
				surface()->DrawFilledRect( x + iBarWidth - iScaledBarWidth - iFlashBarWidth, y, x + iBarWidth - iScaledBarWidth, y + iBarHeight );
			}
			else if ( iFlashBarWidth < 0 )
			{
				surface()->DrawFilledRect( x + iBarWidth - iScaledBarWidth, y, x + iBarWidth - iScaledBarWidth - iFlashBarWidth, y + iBarHeight );
			}

			break;
		}
		case BOSS_BAR_NUMERIC_VALUE:
		{
			wchar_t wszNumericValue[12];
			int iNumberCharCount = V_snwprintf( wszNumericValue, ARRAYSIZE( wszNumericValue ), L"%d", int( MAX( pBar->m_flBarValue, 0 ) ) );

			int iTextWidth, iTextHeight;
			g_pMatSystemSurface->GetTextSize( m_hFont, wszNumericValue, iTextWidth, iTextHeight );

			surface()->DrawSetTextFont( m_hFont );
			surface()->DrawSetTextColor( pBar->m_BarFGColor );
			surface()->DrawSetTextPos( x + ( iBarWidth - iTextWidth ) / 2, y );
			surface()->DrawPrintText( wszNumericValue, iNumberCharCount );

			if ( flFlashProgress < 1 && flProgress != flLastProgress )
			{
				float flAlpha = surface()->DrawGetAlphaMultiplier();

				surface()->DrawSetAlphaMultiplier( flAlpha * ( 1 - flFlashProgress ) );

				surface()->DrawSetTextPos( x + ( iBarWidth - iTextWidth ) / 2, y );
				surface()->DrawSetTextColor( pBar->m_BarFlashColor );
				surface()->DrawPrintText( wszNumericValue, iNumberCharCount );

				surface()->DrawSetAlphaMultiplier( flAlpha );
			}

			break;
		}
		}

		surface()->DrawSetColor( pBar->m_BarBorderColor );
		surface()->DrawOutlinedRect( x, y, x + iBarWidth, y + iBarHeight );
	}
}

void CRD_Hud_Boss_Bar_Container::PerformLayout()
{
	BaseClass::PerformLayout();

	m_pLabel->SetWide( GetWide() );
}
