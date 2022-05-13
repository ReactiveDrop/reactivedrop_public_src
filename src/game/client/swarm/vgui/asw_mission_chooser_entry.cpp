#include "cbase.h"
#include "asw_mission_chooser_frame.h"
#include "asw_mission_chooser_list.h"
#include "asw_mission_chooser_entry.h"
#include "asw_mission_chooser_details.h"
#include "rd_missions_shared.h"
#include "rd_workshop.h"
#include "gameui/swarm/basemodui.h"
#include <vgui_controls/Label.h>
#include <vgui_controls/ImagePanel.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CASW_Mission_Chooser_Entry_FocusHolder : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CASW_Mission_Chooser_Entry_FocusHolder, vgui::EditablePanel );
public:
	CASW_Mission_Chooser_Entry_FocusHolder( vgui::Panel *pParent, const char *pElementName ) : BaseClass( pParent, pElementName )
	{
		SetConsoleStylePanel( true );

		m_bMousePressed = false;
	}
	virtual ~CASW_Mission_Chooser_Entry_FocusHolder()
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

		CASW_Mission_Chooser_Entry *pParent = assert_cast< CASW_Mission_Chooser_Entry * >( GetParent() );
		pParent->m_pHighlight->SetVisible( true );
		pParent->m_pList->m_pFrame->m_pDetails->HighlightEntry( pParent );
	}

	virtual void OnKillFocus()
	{
		BaseClass::OnKillFocus();

		CASW_Mission_Chooser_Entry *pParent = assert_cast< CASW_Mission_Chooser_Entry * >( GetParent() );
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
			CASW_Mission_Chooser_Entry *pParent = assert_cast< CASW_Mission_Chooser_Entry * >( GetParent() );
			m_bMousePressed = false;
			pParent->m_pList->m_pFrame->ApplyEntry( pParent );
			return;
		}

		BaseClass::OnMouseReleased( code );
	}

	bool m_bMousePressed;
};

CASW_Mission_Chooser_Entry::CASW_Mission_Chooser_Entry( vgui::Panel *pParent, const char *pElementName, CASW_Mission_Chooser_List *pList, const RD_Campaign_t *pCampaign, const RD_Mission_t *pMission ) : BaseClass( pParent, pElementName )
{
	SetConsoleStylePanel( true );

	m_pList = pList;
	m_pCampaign = pCampaign;
	m_pMission = pMission;

	m_pFocusHolder = new CASW_Mission_Chooser_Entry_FocusHolder( this, "FocusHolder" );
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

	bool bIsSubscribed = false;
	if ( m_pMission )
	{
		m_pImage->SetImage( STRING( m_pMission->Image ) );
		m_pTitle->SetText( STRING( m_pMission->MissionTitle ) );
		bIsSubscribed = m_pMission->WorkshopID == k_PublishedFileIdInvalid || g_ReactiveDropWorkshop.IsSubscribedToFile( m_pMission->WorkshopID );
	}
	else if ( m_pCampaign )
	{
		m_pImage->SetImage( STRING( m_pCampaign->ChooseCampaignTexture ) );
		m_pTitle->SetText( STRING( m_pCampaign->CampaignName ) );
		bIsSubscribed = m_pCampaign->WorkshopID == k_PublishedFileIdInvalid || g_ReactiveDropWorkshop.IsSubscribedToFile( m_pCampaign->WorkshopID );
	}

	if ( bIsSubscribed )
	{
		SetAlpha( 255 );
	}
	else
	{
		SetAlpha( 96 );
	}
}
