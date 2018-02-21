#include "cbase.h"
#include "rd_vgui_workshop_download_progress.h"
#include <vgui_controls/Label.h>
#include <vgui_controls/ProgressBar.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui/ILocalize.h>
#include "rd_workshop.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

DECLARE_BUILD_FACTORY( CRD_VGUI_Workshop_Download_Progress );

CRD_VGUI_Workshop_Download_Progress::CRD_VGUI_Workshop_Download_Progress( vgui::Panel *parent, const char *panelName ) : BaseClass( parent, panelName )
{
	m_pLblName = new vgui::Label( this, "LblName", "" );
	m_pPrgDownload = new vgui::ProgressBar( this, "PrgDownload" );
	m_pLblQueue = new vgui::Label( this, "LblQueue", "" );
	m_pImgPreview = new vgui::ImagePanel( this, "ImgPreview" );

	SetPaintBackgroundEnabled( false );
}

CRD_VGUI_Workshop_Download_Progress::~CRD_VGUI_Workshop_Download_Progress()
{
}

void CRD_VGUI_Workshop_Download_Progress::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	LoadControlSettings( "Resource/UI/RDWorkshopDownloadProgress.res" );
}

void CRD_VGUI_Workshop_Download_Progress::OnThink()
{
	BaseClass::OnThink();

	Assert( steamapicontext->SteamUGC() );
	if ( !steamapicontext->SteamUGC() )
	{
		return;
	}

	int nInQueue = 0;
	FOR_EACH_VEC( g_ReactiveDropWorkshop.m_EnabledAddons, i )
	{
		PublishedFileId_t nPublishedFileID = g_ReactiveDropWorkshop.m_EnabledAddons[i].details.m_nPublishedFileId;
		if ( steamapicontext->SteamUGC()->GetItemState( nPublishedFileID ) & k_EItemStateDownloadPending )
		{
			nInQueue++;
		}
	}

	if ( nInQueue )
	{
		wchar_t wszQueueCount[21];
		V_snwprintf( wszQueueCount, sizeof( wszQueueCount ), L"%d", nInQueue );
		wchar_t wszQueue[128];
		g_pVGuiLocalize->ConstructString( wszQueue, sizeof( wszQueue ), g_pVGuiLocalize->FindSafe( "#workshop_number_in_queue" ), 1, wszQueueCount );
		m_pLblQueue->SetText( wszQueue );
		m_pLblQueue->SetVisible( true );
		SetZPos( 20 );
	}
	else
	{
		m_pLblQueue->SetVisible( false );
		SetZPos( -1 );
	}

	int iBestAddonIndex = -1;
	FOR_EACH_VEC( g_ReactiveDropWorkshop.m_EnabledAddons, i )
	{
		PublishedFileId_t nPublishedFileID = g_ReactiveDropWorkshop.m_EnabledAddons[i].details.m_nPublishedFileId;
		if ( steamapicontext->SteamUGC()->GetItemState( nPublishedFileID ) & k_EItemStateDownloadPending )
		{
			iBestAddonIndex = i;
			break;
		}
	}
	FOR_EACH_VEC( g_ReactiveDropWorkshop.m_EnabledAddons, i )
	{
		PublishedFileId_t nPublishedFileID = g_ReactiveDropWorkshop.m_EnabledAddons[i].details.m_nPublishedFileId;
		if ( steamapicontext->SteamUGC()->GetItemState( nPublishedFileID ) & k_EItemStateDownloading )
		{
			iBestAddonIndex = i;
			break;
		}
	}
	FOR_EACH_VEC( g_ReactiveDropWorkshop.m_EnabledAddons, i )
	{
		PublishedFileId_t nPublishedFileID = g_ReactiveDropWorkshop.m_EnabledAddons[i].details.m_nPublishedFileId;
		if ( steamapicontext->SteamUGC()->GetItemState( nPublishedFileID ) & k_EItemStateDownloading )
		{
			uint64 nBytesDownloaded, nBytesTotal;
			if ( steamapicontext->SteamUGC()->GetItemDownloadInfo( nPublishedFileID, &nBytesDownloaded, &nBytesTotal ) && nBytesDownloaded > 0 )
			{
				iBestAddonIndex = i;
				break;
			}
		}
	}
	if ( iBestAddonIndex == -1 )
	{
		m_pImgPreview->SetImage( (vgui::IImage *) NULL );
		m_pImgPreview->SetVisible( false );
		m_pLblName->SetVisible( false );
		m_pPrgDownload->SetVisible( false );
		return;
	}

	PublishedFileId_t nPublishedFileID = g_ReactiveDropWorkshop.m_EnabledAddons[iBestAddonIndex].details.m_nPublishedFileId;
	if ( g_ReactiveDropWorkshop.m_EnabledAddons[iBestAddonIndex].pPreviewImage )
	{
		// avoid a repaint if the image is the same as the one we had before.
		if ( m_pImgPreview->GetImage() != g_ReactiveDropWorkshop.m_EnabledAddons[iBestAddonIndex].pPreviewImage )
		{
			m_pImgPreview->SetImage( g_ReactiveDropWorkshop.m_EnabledAddons[iBestAddonIndex].pPreviewImage );
			m_pImgPreview->SetVisible( true );
		}
	}
	else
	{
		m_pImgPreview->SetImage( (vgui::IImage *) NULL );
		m_pImgPreview->SetVisible( false );
	}

	wchar_t wszName[k_cchPublishedDocumentTitleMax];
	V_UTF8ToUnicode( g_ReactiveDropWorkshop.m_EnabledAddons[iBestAddonIndex].details.m_rgchTitle, wszName, sizeof( wszName ) );
	m_pLblName->SetText( wszName );
	m_pLblName->SetVisible( true );

	uint64 nBytesDownloaded, nBytesTotal;
	if ( steamapicontext->SteamUGC()->GetItemDownloadInfo( nPublishedFileID, &nBytesDownloaded, &nBytesTotal ) && nBytesTotal > 0 )
	{
		m_pPrgDownload->SetProgress( float( nBytesDownloaded ) / float( nBytesTotal ) );
		m_pPrgDownload->SetVisible( true );
	}
	else
	{
		m_pPrgDownload->SetVisible( false );
	}
}
