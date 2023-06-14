#include "cbase.h"
#include "vloadouts.h"
#include "rd_loadout.h"
#include "nb_header_footer.h"
#include "rd_vgui_main_menu_top_bar.h"
#include "filesystem.h"
#include "asw_util_shared.h"
#include "vgui/ILocalize.h"
#include "vgenericwaitscreen.h"
#include "vgenericconfirmation.h"
#include "rd_workshop.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;
using namespace BaseModUI;

Loadouts::Loadouts( Panel *parent, const char *panelName )
	: BaseClass( parent, panelName )
{
	SetProportional( true );

	m_pHeaderFooter = new CNB_Header_Footer( this, "HeaderFooter" );
	m_pHeaderFooter->SetTitle( "" );
	m_pHeaderFooter->SetBackgroundStyle( NB_BACKGROUND_NONE );
	m_pHeaderFooter->SetHeaderEnabled( false );
	m_pHeaderFooter->SetFooterEnabled( false );
	m_pTopBar = new CRD_VGUI_Main_Menu_Top_Bar( this, "TopBar" );
	m_pTopBar->m_hActiveButton = m_pTopBar->m_pTopButton[0];

	ReactiveDropLoadout::InitLoadouts();
	m_pGplSavedLoadouts = new GenericPanelList( this, "GplSavedLoadouts", GenericPanelList::ISM_PERITEM );
	for ( int i = 0; i < int( ReactiveDropLoadout::Loadouts.Count() ); i++ )
	{
		m_pGplSavedLoadouts->AddPanelItem( new CRD_VGUI_Loadout_List_Item( this, "LoadoutListItem", ReactiveDropLoadout::Loadouts.GetElementName( i ), ReactiveDropLoadout::Loadouts[i] ), true );
	}
	PublishedFileId_t nPrevFileID = k_PublishedFileIdInvalid;
	FOR_EACH_VEC( ReactiveDropLoadout::GuideLoadouts, i )
	{
		if ( nPrevFileID != ReactiveDropLoadout::GuideLoadouts[i].FileID )
		{
			nPrevFileID = ReactiveDropLoadout::GuideLoadouts[i].FileID;
			m_pGplSavedLoadouts->AddPanelItem( new CRD_VGUI_Loadout_List_Addon_Header( this, "LoadoutListAddonHeader", nPrevFileID ), true );
		}
		m_pGplSavedLoadouts->AddPanelItem( new CRD_VGUI_Loadout_List_Item( this, "LoadoutListItem", ReactiveDropLoadout::GuideLoadouts[i].Name, ReactiveDropLoadout::GuideLoadouts[i].Loadout, nPrevFileID ), true );
	}

	// loadout name list
	// - load
	// - delete
	// save as
	// medal slot x3
	// for each marine (x8):
	// - marine/suit slot
	// - primary weapon slot
	// - secondary weapon slot
	// - equipment slot
	// options:
	// - load medals from config
	// - auto update current loadout

	SetTitle( "", false );
	SetDeleteSelfOnClose( true );
	SetLowerGarnishEnabled( false );
	SetMoveable( false );
}

Loadouts::~Loadouts()
{
}

void Loadouts::DisplayPublishingError( const char *szMessage, int nArgs, const wchar_t *wszArg1, const wchar_t *wszArg2, const wchar_t *wszArg3, const wchar_t *wszArg4 )
{
	wchar_t wszMessageFormat[1024];
	TryLocalize( szMessage, wszMessageFormat, sizeof( wszMessageFormat ) );
	wchar_t wszMessage[1024];
	g_pVGuiLocalize->ConstructString( wszMessage, sizeof( wszMessage ), wszMessageFormat, nArgs, wszArg1, wszArg2, wszArg3, wszArg4 );

	GenericWaitScreen *pWait = assert_cast< GenericWaitScreen * >( CBaseModPanel::GetSingleton().GetWindow( WT_GENERICWAITSCREEN ) );
	if ( pWait )
	{
		pWait->Close();
	}

	GenericConfirmation *pConfirm = assert_cast< GenericConfirmation * >( CBaseModPanel::GetSingleton().OpenWindow( WT_GENERICCONFIRMATION, this, false ) );
	Assert( pConfirm );
	if ( pConfirm )
	{
		GenericConfirmation::Data_t data;
		data.pWindowTitle = "#rd_loadout_share_error_title";
		data.pMessageTextW = wszMessage;
		data.bOkButtonEnabled = true;
		pConfirm->SetUsageData( data );
	}
}

void Loadouts::StartPublishingLoadouts( const char *szTitle, const char *szDescription, const char *szPreviewFile, const CUtlDict<ReactiveDropLoadout::LoadoutData_t> &loadouts )
{
	CUtlBuffer buf;
	ReactiveDropLoadout::WriteLoadoutsForSharing( buf, loadouts );

	ISteamRemoteStorage *pRemoteStorage = SteamRemoteStorage();
	Assert( pRemoteStorage );
	if ( !pRemoteStorage )
	{
		DisplayPublishingError( "#rd_loadout_share_failed_api" );
		return;
	}
	pRemoteStorage->FileWrite( "integrated_guide.bin", buf.Base(), buf.TellMaxPut() );

	buf.Purge();
	if ( !g_pFullFileSystem->ReadFile( szPreviewFile, NULL, buf ) )
	{
		DisplayPublishingError( "#rd_loadout_share_failed_preview_image" );
		pRemoteStorage->FileDelete( "integrated_guide.bin" );
		return;
	}
	pRemoteStorage->FileWrite( "integrated_guide_preview.jpeg", buf.Base(), buf.TellMaxPut() );

	const char *szLoadoutTag = "Loadouts";
	SteamParamStringArray_t tags;
	tags.m_ppStrings = &szLoadoutTag;
	tags.m_nNumStrings = 1;

	SteamAPICall_t hCall = pRemoteStorage->PublishWorkshopFile( "integrated_guide.bin", "integrated_guide_preview.jpeg",
		SteamUtils()->GetAppID(), szTitle, szDescription,
		k_ERemoteStoragePublishedFileVisibilityPublic, &tags, k_EWorkshopFileTypeIntegratedGuide );
	m_RemoteStoragePublishFileResult.Set( hCall, this, &Loadouts::OnRemoteStoragePublishFileResult );

	GenericWaitScreen *pWait = assert_cast< GenericWaitScreen * >( CBaseModPanel::GetSingleton().OpenWindow( WT_GENERICWAITSCREEN, this, false ) );
	Assert( pWait );
	if ( pWait )
	{
		pWait->ClearData();
		pWait->AddMessageText( g_pVGuiLocalize->Find( "#rd_loadout_share_pending" ), INFINITY );
	}
}

void Loadouts::OnRemoteStoragePublishFileResult( RemoteStoragePublishFileResult_t *pParam, bool bIOFailure )
{
	ISteamRemoteStorage *pRemoteStorage = SteamRemoteStorage();
	Assert( pRemoteStorage );
	if ( !pRemoteStorage )
	{
		DisplayPublishingError( "#rd_loadout_share_failed_api" );
		return;
	}

	if ( bIOFailure )
	{
		DisplayPublishingError( "#rd_loadout_share_failed_api" );
		return;
	}

	pRemoteStorage->FileDelete( "integrated_guide.bin" );
	pRemoteStorage->FileDelete( "integrated_guide_preview.jpeg" );

	if ( pParam->m_eResult != k_EResultOK )
	{
		wchar_t wszEResultNumber[16];
		V_snwprintf( wszEResultNumber, NELEMS( wszEResultNumber ), L"%d", pParam->m_eResult );
		wchar_t wszEResult[64];
		V_UTF8ToUnicode( UTIL_RD_EResultToString( pParam->m_eResult ), wszEResult, sizeof( wszEResult ) );

		DisplayPublishingError( "#rd_loadout_share_failed_eresult", 2, wszEResultNumber, wszEResult );
		return;
	}

	GenericWaitScreen *pWait = assert_cast< GenericWaitScreen * >( CBaseModPanel::GetSingleton().GetWindow( WT_GENERICWAITSCREEN ) );
	if ( pWait )
	{
		pWait->Close();
	}

	// ignoring pParam->m_bUserNeedsToAcceptWorkshopLegalAgreement because the webpage will contain that information.
	g_ReactiveDropWorkshop.OpenWorkshopPageForFile( pParam->m_nPublishedFileId );
}

CRD_VGUI_Loadout_List_Item::CRD_VGUI_Loadout_List_Item( vgui::Panel *parent, const char *panelName, const char *szName, const ReactiveDropLoadout::LoadoutData_t &loadout, PublishedFileId_t id ) :
	BaseClass( parent, panelName )
{
}

CRD_VGUI_Loadout_List_Addon_Header::CRD_VGUI_Loadout_List_Addon_Header( vgui::Panel *parent, const char *panelName, PublishedFileId_t id ) :
	BaseClass( parent, panelName )
{
}
