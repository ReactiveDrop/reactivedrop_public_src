#include "cbase.h"
#include "rd_collections.h"
#include "rd_inventory_shared.h"
#include "rd_crafting_defs.h"
#include <vgui/IInput.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/RichText.h>
#include "MultiFontRichText.h"
#include "fmtstr.h"
#include "filesystem.h"
#include "gameui/swarm/basemodpanel.h"
#include "gameui/swarm/vhybridbutton.h"
#include "asw_util_shared.h"
#include "asw_equipment_list.h"
#include "asw_weapon_shared.h"
#include "vgui_bitmapbutton.h"
#include "nb_button_hold.h"
#include "gameui/swarm/vgenericconfirmation.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


ConVar rd_briefing_item_details_displaytype( "rd_briefing_item_details_displaytype", "170 170 170 255" );
extern ConVar rd_equipped_medal[RD_STEAM_INVENTORY_NUM_MEDAL_SLOTS];

static ReactiveDropInventory::CraftItemType_t s_eDeferredCraftType = ReactiveDropInventory::CRAFT_RECIPE;
static SteamItemDef_t s_iDeferredCraftingRecipe = 0;
static CUtlVector<SteamItemInstanceID_t> s_DeferredCraftingIngredients;
static CUtlVector<uint32> s_DeferredCraftingQuantities;
static SteamItemDef_t s_iDeferredCraftingAccessoryDef = 0;
static SteamItemInstanceID_t s_iDeferredCraftingReplaceItemInstance = k_SteamItemInstanceIDInvalid;

static void DeferredCraftingConfirm()
{
	if ( !s_iDeferredCraftingRecipe )
		return;

	ReactiveDropInventory::PerformCraftingAction( s_eDeferredCraftType, s_iDeferredCraftingRecipe, std::initializer_list<SteamItemInstanceID_t>( s_DeferredCraftingIngredients.Base(), s_DeferredCraftingIngredients.Base() + s_DeferredCraftingIngredients.Count() ), std::initializer_list<uint32>( s_DeferredCraftingQuantities.Base(), s_DeferredCraftingQuantities.Base() + s_DeferredCraftingQuantities.Count() ), s_iDeferredCraftingAccessoryDef, s_iDeferredCraftingReplaceItemInstance );

	s_eDeferredCraftType = ReactiveDropInventory::CRAFT_RECIPE;
	s_iDeferredCraftingRecipe = 0;
	s_DeferredCraftingIngredients.Purge();
	s_DeferredCraftingQuantities.Purge();
	s_iDeferredCraftingAccessoryDef = 0;
	s_iDeferredCraftingReplaceItemInstance = k_SteamItemInstanceIDInvalid;
}
static void DeferredCraftingCancel()
{
	s_eDeferredCraftType = ReactiveDropInventory::CRAFT_RECIPE;
	s_iDeferredCraftingRecipe = 0;
	s_DeferredCraftingIngredients.Purge();
	s_DeferredCraftingQuantities.Purge();
	s_iDeferredCraftingAccessoryDef = 0;
	s_iDeferredCraftingReplaceItemInstance = k_SteamItemInstanceIDInvalid;
}
static void DeferredCraft( ReactiveDropInventory::CraftItemType_t eCraftType, SteamItemDef_t iRecipe, std::initializer_list<SteamItemInstanceID_t> ingredients, std::initializer_list<uint32> quantities, const char *szTitle, const char *szMessage, int nParams = 0, const wchar_t *wszParam1 = NULL, const wchar_t *wszParam2 = NULL, const wchar_t *wszParam3 = NULL, const wchar_t *wszParam4 = NULL )
{
	Assert( s_eDeferredCraftType == ReactiveDropInventory::CRAFT_RECIPE );
	Assert( s_iDeferredCraftingRecipe == 0 );
	Assert( s_DeferredCraftingIngredients.Count() == 0 );
	Assert( s_DeferredCraftingQuantities.Count() == 0 );
	Assert( ingredients.size() == quantities.size() );

	s_eDeferredCraftType = eCraftType;
	s_iDeferredCraftingRecipe = iRecipe;
	s_DeferredCraftingIngredients.CopyArray( ingredients.begin(), ingredients.size() );
	s_DeferredCraftingQuantities.CopyArray( quantities.begin(), quantities.size() );

	BaseModUI::GenericConfirmation::Data_t data;
	data.pWindowTitle = szTitle;
	wchar_t wszMessage[4096];
	const wchar_t *wszMessageTemplate = g_pVGuiLocalize->Find( szMessage );
	if ( wszMessageTemplate )
	{
		g_pVGuiLocalize->ConstructString( wszMessage, sizeof( wszMessage ), wszMessageTemplate, nParams, wszParam1, wszParam2, wszParam3, wszParam4 );
	}
	else
	{
		V_UTF8ToUnicode( szMessage, wszMessage, sizeof( wszMessage ) );
	}
	data.pMessageTextW = wszMessage;
	data.bOkButtonEnabled = true;
	data.bCancelButtonEnabled = true;
	data.pfnOkCallback = DeferredCraftingConfirm;
	data.pfnCancelCallback = DeferredCraftingCancel;

	BaseModUI::CBaseModPanel::GetSingleton().OpenFrontScreen();

	BaseModUI::CBaseModFrame *pMainMenu = BaseModUI::CBaseModPanel::GetSingleton().GetWindow( BaseModUI::WT_MAINMENU );
	Assert( pMainMenu );
	BaseModUI::GenericConfirmation *pConfirm = assert_cast< BaseModUI::GenericConfirmation * >( BaseModUI::CBaseModPanel::GetSingleton().OpenWindow( BaseModUI::WT_GENERICCONFIRMATION, pMainMenu ) );
	pConfirm->SetUsageData( data );
}

CRD_Collection_Tab_Inventory::CRD_Collection_Tab_Inventory( TabbedGridDetails *parent, const char *szLabel, const char *szSlot )
	: BaseClass( parent, szLabel )
{
	m_Slots.CopyAndAddToTail( szSlot );
	m_nLastFullUpdateCount = -1;
	m_bInvertSlotFilter = false;
	m_bUnavailable = false;
	m_bForceUpdateMessage = true;
}

CRD_Collection_Tab_Inventory::~CRD_Collection_Tab_Inventory()
{
}

TGD_Details *CRD_Collection_Tab_Inventory::CreateDetails()
{
	return new CRD_Collection_Details_Inventory( this );
}

void CRD_Collection_Tab_Inventory::OnThink()
{
	BaseClass::OnThink();

	if ( !m_pGrid )
	{
		return;
	}

	if ( m_bForceUpdateMessage )
	{
		UpdateErrorMessage( m_pGrid );
	}

	if ( m_nLastFullUpdateCount != ReactiveDropInventory::g_nFullInventoryUpdates )
	{
		m_nLastFullUpdateCount = ReactiveDropInventory::g_nFullInventoryUpdates;

		SteamItemInstanceID_t iSelectedItemID = k_SteamItemInstanceIDInvalid;
		if ( m_pGrid->m_hCurrentEntry )
		{
			iSelectedItemID = assert_cast< CRD_Collection_Entry_Inventory * >( m_pGrid->m_hCurrentEntry.Get() )->m_Details.ItemID;
		}
		m_pGrid->DeleteAllEntries();

		CRD_Collection_Entry_Inventory *pSelectedBefore = NULL;

		CUtlVector<ReactiveDropInventory::ItemInstance_t> inventory;
		ReactiveDropInventory::GetLocalItemCache( inventory );
		FOR_EACH_VEC( inventory, i )
		{
			const ReactiveDropInventory::ItemDef_t *pDef = ReactiveDropInventory::GetItemDef( inventory[i].ItemDefID );
			if ( !pDef || ( pDef->GameOnly && !pDef->HasInGameDescription ) )
				continue;

			bool bMatch = false;
			FOR_EACH_VEC( m_Slots, j )
			{
				if ( pDef->ItemSlotMatches( m_Slots[j] ) )
				{
					bMatch = true;
					break;
				}
			}

			if ( bMatch == m_bInvertSlotFilter )
				continue;

			CRD_Collection_Entry_Inventory *pEntry = new CRD_Collection_Entry_Inventory( m_pGrid, "CollectionEntryInventory", i, inventory[i] );
			if ( inventory[i].ItemID == iSelectedItemID )
				pSelectedBefore = pEntry;
			m_pGrid->AddEntry( pEntry );
		}

		UpdateErrorMessage( m_pGrid );

		bool bVisibleBefore = m_pGrid->IsVisible();
		m_pGrid->InvalidateLayout( true, true );
		m_pGrid->SetVisible( bVisibleBefore );

		if ( pSelectedBefore && bVisibleBefore )
		{
			pSelectedBefore->m_pFocusHolder->RequestFocus();
		}
	}
}

bool CRD_Collection_Tab_Inventory::ShowsItemsForSlot( const char *szSlot )
{
	FOR_EACH_VEC( m_Slots, i )
	{
		if ( !V_strcmp( szSlot, m_Slots[i] ) )
		{
			return !m_bInvertSlotFilter;
		}

		for ( int j = 0; j < NELEMS( ReactiveDropInventory::g_InventorySlotAliases ); j++ )
		{
			if ( !V_strcmp( szSlot, ReactiveDropInventory::g_InventorySlotAliases[j][0] ) &&
				!V_strcmp( m_Slots[i], ReactiveDropInventory::g_InventorySlotAliases[j][1] ) )
			{
				return !m_bInvertSlotFilter;
			}
		}
	}

	return m_bInvertSlotFilter;
}

void CRD_Collection_Tab_Inventory::UpdateErrorMessage( TGD_Grid *pGrid )
{
	FOR_EACH_VEC( pGrid->m_Entries, i )
	{
		// Using GetNumFrames to signal whether the HTTP request has finished.
		if ( !assert_cast< CRD_Collection_Entry_Inventory * >( pGrid->m_Entries[i] )->m_pIcon->GetNumFrames() )
		{
			pGrid->SetMessage( VarArgs( "#rd_collection_inventory_error_%d", k_EResultPending ) );
			m_bForceUpdateMessage = true;
			return;
		}
	}

	pGrid->SetMessage( L"" );
}

CRD_Collection_Details_Inventory::CRD_Collection_Details_Inventory( CRD_Collection_Tab_Inventory *parent )
	: BaseClass( parent )
{
	m_pTitle = new vgui::RichText( this, "Title" );
	m_pDescription = new vgui::MultiFontRichText( this, "Description" );
	m_pIconBackground = new vgui::Panel( this, "IconBackground" );
	m_pIcon = new vgui::ImagePanel( this, "Icon" );

	m_pTitle->SetCursor( vgui::dc_arrow );
	m_pDescription->SetCursor( vgui::dc_arrow );
	m_pTitle->SetPanelInteractive( false );
	m_pDescription->SetPanelInteractive( false );
}


void CRD_Collection_Details_Inventory::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	LoadControlSettings( "Resource/UI/CollectionDetailsInventory.res" );

	BaseClass::ApplySchemeSettings( pScheme );

	m_pTitle->SetFont( pScheme->GetFont( "DefaultMedium", IsProportional() ) );
}

void CRD_Collection_Details_Inventory::DisplayEntry( TGD_Entry *pEntry )
{
	m_iStyleOverride = -2;

	SetItemStyleOverride( pEntry, -1 );
}

void CRD_Collection_Details_Inventory::SetItemStyleOverride( TGD_Entry *pEntry, int iStyle )
{
	if ( m_iStyleOverride == iStyle )
	{
		return;
	}

	m_iStyleOverride = iStyle;
	SetPaintBackgroundEnabled( false );
	m_pIcon->SetVisible( false );
	m_pTitle->SetText( L"" );
	m_pDescription->SetText( L"" );

	CRD_Collection_Entry_Inventory *pInvEntry = assert_cast< CRD_Collection_Entry_Inventory * >( pEntry );
	if ( !pInvEntry )
	{
		return;
	}

	ReactiveDropInventory::ItemInstance_t details{ pInvEntry->m_Details };
	if ( iStyle != -1 )
	{
		char szStyle[32];
		V_snprintf( szStyle, sizeof( szStyle ), "%d", iStyle );
		details.DynamicProps[details.DynamicProps.AddString( "style" )] = szStyle;
	}

	const ReactiveDropInventory::ItemDef_t *pDef = ReactiveDropInventory::GetItemDef( details.ItemDefID );
	Assert( pDef );
	if ( !pDef )
	{
		return;
	}

	m_pIconBackground->SetBgColor( pDef->BackgroundColor );
	m_pIconBackground->SetPaintBackgroundEnabled( true );
	m_pIconBackground->SetPaintBackgroundType( 2 );

	m_pIcon->SetVisible( true );
	m_pIcon->SetImage( details.GetIcon() );

	wchar_t wszBuf[2048];

	V_UTF8ToUnicode( pDef->Name, wszBuf, sizeof( wszBuf ) );
	m_pTitle->InsertColorChange( pDef->NameColor );
	m_pTitle->InsertString( wszBuf );
	m_pTitle->InsertString( L"\n" );

	V_UTF8ToUnicode( pDef->DisplayType, wszBuf, sizeof( wszBuf ) );
	m_pTitle->InsertColorChange( rd_briefing_item_details_displaytype.GetColor() );
	m_pTitle->InsertString( wszBuf );

	m_pDescription->SetText( "" );
	details.FormatDescription( m_pDescription );

	bool bEquipped = false;
	for ( int i = 0; i < RD_STEAM_INVENTORY_NUM_MEDAL_SLOTS; i++ )
	{
		if ( strtoull( rd_equipped_medal[i].GetString(), NULL, 10 ) == details.ItemID )
		{
			bEquipped = true;
			break;
		}
	}

	if ( bEquipped )
	{
		m_pDescription->InsertColorChange( Color( 255, 255, 255, 255 ) );
		m_pDescription->InsertString( L"\n\n" );
		m_pDescription->InsertString( "#rd_collection_inventory_item_equipped" );
	}

	static_cast< vgui::Panel * >( m_pTitle )->OnThink();
	static_cast< vgui::Panel * >( m_pDescription )->OnThink();
	m_pTitle->SetToFullHeight();

	int x, y0, y1;
	m_pTitle->GetPos( x, y0 );
	m_pDescription->GetPos( x, y1 );
	int iTall = y1 - y0 + m_pDescription->GetTall();
	m_pDescription->SetPos( x, y0 + m_pTitle->GetTall() );
	m_pDescription->SetTall( iTall - m_pTitle->GetTall() );

	InvalidateLayout();
}

class CRD_Collection_Panel_Inventory : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CRD_Collection_Panel_Inventory, vgui::EditablePanel );
public:
	CRD_Collection_Panel_Inventory( vgui::Panel *parent, const char *panelName, CRD_Collection_Entry_Inventory *pEntry ) :
		BaseClass{ parent, panelName }
	{
		m_pEntry = pEntry;

		for ( int i = 0; i < NELEMS( m_pButton ); i++ )
		{
			m_pButton[i] = new BaseModUI::BaseModHybridButton( this, VarArgs( "BtnOption%d", i ), "", this, NULL );
			m_iStyle[i] = -1;
		}

		m_nOptions = 0;
	}

	void AddOption( const char *szCommand, const char *szDescription, int iNumFormatParameters = 0, const wchar_t *wszParam1 = NULL, const wchar_t *wszParam2 = NULL, const wchar_t *wszParam3 = NULL, const wchar_t *wszParam4 = NULL )
	{
		MakeReadyForUse();

		Assert( m_nOptions < NELEMS( m_pButton ) );
		if ( m_nOptions >= NELEMS( m_pButton ) )
			return;

		m_pButton[m_nOptions]->SetCommand( szCommand );
		m_pButton[m_nOptions]->SetEnabled( szCommand != NULL );

		wchar_t wszDescription[1024];
		g_pVGuiLocalize->ConstructString( wszDescription, sizeof( wszDescription ),
			g_pVGuiLocalize->Find( szDescription ), iNumFormatParameters, wszParam1, wszParam2, wszParam3, wszParam4 );

		m_pButton[m_nOptions]->SetText( wszDescription );
		m_pButton[m_nOptions]->SetVisible( true );

		m_nOptions++;
	}

	void SetOptionStyleID( int i )
	{
		Assert( m_nOptions );
		m_iStyle[m_nOptions - 1] = i;
	}

	void ApplySchemeSettings( vgui::IScheme *pScheme ) override
	{
		BaseClass::ApplySchemeSettings( pScheme );

		LoadControlSettings( "Resource/UI/CollectionPanelInventory.res" );
	}

	void PerformLayout() override
	{
		BaseClass::PerformLayout();
	}

	void OnThink() override
	{
		BaseClass::OnThink();

		int iStyleOverride = -1;
		bool bAnyFocus = false;

		for ( int i = 0; i < m_nOptions; i++ )
		{
			if ( m_pButton[i]->GetCurrentState() == BaseModUI::BaseModHybridButton::Focus || m_pButton[i]->GetCurrentState() == BaseModUI::BaseModHybridButton::FocusDisabled )
			{
				iStyleOverride = m_iStyle[i];
				bAnyFocus = true;
				break;
			}
		}

		assert_cast< CRD_Collection_Details_Inventory * >( m_pEntry->GetTab()->m_pDetails )->SetItemStyleOverride( m_pEntry, iStyleOverride );

		if ( !bAnyFocus )
		{
			m_pButton[0]->RequestFocus();
		}
	}

	void OnCommand( const char *command ) override
	{
		assert_cast< TabbedGridDetails * >( GetParent() )->SetOverridePanel( NULL );
		MarkForDeletion();

		m_pEntry->OnCommand( command );
	}

	CRD_Collection_Entry_Inventory *m_pEntry;
	BaseModUI::BaseModHybridButton *m_pButton[10];
	int m_iStyle[10];
	int m_nOptions;
};

class HoverableItemIcon : public vgui::ImagePanel
{
	DECLARE_CLASS_SIMPLE( HoverableItemIcon, vgui::ImagePanel );
public:
	HoverableItemIcon( vgui::Panel *parent, const char *panelName, const ReactiveDropInventory::ItemDef_t *pDef ) :
		BaseClass{ parent, panelName }
	{
		m_Details.m_iItemDefID = pDef->ID;

		SetImage( pDef->AccessoryImage ? pDef->AccessoryImage : pDef->Icon );
		SetShouldScaleImage( true );
	}

	HoverableItemIcon( vgui::Panel *parent, const char *panelName, const ReactiveDropInventory::ItemInstance_t &item ) :
		BaseClass{ parent, panelName }, m_Details{ item }
	{
		SetImage( item.GetIcon() );
		SetShouldScaleImage( true );
	}

	// TODO: the actual "hoverable" part (tooltip)

	CRD_ItemInstance m_Details;
};

#ifdef RD_7A_DROPS
class CRD_Collection_Panel_Inventory_Unbox_Choice : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CRD_Collection_Panel_Inventory_Unbox_Choice, vgui::EditablePanel );
public:
	CRD_Collection_Panel_Inventory_Unbox_Choice( vgui::Panel *parent, const char *panelName, CRD_Collection_Entry_Inventory *pEntry ) :
		BaseClass{ parent, panelName }
	{
		m_pEntry = pEntry;
		m_iChoice = 0;

		m_pLblTitle = new vgui::Label( this, "LblTitle", "" );
		m_pLblFlavor = new vgui::Label( this, "LblFlavor", "" );
		m_pLblOptionNumber = new vgui::Label( this, "LblOptionNumber", "" );
		m_pLblItemName = new vgui::Label( this, "LblItemName", "" );
		m_pLblItemType = new vgui::Label( this, "LblItemType", "" );
		m_pImgItemIcon = new vgui::ImagePanel( this, "ImgItemIcon" );
		m_pLblDescription = new vgui::MultiFontRichText( this, "LblDescription" );
		m_pLblDescription->SetDrawTextOnly();
		m_pLblDescription->SetMouseInputEnabled( false );
		m_pPnlDetails = new vgui::Panel( this, "PnlDetails" );
		m_pBtnPrevious = new CBitmapButton( this, "BtnPrevious", " " );
		m_pBtnPrevious->AddActionSignalTarget( this );
		m_pBtnPrevious->SetCommand( "PreviousSelection" );
		m_pBtnNext = new CBitmapButton( this, "BtnNext", " " );
		m_pBtnNext->AddActionSignalTarget( this );
		m_pBtnNext->SetCommand( "NextSelection" );
		m_pBtnConfirm = new CNB_Button_Hold( this, "BtnConfirm", "#rd_unbox_strange_confirm", this, "ConfirmSelection" );
		m_pBtnConfirm->SetControllerButton( KEY_XBUTTON_X );
		m_pBtnCancel = new CNB_Button( this, "BtnCancel", "#rd_unbox_strange_cancel", this, "CancelSelection" );
		m_pBtnCancel->SetControllerButton( KEY_XBUTTON_B );

		const ReactiveDropInventory::ItemDef_t *pDef = ReactiveDropInventory::GetItemDef( pEntry->m_Details.ItemDefID );
		Assert( pDef );
		if ( !pDef )
		{
			Warning( "Missing item definition %d, but we're in the handler for OpenChoiceBox!\n", pEntry->m_Details.ItemDefID );
			return;
		}

		UtlSymId_t iContainsAny = pDef->Tags.Find( "contains_any" );
		Assert( iContainsAny != UTL_INVAL_SYMBOL );
		if ( iContainsAny == UTL_INVAL_SYMBOL )
		{
			Warning( "Item definition %d missing contains_any tag, but we already verified that it has one by this point!\n", pEntry->m_Details.ItemDefID );
			return;
		}

		const CUtlStringList &containsAny = pDef->Tags[iContainsAny];

		FOR_EACH_VEC( containsAny, i )
		{
			if ( !V_strcmp( containsAny[i], "set_1_strange_weapon" ) || !V_strcmp( containsAny[i], "set_1_strange_equipment" ) )
			{
				m_pLblTitle->SetText( "#rd_unbox_strange_weapon_title" );
				m_pLblFlavor->SetText( "#rd_unbox_strange_weapon_flavor" );
				break;
			}

			if ( !V_strcmp( containsAny[i], "set_1_strange_device" ) )
			{
				m_pLblTitle->SetText( "#rd_unbox_strange_device_title" );
				m_pLblFlavor->SetText( "#rd_unbox_strange_device_flavor" );
				break;
			}

			Assert( !"didn't find a string for this contains_any category" );
		}

		FOR_EACH_VEC( containsAny, i )
		{
			FOR_EACH_VEC( g_RD_Crafting_Contains_Any_Lists, j )
			{
				if ( !V_strcmp( containsAny[i], g_RD_Crafting_Contains_Any_Lists[j].m_szTag ) )
				{
					m_Choices.AddVectorToTail( g_RD_Crafting_Contains_Any_Lists[j].m_ItemDefs );
					break;
				}
			}
		}

		FOR_EACH_VEC( m_Choices, i )
		{
			// pre-fetch icons
			( void )ReactiveDropInventory::GetItemDef( m_Choices[i] );
		}

		MakeReadyForUse();
	}

	void UpdateChoice()
	{
		if ( !m_Choices.Count() )
		{
			// failed to initialize
			MarkForDeletion();
			return;
		}

		const ReactiveDropInventory::ItemDef_t *pDef = ReactiveDropInventory::GetItemDef( m_Choices[m_iChoice] );
		Assert( pDef );
		if ( !pDef )
		{
			// don't let people try to open a box that has invalid contents - this problem will result in an assert on debug builds at startup and a crash here.
			Error( "missing item def %d in container %d", m_Choices[m_iChoice], m_pEntry->m_Details.ItemDefID );
			return;
		}

		ClearInfo();

		wchar_t wszLabel[1024];
		g_pVGuiLocalize->ConstructString( wszLabel, sizeof( wszLabel ), g_pVGuiLocalize->Find( "#rd_unbox_strange_option_number" ), 2, UTIL_RD_CommaNumber( m_iChoice + 1 ), UTIL_RD_CommaNumber( m_Choices.Count() ) );
		m_pLblOptionNumber->SetText( wszLabel );

		V_UTF8ToUnicode( pDef->Name, wszLabel, sizeof( wszLabel ) );
		m_pLblItemName->SetFgColor( pDef->NameColor );
		m_pLblItemName->SetText( wszLabel );
		V_UTF8ToUnicode( pDef->DisplayType, wszLabel, sizeof( wszLabel ) );
		m_pLblItemType->SetText( wszLabel );
		m_pImgItemIcon->SetImage( pDef->Icon );

		m_pLblDescription->SetText( "" );
		CRD_ItemInstance futureItem;
		futureItem.m_iItemDefID = pDef->ID;
		futureItem.FormatDescription( m_pLblDescription );

		if ( pDef->ItemSlotMatches( "weapon" ) )
		{
			CASW_EquipItem *pEquip = g_ASWEquipmentList.GetRegular( pDef->EquipIndex );
			Assert( pEquip );
			if ( pEquip )
			{
				Assert( !pEquip->m_bRequiresInventoryItem || GetWeaponLevelRequirement( pEquip->m_szEquipClass ) == -1 );
				if ( !UTIL_ASW_CommanderLevelAtLeast( NULL, GetWeaponLevelRequirement( pEquip->m_szEquipClass ), -1 ) )
				{
					AddWarning( "#rd_unbox_strange_weapon_warning_cannot_currently_use" );
				}
			}
		}

		if ( pDef->ItemSlotMatches( "extra" ) )
		{
			CASW_EquipItem *pEquip = g_ASWEquipmentList.GetExtra( pDef->EquipIndex );
			Assert( pEquip );
			if ( pEquip )
			{
				Assert( !pEquip->m_bRequiresInventoryItem || GetWeaponLevelRequirement( pEquip->m_szEquipClass ) == -1 );
				if ( !UTIL_ASW_CommanderLevelAtLeast( NULL, GetWeaponLevelRequirement( pEquip->m_szEquipClass ), -1 ) )
				{
					AddWarning( "#rd_unbox_strange_weapon_warning_cannot_currently_use" );
				}
			}
		}

		if ( !pDef->AccessoryTag.IsEmpty() )
		{
			UtlSymId_t iAllowedAccessories = pDef->AllowedTagsFromTools.Find( pDef->AccessoryTag );
			Assert( iAllowedAccessories != UTL_INVAL_SYMBOL );
			if ( iAllowedAccessories != UTL_INVAL_SYMBOL )
			{
				BeginItemIconSection( "#rd_unbox_strange_weapon_compatible_devices" );
				FOR_EACH_VEC( pDef->AllowedTagsFromTools[iAllowedAccessories], i )
				{
					const ReactiveDropInventory::ItemDef_t *pAccessoryDef = ReactiveDropInventory::GetItemDef( V_atoi( pDef->AllowedTagsFromTools[iAllowedAccessories][i] ) );
					Assert( pAccessoryDef );
					if ( pAccessoryDef )
					{
						AddItemDefIcon( pAccessoryDef );
					}
				}
			}
		}

		if ( pDef->IsTagTool )
		{
			Assert( pDef->Tags.GetNumStrings() == 1 && pDef->Tags[0].Count() == 1 && pDef->ID == V_atoi( pDef->Tags[0][0] ) );
			if ( pDef->Tags.GetNumStrings() == 1 && pDef->Tags[0].Count() == 1 )
			{
				CUtlVector<ReactiveDropInventory::ItemInstance_t> compatible;
				ReactiveDropInventory::GetItemsForTagTool( compatible, pDef->Tags.String( 0 ), pDef->Tags[0][0] );
				CUtlVector<ReactiveDropInventory::ItemInstance_t> accessories;
				ReactiveDropInventory::GetItemsForDef( accessories, pDef->ID );
				int nCompatible = 0, nAccessories = 0;
				if ( compatible.Count() )
				{
					BeginItemIconSection( "#rd_unbox_strange_device_compatible_owned_items" );
					FOR_EACH_VEC( compatible, i )
					{
						AddItemInstanceIcon( compatible[i] );
						nCompatible += compatible[i].Quantity;
					}
				}

				FOR_EACH_VEC( accessories, i )
				{
					nAccessories += accessories[i].Quantity;
				}

				if ( nCompatible == 0 )
				{
					AddWarning( "#rd_unbox_strange_device_warning_no_compatible_items" );
				}
				if ( nAccessories && nCompatible <= nAccessories )
				{
					AddWarning( "#rd_unbox_strange_device_warning_already_have_accessories" );
				}
			}
		}
		else
		{
			CUtlVector<ReactiveDropInventory::ItemInstance_t> owned;
			ReactiveDropInventory::GetItemsForDef( owned, pDef->ID );
			if ( owned.Count() )
			{
				AddWarning( "#rd_unbox_strange_weapon_warning_already_owned" );
			}
		}

		InvalidateLayout( true );
	}

	void ApplySchemeSettings( vgui::IScheme *pScheme ) override
	{
		LoadControlSettings( "Resource/UI/CollectionPanelInventoryUnboxChoice.res" );

		BaseClass::ApplySchemeSettings( pScheme );

		UpdateChoice();
	}

	void PerformLayout() override
	{
		BaseClass::PerformLayout();

		int x, y, w, t;
		m_pPnlDetails->GetBounds( x, y, w, t );

		FOR_EACH_VEC( m_Warnings, i )
		{
			m_Warnings[i]->MakeReadyForUse();
			m_Warnings[i]->SetFgColor( Color{ 255, 224, 0, 255 } );
			m_Warnings[i]->SetBounds( x, y, w, YRES( 10 ) );
			m_Warnings[i]->InvalidateLayout( true );
			m_Warnings[i]->OnThink();
			y += m_Warnings[i]->GetTall();
			t -= m_Warnings[i]->GetTall();
			Assert( t >= 0 );
		}

		FOR_EACH_VEC( m_ItemIconSections, i )
		{
			m_ItemIconSections[i].m_pLblSectionTitle->MakeReadyForUse();
			m_ItemIconSections[i].m_pLblSectionTitle->SetFgColor( Color{ 180, 180, 180, 255 } );
			m_ItemIconSections[i].m_pLblSectionTitle->SetBounds( x, y, w, YRES( 10 ) );
			m_ItemIconSections[i].m_pLblSectionTitle->InvalidateLayout( true );
			m_ItemIconSections[i].m_pLblSectionTitle->OnThink();
			y += m_ItemIconSections[i].m_pLblSectionTitle->GetTall();
			t -= m_ItemIconSections[i].m_pLblSectionTitle->GetTall();

			int x1 = 0;
			FOR_EACH_VEC( m_ItemIconSections[i].m_Icons, j )
			{
				if ( x1 >= w - YRES( 32 ) )
				{
					y += YRES( 32 );
					t -= YRES( 32 );
					x1 = 0;
				}
				m_ItemIconSections[i].m_Icons[j]->SetBounds( x + x1, y, YRES( 32 ), YRES( 32 ) );
				x1 += YRES( 32 );
			}
			y += YRES( 32 );
			t -= YRES( 32 );
			Assert( t >= 0 );
		}
	}

	void OnCommand( const char *command ) override
	{
		if ( FStrEq( command, "ConfirmSelection" ) )
		{
			BaseModUI::CBaseModPanel::GetSingleton().PlayUISound( BaseModUI::UISOUND_ACCEPT );

			// un-capture the mouse so players can click on the buttons in the confirm this spawns
			vgui::input()->SetMouseCapture( NULL );

			if ( m_Warnings.Count() )
			{
				DeferredCraft( ReactiveDropInventory::CRAFT_RECIPE, m_Choices[m_iChoice], { m_pEntry->m_Details.ItemID }, { 1 }, "#rd_unbox_strange_warnings_title", "#rd_unbox_strange_warnings_desc" );
			}
			else
			{
				SteamItemDef_t iRecipe = m_Choices[m_iChoice];
				SteamItemInstanceID_t iIngredient = m_pEntry->m_Details.ItemID;

				BaseModUI::CBaseModPanel::GetSingleton().OpenFrontScreen();

				ReactiveDropInventory::PerformCraftingAction( ReactiveDropInventory::CRAFT_RECIPE, iRecipe, { iIngredient }, { 1 } );
			}
		}
		else if ( FStrEq( command, "CancelSelection" ) )
		{
			BaseModUI::CBaseModPanel::GetSingleton().PlayUISound( BaseModUI::UISOUND_BACK );
			m_pEntry->m_pParent->m_pParent->m_pParent->SetOverridePanel( NULL );
			MarkForDeletion();
		}
		else if ( FStrEq( command, "PreviousSelection" ) )
		{
			BaseModUI::CBaseModPanel::GetSingleton().PlayUISound( BaseModUI::UISOUND_ACCEPT );
			m_iChoice--;
			if ( m_iChoice < 0 )
				m_iChoice = m_Choices.Count() - 1;

			UpdateChoice();
		}
		else if ( FStrEq( command, "NextSelection" ) )
		{
			BaseModUI::CBaseModPanel::GetSingleton().PlayUISound( BaseModUI::UISOUND_ACCEPT );
			m_iChoice++;
			if ( m_iChoice >= m_Choices.Count() )
				m_iChoice = 0;

			UpdateChoice();
		}
		else
		{
			BaseClass::OnCommand( command );
		}
	}

	void ClearInfo()
	{
		FOR_EACH_VEC( m_Warnings, i )
		{
			m_Warnings[i]->MarkForDeletion();
		}

		m_Warnings.Purge();

		FOR_EACH_VEC( m_ItemIconSections, i )
		{
			m_ItemIconSections[i].m_pLblSectionTitle->MarkForDeletion();

			FOR_EACH_VEC( m_ItemIconSections[i].m_Icons, j )
			{
				m_ItemIconSections[i].m_Icons[j]->MarkForDeletion();
			}
		}

		m_ItemIconSections.Purge();
	}

	void AddWarning( const char *szLabel )
	{
		vgui::Label *pLabel = new vgui::Label( this, "LblWarning", szLabel );
		m_Warnings.AddToTail( pLabel );

		InvalidateLayout();
	}

	void BeginItemIconSection( const char *szLabel )
	{
		int i = m_ItemIconSections.AddToTail();
		m_ItemIconSections[i].m_pLblSectionTitle = new vgui::Label( this, "LblSectionHeading", szLabel );

		InvalidateLayout();
	}

	void AddItemDefIcon( const ReactiveDropInventory::ItemDef_t *pDef )
	{
		Assert( m_ItemIconSections.Count() );
		if ( !m_ItemIconSections.Count() )
			return;

		HoverableItemIcon *pIcon = new HoverableItemIcon( this, "InfoItemIcon", pDef );
		m_ItemIconSections[m_ItemIconSections.Count() - 1].m_Icons.AddToTail( pIcon );

		InvalidateLayout();
	}

	void AddItemInstanceIcon( const ReactiveDropInventory::ItemInstance_t &item )
	{
		const ReactiveDropInventory::ItemDef_t *pDef = ReactiveDropInventory::GetItemDef( item.ItemDefID );
		Assert( pDef );
		if ( !pDef )
			return;

		HoverableItemIcon *pIcon = new HoverableItemIcon( this, "InfoItemIcon", item );
		m_ItemIconSections[m_ItemIconSections.Count() - 1].m_Icons.AddToTail( pIcon );

		InvalidateLayout();
	}

	CRD_Collection_Entry_Inventory *m_pEntry;
	CUtlVector<SteamItemDef_t> m_Choices;
	int m_iChoice;

	vgui::Label *m_pLblTitle;
	vgui::Label *m_pLblFlavor;
	vgui::Label *m_pLblOptionNumber;
	vgui::Label *m_pLblItemName;
	vgui::Label *m_pLblItemType;
	vgui::ImagePanel *m_pImgItemIcon;
	vgui::MultiFontRichText *m_pLblDescription;
	vgui::Panel *m_pPnlDetails;
	CBitmapButton *m_pBtnPrevious;
	CBitmapButton *m_pBtnNext;
	CNB_Button_Hold *m_pBtnConfirm;
	CNB_Button *m_pBtnCancel;
	CUtlVector<vgui::Label *> m_Warnings;
	struct ItemIconSection_t
	{
		vgui::Label *m_pLblSectionTitle;
		CCopyableUtlVector<HoverableItemIcon *> m_Icons;
	};
	CUtlVector<ItemIconSection_t> m_ItemIconSections;
};
#endif

CRD_Collection_Entry_Inventory::CRD_Collection_Entry_Inventory( TGD_Grid *parent, const char *panelName, int index, const ReactiveDropInventory::ItemInstance_t &details )
	: BaseClass( parent, panelName ),
	m_Details{ details }
{
	m_pIconBackground = new vgui::Panel( this, "IconBackground" );
	m_pIcon = new vgui::ImagePanel( this, "Icon" );
	m_pEquippedMarker = new vgui::ImagePanel( this, "EquippedMarker" );
	m_pLblQuantity = new vgui::Label( this, "LblQuantity", "" );

	m_Index = index;
}

void CRD_Collection_Entry_Inventory::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	const ReactiveDropInventory::ItemDef_t *pDef = ReactiveDropInventory::GetItemDef( m_Details.ItemDefID );
	Assert( pDef );
	if ( !pDef )
		return;

	m_pIconBackground->SetBgColor( pDef->BackgroundColor );
	m_pIconBackground->SetPaintBackgroundEnabled( true );
	m_pIconBackground->SetPaintBackgroundType( 0 );
	m_pIcon->SetImage( m_Details.GetIcon() );
	m_pLblQuantity->SetVisible( pDef->AutoStack || m_Details.Quantity > 1 );
	m_pLblQuantity->SetText( UTIL_RD_CommaNumber( m_Details.Quantity ) );

	SetPostChildPaintEnabled( pDef->HasBorder );
	m_BorderColor = pDef->NameColor;

	bool bEquipped = false;
	for ( int i = 0; i < RD_STEAM_INVENTORY_NUM_MEDAL_SLOTS; i++ )
	{
		if ( strtoull( rd_equipped_medal[i].GetString(), NULL, 10 ) == m_Details.ItemID )
		{
			bEquipped = true;
			break;
		}
	}

	m_pEquippedMarker->SetVisible( bEquipped );
}

void CRD_Collection_Entry_Inventory::ApplyEntry()
{
	const ReactiveDropInventory::ItemDef_t *pDef = ReactiveDropInventory::GetItemDef( m_Details.ItemDefID );
	Assert( pDef );
	if ( !pDef )
	{
		BaseModUI::CBaseModPanel::GetSingleton().PlayUISound( BaseModUI::UISOUND_INVALID );
		return;
	}

	TabbedGridDetails *pTGD = m_pParent->m_pParent->m_pParent;
	CRD_Collection_Panel_Inventory *pModal = new CRD_Collection_Panel_Inventory( pTGD, "OptionsModal", this );

	if ( !V_strcmp( pDef->ItemSlot, "medal" ) )
	{
		// There are three medal slots.
		bool bEquipped = false;
		for ( int i = 0; i < RD_STEAM_INVENTORY_NUM_MEDAL_SLOTS; i++ )
		{
			if ( strtoull( rd_equipped_medal[i].GetString(), NULL, 10 ) == m_Details.ItemID )
			{
				bEquipped = true;
				break;
			}
		}

		if ( bEquipped )
		{
			pModal->AddOption( "UnequipMedal", "#rd_inventory_verb_unequip" );
		}
		else
		{
			pModal->AddOption( "Equip_medal", "#rd_inventory_verb_equip_0" );
			pModal->AddOption( "Equip_medal1", "#rd_inventory_verb_equip_1" );
			pModal->AddOption( "Equip_medal2", "#rd_inventory_verb_equip_2" );
		}
	}

	if ( pDef->StyleNames.Count() )
	{
		int iStyle = m_Details.GetStyle();
		wchar_t wszStyleName[1024];

		FOR_EACH_VEC( pDef->StyleNames, i )
		{
			V_UTF8ToUnicode( pDef->StyleNames[i], wszStyleName, sizeof( wszStyleName ) );

			if ( i == iStyle )
				pModal->AddOption( NULL, "#rd_inventory_verb_style_current", 1, wszStyleName );
			else
				pModal->AddOption( VarArgs( "SetStyle%d", i ), "#rd_inventory_verb_style", 1, wszStyleName );
			pModal->SetOptionStyleID( i );
		}
	}

	UtlSymId_t iAccessoryTags = m_Details.Tags.Find( pDef->AccessoryTag );
	if ( !pDef->AccessoryTag.IsEmpty() && ( iAccessoryTags == UTL_INVAL_SYMBOL || m_Details.Tags[iAccessoryTags].Count() < pDef->AccessoryLimit ) )
	{
		UtlSymId_t iAllowedTagsFromTools = pDef->AllowedTagsFromTools.Find( pDef->AccessoryTag );
		Assert( iAllowedTagsFromTools != UTL_INVAL_SYMBOL );
		if ( iAllowedTagsFromTools != UTL_INVAL_SYMBOL )
		{
			FOR_EACH_VEC( pDef->AllowedTagsFromTools[iAllowedTagsFromTools], i )
			{
				bool bAlreadyHave = false;
				if ( iAccessoryTags != UTL_INVAL_SYMBOL )
				{
					FOR_EACH_VEC( m_Details.Tags[iAccessoryTags], j )
					{
						if ( !V_strcmp( pDef->AllowedTagsFromTools[iAllowedTagsFromTools][i], m_Details.Tags[iAccessoryTags][j] ) )
						{
							bAlreadyHave = true;
							break;
						}
					}
				}

				SteamItemDef_t iAccessoryDef = V_atoi( pDef->AllowedTagsFromTools[iAllowedTagsFromTools][i] );
				Assert( iAccessoryDef );
				if ( !bAlreadyHave && iAccessoryDef )
				{
					CUtlVector<ReactiveDropInventory::ItemInstance_t> items;
					ReactiveDropInventory::GetItemsForDef( items, iAccessoryDef );

					const ReactiveDropInventory::ItemDef_t *pAccessoryDef = ReactiveDropInventory::GetItemDef( iAccessoryDef );
					Assert( pAccessoryDef );
					if ( items.Count() && pAccessoryDef )
					{
						Assert( pAccessoryDef->Tags.Defined( pDef->AccessoryTag ) &&
							pAccessoryDef->Tags[pAccessoryDef->Tags.Find( pDef->AccessoryTag )].Count() == 1 &&
							!V_strcmp( pDef->AllowedTagsFromTools[iAllowedTagsFromTools][i], pAccessoryDef->Tags[pAccessoryDef->Tags.Find( pDef->AccessoryTag )][0] ) );

						wchar_t wszAccessoryName[256];
						V_UTF8ToUnicode( pAccessoryDef->Name, wszAccessoryName, sizeof( wszAccessoryName ) );
						pModal->AddOption( VarArgs( "AttachAccessory%d", iAccessoryDef ), "#rd_inventory_verb_accessory", 1, wszAccessoryName );
					}
				}
			}
		}
	}

	if ( pDef->Tags.Defined( "contains_any" ) )
	{
		pModal->AddOption( "UnboxChoice", "#rd_inventory_verb_open" );
	}

	if ( !pDef->ItemSlot.IsEmpty() )
	{
		// TODO: make this selectable
		pModal->AddOption( NULL, "#rd_inventory_verb_delete" );
	}

	pModal->AddOption( "Back", "#asw_menu_back" );

	pTGD->SetOverridePanel( pModal );
}

void CRD_Collection_Entry_Inventory::OnCommand( const char *command )
{
	if ( const char *szStyleNumber = StringAfterPrefix( command, "SetStyle" ) )
	{
		int iStyle = V_atoi( szStyleNumber );

		ReactiveDropInventory::ChangeItemStyle( m_Details.ItemID, iStyle );
	}
	else if ( const char *szEquipSlot = StringAfterPrefix( command, "Equip_" ) )
	{
		char szItemID[32];
		V_snprintf( szItemID, sizeof( szItemID ), "%llu", m_Details.ItemID );

		ConVarRef equipID{ VarArgs( "rd_equipped_%s", szEquipSlot ) };
		Assert( equipID.IsValid() );
		if ( equipID.IsValid() )
		{
			equipID.SetValue( szItemID );
			engine->ClientCmd( "host_writeconfig\n" );

			BaseModUI::CBaseModPanel::GetSingleton().PlayUISound( BaseModUI::UISOUND_ACCEPT );
		}
	}
	else if ( !V_strcmp( command, "UnequipMedal" ) )
	{
		char szItemID[32];
		V_snprintf( szItemID, sizeof( szItemID ), "%llu", m_Details.ItemID );

		for ( int i = 0; i < RD_STEAM_INVENTORY_NUM_MEDAL_SLOTS; i++ )
		{
			if ( !V_strcmp( rd_equipped_medal[i].GetString(), szItemID ) )
			{
				rd_equipped_medal[i].SetValue( "0" );
				engine->ClientCmd( "host_writeconfig\n" );

				BaseModUI::CBaseModPanel::GetSingleton().PlayUISound( BaseModUI::UISOUND_ACCEPT );
			}
		}
	}
#ifdef RD_7A_DROPS
	else if ( const char *szAccessoryDef = StringAfterPrefix( command, "AttachAccessory" ) )
	{
		SteamItemDef_t iAccessoryDef = V_atoi( szAccessoryDef );

		CUtlVector<ReactiveDropInventory::ItemInstance_t> accessories;
		ReactiveDropInventory::GetItemsForDef( accessories, iAccessoryDef );
		if ( !accessories.Count() )
		{
			Warning( "Tried to attach accessory %d but we don't have any?\n", iAccessoryDef );
			return;
		}

		wchar_t wszItemName[256]{};
		wchar_t wszAccessoryName[256]{};
		wchar_t wszNumSlots[4]{};

		const ReactiveDropInventory::ItemDef_t *pItemDef = ReactiveDropInventory::GetItemDef( m_Details.ItemDefID );
		Assert( pItemDef );
		const ReactiveDropInventory::ItemDef_t *pAccessoryDef = ReactiveDropInventory::GetItemDef( iAccessoryDef );
		Assert( pAccessoryDef );

		if ( pItemDef )
			V_UTF8ToUnicode( pItemDef->Name, wszItemName, sizeof( wszItemName ) );
		if ( pAccessoryDef )
			V_UTF8ToUnicode( pAccessoryDef->Name, wszAccessoryName, sizeof( wszItemName ) );
		if ( pItemDef )
			V_snwprintf( wszNumSlots, NELEMS( wszNumSlots ), L"%d", pItemDef->AccessoryLimit );

		DeferredCraft( ReactiveDropInventory::CRAFT_ACCESSORY, m_Details.ItemDefID, { m_Details.ItemID, accessories[0].ItemID }, { 1, 1 }, "#rd_attach_strange_device_warning_title", "#rd_attach_strange_device_warning_desc", 3, wszItemName, wszAccessoryName, wszNumSlots );
		s_iDeferredCraftingAccessoryDef = iAccessoryDef;
		s_iDeferredCraftingReplaceItemInstance = m_Details.ItemID;
	}
	else if ( !V_strcmp( command, "UnboxChoice" ) )
	{
		TabbedGridDetails *pTGD = m_pParent->m_pParent->m_pParent;
		CRD_Collection_Panel_Inventory_Unbox_Choice *pModal = new CRD_Collection_Panel_Inventory_Unbox_Choice( pTGD, "OptionsModal", this );
		pTGD->SetOverridePanel( pModal );
	}
#endif
	else if ( !V_strcmp( command, "Back" ) )
	{
		// do nothing
	}
	else
	{
		BaseClass::OnCommand( command );

		return;
	}

	m_pParent->InvalidateLayout( true, true );
	m_pFocusHolder->OnSetFocus();
}

void CRD_Collection_Entry_Inventory::PostChildPaint()
{
	BaseClass::PostChildPaint();

	int x, y, wide, tall;
	m_pIconBackground->GetBounds( x, y, wide, tall );
	vgui::surface()->DrawSetColor( m_BorderColor.r(), m_BorderColor.g(), m_BorderColor.b(), m_iBorderAlpha );
	vgui::surface()->DrawFilledRect( x, y, x + m_iBorderThickness, y + tall );
	vgui::surface()->DrawFilledRect( x + wide - m_iBorderThickness, y, x + wide, y + tall );
	vgui::surface()->DrawFilledRect( x, y, x + wide, y + m_iBorderThickness );
	vgui::surface()->DrawFilledRect( x, y + tall - m_iBorderThickness, x + wide, y + tall );
}

CRD_Collection_Tab_Inventory *CRD_Collection_Entry_Inventory::GetTab()
{
	return assert_cast< CRD_Collection_Tab_Inventory * >( m_pParent->m_pParent );
}
