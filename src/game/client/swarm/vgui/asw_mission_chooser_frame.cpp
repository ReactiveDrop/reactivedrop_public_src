#include "cbase.h"
#include "vgui/ivgui.h"
#include <vgui/vgui.h>
#include <vgui/ischeme.h>
#include <vgui_controls/Controls.h>
#include <vgui_controls/PropertySheet.h>
#include "convar.h"
#include "asw_mission_chooser_list.h"
#include "asw_mission_chooser_frame.h"
#include "ServerOptionsPanel.h"
#include <vgui/isurface.h>
#include <vgui/IInput.h>
#include "vgui_controls/AnimationController.h"
#include "ienginevgui.h"
#include "missionchooser/iasw_mission_chooser.h"
#include "clientmode_asw.h"
#include "c_asw_voting_mission_chooser_source.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

const char *g_ASW_ChooserTypeName[] =
{
	"campaign",
	"saved_campaign",
	"single_mission",
	"bonus_mission",
	"deathmatch",
};

const char *g_ASW_HostTypeName[] =
{
	"singleplayer",
	"createserver",
	"callvote",
};

CASW_Mission_Chooser_Frame::CASW_Mission_Chooser_Frame( vgui::Panel *pParent, const char *pElementName,
	ASW_HOST_TYPE iHostType, IASW_Mission_Chooser_Source *pMissionSource ) :
	Frame( pParent, pElementName )
{
	SetProportional( false );
	m_pMissionSource = pMissionSource;

	// create a propertysheet
	m_pSheet = new vgui::PropertySheet( this, "MissionChooserSheet" );
	m_pSheet->SetProportional( false );
	m_pSheet->SetVisible( true );

	extern ConVar sv_gametypes;

	CUtlStringList AllowedGameTypes;
	V_SplitString( sv_gametypes.GetString(), ",", AllowedGameTypes );

	// create our lists and put them in as pages in the property sheet
	FOR_EACH_VEC( AllowedGameTypes, i )
	{
		if ( !V_stricmp( AllowedGameTypes[i], g_ASW_ChooserTypeName[int( ASW_CHOOSER_TYPE::CAMPAIGN )] ) )
		{
			CASW_Mission_Chooser_List *pChooserList = new CASW_Mission_Chooser_List( m_pSheet, "CampaignList", ASW_CHOOSER_TYPE::CAMPAIGN, iHostType, pMissionSource );
			m_pSheet->AddPage( pChooserList, "#asw_start_campaign" );
			m_ChooserLists.AddToTail( pChooserList );
		}
		else if ( !V_stricmp( AllowedGameTypes[i], g_ASW_ChooserTypeName[int( ASW_CHOOSER_TYPE::SAVED_CAMPAIGN )] ) )
		{
			// refresh these each time we bring up the panel, since new saves get created during play
			pMissionSource->RefreshSavedCampaigns();

			CASW_Mission_Chooser_List *pChooserList = new CASW_Mission_Chooser_List( m_pSheet, "SavedCampaignList", ASW_CHOOSER_TYPE::SAVED_CAMPAIGN, iHostType, pMissionSource );
			m_pSheet->AddPage( pChooserList, "#asw_load_campaign" );
			m_ChooserLists.AddToTail( pChooserList );
		}
		else if ( !V_stricmp( AllowedGameTypes[i], g_ASW_ChooserTypeName[int( ASW_CHOOSER_TYPE::SINGLE_MISSION )] ) )
		{
			CASW_Mission_Chooser_List *pChooserList = new CASW_Mission_Chooser_List( m_pSheet, "MissionList", ASW_CHOOSER_TYPE::SINGLE_MISSION, iHostType, pMissionSource );
			m_pSheet->AddPage( pChooserList, "#asw_single_mission" );
			m_ChooserLists.AddToTail( pChooserList );
		}
		else if ( !V_stricmp( AllowedGameTypes[i], g_ASW_ChooserTypeName[int( ASW_CHOOSER_TYPE::BONUS_MISSION )] ) )
		{
			CASW_Mission_Chooser_List *pChooserList = new CASW_Mission_Chooser_List( m_pSheet, "BonusMissionList", ASW_CHOOSER_TYPE::BONUS_MISSION, iHostType, pMissionSource );
			m_pSheet->AddPage( pChooserList, "#rd_campaign_name_rd_bonus_missions" );
			m_ChooserLists.AddToTail( pChooserList );
		}
		else if ( !V_stricmp( AllowedGameTypes[i], g_ASW_ChooserTypeName[int( ASW_CHOOSER_TYPE::DEATHMATCH )] ) )
		{
			CASW_Mission_Chooser_List *pChooserList = new CASW_Mission_Chooser_List( m_pSheet, "DeathmatchList", ASW_CHOOSER_TYPE::DEATHMATCH, iHostType, pMissionSource );
			m_pSheet->AddPage( pChooserList, "#rd_campaign_name_deathmatch_campaign" );
			m_ChooserLists.AddToTail( pChooserList );
		}
		else
		{
			AssertMsg1( false, "unhandled game type from sv_gametypes: %s", AllowedGameTypes[i] );
		}
	}

	if ( iHostType == ASW_HOST_TYPE::CREATESERVER )
	{
		m_pOptionsPanel = new ServerOptionsPanel( this, "OptionsPanel" );
		m_pSheet->AddPage( m_pOptionsPanel, "#asw_server_options" );
		FOR_EACH_VEC( m_ChooserLists, i )
		{
			m_ChooserLists[i]->m_pServerOptions = m_pOptionsPanel;
		}
	}
	else
	{
		// no need for server options if they're just playing singleplayer
		m_pOptionsPanel = NULL;
	}

	vgui::HScheme scheme = vgui::scheme()->LoadSchemeFromFileEx( 0, "resource/SwarmFrameScheme.res", "SwarmFrameScheme" );
	SetScheme( scheme );

	switch ( iHostType )
	{
	case ASW_HOST_TYPE::SINGLEPLAYER:
		SetTitle( "#asw_menu_singleplayer", true );
		break;
	case ASW_HOST_TYPE::CREATESERVER:
		SetTitle( "#asw_menu_create_server", true );
		break;
	case ASW_HOST_TYPE::CALLVOTE:
		SetTitle( "#asw_vote_server", true );
		break;
	default:
		AssertMsg( false, "unexpected missionchooser host type" );
		break;
	}

	SetTitleBarVisible( true );
	SetMoveable( false );
	SetSizeable( false );
	SetMenuButtonVisible( false );
	SetMaximizeButtonVisible( false );
	SetMinimizeToSysTrayButtonVisible( false );
	SetMinimizeButtonVisible( false );
	SetCloseButtonVisible( true );

	RequestFocus();
	SetVisible( true );
	SetEnabled( true );

	InvalidateLayout( true, true );

	Msg( "CASW_Mission_Chooser_Frame\n" );
}

CASW_Mission_Chooser_Frame::~CASW_Mission_Chooser_Frame()
{
	Msg( "~CASW_Mission_Chooser_Frame\n" );
}

void CASW_Mission_Chooser_Frame::PerformLayout()
{
	SetPos( 100, 10 );
	int sw, sh;
	vgui::surface()->GetScreenSize( sw, sh );
	SetSize( sw - 200, sh - 20 );

	BaseClass::PerformLayout();

	int x, y, wide, tall;
	GetClientArea( x, y, wide, tall );
	m_pSheet->SetBounds( x, y, wide, tall );

	m_pSheet->SetVisible( true );
	m_pSheet->InvalidateLayout( true );
	//m_pSheet->SetTabWidth(GetWide()/3.0f);

	float top_edge = 30;
	FOR_EACH_VEC( m_ChooserLists, i )
	{
		m_ChooserLists[i]->SetPos( 0, top_edge );
		m_ChooserLists[i]->SetSize( wide, GetTall() - ( 60 ) );
		m_ChooserLists[i]->InvalidateLayout( true );
	}

	if ( m_pOptionsPanel )
		m_pOptionsPanel->SetBounds( x, top_edge, wide - x * 2, GetTall() - top_edge );
}

void CASW_Mission_Chooser_Frame::OnThink()
{
	BaseClass::OnThink();
}

void CASW_Mission_Chooser_Frame::OnClose()
{
	BaseClass::OnClose();
	SetVisible( false );	// have to do this, as the fading transparency causes sorting issues in the briefing
}

void CASW_Mission_Chooser_Frame::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );
}

vgui::DHANDLE<CASW_Mission_Chooser_Frame> g_hChooserFrame;
static void LaunchMissionChooser( ASW_HOST_TYPE iHostType )
{
	CASW_Mission_Chooser_Frame *pFrame = g_hChooserFrame;
	if ( pFrame )
	{
		pFrame->SetVisible( false );
		pFrame->MarkForDeletion();
		g_hChooserFrame = NULL;
	}

	if ( iHostType == ASW_HOST_TYPE::NUM_TYPES )
		return;

	IASW_Mission_Chooser_Source *pSource = missionchooser ? missionchooser->LocalMissionSource() : NULL;
	if ( iHostType == ASW_HOST_TYPE::CALLVOTE )
		pSource = GetVotingMissionSource();

	if ( !pSource )
		return;

	pFrame = new CASW_Mission_Chooser_Frame( NULL, "MissionChooserFrame", iHostType, pSource );

	if ( engine->IsConnected() )
	{
		pFrame->SetParent( GetClientMode()->GetViewport() );
	}
	else
	{
		vgui::VPANEL rootpanel = enginevgui->GetPanel( PANEL_GAMEUIDLL );
		pFrame->SetParent( rootpanel );
	}

	g_hChooserFrame = pFrame;
}

CON_COMMAND( asw_mission_chooser, "asw_mission_chooser [host type] - open mission chooser" )
{
	if ( args.ArgC() != 2 )
	{
		ConMsg( "Usage: asw_mission_chooser [host type]\n" );
		return;
	}

	if ( !V_stricmp( args.Arg( 1 ), "exit" ) )
	{
		LaunchMissionChooser( ASW_HOST_TYPE::NUM_TYPES );
		return;
	}

	for ( int i = 0; i < int( ASW_HOST_TYPE::NUM_TYPES ); i++ )
	{
		if ( !V_stricmp( args.Arg( 1 ), g_ASW_HostTypeName[i] ) )
		{
			LaunchMissionChooser( ASW_HOST_TYPE( i ) );
			return;
		}
	}

	ConMsg( "Invalid host type. Host types are:\n" );
	for ( int i = 0; i < int( ASW_HOST_TYPE::NUM_TYPES ); i++ )
	{
		ConMsg( "  %s\n", g_ASW_HostTypeName[i] );
	}
}
