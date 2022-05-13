#include "cbase.h"
#include "asw_mission_chooser_frame.h"
#include "asw_mission_chooser_list.h"
#include "asw_mission_chooser_entry.h"
#include "rd_missions_shared.h"
#include "gameui/swarm/basemodui.h"
#include <vgui_controls/Label.h>
#include <vgui_controls/ImagePanel.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CASW_Mission_Chooser_Entry::CASW_Mission_Chooser_Entry( vgui::Panel *pParent, const char *pElementName, ASW_CHOOSER_TYPE iChooserType, ASW_HOST_TYPE iHostType, const RD_Campaign_t *pCampaign, const RD_Mission_t *pMission ) : BaseClass( pParent, pElementName )
{
	SetConsoleStylePanel( true );

	m_ChooserType = iChooserType;
	m_HostType = iHostType;
	m_pCampaign = pCampaign;
	m_pMission = pMission;

	m_pHighlight = new vgui::Panel( this, "Highlight" );
	m_pImage = new vgui::ImagePanel( this, "Image" );
	m_pTitle = new vgui::Label( this, "Title", "" );
}

CASW_Mission_Chooser_Entry::~CASW_Mission_Chooser_Entry()
{
}

void CASW_Mission_Chooser_Entry::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	LoadControlSettings( "Resource/UI/MissionChooserEntry.res" );

	BaseClass::ApplySchemeSettings( pScheme );

	if ( m_pMission )
	{
		m_pImage->SetImage( STRING( m_pMission->Image ) );
		m_pTitle->SetText( STRING( m_pMission->MissionTitle ) );
	}
	else if ( m_pCampaign )
	{
		m_pImage->SetImage( STRING( m_pCampaign->ChooseCampaignTexture ) );
		m_pTitle->SetText( STRING( m_pCampaign->CampaignName ) );
	}
}

void CASW_Mission_Chooser_Entry::NavigateTo()
{
	BaseClass::NavigateTo();

	RequestFocus();
}

void CASW_Mission_Chooser_Entry::OnSetFocus()
{
	BaseClass::OnSetFocus();

	m_pHighlight->SetVisible( true );
}

void CASW_Mission_Chooser_Entry::OnKillFocus()
{
	BaseClass::OnKillFocus();

	m_pHighlight->SetVisible( false );
}
