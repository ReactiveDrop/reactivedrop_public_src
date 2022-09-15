#include "cbase.h"
#include "rd_collections.h"
#include "rd_swarmopedia.h"
#include "asw_marine_profile.h"
#include "asw_util_shared.h"
#include <vgui/ILocalize.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/TextImage.h>
#include <vgui_controls/Tooltip.h>
#include "gameui/swarm/vgenericpanellist.h"
#include "nb_button.h"
#include "ibriefing.h"
#include "asw_medal_store.h"
#include "asw_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


ConVar rd_swarmopedia_units_preference( "rd_swarmopedia_units_preference", "0", FCVAR_ARCHIVE, "0=hammer, 1=metric, 2=imperial" );
ConVar rd_swarmopedia_units_per_foot( "rd_swarmopedia_units_per_foot", "16", FCVAR_NONE, "recommended: 12 to 16" );
ConVar rd_swarmopedia_units_per_meter( "rd_swarmopedia_units_per_meter", "52.49", FCVAR_NONE, "recommended: 39.37 to 52.49" );

#if defined(RD_COLLECTIONS_WEAPONS_ENABLED) || defined(RD_COLLECTIONS_WEAPONS_CHOOSER)

extern ConVar asw_unlock_all_weapons;
extern ConVar rd_weapons_show_hidden;
extern ConVar rd_weapons_regular_class_unrestricted;
extern ConVar rd_weapons_extra_class_unrestricted;
extern ConVar rd_reduce_motion;

static const char *CantEquipReason( CRD_Collection_Tab_Equipment *pTab, const RD_Swarmopedia::Weapon *pWeapon )
{
	Assert( pTab->m_pBriefing );
	if ( !pTab->m_pBriefing )
	{
		return "";
	}

	Assert( pTab->m_pProfile );
	if ( !pTab->m_pProfile )
	{
		return "";
	}

	if ( !pWeapon->Builtin )
	{
		return "#rd_cant_equip_generic";
	}

	if ( pWeapon->RequiredClass != MARINE_CLASS_UNDEFINED && pWeapon->RequiredClass != pTab->m_pProfile->GetMarineClass() )
	{
		const ConVar &unrestricted = pWeapon->Extra ? rd_weapons_extra_class_unrestricted : rd_weapons_regular_class_unrestricted;
		if ( unrestricted.GetInt() == -1 )
		{
			return "#rd_cant_equip_class";
		}

		if ( unrestricted.GetInt() != -2 )
		{
			bool bFound = false;

			CSplitString split( unrestricted.GetString(), " " );
			FOR_EACH_VEC( split, i )
			{
				if ( atoi( split[i] ) == pWeapon->EquipIndex )
				{
					bFound = true;
					break;
				}
			}

			if ( !bFound )
			{
				return "#rd_cant_equip_class";
			}
		}
	}

	if ( !pTab->m_pBriefing->IsWeaponUnlocked( pWeapon->ClassName ) )
	{
		return "#rd_cant_equip_locked";
	}

	if ( pWeapon->Unique )
	{
		Assert( pTab->m_nInventorySlot == ASW_INVENTORY_SLOT_PRIMARY || pTab->m_nInventorySlot == ASW_INVENTORY_SLOT_SECONDARY );
		int iOtherWeapon = pTab->m_pBriefing->GetMarineSelectedWeapon( pTab->m_nLobbySlot, pTab->m_nInventorySlot == ASW_INVENTORY_SLOT_PRIMARY ? ASW_INVENTORY_SLOT_SECONDARY : ASW_INVENTORY_SLOT_PRIMARY );
		if ( pWeapon->EquipIndex == iOtherWeapon )
		{
			return "#rd_cant_equip_unique";
		}
	}

	if ( ASWGameRules() && ASWGameRules()->ApplyWeaponSelectionRules( pTab->m_nInventorySlot, pWeapon->EquipIndex ) != pWeapon->EquipIndex )
	{
		return "#rd_cant_equip_game_rules";
	}

	return NULL;
}

CRD_Collection_Tab_Equipment::CRD_Collection_Tab_Equipment( TabbedGridDetails *parent, const char *szLabel, CASW_Marine_Profile *pProfile, int nInventorySlot )
	: BaseClass( parent, szLabel )
{
	m_pCollection = NULL;
	m_pProfile = pProfile;
	m_nInventorySlot = nInventorySlot;

	m_pBriefing = NULL;
	m_nLobbySlot = -1;
}

CRD_Collection_Tab_Equipment::~CRD_Collection_Tab_Equipment()
{
	if ( m_pCollection )
	{
		delete m_pCollection;

		m_pCollection = NULL;
	}

	if ( m_pBriefing )
	{
		m_pBriefing->SetChangingWeaponSlot( -1, 0 );
	}
}

TGD_Grid *CRD_Collection_Tab_Equipment::CreateGrid()
{
	TGD_Grid *pGrid = BaseClass::CreateGrid();

	Assert( !m_pCollection );
	m_pCollection = new RD_Swarmopedia::Collection();
	m_pCollection->ReadFromFiles( m_nInventorySlot == ASW_INVENTORY_SLOT_EXTRA ? RD_Swarmopedia::Subset::ExtraWeapons : RD_Swarmopedia::Subset::RegularWeapons );

	int iEquippedIndex = m_pBriefing ? m_pBriefing->GetMarineSelectedWeapon( m_nLobbySlot, m_nInventorySlot ) : -1;

	FOR_EACH_VEC( m_pCollection->Weapons, i )
	{
		const RD_Swarmopedia::Weapon *pWeapon = m_pCollection->Weapons[i];
		if ( pWeapon->Hidden && !rd_weapons_show_hidden.GetBool() )
		{
			continue;
		}

		pGrid->AddEntry( new CRD_Collection_Entry_Equipment( pGrid, pWeapon->Extra ? "CollectionEntryEquipmentExtra" : "CollectionEntryEquipmentRegular", pWeapon ) );

		if ( pWeapon->EquipIndex != -1 && pWeapon->EquipIndex == iEquippedIndex )
		{
			pGrid->m_iLastFocus = pGrid->m_Entries.Count() - 1;
		}
	}

	return pGrid;
}

TGD_Details *CRD_Collection_Tab_Equipment::CreateDetails()
{
	return new CRD_Collection_Details_Equipment( this );
}

void CRD_Collection_Tab_Equipment::SetBriefing( IBriefing *pBriefing, int nLobbySlot )
{
	Assert( m_pProfile );

	m_pBriefing = pBriefing;
	m_nLobbySlot = nLobbySlot;

	m_pBriefing->SetChangingWeaponSlot( m_nLobbySlot, 2 + m_nInventorySlot );
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

		m_pWeaponNameLabel->GetTextImage()->ResizeImageToContentMaxWidth( m_pWeaponNameLabel->GetWide() );
		int iTall = m_pWeaponNameLabel->GetTextImage()->GetTall();
		int iDiff = iTall - m_pWeaponNameLabel->GetTall();
		m_pWeaponNameLabel->SetTall( iTall );
		int x, y;
		m_pWeaponDescLabel->GetPos( x, y );
		y += iDiff;
		m_pWeaponDescLabel->SetPos( x, y );
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

CRD_Collection_Entry_Equipment::CRD_Collection_Entry_Equipment( TGD_Grid *parent, const char *panelName, const RD_Swarmopedia::Weapon *pWeapon )
	: BaseClass( parent, panelName )
{
	m_pWeapon = pWeapon;

	m_pIcon = new vgui::ImagePanel( this, "Icon" );
	m_pLockedIcon = new vgui::ImagePanel( this, "LockedIcon" );
	m_pLockedOverlay = new vgui::Label( this, "LockedOverlay", "#asw_weapon_details_required_level" );
	m_pLockedLabel = new vgui::Label( this, "LockedLabel", L"" );
	m_pCantEquipLabel = new vgui::Label( this, "CantEquipLabel", L"" );
	m_pNewLabel = new vgui::Label( this, "NewLabel", "#new_weapon" );
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

	CRD_Collection_Tab_Equipment *pTab = assert_cast< CRD_Collection_Tab_Equipment * >( m_pParent->m_pParent );

	bool bLevelLocked = !asw_unlock_all_weapons.GetBool() && !UTIL_ASW_CommanderLevelAtLeast( NULL, m_pWeapon->RequiredLevel, -1 );
	if ( pTab->m_pBriefing && m_pWeapon->Builtin )
	{
		bLevelLocked = !pTab->m_pBriefing->IsWeaponUnlocked( m_pWeapon->ClassName );
	}

	const char *szCantEquipReason = pTab->m_pBriefing ? CantEquipReason( pTab, m_pWeapon ) : NULL;
	if ( szCantEquipReason )
	{
		m_pCantEquipLabel->SetText( szCantEquipReason );
		m_pCantEquipLabel->SetVisible( true );
	}
	else
	{
		m_pCantEquipLabel->SetVisible( false );
	}

	if ( bLevelLocked )
	{
		m_pLockedIcon->SetVisible( true );
		m_pLockedOverlay->SetVisible( true );
		m_pLockedLabel->SetVisible( true );
		m_pIcon->SetVisible( false );
		m_pClassIcon->SetVisible( false );
		m_pClassLabel->SetVisible( false );
		m_pNewLabel->SetVisible( false );
		m_pCantEquipLabel->SetVisible( false );

		wchar_t wszLevel[12];
		V_snwprintf( wszLevel, sizeof( wszLevel ), L"%d", m_pWeapon->RequiredLevel );
		m_pLockedLabel->SetText( wszLevel );
	}
	else
	{
		m_pLockedIcon->SetVisible( false );
		m_pLockedOverlay->SetVisible( false );
		m_pLockedLabel->SetVisible( false );
		m_pIcon->SetVisible( true );

		C_ASW_Medal_Store *pMedalStore = GetMedalStore();
		m_pNewLabel->SetVisible( pMedalStore && pMedalStore->IsWeaponNew( m_pWeapon->Extra, m_pWeapon->EquipIndex ) );
	}
}

void CRD_Collection_Entry_Equipment::ApplyEntry()
{
	if ( m_pLockedLabel->IsVisible() )
	{
		return;
	}

	CRD_Collection_Tab_Equipment *pTab = assert_cast< CRD_Collection_Tab_Equipment * >( m_pParent->m_pParent );
	TabbedGridDetails *pTGD = pTab->m_pParent;
	vgui::Panel *pPanel = pTGD->m_hOverridePanel;
	Assert( !pPanel );
	if ( pPanel )
	{
		pPanel->MarkForDeletion();
	}

	pTGD->SetOverridePanel( new CRD_Collection_Panel_Equipment( pTGD, "EquipmentPanel", pTab, m_pWeapon ) );
}

class CRD_Equipment_WeaponFact : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CRD_Equipment_WeaponFact, vgui::EditablePanel );
public:
	CRD_Equipment_WeaponFact( vgui::Panel *parent, const char *panelName, CRD_Collection_Tab_Equipment *pTab, const RD_Swarmopedia::WeaponFact *pFact );

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme ) override;

	vgui::ImagePanel *m_pIcon;
	vgui::ImagePanel *m_pSkillIcon;
	vgui::Label *m_pLblName;
	vgui::Label *m_pLblValue;

	CRD_Collection_Tab_Equipment *m_pTab;
	const RD_Swarmopedia::WeaponFact *m_pFact;
};

CRD_Equipment_WeaponFact::CRD_Equipment_WeaponFact( vgui::Panel *parent, const char *panelName, CRD_Collection_Tab_Equipment *pTab, const RD_Swarmopedia::WeaponFact *pFact ) :
	BaseClass( parent, panelName )
{
	m_pTab = pTab;
	m_pFact = pFact;

	m_pIcon = new vgui::ImagePanel( this, "Icon" );
	m_pSkillIcon = new vgui::ImagePanel( this, "SkillIcon" );
	m_pLblName = new vgui::Label( this, "LblName", L"" );
	m_pLblValue = new vgui::Label( this, "LblValue", L"" );
}

void CRD_Equipment_WeaponFact::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	LoadControlSettings( "Resource/UI/CollectionPanelEquipmentWeaponFact.res" );

	BaseClass::ApplySchemeSettings( pScheme );

	m_pSkillIcon->SetVisible( false );

	if ( !m_pFact )
	{
		m_pIcon->SetVisible( false );
		m_pLblName->SetVisible( false );
		m_pLblValue->SetVisible( false );
		SetTall( GetTall() * 0.3f );

		return;
	}

	using Type_T = RD_Swarmopedia::WeaponFact::Type_T;

	const char *szIcon = "";
	const char *szCaption = "";
	const char *szValue = NULL;
	bool bHasValue = true;
	bool bIsHammerUnits = false;
	bool bIgnoreCustomCaption = false;
	bool bConVarIsOverride = false;
	bool bShowReciprocal = false;

	switch ( m_pFact->Type )
	{
	case Type_T::Generic:
		szIcon = "swarm/swarmopedia/fact/generic";
		bHasValue = false;
		break;
	case Type_T::Numeric:
		szIcon = "swarm/swarmopedia/fact/generic";
		break;
	case Type_T::HammerUnits:
		szIcon = "swarm/swarmopedia/fact/generic";
		bIsHammerUnits = true;
		break;
	case Type_T::ShotgunPellets:
		szIcon = "swarm/swarmopedia/fact/shotgun_pellets";
		szCaption = "#rd_weapon_fact_shotgun_pellets";
		break;
	case Type_T::DamagePerShot:
		szIcon = "swarm/swarmopedia/fact/damage";
		szCaption = "#rd_weapon_fact_damage_per_shot";
		bConVarIsOverride = true;
		break;
	case Type_T::LargeAlienDamageScale:
		szIcon = "swarm/swarmopedia/fact/large_alien_damage";
		szCaption = "#rd_weapon_fact_large_alien_damage_scale";
		break;
	case Type_T::BulletSpread:
		szIcon = "swarm/swarmopedia/fact/bullet_spread";
		szCaption = m_pFact->Flattened ? "#rd_weapon_fact_bullet_spread_degrees_flattened" : "#rd_weapon_fact_bullet_spread_degrees";
		break;
	case Type_T::Piercing:
		szIcon = "swarm/swarmopedia/fact/piercing";
		szCaption = "#rd_weapon_fact_piercing";
		break;
	case Type_T::FireRate:
		szIcon = "swarm/swarmopedia/fact/fire_rate";
		szCaption = "#rd_weapon_fact_fire_rate";
		bShowReciprocal = true;
		break;
	case Type_T::Ammo:
		szIcon = "swarm/swarmopedia/fact/ammo";
		szCaption = "#rd_weapon_fact_ammo";
		break;
	case Type_T::Recharges:
		szIcon = "swarm/swarmopedia/fact/recharges";
		szCaption = "#rd_weapon_fact_recharges";
		break;
	case Type_T::Secondary:
		szIcon = "swarm/swarmopedia/fact/secondary";
		szCaption = "#rd_weapon_fact_secondary";
		if ( !m_pFact->Caption.IsEmpty() )
		{
			szValue = m_pFact->Caption;
		}

		bIgnoreCustomCaption = true;
		break;
	case Type_T::RequirementLevel:
		szIcon = "swarm/swarmopedia/fact/level";
		szCaption = "#rd_weapon_fact_requires_level";
		break;
	case Type_T::RequirementClass:
		szCaption = "#rd_weapon_fact_requires_class";

		switch ( m_pFact->Class )
		{
		case MARINE_CLASS_NCO:
			szIcon = "swarm/ClassIcons/NCOClassIcon";
			szValue = "#asw_requires_nco";
			break;
		case MARINE_CLASS_SPECIAL_WEAPONS:
			szIcon = "swarm/ClassIcons/SpecialWeaponsClassIcon";
			szValue = "#asw_requires_sw";
			break;
		case MARINE_CLASS_MEDIC:
			szIcon = "swarm/ClassIcons/MedicClassIcon";
			szValue = "#asw_requires_medic";
			break;
		case MARINE_CLASS_TECH:
			szIcon = "swarm/ClassIcons/TechClassIcon";
			szValue = "#asw_requires_tech";
			break;
		}

		break;
	}

	if ( !m_pFact->Icon.IsEmpty() )
	{
		szIcon = m_pFact->Icon;
	}

	if ( !bIgnoreCustomCaption && !m_pFact->Caption.IsEmpty() )
	{
		szCaption = m_pFact->Caption;
	}

	m_pIcon->SetImage( szIcon );
	m_pLblName->SetText( szCaption );

	if ( !bHasValue )
	{
		m_pLblName->SetWide( m_pLblName->GetWide() + m_pLblValue->GetWide() );

		// For some reason, without this, text that would wrap is forced to the top of the text area.
		m_pLblName->InvalidateLayout( true, true );

		m_pLblName->SetFgColor( m_pLblValue->GetFgColor() );
		m_pLblValue->SetVisible( false );

		return;
	}

	if ( szValue )
	{
		m_pLblValue->SetText( szValue );
		return;
	}

	const wchar_t *wszBefore = g_pVGuiLocalize->Find( VarArgs( "%s_before", szCaption ) );
	const wchar_t *wszAfter = g_pVGuiLocalize->Find( VarArgs( "%s_after", szCaption ) );
	if ( !wszBefore )
		wszBefore = L"";
	if ( !wszAfter )
		wszAfter = L"";

	float flBaseValue = m_pFact->Base;
	if ( !m_pFact->CVar.IsEmpty() )
	{
		ConVarRef var( m_pFact->CVar );
		if ( !bConVarIsOverride )
		{
			flBaseValue += var.GetFloat();
		}
		else if ( var.GetFloat() > 0 )
		{
			flBaseValue = var.GetFloat();
		}
	}

	flBaseValue *= m_pFact->BaseMultiplier;
	FOR_EACH_VEC( m_pFact->BaseMultiplierCVar, i )
	{
		ConVarRef var( m_pFact->BaseMultiplierCVar[i] );
		flBaseValue *= var.GetFloat();
	}
	FOR_EACH_VEC( m_pFact->BaseDivisorCVar, i )
	{
		ConVarRef var( m_pFact->BaseDivisorCVar[i] );
		flBaseValue /= var.GetFloat();
	}

	float flSkillValue = 0.0f;
	if ( m_pFact->Skill != ASW_MARINE_SKILL_INVALID )
	{
		m_pSkillIcon->SetImage( MarineSkills()->GetSkillImage( m_pFact->Skill ) );
		m_pSkillIcon->SetVisible( true );

		if ( m_pTab->m_pProfile )
		{
			flSkillValue = MarineSkills()->GetSkillBasedValue( m_pTab->m_pProfile, m_pFact->Skill, m_pFact->SubSkill );
		}
		else
		{
			// TODO: better than this
			int iMidSkill = ( MarineSkills()->GetMaxSkillPoints( m_pFact->Skill ) - 1 ) / 2 + 1;
			flSkillValue = MarineSkills()->GetSkillBasedValue( NULL, m_pFact->Skill, m_pFact->SubSkill, iMidSkill );
		}
	}

	flSkillValue *= m_pFact->SkillMultiplier;
	FOR_EACH_VEC( m_pFact->SkillMultiplierCVar, i )
	{
		ConVarRef var( m_pFact->SkillMultiplierCVar[i] );
		flSkillValue *= var.GetFloat();
	}
	FOR_EACH_VEC( m_pFact->SkillDivisorCVar, i )
	{
		ConVarRef var( m_pFact->SkillDivisorCVar[i] );
		flSkillValue /= var.GetFloat();
	}

	if ( flBaseValue + flSkillValue < m_pFact->MinimumValue )
	{
		flBaseValue = m_pFact->MinimumValue - flSkillValue;
	}
	else if ( flBaseValue + flSkillValue > m_pFact->MaximumValue )
	{
		flBaseValue = m_pFact->MaximumValue - flSkillValue;
	}

	if ( bShowReciprocal )
	{
		flBaseValue += flSkillValue;
		flSkillValue = 0.0f;

		if ( flBaseValue != 0.0f )
		{
			flBaseValue = 1.0f / flBaseValue;
		}
	}

	int iPrecision = m_pFact->Precision;
	const wchar_t *wszBeforeNum = L"";
	const wchar_t *wszAfterNum = L"";
	if ( bIsHammerUnits )
	{
		float flDivisor = 1.0f;
		switch ( rd_swarmopedia_units_preference.GetInt() )
		{
		default:
			wszBeforeNum = g_pVGuiLocalize->Find( "#rd_weapon_fact_units_hammer_before" );
			wszAfterNum = g_pVGuiLocalize->Find( "#rd_weapon_fact_units_hammer_after" );
			break;
		case 1:
			wszBeforeNum = g_pVGuiLocalize->Find( "#rd_weapon_fact_units_meters_before" );
			wszAfterNum = g_pVGuiLocalize->Find( "#rd_weapon_fact_units_meters_after" );
			flDivisor = rd_swarmopedia_units_per_meter.GetFloat();
			iPrecision = MAX( iPrecision, 2 );
			break;
		case 2:
			wszBeforeNum = g_pVGuiLocalize->Find( "#rd_weapon_fact_units_feet_before" );
			wszAfterNum = g_pVGuiLocalize->Find( "#rd_weapon_fact_units_feet_after" );
			flDivisor = rd_swarmopedia_units_per_foot.GetFloat();
			iPrecision = MAX( iPrecision, 1 );
			break;
		}

		Assert( flDivisor > 0.0f );
		if ( flDivisor > 0.0f )
		{
			flBaseValue /= flDivisor;
			flSkillValue /= flDivisor;
		}

		if ( !wszBeforeNum )
			wszBeforeNum = L"";
		if ( !wszAfterNum )
			wszAfterNum = L"";
	}

	wchar_t buf[4096];
	if ( m_pFact->Type == Type_T::Ammo && m_pFact->ClipSize )
	{
		wchar_t wszClips[64];
		wchar_t wszSize[32];
		wchar_t wszAmmoClips[128];
		V_snwprintf( wszClips, sizeof( wszClips ), L"%.*f", iPrecision, ( flBaseValue + flSkillValue ) / m_pFact->ClipSize );
		V_snwprintf( wszSize, sizeof( wszSize ), L"%d", m_pFact->ClipSize );
		g_pVGuiLocalize->ConstructString( wszAmmoClips, sizeof( wszAmmoClips ),
			g_pVGuiLocalize->Find( "#rd_weapon_fact_ammo_clips" ),
			2, wszClips, wszSize );
		V_snwprintf( buf, sizeof( buf ), L"%s%s%s%s%s", wszBefore, wszBeforeNum, wszAmmoClips, wszAfterNum, wszAfter );
	}
	else if ( flBaseValue == 0.0f || flSkillValue == 0.0f )
	{
		V_snwprintf( buf, sizeof( buf ), L"%s%s%.*f%s%s", wszBefore, wszBeforeNum, iPrecision, flBaseValue == 0.0f ? flSkillValue : flBaseValue, wszAfterNum, wszAfter );
	}
	else
	{
		V_snwprintf( buf, sizeof( buf ), L"%s%s%.*f+%.*f%s%s", wszBefore, wszBeforeNum, iPrecision, flBaseValue, iPrecision, flSkillValue, wszAfterNum, wszAfter );
	}

	m_pLblValue->SetText( buf );
}

CRD_Collection_Panel_Equipment::CRD_Collection_Panel_Equipment( vgui::Panel *parent, const char *panelName, CRD_Collection_Tab_Equipment *pTab, const RD_Swarmopedia::Weapon *pWeapon )
	: BaseClass( parent, panelName )
{
	m_pGplFacts = new BaseModUI::GenericPanelList( this, "GplFacts", BaseModUI::GenericPanelList::ISM_PERITEM );
	m_pGplFacts->AddActionSignalTarget( this );
	m_pBtnEquip = new CNB_Button( this, "BtnEquip", "#asw_equip", this, "AcceptEquip" );

	m_pTab = pTab;
	m_pWeapon = pWeapon;
}

void CRD_Collection_Panel_Equipment::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	LoadControlSettings( "Resource/UI/CollectionPanelEquipment.res" );

	BaseClass::ApplySchemeSettings( pScheme );

	if ( m_pTab->m_pBriefing )
	{
		m_pBtnEquip->SetVisible( true );

		const char *szReason = CantEquipReason( m_pTab, m_pWeapon );
		if ( szReason )
		{
			m_pBtnEquip->SetText( szReason );
			m_pBtnEquip->SetEnabled( false );
		}
		else
		{
			m_pBtnEquip->SetEnabled( true );
			m_pBtnEquip->SetControllerButton( KEY_XBUTTON_A );
		}
	}
	else
	{
		m_pBtnEquip->SetVisible( false );
	}

	m_pGplFacts->RemoveAllPanelItems();

	FOR_EACH_VEC( m_pWeapon->Facts, i )
	{
		AddWeaponFact( m_pWeapon->Facts[i] );
	}
}

void CRD_Collection_Panel_Equipment::OnCommand( const char *command )
{
	if ( FStrEq( command, "AcceptEquip" ) )
	{
		if ( CantEquipReason( m_pTab, m_pWeapon ) )
		{
			return;
		}

		CLocalPlayerFilter filter;
		C_BaseEntity::EmitSound( filter, -1 /*SOUND_FROM_LOCAL_PLAYER*/, "ASWInterface.SelectWeapon" );

		if ( C_ASW_Medal_Store *pMedalStore = GetMedalStore() )
		{
			pMedalStore->OnSelectedEquipment( m_pWeapon->Extra, m_pWeapon->EquipIndex );
		}

		m_pTab->m_pBriefing->SelectWeapon( m_pTab->m_pProfile->m_ProfileIndex, m_pTab->m_nInventorySlot, m_pWeapon->EquipIndex );

		m_pTab->m_pParent->MarkForDeletion();
	}
	else
	{
		BaseClass::OnCommand( command );
	}
}

void CRD_Collection_Panel_Equipment::OnKeyCodePressed( vgui::KeyCode keycode )
{
	int lastUser = GetJoystickForCode( keycode );
	BaseModUI::CBaseModPanel::GetSingleton().SetLastActiveUserId( lastUser );

	vgui::KeyCode code = GetBaseButtonCode( keycode );

	switch ( code )
	{
	case KEY_XBUTTON_A:
		OnCommand( "AcceptEquip" );
		break;
	default:
		BaseClass::OnKeyCodePressed( keycode );
		break;
	}
}

void CRD_Collection_Panel_Equipment::OnItemSelected( const char *panelName )
{
	CRD_Equipment_WeaponFact *pSelected = assert_cast< CRD_Equipment_WeaponFact * >( m_pGplFacts->GetSelectedPanelItem() );

	int debug = 3;
	debug = debug;
}

void CRD_Collection_Panel_Equipment::AddWeaponFact( const RD_Swarmopedia::WeaponFact *pFact )
{
	if ( !pFact->RequireCVar.IsEmpty() )
	{
		ConVarRef var( pFact->RequireCVar );
		bool bCorrectValue;
		if ( pFact->HaveRequireValue )
		{
			bCorrectValue = !V_stricmp( pFact->RequireValue.Get(), var.GetString() );
		}
		else
		{
			bCorrectValue = var.GetBool();
		}

		if ( !bCorrectValue )
		{
			return;
		}
	}

	if ( pFact->Type == RD_Swarmopedia::WeaponFact::Type_T::Secondary )
	{
		m_pGplFacts->AddPanelItem( new CRD_Equipment_WeaponFact( m_pGplFacts, "WeaponFact", m_pTab, NULL ), true );
	}

	m_pGplFacts->AddPanelItem( new CRD_Equipment_WeaponFact( m_pGplFacts, "WeaponFact", m_pTab, pFact ), true );

	FOR_EACH_VEC( pFact->Facts, i )
	{
		AddWeaponFact( pFact->Facts[i] );
	}
}

#endif
