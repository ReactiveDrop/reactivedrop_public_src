#include "cbase.h"
#include "clientmode_asw.h"
#include "ienginevgui.h"
#include "rd_collection_frame.h"
#include "rd_collection_list.h"
#include "rd_collection_entry.h"
#include "rd_collection_details.h"
#include "nb_header_footer.h"
#include "gameui/swarm/basemodui.h"
#include <vgui_controls/PropertySheet.h>
#include "asw_shareddefs.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CRD_Collection_Frame::CRD_Collection_Frame( vgui::Panel *pParent, const char *pElementName, RD_COLLECTION_TYPE iCollectionType )
	: BaseClass( pParent, pElementName )
{
	SetConsoleStylePanel( true );
	SetProportional( true );
	SetSizeable( false );
	SetCloseButtonVisible( false );

	HScheme scheme = vgui::scheme()->LoadSchemeFromFile( "resource/SwarmSchemeNew.res", "SwarmSchemeNew" );
	SetScheme( scheme );

	m_pHeaderFooter = new CNB_Header_Footer( this, "HeaderFooter" );
	m_pHeaderFooter->SetGradientBarPos( 40, 410 );
	m_pHeaderFooter->SetBackgroundStyle( NB_BACKGROUND_BLUE );
	m_pHeaderFooter->SetTitle( "#rd_collection_title" );

	m_pSheet = new vgui::PropertySheet( this, "TabSheet" );
	m_pSheet->SetKBNavigationEnabled( false );

	for ( int i = 0; i < NELEMS( m_pDetails ); i++ )
	{
		m_pDetails[i] = NULL;
	}

	m_iPrevActiveTab = 0;

	if ( iCollectionType != RD_COLLECTION_TYPE::NUM_TYPES )
	{
		AddTab( iCollectionType );
	}
	else
	{
		AddTab( RD_COLLECTION_TYPE::INVENTORY_MEDALS );
	}
}

CRD_Collection_Frame::CRD_Collection_Frame( vgui::Panel *pParent, const char *pElementName, int iBriefingSlot, int iInventorySlot )
	: ThisClass( pParent, pElementName, iInventorySlot == ASW_INVENTORY_SLOT_EXTRA ? RD_COLLECTION_TYPE::EQUIPMENT_EXTRA : RD_COLLECTION_TYPE::EQUIPMENT_REGULAR )
{
	Assert( iInventorySlot == ASW_INVENTORY_SLOT_PRIMARY || iInventorySlot == ASW_INVENTORY_SLOT_SECONDARY || iInventorySlot == ASW_INVENTORY_SLOT_EXTRA );

	switch ( iInventorySlot )
	{
	case ASW_INVENTORY_SLOT_PRIMARY:
		m_pHeaderFooter->SetTitle( "#nb_select_weapon_one" );
		break;
	case ASW_INVENTORY_SLOT_SECONDARY:
		m_pHeaderFooter->SetTitle( "nb_select_weapon_two" );
		break;
	case ASW_INVENTORY_SLOT_EXTRA:
		m_pHeaderFooter->SetTitle( "nb_select_offhand" );
		break;
	}

	assert_cast< CRD_Collection_List_Equipment * >( m_pSheet->GetPage( 0 ) )->SetBriefingSlot( iBriefingSlot, iInventorySlot );
}

CRD_Collection_Frame::~CRD_Collection_Frame()
{
}

void CRD_Collection_Frame::OnCommand( const char *command )
{
	if ( !V_stricmp( command, "BackButton" ) )
	{
		BaseModUI::CBaseModPanel::GetSingleton().PlayUISound( BaseModUI::UISOUND_BACK );
		MarkForDeletion();
	}
	else if ( !V_stricmp( command, "ApplyCurrentEntry" ) )
	{
		CRD_Collection_Details *pDetails = m_pDetails[m_pSheet->GetActivePageNum()];
		if ( pDetails )
		{
			CRD_Collection_Entry *pEntry = pDetails->m_pLastEntry;
			if ( pEntry && pEntry->m_pFocusHolder )
			{
				pEntry->m_pFocusHolder->OnMousePressed( MOUSE_LEFT );
				pEntry->m_pFocusHolder->OnMouseReleased( MOUSE_LEFT );
			}
		}
	}
	else if ( !V_stricmp( command, "PrevPage" ) )
	{
		if ( m_pSheet->GetActivePageNum() > 0 )
		{
			m_pSheet->SetActivePage( m_pSheet->GetPage( m_pSheet->GetActivePageNum() - 1 ) );
		}
	}
	else if ( !V_stricmp( command, "NextPage" ) )
	{
		if ( m_pSheet->GetActivePageNum() < m_pSheet->GetNumPages() - 1 )
		{
			m_pSheet->SetActivePage( m_pSheet->GetPage( m_pSheet->GetActivePageNum() + 1 ) );
		}
	}
	else if ( !V_stricmp( command, "CyclePage" ) )
	{
		m_pSheet->SetActivePage( m_pSheet->GetPage( ( m_pSheet->GetActivePageNum() + 1 ) % m_pSheet->GetNumPages() ) );
	}
	else
	{
		BaseClass::OnCommand( command );
	}
}

void CRD_Collection_Frame::OnKeyCodeTyped( vgui::KeyCode keycode )
{
	switch ( keycode )
	{
	case KEY_ESCAPE:
		OnCommand( "BackButton" );
		break;
	case KEY_PAGEUP:
		OnCommand( "PrevPage" );
		break;
	case KEY_PAGEDOWN:
		OnCommand( "NextPage" );
		break;
	case KEY_TAB:
		OnCommand( "CyclePage" );
		break;
	default:
		BaseClass::OnKeyCodeTyped( keycode );
		break;
	}
}

void CRD_Collection_Frame::OnKeyCodePressed( vgui::KeyCode keycode )
{
	int lastUser = GetJoystickForCode( keycode );
	BaseModUI::CBaseModPanel::GetSingleton().SetLastActiveUserId( lastUser );

	vgui::KeyCode code = GetBaseButtonCode( keycode );

	switch ( code )
	{
	case KEY_XBUTTON_B:
		OnCommand( "BackButton" );
		break;
	case KEY_XBUTTON_LEFT_SHOULDER:
		OnCommand( "PrevPage" );
		break;
	case KEY_XBUTTON_RIGHT_SHOULDER:
		OnCommand( "NextPage" );
		break;
	default:
		BaseClass::OnKeyCodePressed( keycode );
		break;
	}
}

void CRD_Collection_Frame::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	LoadControlSettings( "Resource/UI/CollectionFrame.res" );

	BaseClass::ApplySchemeSettings( pScheme );

	for ( int i = 0; i < NELEMS( m_pDetails ); i++ )
	{
		if ( m_pDetails[i] )
		{
			m_pDetails[i]->SetVisible( m_pSheet->GetActivePageNum() == i );
		}
	}
}

void CRD_Collection_Frame::OnThink()
{
	BaseClass::OnThink();

	int iActiveTab = m_pSheet->GetActivePageNum();
	if ( iActiveTab != m_iPrevActiveTab )
	{
		if ( m_pDetails[m_iPrevActiveTab] )
			m_pDetails[m_iPrevActiveTab]->SetVisible( false );

		m_iPrevActiveTab = iActiveTab;

		if ( m_pDetails[iActiveTab] )
			m_pDetails[iActiveTab]->SetVisible( true );
	}
}

void CRD_Collection_Frame::AddTab( RD_COLLECTION_TYPE iCollectionType )
{
	const char *szTabName = "";
	CRD_Collection_List *pList = NULL;
	CRD_Collection_Details *pDetails = NULL;

	switch ( iCollectionType )
	{
	case RD_COLLECTION_TYPE::EQUIPMENT_REGULAR:
		szTabName = "#rd_collection_equipment_regular";
		pDetails = new CRD_Collection_Details_Equipment( this, "CollectionDetailsEquipment" );
		pList = new CRD_Collection_List_Equipment( m_pSheet, "CollectionListEquipment", pDetails, false );
		break;
	case RD_COLLECTION_TYPE::EQUIPMENT_EXTRA:
		szTabName = "#rd_collection_equipment_extra";
		pDetails = new CRD_Collection_Details_Equipment( this, "CollectionDetailsEquipment" );
		pList = new CRD_Collection_List_Equipment( m_pSheet, "CollectionListEquipment", pDetails, true );
		break;
	case RD_COLLECTION_TYPE::ALIENS:
		szTabName = "#rd_collection_aliens";
		pDetails = new CRD_Collection_Details_Aliens( this, "CollectionDetailsAliens" );
		pList = new CRD_Collection_List_Aliens( m_pSheet, "CollectionListAliens", pDetails );
		break;
	case RD_COLLECTION_TYPE::MARINES:
		szTabName = "#rd_collection_marines";
		pDetails = new CRD_Collection_Details_Marines( this, "CollectionDetailsMarines" );
		pList = new CRD_Collection_List_Marines( m_pSheet, "CollectionListMarines", pDetails );
		break;
	case RD_COLLECTION_TYPE::INVENTORY_MEDALS:
		szTabName = "#rd_collection_inventory_medals";
		pDetails = new CRD_Collection_Details_Inventory( this, "CollectionDetailsInventory", "medal" );
		pList = new CRD_Collection_List_Inventory( m_pSheet, "CollectionListInventory", pDetails, "medal" );
		break;
	}

	Assert( pList );
	if ( !pList )
	{
		if ( pDetails )
			pDetails->MarkForDeletion();

		return;
	}

	int index = m_pSheet->GetNumPages();
	m_pSheet->AddPage( pList, szTabName );
	m_pDetails[index] = pDetails;

	if ( pDetails )
		pDetails->SetVisible( index == 0 );

	InvalidateLayout();
}

vgui::DHANDLE<CRD_Collection_Frame> g_hCollectionFrame;
static void LaunchCollectionsFrame()
{
	CRD_Collection_Frame *pFrame = g_hCollectionFrame;
	if ( pFrame )
	{
		pFrame->SetVisible( false );
		pFrame->MarkForDeletion();
		g_hCollectionFrame = NULL;
	}

	pFrame = new CRD_Collection_Frame( NULL, "CollectionFrame" );

	if ( engine->IsConnected() )
	{
		pFrame->SetParent( GetClientMode()->GetViewport() );
	}
	else
	{
		vgui::VPANEL rootpanel = enginevgui->GetPanel( PANEL_GAMEUIDLL );
		pFrame->SetParent( rootpanel );
	}

	pFrame->MakeReadyForUse();

	pFrame->InvalidateLayout();
	pFrame->SetVisible( true );

	g_hCollectionFrame = pFrame;
}

static int rd_collections_completion( const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH] )
{
	return 0;
}

CON_COMMAND_F_COMPLETION( rd_collections, "open collections view", FCVAR_CLIENTCMD_CAN_EXECUTE, rd_collections_completion )
{
	if ( args.ArgC() > 1 )
	{
		ConMsg( "Usage: rd_collections\n" );
		return;
	}

	LaunchCollectionsFrame();
}
