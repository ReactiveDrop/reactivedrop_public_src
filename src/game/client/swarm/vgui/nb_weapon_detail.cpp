#include "cbase.h"
#include "nb_weapon_detail.h"
#include "vgui_controls/Label.h"
#include "asw_weapon_parse.h"
#include "asw_equipment_list.h"
#include <vgui/ILocalize.h>
#include "StatsBar.h"
#include "asw_briefing.h"
#include "c_asw_player.h"
#include "ammodef.h"
#include "asw_marine_skills.h"
#include "c_asw_game_resource.h"
#include "c_asw_marine_resource.h"
#include "rd_swarmopedia.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


CNB_Weapon_Detail::CNB_Weapon_Detail( vgui::Panel *parent, const char *name ) : BaseClass( parent, name )
{
	// == MANAGED_MEMBER_CREATION_START: Do not edit by hand ==
	m_pTitleLabel = new vgui::Label( this, "TitleLabel", "" );
	m_pValueLabel = new vgui::Label( this, "ValueLabel", "" );
	m_pStatsBar = new StatsBar( this, "StatsBar" );
	// == MANAGED_MEMBER_CREATION_END ==

	m_bHidden = false;
	m_pStatsBar->SetShowMaxOnCounter( true );
	m_pStatsBar->m_flBorder = 1.5f;
	m_pStatsBar->AddMinMax( 0, 1.0f );
	m_nEquipIndex = -1;
	m_nProfileIndex = -1;
}

CNB_Weapon_Detail::~CNB_Weapon_Detail()
{

}

void CNB_Weapon_Detail::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );
	
	LoadControlSettings( "resource/ui/nb_weapon_detail.res" );
}

void CNB_Weapon_Detail::PerformLayout()
{
	BaseClass::PerformLayout();
}

void CNB_Weapon_Detail::OnThink()
{
	BaseClass::OnThink();

	if ( m_bHidden )
	{
		m_pTitleLabel->SetVisible( false );
		m_pValueLabel->SetVisible( false );
		m_pStatsBar->SetVisible( false );
		return;
	}

	if ( m_nEquipIndex != -1 )
	{
		CASW_EquipItem *pItem = g_ASWEquipmentList.GetItemForSlot( m_nInventorySlot, m_nEquipIndex );
		if ( pItem )
		{
			CASW_WeaponInfo *pWeaponInfo = g_ASWEquipmentList.GetWeaponDataFor( pItem->m_szEquipClass );
			if ( pWeaponInfo )
			{
				UpdateLabels( pItem, pWeaponInfo );
			}
		}
	}
}

void CNB_Weapon_Detail::SetWeaponDetails( int nEquipIndex, int nInventorySlot, int nProfileIndex, RD_Swarmopedia::WeaponFact::Type_T eWeaponFactType )
{
	m_nEquipIndex = nEquipIndex;
	m_nInventorySlot = nInventorySlot;
	m_nProfileIndex = nProfileIndex;
	m_eWeaponFactType = eWeaponFactType;
}

void CNB_Weapon_Detail::UpdateLabels( CASW_EquipItem *pItem, CASW_WeaponInfo *pWeaponData )
{
	m_pTitleLabel->SetVisible( false );
	m_pValueLabel->SetVisible( false );
	m_pStatsBar->SetVisible( false );

	C_ASW_Player *pPlayer = C_ASW_Player::GetLocalASWPlayer();
	if ( !pPlayer )
		return;

	CASW_Marine_Profile *pProfile = Briefing()->GetMarineProfileByProfileIndex( m_nProfileIndex );

	const RD_Swarmopedia::Weapon *pWeapon = RD_Swarmopedia::FindWeapon( pItem->m_iItemIndex, pItem->m_bIsExtra );
	if ( !pWeapon )
	{
		m_pTitleLabel->SetVisible( true );
		m_pTitleLabel->SetText( pItem->m_szEquipClass );
		m_pValueLabel->SetVisible( true );
		m_pValueLabel->SetText( L"MISSING SWARMOPEDIA ARTICLE" ); // this is not translated because it should not ever show up in the released game
		return;
	}

	if ( m_eWeaponFactType == RD_Swarmopedia::WeaponFact::Type_T::Generic )
	{
		if ( const wchar_t *wszAttributesValue = g_pVGuiLocalize->Find( pItem->m_szAttributeDescription ) )
		{
			m_pTitleLabel->SetVisible( true );
			m_pTitleLabel->SetText( g_pVGuiLocalize->Find( "#asw_weapon_details_notes" ) );
			m_pValueLabel->SetVisible( true );
			m_pValueLabel->SetText( wszAttributesValue );
		}

		return;
	}

	const RD_Swarmopedia::WeaponFact *pFact = NULL;
	FOR_EACH_VEC( pWeapon->Facts, i )
	{
		if ( pWeapon->Facts[i]->Type == m_eWeaponFactType )
		{
			pFact = pWeapon->Facts[i];
			break;
		}
	}

	m_pTitleLabel->SetVisible( true );
	if ( m_eWeaponFactType == RD_Swarmopedia::WeaponFact::Type_T::DamagePerShot )
	{
		m_pTitleLabel->SetText( "#asw_weapon_details_firepower" );
	}
	else if ( m_eWeaponFactType == RD_Swarmopedia::WeaponFact::Type_T::FireRate )
	{
		m_pTitleLabel->SetText( "#asw_weapon_details_firerate" );
	}
	else if ( m_eWeaponFactType == RD_Swarmopedia::WeaponFact::Type_T::ReloadTime )
	{
		m_pTitleLabel->SetText( "#asw_weapon_details_reload" );
	}
	else if ( m_eWeaponFactType == RD_Swarmopedia::WeaponFact::Type_T::Ammo )
	{
		m_pTitleLabel->SetText( "#asw_weapon_details_clipsize" );
	}
	else if ( m_eWeaponFactType == RD_Swarmopedia::WeaponFact::Type_T::Secondary )
	{
		m_pTitleLabel->SetText( "#asw_weapon_details_altfire" );
	}

	if ( !pFact )
	{
		m_pValueLabel->SetVisible( true );
		m_pValueLabel->SetText( m_eWeaponFactType == RD_Swarmopedia::WeaponFact::Type_T::Secondary ? "#asw_weapon_altfire_none" : "#asw_weapon_altfire_NA" );
		return;
	}

	float flBase, flSkill;
	pFact->ComputeBaseAndSkill( flBase, flSkill, pProfile );

	if ( m_eWeaponFactType == RD_Swarmopedia::WeaponFact::Type_T::DamagePerShot )
	{
		const RD_Swarmopedia::WeaponFact *pPellets = NULL;
		FOR_EACH_VEC( pWeapon->Facts, i )
		{
			if ( pWeapon->Facts[i]->Type == RD_Swarmopedia::WeaponFact::Type_T::ShotgunPellets )
			{
				pPellets = pWeapon->Facts[i];
				break;
			}
		}

		if ( pPellets )
		{
			float flBasePellets, flSkillPellets;
			pPellets->ComputeBaseAndSkill( flBasePellets, flSkillPellets, pProfile );

			int iPellets = flBasePellets + flSkillPellets;

			flBase *= iPellets;
			flSkill *= iPellets;
		}

		wchar_t wzDamValue[10];
		if ( flBase + flSkill <= 0 )
		{
			V_snwprintf( wzDamValue, ARRAYSIZE( wzDamValue ), L"%s", g_pVGuiLocalize->Find( "#asw_weapon_altfire_NA" ) );
		}
		else if ( flSkill > 0 )
		{
			V_snwprintf( wzDamValue, ARRAYSIZE( wzDamValue ), L"%d (+%d)", int( flBase + flSkill ), int( flSkill ) );
		}
		else
		{
			V_snwprintf( wzDamValue, ARRAYSIZE( wzDamValue ), L"%d", int( flBase ) );
		}

		m_pValueLabel->SetVisible( true );
		m_pValueLabel->SetText( wzDamValue );

		float flDamage = flBase + flSkill;
		float flCurrent = 0.0f;
		if ( flDamage > 60 )
			flCurrent = 1.0f;
		else if ( flBase > 0 )
			flCurrent = MAX( ( flDamage - 5.0f ) / 55.0f, 0.05f );

		if ( flDamage <= 0.0f )
		{
			m_pValueLabel->SetText( "#asw_weapon_altfire_NA" );
		}
		else
		{
			m_pStatsBar->SetVisible( true );
			m_pStatsBar->Init( flCurrent, flCurrent, 0.1f, false, false );
		}
	}
	else if ( m_eWeaponFactType == RD_Swarmopedia::WeaponFact::Type_T::FireRate )
	{
		wchar_t wzFireValue[32];

		float flTotal = flBase + flSkill;

		float flCurrent = 0.0f;
		if ( flTotal <= 0 )
		{
			V_snwprintf( wzFireValue, ARRAYSIZE( wzFireValue ), L"%s", g_pVGuiLocalize->Find( "#asw_weapon_altfire_NA" ) );
		}
		else
		{
			float flTotalReciprocal = 1.0f / flTotal;
			float flBaseReciprocal = 1.0f / flBase;

			if ( flSkill == 0.0f )
			{
				V_snwprintf( wzFireValue, ARRAYSIZE( wzFireValue ), L"%.1f / %s", flTotalReciprocal, g_pVGuiLocalize->Find( "#asw_weapon_details_seconds" ) );
			}
			else
			{
				V_snwprintf( wzFireValue, ARRAYSIZE( wzFireValue ), L"%.1f (%+.1f) / %s", flTotalReciprocal, flTotalReciprocal - flBaseReciprocal, g_pVGuiLocalize->Find( "#asw_weapon_details_seconds" ) );
			}

			if ( flTotal <= 0.125f )
				flCurrent = 1.0f - MIN( ( ( flTotal - 0.03f ) / 0.125f ) * 0.5f, 0.48f );
			else
				flCurrent = 1.0f - MIN( ( ( flTotal - 0.5f ) / 0.65f ) + 0.5f, 0.98f );
		}

		m_pValueLabel->SetVisible( true );
		m_pValueLabel->SetText( wzFireValue );
		m_pStatsBar->SetVisible( true );
		m_pStatsBar->Init( flCurrent, flCurrent, 0.1f, false, false );
	}
	else if ( m_eWeaponFactType == RD_Swarmopedia::WeaponFact::Type_T::ReloadTime )
	{
		wchar_t wszReloadValue[32];
		if ( flSkill != 0.0f )
			V_snwprintf( wszReloadValue, ARRAYSIZE( wszReloadValue ), L"%.1f (%+.1f) %s", flBase + flSkill, flSkill, g_pVGuiLocalize->Find( "#asw_weapon_details_seconds" ) );
		else
			V_snwprintf( wszReloadValue, ARRAYSIZE( wszReloadValue ), L"%.1f %s", flBase, g_pVGuiLocalize->Find( "#asw_weapon_details_seconds" ) );

		float flCurrent = 1.0f - MIN( ( flBase + flSkill - 0.75f ) / 2.5f, 0.99f );

		m_pValueLabel->SetVisible( true );
		m_pValueLabel->SetText( wszReloadValue );
		m_pStatsBar->SetVisible( true );
		m_pStatsBar->Init( flCurrent, flCurrent, 0.1f, false, false );
	}
	else if ( m_eWeaponFactType == RD_Swarmopedia::WeaponFact::Type_T::Ammo )
	{
		int iClipValue = pItem->MaxAmmo1();
		int nMaxBulletsPerGun = GetAmmoDef()->MaxCarry( pItem->m_iAmmo1, NULL );
		Assert( nMaxBulletsPerGun % iClipValue == 0 );
		int iNumClips = ( nMaxBulletsPerGun / iClipValue ) + 1;

		if ( pWeaponData->m_iDisplayClipSize >= 0 )
			iClipValue = pWeaponData->m_iDisplayClipSize;

		wchar_t wszClipValue[10];
		if ( iClipValue == 111 ) // magic number for infinite ammo
		{
			// this displays an "infinity" symbol in the neosans font
			V_snwprintf( wszClipValue, ARRAYSIZE( wszClipValue ), L"\u221E" );
		}
		else if ( pWeaponData->m_bShowClipsInWeaponDetail )
		{
			V_snwprintf( wszClipValue, ARRAYSIZE( wszClipValue ), L"%d \u00d7 %d", iClipValue, iNumClips );
		}
		else
		{
			V_snwprintf( wszClipValue, ARRAYSIZE( wszClipValue ), L"%d", iClipValue );
		}
		float flCurrent = MIN( ( float )iClipValue / 200.0f, 1.0f );

		m_pValueLabel->SetVisible( true );
		m_pValueLabel->SetText( wszClipValue );
		m_pStatsBar->SetVisible( true );
		m_pStatsBar->Init( flCurrent, flCurrent, 0.1f, false, false );
	}
	else if ( m_eWeaponFactType == RD_Swarmopedia::WeaponFact::Type_T::Secondary )
	{
		wchar_t wszAltValue[64];

		int iAltFire = pItem->DefaultAmmo2();
		if ( iAltFire > 0 )
			V_snwprintf( wszAltValue, ARRAYSIZE( wszAltValue ), L"%d %s", iAltFire, g_pVGuiLocalize->Find( pItem->m_szAltFireDescription ) );
		else
			V_snwprintf( wszAltValue, ARRAYSIZE( wszAltValue ), L"%s", g_pVGuiLocalize->Find( pItem->m_szAltFireDescription ) );

		m_pValueLabel->SetVisible( true );
		m_pValueLabel->SetText( wszAltValue );
	}
}
