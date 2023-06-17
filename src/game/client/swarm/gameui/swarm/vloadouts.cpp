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
#include "vgenericwaitscreen.h"
#include "vgenericconfirmation.h"
#include "rd_workshop.h"
#include "asw_marine_profile.h"
#include "vgui_bitmapbutton.h"
#include "asw_medal_store.h"
#include "asw_player_shared.h"
#include "rd_collections.h"
#include "rd_swarmopedia.h"
#include "asw_weapon_parse.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;
using namespace BaseModUI;

extern ConVar rd_override_commander_promotion;
extern ConVar asw_default_primary[ASW_NUM_MARINE_PROFILES + 1];
extern ConVar asw_default_secondary[ASW_NUM_MARINE_PROFILES + 1];
extern ConVar asw_default_extra[ASW_NUM_MARINE_PROFILES + 1];
extern ConVar rd_equipped_medal[RD_STEAM_INVENTORY_NUM_MEDAL_SLOTS];
extern ConVar rd_equipped_marine[ASW_NUM_MARINE_PROFILES];
extern ConVar rd_equipped_weapon_primary[ASW_NUM_MARINE_PROFILES];
extern ConVar rd_equipped_weapon_secondary[ASW_NUM_MARINE_PROFILES];
extern ConVar rd_equipped_weapon_extra[ASW_NUM_MARINE_PROFILES];

Loadouts::Loadouts( Panel *parent, const char *panelName )
	: BaseClass( parent, panelName )
{
	SetProportional( true );

	m_pHeaderFooter = new CNB_Header_Footer( this, "HeaderFooter" );
	m_pHeaderFooter->SetTitle( "" );
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

	for ( int i = 0; i < RD_STEAM_INVENTORY_NUM_MEDAL_SLOTS; i++ )
	{
		m_pMedalSlot[i] = new CRD_VGUI_Loadout_Slot_Inventory( this, VarArgs( "MedalSlot%d", i ), &rd_equipped_medal[i], ReactiveDropInventory::g_InventorySlotNames[RD_STEAM_INVENTORY_EQUIP_SLOT_FIRST_MEDAL + i] );
		m_pLblMedal[i] = new vgui::Label( this, VarArgs( "LblMedal%d", i ), "" );
	}

	m_pImgPromotionIcon = new vgui::ImagePanel( this, "ImgPromotionIcon" );
	m_pLblPromotion = new vgui::Label( this, "LblPromotion", "" );

	for ( int i = 0; i < ASW_NUM_MARINE_PROFILES; i++ )
	{
		m_pBtnMarineLoadout[i] = new CBitmapButton( this, VarArgs( "BtnMarine%d", i ), "" );
	}

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
}

void Loadouts::OnCommand( const char *command )
{
	if ( const char *szMarineNumber = StringAfterPrefix( command, "MarineLoadout" ) )
	{
		m_hSubScreen = new CRD_VGUI_Loadout_Marine( this, "LoadoutMarine", ASW_Marine_Profile( V_atoi( szMarineNumber ) ) );
		m_hSubScreen->MakeReadyForUse();
		NavigateToChild( m_hSubScreen );
	}
	else
	{
		BaseClass::OnCommand( command );
	}
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
	V_strncpy( m_szName, szName, sizeof( m_szName ) );
	m_Loadout = loadout;
	m_iAddonID = id;
}

CRD_VGUI_Loadout_List_Addon_Header::CRD_VGUI_Loadout_List_Addon_Header( vgui::Panel *parent, const char *panelName, PublishedFileId_t id ) :
	BaseClass( parent, panelName )
{
	m_iAddonID = id;
}

CRD_VGUI_Loadout_Marine::CRD_VGUI_Loadout_Marine( vgui::Panel *parent, const char *panelName, ASW_Marine_Profile iProfile ) :
	BaseClass( parent, panelName )
{
	m_pModelPanel = new CRD_Swarmopedia_Model_Panel( this, "ModelPanel" );
	m_pLblBiography = new vgui::RichText( this, "LblBiography" );
	m_pLblBiography->SetPanelInteractive( false );
	m_pLblBiography->SetCursor( vgui::dc_arrow );
	for ( int i = 0; i < ASW_NUM_SKILL_SLOTS - 1; i++ )
		m_pSkillPanel[i] = new CNB_Skill_Panel( this, VarArgs( "SkillPanel%d", i ) );
	m_pMarine = new CRD_VGUI_Loadout_Slot_Marine( this, "MarineSlot", iProfile, &rd_equipped_marine[iProfile] );
	m_pWeapon[ASW_INVENTORY_SLOT_PRIMARY] = new CRD_VGUI_Loadout_Slot_Weapon( this, "WeaponSlot0", iProfile, &asw_default_primary[iProfile], &rd_equipped_weapon_primary[iProfile], ASW_INVENTORY_SLOT_PRIMARY );
	m_pWeapon[ASW_INVENTORY_SLOT_SECONDARY] = new CRD_VGUI_Loadout_Slot_Weapon( this, "WeaponSlot1", iProfile, &asw_default_secondary[iProfile], &rd_equipped_weapon_secondary[iProfile], ASW_INVENTORY_SLOT_SECONDARY );
	m_pWeapon[ASW_INVENTORY_SLOT_EXTRA] = new CRD_VGUI_Loadout_Slot_Weapon( this, "WeaponSlot2", iProfile, &asw_default_extra[iProfile], &rd_equipped_weapon_extra[iProfile], ASW_INVENTORY_SLOT_EXTRA );
}

void CRD_VGUI_Loadout_Marine::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	LoadControlSettings( "Resource/UI/BaseModUI/CRD_VGUI_Loadout_Marine.res", "GAME" );

	CASW_Marine_Profile *pProfile = MarineProfileList()->GetProfile( m_pMarine->m_iProfile );
	Assert( pProfile );
	if ( !pProfile )
		return;

	const ReactiveDropInventory::ItemInstance_t *pSuit = m_pMarine->GetItem();
	if ( pSuit )
	{
		pSuit->FormatDescription( m_pLblBiography, false );
	}
	else
	{
		m_pLblBiography->SetText( pProfile->m_Bio );
	}

	for ( int i = 0; i < ASW_NUM_SKILL_SLOTS - 1; i++ )
	{
		m_pSkillPanel[i]->SetSkillDetails( m_pMarine->m_iProfile, i, pProfile->GetStaticSkillPoints( i ), pProfile->GetSkillMapping( i ) );
	}

	SetupDisplay();
}

void CRD_VGUI_Loadout_Marine::SetupDisplay()
{
	CASW_Marine_Profile *pProfile = MarineProfileList()->GetProfile( m_pMarine->m_iProfile );
	Assert( pProfile );
	if ( !pProfile )
		return;

	RD_Swarmopedia::Display MarineDisplay;

	MarineDisplay.Models.AddToTail( new RD_Swarmopedia::Model() );
	MarineDisplay.Models[0]->ModelName = pProfile->GetModelName();
	MarineDisplay.Models[0]->Skin = pProfile->GetSkinNum();
	MarineDisplay.Models[0]->BodyGroups.Insert( 1, pProfile->GetSkinNum() );
	MarineDisplay.Models[0]->Animations.Insert( "idle_aiming_neutral", 1.0f );

	//MarineDisplay.Models.AddToTail( new RD_Swarmopedia::Model() );
	//MarineDisplay.Models[1]->ModelName = "models/error.mdl"; // TODO: room model

	if ( CASW_WeaponInfo *pWeaponData = g_ASWEquipmentList.GetWeaponDataFor( g_ASWEquipmentList.GetRegular( m_pWeapon[0]->m_pWeaponVar->GetInt() )->m_szEquipClass ) )
	{
		int i = MarineDisplay.Models.AddToTail( new RD_Swarmopedia::Model() );
		MarineDisplay.Models[i]->ModelName = pWeaponData->szViewModel;
		MarineDisplay.Models[i]->Skin = pWeaponData->m_iPlayerModelSkin;
		MarineDisplay.Models[i]->BoneMerge = 0;
		MarineDisplay.Models[i]->IsWeapon = true;
	}

	const CASW_EquipItem *pExtraInfo = g_ASWEquipmentList.GetExtra( m_pWeapon[2]->m_pWeaponVar->GetInt() );
	if ( pExtraInfo->m_bViewModelIsMarineAttachment )
	{
		if ( CASW_WeaponInfo *pExtraData = g_ASWEquipmentList.GetWeaponDataFor( pExtraInfo->m_szEquipClass ) )
		{
			int i = MarineDisplay.Models.AddToTail( new RD_Swarmopedia::Model() );
			MarineDisplay.Models[i]->ModelName = pExtraData->szViewModel;
			MarineDisplay.Models[i]->Skin = pProfile->GetSkinNum();
			MarineDisplay.Models[i]->BoneMerge = 0;
			if ( pExtraInfo->m_bViewModelHidesMarineBodyGroup1 )
			{
				MarineDisplay.Models[0]->BodyGroups.InsertOrReplace( 1, 0 );
			}
		}
	}

	FOR_EACH_VEC( MarineDisplay.Models, i )
	{
		// fudge the numbers so we get a better camera view.
		MarineDisplay.Models[i]->Z += 32.0f;
		MarineDisplay.Models[i]->Scale *= 0.5f;
	}

	MarineDisplay.LightingState = SwarmopediaDefaultLightingState();
	m_pModelPanel->SetDisplay( &MarineDisplay );
}

CRD_VGUI_Loadout_Slot::CRD_VGUI_Loadout_Slot( vgui::Panel *parent, const char *panelName, ConVar *pInventoryVar ) :
	BaseClass( parent, panelName )
{
	m_pInventoryVar = pInventoryVar;
}

void CRD_VGUI_Loadout_Slot::Paint()
{
	const ReactiveDropInventory::ItemInstance_t *pItem = GetItem();
	if ( !pItem )
		return;

	int w, t;
	GetSize( w, t );

	int ht = t / 2, qt = t / 4;

	CRD_ItemInstance instance{ *pItem };
	int nAccessories = 0;
	for ( int i = 0; i < RD_ITEM_MAX_ACCESSORIES; i++ )
	{
		if ( instance.m_iAccessory[i] )
		{
			nAccessories++;
		}
	}

	IImage *pIcon = pItem->GetIcon();
	if ( pIcon )
	{
		vgui::surface()->DrawSetColor( 255, 255, 255, 255 );
		vgui::surface()->DrawSetTexture( pIcon->GetID() );
		if ( PaintItemFullSize() )
		{
			vgui::surface()->DrawTexturedRect( 0, 0, w, t );
		}
		else
		{
			int y1 = t;
			if ( nAccessories )
				y1 -= qt;

			vgui::surface()->DrawTexturedRect( w - ht, y1 - ht, w, y1 );
		}
	}

	for ( int i = 0, j = 0; i < RD_ITEM_MAX_ACCESSORIES; i++ )
	{
		if ( instance.m_iAccessory[i] )
		{
			const ReactiveDropInventory::ItemDef_t *pAccessory = ReactiveDropInventory::GetItemDef( instance.m_iAccessory[i] );
			Assert( pAccessory && pAccessory->Icon );

			if ( pAccessory && pAccessory->Icon && pAccessory->Icon->GetNumFrames() )
			{
				vgui::surface()->DrawSetColor( 255, 255, 255, 255 );
				vgui::surface()->DrawSetTexture( pAccessory->Icon->GetID() );
				vgui::surface()->DrawTexturedRect( w - ( nAccessories - j ) * qt, t - qt, w - ( nAccessories - j - 1 ) * qt, t );
			}

			j++;
		}
	}
}

const ReactiveDropInventory::ItemInstance_t *CRD_VGUI_Loadout_Slot::GetItem()
{
	SteamItemInstanceID_t id = V_atoui64( m_pInventoryVar->GetString() );
	if ( id == 0 || id == k_SteamItemInstanceIDInvalid )
		return NULL;

	return ReactiveDropInventory::GetLocalItemCache( id );
}

CRD_VGUI_Loadout_Slot_Inventory::CRD_VGUI_Loadout_Slot_Inventory( vgui::Panel *parent, const char *panelName, ConVar *pInventoryVar, const char *szSlot ) :
	BaseClass( parent, panelName, pInventoryVar )
{
	V_strncpy( m_szSlot, szSlot, sizeof( m_szSlot ) );
}

CRD_VGUI_Loadout_Slot_Marine::CRD_VGUI_Loadout_Slot_Marine( vgui::Panel *parent, const char *panelName, ASW_Marine_Profile iProfile, ConVar *pInventoryVar ) :
	BaseClass( parent, panelName, pInventoryVar )
{
	m_iProfile = iProfile;
	V_snprintf( m_szSlot, sizeof( m_szSlot ), "marine%d", iProfile );
}

void CRD_VGUI_Loadout_Slot_Marine::Paint()
{
	CASW_Marine_Profile *pProfile = MarineProfileList()->GetProfile( m_iProfile );
	Assert( pProfile );
	if ( !pProfile )
		return;

	int w, t;
	GetSize( w, t );

	vgui::surface()->DrawSetColor( 255, 255, 255, 255 );
	vgui::surface()->DrawSetTexture( pProfile->m_nPortraitTextureID );
	vgui::surface()->DrawTexturedRect( 0, 0, w, t );

	BaseClass::Paint();
}

CRD_VGUI_Loadout_Slot_Weapon::CRD_VGUI_Loadout_Slot_Weapon( vgui::Panel *parent, const char *panelName, ASW_Marine_Profile iProfile, ConVar *pWeaponVar, ConVar *pInventoryVar, ASW_Inventory_slot_t iSlot ) :
	BaseClass( parent, panelName, pInventoryVar )
{
	m_iProfile = iProfile;
	m_iSlot = iSlot;
	m_pWeaponVar = pWeaponVar;

	if ( iSlot == ASW_INVENTORY_SLOT_EXTRA )
		V_strncpy( m_szSlot, "extra", sizeof( m_szSlot ) );
	else
		V_strncpy( m_szSlot, "weapon", sizeof( m_szSlot ) );
}

void CRD_VGUI_Loadout_Slot_Weapon::Paint()
{
	int w, t;
	GetSize( w, t );

	vgui::surface()->DrawSetColor( 255, 255, 255, 255 );
	vgui::surface()->DrawSetTexture( g_ASWEquipmentList.GetEquipIconTexture( m_iSlot != ASW_INVENTORY_SLOT_EXTRA, m_pWeaponVar->GetInt() ) );
	vgui::surface()->DrawTexturedRect( 0, 0, w, t );

	BaseClass::Paint();
}
