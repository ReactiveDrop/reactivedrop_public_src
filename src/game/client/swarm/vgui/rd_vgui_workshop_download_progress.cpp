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
	m_pPnlBackground = new vgui::Panel( this, "PnlBackground" );
	m_pLblName = new vgui::Label( this, "LblName", "" );
	m_pPrgDownload = new vgui::ProgressBar( this, "PrgDownload" );
	m_pLblQueue = new vgui::Label( this, "LblQueue", "" );
	m_pImgPreview = new vgui::ImagePanel( this, "ImgPreview" );
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
	static int s_nFrameSkip = 0;
	static int s_nLastAddonCount = -1;
	static int s_nInQueue = 0;
	static int s_iBestAddonIndex = -1;
	static uint64 s_nBestBytesDownloaded = 0, s_nBestBytesTotal = 0;
	static bool s_bFoundDownloadWithProgress = false;

	// Only update every nth call to OnThink (reduce update rate for performance)
	float fTickInterval = gpGlobals->interval_per_tick;
	float fTickRate = (fTickInterval > 0) ? (1.0f / fTickInterval) : 0.0f;

	if (++s_nFrameSkip < fTickRate)
	{
		return;
	}
	s_nFrameSkip = 0;

	BaseClass::OnThink();

	ISteamUGC* pUGC = SteamUGC();
	AssertOnce(pUGC);
	if (!pUGC)
	{
		return;
	}

	const int addonCount = g_ReactiveDropWorkshop.m_EnabledAddons.Count();

	// Only re-scan the addon list if the count has changed (sacrifice accuracy for speed)
	if (addonCount != s_nLastAddonCount)
	{
		s_nLastAddonCount = addonCount;
		s_nInQueue = 0;
		s_iBestAddonIndex = -1;
		s_bFoundDownloadWithProgress = false;
		s_nBestBytesDownloaded = 0;
		s_nBestBytesTotal = 0;

		for (int i = 0; i < addonCount; ++i)
		{
			const auto& addon = g_ReactiveDropWorkshop.m_EnabledAddons[i];
			PublishedFileId_t nPublishedFileID = addon.details.m_nPublishedFileId;
			uint32 itemState = pUGC->GetItemState(nPublishedFileID);

			if (itemState & k_EItemStateDownloadPending)
			{
				++s_nInQueue;
				if (s_iBestAddonIndex == -1)
					s_iBestAddonIndex = i;
			}
			if (!s_bFoundDownloadWithProgress && (itemState & k_EItemStateDownloading))
			{
				uint64 nBytesDownloaded, nBytesTotal;
				if (pUGC->GetItemDownloadInfo(nPublishedFileID, &nBytesDownloaded, &nBytesTotal) && nBytesDownloaded > 0)
				{
					s_iBestAddonIndex = i;
					s_nBestBytesDownloaded = nBytesDownloaded;
					s_nBestBytesTotal = nBytesTotal;
					s_bFoundDownloadWithProgress = true;
				}
			}
		}
	}

	if (s_nInQueue)
	{
		wchar_t wszQueueCount[21];
		V_snwprintf(wszQueueCount, ARRAYSIZE(wszQueueCount), L"%d", s_nInQueue);
		wchar_t wszQueue[128];
		g_pVGuiLocalize->ConstructString(wszQueue, sizeof(wszQueue), g_pVGuiLocalize->Find("#workshop_number_in_queue"), 1, wszQueueCount);
		m_pLblQueue->SetText(wszQueue);
		m_pLblQueue->SetVisible(true);
		SetZPos(20);
	}
	else
	{
		m_pLblQueue->SetVisible(false);
		SetZPos(-1);
	}

	if (s_iBestAddonIndex == -1)
	{
		m_pPnlBackground->SetVisible(false);
		m_pImgPreview->SetImage((vgui::IImage*)NULL);
		m_pImgPreview->SetVisible(false);
		m_pLblName->SetVisible(false);
		m_pPrgDownload->SetVisible(false);
		return;
	}

	PublishedFileId_t nPublishedFileID = g_ReactiveDropWorkshop.m_EnabledAddons[s_iBestAddonIndex].details.m_nPublishedFileId;

	if (g_ReactiveDropWorkshop.m_EnabledAddons[s_iBestAddonIndex].pPreviewImage)
	{
		if (m_pImgPreview->GetImage() != static_cast<vgui::IImage*>(g_ReactiveDropWorkshop.m_EnabledAddons[s_iBestAddonIndex].pPreviewImage))
		{
			m_pImgPreview->SetImage(static_cast<vgui::IImage*>(g_ReactiveDropWorkshop.m_EnabledAddons[s_iBestAddonIndex].pPreviewImage));
			m_pImgPreview->SetVisible(true);
		}
	}
	else
	{
		m_pImgPreview->SetImage((vgui::IImage*)NULL);
		m_pImgPreview->SetVisible(false);
	}

	m_pPnlBackground->SetVisible(true);
	wchar_t wszName[k_cchPublishedDocumentTitleMax];
	V_UTF8ToUnicode(g_ReactiveDropWorkshop.m_EnabledAddons[s_iBestAddonIndex].details.m_rgchTitle, wszName, sizeof(wszName) / sizeof(wszName[0]));
	m_pLblName->SetText(wszName);
	m_pLblName->SetVisible(true);

	uint64 nBytesDownloaded = s_nBestBytesDownloaded, nBytesTotal = s_nBestBytesTotal;
	if (!s_bFoundDownloadWithProgress)
	{
		// Only query if not already found in the loop
		pUGC->GetItemDownloadInfo(nPublishedFileID, &nBytesDownloaded, &nBytesTotal);

		// We are busy, reset s_nLastAddonCount to force a re-scan
		s_nLastAddonCount = -1;
	}
	if (nBytesTotal > 0)
	{
		m_pPrgDownload->SetProgress(float(nBytesDownloaded) / float(nBytesTotal));
		m_pPrgDownload->SetVisible(true);

		// We are done, reset s_nLastAddonCount to force a re-scan
		s_nLastAddonCount = -1;
	}
	else
	{
		// We have no work to do
		m_pPrgDownload->SetVisible(false);
	}
}
