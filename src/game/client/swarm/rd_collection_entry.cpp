#include "cbase.h"
#include "rd_collection_list.h"
#include "rd_collection_entry.h"
#include "rd_collection_details.h"
#include "gameui/swarm/basemodui.h"
#include <vgui/ILocalize.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/ScrollBar.h>
#include "rd_inventory_shared.h"
#include "asw_util_shared.h"
#include "asw_weapon_parse.h"
#include "asw_weapon_shared.h"
#include "asw_marine_profile.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar asw_unlock_all_weapons;

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

CRD_Collection_Entry_Equipment::CRD_Collection_Entry_Equipment( vgui::Panel *pParent, const char *pElementName, CRD_Collection_List_Equipment *pList, int index, CASW_WeaponInfo *pWeaponInfo )
	: BaseClass( pParent, pElementName, pList )
{
	m_pIcon = new vgui::ImagePanel( this, "Icon" );
	m_pClassIcon = new vgui::ImagePanel( this, "ClassIcon" );
	m_pClassLabel = new vgui::Label( this, "ClassLabel", L"" );
	m_pLockedIcon = new vgui::ImagePanel( this, "LockedIcon" );
	m_pLockedOverlay = new vgui::Label( this, "LockedOverlay", L"" );
	m_pLockedLabel = new vgui::Label( this, "LockedLabel", L"" );

	m_Index = index;
	m_pWeaponInfo = pWeaponInfo;
	m_nLevelRequirement = GetWeaponLevelRequirement( m_pWeaponInfo->szClassName ) + 1;
}

CRD_Collection_Entry_Equipment::~CRD_Collection_Entry_Equipment()
{
}

void CRD_Collection_Entry_Equipment::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	if ( !asw_unlock_all_weapons.GetBool() && !UTIL_ASW_CommanderLevelAtLeast( NULL, m_nLevelRequirement, -1 ) )
	{
		m_pHighlight->SetSize( 0, 0 );

		m_pLockedIcon->SetVisible( true );
		m_pLockedOverlay->SetVisible( true );
		m_pLockedLabel->SetVisible( true );

		m_pClassIcon->SetVisible( false );
		m_pClassLabel->SetVisible( false );

		m_pLockedLabel->SetText( VarArgs( "%d", m_nLevelRequirement ) );

		return;
	}

	m_pLockedIcon->SetVisible( false );
	m_pLockedOverlay->SetVisible( false );
	m_pLockedLabel->SetVisible( false );

	m_pIcon->SetImage( m_pWeaponInfo->szEquipIcon );

	m_pClassIcon->SetVisible( true );
	m_pClassLabel->SetVisible( true );
	if ( m_pWeaponInfo->m_bTech )
	{
		m_pClassIcon->SetImage( "swarm/ClassIcons/TechClassIcon" );
		m_pClassLabel->SetText( "#asw_requires_tech" );
	}
	else if ( m_pWeaponInfo->m_bFirstAid )
	{
		m_pClassIcon->SetImage( "swarm/ClassIcons/MedicClassIcon" );
		m_pClassLabel->SetText( "#asw_requires_medic" );
	}
	else if ( m_pWeaponInfo->m_bSpecialWeapons )
	{
		m_pClassIcon->SetImage( "swarm/ClassIcons/SpecialWeaponsClassIcon" );
		m_pClassLabel->SetText( "#asw_requires_sw" );
	}
	else if ( m_pWeaponInfo->m_bSapper )
	{
		m_pClassIcon->SetImage( "swarm/ClassIcons/NCOClassIcon" );
		m_pClassLabel->SetText( "#asw_requires_nco" );
	}
	else
	{
		m_pClassIcon->SetVisible( false );
		m_pClassLabel->SetVisible( false );
	}
}

CRD_Collection_Entry_Marines::CRD_Collection_Entry_Marines( vgui::Panel *pParent, const char *pElementName, CRD_Collection_List_Marines *pList, int index, CASW_Marine_Profile *pProfile )
	: BaseClass( pParent, pElementName, pList )
{
	m_pPortrait = new vgui::ImagePanel( this, "Portrait" );
	m_pHighlight->DeletePanel();
	m_pHighlightPortrait = new vgui::ImagePanel( this, "Highlight" );
	m_pHighlight = m_pHighlightPortrait;

	m_Index = index;
	m_pProfile = pProfile;
}

CRD_Collection_Entry_Marines::~CRD_Collection_Entry_Marines()
{
}

void CRD_Collection_Entry_Marines::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	m_pPortrait->SetImage( VarArgs( "briefing/face_%s", m_pProfile->m_PortraitName ) );
	m_pHighlightPortrait->SetImage( VarArgs( "briefing/face_%s_lit", m_pProfile->m_PortraitName ) );
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
	Assert( equipID.IsValid() );
	m_pEquippedMarker->SetVisible( strtoull( equipID.GetString(), NULL, 10 ) == m_Details.m_itemId );
}

void CRD_Collection_Entry_Inventory::Accept()
{
	ConVarRef equipID( VarArgs( "rd_equipped_%s", assert_cast< CRD_Collection_List_Inventory * >( m_pList )->m_szSlot ) );
	Assert( equipID.IsValid() );
	const char *szValue = VarArgs( "%llu", m_Details.m_itemId );
	equipID.SetValue( V_strcmp( equipID.GetString(), szValue ) ? szValue : "0" );
	engine->ClientCmd( "host_writeconfig\n" );

	m_pList->InvalidateLayout( true, true );
	m_pFocusHolder->OnSetFocus();
}
