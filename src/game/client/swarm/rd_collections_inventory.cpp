#include "cbase.h"
#include "rd_collections.h"
#include "rd_inventory_shared.h"
#include <vgui/ILocalize.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/RichText.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


extern ConVar rd_briefing_item_details_color1;
extern ConVar rd_briefing_item_details_color2;

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
		if ( !pDef || pDef->ItemSlot != m_szSlot )
		{
			continue;
		}

		m_pGrid->AddEntry( new CRD_Collection_Entry_Inventory( m_pGrid, "CollectionEntryInventory", i, details[i] ) );
	}

	UpdateErrorMessage( m_pGrid );

	m_pGrid->InvalidateLayout( false, true );
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

CRD_Collection_Details_Inventory::CRD_Collection_Details_Inventory( CRD_Collection_Tab_Inventory *parent )
	: BaseClass( parent )
{
	m_pTitle = new vgui::RichText( this, "Title" );
	m_pDescription = new vgui::RichText( this, "Description" );
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

void CRD_Collection_Details_Inventory::OnThink()
{
	BaseClass::OnThink();

	m_pTitle->SetToFullHeight();

	int x, y0, y1;
	m_pTitle->GetPos( x, y0 );
	m_pDescription->GetPos( x, y1 );
	int iTall = y1 - y0 + m_pDescription->GetTall();
	m_pDescription->SetPos( x, y0 + m_pTitle->GetTall() );
	m_pDescription->SetTall( iTall - m_pTitle->GetTall() );
}

void CRD_Collection_Details_Inventory::DisplayEntry( TGD_Entry *pEntry )
{
	m_pIcon->SetVisible( false );
	m_pTitle->SetText( L"" );
	m_pDescription->SetText( L"" );

	CRD_Collection_Entry_Inventory *pInvEntry = assert_cast< CRD_Collection_Entry_Inventory * >( pEntry );
	if ( !pInvEntry )
	{
		return;
	}

	const ReactiveDropInventory::ItemDef_t *pDef = ReactiveDropInventory::GetItemDef( pInvEntry->m_Details.m_iDefinition );
	Assert( pDef );
	if ( !pDef )
	{
		return;
	}

	CRD_Collection_Tab_Inventory *pTab = assert_cast< CRD_Collection_Tab_Inventory * >( m_pParent );

	m_pIcon->SetVisible( true );
	m_pIcon->SetImage( pDef->Icon );

	wchar_t wszBuf[2048];

	V_UTF8ToUnicode( pDef->Name, wszBuf, sizeof( wszBuf ) );
	m_pTitle->InsertColorChange( Color( 255, 255, 255, 255 ) );
	m_pTitle->InsertString( wszBuf );
	m_pTitle->InsertString( L"\n" );

	V_UTF8ToUnicode( pDef->DisplayType, wszBuf, sizeof( wszBuf ) );
	m_pTitle->InsertColorChange( rd_briefing_item_details_color1.GetColor() );
	m_pTitle->InsertString( wszBuf );

	ReactiveDropInventory::FormatDescription( wszBuf, sizeof( wszBuf ), pDef->BeforeDescription, pTab->m_hResult, pInvEntry->m_Index );
	if ( wszBuf[0] )
	{
		m_pDescription->InsertColorChange( rd_briefing_item_details_color2.GetColor() );
		m_pDescription->InsertString( wszBuf );
		m_pDescription->InsertString( L"\n\n" );
	}

	V_UTF8ToUnicode( pDef->Description, wszBuf, sizeof( wszBuf ) );
	m_pDescription->InsertColorChange( rd_briefing_item_details_color1.GetColor() );
	m_pDescription->InsertString( wszBuf );

	if ( !pDef->AfterDescriptionOnlyMultiStack || pInvEntry->m_Details.m_unQuantity > 1 )
	{
		ReactiveDropInventory::FormatDescription( wszBuf, sizeof( wszBuf ), pDef->AfterDescription, pTab->m_hResult, pInvEntry->m_Index );
		if ( wszBuf[0] )
		{
			m_pDescription->InsertColorChange( rd_briefing_item_details_color2.GetColor() );
			m_pDescription->InsertString( L"\n\n" );
			m_pDescription->InsertString( wszBuf );
		}
	}

	ConVarRef equipID( VarArgs( "rd_equipped_%s", pTab->m_szSlot ) );
	if ( pInvEntry->m_Details.m_itemId == strtoull( equipID.GetString(), NULL, 10 ) )
	{
		m_pDescription->InsertColorChange( Color( 255, 255, 255, 255 ) );
		m_pDescription->InsertString( L"\n\n" );
		m_pDescription->InsertString( "#rd_collection_inventory_item_equipped" );
	}

	InvalidateLayout();
}

CRD_Collection_Entry_Inventory::CRD_Collection_Entry_Inventory( TGD_Grid *parent, const char *panelName, int index, SteamItemDetails_t details )
	: BaseClass( parent, panelName )
{
	m_pIcon = new vgui::ImagePanel( this, "Icon" );
	m_pEquippedMarker = new vgui::ImagePanel( this, "EquippedMarker" );

	m_Index = index;
	m_Details = details;
}

void CRD_Collection_Entry_Inventory::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	m_pIcon->SetImage( ReactiveDropInventory::GetItemDef( m_Details.m_iDefinition )->Icon );

	ConVarRef equipID( VarArgs( "rd_equipped_%s", GetTab()->m_szSlot ) );
	Assert( equipID.IsValid() );
	m_pEquippedMarker->SetVisible( strtoull( equipID.GetString(), NULL, 10 ) == m_Details.m_itemId );
}

void CRD_Collection_Entry_Inventory::ApplyEntry()
{
	ConVarRef equipID( VarArgs( "rd_equipped_%s", GetTab()->m_szSlot ) );
	Assert( equipID.IsValid() );

	const char *szValue = VarArgs( "%llu", m_Details.m_itemId );
	equipID.SetValue( V_strcmp( equipID.GetString(), szValue ) ? szValue : "0" );
	engine->ClientCmd( "host_writeconfig\n" );

	m_pParent->InvalidateLayout( true, true );
	m_pFocusHolder->OnSetFocus();
}

CRD_Collection_Tab_Inventory *CRD_Collection_Entry_Inventory::GetTab()
{
	return assert_cast< CRD_Collection_Tab_Inventory * >( m_pParent->m_pParent );
}
