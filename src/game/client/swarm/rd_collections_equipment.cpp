#include "cbase.h"
#include "rd_collections.h"
#include "asw_equipment_list.h"
#include "asw_weapon_parse.h"
#include "asw_weapon_shared.h"
#include "asw_ammo_drop_shared.h"
#include "ammodef.h"
#include "asw_util_shared.h"
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Label.h>
#include <vgui/ILocalize.h>
#include "asw_model_panel.h"
#include "asw_marine_profile.h"
#include "rd_swarmopedia.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


#ifdef RD_COLLECTIONS_WEAPONS_ENABLED

extern ConVar asw_unlock_all_weapons;
extern ConVar rd_reduce_motion;

CRD_Collection_Tab_Equipment::CRD_Collection_Tab_Equipment( TabbedGridDetails *parent, const char *szLabel, CASW_Marine_Profile *pProfile, bool bExtra )
	: BaseClass( parent, szLabel )
{
	m_pCollection = NULL;
	m_pProfile = pProfile;
	m_bExtra = bExtra;
}

CRD_Collection_Tab_Equipment::~CRD_Collection_Tab_Equipment()
{
	if ( m_pCollection )
	{
		delete m_pCollection;

		m_pCollection = NULL;
	}
}

TGD_Grid *CRD_Collection_Tab_Equipment::CreateGrid()
{
	TGD_Grid *pGrid = BaseClass::CreateGrid();

	Assert( !m_pCollection );
	m_pCollection = new RD_Swarmopedia::Collection();
	m_pCollection->ReadFromFiles( m_bExtra ? RD_Swarmopedia::Subset::ExtraWeapons : RD_Swarmopedia::Subset::RegularWeapons );

	CASW_EquipmentList *pEquipmentList = ASWEquipmentList();
	Assert( pEquipmentList );

	FOR_EACH_VEC( m_pCollection->Weapons, i )
	{
		const RD_Swarmopedia::Weapon *pWeapon = m_pCollection->Weapons[i];
		if ( pWeapon->Hidden )
		{
			continue;
		}

		int index = m_bExtra ? pEquipmentList->GetExtraIndex( pWeapon->ClassName ) : pEquipmentList->GetRegularIndex( pWeapon->ClassName );

		pGrid->AddEntry( new CRD_Collection_Entry_Equipment( pGrid, m_bExtra ? "CollectionEntryEquipmentExtra" : "CollectionEntryEquipmentRegular", index, pWeapon ) );
	}

	return pGrid;
}

TGD_Details *CRD_Collection_Tab_Equipment::CreateDetails()
{
	return new CRD_Collection_Details_Equipment( this );
}

CRD_Collection_Details_Equipment::CRD_Collection_Details_Equipment( CRD_Collection_Tab_Equipment *parent )
	: BaseClass( parent )
{
	m_pModelPanel = new CRD_Swarmopedia_Model_Panel( this, "ModelPanel" );
	m_pWeaponNameLabel = new vgui::Label( this, "WeaponNameLabel", L"" );
	m_pWeaponDescLabel = new vgui::Label( this, "WeaponDescLabel", L"" );
}

void CRD_Collection_Details_Equipment::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	LoadControlSettings( "Resource/UI/CollectionDetailsEquipment.res" );

	BaseClass::ApplySchemeSettings( pScheme );
}

void CRD_Collection_Details_Equipment::DisplayEntry( TGD_Entry *pEntry )
{
	CRD_Collection_Entry_Equipment *pEquip = assert_cast< CRD_Collection_Entry_Equipment * >( pEntry );
	if ( !pEquip )
	{
		m_pModelPanel->m_bShouldPaint = false;
		m_pModelPanel->SetVisible( false );
		m_pWeaponNameLabel->SetText( L"" );
		m_pWeaponDescLabel->SetText( L"" );
		return;
	}

	// TODO: support arbitrary number of Display/Content per weapon
	Assert( pEquip->m_pWeapon->Display.Count() == 1 );
	Assert( pEquip->m_pWeapon->Content.Count() == 1 );

	if ( pEquip->m_pWeapon->Display.Count() == 1 )
	{
		m_pModelPanel->m_bShouldPaint = true;
		m_pModelPanel->SetVisible( true );
		m_pModelPanel->SetDisplay( pEquip->m_pWeapon->Display[0] );

		m_pWeaponNameLabel->SetText( pEquip->m_pWeapon->Display[0]->Caption );
	}
	else
	{
		m_pModelPanel->m_bShouldPaint = false;
		m_pModelPanel->SetVisible( false );
		m_pWeaponNameLabel->SetText( L"" );
	}

	if ( pEquip->m_pLockedLabel->IsVisible() )
	{
		wchar_t wszRequiredLevel[12]{};
		V_snwprintf( wszRequiredLevel, sizeof( wszRequiredLevel ), L"%d", pEquip->m_pWeapon->RequiredLevel );

		wchar_t wszBuf[1024]{};
		g_pVGuiLocalize->ConstructString( wszBuf, sizeof( wszBuf ),
			g_pVGuiLocalize->Find( "#rd_reach_level_to_unlock_weapon" ), 2,
			wszRequiredLevel, g_pVGuiLocalize->Find( pEquip->m_pWeapon->Name ) );
		m_pWeaponDescLabel->SetText( wszBuf );

		return;
	}

	if ( pEquip->m_pWeapon->Content.Count() == 1 )
	{
		m_pWeaponDescLabel->SetText( pEquip->m_pWeapon->Content[0]->Text );
	}
	else
	{
		m_pWeaponDescLabel->SetText( L"" );
	}
}

CRD_Collection_Entry_Equipment::CRD_Collection_Entry_Equipment( TGD_Grid *parent, const char *panelName, int iEquipIndex, const RD_Swarmopedia::Weapon *pWeapon )
	: BaseClass( parent, panelName )
{
	m_iEquipIndex = iEquipIndex;
	m_pWeapon = pWeapon;

	m_pIcon = new vgui::ImagePanel( this, "Icon" );
	m_pLockedIcon = new vgui::ImagePanel( this, "LockedIcon" );
	m_pLockedOverlay = new vgui::Label( this, "LockedOverlay", "#asw_weapon_details_required_level" );
	m_pLockedLabel = new vgui::Label( this, "LockedLabel", L"" );
	m_pClassIcon = new vgui::ImagePanel( this, "ClassIcon" );
	m_pClassLabel = new vgui::Label( this, "ClassLabel", L"" );
}

void CRD_Collection_Entry_Equipment::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	m_pIcon->SetImage( m_pWeapon->Icon );

	m_pClassIcon->SetVisible( true );
	m_pClassLabel->SetVisible( true );

	switch ( m_pWeapon->RequiredClass )
	{
	case MARINE_CLASS_NCO:
		m_pClassIcon->SetImage( "swarm/ClassIcons/NCOClassIcon" );
		m_pClassLabel->SetText( "#asw_requires_nco" );
		break;
	case MARINE_CLASS_SPECIAL_WEAPONS:
		m_pClassIcon->SetImage( "swarm/ClassIcons/SpecialWeaponsClassIcon" );
		m_pClassLabel->SetText( "#asw_requires_sw" );
		break;
	case MARINE_CLASS_MEDIC:
		m_pClassIcon->SetImage( "swarm/ClassIcons/MedicClassIcon" );
		m_pClassLabel->SetText( "#asw_requires_medic" );
		break;
	case MARINE_CLASS_TECH:
		m_pClassIcon->SetImage( "swarm/ClassIcons/TechClassIcon" );
		m_pClassLabel->SetText( "#asw_requires_tech" );
		break;
	default:
		m_pClassIcon->SetVisible( false );
		m_pClassLabel->SetVisible( false );
		break;
	}

	if ( asw_unlock_all_weapons.GetBool() || UTIL_ASW_CommanderLevelAtLeast( NULL, m_pWeapon->RequiredLevel, -1 ) )
	{
		m_pLockedIcon->SetVisible( false );
		m_pLockedOverlay->SetVisible( false );
		m_pLockedLabel->SetVisible( false );
		m_pIcon->SetVisible( true );
	}
	else
	{
		m_pLockedIcon->SetVisible( true );
		m_pLockedOverlay->SetVisible( true );
		m_pLockedLabel->SetVisible( true );
		m_pIcon->SetVisible( false );
		m_pClassIcon->SetVisible( false );
		m_pClassLabel->SetVisible( false );

		wchar_t wszLevel[12];
		V_snwprintf( wszLevel, sizeof( wszLevel ), L"%d", m_pWeapon->RequiredLevel );
		m_pLockedLabel->SetText( wszLevel );
	}
}

void CRD_Collection_Entry_Equipment::ApplyEntry()
{
	if ( m_pLockedLabel->IsVisible() )
	{
		return;
	}

	TabbedGridDetails *pTGD = m_pParent->m_pParent->m_pParent;
	vgui::Panel *pPanel = pTGD->m_hOverridePanel;
	if ( pPanel )
	{
		CRD_Collection_Panel_Equipment *pEquipmentPanel = dynamic_cast< CRD_Collection_Panel_Equipment * >( pPanel );
		bool bStop = pEquipmentPanel && pEquipmentPanel->m_pWeapon == m_pWeapon;

		pTGD->SetOverridePanel( NULL );
		pPanel->MarkForDeletion();

		if ( bStop )
		{
			return;
		}
	}

	pPanel = new CRD_Collection_Panel_Equipment( pTGD, "EquipmentPanel", m_pWeapon );
	pTGD->SetOverridePanel( pPanel );
}

CRD_Collection_Panel_Equipment::CRD_Collection_Panel_Equipment( vgui::Panel *parent, const char *panelName, const RD_Swarmopedia::Weapon *pWeapon )
	: BaseClass( parent, panelName )
{
	m_pWeapon = pWeapon;
}

void CRD_Collection_Panel_Equipment::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	
	Assert( !"TODO: Equipment collection big view" );
}

#endif
