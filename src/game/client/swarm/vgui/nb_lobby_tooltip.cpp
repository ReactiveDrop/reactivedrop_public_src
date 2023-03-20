#include "cbase.h"
#include "nb_lobby_tooltip.h"
#include "vgui_controls/Label.h"
#include "vgui_controls/Panel.h"
#include "vgui_controls/ImagePanel.h"
#include "vgui_controls/RichText.h"
#include "asw_marine_profile.h"
#include "asw_briefing.h"
#include "nb_skill_panel.h"
#include "nb_weapon_detail.h"
#include "asw_equipment_list.h"
#include "asw_weapon_parse.h"
#include "asw_marine_profile.h"
#include "vgui_controls/Panel.h"
#include "asw_model_panel.h"
#include <vgui/IVgui.h>
#include <vgui_controls/AnimationController.h>
#include "asw_gamerules.h"
#include "rd_inventory_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar rd_briefing_item_details_delay( "rd_briefing_item_details_delay", "3.0", FCVAR_NONE );

CNB_Lobby_Tooltip::CNB_Lobby_Tooltip( vgui::Panel *parent, const char *name ) : BaseClass( parent, name )
{
	// == MANAGED_MEMBER_CREATION_START: Do not edit by hand ==	
	m_pBackground = new vgui::Panel( this, "Background" );
	m_pBackgroundInner = new vgui::Panel( this, "BackgroundInner" );
	m_pTitleBG = new vgui::Panel( this, "TitleBG" );
	m_pTitleBGBottom = new vgui::Panel( this, "TitleBGBottom" );
	m_pTitle = new vgui::Label( this, "Title", "" );
	m_pSkillPanel0 = new CNB_Skill_Panel( this, "SkillPanel0" );
	m_pSkillPanel1 = new CNB_Skill_Panel( this, "SkillPanel1" );
	m_pSkillPanel2 = new CNB_Skill_Panel( this, "SkillPanel2" );
	m_pSkillPanel3 = new CNB_Skill_Panel( this, "SkillPanel3" );
	m_pSkillPanel4 = new CNB_Skill_Panel( this, "SkillPanel4" );
	m_pWeaponDetail0 = new CNB_Weapon_Detail( this, "WeaponDetail0" );
	m_pWeaponDetail1 = new CNB_Weapon_Detail( this, "WeaponDetail1" );
	m_pWeaponDetail2 = new CNB_Weapon_Detail( this, "WeaponDetail2" );
	m_pWeaponDetail3 = new CNB_Weapon_Detail( this, "WeaponDetail3" );
	m_pWeaponDetail4 = new CNB_Weapon_Detail( this, "WeaponDetail4" );
	m_pWeaponDetail5 = new CNB_Weapon_Detail( this, "WeaponDetail5" );
	m_pTitleBGLine = new vgui::Panel( this, "TitleBGLine" );
	m_pItemModelPanel = new CASW_Model_Panel( this, "ItemModelPanel" );
	// == MANAGED_MEMBER_CREATION_END ==
	m_pIconBackground = new vgui::Panel( this, "IconBackground" );
	m_pPromotionIcon = new vgui::ImagePanel( this, "PromotionIcon" );
	m_pPromotionLabel = new vgui::Label( this, "PromotionLabel", "" );
	m_pItemDescription = new vgui::RichText( this, "ItemDescription" );

	m_pItemModelPanel->m_bShouldPaint = false;
	m_pItemModelPanel->SetVisible( false );

	m_nLobbySlot = -1;
	m_nLastWeaponHash = -1;
	m_nLastInventorySlot = -1;
	m_bMarineTooltip = false;
	m_bPromotionTooltip = false;
	m_bMedalTooltip = false;
	m_iLastMedal = 0;

	vgui::ivgui()->AddTickSignal( GetVPanel() );
}

CNB_Lobby_Tooltip::~CNB_Lobby_Tooltip()
{

}

void CNB_Lobby_Tooltip::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );
	
	LoadControlSettings( "resource/ui/nb_lobby_tooltip.res" );

	m_pItemDescription->SetCursor( vgui::dc_arrow );

	m_DefaultTitleColor = m_pTitle->GetFgColor();
}

void CNB_Lobby_Tooltip::PerformLayout()
{
	BaseClass::PerformLayout();

	KeyValues *pKV = new KeyValues( "ItemModelPanel" );
	pKV->SetInt( "fov", 20 );
	pKV->SetInt( "start_framed", 0 );
	pKV->SetInt( "disable_manipulation", 1 );
	m_pItemModelPanel->ApplySettings( pKV );

	m_pItemDescription->SetPanelInteractive( false );
}

void CNB_Lobby_Tooltip::OnThink()
{
	BaseClass::OnThink();
}

void CNB_Lobby_Tooltip::ShowMarineTooltip( int nLobbySlot )
{
	m_bMarineTooltip = true;
	m_nLobbySlot = nLobbySlot;
	m_bPromotionTooltip = false;
	m_bMedalTooltip = false;
}

void CNB_Lobby_Tooltip::ShowWeaponTooltip( int nLobbySlot, int nInventorySlot )
{
	m_bMarineTooltip = false;
	m_nLobbySlot = nLobbySlot;
	m_nInventorySlot = nInventorySlot;
	m_bPromotionTooltip = false;
	m_bMedalTooltip = false;
}

void CNB_Lobby_Tooltip::ShowMarinePromotionTooltip( int nLobbySlot )
{
	m_bMarineTooltip = false;
	m_nLobbySlot = nLobbySlot;
	m_bPromotionTooltip = true;
	m_bMedalTooltip = false;
}

void CNB_Lobby_Tooltip::ShowMarineMedalTooltip( int nLobbySlot )
{
	// force reset (this handles cases like multiple players with HoIAF medals)
	if ( m_nLobbySlot != nLobbySlot )
		m_iLastMedal = -1;

	m_bMarineTooltip = false;
	m_nLobbySlot = nLobbySlot;
	m_bPromotionTooltip = false;
	m_bMedalTooltip = true;
}

void CNB_Lobby_Tooltip::OnTick()
{
	if ( ASWGameRules() && ASWGameRules()->GetCurrentVoteType() != ASW_VOTE_NONE )
	{
		SetVisible( false );
		return;
	}
	SetVisible( true );

	m_bValidTooltip = false;
	m_pIconBackground->SetPaintBackgroundEnabled( false );
	m_pTitle->SetVisible( false );
	m_pSkillPanel0->SetVisible( false );
	m_pSkillPanel1->SetVisible( false );
	m_pSkillPanel2->SetVisible( false );
	m_pSkillPanel3->SetVisible( false );
	m_pSkillPanel4->SetVisible( false );
	m_pPromotionIcon->SetVisible( false );
	m_pPromotionLabel->SetVisible( false );
	m_pItemDescription->SetVisible( false );

	m_pWeaponDetail0->m_bHidden = true;
	m_pWeaponDetail1->m_bHidden = true;
	m_pWeaponDetail2->m_bHidden = true;
	m_pWeaponDetail3->m_bHidden = true;
	m_pWeaponDetail4->m_bHidden = true;
	m_pWeaponDetail5->m_bHidden = true;
	m_pItemModelPanel->m_bShouldPaint = false;
	m_pItemModelPanel->SetVisible( false );

	int nPromotion = 0;
	if ( m_bPromotionTooltip )
	{
		nPromotion = Briefing()->GetCommanderPromotion( m_nLobbySlot );
	}

	if ( m_nLobbySlot == -1 || !Briefing()->IsLobbySlotOccupied( m_nLobbySlot ) || ( m_bPromotionTooltip && nPromotion <= 0 ) )
	{
		return;
	}

	if ( m_bMedalTooltip )
	{
		m_pTitle->SetVisible( true );

		const CRD_ItemInstance &details = Briefing()->GetEquippedMedal( m_nLobbySlot );
		if ( details.m_iItemDefID != m_iLastMedal )
		{
			const ReactiveDropInventory::ItemDef_t *pDef = ReactiveDropInventory::GetItemDef( details.m_iItemDefID );
			if ( !pDef )
			{
				return;
			}

			m_pTitle->SetFgColor( pDef->NameColor );

			wchar_t wszBuf[2048];
			V_UTF8ToUnicode( pDef->BriefingName, wszBuf, sizeof( wszBuf ) );
			m_pTitle->SetText( wszBuf );

			m_pIconBackground->SetBgColor( pDef->BackgroundColor );
			m_pIconBackground->SetPaintBackgroundEnabled( true );
			m_pIconBackground->SetPaintBackgroundType( 2 );

			m_pPromotionIcon->SetImage( "" );
			m_pPromotionIcon->SetImage( pDef->Icon );

			V_UTF8ToUnicode( pDef->DisplayType, wszBuf, sizeof( wszBuf ) );
			m_pPromotionLabel->SetText( wszBuf );

			details.FormatDescription( m_pItemDescription );

			if ( rd_briefing_item_details_delay.GetFloat() < 0 )
				m_flInventoryDetailsAfter = FLT_MAX;
			else
				m_flInventoryDetailsAfter = gpGlobals->realtime + rd_briefing_item_details_delay.GetFloat();

			m_iLastMedal = details.m_iItemDefID;
		}

		if ( gpGlobals->realtime < m_flInventoryDetailsAfter )
		{
			m_pPromotionIcon->SetVisible( true );
			m_pPromotionLabel->SetVisible( true );
			m_pPromotionIcon->SetAlpha( MIN( m_flInventoryDetailsAfter - gpGlobals->realtime, 0.125f ) * 8 * 255 );
			m_pPromotionLabel->SetAlpha( MIN( m_flInventoryDetailsAfter - gpGlobals->realtime, 0.125f ) * 8 * 255 );
		}
		else
		{
			m_pItemDescription->SetVisible( true );
			m_pItemDescription->SetAlpha( MIN( gpGlobals->realtime - m_flInventoryDetailsAfter, 0.125f ) * 8 * 255 );
		}

		m_bValidTooltip = true;

		return;
	}

	m_iLastMedal = -1;

	if ( m_bPromotionTooltip )
	{
		m_pTitle->SetFgColor( m_DefaultTitleColor );
		m_pTitle->SetVisible( true );
		m_pPromotionIcon->SetVisible( true );
		m_pPromotionLabel->SetVisible( true );

		switch ( nPromotion )
		{
		case 1:
			m_pPromotionLabel->SetText( "#nb_first_promotion" );
			break;
		case 2:
			m_pPromotionLabel->SetText( "#nb_second_promotion" );
			break;
		case 3:
			m_pPromotionLabel->SetText( "#nb_third_promotion" );
			break;
		case 4:
			m_pPromotionLabel->SetText( "#nb_fourth_promotion" );
			break;
		case 5:
			m_pPromotionLabel->SetText( "#nb_fifth_promotion" );
			break;
		case 6:
			m_pPromotionLabel->SetText( "#nb_sixth_promotion" );
			break;
		case 7:
			m_pPromotionLabel->SetText( "#nb_seventh_promotion" );
			break;
		}
		m_pTitle->SetText( VarArgs( "#nb_promotion_medal_%d", nPromotion ) );
		m_pPromotionIcon->SetImage( VarArgs( "briefing/promotion_%d_LG", nPromotion ) );

		m_pPromotionLabel->SetAlpha( 255 );
		m_pPromotionIcon->SetAlpha( 255 );

		m_bValidTooltip = true;

		return;
	}

	if ( m_bMarineTooltip )
	{
		CASW_Marine_Profile *pProfile = Briefing()->GetMarineProfile( m_nLobbySlot );
		if ( pProfile )
		{
			m_pTitle->SetFgColor( m_DefaultTitleColor );
			m_pTitle->SetVisible( true );
			m_pSkillPanel0->SetVisible( true );
			m_pSkillPanel1->SetVisible( true );
			m_pSkillPanel2->SetVisible( true );
			m_pSkillPanel3->SetVisible( true );
			m_pSkillPanel4->SetVisible( true );

			m_pTitle->SetText( Briefing()->GetMarineName( m_nLobbySlot ) );
			m_pSkillPanel0->SetSkillDetails( pProfile->m_ProfileIndex, 0, Briefing()->GetMarineSkillPoints( m_nLobbySlot, 0 ) );
			m_pSkillPanel1->SetSkillDetails( pProfile->m_ProfileIndex, 1, Briefing()->GetMarineSkillPoints( m_nLobbySlot, 1 ) );
			m_pSkillPanel2->SetSkillDetails( pProfile->m_ProfileIndex, 2, Briefing()->GetMarineSkillPoints( m_nLobbySlot, 2 ) );
			m_pSkillPanel3->SetSkillDetails( pProfile->m_ProfileIndex, 3, Briefing()->GetMarineSkillPoints( m_nLobbySlot, 3 ) );
			m_pSkillPanel4->SetSkillDetails( pProfile->m_ProfileIndex, 4, Briefing()->GetMarineSkillPoints( m_nLobbySlot, 4 ) );

			m_bValidTooltip = true;
		}
		else
		{
			m_bMarineTooltip = false;
		}

		return;
	}

	int nWeapon = Briefing()->GetMarineSelectedWeapon( m_nLobbySlot, m_nInventorySlot );
	if ( nWeapon == -1 )
	{
		return;
	}

	CASW_EquipItem *pItem = g_ASWEquipmentList.GetItemForSlot( m_nInventorySlot, nWeapon );
	if ( !pItem )
		return;

	CASW_WeaponInfo *pWeaponInfo = g_ASWEquipmentList.GetWeaponDataFor( pItem->m_szEquipClass );
	if ( !pWeaponInfo )
		return;

	m_pTitle->SetFgColor( m_DefaultTitleColor );
	m_pTitle->SetVisible( true );

	bool bShowDetails = ( m_nInventorySlot != ASW_INVENTORY_SLOT_EXTRA && pWeaponInfo->m_flBaseDamage > 0 );

	int nWeaponHash = nWeapon * ( m_nInventorySlot + 1 );
	bool bWeaponChanged = false;
	if ( nWeaponHash != m_nLastWeaponHash || m_nInventorySlot != m_nLastInventorySlot )
	{
		m_nLastWeaponHash = nWeaponHash;
		m_nLastInventorySlot = m_nInventorySlot;
		bWeaponChanged = true;
		// debug
		//Msg( "m_nInventorySlot = %d\nnWeapon = %d\nnWeaponHash = %d\nbShowDetails = %d\n", m_nInventorySlot, nWeapon, nWeaponHash, bShowDetails ? 1 : 0 );
	}

	if ( bShowDetails )
	{
		m_pWeaponDetail0->m_bHidden = false;
		m_pWeaponDetail1->m_bHidden = false;
		m_pWeaponDetail2->m_bHidden = false;
		m_pWeaponDetail3->m_bHidden = false;
		m_pWeaponDetail4->m_bHidden = false;
		m_pWeaponDetail5->m_bHidden = false;
	}
	else
	{
		m_pWeaponDetail5->m_bHidden = false;

		m_pItemModelPanel->SetCameraForWeapon( pWeaponInfo->m_flModelPanelZOffset, gpGlobals->curtime );
		m_pItemModelPanel->m_bShouldPaint = true;
		m_pItemModelPanel->SetVisible( true );

		if ( bWeaponChanged )
		{
			m_pItemModelPanel->SetModelByWeapon( pWeaponInfo );

			// force resetup of various things (this block of code fixes size popping when changing model)
			m_pItemModelPanel->InvalidateLayout( true );

			m_pItemModelPanel->SetCameraForWeapon( pWeaponInfo->m_flModelPanelZOffset, gpGlobals->curtime );

			m_pItemModelPanel->InvalidateLayout( true );
			m_pItemModelPanel->SetAlpha( 0 );
			vgui::GetAnimationController()->RunAnimationCommand( m_pItemModelPanel, "Alpha", 255, 0.01f, 0.5f, vgui::AnimationController::INTERPOLATOR_LINEAR );
		}
	}

	CASW_Marine_Profile *pProfile = Briefing()->GetMarineProfile( m_nLobbySlot );
	if ( pProfile )
	{
		int nProfileIndex = pProfile->m_ProfileIndex;
		int nEquipIndex = Briefing()->GetProfileSelectedWeapon( nProfileIndex, m_nInventorySlot );
		m_pWeaponDetail0->SetWeaponDetails( nEquipIndex, m_nInventorySlot, nProfileIndex, 0 );
		m_pWeaponDetail1->SetWeaponDetails( nEquipIndex, m_nInventorySlot, nProfileIndex, 1 );
		m_pWeaponDetail2->SetWeaponDetails( nEquipIndex, m_nInventorySlot, nProfileIndex, 2 );
		m_pWeaponDetail3->SetWeaponDetails( nEquipIndex, m_nInventorySlot, nProfileIndex, 3 );
		m_pWeaponDetail4->SetWeaponDetails( nEquipIndex, m_nInventorySlot, nProfileIndex, 4 );
		m_pWeaponDetail5->SetWeaponDetails( nEquipIndex, m_nInventorySlot, nProfileIndex, 5 );
	}

	m_pTitle->SetText( pItem->m_szLongName );

	m_bValidTooltip = true;
}
