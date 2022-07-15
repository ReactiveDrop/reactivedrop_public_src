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
	m_nForceReLayout = 0;
	m_pImage = new vgui::ImagePanel( this, "Image" );
	m_pBackdrop = new vgui::Panel( this, "Backdrop" );
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

	m_pBackdrop->SetPaintBackgroundEnabled( true );
	m_pBackdrop->SetBgColor( Color( 0, 0, 0, 224 ) );

}

void CASW_Mission_Chooser_Details::PerformLayout()
{
	BaseClass::PerformLayout();

	int discard, y0, y1, tall, titleTallDiff;
	m_pTitle->GetContentSize( discard, tall );
	titleTallDiff = tall - m_pTitle->GetTall();
	m_pTitle->SetTall( tall );
	m_pBackdrop->GetPos( discard, y0 );
	m_pDescription->GetPos( discard, y1 );
	y1 += titleTallDiff;
	m_pDescription->SetPos( discard, y1 );
	m_pDescription->GetContentSize( discard, tall );
	m_pBackdrop->SetTall( tall + m_pTitle->GetTall() / 2 + y1 - y0 );

	if ( m_nForceReLayout )
	{
		m_nForceReLayout--;
		InvalidateLayout();
	}
}

void CASW_Mission_Chooser_Details::HighlightEntry( CASW_Mission_Chooser_Entry *pEntry )
{
	m_nForceReLayout = 1;

	if ( !pEntry )
	{
		m_pImage->SetVisible( false );
		m_pBackdrop->SetVisible( false );
		m_pTitle->SetVisible( false );
		m_pDescription->SetVisible( false );
		m_pMapBase->SetVisible( false );
		m_pMapLayer[0]->SetVisible( false );
		m_pMapLayer[1]->SetVisible( false );
		m_pMapLayer[2]->SetVisible( false );
		m_pSearchLights->SetVisible( false );
		m_pLastEntry = NULL;

		InvalidateLayout();

		return;
	}

	// make sure we don't immediately clear this entry.
	OnThink();

	m_pImage->SetVisible( true );
	m_pBackdrop->SetVisible( true );
	m_pTitle->SetVisible( true );
	m_pDescription->SetVisible( true );
	m_pMapBase->SetVisible( true );

	if ( pEntry->m_szMission[0] )
	{
		const RD_Mission_t *pMission = ReactiveDropMissions::GetMission( pEntry->m_szMission );
		Assert( pMission );
		if ( !pMission )
		{
			HighlightEntry( NULL );
			return;
		}

		m_pImage->SetImage( STRING( pMission->Image ) );
		m_pTitle->SetText( STRING( pMission->MissionTitle ) );
		m_pDescription->SetText( STRING( pMission->Description ) );
		m_pMapBase->SetImage( CFmtStr( "../%s", STRING( pMission->BriefingMaterial ) ) );
		m_pMapLayer[0]->SetVisible( false );
		m_pMapLayer[1]->SetVisible( false );
		m_pMapLayer[2]->SetVisible( false );
		m_pSearchLights->SetVisible( false );
		m_pLastEntry = pEntry;

		InvalidateLayout();

		return;
	}
	
	if ( pEntry->m_szCampaign[0] )
	{
		const RD_Campaign_t *pCampaign = ReactiveDropMissions::GetCampaign( pEntry->m_szCampaign );
		Assert( pCampaign );
		if ( !pCampaign )
		{
			HighlightEntry( NULL );
			return;
		}

		m_pImage->SetImage( STRING( pCampaign->ChooseCampaignTexture ) );
		m_pTitle->SetText( STRING( pCampaign->CampaignName ) );
		m_pDescription->SetText( STRING( pCampaign->CampaignDescription ) );
		m_pMapBase->SetImage( STRING( pCampaign->CampaignTextureName ) );
		m_pMapLayer[0]->SetVisible( true );
		m_pMapLayer[1]->SetVisible( true );
		m_pMapLayer[2]->SetVisible( true );
		m_pMapLayer[0]->SetImage( STRING( pCampaign->CampaignTextureLayer[0] ) );
		m_pMapLayer[1]->SetImage( STRING( pCampaign->CampaignTextureLayer[1] ) );
		m_pMapLayer[2]->SetImage( STRING( pCampaign->CampaignTextureLayer[2] ) );
		m_pSearchLights->SetVisible( true );
		m_pSearchLights->SetCampaign( pCampaign );
		m_pLastEntry = pEntry;

		InvalidateLayout();

		return;
	}

	// not a campaign or a mission; remove overview (it's a placeholder)
	HighlightEntry( NULL );
	m_pLastEntry = pEntry;
}
