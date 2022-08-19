#include "cbase.h"
#include "rd_collections.h"
#include "rd_swarmopedia.h"
#include <vgui/ILocalize.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Label.h>
#include "asw_util_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


ConVar rd_swarmopedia_global_stat_window_days( "rd_swarmopedia_global_stat_window_days", "30", FCVAR_ARCHIVE, "Number of days to sum for global stats in the Swarmopedia. 0 for all time.", true, 0, true, 60 );

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
	m_pCollection->ReadFromFiles();

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
	m_pLblGlobalStatData = new vgui::Label( this, "LblGlobalStatData", L"" );

	if ( SteamUserStats() )
	{
		m_nStatsDays = rd_swarmopedia_global_stat_window_days.GetInt();
		m_bStatsReady = false;

		m_OnGlobalStatsReceived.Set( SteamUserStats()->RequestGlobalStats( rd_swarmopedia_global_stat_window_days.GetInt() ), this, &CRD_Collection_Details_Swarmopedia::OnGlobalStatsReceived );
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

void CRD_Collection_Details_Swarmopedia::DisplayEntry( TGD_Entry *pEntry )
{
	if ( !pEntry )
	{
		m_pLblHeader->SetText( L"" );
		m_pLblGlobalStatData->SetText( L"" );

		return;
	}

	wchar_t wszStatLines[4096]{};
	wchar_t wszStatLine[256]{};
	wchar_t wszStatNum[32]{};

	CRD_Collection_Entry_Swarmopedia *pSwarmopediaEntry = assert_cast< CRD_Collection_Entry_Swarmopedia * >( pEntry );
	const RD_Swarmopedia::Alien *pAlien = pSwarmopediaEntry->m_pAlien;

	if ( pAlien->GetOverallRequirementProgress() < 1.0f )
	{
		m_pLblHeader->SetText( "#rd_so_requirements_not_met" );
		m_pLblHeader->SetFgColor( Color( 255, 96, 0, 255 ) );

		FOR_EACH_VEC( pAlien->Requirements, i )
		{
			float flProgress = pAlien->Requirements[i]->GetProgress();
			if ( flProgress >= 1.0f )
			{
				continue;
			}

			V_snwprintf( wszStatNum, sizeof( wszStatNum ), L"%d", int( flProgress * 100 ) );

			g_pVGuiLocalize->ConstructString( wszStatLine, sizeof( wszStatLine ),
				g_pVGuiLocalize->FindSafe( pAlien->Requirements[i]->Caption ), 1, wszStatNum );

			if ( wszStatLines[0] != L'\0' )
			{
				V_snwprintf( wszStatLines, sizeof( wszStatLines ), L"%s\n%s", wszStatLines, wszStatLine );
			}
			else
			{
				V_wcsncpy( wszStatLines, wszStatLine, sizeof( wszStatLines ) );
			}
		}

		m_pLblGlobalStatData->SetText( wszStatLines );

		return;
	}

	m_pLblHeader->SetText( pAlien->Name );
	m_pLblHeader->SetFgColor( Color( 255, 255, 255, 255 ) );

	if ( m_nStatsDays == -1 )
	{
		m_pLblGlobalStatData->SetText( "#rd_so_global_stat_failed" );
	}
	else if ( m_bStatsReady )
	{
		Assert( SteamUserStats() );

		wchar_t wszDays[4]{};
		V_snwprintf( wszDays, sizeof( wszDays ), L"%d", m_nStatsDays );

		const wchar_t *wszStatFormat = g_pVGuiLocalize->FindSafe( m_nStatsDays ? "#rd_so_global_stat_days" : "#rd_so_global_stat_total" );
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

			if ( nStat[0] < 1000ll )
			{
				V_snwprintf( wszStatNum, sizeof( wszStatNum ), L"%lld", nStat[0] );
			}
			else if ( nStat[0] < 1000000ll )
			{
				V_snwprintf( wszStatNum, sizeof( wszStatNum ), L"%lld,%03lld", nStat[0] / 1000ll, nStat[0] % 1000ll );
			}
			else if ( nStat[0] < 1000000000ll )
			{
				V_snwprintf( wszStatNum, sizeof( wszStatNum ), L"%lld,%03lld,%03lld", nStat[0] / 1000000ll, nStat[0] / 1000ll % 1000ll, nStat[0] % 1000ll );
			}
			else if ( nStat[0] < 1000000000000ll )
			{
				V_snwprintf( wszStatNum, sizeof( wszStatNum ), L"%lld,%03lld,%03lld,%03lld", nStat[0] / 1000000000ll, nStat[0] / 1000000ll % 1000ll, nStat[0] / 1000ll % 1000ll, nStat[0] % 1000ll );
			}
			else if ( nStat[0] < 1000000000000000ll )
			{
				V_snwprintf( wszStatNum, sizeof( wszStatNum ), L"%lld,%03lld,%03lld,%03lld,%03lld", nStat[0] / 1000000000000ll, nStat[0] / 1000000000ll % 1000ll, nStat[0] / 1000000ll % 1000ll, nStat[0] / 1000ll % 1000ll, nStat[0] % 1000ll );
			}
			else if ( nStat[0] < 1000000000000000000ll )
			{
				V_snwprintf( wszStatNum, sizeof( wszStatNum ), L"%lld,%03lld,%03lld,%03lld,%03lld,%03lld", nStat[0] / 1000000000000000ll, nStat[0] / 1000000000000ll % 1000ll, nStat[0] / 1000000000ll % 1000ll, nStat[0] / 1000000ll % 1000ll, nStat[0] / 1000ll % 1000ll, nStat[0] % 1000ll );
			}
			else
			{
				V_snwprintf( wszStatNum, sizeof( wszStatNum ), L"%lld,%03lld,%03lld,%03lld,%03lld,%03lld,%03lld", nStat[0] / 1000000000000000000ll, nStat[0] / 1000000000000000ll % 1000ll, nStat[0] / 1000000000000ll % 1000ll, nStat[0] / 1000000000ll % 1000ll, nStat[0] / 1000000ll % 1000ll, nStat[0] / 1000ll % 1000ll, nStat[0] % 1000ll );
			}

			g_pVGuiLocalize->ConstructString( wszStatLine, sizeof( wszStatLine ),
				wszStatFormat, 3, g_pVGuiLocalize->FindSafe( pAlien->GlobalStats[i]->Caption ), wszDays, wszStatNum );

			if ( i )
			{
				V_snwprintf( wszStatLines, sizeof( wszStatLines ), L"%s\n%s", wszStatLines, wszStatLine );
			}
			else
			{
				V_wcsncpy( wszStatLines, wszStatLine, sizeof( wszStatLines ) );
			}
		}

		m_pLblGlobalStatData->SetText( wszStatLines );
	}
	else
	{
		m_pLblGlobalStatData->SetText( "#rd_so_global_stat_loading" );
	}
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
}

void CRD_Collection_Panel_Swarmopedia::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	Assert( !"TODO: Swarmopedia big view" );
}
