#include "cbase.h"
#include "rd_workshop_frame.h"
#include "uigamedata.h"
#include "basemodui.h"
#include "nb_header_footer.h"
#include "nb_button.h"
#include "vgui_controls/ImagePanel.h"
#include "vgui_controls/TextEntry.h"
#include "vgui_controls/FileOpenDialog.h"
#include "vgui_controls/ProgressBar.h"
#include "vgui/ISystem.h"
#include "vgui/ILocalize.h"
#include "filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;
using namespace BaseModUI;

DECLARE_BUILD_FACTORY( ReactiveDropWorkshopListItem );
DECLARE_BUILD_FACTORY( ReactiveDropWorkshop );

ConVar rd_workshop_allow_item_creation( "rd_workshop_allow_item_creation", "0", FCVAR_ARCHIVE, "If set to 1 then you can create new workshop items." );

ReactiveDropWorkshopListItem::ReactiveDropWorkshopListItem( Panel *parent, const char *panelName ) : BaseClass( parent, panelName )
{
	SetProportional( true );

	m_nPublishedFileID = k_PublishedFileIdInvalid;

	m_pPnlHighlight = new vgui::EditablePanel( this, "PnlHighlight" );
	m_pPnlHighlight->SetMouseInputEnabled( false );
	m_pLblHighlight = new vgui::Label( this, "LblHighlight", "" );
	m_pLblHighlight->SetMouseInputEnabled( false );
	m_pImgPreview = new vgui::ImagePanel( this, "ImgPreview" );
	m_pImgPreview->SetMouseInputEnabled( false );
	m_pLblName = new vgui::Label( this, "LblName", "" );
	m_pLblName->SetMouseInputEnabled( false );

	SetMouseInputEnabled( true );
}

ReactiveDropWorkshopListItem::~ReactiveDropWorkshopListItem()
{
}

const CReactiveDropWorkshop::WorkshopItem_t &ReactiveDropWorkshopListItem::GetDetails()
{
	FOR_EACH_VEC( g_ReactiveDropWorkshop.m_EnabledAddons, i )
	{
		if ( g_ReactiveDropWorkshop.m_EnabledAddons[i].details.m_nPublishedFileId == m_nPublishedFileID )
		{
			return g_ReactiveDropWorkshop.m_EnabledAddons[i];
		}
	}

	static CReactiveDropWorkshop::WorkshopItem_t emptyWorkshopItem;

	Assert( !"could not find workshop item" );
	return emptyWorkshopItem;
}

CReactiveDropWorkshopPreviewImage *ReactiveDropWorkshopListItem::GetPreviewImage()
{
	FOR_EACH_VEC( g_ReactiveDropWorkshop.m_EnabledAddons, i )
	{
		if ( g_ReactiveDropWorkshop.m_EnabledAddons[i].details.m_nPublishedFileId == m_nPublishedFileID )
		{
			return g_ReactiveDropWorkshop.m_EnabledAddons[i].pPreviewImage;
		}
	}

	Assert( !"could not find workshop item" );
	return NULL;
}

void ReactiveDropWorkshopListItem::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	LoadControlSettings( "Resource/UI/BaseModUI/RDWorkshopListItem.res" );

	if ( m_nPublishedFileID != k_PublishedFileIdInvalid )
	{
		UpdateDetails();
	}
}

void ReactiveDropWorkshopListItem::OnMousePressed( vgui::MouseCode code )
{
	BaseClass::OnMousePressed( code );
	if ( code == MOUSE_LEFT )
	{
		if ( GenericPanelList *pGPL = dynamic_cast<GenericPanelList *>( GetParent()->GetParent() ) )
		{
			pGPL->SelectPanelItemByPanel( this );
		}
	}
}

void ReactiveDropWorkshopListItem::OnPanelUnSelected()
{
	m_pPnlHighlight->SetVisible( false );
	m_pLblHighlight->SetVisible( false );
}

void ReactiveDropWorkshopListItem::OnPanelSelected()
{
	m_pPnlHighlight->SetVisible( true );
	m_pLblHighlight->SetVisible( true );
}

void ReactiveDropWorkshopListItem::UpdateDetails()
{
	const CReactiveDropWorkshop::WorkshopItem_t &item = GetDetails();

	if ( !item.details.m_rgchTitle[0] )
	{
		m_pImgPreview->SetImage( "" );
		m_pImgPreview->SetImage( "swarm/MissionPics/UnknownMissionPic" );
		m_pLblName->SetText( "#rd_workshop_dummy_item" );
		m_pLblHighlight->SetText( "#rd_workshop_dummy_item" );
	}
	else
	{
		if ( CReactiveDropWorkshopPreviewImage *pPreview = GetPreviewImage() )
		{
			m_pImgPreview->SetImage( pPreview );
		}
		else
		{
			m_pImgPreview->SetImage( "" );
			m_pImgPreview->SetImage( "common/swarm_cycle" );
		}
		wchar_t wszTitle[k_cchPublishedDocumentTitleMax];
		V_UTF8ToUnicode( item.details.m_rgchTitle, wszTitle, sizeof( wszTitle ) );
		m_pLblName->SetText( wszTitle );
		m_pLblHighlight->SetText( wszTitle );
	}
}

ReactiveDropWorkshop::ReactiveDropWorkshop( Panel *parent, const char *panelName ) : BaseClass( parent, panelName )
{
	GameUI().PreventEngineHideGameUI();

	SetDeleteSelfOnClose( true );
	SetProportional( true );

	m_hSingleItemQuery = k_UGCQueryHandleInvalid;
	m_nEditingWorkshopID = k_PublishedFileIdInvalid;
	m_hUpdate = k_UGCUpdateHandleInvalid;
	m_bEditing = false;

	m_pHeaderFooter = new CNB_Header_Footer( this, "HeaderFooter" );
	m_pHeaderFooter->SetTitle( "#rd_workshop_published_addons" );
	m_pHeaderFooter->SetHeaderEnabled( true );
	m_pHeaderFooter->SetGradientBarEnabled( true );
	m_pHeaderFooter->SetGradientBarPos( 40, 410 );

	m_pLblWaiting = new vgui::Label( this, "LblWaiting", "#rd_workshop_waiting_for_addon_list" );
	m_pPrgWaiting = new vgui::ProgressBar( this, "PrgWaiting" );

	m_pBtnCreateWorkshopItem = new CNB_Button( this, "BtnCreateWorkshopItem", "#rd_workshop_create_addon", this, "CreateWorkshopItem" );

	m_pGplWorkshopItems = new GenericPanelList( this, "GplWorkshopItems", GenericPanelList::ISM_ELEVATOR );
	m_pGplWorkshopItems->AddActionSignalTarget( this );
	m_pGplWorkshopItems->SetScrollBarVisible( true );

	m_pBtnEditContent = new CNB_Button( this, "BtnEditContent", "#rd_workshop_choose_vpk", this, "ChooseVPK" );
	m_pLblEditingPreview = new vgui::Label( this, "LblEditingPreview", "#rd_workshop_editing_preview" );
	m_pImgEditingPreview = new vgui::ImagePanel( this, "ImgEditingPreview" );
	m_pBtnEditPreview = new CNB_Button( this, "BtnEditPreview", "#rd_workshop_choose_jpeg", this, "ChooseJPEG" );
	m_pLblEditingTitle = new vgui::Label( this, "LblEditingTitle", "#rd_workshop_editing_title" );
	m_pTxtEditingTitle = new vgui::TextEntry( this, "TxtEditingTitle" );
	m_pTxtEditingTitle->SetMaximumCharCount( k_cchPublishedDocumentTitleMax - 1 );
	m_pLblEditingDescription = new vgui::Label( this, "LblEditingDescription", "#rd_workshop_editing_description" );
	m_pTxtEditingDescription = new vgui::TextEntry( this, "TxtEditingDescription" );
	m_pTxtEditingDescription->SetMultiline( true );
	m_pTxtEditingDescription->SetCatchEnterKey( true );
	m_pTxtEditingDescription->SetMaximumCharCount( k_cchPublishedDocumentDescriptionMax - 1 );
	m_pLblEditingTags = new vgui::Label( this, "LblEditingTags", "#rd_workshop_editing_tags" );
	m_pTxtEditingTags = new vgui::TextEntry( this, "TxtEditingTags" );
	m_pTxtEditingTags->SetMaximumCharCount( 127 );
	m_pLblEditingChangeDescription = new vgui::Label( this, "LblEditingChangeDescription", "#rd_workshop_editing_change_description" );
	m_pTxtEditingChangeDescription = new vgui::TextEntry( this, "TxtEditingChangeDescription" );
	m_pTxtEditingChangeDescription->SetMultiline( true );
	m_pTxtEditingChangeDescription->SetCatchEnterKey( true );
	m_pTxtEditingChangeDescription->SetMaximumCharCount( k_cchPublishedDocumentChangeDescriptionMax - 1 );
	m_pBtnEdit = new CNB_Button( this, "BtnEdit", "#rd_workshop_edit_addon", this, "EditWorkshopItem" );
	m_pBtnCancel = new CNB_Button( this, "BtnCancel", "#rd_workshop_cancel_edit", this, "CancelEdit" );
	m_pBtnSubmit = new CNB_Button( this, "BtnSubmit", "#rd_workshop_submit_edit", this, "SubmitEdit" );
	m_pBtnOpen = new CNB_Button( this, "BtnOpen", "#rd_workshop_open", this, "OpenInWorkshop" );

	m_pFreePreviewImage = NULL;

	LoadControlSettings( "Resource/UI/BaseModUI/RDWorkshop.res" );
}

ReactiveDropWorkshop::~ReactiveDropWorkshop()
{
	GameUI().AllowEngineHideGameUI();
}

void ReactiveDropWorkshop::Activate()
{
	BaseClass::Activate();

	InvalidateLayout();
}

void ReactiveDropWorkshop::PerformLayout()
{
	if ( !m_bEditing )
	{
		if ( g_ReactiveDropWorkshop.m_bHaveAllPublishedAddons )
		{
			InitReady();
		}
		else
		{
			InitWait();
		}
	}

	BaseClass::PerformLayout();
}

void ReactiveDropWorkshop::OnCommand( const char *command )
{
	if ( !V_strcmp( command, "Back" ) )
	{
		// Act as though 360 back button was pressed
		OnKeyCodePressed( ButtonCodeToJoystickButtonCode( KEY_XBUTTON_B, CBaseModPanel::GetSingleton().GetLastActiveUserId() ) );
	}
	else if ( !V_strcmp( command, "CreateWorkshopItem" ) )
	{
		if ( m_CreateItemCall.IsActive() )
		{
			CBaseModPanel::GetSingleton().PlayUISound( UISOUND_DENY );
			return;
		}

		if ( rd_workshop_allow_item_creation.GetBool() )
		{
			CUIGameData::Get()->OpenWaitScreen( "#rd_workshop_creating_item", 0 );
			SteamAPICall_t hAPICall = SteamUGC()->CreateItem( SteamUtils()->GetAppID(), k_EWorkshopFileTypeCommunity );
			m_CreateItemCall.Set( hAPICall, this, &ReactiveDropWorkshop::CreateItemCall );
		}
		else
		{
			CUIGameData::Get()->DisplayOkOnlyMsgBox( this, "#RDUI_WorkshopModeWarning_Title", "#RDUI_WorkshopModeWarning_Text" );
			Msg( "Please set rd_workshop_allow_item_creation 1 before doing this action\n" );
		}
	}
	else if ( !V_strcmp( command, "EditWorkshopItem" ) )
	{
		m_bEditing = true;
		m_pGplWorkshopItems->SetMouseInputEnabled( false );

		m_pTxtEditingTitle->SetEditable( true );
		m_pTxtEditingDescription->SetEditable( true );
		m_pTxtEditingTags->SetEditable( true );
		m_pLblEditingChangeDescription->SetVisible( true );
		m_pTxtEditingChangeDescription->SetEditable( true );
		m_pTxtEditingChangeDescription->SetVisible( true );
		m_pBtnEdit->SetVisible( false );
		m_pBtnOpen->SetVisible( false );
		m_pBtnCancel->SetVisible( true );
		m_pBtnSubmit->SetVisible( true );
		m_pBtnSubmit->SetEnabled( false );
		if ( ReactiveDropWorkshopListItem *pItem = dynamic_cast<ReactiveDropWorkshopListItem *>( m_pGplWorkshopItems->GetSelectedPanelItem() ) )
		{
			if ( pItem->GetDetails().details.m_rgchTitle[0] && pItem->GetPreviewImage() )
			{
				m_pBtnSubmit->SetEnabled( true );
			}
		}
		m_pBtnEditContent->SetVisible( true );
		m_pBtnEditContent->SetText( "#rd_workshop_choose_vpk" );
		m_pBtnEditPreview->SetVisible( true );
	}
	else if ( !V_strcmp( command, "OpenInWorkshop" ) )
	{
		if ( ReactiveDropWorkshopListItem *pItem = dynamic_cast<ReactiveDropWorkshopListItem *>( m_pGplWorkshopItems->GetSelectedPanelItem() ) )
		{
			g_ReactiveDropWorkshop.OpenWorkshopPageForFile( pItem->m_nPublishedFileID );
		}
	}
	else if ( !V_strcmp( command, "CancelEdit" ) )
	{
		g_ReactiveDropWorkshop.m_szContentPath.Clear();
		m_szPreviewImage.Clear();
		m_bEditing = false;
		m_pGplWorkshopItems->SetMouseInputEnabled( true );

		if ( ReactiveDropWorkshopListItem *pItem = dynamic_cast<ReactiveDropWorkshopListItem *>( m_pGplWorkshopItems->GetSelectedPanelItem() ) )
		{
			InitEdit( pItem->GetDetails(), pItem->GetPreviewImage() );
		}
		else
		{
			Activate();
		}
	}
	else if ( !V_strcmp( command, "SubmitEdit" ) )
	{
		m_hUpdate = SteamUGC()->StartItemUpdate( SteamUtils()->GetAppID(), m_nEditingWorkshopID );

		if ( !m_szPreviewImage.IsEmpty() )
		{
			if ( !SteamUGC()->SetItemPreview( m_hUpdate, m_szPreviewImage ) )
			{
				CUIGameData::Get()->DisplayOkOnlyMsgBox( this, "#rd_workshop_error_title", "#rd_workshop_error_update_preview" );
				return;
			}
		}

		if ( !g_ReactiveDropWorkshop.m_szContentPath.IsEmpty() )
		{
			if ( !SteamUGC()->SetItemContent( m_hUpdate, g_ReactiveDropWorkshop.m_szContentPath ) )
			{
				CUIGameData::Get()->DisplayOkOnlyMsgBox( this, "#rd_workshop_error_title", "#rd_workshop_error_update_content" );
				return;
			}
			g_ReactiveDropWorkshop.SetWorkshopKeyValues( m_hUpdate );
		}
		else
		{
			FOR_EACH_VEC( m_aszAutoTags, i )
			{
				g_ReactiveDropWorkshop.m_aszTags.CopyAndAddToTail( m_aszAutoTags[i] );
			}
		}
		char szTags[k_cchTagListMax];
		m_pTxtEditingTags->GetText( szTags, sizeof( szTags ) );
		CSplitString aszTags( szTags, "," );
		FOR_EACH_VEC( aszTags, i )
		{
			while ( aszTags[i][0] && V_isspace( aszTags[i][0] ) )
			{
				aszTags[i]++;
			}
			int len = V_strlen( aszTags[i] );
			while ( len > 0 && V_isspace( aszTags[i][len - 1] ) )
			{
				len--;
				aszTags[i][len] = 0;
			}
			if ( len == 0 )
			{
				continue;
			}
			if ( !g_ReactiveDropWorkshop.IsAutoTag( aszTags[i] ) )
			{
				g_ReactiveDropWorkshop.m_aszTags.CopyAndAddToTail( aszTags[i] );
			}
		}
		g_ReactiveDropWorkshop.RemoveDuplicateTags();
		SteamParamStringArray_t tags;
		tags.m_nNumStrings = g_ReactiveDropWorkshop.m_aszTags.Count();
		tags.m_ppStrings = const_cast<const char **>( g_ReactiveDropWorkshop.m_aszTags.Base() );
		if ( !SteamUGC()->SetItemTags( m_hUpdate, &tags ) )
		{
			CUIGameData::Get()->DisplayOkOnlyMsgBox( this, "#rd_workshop_error_title", "#rd_workshop_error_update_tags" );
			return;
		}

		char szTitle[k_cchPublishedDocumentTitleMax];
		m_pTxtEditingTitle->GetText( szTitle, sizeof( szTitle ) );
		if ( !*szTitle )
		{
			V_snprintf( szTitle, sizeof( szTitle ), "Untitled Addon" );
		}
		if ( !SteamUGC()->SetItemTitle( m_hUpdate, szTitle ) )
		{
			CUIGameData::Get()->DisplayOkOnlyMsgBox( this, "#rd_workshop_error_title", "#rd_workshop_error_update_title" );
			return;
		}

		char szDescription[k_cchPublishedDocumentDescriptionMax];
		m_pTxtEditingDescription->GetText( szDescription, sizeof( szDescription ) );
		if ( !SteamUGC()->SetItemDescription( m_hUpdate, szDescription ) )
		{
			CUIGameData::Get()->DisplayOkOnlyMsgBox( this, "#rd_workshop_error_title", "#rd_workshop_error_update_description" );
			return;
		}

		SetMouseInputEnabled( false );
		SetKeyBoardInputEnabled( false );

		char szChangeDescription[k_cchPublishedDocumentChangeDescriptionMax];
		m_pTxtEditingChangeDescription->GetText( szChangeDescription, sizeof( szChangeDescription ) );
		SteamAPICall_t hAPICall = SteamUGC()->SubmitItemUpdate( m_hUpdate, szChangeDescription );
		m_SubmitItemUpdateCall.Set( hAPICall, this, &ReactiveDropWorkshop::SubmitItemUpdateCall );

		InitWait();
	}
	else if ( !V_strcmp( command, "ChooseVPK" ) )
	{
		vgui::FileOpenDialog *pDialog = new vgui::FileOpenDialog( NULL, "#rd_workshop_choose_vpk", vgui::FOD_OPEN );
		pDialog->AddFilter( "*.vpk", "VPK Files", true, "VPK" );
		pDialog->SetDeleteSelfOnClose( true );
		pDialog->AddActionSignalTarget( this );
		pDialog->SetStartDirectoryContext( "workshop_vpk", vgui::system()->GetDesktopFolderPath() );
		pDialog->DoModal();
		pDialog->Activate();
	}
	else if ( !V_strcmp( command, "ChooseJPEG" ) )
	{
		vgui::FileOpenDialog *pDialog = new vgui::FileOpenDialog( NULL, "#rd_workshop_choose_jpeg", vgui::FOD_OPEN );
		pDialog->AddFilter( "*.jpg,*.jpeg", "JPEG Images", true, "JPEG" );
		pDialog->SetDeleteSelfOnClose( true );
		pDialog->AddActionSignalTarget( this );
		pDialog->SetStartDirectoryContext( "workshop_jpeg", vgui::system()->GetDesktopFolderPath() );
		pDialog->DoModal();
		pDialog->Activate();
	}
	else
	{
		BaseClass::OnCommand( command );
	}
}

void ReactiveDropWorkshop::OnThink()
{
	BaseClass::OnThink();

	if ( m_SubmitItemUpdateCall.IsActive() )
	{
		uint64 nBytesProcessed, nBytesTotal;
		EItemUpdateStatus status = SteamUGC()->GetItemUpdateProgress( m_hUpdate, &nBytesProcessed, &nBytesTotal );

		switch ( status )
		{
		default:
		case k_EItemUpdateStatusInvalid:
			// The item update handle was invalid, job might be finished, listen too SubmitItemUpdateResult_t
			m_pLblWaiting->SetText( "#rd_workshop_update_status_invalid" );
			break;
		case k_EItemUpdateStatusPreparingConfig:
			// The item update is processing configuration data
			m_pLblWaiting->SetText("#rd_workshop_update_status_preparing_config");
			break;
		case k_EItemUpdateStatusPreparingContent:
			// The item update is reading and processing content files
			m_pLblWaiting->SetText( "#rd_workshop_update_status_preparing_content" );
			break;
		case k_EItemUpdateStatusUploadingContent:
			// The item update is uploading content changes to Steam
			m_pLblWaiting->SetText( "#rd_workshop_update_status_uploading_content" );
			break;
		case k_EItemUpdateStatusUploadingPreviewFile:
			// The item update is uploading new preview file image
			m_pLblWaiting->SetText( "#rd_workshop_update_status_uploading_preview_file" );
			break;
		case k_EItemUpdateStatusCommittingChanges:
			// The item update is committing all changes
			m_pLblWaiting->SetText( "#rd_workshop_update_status_committing_changes" );
			break;
		}

		m_pLblWaiting->SetVisible( true );
		m_pPrgWaiting->SetProgress( nBytesTotal ? float( nBytesProcessed ) / float( nBytesTotal ) : 0 );
		m_pPrgWaiting->SetVisible( true );
	}
}

void ReactiveDropWorkshop::OnClose()
{
	if ( m_SubmitItemUpdateCall.IsActive() )
	{
		return;
	}

	BaseClass::OnClose();
}

void ReactiveDropWorkshop::OnWorkshopPreviewReady( PublishedFileId_t nFileID, CReactiveDropWorkshopPreviewImage *pPreviewImage )
{
	for ( unsigned short i = 0; i < m_pGplWorkshopItems->GetPanelItemCount(); i++ )
	{
		if ( ReactiveDropWorkshopListItem *pItem = dynamic_cast<ReactiveDropWorkshopListItem *>( m_pGplWorkshopItems->GetPanelItem( i ) ) )
		{
			if ( pItem->m_nPublishedFileID == nFileID )
			{
				pItem->m_pImgPreview->SetImage( pPreviewImage );
				break;
			}
		}
	}

	if ( m_nEditingWorkshopID == nFileID )
	{
		m_pImgEditingPreview->SetImage( pPreviewImage );
	}
}

void ReactiveDropWorkshop::OnItemSelected( const char *panelName )
{
	if ( ReactiveDropWorkshopListItem *pItem = dynamic_cast<ReactiveDropWorkshopListItem *>( m_pGplWorkshopItems->GetSelectedPanelItem() ) )
	{
		InitEdit( pItem->GetDetails(), pItem->GetPreviewImage() );
	}
}

void ReactiveDropWorkshop::OnFileSelected( const char *fullpath, const char *filterinfo )
{
	if ( !V_strcmp( filterinfo, "VPK" ) )
	{
		if ( !g_ReactiveDropWorkshop.m_szContentPath.IsEmpty() && g_ReactiveDropWorkshop.m_SubmitItemUpdateResultCallback.IsActive() )
		{
			m_pBtnEditContent->SetText( "#rd_workshop_choose_vpk" );
			CUIGameData::Get()->DisplayOkOnlyMsgBox( this, "#rd_workshop_error_title", "#rd_workshop_error_unknown" );
			return;
		}
		g_ReactiveDropWorkshop.m_szContentPath.Clear();
		CUtlString szDisallowedFiles;
		if ( !g_ReactiveDropWorkshop.PrepareWorkshopVPK( fullpath, &szDisallowedFiles ) )
		{
			m_pBtnEditContent->SetText( "#rd_workshop_choose_vpk" );
			if ( !szDisallowedFiles.IsEmpty() )
			{
				CUIGameData::Get()->DisplayOkOnlyMsgBox( this, "#rd_workshop_error_disallowed_filenames", szDisallowedFiles );
				return;
			}

			CUIGameData::Get()->DisplayOkOnlyMsgBox( this, "#rd_workshop_error_title", "#rd_workshop_error_unknown" );
			return;
		}
		wchar_t wszFileSize[64];
		V_snwprintf( wszFileSize, ARRAYSIZE( wszFileSize ), L"%.2f", filesystem->Size( fullpath ) / 1024.0f / 1024.0f );
		wchar_t wszEditContent[256];
		g_pVGuiLocalize->ConstructString( wszEditContent, sizeof( wszEditContent ), g_pVGuiLocalize->FindSafe( "#rd_workshop_choose_vpk_size" ), 1, wszFileSize );
		m_pBtnEditContent->SetText( wszEditContent );
		if ( !m_szPreviewImage.IsEmpty() )
		{
			m_pBtnSubmit->SetEnabled( true );
		}
	}
	else if ( !V_strcmp( filterinfo, "JPEG" ) )
	{
		unsigned int iSize = filesystem->Size( fullpath );
		if ( iSize > 1024 * 1024 )
		{
			CUIGameData::Get()->DisplayOkOnlyMsgBox( this, "#rd_workshop_error_title", "#rd_workshop_error_jpeg_size_limit" );
			return;
		}
		CUtlBuffer buf;
		if ( !filesystem->ReadFile( fullpath, NULL, buf ) )
		{
			Warning( "Could not read file chosen by JPEG file selection dialog: %s\n", fullpath );
			CUIGameData::Get()->DisplayOkOnlyMsgBox( this, "#rd_workshop_error_title", "#rd_workshop_error_unknown" );
			return;
		}
		CReactiveDropWorkshopPreviewImage *pImage = new CReactiveDropWorkshopPreviewImage( buf );
		if ( pImage->GetID() == -1 )
		{
			Warning( "Could not decode file chosen by JPEG file selection dialog: %s\n", fullpath );
			delete pImage;
			CUIGameData::Get()->DisplayOkOnlyMsgBox( this, "#rd_workshop_error_title", "#rd_workshop_error_unknown" );
			return;
		}
		m_pImgEditingPreview->SetImage( pImage );
		m_pFreePreviewImage = pImage;
		m_szPreviewImage = fullpath;
		if ( !g_ReactiveDropWorkshop.m_szContentPath.IsEmpty() )
		{
			m_pBtnSubmit->SetEnabled( true );
		}
	}
}

void ReactiveDropWorkshop::InitReady()
{
	m_pLblWaiting->SetVisible( false );
	m_pPrgWaiting->SetVisible( false );
	m_pBtnEditContent->SetVisible( false );
	m_pBtnEditPreview->SetVisible( false );
	m_pImgEditingPreview->SetVisible( false );
	m_pTxtEditingTitle->SetVisible( false );
	m_pTxtEditingDescription->SetVisible( false );
	m_pTxtEditingTags->SetVisible( false );
	m_pTxtEditingChangeDescription->SetVisible( false );
	m_pLblEditingTitle->SetVisible( false );
	m_pLblEditingPreview->SetVisible( false );
	m_pLblEditingDescription->SetVisible( false );
	m_pLblEditingTags->SetVisible( false );
	m_pLblEditingChangeDescription->SetVisible( false );
	m_pBtnEdit->SetVisible( false );
	m_pBtnCancel->SetVisible( false );
	m_pBtnSubmit->SetVisible( false );
	m_pBtnOpen->SetVisible( false );
	m_pGplWorkshopItems->RemoveAllPanelItems();

	PublishedFileId_t nDummyItem = k_PublishedFileIdInvalid;

	CSteamID currentUser = SteamUser()->GetSteamID();
	FOR_EACH_VEC( g_ReactiveDropWorkshop.m_EnabledAddons, i )
	{
		if ( currentUser == g_ReactiveDropWorkshop.m_EnabledAddons[i].details.m_ulSteamIDOwner )
		{
			if ( !g_ReactiveDropWorkshop.m_EnabledAddons[i].details.m_rgchTitle[0] )
			{
				nDummyItem = g_ReactiveDropWorkshop.m_EnabledAddons[i].details.m_nPublishedFileId;
			}
			else
			{
				AddWorkshopItem( g_ReactiveDropWorkshop.m_EnabledAddons[i].details.m_nPublishedFileId );
			}
		}
	}

	if ( nDummyItem != k_PublishedFileIdInvalid )
	{
		AddWorkshopItem( nDummyItem );
		m_pBtnCreateWorkshopItem->SetVisible( false );
	}
	else
	{
		m_pBtnCreateWorkshopItem->SetVisible( true );
	}
}

void ReactiveDropWorkshop::InitWait()
{
	m_pGplWorkshopItems->RemoveAllPanelItems();
	m_pGplWorkshopItems->SetVisible( false );
	m_pBtnCreateWorkshopItem->SetVisible( false );
	m_pLblWaiting->SetVisible( true );
	m_pPrgWaiting->SetVisible( false );
	m_pBtnEditContent->SetVisible( false );
	m_pBtnEditPreview->SetVisible( false );
	m_pImgEditingPreview->SetVisible( false );
	m_pTxtEditingTitle->SetVisible( false );
	m_pTxtEditingDescription->SetVisible( false );
	m_pTxtEditingTags->SetVisible( false );
	m_pTxtEditingChangeDescription->SetVisible( false );
	m_pLblEditingPreview->SetVisible( false );
	m_pLblEditingTitle->SetVisible( false );
	m_pLblEditingDescription->SetVisible( false );
	m_pLblEditingTags->SetVisible( false );
	m_pLblEditingChangeDescription->SetVisible( false );
	m_pBtnEdit->SetVisible( false );
	m_pBtnCancel->SetVisible( false );
	m_pBtnSubmit->SetVisible( false );
	m_pBtnOpen->SetVisible( false );
}

void ReactiveDropWorkshop::InitEdit( const CReactiveDropWorkshop::WorkshopItem_t &item, CReactiveDropWorkshopPreviewImage *pPreview )
{
	m_nEditingWorkshopID = item.details.m_nPublishedFileId;

	m_pLblEditingPreview->SetVisible( true );
	if ( pPreview )
	{
		m_pImgEditingPreview->SetImage( pPreview );
	}
	else
	{
		m_pImgEditingPreview->SetImage( "" ); // clear out the image
		m_pImgEditingPreview->SetImage( "swarm/MissionPics/UnknownMissionPic" );
	}
	m_pImgEditingPreview->SetVisible( true );

	wchar_t wszTitle[k_cchPublishedDocumentTitleMax];
	V_UTF8ToUnicode( item.details.m_rgchTitle, wszTitle, sizeof( wszTitle ) );
	m_pTxtEditingTitle->SetText( wszTitle );
	m_pTxtEditingTitle->SetVisible( true );
	m_pTxtEditingTitle->SetEditable( false );

	wchar_t wszDescription[k_cchPublishedDocumentDescriptionMax];
	V_UTF8ToUnicode( item.details.m_rgchDescription, wszDescription, sizeof( wszDescription ) );
	m_pTxtEditingDescription->SetText( wszDescription );
	m_pTxtEditingDescription->SetVisible( true );
	m_pTxtEditingDescription->SetEditable( false );

	m_aszAutoTags.PurgeAndDeleteElements();
	CUtlString szUserTags;

	CSplitString tags( item.details.m_rgchTags, "," );
	FOR_EACH_VEC( tags, i )
	{
		if ( g_ReactiveDropWorkshop.IsAutoTag( tags[i] ) )
		{
			m_aszAutoTags.CopyAndAddToTail( tags[i] );
		}
		else
		{
			if ( !szUserTags.IsEmpty() )
			{
				szUserTags += ", ";
			}
			szUserTags += tags[i];
		}
	}
	m_pTxtEditingTags->SetText( szUserTags );
	m_pTxtEditingTags->SetVisible( true );
	m_pTxtEditingTags->SetEditable( false );

	m_pTxtEditingChangeDescription->SetText( "" );
	m_pTxtEditingChangeDescription->SetVisible( false );

	m_pLblEditingTitle->SetVisible( true );
	m_pLblEditingDescription->SetVisible( true );
	m_pLblEditingTags->SetVisible( true );
	m_pLblEditingChangeDescription->SetVisible( false );

	m_pBtnEditContent->SetVisible( false );
	m_pBtnEditPreview->SetVisible( false );
	m_pBtnEdit->SetVisible( true );
	m_pBtnCancel->SetVisible( false );
	m_pBtnSubmit->SetVisible( false );
	m_pBtnOpen->SetVisible( true );
	m_pBtnOpen->SetEnabled( item.details.m_rgchTitle[0] != 0 );
}

ReactiveDropWorkshopListItem *ReactiveDropWorkshop::AddWorkshopItem( PublishedFileId_t nFileID )
{
	ReactiveDropWorkshopListItem *pItem = m_pGplWorkshopItems->AddPanelItem<ReactiveDropWorkshopListItem>( "RDWorkshopListItem" );
	pItem->m_nPublishedFileID = nFileID;
	pItem->UpdateDetails();
	pItem->SetVisible( true );
	return pItem;
}

void ReactiveDropWorkshop::RequestSingleItem( PublishedFileId_t nPublishedFileID )
{
	Assert( !m_RequestSingleItemCall.IsActive() );

	CUIGameData::Get()->OpenWaitScreen( "#rd_workshop_retrieving_item_details", 0 );

	m_hSingleItemQuery = SteamUGC()->CreateQueryUGCDetailsRequest( &nPublishedFileID, 1 );
	SteamUGC()->SetReturnLongDescription( m_hSingleItemQuery, true );
	SteamUGC()->SetReturnKeyValueTags( m_hSingleItemQuery, true );
	SteamAPICall_t hAPICall = SteamUGC()->SendQueryUGCRequest( m_hSingleItemQuery );
	m_RequestSingleItemCall.Set( hAPICall, this, &ReactiveDropWorkshop::RequestSingleItemCall );
}

template<typename Result_t>
bool ReactiveDropWorkshop::OnAPIResult( const Result_t *pResult, bool bIOFailure, const char *szCallName )
{
	CUIGameData::Get()->CloseWaitScreen( NULL, NULL );

	if ( bIOFailure )
	{
		CUIGameData::Get()->DisplayOkOnlyMsgBox( this, "#rd_workshop_error_title", "#rd_workshop_error_io_failure" );
		return false;
	}

	if ( pResult->m_eResult != k_EResultOK )
	{
		Warning( "Workshop %s failed: EResult %d\n", szCallName, pResult->m_eResult );
	}

	if ( pResult->m_eResult == k_EResultInsufficientPrivilege )
	{
		CUIGameData::Get()->DisplayOkOnlyMsgBox( this, "#rd_workshop_error_title", "#rd_workshop_error_insufficient_privilege" );
		return false;
	}

	if ( pResult->m_eResult == k_EResultTimeout )
	{
		CUIGameData::Get()->DisplayOkOnlyMsgBox( this, "#rd_workshop_error_title", "#rd_workshop_error_timeout" );
		return false;
	}

	if ( pResult->m_eResult == k_EResultNotLoggedOn )
	{
		CUIGameData::Get()->DisplayOkOnlyMsgBox( this, "#rd_workshop_error_title", "#rd_workshop_error_not_logged_on" );
		return false;
	}

	if ( pResult->m_eResult != k_EResultOK )
	{
		CUIGameData::Get()->DisplayOkOnlyMsgBox( this, "#rd_workshop_error_title", "#rd_workshop_error_unknown" );
		return false;
	}

	return true;
}

void ReactiveDropWorkshop::RequestSingleItemCall( SteamUGCQueryCompleted_t *pResult, bool bIOFailure )
{
	if ( !OnAPIResult( pResult, bIOFailure, "request workshop item" ) )
	{
		return;
	}

	SteamUGCDetails_t details;
	SteamUGC()->GetQueryUGCResult( m_hSingleItemQuery, 0, &details );

	g_ReactiveDropWorkshop.AddAddonsToCache( pResult, bIOFailure, m_hSingleItemQuery );

	for ( unsigned short i = 0; i < m_pGplWorkshopItems->GetPanelItemCount(); i++ )
	{
		if ( ReactiveDropWorkshopListItem *pItem = dynamic_cast<ReactiveDropWorkshopListItem *>( m_pGplWorkshopItems->GetPanelItem( i ) ) )
		{
			if ( pItem->m_nPublishedFileID == details.m_nPublishedFileId )
			{
				pItem->UpdateDetails();
				m_pGplWorkshopItems->SelectPanelItemByPanel( pItem );
				return;
			}
		}
	}

	ReactiveDropWorkshopListItem *pItem = AddWorkshopItem( details.m_nPublishedFileId );
	m_pGplWorkshopItems->SelectPanelItemByPanel( pItem );
	if ( !details.m_rgchTitle[0] )
	{
		OnCommand( "Edit" );
	}
}

void ReactiveDropWorkshop::CreateItemCall( CreateItemResult_t *pResult, bool bIOFailure )
{
	if ( !OnAPIResult( pResult, bIOFailure, "create item" ) )
	{
		return;
	}

	m_pBtnCreateWorkshopItem->SetVisible( false );
	RequestSingleItem( pResult->m_nPublishedFileId );
}

void ReactiveDropWorkshop::SubmitItemUpdateCall( SubmitItemUpdateResult_t *pResult, bool bIOFailure )
{
	m_pLblWaiting->SetVisible( false );
	m_pPrgWaiting->SetVisible( false );
	SetMouseInputEnabled( true );
	SetKeyBoardInputEnabled( true );

	Activate();

	if ( !OnAPIResult( pResult, bIOFailure, "submit item update" ) )
	{
		return;
	}

	g_ReactiveDropWorkshop.m_szContentPath.Clear();
	m_szPreviewImage.Clear();
	g_ReactiveDropWorkshop.OpenWorkshopPageForFile( m_nEditingWorkshopID );
	m_bEditing = false;
	m_pGplWorkshopItems->SetMouseInputEnabled( true );
	RequestSingleItem( m_nEditingWorkshopID );
}
