#include "cbase.h"
#include "tabbedgriddetails.h"
#include <vgui_controls/Label.h>
#include <vgui_controls/ScrollBar.h>
#include "ienginevgui.h"
#include "nb_button.h"
#include "nb_header_footer.h"
#include "iclientmode.h"
#include "gameui/swarm/basemodui.h"
#include "inputsystem/iinputsystem.h"
#include <vgui/IInput.h>
#include "rd_steam_input.h"
#include "rd_vgui_main_menu_top_bar.h"
#include "rd_vgui_stock_ticker.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


TabbedGridDetails::TabbedGridDetails() : BaseClass( NULL, "TabbedGridDetails" )
{
	SetConsoleStylePanel( true );
	SetProportional( true );
	SetSizeable( false );
	SetCloseButtonVisible( false );
	SetMenuButtonVisible( false );

	vgui::HScheme scheme = vgui::scheme()->LoadSchemeFromFile( "resource/SwarmSchemeNew.res", "SwarmSchemeNew" );
	SetScheme( scheme );

	m_pHeaderFooter = new CNB_Header_Footer( this, "HeaderFooter" );
	m_pHeaderFooter->SetGradientBarPos( 40, 410 );
	m_pHeaderFooter->SetBackgroundStyle( NB_BACKGROUND_BLUE );
	m_pHeaderFooter->SetTitle( L"" );

	m_pBackButton = new CNB_Button( this, "BackButton", "#L4D360UI_Back_Caps", this, "BackButton" );
	m_pBackButton->SetControllerButton( KEY_XBUTTON_B );

	m_pTabStrip = new vgui::Panel( this, "TabStrip" );
	m_pTabLeftHint = new vgui::Label( this, "TabLeftHint", "#GameUI_Icons_LEFT_BUMPER" );
	m_pTabRightHint = new vgui::Label( this, "TabRightHint", "#GameUI_Icons_RIGHT_BUMPER" );

	m_pMainMenuBar = NULL;
	m_pMainMenuTicker = NULL;
	m_pLastTabConVar = NULL;
	m_pGridParent = NULL;
	m_pCombinedScrollBar = NULL;
}

void TabbedGridDetails::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	LoadControlSettings( m_pCombinedScrollBar ? "Resource/UI/CombinedGridDetails.res" : "Resource/UI/TabbedGridDetails.res" );

	BaseClass::ApplySchemeSettings( pScheme );

	if ( m_pCombinedScrollBar )
		m_pCombinedScrollBar->UseImages( "scroll_up", "scroll_down", "scroll_line", "scroll_box" );
}

void TabbedGridDetails::PerformLayout()
{
	BaseClass::PerformLayout();

	if ( m_hCurrentTab )
	{
		m_hCurrentTab->m_pGrid->SetVisible( !m_hOverridePanel );
	}

	if ( m_hOverridePanel )
	{
		m_pTabLeftHint->SetVisible( false );
		m_pTabRightHint->SetVisible( false );

		FOR_EACH_VEC( m_Tabs, i )
		{
			m_Tabs[i]->SetVisible( false );
		}

		return;
	}

	bool bHaveGamepad = g_RD_Steam_Input.GetJoystickCount() > 0 && m_Tabs.Count() > 1;
	m_pTabLeftHint->SetVisible( bHaveGamepad );
	m_pTabRightHint->SetVisible( bHaveGamepad );

	int x, y;
	int hx, hy;
	m_pTabLeftHint->GetPos( hx, hy );
	m_pTabStrip->GetPos( x, y );

	if ( hx < 0 )
	{
		x -= hx;
		hx = 0;
		m_pTabLeftHint->SetPos( hx, hy );
	}

	hx = x - ( hx + m_pTabLeftHint->GetWide() );

	FOR_EACH_VEC( m_Tabs, i )
	{
		m_Tabs[i]->SetVisible( true );
		m_Tabs[i]->SetPos( x, y );
		x += m_Tabs[i]->GetWide() + YRES( 2 );
	}

	m_pTabRightHint->SetPos( x + hx, hy );

	if ( m_pMainMenuBar )
	{
		m_pBackButton->SetVisible( false );
	}

	if ( m_pCombinedScrollBar )
	{
		int yOrigin = -m_pCombinedScrollBar->GetValue();

		int totalHeight = 0;
		FOR_EACH_VEC( m_Tabs, i )
		{
			m_Tabs[i]->m_pGrid->m_iScrollOffset = totalHeight;
			m_Tabs[i]->m_pGrid->SetPos( 0, yOrigin + totalHeight );
			totalHeight += m_Tabs[i]->m_pGrid->GetTall();
		}

		m_pCombinedScrollBar->SetVisible( m_pGridParent->GetTall() < totalHeight );
		m_pCombinedScrollBar->SetTall( m_pGridParent->GetTall() );
		m_pCombinedScrollBar->SetButtonPressedScrollValue( m_pGridParent->GetTall() / 2 );
		m_pCombinedScrollBar->SetRangeWindow( MIN( m_pGridParent->GetTall(), totalHeight ) );
		m_pCombinedScrollBar->SetRange( 0, totalHeight );
	}
}

void TabbedGridDetails::OnMouseWheeled( int delta )
{
	BaseClass::OnMouseWheeled( delta );

	if ( !m_pCombinedScrollBar )
		return;

	int val = m_pCombinedScrollBar->GetValue();
	val -= ( delta * YRES( 15 ) );
	m_pCombinedScrollBar->SetValue( val );
}

void TabbedGridDetails::OnSliderMoved( int position )
{
	if ( m_pCombinedScrollBar )
	{
		InvalidateLayout();
		Repaint();
	}
}

void TabbedGridDetails::OnCommand( const char *command )
{
	if ( !V_strcmp( command, "BackButton" ) )
	{
		BaseModUI::CBaseModPanel::GetSingleton().PlayUISound( BaseModUI::UISOUND_BACK );
		if ( m_hOverridePanel )
		{
			m_hOverridePanel->MarkForDeletion();
			SetOverridePanel( NULL );

			if ( m_hCurrentTab && m_hCurrentTab->m_pGrid && m_hCurrentTab->m_pGrid->m_hCurrentEntry )
			{
				m_hCurrentTab->m_pGrid->m_hCurrentEntry->m_pFocusHolder->RequestFocus();
			}
		}
		else
		{
			MarkForDeletion();
		}
	}
	else if ( !V_stricmp( command, "PrevPage" ) )
	{
		if ( m_Tabs.Count() <= 1 || m_hOverridePanel )
		{
			BaseModUI::CBaseModPanel::GetSingleton().PlayUISound( BaseModUI::UISOUND_INVALID );
			return;
		}

		int iTab = m_Tabs.Find( m_hCurrentTab );
		Assert( m_Tabs.IsValidIndex( iTab ) );

		if ( iTab <= 0 )
		{
			BaseModUI::CBaseModPanel::GetSingleton().PlayUISound( BaseModUI::UISOUND_INVALID );
			return;
		}

		BaseModUI::CBaseModPanel::GetSingleton().PlayUISound( BaseModUI::UISOUND_FOCUS );
		ActivateTab( m_Tabs[iTab - 1] );
	}
	else if ( !V_stricmp( command, "NextPage" ) )
	{
		if ( m_Tabs.Count() <= 1 || m_hOverridePanel )
		{
			BaseModUI::CBaseModPanel::GetSingleton().PlayUISound( BaseModUI::UISOUND_INVALID );
			return;
		}

		int iTab = m_Tabs.Find( m_hCurrentTab );
		Assert( m_Tabs.IsValidIndex( iTab ) );

		if ( iTab < 0 || iTab >= m_Tabs.Count() - 1 )
		{
			BaseModUI::CBaseModPanel::GetSingleton().PlayUISound( BaseModUI::UISOUND_INVALID );
			return;
		}

		BaseModUI::CBaseModPanel::GetSingleton().PlayUISound( BaseModUI::UISOUND_FOCUS );
		ActivateTab( m_Tabs[iTab + 1] );
	}
	else if ( !V_stricmp( command, "CyclePage" ) )
	{
		if ( m_Tabs.Count() <= 1 || m_hOverridePanel )
		{
			BaseModUI::CBaseModPanel::GetSingleton().PlayUISound( BaseModUI::UISOUND_INVALID );
			return;
		}

		int iTab = m_Tabs.Find( m_hCurrentTab );
		Assert( m_Tabs.IsValidIndex( iTab ) );

		BaseModUI::CBaseModPanel::GetSingleton().PlayUISound( BaseModUI::UISOUND_FOCUS );
		ActivateTab( m_Tabs[( iTab + 1 ) % m_Tabs.Count()] );
	}
	else
	{
		BaseClass::OnCommand( command );
	}
}

void TabbedGridDetails::OnKeyCodeTyped( vgui::KeyCode keycode )
{
	switch ( keycode )
	{
	case KEY_ESCAPE:
		OnCommand( "BackButton" );
		break;
	case KEY_PAGEUP:
		OnCommand( "PrevPage" );
		break;
	case KEY_PAGEDOWN:
		OnCommand( "NextPage" );
		break;
	case KEY_TAB:
		OnCommand( "CyclePage" );
		break;
	default:
		BaseClass::OnKeyCodeTyped( keycode );
		break;
	}
}

void TabbedGridDetails::OnKeyCodePressed( vgui::KeyCode keycode )
{
	static bool s_bForwarded = false;
	if ( s_bForwarded )
	{
		return;
	}

	if ( m_hOverridePanel )
	{
		s_bForwarded = true;
		m_hOverridePanel->OnKeyCodePressed( keycode );
		s_bForwarded = false;
	}
	else if ( m_hCurrentTab && m_hCurrentTab->m_pGrid->m_hCurrentEntry )
	{
		s_bForwarded = true;
		m_hCurrentTab->m_pGrid->m_hCurrentEntry->OnKeyCodePressed( keycode );
		s_bForwarded = false;
	}

	int lastUser = GetJoystickForCode( keycode );
	BaseModUI::CBaseModPanel::GetSingleton().SetLastActiveUserId( lastUser );

	vgui::KeyCode code = GetBaseButtonCode( keycode );

	switch ( code )
	{
	case KEY_XBUTTON_B:
		OnCommand( "BackButton" );
		break;
	case KEY_XBUTTON_LEFT_SHOULDER:
		OnCommand( "PrevPage" );
		break;
	case KEY_XBUTTON_RIGHT_SHOULDER:
		OnCommand( "NextPage" );
		break;
	default:
		BaseClass::OnKeyCodePressed( keycode );
		break;
	}
}

void TabbedGridDetails::SetTitle( const char *title, bool surfaceTitle )
{
	BaseClass::SetTitle( title, surfaceTitle );
	m_pHeaderFooter->SetTitle( title );
}

void TabbedGridDetails::SetTitle( const wchar_t *title, bool surfaceTitle )
{
	BaseClass::SetTitle( title, surfaceTitle );
	m_pHeaderFooter->SetTitle( title );
}

void TabbedGridDetails::ShowFullScreen()
{
	if ( engine->IsConnected() )
	{
		SetParent( GetClientMode()->GetViewport() );
	}
	else
	{
		vgui::VPANEL rootpanel = enginevgui->GetPanel( PANEL_GAMEUIDLL );
		SetParent( rootpanel );
	}

	SetVisible( true );
}

void TabbedGridDetails::UseMainMenuLayout( int iTopButtonIndex )
{
	m_pHeaderFooter->SetTitle( L"" );
	m_pHeaderFooter->SetHeaderEnabled( false );
	m_pHeaderFooter->SetFooterEnabled( false );
	m_pMainMenuBar = new CRD_VGUI_Main_Menu_Top_Bar( this, "TopBar" );
	m_pMainMenuBar->m_hActiveButton = m_pMainMenuBar->m_pTopButton[iTopButtonIndex];
	m_pMainMenuTicker = new CRD_VGUI_Stock_Ticker_Helper( this, "StockTickerHelper" );
	m_pBackButton->SetVisible( false );

	InvalidateLayout();
}

void TabbedGridDetails::RememberTabIndex( ConVar *pCVar )
{
	ActivateTab( m_Tabs[clamp<int>( pCVar->GetInt(), 0, m_Tabs.Count() - 1 )] );

	m_pLastTabConVar = pCVar;
}

void TabbedGridDetails::AddTab( TGD_Tab *pTab )
{
	Assert( pTab );
	if ( !pTab )
	{
		return;
	}

	Assert( m_Tabs.Find( pTab ) == m_Tabs.InvalidIndex() );

	m_Tabs.AddToTail( pTab );

	if ( m_hCurrentTab )
	{
		pTab->DeactivateTab();
	}
	else
	{
		ActivateTab( pTab );
	}

	pTab->SetVisible( true );

	if ( m_pGridParent && pTab->m_pGrid->m_pScrollBar )
	{
		pTab->InitCombinedGrid( m_pGridParent );
	}

	InvalidateLayout();
}

void TabbedGridDetails::RemoveTab( TGD_Tab *pTab )
{
	Assert( pTab );
	if ( !pTab )
	{
		return;
	}

	Assert( m_Tabs.Find( pTab ) != m_Tabs.InvalidIndex() );

	if ( m_hCurrentTab.Get() == pTab )
	{
		int index = m_Tabs.Find( pTab );
		if ( index > 0 )
		{
			ActivateTab( m_Tabs[index - 1] );
		}
		else if ( index == 0 )
		{
			ActivateTab( m_Tabs.Count() == 1 ? NULL : m_Tabs[1] );
		}
	}

	Assert( m_hCurrentTab.Get() != pTab );

	m_Tabs.FindAndRemove( pTab );
	pTab->m_pGrid->SetVisible( false );

	pTab->SetVisible( false );

	InvalidateLayout();
}

void TabbedGridDetails::ActivateTab( TGD_Tab *pTab )
{
	Assert( !pTab || m_Tabs.Find( pTab ) != m_Tabs.InvalidIndex() );

	if ( m_hCurrentTab )
	{
		Assert( m_Tabs.Find( m_hCurrentTab ) != m_Tabs.InvalidIndex() );
		m_hCurrentTab->DeactivateTab();
	}

	m_hCurrentTab = pTab;

	if ( pTab )
	{
		if ( m_pLastTabConVar )
		{
			m_pLastTabConVar->SetValue( m_Tabs.Find( pTab ) );
			engine->ClientCmd_Unrestricted( "host_writeconfig\n" );
		}

		pTab->ActivateTab();
	}
}

void TabbedGridDetails::SetOverridePanel( vgui::Panel *pPanel )
{
	m_hOverridePanel = pPanel;

	InvalidateLayout();

	if ( pPanel )
	{
		NavigateToChild( pPanel );
	}
}

void TabbedGridDetails::UseCombinedGrid()
{
	MakeReadyForUse();

	Assert( !m_pGridParent );
	m_pGridParent = new vgui::Panel( this, "GridParent" );
	Assert( !m_pCombinedScrollBar );
	m_pCombinedScrollBar = new vgui::ScrollBar( this, "ScrollBar", true );

	FOR_EACH_VEC( m_Tabs, i )
	{
		m_Tabs[i]->InitCombinedGrid( m_pGridParent );
	}

	InvalidateLayout( true, true );

	if ( m_Tabs.Count() != 0 )
	{
		ActivateTab( m_Tabs[0] );
	}
}

TGD_Tab::TGD_Tab( TabbedGridDetails *parent )
	: BaseClass( parent, "Tab" )
{
	SetConsoleStylePanel( true );

	m_pParent = parent;
	m_pLabel = new vgui::Label( this, "Label", L"" );
	m_pLabelHighlight = new vgui::Label( this, "LabelHighlight", L"" );
	m_pHighlight = new vgui::Panel( this, "Highlight" );
	m_pGrid = NULL;
	m_pDetails = NULL;
	m_iMinWidth = YRES( 30 );
}

TGD_Tab::TGD_Tab( TabbedGridDetails *parent, const char *szLabel )
	: ThisClass( parent )
{
	Assert( *szLabel == '#' );
	m_pLabel->SetText( szLabel );
	m_pLabelHighlight->SetText( szLabel );
}

TGD_Tab::TGD_Tab( TabbedGridDetails *parent, const wchar_t *wszLabel )
	: ThisClass( parent )
{
	m_pLabel->SetText( wszLabel );
	m_pLabelHighlight->SetText( wszLabel );
}

TGD_Tab::~TGD_Tab()
{
	// The grid and details aren't our children, but they should be removed with us.
	if ( m_pGrid )
	{
		m_pGrid->SetVisible( false );
		m_pGrid->MarkForDeletion();
	}
	if ( m_pDetails )
	{
		m_pDetails->SetVisible( false );
		m_pDetails->MarkForDeletion();
	}
}

void TGD_Tab::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	if ( !m_pGrid )
	{
		m_pGrid = CreateGrid();
	}
	if ( !m_pDetails )
	{
		m_pDetails = CreateDetails();
	}

	LoadControlSettings( "Resource/UI/TGD_Tab.res" );
	m_iMinWidth = GetWide();

	BaseClass::ApplySchemeSettings( pScheme );
}

void TGD_Tab::PerformLayout()
{
	BaseClass::PerformLayout();

	m_pLabel->SizeToContents();

	int w = MAX( m_iMinWidth, m_pLabel->GetWide() + YRES( 20 ) );

	SetWide( w );
	m_pLabel->SetWide( w );
	m_pLabelHighlight->SetWide( w );
	m_pHighlight->SetWide( w );
}

void TGD_Tab::OnKeyFocusTicked()
{
	BaseClass::OnKeyFocusTicked();

	m_pParent->RequestFocus();
	m_pParent->ActivateTab( this );
}

TGD_Grid *TGD_Tab::CreateGrid()
{
	return new TGD_Grid( this );
}

void TGD_Tab::ActivateTab()
{
	MakeReadyForUse();
	if ( m_pGrid->m_pScrollBar )
		m_pGrid->SetVisible( true );
	m_pDetails->SetVisible( true );
	m_pLabelHighlight->SetVisible( true );
	m_pHighlight->SetVisible( true );

	m_pGrid->InvalidateLayout();
	m_pDetails->InvalidateLayout();
}

void TGD_Tab::DeactivateTab()
{
	MakeReadyForUse();
	if ( m_pGrid->m_pScrollBar )
		m_pGrid->SetVisible( false );
	m_pDetails->SetVisible( false );
	m_pLabelHighlight->SetVisible( false );
	m_pHighlight->SetVisible( false );
}

void TGD_Tab::InitCombinedGrid( vgui::Panel *pGridParent )
{
	Assert( m_pGrid );
	Assert( !m_pGrid->m_pTitle );
	Assert( m_pGrid->m_pScrollBar );

	char szTitle[1024];
	m_pLabel->GetText( szTitle, sizeof( szTitle ) );
	m_pGrid->m_pTitle = new vgui::Label( m_pGrid, "Title", szTitle );

	m_pGrid->m_pScrollBar->MarkForDeletion();
	m_pGrid->m_pScrollBar = NULL;

	m_pGrid->SetParent( pGridParent );
}

TGD_Grid::TGD_Grid( TGD_Tab *pTab )
	: BaseClass( pTab->m_pParent, "Grid" )
{
	SetConsoleStylePanel( true );

	m_pParent = pTab;
	m_pTitle = NULL;
	m_pMessage = new vgui::Label( this, "Message", L"" );
	m_pScrollBar = new vgui::ScrollBar( this, "ScrollBar", true );
	m_pScrollBar->AddActionSignalTarget( this );

	m_iLastFocus = 0;
	m_iScrollOffset = 0;
}

TGD_Grid::~TGD_Grid()
{
	m_pParent->m_pGrid = NULL;
}

void TGD_Grid::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	LoadControlSettings( m_pScrollBar ? "Resource/UI/TGD_Grid.res" : "Resource/UI/CGD_Grid.res" );

	BaseClass::ApplySchemeSettings( pScheme );

	if ( m_pScrollBar )
		m_pScrollBar->UseImages( "scroll_up", "scroll_down", "scroll_line", "scroll_box" );
}

void TGD_Grid::PerformLayout()
{
	BaseClass::PerformLayout();

	int x, y;
	GetPos( x, y );
	if ( x < 0 )
	{
		SetWide( GetWide() + x * 2 );
		SetPos( 0, y );
	}

	if ( m_Entries.Count() == 0 )
	{
		if ( m_pScrollBar )
			m_pScrollBar->SetVisible( false );
		return;
	}

	int totalWide = GetWide();
	if ( m_pScrollBar )
		totalWide -= m_pScrollBar->GetWide();
	int totalTall = GetTall();

	int eachWide, eachTall;
	m_Entries[0]->MakeReadyForUse();
	m_Entries[0]->GetSize( eachWide, eachTall );

	int perRow = MAX( totalWide / eachWide, 1 );

	int totalHeight = ( m_Entries.Count() + perRow - 1 ) / perRow * eachTall;

	if ( m_pScrollBar )
	{
		m_pScrollBar->SetVisible( totalTall < totalHeight );
		m_pScrollBar->SetTall( totalTall );
		m_pScrollBar->SetButtonPressedScrollValue( totalTall / 2 );
		m_pScrollBar->SetRangeWindow( MIN( totalTall, totalHeight ) );
		m_pScrollBar->SetRange( 0, totalHeight );
	}
	else
	{
		totalHeight += m_pTitle->GetTall();
		SetTall( totalHeight );

		if ( totalTall != totalHeight )
			m_pParent->m_pParent->InvalidateLayout();
	}

	TGD_Grid *pGridUp = NULL, *pGridDown = NULL;
	if ( m_pScrollBar == NULL )
	{
		int i = m_pParent->m_pParent->m_Tabs.Find( m_pParent );

		for ( int j = i - 1; j >= 0; j-- )
		{
			if ( m_pParent->m_pParent->m_Tabs[j]->m_pGrid->m_Entries.Count() )
			{
				pGridUp = m_pParent->m_pParent->m_Tabs[j]->m_pGrid;
				break;
			}
		}

		for ( int j = i + 1; j < m_pParent->m_pParent->m_Tabs.Count(); j++ )
		{
			if ( m_pParent->m_pParent->m_Tabs[j]->m_pGrid->m_Entries.Count() )
			{
				pGridDown = m_pParent->m_pParent->m_Tabs[j]->m_pGrid;
				break;
			}
		}
	}

	if ( m_pParent->m_pParent->m_pMainMenuBar && !pGridUp )
		m_pParent->m_pParent->m_pMainMenuBar->SetNavDown( m_Entries[0]->m_pFocusHolder );

	int yOffset = m_pScrollBar ? -m_pScrollBar->GetValue() : m_pTitle->GetTall();

	bool bAnyFocus = false;

	FOR_EACH_VEC( m_Entries, i )
	{
		m_Entries[i]->MakeReadyForUse();
		Assert( m_Entries[i]->GetWide() == eachWide );
		Assert( m_Entries[i]->GetTall() == eachTall );

		int col = i % perRow;
		int row = i / perRow;

		m_Entries[i]->SetPos( col * eachWide, row * eachTall + yOffset );

		constexpr bool bAllowWrapping = true;

		int up = col + ( row - 1 ) * perRow;
		int down = col + ( row + 1 ) * perRow;
		int left = !bAllowWrapping && col == 0 ? -1 : col - 1 + row * perRow;
		int right = !bAllowWrapping && col == perRow - 1 ? -1 : col + 1 + row * perRow;

		m_Entries[i]->m_pFocusHolder->SetNavUp( up >= 0 && up < m_Entries.Count() ? m_Entries[up]->m_pFocusHolder : ( pGridUp ? pGridUp->m_Entries[pGridUp->m_Entries.Count() - 1]->m_pFocusHolder : m_pParent->m_pParent->m_pMainMenuBar ) );
		m_Entries[i]->m_pFocusHolder->SetNavDown( down >= 0 && down < m_Entries.Count() ? m_Entries[down]->m_pFocusHolder : ( pGridDown ? pGridDown->m_Entries[0]->m_pFocusHolder : NULL ) );
		m_Entries[i]->m_pFocusHolder->SetNavLeft( left >= 0 && left < m_Entries.Count() ? m_Entries[left]->m_pFocusHolder : ( pGridUp ? pGridUp->m_Entries[pGridUp->m_Entries.Count() - 1]->m_pFocusHolder : NULL ) );
		m_Entries[i]->m_pFocusHolder->SetNavRight( right >= 0 && right < m_Entries.Count() ? m_Entries[right]->m_pFocusHolder : ( pGridDown ? pGridDown->m_Entries[0]->m_pFocusHolder : NULL ) );

		if ( m_Entries[i]->m_pFocusHolder->HasFocus() )
		{
			bAnyFocus = true;
			m_iLastFocus = i;
		}
	}

	if ( !bAnyFocus && ( m_pScrollBar || m_pParent->m_pParent->m_hCurrentTab.Get() == m_pParent ) )
	{
		if ( m_Entries.Count() > m_iLastFocus )
		{
			DisplayEntry( m_Entries[m_iLastFocus] );
		}
		else if ( m_Entries.Count() > 0 )
		{
			DisplayEntry( m_Entries[0] );
		}
		else
		{
			DisplayEntry( NULL );
		}
	}
}

void TGD_Grid::OnMouseWheeled( int delta )
{
	BaseClass::OnMouseWheeled( delta );

	if ( !m_pScrollBar )
		return;

	int val = m_pScrollBar->GetValue();
	val -= ( delta * YRES( 15 ) );
	m_pScrollBar->SetValue( val );
}

void TGD_Grid::OnSliderMoved( int position )
{
	InvalidateLayout();
	Repaint();
}

void TGD_Grid::OnKeyFocusTicked()
{
	BaseClass::OnKeyFocusTicked();

	vgui::Panel *pFocus = vgui::ipanel()->GetPanel( GetCurrentKeyFocus(), GetModuleName() );
	if ( !pFocus || ( m_hCurrentEntry && pFocus->HasParent( m_hCurrentEntry->GetVPanel() ) ) )
	{
		return;
	}

	FOR_EACH_VEC( m_Entries, i )
	{
		if ( pFocus->HasParent( m_Entries[i]->GetVPanel() ) )
		{
			DisplayEntry( m_Entries[i] );
			break;
		}
	}
}

void TGD_Grid::DeleteAllEntries()
{
	m_pParent->m_pDetails->DisplayEntry( NULL );
	m_hCurrentEntry = NULL;

	FOR_EACH_VEC( m_Entries, i )
	{
		m_Entries[i]->MarkForDeletion();
	}

	m_Entries.Purge();
}

void TGD_Grid::AddEntry( TGD_Entry *pEntry )
{
	Assert( pEntry );
	if ( !pEntry )
	{
		return;
	}

	Assert( m_Entries.Find( pEntry ) == m_Entries.InvalidIndex() );

	m_Entries.AddToTail( pEntry );
	InvalidateLayout();
}

void TGD_Grid::RemoveEntry( TGD_Entry *pEntry )
{
	Assert( pEntry );
	if ( !pEntry )
	{
		return;
	}

	m_Entries.FindAndFastRemove( pEntry );
	InvalidateLayout();
}

void TGD_Grid::DisplayEntry( TGD_Entry *pEntry )
{
	m_hCurrentEntry = pEntry;
	m_pParent->m_pDetails->DisplayEntry( pEntry );

	if ( pEntry )
	{
		NavigateToChild( pEntry->m_pFocusHolder );
	}

	FOR_EACH_VEC( m_pParent->m_pParent->m_Tabs, i )
	{
		if ( m_pParent->m_pParent->m_Tabs[i]->m_pGrid != this )
		{
			m_pParent->m_pParent->m_Tabs[i]->m_pDetails->SetVisible( false );
			m_pParent->m_pParent->m_Tabs[i]->m_pHighlight->SetVisible( false );
			m_pParent->m_pParent->m_Tabs[i]->m_pLabelHighlight->SetVisible( false );
		}
	}

	m_pParent->m_pDetails->SetVisible( true );
	m_pParent->m_pHighlight->SetVisible( true );
	m_pParent->m_pLabelHighlight->SetVisible( true );

	if ( m_pParent->m_pParent->m_hCurrentTab != m_pParent )
	{
		m_pParent->m_pParent->m_hCurrentTab = m_pParent;
		if ( m_pParent->m_pParent->m_pLastTabConVar )
		{
			m_pParent->m_pParent->m_pLastTabConVar->SetValue( m_pParent->m_pParent->m_Tabs.Find( m_pParent ) );
			engine->ClientCmd_Unrestricted( "host_writeconfig\n" );
		}
	}
}

void TGD_Grid::SetMessage( const char *szMessage )
{
	m_pMessage->SetVisible( szMessage && *szMessage != '\0' );
	m_pMessage->SetText( szMessage ? szMessage : "" );
}

void TGD_Grid::SetMessage( const wchar_t *wszMessage )
{
	m_pMessage->SetVisible( wszMessage && *wszMessage != L'\0' );
	m_pMessage->SetText( wszMessage ? wszMessage : L"" );
}

class TGD_Entry_FocusHolder : public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( TGD_Entry_FocusHolder, vgui::Panel );
public:
	TGD_Entry_FocusHolder( vgui::Panel *pParent, const char *pElementName ) : BaseClass( pParent, pElementName )
	{
		SetConsoleStylePanel( true );

		m_bMousePressed = false;
		m_bNoScrollOnFocus = false;
	}

	virtual void NavigateTo() override
	{
		BaseClass::NavigateTo();

		BaseModUI::CBaseModPanel::GetSingleton().PlayUISound( BaseModUI::UISOUND_FOCUS );
		RequestFocus();
	}

	virtual void OnSetFocus() override
	{
		BaseClass::OnSetFocus();

		TGD_Entry *pParent = assert_cast< TGD_Entry * >( GetParent() );
		pParent->m_pHighlight->SetVisible( true );
		pParent->m_pParent->m_pParent->m_pDetails->DisplayEntry( pParent );

		if ( !m_bNoScrollOnFocus )
		{
			TGD_Grid *pGrid = pParent->m_pParent;
			Assert( ( pGrid->m_pScrollBar == NULL ) != ( pGrid->m_pParent->m_pParent->m_pCombinedScrollBar == NULL ) );

			vgui::ScrollBar *pScrollBar = pGrid->m_pScrollBar ? pGrid->m_pScrollBar : pGrid->m_pParent->m_pParent->m_pCombinedScrollBar;

			int x, y;
			pParent->GetPos( x, y );
			y += pGrid->m_iScrollOffset;

			int scroll = pScrollBar->GetValue();
			if ( pGrid->m_pScrollBar )
				y += scroll;

			int minScroll = y + GetTall() * 1.5f - pScrollBar->GetRangeWindow();
			int maxScroll = y - GetTall() * 0.5f;
			if ( scroll < minScroll )
			{
				pScrollBar->SetValue( minScroll );
			}
			else if ( scroll > maxScroll )
			{
				pScrollBar->SetValue( maxScroll );
			}
		}
	}

	virtual void OnKillFocus() override
	{
		BaseClass::OnKillFocus();

		TGD_Entry *pParent = assert_cast< TGD_Entry * >( GetParent() );
		m_bMousePressed = false;
		m_bNoScrollOnFocus = false;
		pParent->m_pHighlight->SetVisible( false );
	}

	virtual void OnCursorMoved( int x, int y ) override
	{
		m_bNoScrollOnFocus = true;

		if ( GetParent() )
			GetParent()->NavigateToChild( this );
		else
			NavigateTo();
	}

	virtual void OnMousePressed( vgui::MouseCode code ) override
	{
		if ( code == MOUSE_LEFT && HasFocus() )
		{
			m_bMousePressed = true;
			return;
		}

		BaseClass::OnMousePressed( code );
	}

	virtual void OnMouseReleased( vgui::MouseCode code ) override
	{
		if ( code == MOUSE_LEFT && m_bMousePressed )
		{
			TGD_Entry *pParent = assert_cast< TGD_Entry * >( GetParent() );
			m_bMousePressed = false;
			pParent->ApplyEntry();
			return;
		}

		BaseClass::OnMouseReleased( code );
	}

	bool m_bMousePressed;
	bool m_bNoScrollOnFocus;
};

TGD_Entry::TGD_Entry( TGD_Grid *parent, const char *panelName )
	: BaseClass( parent, panelName )
{
	SetConsoleStylePanel( true );

	m_pParent = parent;

	m_pFocusHolder = new TGD_Entry_FocusHolder( this, "FocusHolder" );
	m_pHighlight = new vgui::Panel( this, "Highlight" );
}

void TGD_Entry::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	LoadControlSettings( VarArgs( "Resource/UI/%s.res", GetName() ) );

	BaseClass::ApplySchemeSettings( pScheme );

	m_pFocusHolder->SetPos( 0, 0 );
	m_pFocusHolder->SetZPos( 999 );
	m_pFocusHolder->SetWide( GetWide() );
	m_pFocusHolder->SetTall( GetTall() );
}

void TGD_Entry::OnKeyCodePressed( vgui::KeyCode keycode )
{
	int lastUser = GetJoystickForCode( keycode );
	BaseModUI::CBaseModPanel::GetSingleton().SetLastActiveUserId( lastUser );

	vgui::KeyCode code = GetBaseButtonCode( keycode );

	switch ( code )
	{
	case KEY_XBUTTON_A:
		BaseModUI::CBaseModPanel::GetSingleton().PlayUISound( BaseModUI::UISOUND_ACCEPT );
		ApplyEntry();
		break;
	default:
		BaseClass::OnKeyCodePressed( keycode );
		break;
	}
}

TGD_Details::TGD_Details( TGD_Tab *pTab )
	: BaseClass( pTab->m_pParent, "Details" )
{
	SetConsoleStylePanel( true );

	m_pParent = pTab;
}

TGD_Details::~TGD_Details()
{
	m_pParent->m_pDetails = NULL;
}

void TGD_Details::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	LoadControlSettings( "Resource/UI/TGD_Details.res" );

	BaseClass::ApplySchemeSettings( pScheme );
}

void TGD_Details::PerformLayout()
{
	BaseClass::PerformLayout();

	int x, y;
	GetPos( x, y );
	int overhang = x + GetWide() - ScreenWidth();
	if ( overhang > 0 )
	{
		x -= overhang;
		SetPos( x, y );
	}
}

TGD_Entry *TGD_Details::GetCurrentEntry()
{
	return m_pParent->m_pGrid->m_hCurrentEntry;
}
