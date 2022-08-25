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

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


#ifdef RD_COLLECTIONS_WEAPONS_ENABLED

extern ConVar asw_unlock_all_weapons;
extern ConVar rd_reduce_motion;
ConVar rd_weapon_rotate_speed( "rd_weapon_rotate_speed", "0.3", FCVAR_ARCHIVE, "" );

CRD_Collection_Tab_Equipment::CRD_Collection_Tab_Equipment( TabbedGridDetails *parent, const char *szLabel, bool bExtra )
	: BaseClass( parent, szLabel )
{
	m_bExtra = bExtra;
}

TGD_Grid *CRD_Collection_Tab_Equipment::CreateGrid()
{
	TGD_Grid *pGrid = BaseClass::CreateGrid();

	CASW_EquipmentList *pEquipmentList = ASWEquipmentList();
	Assert( pEquipmentList );

	int nCount = m_bExtra ? pEquipmentList->GetNumExtra( false ) : pEquipmentList->GetNumRegular( false );
	for ( int i = 0; i < nCount; i++ )
	{
		CASW_EquipItem *pEquip = m_bExtra ? pEquipmentList->GetExtra( i ) : pEquipmentList->GetRegular( i );
		pGrid->AddEntry( new CRD_Collection_Entry_Equipment( pGrid, m_bExtra ? "CollectionEntryEquipmentExtra" : "CollectionEntryEquipmentRegular", i, STRING( pEquip->m_EquipClass ) ) );
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
	m_flZOffset = 0.0f;
	m_pModelPanel = new CASW_Model_Panel( this, "ModelPanel" );
	m_pWeaponNameLabel = new vgui::Label( this, "WeaponNameLabel", L"" );
	m_pWeaponDescLabel = new vgui::Label( this, "WeaponDescLabel", L"" );
}

void CRD_Collection_Details_Equipment::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	LoadControlSettings( "Resource/UI/CollectionDetailsEquipment.res" );

	BaseClass::ApplySchemeSettings( pScheme );
}

void CRD_Collection_Details_Equipment::OnThink()
{
	if ( m_pModelPanel->m_bShouldPaint )
	{
		m_pModelPanel->SetCameraForWeapon( m_flZOffset, gpGlobals->curtime * rd_weapon_rotate_speed.GetFloat() );
	}
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

	m_pModelPanel->m_bShouldPaint = true;
	m_pModelPanel->SetVisible( true );
	m_pModelPanel->SetModelByWeapon( pEquip->m_pWeaponInfo );
	m_flZOffset = pEquip->m_pWeaponInfo->m_flModelPanelZOffset;

	m_pWeaponNameLabel->SetText( pEquip->m_pWeaponInfo->szEquipLongName );

	if ( pEquip->m_pLockedLabel->IsVisible() )
	{
		wchar_t wszRequiredLevel[12]{};
		V_snwprintf( wszRequiredLevel, sizeof( wszRequiredLevel ), L"%d", pEquip->m_iRequiredLevel );

		wchar_t wszBuf[1024]{};
		g_pVGuiLocalize->ConstructString( wszBuf, sizeof( wszBuf ),
			g_pVGuiLocalize->Find( "#rd_reach_level_to_unlock_weapon" ), 2,
			wszRequiredLevel, g_pVGuiLocalize->Find( pEquip->m_pWeaponInfo->szPrintName ) );
		m_pWeaponDescLabel->SetText( wszBuf );

		return;
	}

	m_pWeaponDescLabel->SetText( pEquip->m_pWeaponInfo->szEquipDescription1 );
}

CRD_Collection_Entry_Equipment::CRD_Collection_Entry_Equipment( TGD_Grid *parent, const char *panelName, int iEquipIndex, const char *szEquipClass )
	: BaseClass( parent, panelName )
{
	m_iEquipIndex = iEquipIndex;
	m_pWeaponInfo = ASWEquipmentList()->GetWeaponDataFor( szEquipClass );
	Assert( m_pWeaponInfo );
	m_iRequiredLevel = GetWeaponLevelRequirement( m_pWeaponInfo->szClassName ) + 1;

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

	m_pIcon->SetImage( m_pWeaponInfo->szEquipIcon );

	m_pClassIcon->SetVisible( true );
	m_pClassLabel->SetVisible( true );

	if ( m_pWeaponInfo->m_bSapper )
	{
		m_pClassIcon->SetImage( "swarm/ClassIcons/NCOClassIcon" );
		m_pClassLabel->SetText( "#asw_requires_nco" );
	}
	else if ( m_pWeaponInfo->m_bSpecialWeapons )
	{
		m_pClassIcon->SetImage( "swarm/ClassIcons/SpecialWeaponsClassIcon" );
		m_pClassLabel->SetText( "#asw_requires_sw" );
	}
	else if ( m_pWeaponInfo->m_bFirstAid )
	{
		m_pClassIcon->SetImage( "swarm/ClassIcons/MedicClassIcon" );
		m_pClassLabel->SetText( "#asw_requires_medic" );
	}
	else if ( m_pWeaponInfo->m_bTech )
	{
		m_pClassIcon->SetImage( "swarm/ClassIcons/TechClassIcon" );
		m_pClassLabel->SetText( "#asw_requires_tech" );
	}
	else
	{
		m_pClassIcon->SetVisible( false );
		m_pClassLabel->SetVisible( false );
	}

	if ( asw_unlock_all_weapons.GetBool() || UTIL_ASW_CommanderLevelAtLeast( NULL, m_iRequiredLevel, -1 ) )
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
		V_snwprintf( wszLevel, sizeof( wszLevel ), L"%d", m_iRequiredLevel );
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
		bool bStop = pEquipmentPanel && pEquipmentPanel->m_pWeaponInfo == m_pWeaponInfo;

		pTGD->SetOverridePanel( NULL );
		pPanel->MarkForDeletion();

		if ( bStop )
		{
			return;
		}
	}

	pPanel = new CRD_Collection_Panel_Equipment( pTGD, "EquipmentPanel", m_pWeaponInfo );
	pTGD->SetOverridePanel( pPanel );
}

CRD_Collection_Panel_Equipment::CRD_Collection_Panel_Equipment( vgui::Panel *parent, const char *panelName, CASW_WeaponInfo *pWeaponInfo )
	: BaseClass( parent, panelName )
{
	m_pWeaponInfo = pWeaponInfo;
}

void CRD_Collection_Panel_Equipment::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	Assert( !"TODO: Equipment collection big view" );

	// TODO:
	int iMaxAmmo = GetAmmoDef()->MaxCarry( m_pWeaponInfo->iAmmoType, NULL );
	int iMaxAmmo2 = GetAmmoDef()->MaxCarry( m_pWeaponInfo->iAmmo2Type, NULL );
	m_pWeaponInfo->iMaxClip1;
	m_pWeaponInfo->iMaxClip2;
	m_pWeaponInfo->iDefaultClip1;
	m_pWeaponInfo->iDefaultClip2;
	m_pWeaponInfo->m_flBaseDamage;
	m_pWeaponInfo->m_flFireRate;
	m_pWeaponInfo->szAttributesText;
	m_pWeaponInfo->szAltFireText;
	m_pWeaponInfo->m_bUnique;
	m_pWeaponInfo->m_bShowClipsDoubled;
	CASW_Ammo_Drop_Shared::GetAmmoUnitCost( m_pWeaponInfo->iAmmoType );
	CASW_Ammo_Drop_Shared::GetAmmoClipsToGive( m_pWeaponInfo->iAmmoType );
}

#endif
