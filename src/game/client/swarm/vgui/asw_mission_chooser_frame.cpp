#include "cbase.h"
#include "clientmode_asw.h"
#include "ienginevgui.h"
#include "asw_mission_chooser_frame.h"
#include "asw_mission_chooser_list.h"
#include "asw_mission_chooser_entry.h"
#include "asw_mission_chooser_details.h"
#include "nb_header_footer.h"
#include "gameui/swarm/basemodui.h"
#include <vgui_controls/PropertySheet.h>
#include "rd_missions_shared.h"
#include "rd_workshop.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar rd_last_game_access;

const char *const g_ASW_ChooserTypeName[] =
{
	"campaign",
	"saved_campaign",
	"single_mission",
	"bonus_mission",
	"deathmatch",
	"endless",
};

static const char *const s_ChooserTabName[] =
{
	"#asw_start_campaign",
	"#asw_load_campaign",
	"#asw_single_mission",
	"#rd_campaign_name_rd_bonus_missions",
	"#rd_campaign_name_deathmatch_campaign",
	"#rd_endless_missions",
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

	m_HostType = iHostType;
	m_bViewingCampaign = false;

	m_pHeaderFooter = new CNB_Header_Footer( this, "HeaderFooter" );
	m_pHeaderFooter->SetGradientBarPos( 40, 410 );

	switch ( iHostType )
	{
	case ASW_HOST_TYPE::SINGLEPLAYER:
		m_pHeaderFooter->SetTitle( "#nb_select_mission" );
		break;
	case ASW_HOST_TYPE::CREATESERVER:
		m_pHeaderFooter->SetTitle( "#nb_select_mission" );
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

			CASW_Mission_Chooser_List *pChooserList = new CASW_Mission_Chooser_List( m_pSheet, "MissionChooserList", ASW_CHOOSER_TYPE( j ), this );
			m_pSheet->AddPage( pChooserList, s_ChooserTabName[j] );
		}
	}

	m_pDetails = new CASW_Mission_Chooser_Details( this, "MissionChooserDetails" );

	m_pCampaignMissionList = new vgui::PropertySheet( this, "CampaignMissionList" );
	m_pCampaignMissionList->SetKBNavigationEnabled( false );
}

CASW_Mission_Chooser_Frame::~CASW_Mission_Chooser_Frame()
{
}

void CASW_Mission_Chooser_Frame::OnCommand( const char *command )
{
	if ( !V_stricmp( command, "BackButton" ) )
	{
		BaseModUI::CBaseModPanel::GetSingleton().PlayUISound( BaseModUI::UISOUND_BACK );
		if ( m_bViewingCampaign )
		{
			m_pCampaignMissionList->SetVisible( false );
			m_pSheet->SetVisible( true );
			m_pSheet->InvalidateLayout();
			m_bViewingCampaign = false;
		}
		else
		{
			MarkForDeletion();
		}
	}
	else if ( !V_stricmp( command, "ApplyCurrentEntry" ) )
	{
		CASW_Mission_Chooser_Entry *pEntry = m_pDetails->m_pLastEntry;
		if ( pEntry && pEntry->m_pFocusHolder )
		{
			pEntry->m_pFocusHolder->OnMousePressed( MOUSE_LEFT );
			pEntry->m_pFocusHolder->OnMouseReleased( MOUSE_LEFT );
		}
	}
	else if ( !V_stricmp( command, "PrevPage" ) )
	{
		if ( !m_bViewingCampaign && m_pSheet->GetActivePageNum() > 0 )
		{
			m_pSheet->SetActivePage( m_pSheet->GetPage( m_pSheet->GetActivePageNum() - 1 ) );
		}
	}
	else if ( !V_stricmp( command, "NextPage" ) )
	{
		if ( !m_bViewingCampaign && m_pSheet->GetActivePageNum() < m_pSheet->GetNumPages() - 1 )
		{
			m_pSheet->SetActivePage( m_pSheet->GetPage( m_pSheet->GetActivePageNum() + 1 ) );
		}
	}
	else if ( !V_stricmp( command, "CyclePage" ) )
	{
		if ( !m_bViewingCampaign )
		{
			m_pSheet->SetActivePage( m_pSheet->GetPage( ( m_pSheet->GetActivePageNum() + 1 ) % m_pSheet->GetNumPages() ) );
		}
	}
	else
	{
		BaseClass::OnCommand( command );
	}
}

void CASW_Mission_Chooser_Frame::OnKeyCodeTyped( vgui::KeyCode keycode )
{
	switch ( keycode )
	{
	case KEY_ESCAPE:
		OnCommand( "BackButton" );
		break;
	case KEY_ENTER:
		OnCommand( "ApplyCurrentEntry" );
		break;
	case KEY_PAGEUP:
		OnCommand( "PrevPage" );
		break;
	case KEY_PAGEDOWN:
		OnCommand( "NextPage" );
		break;
	case KEY_TAB:
		OnCommand( "CyclePage" );
		break;
	default:
		BaseClass::OnKeyCodeTyped( keycode );
		break;
	}
}

void CASW_Mission_Chooser_Frame::OnKeyCodePressed( vgui::KeyCode keycode )
{
	int lastUser = GetJoystickForCode( keycode );
	BaseModUI::CBaseModPanel::GetSingleton().SetLastActiveUserId( lastUser );

	vgui::KeyCode code = GetBaseButtonCode( keycode );

	switch ( code )
	{
	case KEY_XBUTTON_A:
		OnCommand( "ApplyCurrentEntry" );
		break;
	case KEY_XBUTTON_B:
		OnCommand( "BackButton" );
		break;
	case KEY_XBUTTON_LEFT_SHOULDER:
		OnCommand( "PrevPage" );
		break;
	case KEY_XBUTTON_RIGHT_SHOULDER:
		OnCommand( "NextPage" );
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

void CASW_Mission_Chooser_Frame::ApplyEntry( CASW_Mission_Chooser_Entry *pEntry )
{
	if ( pEntry->m_szMission[0] )
	{
		PublishedFileId_t iWorkshopID = ReactiveDropMissions::MissionWorkshopID( ReactiveDropMissions::GetMissionIndex( pEntry->m_szMission ) );
		if ( iWorkshopID && !g_ReactiveDropWorkshop.IsAddonEnabled( iWorkshopID ) )
		{
			g_ReactiveDropWorkshop.OpenWorkshopPageForFile( iWorkshopID );
			return;
		}

		if ( m_HostType == ASW_HOST_TYPE::CALLVOTE )
		{
			if ( pEntry->m_szCampaign[0] )
			{
				engine->ServerCmd( CFmtStr( "asw_vote_campaign %d %s", ReactiveDropMissions::GetCampaignIndex( pEntry->m_szCampaign ), pEntry->m_szMission ) );
			}
			else
			{
				engine->ServerCmd( CFmtStr( "asw_vote_mission %s", pEntry->m_szMission ) );
			}

			MarkForDeletion();
			return;
		}

		KeyValues::AutoDelete pSettings( "Settings" );

		if ( m_HostType == ASW_HOST_TYPE::SINGLEPLAYER )
		{
			pSettings->SetString( "system/network", "offline" );
		}
		else
		{
			Assert( m_HostType == ASW_HOST_TYPE::CREATESERVER );
			pSettings->SetString( "system/network", "LIVE" );
			pSettings->SetString( "system/access", rd_last_game_access.GetString() );
			pSettings->SetString( "options/action", "create" );
		}

		if ( pEntry->m_szCampaign[0] )
		{
			pSettings->SetString( "game/mode", "campaign" );
			pSettings->SetString( "game/campaign", pEntry->m_szCampaign );
		}
		else
		{
			pSettings->SetString( "game/mode", "single_mission" );
		}

		pSettings->SetString( "game/mission", pEntry->m_szMission );
		pSettings->SetString( "game/difficulty", GameModeGetDefaultDifficulty( pSettings->GetString( "game/mode" ) ) );

		BaseModUI::CBaseModPanel::GetSingleton().PlayUISound( BaseModUI::UISOUND_ACCEPT );
		BaseModUI::CBaseModPanel::GetSingleton().CloseAllWindows();

		if ( m_HostType == ASW_HOST_TYPE::SINGLEPLAYER )
		{
			g_pMatchFramework->CreateSession( pSettings );
		}
		else
		{
			BaseModUI::CBaseModPanel::GetSingleton().OpenWindow( BaseModUI::WT_GAMESETTINGS, NULL, true, pSettings );
		}

		MarkForDeletion();
		return;
	}

	Assert( pEntry->m_szCampaign[0] );
	if ( !pEntry->m_szCampaign[0] )
		return;

	ApplyCampaign( pEntry->m_pList->m_ChooserType, pEntry->m_szCampaign );
}

void CASW_Mission_Chooser_Frame::ApplyCampaign( ASW_CHOOSER_TYPE iChooserType, const char *szCampaignName )
{
	PublishedFileId_t iWorkshopID = ReactiveDropMissions::CampaignWorkshopID( ReactiveDropMissions::GetCampaignIndex( szCampaignName ) );
	if ( iWorkshopID && !g_ReactiveDropWorkshop.IsAddonEnabled( iWorkshopID ) )
	{
		g_ReactiveDropWorkshop.OpenWorkshopPageForFile( iWorkshopID );
		return;
	}

	m_bViewingCampaign = true;
	m_pSheet->SetVisible( false );

	while ( m_pCampaignMissionList->GetNumPages() )
	{
		m_pCampaignMissionList->DeletePage( m_pCampaignMissionList->GetPage( 0 ) );
	}

	m_pCampaignMissionList->SetVisible( true );
	m_pCampaignMissionList->AddPage( new CASW_Mission_Chooser_List( m_pCampaignMissionList, "MissionChooserList", iChooserType, this, szCampaignName ), "#nb_select_starting_mission" );
}

bool CASW_Mission_Chooser_Frame::SelectTab( ASW_CHOOSER_TYPE iChooserType )
{
	for ( int i = 0; i < m_pSheet->GetNumPages(); i++ )
	{
		CASW_Mission_Chooser_List *pList = assert_cast< CASW_Mission_Chooser_List * >( m_pSheet->GetPage( i ) );
		if ( pList->m_ChooserType == iChooserType )
		{
			m_pSheet->SetActivePage( pList );
			return true;
		}
	}

	return false;
}

vgui::DHANDLE<CASW_Mission_Chooser_Frame> g_hChooserFrame;
static void LaunchMissionChooser( ASW_HOST_TYPE iHostType, ASW_CHOOSER_TYPE iChooserType, const char *szCampaignName )
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

	pFrame->MakeReadyForUse();

	if ( iChooserType != ASW_CHOOSER_TYPE::NUM_TYPES )
	{
		pFrame->SelectTab( iChooserType );
	}

	if ( iChooserType == ASW_CHOOSER_TYPE::CAMPAIGN && szCampaignName )
	{
		pFrame->ApplyCampaign( iChooserType, szCampaignName );
	}

	pFrame->InvalidateLayout();
	pFrame->SetVisible( true );

	g_hChooserFrame = pFrame;
}

static int asw_mission_chooser_completion( const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH] )
{
	int count = 0;
	char szCandidate[COMMAND_COMPLETION_ITEM_LENGTH]{};

#define COMMAND_COMPLETION_CANDIDATE( pFormat, ... ) \
	if ( count < COMMAND_COMPLETION_MAXITEMS ) \
	{ \
		V_snprintf( szCandidate, sizeof(szCandidate), pFormat, __VA_ARGS__ ); \
		if ( StringHasPrefix( szCandidate, partial ) ) \
		{ \
			V_strncpy( commands[count], szCandidate, sizeof( commands[count] ) ); \
			count++; \
		} \
	} \
	else \
	{ \
		return count; \
	}

	COMMAND_COMPLETION_CANDIDATE( "asw_mission_chooser exit" );
	for ( int i = 0; i < NELEMS( g_ASW_HostTypeName ); i++ )
	{
		COMMAND_COMPLETION_CANDIDATE( "asw_mission_chooser %s", g_ASW_HostTypeName[i] );
	}
	for ( int i = 0; i < NELEMS( g_ASW_HostTypeName ); i++ )
	{
		for ( int j = 0; j < NELEMS( g_ASW_ChooserTypeName ); j++ )
		{
			COMMAND_COMPLETION_CANDIDATE( "asw_mission_chooser %s %s", g_ASW_HostTypeName[i], g_ASW_ChooserTypeName[j] );
		}

		for ( int j = 0; j < ReactiveDropMissions::CountCampaigns(); j++ )
		{
			COMMAND_COMPLETION_CANDIDATE( "asw_mission_chooser %s campaign %s", g_ASW_HostTypeName[i], ReactiveDropMissions::CampaignName( j ) );
		}
	}

#undef COMMAND_COMPLETION_CANDIDATE

	return count;
}

CON_COMMAND_F_COMPLETION( asw_mission_chooser, "asw_mission_chooser host [chooser] [campaign] - open mission chooser", FCVAR_CLIENTCMD_CAN_EXECUTE, asw_mission_chooser_completion )
{
	if ( args.ArgC() < 2 || args.ArgC() > 4 )
	{
		ConMsg( "Usage: asw_mission_chooser host [chooser] [campaign]\n" );
		return;
	}

	if ( !V_stricmp( args.Arg( 1 ), "exit" ) )
	{
		LaunchMissionChooser( ASW_HOST_TYPE::NUM_TYPES, ASW_CHOOSER_TYPE::NUM_TYPES, NULL );
		return;
	}

	ASW_CHOOSER_TYPE iChooserType = ASW_CHOOSER_TYPE::NUM_TYPES;
	if ( args.ArgC() >= 3 )
	{
		for ( int i = 0; i < int( ASW_CHOOSER_TYPE::NUM_TYPES ); i++ )
		{
			if ( !V_stricmp( args.Arg( 2 ), g_ASW_ChooserTypeName[i] ) )
			{
				iChooserType = ASW_CHOOSER_TYPE( i );
				break;
			}
		}

		if ( iChooserType == ASW_CHOOSER_TYPE::NUM_TYPES )
		{
			ConMsg( "Invalid host type.\n" );
			return;
		}
	}

	if ( args.ArgC() >= 4 && iChooserType != ASW_CHOOSER_TYPE::CAMPAIGN )
	{
		ConMsg( "Usage: asw_mission_chooser host [chooser] [campaign]\n" );
		return;
	}

	const char *szCampaignName = NULL;
	if ( args.ArgC() >= 4 )
	{
		szCampaignName = args.Arg( 3 );
	}

	for ( int i = 0; i < int( ASW_HOST_TYPE::NUM_TYPES ); i++ )
	{
		if ( !V_stricmp( args.Arg( 1 ), g_ASW_HostTypeName[i] ) )
		{
			LaunchMissionChooser( ASW_HOST_TYPE( i ), iChooserType, szCampaignName );
			return;
		}
	}

	ConMsg( "Invalid host type.\n" );
}
