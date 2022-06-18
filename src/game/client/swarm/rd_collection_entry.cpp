#include "cbase.h"
#include "rd_collection_list.h"
#include "rd_collection_entry.h"
#include "rd_collection_details.h"
#include "gameui/swarm/basemodui.h"
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/ScrollBar.h>
#include "rd_inventory_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CRD_Collection_Entry_FocusHolder : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CRD_Collection_Entry_FocusHolder, vgui::EditablePanel );
public:
	CRD_Collection_Entry_FocusHolder( vgui::Panel *pParent, const char *pElementName ) : BaseClass( pParent, pElementName )
	{
		SetConsoleStylePanel( true );

		m_bMousePressed = false;
	}
	virtual ~CRD_Collection_Entry_FocusHolder()
	{
	}

	virtual void NavigateTo()
	{
		BaseClass::NavigateTo();

		BaseModUI::CBaseModPanel::GetSingleton().PlayUISound( BaseModUI::UISOUND_FOCUS );
		RequestFocus();
	}

	virtual void OnSetFocus()
	{
		BaseClass::OnSetFocus();

		CRD_Collection_Entry *pParent = assert_cast< CRD_Collection_Entry * >( GetParent() );
		pParent->m_pHighlight->SetVisible( true );
		pParent->m_pList->m_pDetails->HighlightEntry( pParent );

		int x, y;
		pParent->GetPos( x, y );
		int minScroll = y + GetTall() - pParent->m_pList->m_pScrollBar->GetRangeWindow();
		int maxScroll = y;
		int scroll = pParent->m_pList->m_pScrollBar->GetValue();
		if ( scroll < minScroll )
		{
			pParent->m_pList->m_pScrollBar->SetValue( minScroll );
		}
		else if ( scroll > maxScroll )
		{
			pParent->m_pList->m_pScrollBar->SetValue( maxScroll );
		}
	}

	virtual void OnKillFocus()
	{
		BaseClass::OnKillFocus();

		CRD_Collection_Entry *pParent = assert_cast< CRD_Collection_Entry * >( GetParent() );
		m_bMousePressed = false;
		pParent->m_pHighlight->SetVisible( false );
	}

	virtual void OnCursorMoved( int x, int y )
	{
		if ( GetParent() )
			GetParent()->NavigateToChild( this );
		else
			NavigateTo();
	}

	virtual void OnMousePressed( vgui::MouseCode code )
	{
		if ( code == MOUSE_LEFT && HasFocus() )
		{
			m_bMousePressed = true;
			return;
		}

		BaseClass::OnMousePressed( code );
	}

	virtual void OnMouseReleased( vgui::MouseCode code )
	{
		if ( code == MOUSE_LEFT && m_bMousePressed )
		{
			CRD_Collection_Entry *pParent = assert_cast< CRD_Collection_Entry * >( GetParent() );
			m_bMousePressed = false;
			pParent->Accept();
			return;
		}

		BaseClass::OnMouseReleased( code );
	}

	bool m_bMousePressed;
};

CRD_Collection_Entry::CRD_Collection_Entry( vgui::Panel *pParent, const char *pElementName, CRD_Collection_List *pList )
	: BaseClass( pParent, pElementName )
{
	SetConsoleStylePanel( true );

	m_pList = pList;
	m_pFocusHolder = new CRD_Collection_Entry_FocusHolder( this, "FocusHolder" );
	m_pHighlight = new vgui::Panel( this, "Highlight" );
}

CRD_Collection_Entry::~CRD_Collection_Entry()
{
}

void CRD_Collection_Entry::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	LoadControlSettings( VarArgs( "Resource/UI/%s.res", GetName() ) );

	BaseClass::ApplySchemeSettings( pScheme );
}

void CRD_Collection_Entry::Accept()
{
}

CRD_Collection_Entry_Equipment::CRD_Collection_Entry_Equipment( vgui::Panel *pParent, const char *pElementName, CRD_Collection_List_Equipment *pList )
	: BaseClass( pParent, pElementName, pList )
{
}

CRD_Collection_Entry_Equipment::~CRD_Collection_Entry_Equipment()
{
}

CRD_Collection_Entry_Inventory::CRD_Collection_Entry_Inventory( vgui::Panel *pParent, const char *pElementName, CRD_Collection_List_Inventory *pList, int index, SteamItemDetails_t details )
	: BaseClass( pParent, pElementName, pList )
{
	m_pIcon = new vgui::ImagePanel( this, "Icon" );
	m_pEquippedMarker = new vgui::ImagePanel( this, "EquippedMarker" );

	m_Index = index;
	m_Details = details;
}

CRD_Collection_Entry_Inventory::~CRD_Collection_Entry_Inventory()
{
}

void CRD_Collection_Entry_Inventory::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	CRD_Collection_List_Inventory *pList = assert_cast< CRD_Collection_List_Inventory * >( m_pList );

	m_pIcon->SetImage( ReactiveDropInventory::GetItemDef( m_Details.m_iDefinition )->Icon );

	ConVarRef equipID( VarArgs( "rd_equipped_%s", pList->m_szSlot ) );
	m_pEquippedMarker->SetVisible( strtoull( equipID.GetString(), NULL, 10 ) == m_Details.m_itemId );
}

void CRD_Collection_Entry_Inventory::Accept()
{
	ConVarRef equipID( VarArgs( "rd_equipped_%s", assert_cast< CRD_Collection_List_Inventory * >( m_pList )->m_szSlot ) );
	const char *szValue = VarArgs( "%llu", m_Details.m_itemId );
	equipID.SetValue( V_strcmp( equipID.GetString(), szValue ) ? szValue : "0" );
	engine->ClientCmd( "host_writeconfig\n" );

	m_pList->InvalidateLayout( true, true );
	if ( m_pList->m_pDetails )
	{
		m_pList->m_pDetails->HighlightEntry( this );
	}
}
