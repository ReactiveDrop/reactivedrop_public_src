#include "cbase.h"
#include "rd_collections.h"
#include "rd_swarmopedia.h"
#include "asw_marine_profile.h"
#include "asw_util_shared.h"
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/TextImage.h>
#include <vgui_controls/Tooltip.h>
#include "vgui_bitmapbutton.h"
#include "gameui/swarm/vgenericpanellist.h"
#include "nb_button.h"
#include "ibriefing.h"
#include "asw_medal_store.h"
#include "asw_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


ConVar rd_swarmopedia_units_preference( "rd_swarmopedia_units_preference", "1", FCVAR_ARCHIVE, "0=hammer, 1=metric, 2=imperial" );
ConVar rd_swarmopedia_units_per_foot( "rd_swarmopedia_units_per_foot", "16", FCVAR_NONE, "recommended: 12 to 16" );
ConVar rd_swarmopedia_units_per_meter( "rd_swarmopedia_units_per_meter", "52.49", FCVAR_NONE, "recommended: 39.37 to 52.49" );
extern ConVar asw_unlock_all_weapons;
extern ConVar rd_weapons_show_hidden;
extern ConVar rd_weapons_regular_class_unrestricted;
extern ConVar rd_weapons_extra_class_unrestricted;
extern ConVar rd_reduce_motion;
extern ConVar rd_swarmopedia_global_stat_window_days;
extern ConVar rd_swarmopedia_global_stat_update_seconds;

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
	Assert( MarineSkills() );

	m_pCollection = NULL;
	m_pProfile = pProfile;
	m_nInventorySlot = nInventorySlot;

	m_pBriefing = NULL;
	m_nLobbySlot = -1;

	for ( int i = 0; i < NELEMS( m_nSkillValue ); i++ )
	{
		m_nSkillValue[i] = pProfile ?
			MarineSkills()->GetSkillPoints( pProfile, ( ASW_Skill )i ) :
			MarineSkills()->GetMaxSkillPoints( ( ASW_Skill )i ) / 2 + 1;
	}
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
	m_pWeaponAttrLabel = new vgui::Label( this, "WeaponAttrLabel", L"" );
	m_pWeaponDescLabel = new vgui::Label( this, "WeaponDescLabel", L"" );
	m_pGplStats = new BaseModUI::GenericPanelList( this, "GplStats", BaseModUI::GenericPanelList::ISM_ELEVATOR );

	m_nDisplayedFrames = 0;

	if ( SteamUserStats() )
	{
		m_nStatsDays = rd_swarmopedia_global_stat_window_days.GetInt();
		m_bStatsReady = false;

		SteamAPICall_t hAPICall = SteamUserStats()->RequestGlobalStats( rd_swarmopedia_global_stat_window_days.GetInt() );
		m_OnGlobalStatsReceived.Set( hAPICall, this, &CRD_Collection_Details_Equipment::OnGlobalStatsReceived );
	}
	else
	{
		m_nStatsDays = -1;
		m_bStatsReady = true;
	}
}

void CRD_Collection_Details_Equipment::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	LoadControlSettings( "Resource/UI/CollectionDetailsEquipment.res" );

	BaseClass::ApplySchemeSettings( pScheme );
}

void CRD_Collection_Details_Equipment::DisplayEntry( TGD_Entry *pEntry )
{
	m_pGplStats->RemoveAllPanelItems();

	CRD_Collection_Entry_Equipment *pEquip = assert_cast< CRD_Collection_Entry_Equipment * >( pEntry );
	if ( !pEquip )
	{
		m_pModelPanel->m_bShouldPaint = false;
		m_pModelPanel->SetVisible( false );
		m_pWeaponNameLabel->SetText( L"" );
		m_pWeaponAttrLabel->SetText( L"" );
		m_pWeaponDescLabel->SetText( L"" );
		return;
	}

	// TODO: support arbitrary number of Display/Content per weapon
	Assert( pEquip->m_pWeapon->Display.Count() == 1 );
	Assert( pEquip->m_pWeapon->Content.Count() == 1 );

	wchar_t buf[1024]{};
	FOR_EACH_VEC( pEquip->m_pWeapon->Abilities, i )
	{
		int len = V_wcslen( buf );
		if ( i != 0 )
		{
			V_wcsncpy( &buf[len], L" \u2022 ", sizeof( buf ) - len * sizeof( wchar_t ) );
			len = V_wcslen( buf );
		}

		TryLocalize( pEquip->m_pWeapon->Abilities[i]->Caption, &buf[len], sizeof( buf ) - len * sizeof( wchar_t ) );
	}

	m_pWeaponAttrLabel->SetText( buf );

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
		V_snwprintf( wszRequiredLevel, ARRAYSIZE( wszRequiredLevel ), L"%d", pEquip->m_pWeapon->RequiredLevel );

		wchar_t wszBuf[1024]{};
		g_pVGuiLocalize->ConstructString( wszBuf, sizeof( wszBuf ),
			g_pVGuiLocalize->Find( "#rd_reach_level_to_unlock_weapon" ), 2,
			wszRequiredLevel, g_pVGuiLocalize->Find( pEquip->m_pWeapon->Name ) );
		m_pWeaponDescLabel->SetText( wszBuf );
	}
	else
	{
		if ( pEquip->m_pWeapon->Content.Count() == 1 )
		{
			m_pWeaponDescLabel->SetText( pEquip->m_pWeapon->Content[0]->Text );
		}
		else
		{
			m_pWeaponDescLabel->SetText( L"" );
		}

		if ( m_nStatsDays != -1 && m_bStatsReady )
		{
			Assert( SteamUserStats() );

			wchar_t wszDays[4]{};
			V_snwprintf( wszDays, ARRAYSIZE( wszDays ), L"%d", m_nStatsDays );

			g_pVGuiLocalize->ConstructString( buf, sizeof( buf ),
				g_pVGuiLocalize->FindSafe( m_nStatsDays ? "#rd_so_global_stat_days" : "#rd_so_global_stat_total" ), 1, wszDays );

			vgui::Label *pDaysLabel = m_pGplStats->AddPanelItem<vgui::Label>( "DaysLabel", "" );
			pDaysLabel->SetContentAlignment( vgui::Label::a_east );
			pDaysLabel->SetText( buf );

			FOR_EACH_VEC( pEquip->m_pWeapon->GlobalStats, i )
			{
				int nOK{};
				int64 nStat[61]{};
				if ( m_nStatsDays == 0 )
				{
					nOK = SteamUserStats()->GetGlobalStat( pEquip->m_pWeapon->GlobalStats[i]->StatName, &nStat[1] ) ? 1 : 0;
				}
				else
				{
					nOK = SteamUserStats()->GetGlobalStatHistory( pEquip->m_pWeapon->GlobalStats[i]->StatName, &nStat[1], sizeof( nStat ) - sizeof( nStat[0] ) );
				}

				for ( int j = 1; j <= nOK; j++ )
				{
					nStat[0] += nStat[j];
				}

				CRD_Collection_StatLine *pStatLine = m_pGplStats->AddPanelItem<CRD_Collection_StatLine>( "StatLine" );
				pStatLine->SetLabel( g_pVGuiLocalize->FindSafe( pEquip->m_pWeapon->GlobalStats[i]->Caption ) );
				pStatLine->SetValue( nStat[0] );
			}
		}
	}

	m_pWeaponNameLabel->GetTextImage()->ResizeImageToContentMaxWidth( m_pWeaponNameLabel->GetWide() );
	int iTall = m_pWeaponNameLabel->GetTextImage()->GetTall();
	int iDiff = iTall - m_pWeaponNameLabel->GetTall();
	m_pWeaponNameLabel->SetTall( iTall );

	int x, y;
	m_pWeaponAttrLabel->GetPos( x, y );
	y += iDiff;
	m_pWeaponAttrLabel->SetPos( x, y );

	m_pWeaponAttrLabel->GetTextImage()->ResizeImageToContentMaxWidth( m_pWeaponAttrLabel->GetWide() );
	iTall = m_pWeaponAttrLabel->GetTextImage()->GetUText()[0] != L'\0' ? m_pWeaponAttrLabel->GetTextImage()->GetTall() : 0;
	iDiff += iTall - m_pWeaponAttrLabel->GetTall();
	m_pWeaponAttrLabel->SetTall( iTall );

	m_pWeaponDescLabel->GetPos( x, y );
	y += iDiff;
	m_pWeaponDescLabel->SetPos( x, y );

	m_pGplStats->SetPos( 0, YRES( 380 ) - YRES( 12 ) * m_pGplStats->GetPanelItemCount() );
}

void CRD_Collection_Details_Equipment::OnThink()
{
	if ( !m_OnGlobalStatsReceived.IsActive() )
	{
		m_nDisplayedFrames++;

		if ( m_nDisplayedFrames >= rd_swarmopedia_global_stat_update_seconds.GetInt() * 60 )
		{
			m_nDisplayedFrames = 0;

			m_nStatsDays = rd_swarmopedia_global_stat_window_days.GetInt();
			m_bStatsReady = false;

			SteamAPICall_t hAPICall = SteamUserStats()->RequestGlobalStats( rd_swarmopedia_global_stat_window_days.GetInt() );
			m_OnGlobalStatsReceived.Set( hAPICall, this, &CRD_Collection_Details_Equipment::OnGlobalStatsReceived );
		}
	}
}

void CRD_Collection_Details_Equipment::OnGlobalStatsReceived( GlobalStatsReceived_t *pParam, bool bIOError )
{
	if ( bIOError || pParam->m_eResult != k_EResultOK )
	{
		Warning( "Failed to retrieve global stat history for Swarmopedia: %s\n", bIOError ? "IO Error" : UTIL_RD_EResultToString( pParam->m_eResult ) );
		m_nStatsDays = -1;
	}

	m_bStatsReady = true;
	DisplayEntry( GetCurrentEntry() );
}

CRD_Collection_Entry_Equipment::CRD_Collection_Entry_Equipment( TGD_Grid *parent, const char *panelName, const RD_Swarmopedia::Weapon *pWeapon )
	: BaseClass( parent, panelName )
{
	m_pWeapon = pWeapon;
	m_bNoDirectEquip = false;

	m_pIcon = new vgui::ImagePanel( this, "Icon" );
	m_pLockedIcon = new vgui::ImagePanel( this, "LockedIcon" );
	m_pLockedOverlay = new vgui::Label( this, "LockedOverlay", "#asw_weapon_details_required_level" );
	m_pLockedLabel = new vgui::Label( this, "LockedLabel", L"" );
	m_pCantEquipLabel = new vgui::Label( this, "CantEquipLabel", L"" );
	m_pNewLabel = new vgui::Label( this, "NewLabel", "#new_weapon" );
	m_pClassIcon = new vgui::ImagePanel( this, "ClassIcon" );
	m_pClassLabel = new vgui::Label( this, "ClassLabel", L"" );
	m_pInfoButton = new CBitmapButton( this, "InfoButton", "" );
	m_pInfoButton->AddActionSignalTarget( this );
}

void CRD_Collection_Entry_Equipment::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	m_pInfoButton->SetVisible( false );

	color32 white{ 255, 255, 255, 255 };
	color32 lgray{ 192, 192, 192, 255 };
	color32 transparent{ 0, 0, 0, 0 };
	m_pInfoButton->SetImage( CBitmapButton::BUTTON_ENABLED, "vgui/swarm/swarmopedia/fact/generic", lgray );
	m_pInfoButton->SetImage( CBitmapButton::BUTTON_ENABLED_MOUSE_OVER, "vgui/swarm/swarmopedia/fact/generic", white );
	m_pInfoButton->SetImage( CBitmapButton::BUTTON_PRESSED, "vgui/swarm/swarmopedia/fact/generic", white );
	m_pInfoButton->SetImage( CBitmapButton::BUTTON_DISABLED, "vgui/swarm/swarmopedia/fact/generic", transparent );

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

	bool bLevelLocked = !asw_unlock_all_weapons.GetBool() && !UTIL_ASW_CommanderLevelAtLeast( NULL, m_pWeapon->RequiredLevel - 1, -1 );
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

		if ( pTab->m_pBriefing )
		{
			m_pInfoButton->SetVisible( true );
		}
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
		V_snwprintf( wszLevel, ARRAYSIZE( wszLevel ), L"%d", m_pWeapon->RequiredLevel );
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

void CRD_Collection_Entry_Equipment::OnKeyCodePressed( vgui::KeyCode keycode )
{
	if ( !m_pInfoButton->IsVisible() )
	{
		BaseClass::OnKeyCodePressed( keycode );
		return;
	}

	int lastUser = GetJoystickForCode( keycode );
	BaseModUI::CBaseModPanel::GetSingleton().SetLastActiveUserId( lastUser );

	vgui::KeyCode code = GetBaseButtonCode( keycode );

	switch ( code )
	{
	case KEY_XBUTTON_A:
		BaseModUI::CBaseModPanel::GetSingleton().PlayUISound( BaseModUI::UISOUND_ACCEPT );
		ApplyEntry();
		break;
	case KEY_XBUTTON_X:
		BaseModUI::CBaseModPanel::GetSingleton().PlayUISound( BaseModUI::UISOUND_ACCEPT );
		m_bNoDirectEquip = true;
		ApplyEntry();
		break;
	default:
		BaseClass::OnKeyCodePressed( keycode );
		break;
	}
}

void CRD_Collection_Entry_Equipment::OnCommand( const char *command )
{
	if ( !V_strcmp( command, "ShowInfo" ) )
	{
		m_bNoDirectEquip = true;
		ApplyEntry();
	}
	else
	{
		BaseClass::OnCommand( command );
	}
}

void CRD_Collection_Entry_Equipment::OnThink()
{
	BaseClass::OnThink();

	m_pInfoButton->SetEnabled( IsCursorOver() );
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

	if ( !m_bNoDirectEquip && pTab->m_pBriefing )
	{
		pTGD->m_hOverridePanel->OnCommand( "AcceptEquip" );
	}

	m_bNoDirectEquip = false;
}

class CRD_Equipment_WeaponFact : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CRD_Equipment_WeaponFact, vgui::EditablePanel );
public:
	CRD_Equipment_WeaponFact( vgui::Panel *parent, const char *panelName, CRD_Collection_Tab_Equipment *pTab, const RD_Swarmopedia::WeaponFact *pFact );

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	virtual void PerformLayout() override;
	virtual void OnCommand( const char *command ) override;

	vgui::ImagePanel *m_pIcon;
	CBitmapButton *m_pSkillIcon;
	vgui::ImagePanel *m_pSkillPips[5];
	vgui::Label *m_pLblName;
	vgui::Label *m_pLblValue;

	CRD_Collection_Tab_Equipment *m_pTab;
	const RD_Swarmopedia::WeaponFact *m_pFact;
	int m_iOriginalTall;
};

CRD_Equipment_WeaponFact::CRD_Equipment_WeaponFact( vgui::Panel *parent, const char *panelName, CRD_Collection_Tab_Equipment *pTab, const RD_Swarmopedia::WeaponFact *pFact ) :
	BaseClass( parent, panelName )
{
	m_pTab = pTab;
	m_pFact = pFact;

	m_pIcon = new vgui::ImagePanel( this, "Icon" );
	m_pSkillIcon = new CBitmapButton( this, "SkillIcon", "" );
	m_pSkillIcon->AddActionSignalTarget( this );
	m_pSkillIcon->SetCommand( "SkillCycle" );
	for ( int i = 0; i < NELEMS( m_pSkillPips ); i++ )
	{
		m_pSkillPips[i] = new vgui::ImagePanel( this, "SkillPip" );
		m_pSkillPips[i]->SetShouldScaleImage( true );
		m_pSkillPips[i]->SetSize( YRES( 6 ), YRES( 6 ) );
	}
	m_pLblName = new vgui::Label( this, "LblName", L"" );
	m_pLblValue = new vgui::Label( this, "LblValue", L"" );
}

void CRD_Equipment_WeaponFact::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	LoadControlSettings( "Resource/UI/CollectionPanelEquipmentWeaponFact.res" );

	BaseClass::ApplySchemeSettings( pScheme );

	Assert( m_pLblName->GetTall() == m_pLblValue->GetTall() && m_pLblName->GetTall() == GetTall() );
	m_iOriginalTall = GetTall();

	m_pSkillIcon->SetVisible( false );
	for ( int i = 0; i < NELEMS( m_pSkillPips ); i++ )
	{
		m_pSkillPips[i]->SetVisible( false );
	}

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
	bool bShowReciprocal = m_pFact->ShowReciprocal;

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
		bShowReciprocal = !bShowReciprocal;
		break;
	case Type_T::Ammo:
		szIcon = "swarm/swarmopedia/fact/ammo";
		szCaption = "#rd_weapon_fact_ammo";
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
	case Type_T::Deployed:
		// doesn't get displayed.
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
		CFmtStr szSkillImage( "vgui/%s", MarineSkills()->GetSkillImage( m_pFact->Skill ) );
		m_pSkillIcon->SetImage( CBitmapButton::BUTTON_ENABLED, szSkillImage, color32{ 255, 255, 255, 255 } );
		m_pSkillIcon->SetImage( CBitmapButton::BUTTON_ENABLED_MOUSE_OVER, szSkillImage, color32{ 255, 255, 255, 255 } );
		m_pSkillIcon->SetImage( CBitmapButton::BUTTON_PRESSED, szSkillImage, color32{ 255, 255, 255, 255 } );
		m_pSkillIcon->SetImage( CBitmapButton::BUTTON_DISABLED, szSkillImage, color32{ 255, 255, 255, 255 } );
		m_pSkillIcon->SetVisible( true );
		m_pSkillIcon->SetEnabled( !m_pTab->m_pProfile );
		m_pSkillIcon->GetTooltip()->SetText( MarineSkills()->GetSkillName( m_pFact->Skill ) );

		int iMaxSkillPoints = MarineSkills()->GetMaxSkillPoints( m_pFact->Skill );
		int iSkillValue = m_pTab->m_nSkillValue[m_pFact->Skill];

		int x, y;
		m_pSkillIcon->GetPos( x, y );
		Assert( NELEMS( m_pSkillPips ) >= iMaxSkillPoints );
		for ( int i = 0; i < NELEMS( m_pSkillPips ); i++ )
		{
			m_pSkillPips[i]->SetVisible( i < iMaxSkillPoints );
			m_pSkillPips[i]->SetImage( i < iSkillValue ? "swarm/swarmopedia/skill_full" : "swarm/swarmopedia/skill_empty" );

			if ( i < iMaxSkillPoints )
			{
				m_pSkillPips[i]->SetPos( x - YRES( 6 ), y + ( iMaxSkillPoints - i ) * m_pSkillIcon->GetTall() / ( iMaxSkillPoints + 2 ) );
			}
		}

		flSkillValue = MarineSkills()->GetSkillBasedValue( m_pTab->m_pProfile, m_pFact->Skill, m_pFact->SubSkill, iSkillValue );
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
		if ( m_pFact->SkillValueIsClipSize )
		{
			V_snwprintf( wszClips, ARRAYSIZE( wszClips ), L"%.*f", iPrecision, flBaseValue );
			V_snwprintf( wszSize, ARRAYSIZE( wszSize ), L"%.0f", flSkillValue );
		}
		else
		{
			V_snwprintf( wszClips, ARRAYSIZE( wszClips ), L"%.*f", iPrecision, ( flBaseValue + flSkillValue ) / m_pFact->ClipSize );
			V_snwprintf( wszSize, ARRAYSIZE( wszSize ), L"%d", m_pFact->ClipSize );
		}
		g_pVGuiLocalize->ConstructString( wszAmmoClips, sizeof( wszAmmoClips ),
			g_pVGuiLocalize->Find( "#rd_weapon_fact_ammo_clips" ),
			2, wszClips, wszSize );
		V_snwprintf( buf, ARRAYSIZE( buf ), L"%s%s%s%s%s", wszBefore, wszBeforeNum, wszAmmoClips, wszAfterNum, wszAfter );
	}
	else if ( flBaseValue == 0.0f || flSkillValue == 0.0f )
	{
		V_snwprintf( buf, ARRAYSIZE( buf ), L"%s%s%.*f%s%s", wszBefore, wszBeforeNum, iPrecision, flBaseValue == 0.0f ? flSkillValue : flBaseValue, wszAfterNum, wszAfter );
	}
	else
	{
		V_snwprintf( buf, ARRAYSIZE( buf ), L"%s%s%.*f+%.*f%s%s", wszBefore, wszBeforeNum, iPrecision, flBaseValue, iPrecision, flSkillValue, wszAfterNum, wszAfter );
	}

	m_pLblValue->SetText( buf );
}

void CRD_Equipment_WeaponFact::PerformLayout()
{
	BaseClass::PerformLayout();

	Assert( m_pLblName->GetFont() == m_pLblValue->GetFont() );

	if ( GetTall() < m_iOriginalTall )
	{
		return;
	}

	m_pLblName->GetTextImage()->RecalculateNewLinePositions();
	m_pLblValue->GetTextImage()->RecalculateNewLinePositions();

	int x, y;

	m_pLblName->GetTextImage()->GetContentSize( x, y );
	int iTextTall = y;

	m_pLblValue->GetTextImage()->GetContentSize( x, y );
	if ( y > iTextTall )
	{
		iTextTall = y;
	}

	int iFontTall = vgui::surface()->GetFontTall( m_pLblName->GetFont() );
	int iNewTall = iTextTall - iFontTall + m_iOriginalTall;
	int iTallDiff = iNewTall - GetTall();

	if ( iTallDiff != 0 )
	{
		Assert( GetParent() && GetParent()->GetParent() );
		GetParent()->GetParent()->InvalidateLayout();
	}

	m_pIcon->GetPos( x, y );
	y += iTallDiff / 2;
	m_pIcon->SetPos( x, y );

	m_pSkillIcon->GetPos( x, y );
	y += iTallDiff / 2;
	m_pSkillIcon->SetPos( x, y );

	if ( vgui::Panel *pBackground = FindChildByName( "Background" ) )
	{
		pBackground->SetTall( iNewTall );
	}

	m_pLblName->SetTall( iNewTall );
	m_pLblValue->SetTall( iNewTall );
	SetTall( iNewTall );
}

void CRD_Equipment_WeaponFact::OnCommand( const char *command )
{
	if ( !V_strcmp( command, "SkillCycle" ) )
	{
		BaseModUI::CBaseModPanel::GetSingleton().PlayUISound( BaseModUI::UISOUND_ACCEPT );

		Assert( m_pFact->Skill != ASW_MARINE_SKILL_INVALID );
		m_pTab->m_nSkillValue[m_pFact->Skill]++;
		m_pTab->m_nSkillValue[m_pFact->Skill] %= MarineSkills()->GetMaxSkillPoints( m_pFact->Skill ) + 1;

		m_pTab->m_pParent->m_hOverridePanel->InvalidateLayout( false, true );
	}
	else
	{
		BaseClass::OnCommand( command );
	}
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
		if ( m_pTab->m_pBriefing )
			OnCommand( "AcceptEquip" );
		else
			BaseClass::OnKeyCodePressed( keycode );
		break;
	default:
		BaseClass::OnKeyCodePressed( keycode );
		break;
	}
}

void CRD_Collection_Panel_Equipment::OnItemSelected( const char *panelName )
{
	CRD_Equipment_WeaponFact *pSelected = assert_cast< CRD_Equipment_WeaponFact * >( m_pGplFacts->GetSelectedPanelItem() );

	// TODO
	( void )pSelected;
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

	if ( pFact->Type == RD_Swarmopedia::WeaponFact::Type_T::Secondary || pFact->Type == RD_Swarmopedia::WeaponFact::Type_T::Deployed )
	{
		m_pGplFacts->AddPanelItem( new CRD_Equipment_WeaponFact( m_pGplFacts, "WeaponFact", m_pTab, NULL ), true );
	}

	if ( pFact->Type != RD_Swarmopedia::WeaponFact::Type_T::Deployed )
	{
		m_pGplFacts->AddPanelItem( new CRD_Equipment_WeaponFact( m_pGplFacts, "WeaponFact", m_pTab, pFact ), true );
	}

	FOR_EACH_VEC( pFact->Facts, i )
	{
		AddWeaponFact( pFact->Facts[i] );
	}
}
