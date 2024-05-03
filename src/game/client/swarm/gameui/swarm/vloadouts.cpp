#include "cbase.h"
#include "vloadouts.h"
#include "rd_loadout.h"
#include "nb_header_footer.h"
#include "nb_skill_panel.h"
#include "rd_vgui_main_menu_top_bar.h"
#include "filesystem.h"
#include "asw_util_shared.h"
#include "vgui/ILocalize.h"
#include "vgui/ISurface.h"
#include "vgui_controls/ImagePanel.h"
#include "vgui_controls/RichText.h"
#include "MultiFontRichText.h"
#include "vgenericwaitscreen.h"
#include "vgenericconfirmation.h"
#include "vpasswordentry.h"
#include "rd_workshop.h"
#include "asw_marine_profile.h"
#include "vgui_bitmapbutton.h"
#include "asw_medal_store.h"
#include "asw_player_shared.h"
#include "rd_collections.h"
#include "rd_swarmopedia.h"
#include "asw_weapon_parse.h"
#include "briefingtooltip.h"
#include "asw_weapon_shared.h"
#include "vgui_avatarimage.h"
#include "rd_workshop_preview_image.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;
using namespace BaseModUI;

extern ConVar rd_override_commander_promotion;
extern ConVar asw_default_primary[ASW_NUM_MARINES_PER_LOADOUT];
extern ConVar asw_default_secondary[ASW_NUM_MARINES_PER_LOADOUT];
extern ConVar asw_default_extra[ASW_NUM_MARINES_PER_LOADOUT];
extern ConVar rd_equipped_medal[RD_STEAM_INVENTORY_NUM_MEDAL_SLOTS];
extern ConVar asw_unlock_all_weapons;

ConVar rd_loadout_temp_animation( "rd_loadout_temp_animation", "run_aiming_all" );

const IBriefing_ItemInstance Loadouts::s_EmptyItemInstance;

static int GetWeaponIndex( ASW_Inventory_slot_t iSlot, int iWeapon, int iDefaultLoadoutIndex )
{
	if ( iWeapon == -1 )
	{
		switch ( iSlot )
		{
		case ASW_INVENTORY_SLOT_PRIMARY:
			return ReactiveDropLoadout::DefaultLoadout.Marines[iDefaultLoadoutIndex].Primary;
		case ASW_INVENTORY_SLOT_SECONDARY:
			return ReactiveDropLoadout::DefaultLoadout.Marines[iDefaultLoadoutIndex].Secondary;
		case ASW_INVENTORY_SLOT_EXTRA:
			return ReactiveDropLoadout::DefaultLoadout.Marines[iDefaultLoadoutIndex].Extra;
		}
	}

	return iWeapon;
}

Loadouts::Loadouts( Panel *parent, const char *panelName )
	: BaseClass( parent, panelName )
{
	SetProportional( true );

	m_pHeaderFooter = new CNB_Header_Footer( this, "HeaderFooter" );
	m_pHeaderFooter->SetTitle( "" );
	m_pHeaderFooter->SetHeaderEnabled( false );
	m_pHeaderFooter->SetFooterEnabled( false );
	m_pTopBar = new CRD_VGUI_Main_Menu_Top_Bar( this, "TopBar" );
	m_pTopBar->m_hActiveButton = m_pTopBar->m_pTopButton[CRD_VGUI_Main_Menu_Top_Bar::BTN_LOADOUTS];

	m_pGplSavedLoadouts = new GenericPanelList( this, "GplSavedLoadouts", GenericPanelList::ISM_PERITEM );
	m_pGplSavedLoadouts->SetScrollBarVisible( true );

	for ( int i = 0; i < RD_STEAM_INVENTORY_NUM_MEDAL_SLOTS; i++ )
	{
		m_pMedalSlot[i] = new CRD_VGUI_Loadout_Slot_Inventory( this, VarArgs( "MedalSlot%d", i ), &rd_equipped_medal[i], ReactiveDropInventory::g_PlayerInventorySlotNames[RD_STEAM_INVENTORY_EQUIP_SLOT_FIRST_MEDAL + i] );
		m_pLblMedal[i] = new vgui::Label( this, VarArgs( "LblMedal%d", i ), "" );
	}

	m_pImgPromotionIcon = new vgui::ImagePanel( this, "ImgPromotionIcon" );
	m_pLblPromotion = new vgui::Label( this, "LblPromotion", "" );

	for ( int i = 0; i < ASW_NUM_MARINES_PER_LOADOUT; i++ )
	{
		m_pLblMarineName[i] = new vgui::Label( this, VarArgs( "LblMarine%d", i ), "" );
		m_pBtnMarineLoadout[i] = new CBitmapButton( this, VarArgs( "BtnMarine%d", i ), "" );
		m_pBtnMarineLoadout[i]->SetConsoleStylePanel( true );
		m_pBtnMarineLoadout[i]->SetFocusOnNavigateTo( true );
	}

	m_pLblMarines = new vgui::Label( this, "LblMarines", "#rd_loadout_equipped_marines" );
	m_pLblMedals = new vgui::Label( this, "LblMedals", "#rd_loadout_equipped_medals" );

	m_pBtnCreateLoadout = new BaseModHybridButton( this, "BtnCreateLoadout", "#rd_loadout_new", this, "CreateNewLoadout" );
	m_pBtnDeleteLoadout = new BaseModHybridButton( this, "BtnDeleteLoadout", "#rd_loadout_delete", this, "DeleteLoadout" );
	m_pBtnCopyFromLive = new BaseModHybridButton( this, "BtnCopyFromLive", "#rd_loadout_save", this, "CopyFromLive" );
	m_pBtnCopyToLive = new BaseModHybridButton( this, "BtnCopyToLive", "#rd_loadout_load", this, "CopyToLive" );
	m_pBtnViewOnWorkshop = new BaseModHybridButton( this, "BtnViewOnWorkshop", "#rd_loadout_view_on_workshop", this, "ViewOnWorkshop" );
	m_pBtnBrowseWorkshop = new BaseModHybridButton( this, "BtnBrowseWorkshop", "#rd_loadout_browse_workshop", this, "FindLoadoutsOnWorkshop" );
	m_pBtnShare = new BaseModHybridButton( this, "BtnShare", "#rd_loadout_share_start", this, "ShareLoadoutsOnWorkshop" );

	SetTitle( "", false );
	SetDeleteSelfOnClose( true );
	SetLowerGarnishEnabled( false );
	SetMoveable( false );
}

Loadouts::~Loadouts()
{
}

void Loadouts::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	constexpr const char *s_szClassPanelNames[4][2] =
	{
		{ "ImgMarineClassOfficer", "LblMarineClassOfficer" },
		{ "ImgMarineClassSpecialWeapons", "LblMarineClassSpecialWeapons" },
		{ "ImgMarineClassMedic", "LblMarineClassMedic" },
		{ "ImgMarineClassTech", "LblMarineClassTech" },
	};

	for ( int i = 0; i < NELEMS( s_szClassPanelNames ); i++ )
	{
		vgui::Panel *pClassImage = FindChildByName( s_szClassPanelNames[i][0] );
		vgui::Label *pClassLabel = dynamic_cast< vgui::Label * >( FindChildByName( s_szClassPanelNames[i][1] ) );
		if ( pClassImage && pClassLabel )
		{
			int w, t;
			pClassLabel->GetContentSize( w, t );

			int iOffset = ( pClassLabel->GetWide() - w ) / 2;

			int x, y;
			pClassImage->GetPos( x, y );
			x += iOffset;
			pClassImage->SetPos( x, y );
		}
	}

	int iPromotion = rd_override_commander_promotion.GetInt();
	if ( iPromotion < 0 )
	{
		C_ASW_Medal_Store *pMedals = GetMedalStore();
		if ( pMedals )
		{
			iPromotion = pMedals->GetPromotion();
		}
	}

	for ( int i = 0; i < 4; i++ )
	{
		m_pBtnMarineLoadout[i]->SetNavUp( m_pTopBar );
	}

	if ( iPromotion <= 0 || iPromotion > ASW_PROMOTION_CAP )
	{
		m_pLblPromotion->SetVisible( false );
		m_pImgPromotionIcon->SetVisible( false );
	}
	else
	{
		m_pLblPromotion->SetVisible( true );
		m_pLblPromotion->SetText( VarArgs( "#nb_promotion_medal_%d", iPromotion ) );
		m_pImgPromotionIcon->SetVisible( true );
		m_pImgPromotionIcon->SetImage( VarArgs( "briefing/promotion_%d_LG", iPromotion ) );
	}

	for ( int i = 0; i < RD_STEAM_INVENTORY_NUM_MEDAL_SLOTS; i++ )
	{
		const ReactiveDropInventory::ItemInstance_t *pInstance = m_pMedalSlot[i]->GetItem();
		const ReactiveDropInventory::ItemDef_t *pDef = pInstance ? ReactiveDropInventory::GetItemDef( pInstance->ItemDefID ) : NULL;
		Assert( pDef || !pInstance );
		if ( pDef )
		{
			wchar_t wszMedalName[2048];
			V_UTF8ToUnicode( pDef->BriefingName, wszMedalName, sizeof( wszMedalName ) );
			m_pLblMedal[i]->SetText( wszMedalName );
			m_pLblMedal[i]->SetVisible( true );
		}
		else
		{
			m_pLblMedal[i]->SetVisible( false );
		}
	}

	m_pTopBar->SetNavUp( m_pTopBar->m_pTopButton[CRD_VGUI_Main_Menu_Top_Bar::BTN_LOADOUTS] );
}

void Loadouts::OnOpen()
{
	BaseClass::OnOpen();

	MakeReadyForUse();

	OnLoadoutsUpdated();
}

static void CreateLoadoutHelper()
{
	PasswordEntry *pEntry = assert_cast< PasswordEntry * >( CBaseModPanel::GetSingleton().GetWindow( WT_PASSWORDENTRY ) );
	Assert( pEntry );
	Loadouts *pLoadouts = assert_cast< Loadouts * >( CBaseModPanel::GetSingleton().GetWindow( WT_LOADOUTS ) );
	Assert( pLoadouts );
	if ( !pEntry || !pLoadouts )
		return;

	char szName[MAX_VALUE];
	pEntry->GetPassword( szName, sizeof( szName ) );
	if ( ReactiveDropLoadout::Loadouts.IsValidIndex( ReactiveDropLoadout::Loadouts.Find( szName ) ) )
	{
		PasswordEntry::Data_t data;

		data.pWindowTitle = "#rd_loadout_new";
		data.pMessageText = "#rd_loadout_name_in_use";
		data.m_szCurrentPW.Set( szName );
		data.bOkButtonEnabled = true;
		data.bCancelButtonEnabled = true;
		data.bShowPassword = true;
		data.pfnOkCallback = &CreateLoadoutHelper;

		pEntry->SetUsageData( data );

		return;
	}

	pEntry->Close();

	ReactiveDropLoadout::Loadouts.Insert( szName, ReactiveDropLoadout::LoadoutData_t{} );
	ReactiveDropLoadout::WriteLoadouts();
	pLoadouts->InitLoadoutList();

	int nCount = pLoadouts->m_pGplSavedLoadouts->GetPanelItemCount();
	for ( int i = 0; i < nCount; i++ )
	{
		CRD_VGUI_Loadout_List_Item *pLoadout = dynamic_cast< CRD_VGUI_Loadout_List_Item * >( pLoadouts->m_pGplSavedLoadouts->GetPanelItem( i ) );
		if ( !pLoadout )
			continue;

		if ( pLoadout->m_bCurrentLoadout || pLoadout->m_bDefaultLoadout || pLoadout->m_iAddonID != k_PublishedFileIdInvalid )
			continue;

		if ( V_strcmp( pLoadout->m_szName, szName ) )
			continue;

		pLoadouts->m_pGplSavedLoadouts->SelectPanelItem( i );
		pLoadouts->ShowLoadoutItem( pLoadout );
		return;
	}

	Assert( !"didn't find new loadout in list" );
}

static void DeleteLoadoutConfirmHelper()
{
	Loadouts *pLoadouts = assert_cast< Loadouts * >( CBaseModPanel::GetSingleton().GetWindow( WT_LOADOUTS ) );
	Assert( pLoadouts );
	if ( pLoadouts )
	{
		pLoadouts->OnCommand( "DeleteLoadoutNoConfirm" );
	}
}

void Loadouts::OnCommand( const char *command )
{
	CRD_VGUI_Loadout_List_Item *pLoadout = assert_cast< CRD_VGUI_Loadout_List_Item * >( m_pGplSavedLoadouts->GetSelectedPanelItem() );
	Assert( pLoadout );
	if ( const char *szMarineNumber = StringAfterPrefix( command, "MarineLoadout" ) )
	{
		int iSlot = V_atoi( szMarineNumber );
		if ( !pLoadout->m_Loadout.MarineIncluded[iSlot] )
		{
			if ( pLoadout->m_iAddonID != k_PublishedFileIdInvalid )
			{
				return;
			}

			// TODO: this should be an item selection screen; same for marine button in the screen we're about to open
			pLoadout->SetMarineForSlot( iSlot, k_SteamItemInstanceIDInvalid );
		}

		m_hSubScreen = new CRD_VGUI_Loadout_Marine( this, "LoadoutMarine", iSlot, pLoadout );
		m_hSubScreen->MakeReadyForUse();
		NavigateToChild( m_hSubScreen );
	}
	else if ( !V_stricmp( command, "CreateNewLoadout" ) )
	{
		PasswordEntry *pEntry = assert_cast< PasswordEntry * >( CBaseModPanel::GetSingleton().OpenWindow( WT_PASSWORDENTRY, this, false ) );

		PasswordEntry::Data_t data;

		data.pWindowTitle = "#rd_loadout_new";
		data.pMessageText = "#rd_loadout_name_initial";
		data.bOkButtonEnabled = true;
		data.bCancelButtonEnabled = true;
		data.bShowPassword = true;
		data.pfnOkCallback = &CreateLoadoutHelper;

		pEntry->SetUsageData( data );
	}
	else if ( !V_stricmp( command, "DeleteLoadout" ) )
	{
		Assert( !pLoadout->IsLoadoutReadOnly() );
		Assert( !pLoadout->m_bCurrentLoadout );

		GenericConfirmation *pConfirm = assert_cast< GenericConfirmation * >( CBaseModPanel::GetSingleton().OpenWindow( WT_GENERICCONFIRMATION, this, false ) );

		GenericConfirmation::Data_t data;

		wchar_t wszLoadoutTitle[MAX_VALUE];
		V_UTF8ToUnicode( pLoadout->m_szName, wszLoadoutTitle, sizeof( wszLoadoutTitle ) );
		wchar_t wszWindowTitle[1024];
		g_pVGuiLocalize->ConstructString( wszWindowTitle, sizeof( wszWindowTitle ), g_pVGuiLocalize->Find( "#rd_loadout_delete_title" ), 1, wszLoadoutTitle );
		char szWindowTitle[1024];
		V_UnicodeToUTF8( wszWindowTitle, szWindowTitle, sizeof( szWindowTitle ) );

		data.pWindowTitle = szWindowTitle;
		data.pMessageText = "#rd_loadout_delete_desc";
		data.bOkButtonEnabled = true;
		data.bCancelButtonEnabled = true;
		data.pfnOkCallback = &DeleteLoadoutConfirmHelper;

		pConfirm->SetUsageData( data );
	}
	else if ( !V_stricmp( command, "DeleteLoadoutNoConfirm" ) )
	{
		Assert( !pLoadout->IsLoadoutReadOnly() );
		Assert( !pLoadout->m_bCurrentLoadout );

		ReactiveDropLoadout::Delete( pLoadout->m_szName );
		OnLoadoutsUpdated();
	}
	else if ( !V_stricmp( command, "CopyToLive" ) )
	{
		Assert( !pLoadout->m_bCurrentLoadout );

		pLoadout->m_Loadout.CopyToLive();
		OnLoadoutsUpdated();
	}
	else if ( !V_stricmp( command, "CopyFromLive" ) )
	{
		Assert( !pLoadout->IsLoadoutReadOnly() );
		Assert( !pLoadout->m_bCurrentLoadout );

		ReactiveDropLoadout::Save( pLoadout->m_szName );
		OnLoadoutsUpdated();
	}
	else if ( !V_stricmp( command, "ViewOnWorkshop" ) )
	{
		Assert( pLoadout->m_iAddonID != k_PublishedFileIdInvalid );

		char szURL[255];
		V_snprintf( szURL, sizeof( szURL ), "https://steamcommunity.com/sharedfiles/filedetails/?id=%llu", pLoadout->m_iAddonID );
		CUIGameData::Get()->ExecuteOverlayUrl( szURL );
	}
	else if ( !V_stricmp( command, "FindLoadoutsOnWorkshop" ) )
	{
		CUIGameData::Get()->ExecuteOverlayUrl( "https://steamcommunity.com/app/563560/guides/?browsefilter=trend&requiredtags[]=Loadouts&filetype=12", true );
	}
	else if ( !V_stricmp( command, "ShareLoadoutsOnWorkshop" ) )
	{
		m_hSubScreen = new CRD_VGUI_Loadout_Share( this, "ShareLoadouts" );
		m_hSubScreen->MakeReadyForUse();
		NavigateToChild( m_hSubScreen );
	}
	else
	{
		BaseClass::OnCommand( command );
	}
}

void Loadouts::OnKeyCodeTyped( vgui::KeyCode code )
{
	if ( code == KEY_ESCAPE && m_hSubScreen )
	{
		m_hSubScreen->MarkForDeletion();

		return;
	}

	BaseClass::OnKeyCodeTyped( code );
}

void Loadouts::OnLoadoutsUpdated()
{
	bool bHadPreviousLoadout = false;
	bool bPreviousLoadoutWasCurrent = false;
	bool bPreviousLoadoutWasDefault = false;
	PublishedFileId_t iPreviousLoadoutAddon = k_PublishedFileIdInvalid;
	char szLoadoutName[MAX_VALUE]{};
	CRD_VGUI_Loadout_List_Item *pPreviousSelection = assert_cast< CRD_VGUI_Loadout_List_Item * >( m_pGplSavedLoadouts->GetSelectedPanelItem() );
	if ( pPreviousSelection )
	{
		bHadPreviousLoadout = true;
		bPreviousLoadoutWasCurrent = pPreviousSelection->m_bCurrentLoadout;
		bPreviousLoadoutWasDefault = pPreviousSelection->m_bDefaultLoadout;
		iPreviousLoadoutAddon = pPreviousSelection->m_iAddonID;
		V_strncpy( szLoadoutName, pPreviousSelection->m_szName, sizeof( szLoadoutName ) );
	}

	InitLoadoutList();

	if ( bHadPreviousLoadout )
	{
		int nCount = m_pGplSavedLoadouts->GetPanelItemCount();
		for ( int i = 0; i < nCount; i++ )
		{
			CRD_VGUI_Loadout_List_Item *pLoadout = dynamic_cast< CRD_VGUI_Loadout_List_Item * >( m_pGplSavedLoadouts->GetPanelItem( i ) );
			if ( !pLoadout )
				continue;

			if ( bPreviousLoadoutWasCurrent && pLoadout->m_bCurrentLoadout )
			{
				m_pGplSavedLoadouts->SelectPanelItem( i );
				break;
			}

			if ( bPreviousLoadoutWasDefault && pLoadout->m_bDefaultLoadout )
			{
				m_pGplSavedLoadouts->SelectPanelItem( i );
				break;
			}

			if ( iPreviousLoadoutAddon == pLoadout->m_iAddonID && !V_strcmp( szLoadoutName, pLoadout->m_szName ) )
			{
				m_pGplSavedLoadouts->SelectPanelItem( i );
				break;
			}
		}
	}
}

void Loadouts::InitLoadoutList()
{
	ReactiveDropLoadout::InitLoadouts();

	m_pGplSavedLoadouts->RemoveAllPanelItems();

	char szLoadoutName[128];
	V_UnicodeToUTF8( g_pVGuiLocalize->Find( "#rd_loadout_current" ), szLoadoutName, sizeof( szLoadoutName ) );
	ReactiveDropLoadout::LoadoutData_t CurrentLoadout;
	CurrentLoadout.CopyFromLive();
	CRD_VGUI_Loadout_List_Item *pCurrentLoadout = new CRD_VGUI_Loadout_List_Item( this, "LoadoutListItem", szLoadoutName, CurrentLoadout );
	pCurrentLoadout->m_bCurrentLoadout = true;
	m_pGplSavedLoadouts->AddPanelItem( pCurrentLoadout, true );

	V_UnicodeToUTF8( g_pVGuiLocalize->Find( "#rd_loadout_default" ), szLoadoutName, sizeof( szLoadoutName ) );
	CRD_VGUI_Loadout_List_Item *pDefaultLoadout = new CRD_VGUI_Loadout_List_Item( this, "LoadoutListItem", szLoadoutName, ReactiveDropLoadout::DefaultLoadout );
	pDefaultLoadout->m_bDefaultLoadout = true;
	m_pGplSavedLoadouts->AddPanelItem( pDefaultLoadout, true );

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

	m_pGplSavedLoadouts->RelinkNavigation();
	m_pGplSavedLoadouts->GetPanelItem( 0 )->SetNavUp( m_pBtnCreateLoadout );
	m_pGplSavedLoadouts->GetPanelItem( m_pGplSavedLoadouts->GetPanelItemCount() - 1 )->SetNavDown( m_pBtnBrowseWorkshop );
	m_pGplSavedLoadouts->SelectPanelItem( 0 );
}

void Loadouts::ShowLoadoutItem( CRD_VGUI_Loadout_List_Item *pLoadout )
{
	for ( int i = 0; i < ASW_NUM_MARINES_PER_LOADOUT; i++ )
	{
		if ( pLoadout->m_Loadout.MarineIncluded[i] )
		{
			int iProfile = i;
			if ( pLoadout->m_Loadout.Marines[i].SuitDef != 0 )
			{
				const ReactiveDropInventory::ItemDef_t *pDef = ReactiveDropInventory::GetItemDef( pLoadout->m_Loadout.Marines[i].SuitDef );
				Assert( pDef );
				if ( pDef )
				{
					Assert( pDef->EquipIndex >= 0 && pDef->EquipIndex < ASW_NUM_MARINE_PROFILES );
					if ( pDef->EquipIndex >= 0 && pDef->EquipIndex < ASW_NUM_MARINE_PROFILES )
					{
						iProfile = pDef->EquipIndex;
					}
				}
			}

			CASW_Marine_Profile *pProfile = MarineProfileList()->GetProfile( iProfile );
			Assert( pProfile );
			if ( !pProfile )
				continue;

			m_pBtnMarineLoadout[i]->SetImage( CBitmapButton::BUTTON_ENABLED, CFmtStr{ "vgui/briefing/face_%s", pProfile->m_PortraitName }, color32{ 255, 255, 255, 255 } );
			m_pBtnMarineLoadout[i]->SetImage( CBitmapButton::BUTTON_ENABLED_MOUSE_OVER, CFmtStr{ "vgui/briefing/face_%s_lit", pProfile->m_PortraitName }, color32{ 255, 255, 255, 255 } );
			m_pBtnMarineLoadout[i]->SetImage( CBitmapButton::BUTTON_PRESSED, CFmtStr{ "vgui/briefing/face_%s_lit", pProfile->m_PortraitName }, color32{ 255, 255, 255, 255 } );
			m_pLblMarineName[i]->SetText( pProfile->GetShortName() );
			m_pLblMarineName[i]->SetVisible( true );
		}
		else
		{
			m_pBtnMarineLoadout[i]->SetImage( CBitmapButton::BUTTON_ENABLED, "vgui/briefing/face_empty", color32{ 255, 255, 255, 255 } );
			m_pBtnMarineLoadout[i]->SetImage( CBitmapButton::BUTTON_ENABLED_MOUSE_OVER, "vgui/briefing/face_empty_lit", color32{ 255, 255, 255, 255 } );
			m_pBtnMarineLoadout[i]->SetImage( CBitmapButton::BUTTON_PRESSED, "vgui/briefing/face_empty_lit", color32{ 255, 255, 255, 255 } );
			m_pLblMarineName[i]->SetVisible( false );
		}
	}

	m_pLblMedals->SetVisible( pLoadout->m_bCurrentLoadout );
	for ( int i = 0; i < RD_STEAM_INVENTORY_NUM_MEDAL_SLOTS; i++ )
	{
		m_pMedalSlot[i]->SetVisible( pLoadout->m_bCurrentLoadout );
		m_pLblMedal[i]->SetVisible( pLoadout->m_bCurrentLoadout );
	}
	m_pImgPromotionIcon->SetVisible( pLoadout->m_bCurrentLoadout );
	m_pLblPromotion->SetVisible( pLoadout->m_bCurrentLoadout );

	m_pBtnDeleteLoadout->SetVisible( pLoadout->m_iAddonID == k_PublishedFileIdInvalid && !pLoadout->m_bCurrentLoadout );
	m_pBtnDeleteLoadout->SetEnabled( !pLoadout->m_bDefaultLoadout );
	m_pBtnViewOnWorkshop->SetVisible( pLoadout->m_iAddonID != k_PublishedFileIdInvalid );
	m_pBtnCopyFromLive->SetVisible( !pLoadout->m_bCurrentLoadout );
	m_pBtnCopyFromLive->SetEnabled( !pLoadout->IsLoadoutReadOnly() );
	m_pBtnCopyToLive->SetVisible( !pLoadout->m_bCurrentLoadout );
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

CASW_Marine_Profile *Loadouts::GetMarineProfile( int nLobbySlot )
{
	CRD_VGUI_Loadout_List_Item *pLoadout = assert_cast< CRD_VGUI_Loadout_List_Item * >( m_pGplSavedLoadouts->GetSelectedPanelItem() );
	Assert( pLoadout );
	if ( !pLoadout )
		return NULL;

	return GetMarineProfileByProfileIndex( pLoadout->GetMarineProfileForSlot( nLobbySlot ) );
}

CASW_Marine_Profile *Loadouts::GetMarineProfileByProfileIndex( int nProfileIndex )
{
	return MarineProfileList()->GetProfile( nProfileIndex );
}

int Loadouts::GetProfileSelectedWeapon( int nProfileIndex, int nWeaponSlot )
{
	CRD_VGUI_Loadout_List_Item *pLoadout = assert_cast< CRD_VGUI_Loadout_List_Item * >( m_pGplSavedLoadouts->GetSelectedPanelItem() );
	Assert( pLoadout );
	if ( !pLoadout )
		return -1;

	int nLobbySlot = pLoadout->GetSlotByMarineProfile( ASW_Marine_Profile( nProfileIndex ) );
	Assert( nLobbySlot != -1 );
	if ( nLobbySlot == -1 )
		return -1;

	return GetMarineSelectedWeapon( nLobbySlot, nWeaponSlot );
}

int Loadouts::GetMarineSelectedWeapon( int nLobbySlot, int nWeaponSlot )
{
	CRD_VGUI_Loadout_List_Item *pLoadout = assert_cast< CRD_VGUI_Loadout_List_Item * >( m_pGplSavedLoadouts->GetSelectedPanelItem() );
	Assert( pLoadout );
	if ( !pLoadout )
		return -1;

	return pLoadout->GetWeaponForSlot( nLobbySlot, ASW_Inventory_slot_t( nWeaponSlot ) );
}

const char *Loadouts::GetMarineWeaponClass( int nLobbySlot, int nWeaponSlot )
{
	if ( nWeaponSlot == ASW_INVENTORY_SLOT_EXTRA )
		return g_ASWEquipmentList.GetExtra( GetMarineSelectedWeapon( nLobbySlot, nWeaponSlot ) )->m_szEquipClass;

	return g_ASWEquipmentList.GetRegular( GetMarineSelectedWeapon( nLobbySlot, nWeaponSlot ) )->m_szEquipClass;
}

int Loadouts::GetMarineSkillPoints( int nLobbySlot, int nSkillSlot )
{
	return GetMarineProfile( nLobbySlot )->GetStaticSkillPoints( nSkillSlot );
}

int Loadouts::GetProfileSkillPoints( int nProfileIndex, int nSkillSlot )
{
	return GetMarineProfileByProfileIndex( nProfileIndex )->GetStaticSkillPoints( nSkillSlot );
}

bool Loadouts::IsWeaponUnlocked( const char *szWeaponClass )
{
	if ( asw_unlock_all_weapons.GetBool() )
		return true;

	return UTIL_ASW_CommanderLevelAtLeast( NULL, GetWeaponLevelRequirement( szWeaponClass ), -1 );
}

void Loadouts::SelectWeapon( int nLobbySlot, int nInventorySlot, int nEquipIndex, SteamItemInstanceID_t iItemInstance )
{
	CRD_VGUI_Loadout_List_Item *pLoadout = assert_cast< CRD_VGUI_Loadout_List_Item * >( m_pGplSavedLoadouts->GetSelectedPanelItem() );
	Assert( pLoadout );
	if ( !pLoadout )
		return;

	pLoadout->SetWeaponForSlot( nLobbySlot, ASW_Inventory_slot_t( nInventorySlot ), nEquipIndex, iItemInstance );

	if ( m_hSubScreen )
	{
		m_hSubScreen->InvalidateLayout( true, true );

		if ( vgui::Panel *pSlot = m_hSubScreen->FindChildByName( VarArgs( "WeaponSlot%d", nInventorySlot ) ) )
		{
			pSlot->RequestFocus();
		}
	}
}

CRD_VGUI_Loadout_List_Item::CRD_VGUI_Loadout_List_Item( vgui::Panel *parent, const char *panelName, const char *szName, const ReactiveDropLoadout::LoadoutData_t &loadout, PublishedFileId_t id ) :
	BaseClass( parent, panelName )
{
	SetConsoleStylePanel( true );

	V_strncpy( m_szName, szName, sizeof( m_szName ) );
	m_Loadout = loadout;
	m_iAddonID = id;

	wchar_t wszName[MAX_VALUE];
	V_UTF8ToUnicode( szName, wszName, sizeof( wszName ) );

	m_pLblTitle = new vgui::Label( this, "LblTitle", wszName );
	m_bSelected = false;
	m_bCurrentLoadout = false;
	m_bDefaultLoadout = false;
}

void CRD_VGUI_Loadout_List_Item::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	LoadControlSettings( "resource/ui/CRD_VGUI_Loadout_List_Item.res" );
}

void CRD_VGUI_Loadout_List_Item::ApplySettings( KeyValues *pSettings )
{
	BaseClass::ApplySettings( pSettings );

	SetTall( GetTall() + m_iRowHeight * ( m_Loadout.NumMarinesIncluded() + m_iNumColumns - 1 ) / m_iNumColumns );
}

void CRD_VGUI_Loadout_List_Item::Paint()
{
	BaseClass::Paint();

	Color color{ 255, 255, 255, 255 };
	if ( !m_bSelected )
		color = Color{ 192, 192, 192, 255 };

	int wide, tall;
	GetSize( wide, tall );
	int y = tall - m_iRowHeight * ( m_Loadout.NumMarinesIncluded() + m_iNumColumns - 1 ) / m_iNumColumns - YRES( 2 );

	if ( m_bSelected )
	{
		vgui::IScheme *pScheme = vgui::scheme()->GetIScheme( GetScheme() );
		Color blotchColor = pScheme->GetColor( "HybridButton.BlotchColor", Color( 0, 0, 0, 255 ) );
		Color borderColor = pScheme->GetColor( "HybridButton.BorderColor", Color( 0, 0, 0, 255 ) );

		if ( HasFocus() )
		{
			borderColor = Color{ 255, 255, 255, 255 };
		}

		int blotchWide = wide - YRES( 40 );
		vgui::surface()->DrawSetColor( blotchColor );
		vgui::surface()->DrawFilledRectFade( 0, y - YRES( 2 ), blotchWide / 4, tall, 0, 150, true );
		vgui::surface()->DrawFilledRectFade( blotchWide / 4, y - YRES( 2 ), blotchWide, tall, 150, 0, true );

		vgui::surface()->DrawSetColor( borderColor );
		vgui::surface()->DrawFilledRectFade( 0, y - 2 - YRES( 2 ), wide / 2, y - YRES( 2 ), 0, 255, true );
		vgui::surface()->DrawFilledRectFade( wide / 2, y - 2 - YRES( 2 ), wide, y - YRES( 2 ), 255, 0, true );
		vgui::surface()->DrawFilledRectFade( 0, tall - 2, wide / 2, tall, 0, 255, true );
		vgui::surface()->DrawFilledRectFade( wide / 2, tall - 2, wide, tall, 255, 0, true );
	}

	int col = 0;
	for ( int i = 0; i < ASW_NUM_MARINES_PER_LOADOUT; i++ )
	{
		if ( !m_Loadout.MarineIncluded[i] )
			continue;

		ASW_Marine_Profile iProfile = ASW_Marine_Profile( i );
		if ( m_Loadout.Marines[i].SuitDef != 0 )
		{
			const ReactiveDropInventory::ItemDef_t *pSuitDef = ReactiveDropInventory::GetItemDef( m_Loadout.Marines[i].SuitDef );
			Assert( pSuitDef );
			if ( pSuitDef )
				iProfile = ASW_Marine_Profile( pSuitDef->EquipIndex );
		}

		CASW_Marine_Profile *pProfile = MarineProfileList()->GetProfile( iProfile );
		Assert( pProfile );
		if ( pProfile )
		{
			int x = ( ( GetWide() - m_iRowHeight * 6 * m_iNumColumns ) / ( m_iNumColumns + 1 ) + m_iRowHeight * 6 ) * ( col + 1 ) - m_iRowHeight * 6;
			vgui::surface()->DrawSetColor( color );
			vgui::surface()->DrawSetTexture( pProfile->m_nPortraitTextureID );
			vgui::surface()->DrawTexturedRect( x, y, x + m_iRowHeight, y + m_iRowHeight );

			x += m_iRowHeight;
			vgui::surface()->DrawSetTexture( g_ASWEquipmentList.GetEquipIconTexture( true, GetWeaponIndex( ASW_INVENTORY_SLOT_PRIMARY, m_Loadout.Marines[i].Primary, pProfile->m_iDefaultLoadoutIndex ) ) );
			vgui::surface()->DrawTexturedRect( x, y, x + m_iRowHeight * 2, y + m_iRowHeight );

			x += m_iRowHeight * 2;
			vgui::surface()->DrawSetTexture( g_ASWEquipmentList.GetEquipIconTexture( true, GetWeaponIndex( ASW_INVENTORY_SLOT_SECONDARY, m_Loadout.Marines[i].Secondary, pProfile->m_iDefaultLoadoutIndex ) ) );
			vgui::surface()->DrawTexturedRect( x, y, x + m_iRowHeight * 2, y + m_iRowHeight );

			x += m_iRowHeight * 2;
			vgui::surface()->DrawSetTexture( g_ASWEquipmentList.GetEquipIconTexture( false, GetWeaponIndex( ASW_INVENTORY_SLOT_EXTRA, m_Loadout.Marines[i].Extra, pProfile->m_iDefaultLoadoutIndex ) ) );
			vgui::surface()->DrawTexturedRect( x, y, x + m_iRowHeight, y + m_iRowHeight );
		}

		col++;
		if ( col >= m_iNumColumns )
		{
			col = 0;
			y += m_iRowHeight;
		}
	}

	if ( col != 0 )
	{
		col = 0;
		y += m_iRowHeight;
	}
}

void CRD_VGUI_Loadout_List_Item::NavigateTo()
{
	BaseClass::NavigateTo();

	RequestFocus();
}

void CRD_VGUI_Loadout_List_Item::OnSetFocus()
{
	BaseClass::OnSetFocus();

	GenericPanelList *pGPL = assert_cast< GenericPanelList * >( GetParent()->GetParent() );
	pGPL->SelectPanelItemByPanel( this );

	SetNavLeft( pGPL->GetNavLeft() );
	SetNavRight( pGPL->GetNavRight() );

	assert_cast< Loadouts * >( pGPL->GetParent() )->ShowLoadoutItem( this );
}

bool CRD_VGUI_Loadout_List_Item::IsLoadoutReadOnly()
{
	if ( m_iAddonID != k_PublishedFileIdInvalid )
		return true;

	if ( m_bDefaultLoadout )
		return true;

	return false;
}

int CRD_VGUI_Loadout_List_Item::GetSlotByMarineProfile( ASW_Marine_Profile iProfile )
{
	for ( int i = 0; i < ASW_NUM_MARINES_PER_LOADOUT; i++ )
	{
		if ( GetMarineProfileForSlot( iProfile ) == iProfile )
		{
			return i;
		}
	}

	return -1;
}

ASW_Marine_Profile CRD_VGUI_Loadout_List_Item::GetMarineProfileForSlot( int iSlot )
{
	SteamItemDef_t id = GetMarineItemDefID( iSlot );
	if ( id == 0 )
		return ASW_Marine_Profile( iSlot );

	const ReactiveDropInventory::ItemDef_t *pDef = ReactiveDropInventory::GetItemDef( id );
	Assert( pDef );
	if ( !pDef )
		return ASW_Marine_Profile( iSlot );

	return ASW_Marine_Profile( pDef->EquipIndex );
}

SteamItemDef_t CRD_VGUI_Loadout_List_Item::GetMarineItemDefID( int iSlot )
{
	Assert( iSlot >= 0 && iSlot < ASW_NUM_MARINES_PER_LOADOUT );

	return m_Loadout.Marines[iSlot].SuitDef;
}

SteamItemInstanceID_t CRD_VGUI_Loadout_List_Item::GetMarineItemID( int iSlot )
{
	Assert( iSlot >= 0 && iSlot < ASW_NUM_MARINES_PER_LOADOUT );

	return m_Loadout.Marines[iSlot].Suit;
}

int CRD_VGUI_Loadout_List_Item::GetWeaponForSlot( int iSlot, ASW_Inventory_slot_t iEquipSlot )
{
	Assert( iSlot >= 0 && iSlot < ASW_NUM_MARINES_PER_LOADOUT );
	Assert( iEquipSlot == ASW_INVENTORY_SLOT_PRIMARY || iEquipSlot == ASW_INVENTORY_SLOT_SECONDARY || iEquipSlot == ASW_INVENTORY_SLOT_EXTRA );

	CASW_Marine_Profile *pProfile = MarineProfileList()->GetProfile( GetMarineProfileForSlot( iSlot ) );
	Assert( pProfile );
	if ( !pProfile )
		return -1;

	if ( iEquipSlot == ASW_INVENTORY_SLOT_PRIMARY )
		return GetWeaponIndex( iEquipSlot, m_Loadout.Marines[iSlot].Primary, pProfile->m_iDefaultLoadoutIndex );
	if ( iEquipSlot == ASW_INVENTORY_SLOT_SECONDARY )
		return GetWeaponIndex( iEquipSlot, m_Loadout.Marines[iSlot].Secondary, pProfile->m_iDefaultLoadoutIndex );
	if ( iEquipSlot == ASW_INVENTORY_SLOT_EXTRA )
		return GetWeaponIndex( iEquipSlot, m_Loadout.Marines[iSlot].Extra, pProfile->m_iDefaultLoadoutIndex );

	return -1;
}

SteamItemDef_t CRD_VGUI_Loadout_List_Item::GetWeaponItemDefID( int iSlot, ASW_Inventory_slot_t iEquipSlot )
{
	Assert( iSlot >= 0 && iSlot < ASW_NUM_MARINES_PER_LOADOUT );
	Assert( iEquipSlot == ASW_INVENTORY_SLOT_PRIMARY || iEquipSlot == ASW_INVENTORY_SLOT_SECONDARY || iEquipSlot == ASW_INVENTORY_SLOT_EXTRA );

	if ( iEquipSlot == ASW_INVENTORY_SLOT_PRIMARY )
		return m_Loadout.Marines[iSlot].PrimaryDef;
	if ( iEquipSlot == ASW_INVENTORY_SLOT_SECONDARY )
		return m_Loadout.Marines[iSlot].SecondaryDef;
	if ( iEquipSlot == ASW_INVENTORY_SLOT_EXTRA )
		return m_Loadout.Marines[iSlot].ExtraDef;

	return 0;
}

SteamItemInstanceID_t CRD_VGUI_Loadout_List_Item::GetWeaponItemID( int iSlot, ASW_Inventory_slot_t iEquipSlot )
{
	Assert( iSlot >= 0 && iSlot < ASW_NUM_MARINES_PER_LOADOUT );
	Assert( iEquipSlot == ASW_INVENTORY_SLOT_PRIMARY || iEquipSlot == ASW_INVENTORY_SLOT_SECONDARY || iEquipSlot == ASW_INVENTORY_SLOT_EXTRA );

	if ( iEquipSlot == ASW_INVENTORY_SLOT_PRIMARY )
		return m_Loadout.Marines[iSlot].PrimaryItem;
	if ( iEquipSlot == ASW_INVENTORY_SLOT_SECONDARY )
		return m_Loadout.Marines[iSlot].SecondaryItem;
	if ( iEquipSlot == ASW_INVENTORY_SLOT_EXTRA )
		return m_Loadout.Marines[iSlot].ExtraItem;

	return k_SteamItemInstanceIDInvalid;
}

bool CRD_VGUI_Loadout_List_Item::SetMarineForSlot( int iSlot, SteamItemInstanceID_t iItemInstance )
{
	Assert( !IsLoadoutReadOnly() );
	Assert( iSlot >= 0 && iSlot < ASW_NUM_MARINES_PER_LOADOUT );

	ASW_Marine_Profile iProfile = ASW_Marine_Profile( iSlot );
	if ( iItemInstance == 0 )
		iItemInstance = k_SteamItemInstanceIDInvalid;

	SteamItemDef_t iItemDef = 0;
	if ( iItemInstance != k_SteamItemInstanceIDInvalid )
	{
		const ReactiveDropInventory::ItemInstance_t *pItem = ReactiveDropInventory::GetLocalItemCache( iItemInstance );
		Assert( pItem );
		if ( !pItem )
		{
			Warning( "No cache for marine suit item %llu!\n", iItemInstance );
			return false;
		}

		iItemDef = pItem->ItemDefID;

		const ReactiveDropInventory::ItemDef_t *pDef = ReactiveDropInventory::GetItemDef( pItem->ItemDefID );
		Assert( pDef );
		if ( !pDef )
		{
			Warning( "No def for marine suit item!\n" );
			return false;
		}

		char szSlot[16];
		V_snprintf( szSlot, sizeof( szSlot ), "marine%d", iSlot );
		if ( !pDef->ItemSlotMatches( szSlot ) )
		{
			Warning( "Tried to equip item %s in slot %s\n", pDef->Name.Get(), szSlot );
			return false;
		}

		iProfile = ASW_Marine_Profile( pDef->EquipIndex );
	}

	for ( int i = 0; i < ASW_NUM_MARINES_PER_LOADOUT; i++ )
	{
		if ( i == iSlot )
			continue;

		ASW_Marine_Profile iOtherProfile = GetMarineProfileForSlot( i );
		if ( iOtherProfile == iProfile )
		{
			CASW_Marine_Profile *pProfile = MarineProfileList()->GetProfile( iProfile );
			Assert( pProfile );
			Warning( "Cannot equip marine %s in loadout slot %d - already equipped in slot %d\n", pProfile ? pProfile->m_PortraitName : "(MISSING)", iSlot, i );
			return false;
		}
	}

	if ( m_bCurrentLoadout )
	{
		engine->ClientCmd_Unrestricted( "host_writeconfig\n" );

		ReactiveDropLoadout::LoadoutData_t CurrentLoadout;
		CurrentLoadout.CopyFromLive();
		m_Loadout = CurrentLoadout;

		return true;
	}

	int iLoadoutIndex = ReactiveDropLoadout::Loadouts.Find( m_szName );
	Assert( ReactiveDropLoadout::Loadouts.IsValidIndex( iLoadoutIndex ) );
	if ( ReactiveDropLoadout::Loadouts.IsValidIndex( iLoadoutIndex ) )
	{
		ReactiveDropLoadout::Loadouts[iLoadoutIndex].MarineIncluded[iSlot] = true;
		ReactiveDropLoadout::Loadouts[iLoadoutIndex].Marines[iSlot].Suit = iItemInstance;
		ReactiveDropLoadout::Loadouts[iLoadoutIndex].Marines[iSlot].SuitDef = iItemDef;
		if ( ISteamUtils *pUtils = SteamUtils() )
			ReactiveDropLoadout::Loadouts[iLoadoutIndex].LastModified = pUtils->GetServerRealTime();
		ReactiveDropLoadout::WriteLoadouts();

		m_Loadout = ReactiveDropLoadout::Loadouts[iLoadoutIndex];

		return true;
	}

	Warning( "Failed to equip marine in loadout: loadout \"%s\" not found!\n", m_szName );

	return false;
}

bool CRD_VGUI_Loadout_List_Item::SetWeaponForSlot( int iSlot, ASW_Inventory_slot_t iEquipSlot, int iEquipIndex, SteamItemInstanceID_t iItemInstance )
{
	Assert( !IsLoadoutReadOnly() );
	Assert( iSlot >= 0 && iSlot < ASW_NUM_MARINES_PER_LOADOUT );
	Assert( iEquipSlot == ASW_INVENTORY_SLOT_PRIMARY || iEquipSlot == ASW_INVENTORY_SLOT_SECONDARY || iEquipSlot == ASW_INVENTORY_SLOT_EXTRA );

	if ( iItemInstance == 0 )
		iItemInstance = k_SteamItemInstanceIDInvalid;

	SteamItemDef_t iItemDef = 0;
	if ( iItemInstance != k_SteamItemInstanceIDInvalid )
	{
		const ReactiveDropInventory::ItemInstance_t *pItem = ReactiveDropInventory::GetLocalItemCache( iItemInstance );
		Assert( pItem );
		if ( !pItem )
		{
			Warning( "No cache for weapon item %llu!\n", iItemInstance );
			return false;
		}

		iItemDef = pItem->ItemDefID;

		const ReactiveDropInventory::ItemDef_t *pDef = ReactiveDropInventory::GetItemDef( pItem->ItemDefID );
		Assert( pDef );
		if ( !pDef )
		{
			Warning( "No def for weapon item!\n" );
			return false;
		}

		if ( !pDef->ItemSlotMatches( iEquipSlot == ASW_INVENTORY_SLOT_EXTRA ? "extra" : "weapon" ) )
		{
			Warning( "Tried to equip item %s in slot %s\n", pDef->Name.Get(), iEquipSlot == ASW_INVENTORY_SLOT_EXTRA ? "extra" : "weapon" );
			return false;
		}

		if ( iEquipIndex != pDef->EquipIndex )
		{
			Warning( "Equip index for item %s (%d) does not match requested equip index (%d)\n", pDef->Name.Get(), pDef->EquipIndex, iEquipIndex );
			return false;
		}
	}

	if ( m_bCurrentLoadout )
	{
		if ( iEquipSlot == ASW_INVENTORY_SLOT_PRIMARY )
		{
			asw_default_primary[iSlot].SetValue( iEquipIndex );
		}
		else if ( iEquipSlot == ASW_INVENTORY_SLOT_SECONDARY )
		{
			asw_default_secondary[iSlot].SetValue( iEquipIndex );
		}
		else if ( iEquipSlot == ASW_INVENTORY_SLOT_EXTRA )
		{
			asw_default_extra[iSlot].SetValue( iEquipIndex );
		}
		engine->ClientCmd_Unrestricted( "host_writeconfig\n" );

		ReactiveDropLoadout::LoadoutData_t CurrentLoadout;
		CurrentLoadout.CopyFromLive();
		m_Loadout = CurrentLoadout;

		return true;
	}

	int iLoadoutIndex = ReactiveDropLoadout::Loadouts.Find( m_szName );
	Assert( ReactiveDropLoadout::Loadouts.IsValidIndex( iLoadoutIndex ) );
	if ( ReactiveDropLoadout::Loadouts.IsValidIndex( iLoadoutIndex ) )
	{
		Assert( ReactiveDropLoadout::Loadouts[iLoadoutIndex].MarineIncluded[iSlot] );
		if ( iEquipSlot == ASW_INVENTORY_SLOT_PRIMARY )
		{
			ReactiveDropLoadout::Loadouts[iLoadoutIndex].Marines[iSlot].Primary = ASW_Equip_Regular( iEquipIndex );
			ReactiveDropLoadout::Loadouts[iLoadoutIndex].Marines[iSlot].PrimaryItem = iItemInstance;
			ReactiveDropLoadout::Loadouts[iLoadoutIndex].Marines[iSlot].PrimaryDef = iItemDef;
		}
		else if ( iEquipSlot == ASW_INVENTORY_SLOT_SECONDARY )
		{
			ReactiveDropLoadout::Loadouts[iLoadoutIndex].Marines[iSlot].Secondary = ASW_Equip_Regular( iEquipIndex );
			ReactiveDropLoadout::Loadouts[iLoadoutIndex].Marines[iSlot].SecondaryItem = iItemInstance;
			ReactiveDropLoadout::Loadouts[iLoadoutIndex].Marines[iSlot].SecondaryDef = iItemDef;
		}
		else if ( iEquipSlot == ASW_INVENTORY_SLOT_EXTRA )
		{
			ReactiveDropLoadout::Loadouts[iLoadoutIndex].Marines[iSlot].Extra = ASW_Equip_Extra( iEquipIndex );
			ReactiveDropLoadout::Loadouts[iLoadoutIndex].Marines[iSlot].ExtraItem = iItemInstance;
			ReactiveDropLoadout::Loadouts[iLoadoutIndex].Marines[iSlot].ExtraDef = iItemDef;
		}
		if ( ISteamUtils *pUtils = SteamUtils() )
			ReactiveDropLoadout::Loadouts[iLoadoutIndex].LastModified = pUtils->GetServerRealTime();
		ReactiveDropLoadout::WriteLoadouts();

		m_Loadout = ReactiveDropLoadout::Loadouts[iLoadoutIndex];

		return true;
	}

	Warning( "Failed to equip marine in loadout: loadout \"%s\" not found!\n", m_szName );

	return false;
}

void CRD_VGUI_Loadout_List_Item::OnPanelSelected()
{
	m_bSelected = true;
}

void CRD_VGUI_Loadout_List_Item::OnPanelUnSelected()
{
	m_bSelected = false;
}

CRD_VGUI_Loadout_List_Addon_Header::CRD_VGUI_Loadout_List_Addon_Header( vgui::Panel *parent, const char *panelName, PublishedFileId_t id ) :
	BaseClass( parent, panelName )
{
	m_iAddonID = id;

	m_pImgWorkshopIcon = new vgui::ImagePanel( this, "ImgWorkshopIcon" );
	m_pLblTitle = new vgui::Label( this, "LblTitle", VarArgs( "%llu", id ) );
	m_pImgAuthorAvatar = new CAvatarImagePanel( this, "ImgAuthorAvatar" );
	m_pLblAuthorName = new vgui::Label( this, "LblAuthorName", "" );

	m_bLoadedAddonDetails = false;
}

void CRD_VGUI_Loadout_List_Addon_Header::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	LoadControlSettings( "resource/ui/CRD_VGUI_Loadout_List_Addon_Header.res" );
}

void CRD_VGUI_Loadout_List_Addon_Header::OnThink()
{
	BaseClass::OnThink();

	if ( m_bLoadedAddonDetails )
		return;

	ISteamFriends *pFriends = SteamFriends();
	Assert( pFriends );
	if ( !pFriends )
		return;

	if ( const CReactiveDropWorkshop::WorkshopItem_t &item = g_ReactiveDropWorkshop.TryQueryAddon( m_iAddonID ) )
	{
		if ( vgui::IImage *pImage = const_cast<CReactiveDropWorkshopPreviewImage *>( item.pPreviewImage.GetObject() ) )
		{
			m_pImgWorkshopIcon->SetImage( pImage );
			m_bLoadedAddonDetails = true;
		}

		wchar_t wszTitle[k_cchPublishedDocumentTitleMax];
		V_UTF8ToUnicode( item.details.m_rgchTitle, wszTitle, sizeof( wszTitle ) );

		m_pLblTitle->SetText( wszTitle );

		CSteamID authorID( item.details.m_ulSteamIDOwner );
		m_pImgAuthorAvatar->SetAvatarBySteamID( &authorID );

		wchar_t wszPersonaName[k_cwchPersonaNameMax];
		V_UTF8ToUnicode( pFriends->GetFriendPersonaName( authorID ), wszPersonaName, sizeof( wszPersonaName ) );
		m_pLblAuthorName->SetText( wszPersonaName );
	}
}

void CRD_VGUI_Loadout_List_Addon_Header::OnPersonaStateChange( PersonaStateChange_t *pParam )
{
	ISteamFriends *pFriends = SteamFriends();
	Assert( pFriends );
	if ( !pFriends )
		return;

	if ( !m_bLoadedAddonDetails )
		return;

	if ( g_ReactiveDropWorkshop.TryQueryAddon( m_iAddonID ).details.m_ulSteamIDOwner != pParam->m_ulSteamID )
		return;

	wchar_t wszPersonaName[k_cwchPersonaNameMax];
	V_UTF8ToUnicode( pFriends->GetFriendPersonaName( pParam->m_ulSteamID ), wszPersonaName, sizeof( wszPersonaName ) );
	m_pLblAuthorName->SetText( wszPersonaName );
}

CRD_VGUI_Loadout_Marine::CRD_VGUI_Loadout_Marine( vgui::Panel *parent, const char *panelName, int iSlot, CRD_VGUI_Loadout_List_Item *pLoadout ) :
	BaseClass( parent, panelName )
{
	UTIL_RD_LoadKeyValuesFromFile( m_pKVDisplay, g_pFullFileSystem, "resource/swarmopedia_loadout_display.txt", "GAME" );

	m_hLoadout = pLoadout;
	m_pModelPanel = new CRD_Swarmopedia_Model_Panel( this, "ModelPanel" );
	m_pModelPanel->m_eMode = CRD_Swarmopedia_Model_Panel::MODE_FULLSCREEN_MOUSE;

	m_pLblBiography = new vgui::MultiFontRichText( this, "LblBiography" );
	m_pLblBiography->SetPanelInteractive( false );
	m_pLblBiography->SetCursor( vgui::dc_arrow );

	for ( int i = 0; i < ASW_NUM_SKILL_SLOTS - 1; i++ )
		m_pSkillPanel[i] = new CNB_Skill_Panel( this, VarArgs( "SkillPanel%d", i ) );

	m_pMarine = new CRD_VGUI_Loadout_Slot_Marine( this, "MarineSlot", iSlot );
	m_pWeapon[ASW_INVENTORY_SLOT_PRIMARY] = new CRD_VGUI_Loadout_Slot_Weapon( this, "WeaponSlot0", iSlot, ASW_INVENTORY_SLOT_PRIMARY );
	m_pWeapon[ASW_INVENTORY_SLOT_SECONDARY] = new CRD_VGUI_Loadout_Slot_Weapon( this, "WeaponSlot1", iSlot, ASW_INVENTORY_SLOT_SECONDARY );
	m_pWeapon[ASW_INVENTORY_SLOT_EXTRA] = new CRD_VGUI_Loadout_Slot_Weapon( this, "WeaponSlot2", iSlot, ASW_INVENTORY_SLOT_EXTRA );

	m_pImgClass = new vgui::ImagePanel( this, "ImgClass" );
	m_pLblClass = new vgui::Label( this, "LblClass", "" );
	m_pLblMarine = new vgui::Label( this, "LblMarineSlot", "" );
	for ( int i = 0; i < ASW_NUM_INVENTORY_SLOTS; i++ )
		m_pLblWeapon[i] = new vgui::Label( this, VarArgs( "LblWeaponSlot%d", i ), "" );
}

CRD_VGUI_Loadout_Marine::~CRD_VGUI_Loadout_Marine()
{
	if ( Loadouts *pLoadouts = assert_cast< Loadouts * >( GetParent() ) )
	{
		pLoadouts->OnLoadoutsUpdated();
	}
}

void CRD_VGUI_Loadout_Marine::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	LoadControlSettings( "Resource/UI/BaseModUI/CRD_VGUI_Loadout_Marine.res", "GAME" );

	CASW_Marine_Profile *pProfile = m_pMarine->GetProfile();
	Assert( pProfile );
	if ( !pProfile )
		return;

	m_pLblClass->SetText( g_szMarineClassLabel[pProfile->m_iMarineClass] );
	m_pImgClass->SetImage( g_szMarineClassImage[pProfile->m_iMarineClass] );

	if ( const ReactiveDropInventory::ItemInstance_t *pSuit = m_pMarine->GetItem() )
	{
		const ReactiveDropInventory::ItemDef_t *pDef = ReactiveDropInventory::GetItemDef( pSuit->ItemDefID );
		m_pLblBiography->SetText( "" );
		pSuit->FormatDescription( m_pLblBiography, false );
		m_pLblMarine->SetText( pDef->Name );
		m_pLblMarine->SetFgColor( pDef->NameColor );
	}
	else if ( const ReactiveDropInventory::ItemDef_t *pDef = m_pMarine->GetItemDef() )
	{
		CRD_ItemInstance dummyInstance;
		dummyInstance.m_iItemDefID = pDef->ID;
		m_pLblBiography->SetText( "" );
		dummyInstance.FormatDescription( m_pLblBiography, false );
		m_pLblMarine->SetText( pDef->Name );
		m_pLblMarine->SetFgColor( pDef->NameColor );
	}
	else
	{
		m_pLblBiography->SetText( pProfile->m_Bio );
		m_pLblMarine->SetText( pProfile->m_ShortName );
	}

	for ( int i = 0; i < ASW_NUM_SKILL_SLOTS - 1; i++ )
	{
		m_pSkillPanel[i]->SetSkillDetails( pProfile->m_ProfileIndex, i, pProfile->GetStaticSkillPoints( i ), pProfile->GetSkillMapping( i ) );
	}

	for ( int iSlot = 0; iSlot < ASW_NUM_INVENTORY_SLOTS; iSlot++ )
	{
		const ReactiveDropInventory::ItemInstance_t *pWeapon = m_pWeapon[iSlot]->GetItem();
		if ( pWeapon )
		{
			const ReactiveDropInventory::ItemDef_t *pDef = ReactiveDropInventory::GetItemDef( pWeapon->ItemDefID );
			m_pLblWeapon[iSlot]->SetText( pDef->Name );
			m_pLblWeapon[iSlot]->SetFgColor( pDef->NameColor );
		}
		else
		{
			int iWeaponIndex = m_pWeapon[iSlot]->GetEquipIndex( pProfile->m_iDefaultLoadoutIndex );
			CASW_EquipItem *pItem = g_ASWEquipmentList.GetItemForSlot( iSlot, iWeaponIndex );
			m_pLblWeapon[iSlot]->SetText( pItem->m_szLongName );
		}
	}

	if ( !m_hLoadout || m_hLoadout->IsLoadoutReadOnly() )
	{
		m_pMarine->SetEnabled( false );
		m_pWeapon[ASW_INVENTORY_SLOT_PRIMARY]->SetEnabled( false );
		m_pWeapon[ASW_INVENTORY_SLOT_SECONDARY]->SetEnabled( false );
		m_pWeapon[ASW_INVENTORY_SLOT_EXTRA]->SetEnabled( false );
	}

	SetupDisplay();
}

void CRD_VGUI_Loadout_Marine::OnCommand( const char *command )
{
	if ( !V_stricmp( command, "Back" ) )
	{
		CBaseModPanel::GetSingleton().PlayUISound( UISOUND_BACK );

		MarkForDeletion();

		return;
	}

	BaseClass::OnCommand( command );
}

void CRD_VGUI_Loadout_Marine::SetupDisplay()
{
	CASW_Marine_Profile *pProfile = m_pMarine->GetProfile();
	Assert( pProfile );
	if ( !pProfile )
		return;

	RD_Swarmopedia::Display *pDisplay = RD_Swarmopedia::Helpers::ReadFromFile<RD_Swarmopedia::Display>( "resource/swarmopedia_loadout_display.txt", m_pKVDisplay );
	if ( !pDisplay )
		pDisplay = new RD_Swarmopedia::Display();

	if ( pDisplay->Models.Count() == 0 )
		pDisplay->Models.AddToTail( new RD_Swarmopedia::Model() );
	pDisplay->Models[0]->ModelName = pProfile->GetModelName();
	pDisplay->Models[0]->Skin = pProfile->GetSkinNum();
	pDisplay->Models[0]->BodyGroups.Purge();
	pDisplay->Models[0]->BodyGroups.Insert( 1, pProfile->GetSkinNum() );
	pDisplay->Models[0]->Animations.Purge();
	pDisplay->Models[0]->Animations.Insert( rd_loadout_temp_animation.GetString(), 1.0f);

	ASW_Inventory_slot_t iDisplayedSlot = m_bSecondaryWeapon ? ASW_INVENTORY_SLOT_SECONDARY : ASW_INVENTORY_SLOT_PRIMARY;
	CASW_EquipItem *pWeaponInfo = g_ASWEquipmentList.GetRegular( m_pWeapon[iDisplayedSlot]->GetEquipIndex( pProfile->m_iDefaultLoadoutIndex ) );
	if ( CASW_WeaponInfo *pWeaponData = g_ASWEquipmentList.GetWeaponDataFor( pWeaponInfo->m_szEquipClass ) )
	{
		int i = pDisplay->Models.AddToTail( new RD_Swarmopedia::Model() );
		pDisplay->Models[i]->ModelName = pWeaponData->szViewModel;
		pDisplay->Models[i]->Skin = pWeaponData->m_iPlayerModelSkin;
		pDisplay->Models[i]->BoneMerge = 0;
		pDisplay->Models[i]->IsWeapon = true;
	}

	const CASW_EquipItem *pExtraInfo = g_ASWEquipmentList.GetExtra( m_pWeapon[ASW_INVENTORY_SLOT_EXTRA]->GetEquipIndex( pProfile->m_iDefaultLoadoutIndex ) );
	if ( pExtraInfo->m_bViewModelIsMarineAttachment )
	{
		if ( CASW_WeaponInfo *pExtraData = g_ASWEquipmentList.GetWeaponDataFor( pExtraInfo->m_szEquipClass ) )
		{
			int i = pDisplay->Models.AddToTail( new RD_Swarmopedia::Model() );
			pDisplay->Models[i]->ModelName = pExtraData->szViewModel;
			pDisplay->Models[i]->Skin = pProfile->GetSkinNum();
			pDisplay->Models[i]->BoneMerge = 0;
			if ( pExtraInfo->m_bViewModelHidesMarineBodyGroup1 )
			{
				pDisplay->Models[0]->BodyGroups.InsertOrReplace( 1, 0 );
			}
		}
	}

	m_pModelPanel->m_bUseTimeScale = false;
	m_pModelPanel->m_bAutoPosition = false;
	m_pModelPanel->m_vecCenter = Vector{ 0.0f, 0.0f, 56.0f };
	m_pModelPanel->m_flRadius = 128.0f;
	m_pModelPanel->m_flPitchIntensity = -15.0f;
	m_pModelPanel->m_flYawIntensity = 10.0f;
	m_pModelPanel->m_angPanOrigin.Init( 25.0f, 0.0f, 0.0f );
	m_pModelPanel->SetDisplay( pDisplay );
	delete pDisplay;

	if ( g_hBriefingTooltip )
		g_hBriefingTooltip->SetTooltipIgnoresCursor( false );
}

void CRD_VGUI_Loadout_Marine::OnThink()
{
	BaseClass::OnThink();

	if ( m_pWeapon[ASW_INVENTORY_SLOT_PRIMARY]->HasFocus() && m_bSecondaryWeapon )
	{
		m_bSecondaryWeapon = false;
		InvalidateLayout( true, true );
	}
	else if ( m_pWeapon[ASW_INVENTORY_SLOT_SECONDARY]->HasFocus() && !m_bSecondaryWeapon )
	{
		m_bSecondaryWeapon = true;
		InvalidateLayout( true, true );
	}
}

void CRD_VGUI_Loadout_Marine::NavigateTo()
{
	BaseClass::NavigateTo();

	NavigateToChild( m_pWeapon[0] );
}

CRD_VGUI_Loadout_Slot::CRD_VGUI_Loadout_Slot( vgui::Panel *parent, const char *panelName ) :
	BaseClass( parent, panelName, "", this, "Click" )
{
	SetConsoleStylePanel( true );
	SetPaintBackgroundEnabled( false );
}

void CRD_VGUI_Loadout_Slot::Paint()
{
	const ReactiveDropInventory::ItemDef_t *pDef = GetItemDef();
	const ReactiveDropInventory::ItemInstance_t *pItem = GetItem();
	if ( !pDef )
		return;

	int w, t;
	GetSize( w, t );

	int ht = t / 2;

	IImage *pIcon = pItem ? pItem->GetIcon() : pDef->Icon;
	if ( pIcon )
	{
		if ( HasFocus() )
			vgui::surface()->DrawSetColor( 255, 255, 255, 255 );
		else
			vgui::surface()->DrawSetColor( 128, 128, 128, 255 );

		vgui::surface()->DrawSetTexture( pIcon->GetID() );

		if ( PaintItemFullSize() )
		{
			vgui::surface()->DrawTexturedRect( 0, 0, w, t );
		}
		else
		{
			vgui::surface()->DrawTexturedRect( w - ht, t - ht, w, t );
		}
	}
}

void CRD_VGUI_Loadout_Slot::OnCursorEntered()
{
	BaseClass::OnCursorEntered();

	GetParent()->NavigateToChild( this );
}

void CRD_VGUI_Loadout_Slot::NavigateTo()
{
	BaseClass::NavigateTo();

	RequestFocus();
}

void CRD_VGUI_Loadout_Slot::OnCommand( const char *command )
{
	if ( !V_stricmp( command, "Click" ) )
	{
		OnClick();
	}
	else
	{
		BaseClass::OnCommand( command );
	}
}

const ReactiveDropInventory::ItemDef_t *CRD_VGUI_Loadout_Slot::GetItemDef()
{
	SteamItemDef_t id = GetItemDefID();
	if ( id == 0 )
		return NULL;

	return ReactiveDropInventory::GetItemDef( id );
}

const ReactiveDropInventory::ItemInstance_t *CRD_VGUI_Loadout_Slot::GetItem()
{
	SteamItemInstanceID_t id = GetItemInstanceID();
	if ( id == 0 || id == k_SteamItemInstanceIDInvalid )
		return NULL;

	return ReactiveDropInventory::GetLocalItemCache( id );
}

CRD_VGUI_Loadout_Slot_Inventory::CRD_VGUI_Loadout_Slot_Inventory( vgui::Panel *parent, const char *panelName, ConVar *pInventoryVar, const char *szSlot ) :
	BaseClass( parent, panelName )
{
	m_pInventoryVar = pInventoryVar;
	V_strncpy( m_szSlot, szSlot, sizeof( m_szSlot ) );
}

SteamItemDef_t CRD_VGUI_Loadout_Slot_Inventory::GetItemDefID()
{
	const ReactiveDropInventory::ItemInstance_t *pItem = GetItem();
	if ( pItem )
		return pItem->ItemDefID;

	return 0;
}

SteamItemInstanceID_t CRD_VGUI_Loadout_Slot_Inventory::GetItemInstanceID()
{
	return V_atoui64( m_pInventoryVar->GetString() );
}

void CRD_VGUI_Loadout_Slot_Inventory::OnClick()
{
	CBaseModFrame *pCaller = CBaseModPanel::GetSingleton().GetWindow( WT_LOADOUTS );
	LaunchCollectionsFrame();

	TabbedGridDetails *pCollections = assert_cast< TabbedGridDetails * >( CBaseModPanel::GetSingleton().GetWindow( WT_COLLECTIONS ) );
	Assert( pCollections );
	if ( pCollections )
	{
		Assert( pCaller );
		if ( pCaller )
		{
			pCollections->SetNavBack( pCaller );
		}

		CRD_Collection_Tab_Inventory *pFoundTab = NULL;

		FOR_EACH_VEC( pCollections->m_Tabs, i )
		{
			CRD_Collection_Tab_Inventory *pTab = assert_cast< CRD_Collection_Tab_Inventory * >( pCollections->m_Tabs[i] );
			if ( pTab->ShowsItemsForSlot( m_szSlot ) )
			{
				pCollections->ActivateTab( pTab );
				pFoundTab = pTab;

				break;
			}

			if ( pFoundTab )
			{
				break;
			}
		}

		if ( pFoundTab )
		{
			SteamItemInstanceID_t id = V_atoui64( m_pInventoryVar->GetString() );

			FOR_EACH_VEC( pFoundTab->m_pGrid->m_Entries, i )
			{
				CRD_Collection_Entry_Inventory *pEntry = assert_cast< CRD_Collection_Entry_Inventory * >( pFoundTab->m_pGrid->m_Entries[i] );
				if ( pEntry && pEntry->m_Details.ItemID == id )
				{
					pEntry->m_pFocusHolder->RequestFocus();

					break;
				}
			}
		}
	}
}

CRD_VGUI_Loadout_Slot_Marine::CRD_VGUI_Loadout_Slot_Marine( CRD_VGUI_Loadout_Marine *parent, const char *panelName, int iMarineSlot ) :
	BaseClass( parent, panelName )
{
	m_iMarineSlot = iMarineSlot;
	V_snprintf( m_szSlot, sizeof( m_szSlot ), "marine%d", iMarineSlot );
}

void CRD_VGUI_Loadout_Slot_Marine::Paint()
{
	CASW_Marine_Profile *pProfile = GetProfile();
	Assert( pProfile );
	if ( !pProfile )
		return;

	int w, t;
	GetSize( w, t );

	vgui::surface()->DrawSetColor( 255, 255, 255, 255 );
	vgui::surface()->DrawSetTexture( HasFocus() ? pProfile->m_nPortraitLitTextureID : pProfile->m_nPortraitTextureID );
	vgui::surface()->DrawTexturedRect( 0, 0, w, t );

	BaseClass::Paint();
}

SteamItemDef_t CRD_VGUI_Loadout_Slot_Marine::GetItemDefID()
{
	CRD_VGUI_Loadout_List_Item *pLoadout = assert_cast< CRD_VGUI_Loadout_Marine * >( GetParent() )->m_hLoadout.Get();
	Assert( pLoadout );
	if ( !pLoadout )
		return 0;

	return pLoadout->GetMarineItemDefID( m_iMarineSlot );
}

SteamItemInstanceID_t CRD_VGUI_Loadout_Slot_Marine::GetItemInstanceID()
{
	CRD_VGUI_Loadout_List_Item *pLoadout = assert_cast< CRD_VGUI_Loadout_Marine * >( GetParent() )->m_hLoadout.Get();
	Assert( pLoadout );
	if ( !pLoadout )
		return k_SteamItemInstanceIDInvalid;

	return pLoadout->GetMarineItemID( m_iMarineSlot );
}

void CRD_VGUI_Loadout_Slot_Marine::OnClick()
{
	Assert( !"TODO" );
}

CASW_Marine_Profile *CRD_VGUI_Loadout_Slot_Marine::GetProfile()
{
	CRD_VGUI_Loadout_List_Item *pLoadout = assert_cast< CRD_VGUI_Loadout_Marine * >( GetParent() )->m_hLoadout.Get();
	Assert( pLoadout );
	if ( !pLoadout )
		return NULL;

	return MarineProfileList()->GetProfile( pLoadout->GetMarineProfileForSlot( m_iMarineSlot ) );
}

CRD_VGUI_Loadout_Slot_Weapon::CRD_VGUI_Loadout_Slot_Weapon( CRD_VGUI_Loadout_Marine *parent, const char *panelName, int iMarineSlot, ASW_Inventory_slot_t iSlot ) :
	BaseClass( parent, panelName )
{
	m_iMarineSlot = iMarineSlot;
	m_iSlot = iSlot;

	if ( iSlot == ASW_INVENTORY_SLOT_EXTRA )
		V_strncpy( m_szSlot, "extra", sizeof( m_szSlot ) );
	else
		V_strncpy( m_szSlot, "weapon", sizeof( m_szSlot ) );
}

void CRD_VGUI_Loadout_Slot_Weapon::Paint()
{
	int w, t;
	GetSize( w, t );

	if ( HasFocus() )
		vgui::surface()->DrawSetColor( 255, 255, 255, 255 );
	else
		vgui::surface()->DrawSetColor( 64, 96, 128, 255 );

	CRD_VGUI_Loadout_Marine *pParent = assert_cast< CRD_VGUI_Loadout_Marine * >( GetParent() );
	CASW_Marine_Profile *pProfile = pParent->m_pMarine->GetProfile();
	Assert( pProfile );
	if ( !pProfile )
		return;

	vgui::surface()->DrawSetTexture( g_ASWEquipmentList.GetEquipIconTexture( m_iSlot != ASW_INVENTORY_SLOT_EXTRA, GetEquipIndex( pProfile->m_iDefaultLoadoutIndex ) ) );
	vgui::surface()->DrawTexturedRect( 0, 0, w, t );

	BaseClass::Paint();
}

SteamItemDef_t CRD_VGUI_Loadout_Slot_Weapon::GetItemDefID()
{
	CRD_VGUI_Loadout_List_Item *pLoadout = assert_cast< CRD_VGUI_Loadout_Marine * >( GetParent() )->m_hLoadout.Get();
	Assert( pLoadout );
	if ( !pLoadout )
		return 0;

	return pLoadout->GetWeaponItemDefID( m_iMarineSlot, m_iSlot );
}

SteamItemInstanceID_t CRD_VGUI_Loadout_Slot_Weapon::GetItemInstanceID()
{
	CRD_VGUI_Loadout_List_Item *pLoadout = assert_cast< CRD_VGUI_Loadout_Marine * >( GetParent() )->m_hLoadout.Get();
	Assert( pLoadout );
	if ( !pLoadout )
		return k_SteamItemInstanceIDInvalid;

	return pLoadout->GetWeaponItemID( m_iMarineSlot, m_iSlot );
}

void CRD_VGUI_Loadout_Slot_Weapon::OnClick()
{
	TabbedGridDetails *pWeaponPanel = new TabbedGridDetails();
	if ( m_iSlot == ASW_INVENTORY_SLOT_PRIMARY )
		pWeaponPanel->SetTitle( "#nb_select_weapon_one", true );
	else if ( m_iSlot == ASW_INVENTORY_SLOT_SECONDARY )
		pWeaponPanel->SetTitle( "#nb_select_weapon_two", true );
	else if ( m_iSlot == ASW_INVENTORY_SLOT_EXTRA )
		pWeaponPanel->SetTitle( "#nb_select_offhand", true );

	CRD_VGUI_Loadout_Marine *pParent = assert_cast< CRD_VGUI_Loadout_Marine * >( GetParent() );
	CASW_Marine_Profile *pProfile = pParent->m_pMarine->GetProfile();
	Assert( pProfile );
	if ( !pProfile )
		return;

	CRD_Collection_Tab_Equipment *pTab = new CRD_Collection_Tab_Equipment( pWeaponPanel, m_iSlot == ASW_INVENTORY_SLOT_EXTRA ? "#rd_collection_equipment" : "#rd_collection_weapons", pProfile, m_iSlot );
	pTab->SetBriefing( assert_cast< Loadouts * >( CBaseModPanel::GetSingleton().GetWindow( WT_LOADOUTS ) ), m_iMarineSlot );
	pWeaponPanel->AddTab( pTab );

	pWeaponPanel->ShowFullScreen();
}

int CRD_VGUI_Loadout_Slot_Weapon::GetEquipIndex( int iDefaultLoadoutSlot )
{
	CRD_VGUI_Loadout_List_Item *pLoadout = assert_cast< CRD_VGUI_Loadout_Marine * >( GetParent() )->m_hLoadout.Get();
	Assert( pLoadout );
	if ( !pLoadout )
		return -1;

	return pLoadout->GetWeaponForSlot( m_iMarineSlot, m_iSlot );
}

CRD_VGUI_Loadout_Share::CRD_VGUI_Loadout_Share( vgui::Panel *parent, const char *panelName ) :
	BaseClass( parent, panelName )
{
	MarkForDeletion();

	GenericConfirmation *pConfirm = assert_cast< GenericConfirmation * >( CBaseModPanel::GetSingleton().OpenWindow( WT_GENERICCONFIRMATION, assert_cast< Loadouts * >( parent ) ) );

	GenericConfirmation::Data_t data;

	data.pWindowTitle = "#rd_beta_coming_soon";
	data.pMessageText = "#rd_beta_coming_soon_desc";
	data.bOkButtonEnabled = true;

	pConfirm->SetUsageData( data );
}
