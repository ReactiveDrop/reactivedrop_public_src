#include "cbase.h"
#include "asw_mission_chooser_entry.h"
#include "asw_mission_chooser_details.h"
#include "rd_missions_shared.h"
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Label.h>
#include "campaignmapsearchlights.h"
#include "fmtstr.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CASW_Mission_Chooser_Details::CASW_Mission_Chooser_Details( vgui::Panel *pParent, const char *pElementName ) : BaseClass( pParent, pElementName )
{
	m_nDataResets = 0;
	m_pImage = new vgui::ImagePanel( this, "Image" );
	m_pTitle = new vgui::Label( this, "Title", "" );
	m_pDescription = new vgui::Label( this, "Description", "" );
	m_pMapBase = new vgui::ImagePanel( this, "MapBase" );
	m_pMapLayer[0] = new vgui::ImagePanel( this, "MapLayer1" );
	m_pMapLayer[1] = new vgui::ImagePanel( this, "MapLayer2" );
	m_pMapLayer[2] = new vgui::ImagePanel( this, "MapLayer3" );
	m_pSearchLights = new CampaignMapSearchLights( this, "MapSearchLights" );

	HighlightEntry( NULL );
}

CASW_Mission_Chooser_Details::~CASW_Mission_Chooser_Details()
{
}

void CASW_Mission_Chooser_Details::OnThink()
{
	BaseClass::OnThink();

	// make sure data reset count is up to date.
	ReactiveDropMissions::GetCampaign( -1 );

	if ( ReactiveDropMissions::s_nDataResets == m_nDataResets )
	{
		return;
	}

	m_nDataResets = ReactiveDropMissions::s_nDataResets;

	HighlightEntry( NULL );
}

void CASW_Mission_Chooser_Details::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	LoadControlSettings( "Resource/UI/MissionChooserDetails.res" );

	BaseClass::ApplySchemeSettings( pScheme );
}

void CASW_Mission_Chooser_Details::PerformLayout()
{
	BaseClass::PerformLayout();
}

void CASW_Mission_Chooser_Details::HighlightEntry( CASW_Mission_Chooser_Entry *pEntry )
{
	if ( !pEntry )
	{
		m_pImage->SetVisible( false );
		m_pTitle->SetVisible( false );
		m_pDescription->SetVisible( false );
		m_pMapBase->SetVisible( false );
		m_pMapLayer[0]->SetVisible( false );
		m_pMapLayer[1]->SetVisible( false );
		m_pMapLayer[2]->SetVisible( false );
		m_pSearchLights->SetVisible( false );

		return;
	}

	// make sure we don't immediately clear this entry.
	OnThink();

	m_pImage->SetVisible( true );
	m_pTitle->SetVisible( true );
	m_pDescription->SetVisible( true );
	m_pMapBase->SetVisible( true );

	if ( pEntry->m_pMission )
	{
		m_pImage->SetImage( STRING( pEntry->m_pMission->Image ) );
		m_pTitle->SetText( STRING( pEntry->m_pMission->MissionTitle ) );
		m_pDescription->SetText( STRING( pEntry->m_pMission->Description ) );
		m_pMapBase->SetImage( CFmtStr( "../%s", STRING( pEntry->m_pMission->BriefingMaterial ) ) );
		m_pMapLayer[0]->SetVisible( false );
		m_pMapLayer[1]->SetVisible( false );
		m_pMapLayer[2]->SetVisible( false );
		m_pSearchLights->SetVisible( false );

		return;
	}
	
	if ( pEntry->m_pCampaign )
	{
		m_pImage->SetImage( STRING( pEntry->m_pCampaign->ChooseCampaignTexture ) );
		m_pTitle->SetText( STRING( pEntry->m_pCampaign->CampaignName ) );
		m_pDescription->SetText( STRING( pEntry->m_pCampaign->CampaignDescription ) );
		m_pMapBase->SetImage( STRING( pEntry->m_pCampaign->CampaignTextureName ) );
		m_pMapLayer[0]->SetVisible( true );
		m_pMapLayer[1]->SetVisible( true );
		m_pMapLayer[2]->SetVisible( true );
		m_pMapLayer[0]->SetImage( STRING( pEntry->m_pCampaign->CampaignTextureLayer[0] ) );
		m_pMapLayer[1]->SetImage( STRING( pEntry->m_pCampaign->CampaignTextureLayer[1] ) );
		m_pMapLayer[2]->SetImage( STRING( pEntry->m_pCampaign->CampaignTextureLayer[2] ) );
		m_pSearchLights->SetVisible( true );
		m_pSearchLights->SetCampaign( pEntry->m_pCampaign );

		return;
	}

	Assert( !"entry contains neither a campaign nor a mission" );
	HighlightEntry( NULL );
}
