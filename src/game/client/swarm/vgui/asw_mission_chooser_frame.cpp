#include "cbase.h"
#include "clientmode_asw.h"
#include "ienginevgui.h"
#include "asw_mission_chooser_frame.h"
#include "asw_mission_chooser_list.h"
#include "nb_header_footer.h"
#include "gameui/swarm/basemodui.h"
#include "vgui_controls/PropertySheet.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

const char *const g_ASW_ChooserTypeName[] =
{
	"campaign",
	"saved_campaign",
	"single_mission",
	"bonus_mission",
	"deathmatch",
};

static const char *const s_ChooserTabName[] =
{
	"#asw_start_campaign",
	"#asw_load_campaign",
	"#asw_single_mission",
	"#rd_campaign_name_rd_bonus_missions",
	"#rd_campaign_name_deathmatch_campaign",
};

const char *const g_ASW_HostTypeName[] =
{
	"singleplayer",
	"createserver",
	"callvote",
};

CASW_Mission_Chooser_Frame::CASW_Mission_Chooser_Frame( vgui::Panel *pParent, const char *pElementName, ASW_HOST_TYPE iHostType )
	: BaseClass( pParent, pElementName )
{
	SetConsoleStylePanel( true );
	SetProportional( true );
	SetSizeable( false );
	SetCloseButtonVisible( false );

	HScheme scheme = vgui::scheme()->LoadSchemeFromFile( "resource/SwarmSchemeNew.res", "SwarmSchemeNew" );
	SetScheme( scheme );

	m_pHeaderFooter = new CNB_Header_Footer( this, "HeaderFooter" );
	m_pHeaderFooter->SetGradientBarPos( 40, 410 );

	switch ( iHostType )
	{
	case ASW_HOST_TYPE::SINGLEPLAYER:
		m_pHeaderFooter->SetTitle( "#asw_menu_singleplayer" );
		break;
	case ASW_HOST_TYPE::CREATESERVER:
		m_pHeaderFooter->SetTitle( "#asw_menu_create_server" );
		break;
	case ASW_HOST_TYPE::CALLVOTE:
		m_pHeaderFooter->SetTitle( "#asw_vote_server" );
		break;
	default:
		Assert( !"unexpected missionchooser host type" );
		break;
	}

	m_pSheet = new vgui::PropertySheet( this, "TabSheet" );
	m_pSheet->SetKBNavigationEnabled( false );

	extern ConVar sv_gametypes;
	CSplitString AllowedGameTypes( sv_gametypes.GetString(), "," );

	// create our lists and put them in as pages in the property sheet
	FOR_EACH_VEC( AllowedGameTypes, i )
	{
		for ( int j = 0; j < NELEMS( g_ASW_ChooserTypeName ); j++ )
		{
			if ( V_stricmp( AllowedGameTypes[i], g_ASW_ChooserTypeName[j] ) )
			{
				continue;
			}

			CASW_Mission_Chooser_List *pChooserList = new CASW_Mission_Chooser_List( m_pSheet, "MissionChooserList", ASW_CHOOSER_TYPE( j ), iHostType );
			m_pSheet->AddPage( pChooserList, s_ChooserTabName[j] );
			m_ChooserLists.AddToTail( pChooserList );
		}
	}
}

CASW_Mission_Chooser_Frame::~CASW_Mission_Chooser_Frame()
{
}

void CASW_Mission_Chooser_Frame::OnCommand( const char *command )
{
	if ( !V_stricmp( command, "BackButton" ) )
	{
		BaseModUI::CBaseModPanel::GetSingleton().PlayUISound( BaseModUI::UISOUND_BACK );
		MarkForDeletion();
	}
	else
	{
		BaseClass::OnCommand( command );
	}
}

void CASW_Mission_Chooser_Frame::OnKeyCodePressed( vgui::KeyCode keycode )
{
	int lastUser = GetJoystickForCode( keycode );
	BaseModUI::CBaseModPanel::GetSingleton().SetLastActiveUserId( lastUser );

	vgui::KeyCode code = GetBaseButtonCode( keycode );

	switch ( code )
	{
	case KEY_ESCAPE:
	case KEY_XBUTTON_B:
		OnCommand( "BackButton" );
		break;
	case KEY_XBUTTON_LEFT_SHOULDER:
		if ( m_pSheet->GetActivePageNum() > 0 )
		{
			m_pSheet->SetActivePage( m_pSheet->GetPage( m_pSheet->GetActivePageNum() - 1 ) );
		}
		break;
	case KEY_XBUTTON_RIGHT_SHOULDER:
		if ( m_pSheet->GetActivePageNum() < m_pSheet->GetNumPages() - 1 )
		{
			m_pSheet->SetActivePage( m_pSheet->GetPage( m_pSheet->GetActivePageNum() + 1 ) );
		}
		break;
	default:
		BaseClass::OnKeyCodePressed( keycode );
		break;
	}
}

void CASW_Mission_Chooser_Frame::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	LoadControlSettings( "Resource/UI/MissionChooserFrame.res" );

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

	pFrame = new CASW_Mission_Chooser_Frame( NULL, "MissionChooserFrame", iHostType );

	if ( engine->IsConnected() )
	{
		pFrame->SetParent( GetClientMode()->GetViewport() );
	}
	else
	{
		vgui::VPANEL rootpanel = enginevgui->GetPanel( PANEL_GAMEUIDLL );
		pFrame->SetParent( rootpanel );
	}

	pFrame->InvalidateLayout();
	pFrame->SetVisible( true );

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
