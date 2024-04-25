#include "cbase.h"
#include "rd_collections.h"
#include "rd_inventory_shared.h"
#include <vgui/ILocalize.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/RichText.h>
#include "MultiFontRichText.h"
#include "fmtstr.h"
#include "filesystem.h"
#include "gameui/swarm/basemodpanel.h"
#include "gameui/swarm/vhybridbutton.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


ConVar rd_briefing_item_details_displaytype( "rd_briefing_item_details_displaytype", "170 170 170 255" );
extern ConVar rd_equipped_medal[RD_STEAM_INVENTORY_NUM_MEDAL_SLOTS];

CRD_Collection_Tab_Inventory::CRD_Collection_Tab_Inventory( TabbedGridDetails *parent, const char *szLabel, const char *szSlot )
	: BaseClass( parent, szLabel )
{
	m_szSlot = szSlot;

	m_hResult = k_SteamInventoryResultInvalid;
	m_bUnavailable = false;
	m_bForceUpdateMessage = true;

	ISteamInventory *pInventory = SteamInventory();
	Assert( pInventory );
	if ( !pInventory || !pInventory->GetAllItems( &m_hResult ) )
	{
		m_hResult = k_SteamInventoryResultInvalid;
		m_bUnavailable = true;
	}
}

CRD_Collection_Tab_Inventory::~CRD_Collection_Tab_Inventory()
{
	if ( ISteamInventory *pInventory = SteamInventory() )
	{
		pInventory->DestroyResult( m_hResult );
	}
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

	if ( m_pGrid->m_Entries.Count() || m_bUnavailable )
	{
		return;
	}

	ISteamInventory *pInventory = SteamInventory();
	if ( !pInventory )
	{
		return;
	}

	EResult eResultStatus = pInventory->GetResultStatus( m_hResult );
	if ( eResultStatus == k_EResultPending )
	{
		return;
	}

	if ( eResultStatus != k_EResultOK )
	{
		m_bUnavailable = true;
		UpdateErrorMessage( m_pGrid );
		return;
	}

	uint32_t nResults = 0;
	if ( !pInventory->GetResultItems( m_hResult, NULL, &nResults ) )
	{
		Assert( !"first call to GetResultItems failed" );
		m_bUnavailable = true;
		UpdateErrorMessage( m_pGrid );
		return;
	}

	CUtlMemory<SteamItemDetails_t> details( 0, nResults );
	if ( !pInventory->GetResultItems( m_hResult, details.Base(), &nResults ) )
	{
		Assert( !"second call to GetResultItems failed" );
		m_bUnavailable = true;
		UpdateErrorMessage( m_pGrid );
		return;
	}

	Assert( int( nResults ) == details.Count() );

	for ( uint32_t i = 0; i < nResults; i++ )
	{
		const ReactiveDropInventory::ItemDef_t *pDef = ReactiveDropInventory::GetItemDef( details[i].m_iDefinition );
		if ( !pDef || !pDef->ItemSlotMatches( m_szSlot ) )
		{
			continue;
		}

		m_pGrid->AddEntry( new CRD_Collection_Entry_Inventory( m_pGrid, "CollectionEntryInventory", m_hResult, i ) );
	}

	UpdateErrorMessage( m_pGrid );

	bool bVisibleBefore = m_pGrid->IsVisible();
	m_pGrid->InvalidateLayout( true, true );
	m_pGrid->SetVisible( bVisibleBefore );
}

void CRD_Collection_Tab_Inventory::UpdateErrorMessage( TGD_Grid *pGrid )
{
	ISteamInventory *pInventory = SteamInventory();
	EResult eResultStatus = pInventory ? pInventory->GetResultStatus( m_hResult ) : k_EResultNoConnection;
	if ( m_bUnavailable || eResultStatus == k_EResultPending )
	{
		if ( const wchar_t *wszMessage = g_pVGuiLocalize->Find( VarArgs( "#rd_collection_inventory_error_%d", eResultStatus ) ) )
		{
			pGrid->SetMessage( wszMessage );
		}
		else
		{
			pGrid->SetMessage( "#rd_collection_inventory_error_generic" );
		}

		if ( m_bUnavailable )
		{
			LoadCachedInventory();
		}
	}
	else
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
}

void CRD_Collection_Tab_Inventory::LoadCachedInventory()
{
	if ( !m_pGrid || m_pGrid->m_Entries.Count() != 0 )
		return;

	// no (offline) Steam API means we can't even do this
	if ( !SteamUser() )
		return;

	if ( !g_pFullFileSystem )
		return;

	CFmtStr szCacheFileName{ "cfg/clienti_%llu.dat", SteamUser()->GetSteamID().ConvertToUint64() };
	CUtlBuffer buf;

	if ( !g_pFullFileSystem->ReadFile( szCacheFileName, "MOD", buf ) )
		return;

	KeyValues::AutoDelete pCache{ "IC" };

	if ( !pCache->ReadAsBinary( buf ) )
		return;

	m_pGrid->SetMouseInputEnabled( false );

	int i = 0;
	FOR_EACH_SUBKEY( pCache, pItem )
	{
		const ReactiveDropInventory::ItemDef_t *pDef = ReactiveDropInventory::GetItemDef( pItem->GetInt( "d" ) );
		if ( !pDef || !pDef->ItemSlotMatches( m_szSlot ) )
			continue;

		m_pGrid->AddEntry( new CRD_Collection_Entry_Inventory( m_pGrid, "CollectionEntryInventory", pItem, i ) );
		i++;
	}
}

void CRD_Collection_Tab_Inventory::ForceRefreshItems( SteamInventoryResult_t hResult )
{
	Assert( m_pGrid );
	if ( !m_pGrid )
		return;

	SteamInventoryResult_t hPreviousResult = m_hResult;
	m_hResult = hResult;
	m_bForceUpdateMessage = true;
	m_bUnavailable = false;

	SteamItemInstanceID_t iSelectedItem = k_SteamItemInstanceIDInvalid;
	if ( m_pGrid->m_hCurrentEntry )
	{
		iSelectedItem = assert_cast< CRD_Collection_Entry_Inventory * >( m_pGrid->m_hCurrentEntry.Get() )->m_Details.ItemID;
	}

	m_pGrid->DeleteAllEntries();

	OnThink();

	m_hResult = hPreviousResult;

	FOR_EACH_VEC( m_pGrid->m_Entries, i )
	{
		if ( assert_cast< CRD_Collection_Entry_Inventory * >( m_pGrid->m_Entries[i] )->m_Details.ItemID == iSelectedItem )
		{
			m_pGrid->m_Entries[i]->m_pFocusHolder->RequestFocus();
			break;
		}
	}
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

	CRD_Collection_Tab_Inventory *pTab = assert_cast< CRD_Collection_Tab_Inventory * >( m_pParent );

	ConVarRef equipID( VarArgs( "rd_equipped_%s", pTab->m_szSlot ) );
	bool bEquipped = strtoull( equipID.GetString(), NULL, 10 ) == details.ItemID;

	for ( int i = 0; i < NELEMS( ReactiveDropInventory::g_InventorySlotAliases ) && !bEquipped; i++ )
	{
		if ( !V_strcmp( ReactiveDropInventory::g_InventorySlotAliases[i][1], pTab->m_szSlot ) )
		{
			ConVarRef equipAliasID( VarArgs( "rd_equipped_%s", ReactiveDropInventory::g_InventorySlotAliases[i][0] ) );
			Assert( equipAliasID.IsValid() );
			bEquipped = strtoull( equipAliasID.GetString(), NULL, 10 ) == details.ItemID;
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

CRD_Collection_Entry_Inventory::CRD_Collection_Entry_Inventory( TGD_Grid *parent, const char *panelName, SteamInventoryResult_t hResult, int index )
	: BaseClass( parent, panelName ),
	m_Details{ hResult, uint32( index ) }
{
	m_pIconBackground = new vgui::Panel( this, "IconBackground" );
	m_pIcon = new vgui::ImagePanel( this, "Icon" );
	m_pEquippedMarker = new vgui::ImagePanel( this, "EquippedMarker" );

	m_Index = index;
}

CRD_Collection_Entry_Inventory::CRD_Collection_Entry_Inventory( TGD_Grid *parent, const char *panelName, KeyValues *pCached, int index )
	: BaseClass( parent, panelName ),
	m_Details{ pCached }
{
	m_pIconBackground = new vgui::Panel( this, "IconBackground" );
	m_pIcon = new vgui::ImagePanel( this, "Icon" );
	m_pEquippedMarker = new vgui::ImagePanel( this, "EquippedMarker" );

	m_Index = index;
}

void CRD_Collection_Entry_Inventory::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	const ReactiveDropInventory::ItemDef_t *pDef = ReactiveDropInventory::GetItemDef( m_Details.ItemDefID );

	m_pIconBackground->SetBgColor( pDef->BackgroundColor );
	m_pIconBackground->SetPaintBackgroundEnabled( true );
	m_pIconBackground->SetPaintBackgroundType( 0 );
	m_pIcon->SetImage( m_Details.GetIcon() );

	ConVarRef equipID( VarArgs( "rd_equipped_%s", GetTab()->m_szSlot ) );
	Assert( equipID.IsValid() );
	bool bEquipped = strtoull( equipID.GetString(), NULL, 10 ) == m_Details.ItemID;

	for ( int i = 0; i < NELEMS( ReactiveDropInventory::g_InventorySlotAliases ) && !bEquipped; i++ )
	{
		if ( !V_strcmp( ReactiveDropInventory::g_InventorySlotAliases[i][1], GetTab()->m_szSlot ) )
		{
			ConVarRef equipAliasID( VarArgs( "rd_equipped_%s", ReactiveDropInventory::g_InventorySlotAliases[i][0] ) );
			Assert( equipAliasID.IsValid() );
			bEquipped = strtoull( equipAliasID.GetString(), NULL, 10 ) == m_Details.ItemID;
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

	// TODO: rd_inventory_verb_accessory
	// TODO: rd_inventory_verb_open

	// TODO: make this selectable
	pModal->AddOption( NULL, "#rd_inventory_verb_delete" );

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

CRD_Collection_Tab_Inventory *CRD_Collection_Entry_Inventory::GetTab()
{
	return assert_cast< CRD_Collection_Tab_Inventory * >( m_pParent->m_pParent );
}
