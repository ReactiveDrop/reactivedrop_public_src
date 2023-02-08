#include "cbase.h"
#include "rd_collections.h"
#include "rd_swarmopedia.h"
#include "rd_workshop.h"
#include <vgui/ILocalize.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/RichText.h>
#include <vgui_controls/ScrollBar.h>
#include <vgui_controls/TextImage.h>
#include "asw_model_panel.h"
#include "nb_button.h"
#include "asw_util_shared.h"
#include "gameui/swarm/basemodpanel.h"
#include "gameui/swarm/vgenericpanellist.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar rd_swarmopedia_global_stat_window_days( "rd_swarmopedia_global_stat_window_days", "30", FCVAR_ARCHIVE, "Number of days to sum for global stats in the Swarmopedia. 0 for all time.", true, 0, true, 60 );
ConVar rd_swarmopedia_global_stat_update_seconds( "rd_swarmopedia_global_stat_update_seconds", "600", FCVAR_HIDDEN, "", true, 30, true, 10000000 );

CRD_Collection_Tab_Swarmopedia::CRD_Collection_Tab_Swarmopedia( TabbedGridDetails *parent, const char *szLabel )
	: BaseClass( parent, szLabel )
{
	m_pCollection = NULL;
}

CRD_Collection_Tab_Swarmopedia::~CRD_Collection_Tab_Swarmopedia()
{
	if ( m_pCollection )
	{
		delete m_pCollection;

		m_pCollection = NULL;
	}
}

TGD_Grid *CRD_Collection_Tab_Swarmopedia::CreateGrid()
{
	TGD_Grid *pGrid = BaseClass::CreateGrid();

	Assert( !m_pCollection );
	m_pCollection = new RD_Swarmopedia::Collection();
	m_pCollection->ReadFromFiles( RD_Swarmopedia::Subset::Aliens );

	FOR_EACH_VEC( m_pCollection->Aliens, i )
	{
		pGrid->AddEntry( new CRD_Collection_Entry_Swarmopedia( pGrid, "CollectionEntrySwarmopedia", m_pCollection->Aliens[i] ) );
	}

	return pGrid;
}

TGD_Details *CRD_Collection_Tab_Swarmopedia::CreateDetails()
{
	return new CRD_Collection_Details_Swarmopedia( this );
}

CRD_Collection_Details_Swarmopedia::CRD_Collection_Details_Swarmopedia( CRD_Collection_Tab_Swarmopedia *parent )
	: BaseClass( parent )
{
	m_pLblHeader = new vgui::Label( this, "LblHeader", L"" );
	m_pLblAbilities = new vgui::Label( this, "LblAbilities", L"" );
	m_pLblError = new vgui::Label( this, "LblError", L"" );
	m_pGplStats = new BaseModUI::GenericPanelList( this, "GplStats", BaseModUI::GenericPanelList::ISM_ELEVATOR );

	m_nDisplayedFrames = 0;

	if ( SteamUserStats() )
	{
		m_nStatsDays = rd_swarmopedia_global_stat_window_days.GetInt();
		m_bStatsReady = false;

		SteamAPICall_t hAPICall = SteamUserStats()->RequestGlobalStats( rd_swarmopedia_global_stat_window_days.GetInt() );
		m_OnGlobalStatsReceived.Set( hAPICall, this, &CRD_Collection_Details_Swarmopedia::OnGlobalStatsReceived );
	}
	else
	{
		m_nStatsDays = -1;
		m_bStatsReady = true;
	}
}

void CRD_Collection_Details_Swarmopedia::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	LoadControlSettings( "Resource/UI/CollectionDetailsSwarmopedia.res" );

	BaseClass::ApplySchemeSettings( pScheme );
}

void CRD_Collection_Details_Swarmopedia::PerformLayout()
{
	BaseClass::PerformLayout();

	int discard, x, y, t;
	m_pLblAbilities->GetPos( discard, y );
	m_pLblAbilities->GetTextImage()->ResizeImageToContentMaxWidth( m_pLblAbilities->GetWide() );
	m_pLblAbilities->GetContentSize( discard, t );

	if ( *m_pLblAbilities->GetTextImage()->GetUText() == L'\0' )
	{
		t = 0;
	}

	y += t + vgui::Label::Content;

	m_pLblError->GetPos( x, discard );
	m_pLblError->SetPos( x, y );
	m_pLblError->GetTextImage()->ResizeImageToContentMaxWidth( m_pLblError->GetWide() );
	m_pLblError->GetContentSize( discard, t );

	if ( *m_pLblError->GetTextImage()->GetUText() == L'\0' )
	{
		t = 0;
	}

	y += t + vgui::Label::Content;

	m_pGplStats->GetPos( x, discard );
	m_pGplStats->SetPos( x, y );
}

void CRD_Collection_Details_Swarmopedia::DisplayEntry( TGD_Entry *pEntry )
{
	m_pGplStats->RemoveAllPanelItems();

	if ( !pEntry )
	{
		m_pLblHeader->SetText( L"" );
		m_pLblAbilities->SetText( L"" );
		m_pLblError->SetText( L"" );

		InvalidateLayout();

		return;
	}

	CRD_Collection_Entry_Swarmopedia *pSwarmopediaEntry = assert_cast< CRD_Collection_Entry_Swarmopedia * >( pEntry );
	const RD_Swarmopedia::Alien *pAlien = pSwarmopediaEntry->m_pAlien;

	if ( pAlien->GetOverallRequirementProgress() < 1.0f )
	{
		DisplayEntryLocked( pAlien );

		return;
	}

	m_pLblHeader->SetText( pAlien->Name );
	m_pLblHeader->SetFgColor( Color( 255, 255, 255, 255 ) );

	wchar_t buf[1024]{};
	FOR_EACH_VEC( pAlien->Abilities, i )
	{
		int len = V_wcslen( buf );
		if ( i != 0 )
		{
			V_wcsncpy( &buf[len], L" \u2022 ", sizeof( buf ) - len * sizeof( wchar_t ) );
			len = V_wcslen( buf );
		}

		TryLocalize( pAlien->Abilities[i]->Caption, &buf[len], sizeof( buf ) - len * sizeof( wchar_t ) );
	}

	m_pLblAbilities->SetText( buf );

	if ( m_nStatsDays == -1 )
	{
		m_pLblError->SetText( "#rd_so_global_stat_failed" );
	}
	else if ( m_bStatsReady )
	{
		m_pLblError->SetText( L"" );

		Assert( SteamUserStats() );

		wchar_t wszDays[4]{};
		V_snwprintf( wszDays, ARRAYSIZE( wszDays ), L"%d", m_nStatsDays );

		g_pVGuiLocalize->ConstructString( buf, sizeof( buf ),
			g_pVGuiLocalize->FindSafe( m_nStatsDays ? "#rd_so_global_stat_days" : "#rd_so_global_stat_total" ), 1, wszDays );

		vgui::Label *pDaysLabel = m_pGplStats->AddPanelItem<vgui::Label>( "DaysLabel", "" );
		pDaysLabel->SetContentAlignment( vgui::Label::a_east );
		pDaysLabel->SetText( buf );

		FOR_EACH_VEC( pAlien->GlobalStats, i )
		{
			int nOK{};
			int64 nStat[61]{};
			if ( m_nStatsDays == 0 )
			{
				nOK = SteamUserStats()->GetGlobalStat( pAlien->GlobalStats[i]->StatName, &nStat[1] ) ? 1 : 0;
			}
			else
			{
				nOK = SteamUserStats()->GetGlobalStatHistory( pAlien->GlobalStats[i]->StatName, &nStat[1], sizeof( nStat ) - sizeof( nStat[0] ) );
			}

			for ( int j = 1; j <= nOK; j++ )
			{
				nStat[0] += nStat[j];
			}

			CRD_Collection_StatLine *pStatLine = m_pGplStats->AddPanelItem<CRD_Collection_StatLine>( "StatLine" );
			pStatLine->SetLabel( g_pVGuiLocalize->FindSafe( pAlien->GlobalStats[i]->Caption ) );
			pStatLine->SetValue( nStat[0] );
		}
	}
	else
	{
		m_pLblError->SetText( "#rd_so_global_stat_loading" );
	}

	InvalidateLayout();
}

void CRD_Collection_Details_Swarmopedia::OnThink()
{
	if ( !m_OnGlobalStatsReceived.IsActive() )
	{
		m_nDisplayedFrames++;

		if ( m_nDisplayedFrames >= rd_swarmopedia_global_stat_update_seconds.GetInt() * 60 )
		{
			m_nDisplayedFrames = 0;

			m_nStatsDays = rd_swarmopedia_global_stat_window_days.GetInt();
			m_bStatsReady = false;

			SteamAPICall_t hAPICall = SteamUserStats()->RequestGlobalStats( rd_swarmopedia_global_stat_window_days.GetInt() );
			m_OnGlobalStatsReceived.Set( hAPICall, this, &CRD_Collection_Details_Swarmopedia::OnGlobalStatsReceived );
		}
	}
}

void CRD_Collection_Details_Swarmopedia::DisplayEntryLocked( const RD_Swarmopedia::Alien *pAlien )
{
	wchar_t buf[4096]{};
	wchar_t line[256]{};

	m_pLblHeader->SetText( "#rd_so_requirements_not_met" );
	m_pLblHeader->SetFgColor( Color( 255, 96, 0, 255 ) );

	m_pLblAbilities->SetText( L"" );

	FOR_EACH_VEC( pAlien->Requirements, i )
	{
		float flProgress = pAlien->Requirements[i]->GetProgress();
		if ( flProgress >= 1.0f )
		{
			continue;
		}

		g_pVGuiLocalize->ConstructString( line, sizeof( line ),
			g_pVGuiLocalize->FindSafe( pAlien->Requirements[i]->Caption ), 1,
			UTIL_RD_CommaNumber( int( flProgress * 100 ) ) );

		if ( buf[0] != L'\0' )
		{
			V_snwprintf( buf, ARRAYSIZE( buf ), L"%s\n\n%s", buf, line );
		}
		else
		{
			V_wcsncpy( buf, line, sizeof( buf ) );
		}
	}

	m_pLblError->SetText( buf );

	InvalidateLayout();
}

void CRD_Collection_Details_Swarmopedia::OnGlobalStatsReceived( GlobalStatsReceived_t *pParam, bool bIOError )
{
	if ( bIOError || pParam->m_eResult != k_EResultOK )
	{
		Warning( "Failed to retrieve global stat history for Swarmopedia: %s\n", bIOError ? "IO Error" : UTIL_RD_EResultToString( pParam->m_eResult ) );
		m_nStatsDays = -1;
	}

	m_bStatsReady = true;
	DisplayEntry( GetCurrentEntry() );
}

CRD_Collection_Entry_Swarmopedia::CRD_Collection_Entry_Swarmopedia( TGD_Grid *parent, const char *panelName, const RD_Swarmopedia::Alien *pAlien )
	: BaseClass( parent, panelName )
{
	m_pAlien = pAlien;

	m_pIcon = new vgui::ImagePanel( this, "Icon" );
	m_pUnlockProgress = new vgui::Panel( this, "UnlockProgress" );
}

void CRD_Collection_Entry_Swarmopedia::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	bool bUnlocked = m_pAlien->GetOverallRequirementProgress() >= 1.0f;

	m_pUnlockProgress->SetWide( m_pHighlight->GetWide() * m_pAlien->GetOverallRequirementProgress() );
	m_pUnlockProgress->SetVisible( !bUnlocked );

	m_pIcon->SetImage( m_pAlien->Icon );
	m_pIcon->SetDrawColor( bUnlocked ? Color( 255, 255, 255, 255 ) : Color( 0, 0, 0, 255 ) );
}

void CRD_Collection_Entry_Swarmopedia::ApplyEntry()
{
	TabbedGridDetails *pTGD = m_pParent->m_pParent->m_pParent;
	vgui::Panel *pPanel = pTGD->m_hOverridePanel;
	if ( pPanel )
	{
		CRD_Collection_Panel_Swarmopedia *pSwarmopediaPanel = dynamic_cast< CRD_Collection_Panel_Swarmopedia * >( pPanel );
		bool bStop = pSwarmopediaPanel && pSwarmopediaPanel->m_pAlien == m_pAlien;

		pTGD->SetOverridePanel( NULL );
		pPanel->MarkForDeletion();

		if ( bStop )
		{
			return;
		}
	}

	if ( m_pAlien->GetOverallRequirementProgress() >= 1.0f )
	{
		pPanel = new CRD_Collection_Panel_Swarmopedia( pTGD, "SwarmopediaPanel", m_pAlien );
		pTGD->SetOverridePanel( pPanel );
	}
}

CRD_Collection_Panel_Swarmopedia::CRD_Collection_Panel_Swarmopedia( vgui::Panel *parent, const char *panelName, const RD_Swarmopedia::Alien *pAlien )
	: BaseClass( parent, panelName )
{
	m_pAlien = pAlien;
	m_iCurrentDisplay = 0;

	m_pModelPanel = new CRD_Swarmopedia_Model_Panel( this, "ModelPanel" );
	m_pModelButton = new CNB_Button( this, "ModelButton", L"", this, "CycleDisplay" );
	m_pLblNoModel = new vgui::Label( this, "LblNoModel", "#rd_so_display_no_model" );
	m_pContent = new vgui::RichText( this, "Content" );

	m_pModelPanel->SetMouseInputEnabled( false );
	m_pModelButton->SetControllerButton( KEY_XBUTTON_X );
	m_pContent->GetScrollBar()->UseImages( "scroll_up", "scroll_down", "scroll_line", "scroll_box" );
	m_pContent->SetUnusedScrollbarInvisible( true );
	m_pContent->SetPanelInteractive( false );
	m_pContent->SetCursor( vgui::dc_arrow );
}

void CRD_Collection_Panel_Swarmopedia::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	LoadControlSettings( "resource/UI/CollectionPanelSwarmopedia.res" );

	BaseClass::ApplySchemeSettings( pScheme );

	m_pContent->SetFont( pScheme->GetFont( "DefaultMedium", true ) );
	m_pContent->SetText( L"" );

	m_pModelButton->SetEnabled( m_pAlien->Display.Count() > 1 );

	wchar_t wszBuf[4096]{};
	FOR_EACH_VEC( m_pAlien->Content, i )
	{
		switch ( m_pAlien->Content[i]->Type )
		{
		case RD_Swarmopedia::Content::Type_t::Paragraph:
			TryLocalize( m_pAlien->Content[i]->Text, wszBuf, sizeof( wszBuf ) );
			m_pContent->InsertColorChange( m_pAlien->Content[i]->Color );
			m_pContent->InsertString( wszBuf );
			m_pContent->InsertString( L"\n\n" );
			break;
		default:
			Assert( !"Unhandled content type" );
			break;
		}
	}

	m_pContent->InsertColorChange( Color{ 169, 213, 255, 255 } );
	m_pContent->InsertString( g_pVGuiLocalize->FindSafe( "#rd_so_sources" ) );

	m_pContent->InsertColorChange( Color{ 83, 148, 192, 255 } );
	FOR_EACH_VEC( m_pAlien->Sources, i )
	{
		m_pContent->InsertString( L"\n" );
		m_pContent->InsertString( g_ReactiveDropWorkshop.AddonName( m_pAlien->Sources[i] ) );
	}
}

void CRD_Collection_Panel_Swarmopedia::PerformLayout()
{
	BaseClass::PerformLayout();

	if ( 0 <= m_iCurrentDisplay && m_iCurrentDisplay < m_pAlien->Display.Count() )
	{
		m_pModelPanel->SetDisplay( m_pAlien->Display[m_iCurrentDisplay] );
		m_pModelPanel->m_bShouldPaint = true;
		m_pModelPanel->SetVisible( true );

		m_pModelButton->SetText( m_pAlien->Display[m_iCurrentDisplay]->Caption );
		m_pModelButton->SetVisible( true );
		m_pLblNoModel->SetVisible( false );
	}
	else
	{
		m_pModelPanel->m_bShouldPaint = false;
		m_pModelPanel->SetVisible( false );

		m_pModelButton->SetVisible( false );
		m_pLblNoModel->SetVisible( true );
	}
}

void CRD_Collection_Panel_Swarmopedia::OnCommand( const char *command )
{
	if ( !V_strcmp( command, "CycleDisplay" ) )
	{
		if ( m_pAlien->Display.Count() )
		{
			m_iCurrentDisplay = ( m_iCurrentDisplay + 1 ) % m_pAlien->Display.Count();
			InvalidateLayout();
		}
	}
	else
	{
		BaseClass::OnCommand( command );
	}
}

void CRD_Collection_Panel_Swarmopedia::OnKeyCodePressed( vgui::KeyCode keycode )
{
	int lastUser = GetJoystickForCode( keycode );
	BaseModUI::CBaseModPanel::GetSingleton().SetLastActiveUserId( lastUser );

	vgui::KeyCode code = GetBaseButtonCode( keycode );

	switch ( code )
	{
	case KEY_XBUTTON_X:
		OnCommand( "CycleDisplay" );
		break;
	case KEY_XBUTTON_UP:
	case KEY_XSTICK1_UP:
	case KEY_XSTICK2_UP:
		m_pContent->GetScrollBar()->SetValue( m_pContent->GetScrollBar()->GetValue() - 10 );
		break;
	case KEY_XBUTTON_DOWN:
	case KEY_XSTICK1_DOWN:
	case KEY_XSTICK2_DOWN:
		m_pContent->GetScrollBar()->SetValue( m_pContent->GetScrollBar()->GetValue() + 10 );
		break;
	default:
		BaseClass::OnKeyCodePressed( keycode );
		break;
	}
}
