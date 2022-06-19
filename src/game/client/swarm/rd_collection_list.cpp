#include "cbase.h"
#include "rd_collection_list.h"
#include "rd_collection_entry.h"
#include "rd_collection_details.h"
#include <vgui/ILocalize.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/ScrollBar.h>
#include "asw_equipment_list.h"
#include "asw_marine_profile.h"
#include "rd_inventory_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar rd_collection_equipment_show_hidden( "rd_collection_equipment_show_hidden", "0", FCVAR_NONE );

CRD_Collection_List::CRD_Collection_List( vgui::Panel *pParent, const char *pElementName, CRD_Collection_Details *pDetails )
	: BaseClass( pParent, pElementName )
{
	m_pDetails = pDetails;

	m_pHolder = new vgui::Panel( this, "Holder" );
	m_pErrorMessage = new vgui::Label( this, "ErrorMessage", L"" );
	m_pScrollBar = new vgui::ScrollBar( this, "ScrollBar", true );

	m_pScrollBar->AddActionSignalTarget( this );

	m_nLastX = -1;
	m_nLastY = -1;
}

CRD_Collection_List::~CRD_Collection_List()
{
}

void CRD_Collection_List::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	LoadControlSettings( "Resource/UI/CollectionList.res" );

	BaseClass::ApplySchemeSettings( pScheme );

	m_pScrollBar->UseImages( "scroll_up", "scroll_down", "scroll_line", "scroll_box" );
}

void CRD_Collection_List::PerformLayout()
{
	BaseClass::PerformLayout();

	if ( m_Entries.Count() == 0 )
	{
		m_pScrollBar->SetVisible( false );
		return;
	}

	int totalWide, totalTall, eachWide, eachTall;
	totalWide = m_pHolder->GetWide();
	totalTall = GetTall();

	m_Entries[0]->GetSize( eachWide, eachTall );

	int perRow = MAX( totalWide / eachWide, 1 );

	int totalHeight = ( m_Entries.Count() + perRow - 1 ) / perRow * eachTall;

	m_pHolder->SetTall( totalHeight );

	m_pScrollBar->SetVisible( totalTall < totalHeight );
	m_pScrollBar->SetTall( GetTall() );
	m_pScrollBar->SetButtonPressedScrollValue( totalTall / 2 );
	m_pScrollBar->SetRangeWindow( MIN( totalTall, totalHeight ) );
	m_pScrollBar->SetRange( 0, totalHeight );

	m_pHolder->SetPos( 0, -m_pScrollBar->GetValue() );

	bool bAnyFocus = false;

	FOR_EACH_VEC( m_Entries, i )
	{
		Assert( m_Entries[i]->GetWide() == eachWide );
		Assert( m_Entries[i]->GetTall() == eachTall );

		int col = i % perRow;
		int row = i / perRow;

		m_Entries[i]->SetPos( col * eachWide, row * eachTall );

		constexpr bool bAllowWrapping = true;

		int up = col + ( row - 1 ) * perRow;
		int down = col + ( row + 1 ) * perRow;
		int left = !bAllowWrapping && col == 0 ? -1 : col - 1 + row * perRow;
		int right = !bAllowWrapping && col == perRow - 1 ? -1 : col + 1 + row * perRow;

		m_Entries[i]->m_pFocusHolder->SetNavUp( up >= 0 && up < m_Entries.Count() ? m_Entries[up]->m_pFocusHolder : NULL );
		m_Entries[i]->m_pFocusHolder->SetNavDown( down >= 0 && down < m_Entries.Count() ? m_Entries[down]->m_pFocusHolder : NULL );
		m_Entries[i]->m_pFocusHolder->SetNavLeft( left >= 0 && left < m_Entries.Count() ? m_Entries[left]->m_pFocusHolder : NULL );
		m_Entries[i]->m_pFocusHolder->SetNavRight( right >= 0 && right < m_Entries.Count() ? m_Entries[right]->m_pFocusHolder : NULL );

		if ( m_Entries[i]->m_pFocusHolder->HasFocus() )
		{
			bAnyFocus = true;
		}
	}

	if ( !bAnyFocus )
	{
		if ( m_Entries.Count() > 0 )
		{
			NavigateToChild( m_Entries[0]->m_pFocusHolder );
		}
		else if ( m_pDetails )
		{
			m_pDetails->HighlightEntry( NULL );
		}
	}

	m_nLastX = -1;
	m_nLastY = -1;
}

void CRD_Collection_List::OnMouseWheeled( int delta )
{
	int val = m_pScrollBar->GetValue();
	val -= ( delta * 3 * 5 );
	m_pScrollBar->SetValue( val );
}

void CRD_Collection_List::OnSliderMoved( int position )
{
	InvalidateLayout();
	Repaint();
}

CRD_Collection_List_Equipment::CRD_Collection_List_Equipment( vgui::Panel *pParent, const char *pElementName, CRD_Collection_Details *pDetails, bool bExtra )
	: BaseClass( pParent, pElementName, pDetails )
{
	CASW_EquipmentList *pList = ASWEquipmentList();
	Assert( pList );
	if ( !pList )
		return;

	bool bShowHidden = !engine->IsConnected() && rd_collection_equipment_show_hidden.GetBool();

	if ( bExtra )
	{
		int nExtra = pList->GetNumExtra( bShowHidden );
		for ( int i = 0; i < nExtra; i++ )
		{
			CASW_EquipItem *pExtra = pList->GetExtra( i );
			Assert( pExtra );
			if ( !pExtra )
				continue;

			CASW_WeaponInfo *pWeaponData = pList->GetWeaponDataFor( STRING( pExtra->m_EquipClass ) );
			m_Entries[m_Entries.AddToTail()] = new CRD_Collection_Entry_Equipment( m_pHolder, "CollectionEntryEquipmentExtra", this, i, pWeaponData );
		}
	}
	else
	{
		int nRegular = pList->GetNumRegular( bShowHidden );
		for ( int i = 0; i < nRegular; i++ )
		{
			CASW_EquipItem *pRegular = pList->GetRegular( i );
			Assert( pRegular );
			if ( !pRegular )
				continue;

			CASW_WeaponInfo *pWeaponData = pList->GetWeaponDataFor( STRING( pRegular->m_EquipClass ) );
			m_Entries[m_Entries.AddToTail()] = new CRD_Collection_Entry_Equipment( m_pHolder, "CollectionEntryEquipmentRegular", this, i, pWeaponData );
		}
	}
}

CRD_Collection_List_Equipment::~CRD_Collection_List_Equipment()
{
}

void CRD_Collection_List_Equipment::SetBriefingSlot( int iBriefingSlot, int iInventorySlot )
{
	m_iBriefingSlot = iBriefingSlot;
	m_iInventorySlot = iInventorySlot;
}

CRD_Collection_List_Aliens::CRD_Collection_List_Aliens( vgui::Panel *pParent, const char *pElementName, CRD_Collection_Details *pDetails )
	: BaseClass( pParent, pElementName, pDetails )
{
}

CRD_Collection_List_Aliens::~CRD_Collection_List_Aliens()
{
}

CRD_Collection_List_Marines::CRD_Collection_List_Marines( vgui::Panel *pParent, const char *pElementName, CRD_Collection_Details *pDetails )
	: BaseClass( pParent, pElementName, pDetails )
{
	CASW_Marine_ProfileList *pList = MarineProfileList();
	Assert( pList );
	if ( !pList )
		return;

	for ( int i = 0; i < pList->m_NumProfiles; i++ )
	{
		CASW_Marine_Profile *pProfile = pList->GetProfile( i );
		Assert( pProfile );
		if ( !pProfile )
			continue;

		m_Entries[m_Entries.AddToTail()] = new CRD_Collection_Entry_Marines( m_pHolder, "CollectionEntryMarines", this, i, pProfile );
	}
}

CRD_Collection_List_Marines::~CRD_Collection_List_Marines()
{
}

CRD_Collection_List_Inventory::CRD_Collection_List_Inventory( vgui::Panel *pParent, const char *pElementName, CRD_Collection_Details *pDetails, const char *szSlot )
	: BaseClass( pParent, pElementName, pDetails )
{
	m_szSlot = szSlot;

	m_hResult = k_SteamInventoryResultInvalid;
	m_bUnavailable = false;

	ISteamInventory *pInventory = SteamInventory();
	Assert( pInventory );
	if ( !pInventory || !pInventory->GetAllItems( &m_hResult ) )
	{
		m_hResult = k_SteamInventoryResultInvalid;
		m_bUnavailable = true;
	}
}

CRD_Collection_List_Inventory::~CRD_Collection_List_Inventory()
{
	if ( ISteamInventory *pInventory = SteamInventory() )
		pInventory->DestroyResult( m_hResult );
}

void CRD_Collection_List_Inventory::PerformLayout()
{
	ISteamInventory *pInventory = SteamInventory();
	EResult eResultStatus = pInventory ? pInventory->GetResultStatus( m_hResult ) : k_EResultNoConnection;
	if ( m_bUnavailable || eResultStatus == k_EResultPending )
	{
		m_pErrorMessage->SetVisible( true );
		if ( const wchar_t *wszMessage = g_pVGuiLocalize->Find( VarArgs( "#rd_collection_inventory_error_%d", eResultStatus ) ) )
			m_pErrorMessage->SetText( wszMessage );
		else
			m_pErrorMessage->SetText( "#rd_collection_inventory_error_generic" );
	}
	else
	{
		m_pErrorMessage->SetVisible( false );
	}

	BaseClass::PerformLayout();
}

void CRD_Collection_List_Inventory::OnThink()
{
	BaseClass::OnThink();

	if ( m_bUnavailable || m_Entries.Count() )
		return;

	ISteamInventory *pInventory = SteamInventory();
	if ( !pInventory )
		return;

	EResult eResultStatus = pInventory->GetResultStatus( m_hResult );
	if ( eResultStatus == k_EResultPending )
		return;

	if ( eResultStatus != k_EResultOK )
	{
		m_bUnavailable = true;
		InvalidateLayout();
		return;
	}

	uint32_t nResults = 0;
	if ( !pInventory->GetResultItems( m_hResult, NULL, &nResults ) )
	{
		Assert( !"first call to GetResultItems failed" );
		m_bUnavailable = true;
		InvalidateLayout();
		return;
	}

	CUtlMemory<SteamItemDetails_t> details( 0, nResults );
	if ( !pInventory->GetResultItems( m_hResult, details.Base(), &nResults ) )
	{
		Assert( !"second call to GetResultItems failed" );
		m_bUnavailable = true;
		InvalidateLayout();
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

		m_Entries[m_Entries.AddToTail()] = new CRD_Collection_Entry_Inventory( m_pHolder, "CollectionEntryInventory", this, i, details[i] );
	}

	InvalidateLayout();
}
