#include "cbase.h"
#include "clientmode_asw.h"
#include "rd_collection_list.h"
#include "rd_collection_entry.h"
#include "rd_collection_details.h"
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/RichText.h>
#include "rd_inventory_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar rd_briefing_item_details_color1;
extern ConVar rd_briefing_item_details_color2;

CRD_Collection_Details::CRD_Collection_Details( vgui::Panel *pParent, const char *pElementName )
	: BaseClass( pParent, pElementName )
{
	m_pLastEntry = NULL;
}

CRD_Collection_Details::~CRD_Collection_Details()
{
}

void CRD_Collection_Details::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	LoadControlSettings( VarArgs( "Resource/UI/%s.res", GetName() ) );

	BaseClass::ApplySchemeSettings( pScheme );
}

void CRD_Collection_Details::HighlightEntry( CRD_Collection_Entry *pEntry )
{
	m_pLastEntry = pEntry;
}

CRD_Collection_Details_Equipment::CRD_Collection_Details_Equipment( vgui::Panel *pParent, const char *pElementName )
	: BaseClass( pParent, pElementName )
{
}

CRD_Collection_Details_Equipment::~CRD_Collection_Details_Equipment()
{
}

CRD_Collection_Details_Inventory::CRD_Collection_Details_Inventory( vgui::Panel *pParent, const char *pElementName, const char *szSlot )
	: BaseClass( pParent, pElementName )
{
	m_szSlot = szSlot;

	m_pTitle = new vgui::RichText( this, "Title" );
	m_pDescription = new vgui::RichText( this, "Description" );
	m_pIcon = new vgui::ImagePanel( this, "Icon" );

	m_pTitle->SetCursor( vgui::dc_arrow );
	m_pDescription->SetCursor( vgui::dc_arrow );
	m_pTitle->SetPanelInteractive( false );
	m_pDescription->SetPanelInteractive( false );
}

CRD_Collection_Details_Inventory::~CRD_Collection_Details_Inventory()
{
}

void CRD_Collection_Details_Inventory::ApplySchemeSettings( vgui::IScheme *pScheme )
{
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

void CRD_Collection_Details_Inventory::HighlightEntry( CRD_Collection_Entry *pEntry )
{
	BaseClass::HighlightEntry( pEntry );

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

	CRD_Collection_List_Inventory *pList = assert_cast< CRD_Collection_List_Inventory * >( pInvEntry->m_pList );

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

	ReactiveDropInventory::FormatDescription( wszBuf, sizeof( wszBuf ), pDef->BeforeDescription, pList->m_hResult, pInvEntry->m_Index );
	if ( wszBuf[0] )
	{
		m_pDescription->InsertColorChange( rd_briefing_item_details_color2.GetColor() );
		m_pDescription->InsertString( wszBuf );
		m_pDescription->InsertString( L"\n\n" );
	}

	V_UTF8ToUnicode( pDef->Description, wszBuf, sizeof( wszBuf ) );
	m_pDescription->InsertColorChange( rd_briefing_item_details_color1.GetColor() );
	m_pDescription->InsertString( wszBuf );

	ReactiveDropInventory::FormatDescription( wszBuf, sizeof( wszBuf ), pDef->AfterDescription, pList->m_hResult, pInvEntry->m_Index );
	if ( wszBuf[0] )
	{
		m_pDescription->InsertColorChange( rd_briefing_item_details_color2.GetColor() );
		m_pDescription->InsertString( L"\n\n" );
		m_pDescription->InsertString( wszBuf );
	}

	ConVarRef equipID( VarArgs( "rd_equipped_%s", pList->m_szSlot ) );
	if ( pInvEntry->m_Details.m_itemId == strtoull( equipID.GetString(), NULL, 10 ) )
	{
		m_pDescription->InsertColorChange( Color( 255, 255, 255, 255 ) );
		m_pDescription->InsertString( L"\n\n" );
		m_pDescription->InsertString( "#rd_collection_inventory_item_equipped" );
	}

	InvalidateLayout();
}
