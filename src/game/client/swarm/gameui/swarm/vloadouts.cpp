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
#include "briefingtooltip.h"
#include "asw_weapon_shared.h"
#include "vgui_avatarimage.h"
#include "rd_workshop_preview_image.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;
using namespace BaseModUI;

extern ConVar rd_override_commander_promotion;
extern ConVar asw_default_primary[ASW_NUM_MARINE_PROFILES];
extern ConVar asw_default_secondary[ASW_NUM_MARINE_PROFILES];
extern ConVar asw_default_extra[ASW_NUM_MARINE_PROFILES];
extern ConVar rd_equipped_medal[RD_STEAM_INVENTORY_NUM_MEDAL_SLOTS];
extern ConVar rd_equipped_marine[ASW_NUM_MARINE_PROFILES];
extern ConVar rd_equipped_weapon_primary[ASW_NUM_MARINE_PROFILES];
extern ConVar rd_equipped_weapon_secondary[ASW_NUM_MARINE_PROFILES];
extern ConVar rd_equipped_weapon_extra[ASW_NUM_MARINE_PROFILES];
extern ConVar asw_unlock_all_weapons;

const IBriefing_ItemInstance Loadouts::s_EmptyItemInstance;

static int GetWeaponIndex( ASW_Inventory_slot_t iSlot, int iWeapon, ASW_Marine_Profile iProfile )
{
	if ( iWeapon == -1 )
	{
		CASW_Marine_Profile *pProfile = MarineProfileList()->GetProfile( iProfile );

		switch ( iSlot )
		{
		case ASW_INVENTORY_SLOT_PRIMARY:
			return ReactiveDropLoadout::DefaultLoadout.Marines[pProfile->m_iDefaultLoadoutIndex].Primary;
		case ASW_INVENTORY_SLOT_SECONDARY:
			return ReactiveDropLoadout::DefaultLoadout.Marines[pProfile->m_iDefaultLoadoutIndex].Secondary;
		case ASW_INVENTORY_SLOT_EXTRA:
			return ReactiveDropLoadout::DefaultLoadout.Marines[pProfile->m_iDefaultLoadoutIndex].Extra;
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

	for ( int i = 0; i < RD_STEAM_INVENTORY_NUM_MEDAL_SLOTS; i++ )
	{
		m_pMedalSlot[i] = new CRD_VGUI_Loadout_Slot_Inventory( this, VarArgs( "MedalSlot%d", i ), &rd_equipped_medal[i], ReactiveDropInventory::g_InventorySlotNames[RD_STEAM_INVENTORY_EQUIP_SLOT_FIRST_MEDAL + i] );
		m_pLblMedal[i] = new vgui::Label( this, VarArgs( "LblMedal%d", i ), "" );
	}

	m_pImgPromotionIcon = new vgui::ImagePanel( this, "ImgPromotionIcon" );
	m_pLblPromotion = new vgui::Label( this, "LblPromotion", "" );

	for ( int i = 0; i < ASW_NUM_MARINE_PROFILES; i++ )
	{
		m_pLblMarineName[i] = new vgui::Label( this, VarArgs( "LblMarine%d", i ), "" );
		m_pBtnMarineLoadout[i] = new CBitmapButton( this, VarArgs( "BtnMarine%d", i ), "" );
		m_pBtnMarineLoadout[i]->SetConsoleStylePanel( true );
		m_pBtnMarineLoadout[i]->SetFocusOnNavigateTo( true );
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

	m_pGplSavedLoadouts->SetNavUp( m_pTopBar );
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

	InitLoadoutList();
	NavigateToChild( m_pBtnMarineLoadout[0] );
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

void Loadouts::OnKeyCodeTyped( vgui::KeyCode code )
{
	if ( code == KEY_ESCAPE && m_hSubScreen )
	{
		m_hSubScreen->MarkForDeletion();

		return;
	}

	BaseClass::OnKeyCodeTyped( code );
}

void Loadouts::InitLoadoutList()
{
	ReactiveDropLoadout::InitLoadouts();

	m_pGplSavedLoadouts->RemoveAllPanelItems();

	char szLoadoutName[128];
	V_UnicodeToUTF8( g_pVGuiLocalize->Find( "#rd_loadout_current" ), szLoadoutName, sizeof( szLoadoutName ) );
	ReactiveDropLoadout::LoadoutData_t CurrentLoadout;
	CurrentLoadout.CopyFromLive();
	m_pGplSavedLoadouts->AddPanelItem( new CRD_VGUI_Loadout_List_Item( this, "LoadoutListItem", szLoadoutName, CurrentLoadout ), true );

	V_UnicodeToUTF8( g_pVGuiLocalize->Find( "#rd_loadout_default" ), szLoadoutName, sizeof( szLoadoutName ) );
	m_pGplSavedLoadouts->AddPanelItem( new CRD_VGUI_Loadout_List_Item( this, "LoadoutListItem", szLoadoutName, ReactiveDropLoadout::DefaultLoadout ), true );

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
	m_pGplSavedLoadouts->SelectPanelItem( 0 );
}

void Loadouts::ShowLoadoutItem( CRD_VGUI_Loadout_List_Item *pItem )
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

CASW_Marine_Profile *Loadouts::GetMarineProfile( int nLobbySlot )
{
	return GetMarineProfileByProfileIndex( nLobbySlot );
}

CASW_Marine_Profile *Loadouts::GetMarineProfileByProfileIndex( int nProfileIndex )
{
	return MarineProfileList()->GetProfile( nProfileIndex );
}

int Loadouts::GetProfileSelectedWeapon( int nProfileIndex, int nWeaponSlot )
{
	if ( nWeaponSlot == ASW_INVENTORY_SLOT_PRIMARY )
		return GetWeaponIndex( ASW_INVENTORY_SLOT_PRIMARY, asw_default_primary[nProfileIndex].GetInt(), ASW_Marine_Profile( nProfileIndex ) );
	if ( nWeaponSlot == ASW_INVENTORY_SLOT_SECONDARY )
		return GetWeaponIndex( ASW_INVENTORY_SLOT_SECONDARY, asw_default_secondary[nProfileIndex].GetInt(), ASW_Marine_Profile( nProfileIndex ) );
	if ( nWeaponSlot == ASW_INVENTORY_SLOT_EXTRA )
		return GetWeaponIndex( ASW_INVENTORY_SLOT_EXTRA, asw_default_extra[nProfileIndex].GetInt(), ASW_Marine_Profile( nProfileIndex ) );
	return -1;
}

const char *Loadouts::GetMarineWeaponClass( int nLobbySlot, int nWeaponSlot )
{
	if ( nWeaponSlot == ASW_INVENTORY_SLOT_EXTRA )
		return g_ASWEquipmentList.GetExtra( GetMarineSelectedWeapon( nLobbySlot, nWeaponSlot ) )->m_szEquipClass;
	return g_ASWEquipmentList.GetRegular( GetMarineSelectedWeapon( nLobbySlot, nWeaponSlot ) )->m_szEquipClass;
}

int Loadouts::GetMarineSkillPoints( int nLobbySlot, int nSkillSlot )
{
	return GetProfileSkillPoints( nLobbySlot, nSkillSlot );
}

int Loadouts::GetProfileSkillPoints( int nProfileIndex, int nSkillSlot )
{
	return MarineProfileList()->GetProfile( nProfileIndex )->GetStaticSkillPoints( nSkillSlot );
}

bool Loadouts::IsWeaponUnlocked( const char *szWeaponClass )
{
	if ( asw_unlock_all_weapons.GetBool() )
		return true;

	return UTIL_ASW_CommanderLevelAtLeast( NULL, GetWeaponLevelRequirement( szWeaponClass ), -1 );
}

void Loadouts::SelectWeapon( int nProfileIndex, int nInventorySlot, int nEquipIndex, SteamItemInstanceID_t iItemInstance )
{
	if ( nInventorySlot == ASW_INVENTORY_SLOT_PRIMARY )
	{
		asw_default_primary[nProfileIndex].SetValue( nEquipIndex );
		rd_equipped_weapon_primary[nProfileIndex].SetValue( VarArgs( "%llu", iItemInstance ) );
	}
	else if ( nInventorySlot == ASW_INVENTORY_SLOT_SECONDARY )
	{
		asw_default_secondary[nProfileIndex].SetValue( nEquipIndex );
		rd_equipped_weapon_secondary[nProfileIndex].SetValue( VarArgs( "%llu", iItemInstance ) );
	}
	else if ( nInventorySlot == ASW_INVENTORY_SLOT_EXTRA )
	{
		asw_default_extra[nProfileIndex].SetValue( nEquipIndex );
		rd_equipped_weapon_extra[nProfileIndex].SetValue( VarArgs( "%llu", iItemInstance ) );
	}

	engine->ClientCmd_Unrestricted( "host_writeconfig\n" );

	if ( m_hSubScreen )
	{
		m_hSubScreen->InvalidateLayout( true, true );

		if ( vgui::Panel *pSlot = m_hSubScreen->FindChildByName( VarArgs( "WeaponSlot%d", nInventorySlot ) ) )
		{
			pSlot->RequestFocus();
		}
		else
		{
			m_hSubScreen->RequestFocus();
		}
	}
	else
	{
		RequestFocus();
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
	for ( int i = 0; i < ASW_NUM_MARINE_PROFILES; i++ )
	{
		if ( !m_Loadout.MarineIncluded[i] )
			continue;

		int x = ( ( GetWide() - m_iRowHeight * 6 * m_iNumColumns ) / ( m_iNumColumns + 1 ) + m_iRowHeight * 6 ) * ( col + 1 ) - m_iRowHeight * 6;
		vgui::surface()->DrawSetColor( color );
		vgui::surface()->DrawSetTexture( MarineProfileList()->GetProfile( i )->m_nPortraitTextureID );
		vgui::surface()->DrawTexturedRect( x, y, x + m_iRowHeight, y + m_iRowHeight );

		x += m_iRowHeight;
		vgui::surface()->DrawSetTexture( g_ASWEquipmentList.GetEquipIconTexture( true, GetWeaponIndex( ASW_INVENTORY_SLOT_PRIMARY, m_Loadout.Marines[i].Primary, ASW_Marine_Profile( i ) ) ) );
		vgui::surface()->DrawTexturedRect( x, y, x + m_iRowHeight * 2, y + m_iRowHeight );

		x += m_iRowHeight * 2;
		vgui::surface()->DrawSetTexture( g_ASWEquipmentList.GetEquipIconTexture( true, GetWeaponIndex( ASW_INVENTORY_SLOT_SECONDARY, m_Loadout.Marines[i].Secondary, ASW_Marine_Profile( i ) ) ) );
		vgui::surface()->DrawTexturedRect( x, y, x + m_iRowHeight * 2, y + m_iRowHeight );

		x += m_iRowHeight * 2;
		vgui::surface()->DrawSetTexture( g_ASWEquipmentList.GetEquipIconTexture( false, GetWeaponIndex( ASW_INVENTORY_SLOT_EXTRA, m_Loadout.Marines[i].Extra, ASW_Marine_Profile( i ) ) ) );
		vgui::surface()->DrawTexturedRect( x, y, x + m_iRowHeight, y + m_iRowHeight );

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

CRD_VGUI_Loadout_Marine::CRD_VGUI_Loadout_Marine( vgui::Panel *parent, const char *panelName, ASW_Marine_Profile iProfile ) :
	BaseClass( parent, panelName )
{
	m_pModelPanel = new CRD_Swarmopedia_Model_Panel( this, "ModelPanel" );
	m_pModelPanel->m_eMode = CRD_Swarmopedia_Model_Panel::MODE_FULLSCREEN_MOUSE;
	m_pLblBiography = new vgui::RichText( this, "LblBiography" );
	m_pLblBiography->SetPanelInteractive( false );
	m_pLblBiography->SetCursor( vgui::dc_arrow );
	for ( int i = 0; i < ASW_NUM_SKILL_SLOTS - 1; i++ )
		m_pSkillPanel[i] = new CNB_Skill_Panel( this, VarArgs( "SkillPanel%d", i ) );
	m_pMarine = new CRD_VGUI_Loadout_Slot_Marine( this, "MarineSlot", iProfile, &rd_equipped_marine[iProfile] );
	m_pWeapon[ASW_INVENTORY_SLOT_PRIMARY] = new CRD_VGUI_Loadout_Slot_Weapon( this, "WeaponSlot0", iProfile, &asw_default_primary[iProfile], &rd_equipped_weapon_primary[iProfile], ASW_INVENTORY_SLOT_PRIMARY );
	m_pWeapon[ASW_INVENTORY_SLOT_SECONDARY] = new CRD_VGUI_Loadout_Slot_Weapon( this, "WeaponSlot1", iProfile, &asw_default_secondary[iProfile], &rd_equipped_weapon_secondary[iProfile], ASW_INVENTORY_SLOT_SECONDARY );
	m_pWeapon[ASW_INVENTORY_SLOT_EXTRA] = new CRD_VGUI_Loadout_Slot_Weapon( this, "WeaponSlot2", iProfile, &asw_default_extra[iProfile], &rd_equipped_weapon_extra[iProfile], ASW_INVENTORY_SLOT_EXTRA );
	m_pImgClass = new vgui::ImagePanel( this, "ImgClass" );
	m_pLblClass = new vgui::Label( this, "LblClass", "" );
	m_pLblMarine = new vgui::Label( this, "LblMarineSlot", "" );
	for ( int i = 0; i < ASW_NUM_INVENTORY_SLOTS; i++ )
		m_pLblWeapon[i] = new vgui::Label( this, VarArgs( "LblWeaponSlot%d", i ), "" );
}

void CRD_VGUI_Loadout_Marine::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	LoadControlSettings( "Resource/UI/BaseModUI/CRD_VGUI_Loadout_Marine.res", "GAME" );

	CASW_Marine_Profile *pProfile = MarineProfileList()->GetProfile( m_pMarine->m_iProfile );
	Assert( pProfile );
	if ( !pProfile )
		return;

	m_pLblClass->SetText( g_szMarineClassLabel[pProfile->m_iMarineClass] );
	m_pImgClass->SetImage( g_szMarineClassImage[pProfile->m_iMarineClass] );

	const ReactiveDropInventory::ItemInstance_t *pSuit = m_pMarine->GetItem();
	if ( pSuit )
	{
		const ReactiveDropInventory::ItemDef_t *pDef = ReactiveDropInventory::GetItemDef( pSuit->ItemDefID );
		pSuit->FormatDescription( m_pLblBiography, false );
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
		m_pSkillPanel[i]->SetSkillDetails( m_pMarine->m_iProfile, i, pProfile->GetStaticSkillPoints( i ), pProfile->GetSkillMapping( i ) );
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
			int iWeaponIndex = GetWeaponIndex( ASW_Inventory_slot_t( iSlot ), m_pWeapon[iSlot]->m_pWeaponVar->GetInt(), m_pMarine->m_iProfile );
			CASW_EquipItem *pItem = g_ASWEquipmentList.GetItemForSlot( iSlot, iWeaponIndex );
			m_pLblWeapon[iSlot]->SetText( pItem->m_szLongName );
		}
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
	CASW_Marine_Profile *pProfile = MarineProfileList()->GetProfile( m_pMarine->m_iProfile );
	Assert( pProfile );
	if ( !pProfile )
		return;

	const char *const szFileName = "resource/swarmopedia_loadout_display.txt";

	KeyValues::AutoDelete pKV{ "Display" };
	UTIL_RD_LoadKeyValuesFromFile( pKV, g_pFullFileSystem, szFileName, "GAME" );

	RD_Swarmopedia::Display *pDisplay = RD_Swarmopedia::Helpers::ReadFromFile<RD_Swarmopedia::Display>( szFileName, pKV );
	if ( !pDisplay )
		pDisplay = new RD_Swarmopedia::Display();

	if ( pDisplay->Models.Count() == 0 )
		pDisplay->Models.AddToTail( new RD_Swarmopedia::Model() );
	pDisplay->Models[0]->ModelName = pProfile->GetModelName();
	pDisplay->Models[0]->Skin = pProfile->GetSkinNum();
	pDisplay->Models[0]->BodyGroups.Purge();
	pDisplay->Models[0]->BodyGroups.Insert( 1, pProfile->GetSkinNum() );
	pDisplay->Models[0]->Animations.Purge();
	pDisplay->Models[0]->Animations.Insert( "run_aiming_all", 1.0f );

	ASW_Inventory_slot_t iDisplayedSlot = m_bSecondaryWeapon ? ASW_INVENTORY_SLOT_SECONDARY : ASW_INVENTORY_SLOT_PRIMARY;
	if ( CASW_WeaponInfo *pWeaponData = g_ASWEquipmentList.GetWeaponDataFor( g_ASWEquipmentList.GetRegular( GetWeaponIndex( iDisplayedSlot, m_pWeapon[iDisplayedSlot]->m_pWeaponVar->GetInt(), m_pMarine->m_iProfile ) )->m_szEquipClass ) )
	{
		int i = pDisplay->Models.AddToTail( new RD_Swarmopedia::Model() );
		pDisplay->Models[i]->ModelName = pWeaponData->szViewModel;
		pDisplay->Models[i]->Skin = pWeaponData->m_iPlayerModelSkin;
		pDisplay->Models[i]->BoneMerge = 0;
		pDisplay->Models[i]->IsWeapon = true;
	}

	const CASW_EquipItem *pExtraInfo = g_ASWEquipmentList.GetExtra( GetWeaponIndex( ASW_INVENTORY_SLOT_EXTRA, m_pWeapon[ASW_INVENTORY_SLOT_EXTRA]->m_pWeaponVar->GetInt(), m_pMarine->m_iProfile ) );
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

CRD_VGUI_Loadout_Slot::CRD_VGUI_Loadout_Slot( vgui::Panel *parent, const char *panelName, ConVar *pInventoryVar ) :
	BaseClass( parent, panelName, "", this, "Click" )
{
	SetConsoleStylePanel( true );
	SetPaintBackgroundEnabled( false );

	m_pInventoryVar = pInventoryVar;
}

void CRD_VGUI_Loadout_Slot::Paint()
{
	const ReactiveDropInventory::ItemInstance_t *pItem = GetItem();
	if ( !pItem )
		return;

	int w, t;
	GetSize( w, t );

	int ht = t / 2;

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

void CRD_VGUI_Loadout_Slot_Inventory::OnCommand( const char *command )
{
	if ( !V_stricmp( command, "Click" ) )
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
				if ( !V_strcmp( pTab->m_szSlot, m_szSlot ) )
				{
					pCollections->ActivateTab( pTab );
					pFoundTab = pTab;

					break;
				}

				for ( int j = 0; j < NELEMS( ReactiveDropInventory::g_InventorySlotAliases ); j++ )
				{
					if ( !V_strcmp( m_szSlot, ReactiveDropInventory::g_InventorySlotAliases[j][0] ) &&
						!V_strcmp( pTab->m_szSlot, ReactiveDropInventory::g_InventorySlotAliases[j][1] ) )
					{
						pCollections->ActivateTab( pTab );
						pFoundTab = pTab;
						break;
					}
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
	else
	{
		BaseClass::OnCommand( command );
	}
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
	vgui::surface()->DrawSetTexture( HasFocus() ? pProfile->m_nPortraitLitTextureID : pProfile->m_nPortraitTextureID );
	vgui::surface()->DrawTexturedRect( 0, 0, w, t );

	BaseClass::Paint();
}

void CRD_VGUI_Loadout_Slot_Marine::OnCursorEntered()
{
	BaseClass::OnCursorEntered();

	RequestFocus();
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

	if ( HasFocus() )
		vgui::surface()->DrawSetColor( 255, 255, 255, 255 );
	else
		vgui::surface()->DrawSetColor( 64, 96, 128, 255 );
	vgui::surface()->DrawSetTexture( g_ASWEquipmentList.GetEquipIconTexture( m_iSlot != ASW_INVENTORY_SLOT_EXTRA, GetWeaponIndex( m_iSlot, m_pWeaponVar->GetInt(), m_iProfile ) ) );
	vgui::surface()->DrawTexturedRect( 0, 0, w, t );

	BaseClass::Paint();
}

void CRD_VGUI_Loadout_Slot_Weapon::OnCommand( const char *command )
{
	if ( !V_stricmp( command, "Click" ) )
	{
		TabbedGridDetails *pWeaponPanel = new TabbedGridDetails();
		if ( m_iSlot == ASW_INVENTORY_SLOT_PRIMARY )
			pWeaponPanel->SetTitle( "#nb_select_weapon_one", true );
		else if ( m_iSlot == ASW_INVENTORY_SLOT_SECONDARY )
			pWeaponPanel->SetTitle( "#nb_select_weapon_two", true );
		else if ( m_iSlot == ASW_INVENTORY_SLOT_EXTRA )
			pWeaponPanel->SetTitle( "#nb_select_offhand", true );

		CRD_Collection_Tab_Equipment *pTab = new CRD_Collection_Tab_Equipment( pWeaponPanel, m_iSlot == ASW_INVENTORY_SLOT_EXTRA ? "#rd_collection_equipment" : "#rd_collection_weapons", MarineProfileList()->GetProfile( m_iProfile ), m_iSlot );
		pTab->SetBriefing( assert_cast< Loadouts * >( CBaseModPanel::GetSingleton().GetWindow( WT_LOADOUTS ) ), m_iProfile );
		pWeaponPanel->AddTab( pTab );

		pWeaponPanel->ShowFullScreen();
	}

	BaseClass::OnCommand( command );
}
