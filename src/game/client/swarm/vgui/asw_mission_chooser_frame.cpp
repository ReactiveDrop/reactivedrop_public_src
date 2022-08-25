#include "cbase.h"
#include "asw_mission_chooser_frame.h"
#include "gameui/swarm/basemodpanel.h"
#include "gameui/swarm/uigamedata.h"
#include "rd_missions_shared.h"
#include "rd_workshop.h"
#include "rd_missions_shared.h"
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Label.h>
#include "campaignmapsearchlights.h"
#include "fmtstr.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


extern ConVar rd_last_game_access;
extern ConVar rd_last_game_difficulty;
extern ConVar rd_last_game_challenge;
extern ConVar rd_last_game_onslaught;
extern ConVar rd_last_game_hardcoreff;
extern ConVar rd_last_game_maxplayers;
extern ConVar rd_reduce_motion;

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

static const char *const s_WorkshopChooserTypeName[] =
{
	"#rd_workshop_find_new_campaign",
	"", // saved campaign
	"", // single mission
	"#rd_workshop_find_new_bonus_mission",
	"#rd_workshop_find_new_deathmatch",
	"#rd_workshop_find_new_endless",
};

static const char *const s_WorkshopChooserTypeTag[] =
{
	"Campaign",
	"", // saved campaign
	"", // single mission
	"Bonus",
	"Deathmatch",
	"Endless",
};

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
	{
		return;
	}

	pFrame = new CASW_Mission_Chooser_Frame( iHostType );
	pFrame->ShowFullScreen();

	if ( iChooserType != ASW_CHOOSER_TYPE::NUM_TYPES )
	{
		pFrame->SelectTab( iChooserType );
	}

	if ( iChooserType == ASW_CHOOSER_TYPE::CAMPAIGN && szCampaignName )
	{
		pFrame->ApplyCampaign( iChooserType, szCampaignName );
	}

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
		Msg( "Usage: asw_mission_chooser host [chooser] [campaign]\n" );
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
			Msg( "Invalid host type.\n" );
			return;
		}
	}

	if ( args.ArgC() >= 4 && iChooserType != ASW_CHOOSER_TYPE::CAMPAIGN )
	{
		Msg( "Usage: asw_mission_chooser host [chooser] [campaign]\n" );
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

	Msg( "Invalid host type.\n" );
}

CASW_Mission_Chooser_Frame::CASW_Mission_Chooser_Frame( ASW_HOST_TYPE iHostType )
{
	m_HostType = iHostType;
	m_bViewingCampaign = false;

	switch ( iHostType )
	{
	case ASW_HOST_TYPE::SINGLEPLAYER:
		SetTitle( "#nb_select_mission", true );
		break;
	case ASW_HOST_TYPE::CREATESERVER:
		SetTitle( "#nb_select_mission", true );
		break;
	case ASW_HOST_TYPE::CALLVOTE:
		SetTitle( "#asw_vote_server", true );
		break;
	default:
		Assert( !"unexpected missionchooser host type" );
		break;
	}

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

			CASW_Mission_Chooser_Tab *pChooserList = new CASW_Mission_Chooser_Tab( this, ASW_CHOOSER_TYPE( j ) );
			m_MainTabs.AddToTail( pChooserList );
			AddTab( pChooserList );
		}
	}
}

void CASW_Mission_Chooser_Frame::OnCommand( const char *command )
{
	if ( !V_stricmp( command, "BackButton" ) && m_bViewingCampaign )
	{
		BaseModUI::CBaseModPanel::GetSingleton().PlayUISound( BaseModUI::UISOUND_BACK );

		FOR_EACH_VEC_BACK( m_Tabs, i )
		{
			TGD_Tab *pTab = m_Tabs[i];
			RemoveTab( pTab );
			pTab->MarkForDeletion();
		}

		FOR_EACH_VEC( m_MainTabs, i )
		{
			AddTab( m_MainTabs[i] );
		}

		m_bViewingCampaign = false;
	}
	else
	{
		BaseClass::OnCommand( command );
	}
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

	FOR_EACH_VEC( m_MainTabs, i )
	{
		RemoveTab( m_MainTabs[i] );
	}

	AddTab( new CASW_Mission_Chooser_Tab( this, iChooserType, szCampaignName ) );
}

bool CASW_Mission_Chooser_Frame::SelectTab( ASW_CHOOSER_TYPE iChooserType )
{
	FOR_EACH_VEC( m_Tabs, i )
	{
		CASW_Mission_Chooser_Tab *pTab = assert_cast< CASW_Mission_Chooser_Tab * >( m_Tabs[i] );
		if ( pTab->m_ChooserType == iChooserType )
		{
			ActivateTab( pTab );
			return true;
		}
	}

	return false;
}

CASW_Mission_Chooser_Tab::CASW_Mission_Chooser_Tab( CASW_Mission_Chooser_Frame *pFrame, ASW_CHOOSER_TYPE iChooserType, const char *szCampaignName ) : BaseClass( pFrame, szCampaignName ? "#nb_select_starting_mission" : s_ChooserTabName[int(iChooserType)] )
{
	m_nDataResets = 0;
	m_nLastX = -1;
	m_nLastY = -1;
	m_ChooserType = iChooserType;
	m_szCampaignName[0] = '\0';

	if ( szCampaignName )
	{
		V_strncpy( m_szCampaignName, szCampaignName, sizeof( m_szCampaignName ) );
	}
}

TGD_Details *CASW_Mission_Chooser_Tab::CreateDetails()
{
	return new CASW_Mission_Chooser_Details( this );
}

void CASW_Mission_Chooser_Tab::OnThink()
{
	BaseClass::OnThink();

	if ( !m_pGrid || !m_pDetails )
	{
		// wait until we get initialized.
		return;
	}

	// make sure data reset count is up to date.
	ReactiveDropMissions::GetCampaign( -1 );

	if ( ReactiveDropMissions::s_nDataResets == m_nDataResets )
	{
		return;
	}

	if ( m_szCampaignName[0] )
	{
		BuildCampaignMissionList();

		m_nDataResets = ReactiveDropMissions::s_nDataResets;

		InvalidateLayout();

		return;
	}

	bool bOnWorkshop = true;

	switch ( m_ChooserType )
	{
	case ASW_CHOOSER_TYPE::CAMPAIGN:
		BuildCampaignList( NULL );
		break;
	case ASW_CHOOSER_TYPE::SAVED_CAMPAIGN:
		Assert( !"Saved campaign mission chooser list not implemented!" );
		bOnWorkshop = false;
		break;
	case ASW_CHOOSER_TYPE::SINGLE_MISSION:
		BuildMissionList( NULL );
		bOnWorkshop = false;
		break;
	case ASW_CHOOSER_TYPE::BONUS_MISSION:
		BuildMissionList( "bonus" );
		break;
	case ASW_CHOOSER_TYPE::DEATHMATCH:
		BuildMissionList( "deathmatch" );
		break;
	case ASW_CHOOSER_TYPE::ENDLESS:
		BuildMissionList( "endless" );
		break;
	default:
		Assert( !"Unhandled ASW_CHOOSER_TYPE in CASW_Mission_Chooser_List" );
		bOnWorkshop = false;
		break;
	}

	if ( bOnWorkshop )
	{
		m_pGrid->AddEntry( new CASW_Mission_Chooser_Entry( m_pGrid, "MissionChooserEntry", m_ChooserType ) );
	}

	m_nDataResets = ReactiveDropMissions::s_nDataResets;
}

void CASW_Mission_Chooser_Tab::BuildCampaignList( const char *szRequiredTag )
{
	m_pGrid->DeleteAllEntries();

	for ( int i = 0; i < ReactiveDropMissions::CountCampaigns(); i++ )
	{
		const RD_Campaign_t *pCampaign = ReactiveDropMissions::GetCampaign( i );
		Assert( pCampaign );
		if ( !pCampaign )
			continue;

		if ( szRequiredTag && !pCampaign->HasTag( szRequiredTag ) )
			continue;

		m_pGrid->AddEntry( new CASW_Mission_Chooser_Entry( m_pGrid, "MissionChooserEntry", pCampaign, NULL ) );
	}
}

void CASW_Mission_Chooser_Tab::BuildMissionList( const char *szRequiredTag )
{
	m_pGrid->DeleteAllEntries();

	for ( int i = 0; i < ReactiveDropMissions::CountMissions(); i++ )
	{
		const RD_Mission_t *pMission = ReactiveDropMissions::GetMission( i );
		Assert( pMission );
		if ( !pMission )
			continue;

		if ( szRequiredTag && !pMission->HasTag( szRequiredTag ) )
			continue;

		m_pGrid->AddEntry( new CASW_Mission_Chooser_Entry( m_pGrid, "MissionChooserEntry", NULL, pMission ) );
	}
}

void CASW_Mission_Chooser_Tab::BuildCampaignMissionList()
{
	m_pGrid->DeleteAllEntries();

	const RD_Campaign_t *pCampaign = ReactiveDropMissions::GetCampaign( m_szCampaignName );
	Assert( pCampaign );
	if ( !pCampaign )
		return;

	for ( int i = 1; i < pCampaign->Missions.Count(); i++ )
	{
		const RD_Mission_t *pMission = ReactiveDropMissions::GetMission( pCampaign->Missions[i].MapName );
		Assert( pMission );
		if ( !pMission )
			continue;

		m_pGrid->AddEntry( new CASW_Mission_Chooser_Entry( m_pGrid, "MissionChooserEntry", pCampaign, pMission ) );
	}
}

CASW_Mission_Chooser_Details::CASW_Mission_Chooser_Details( TGD_Tab *pTab ) : BaseClass( pTab )
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

	DisplayEntry( NULL );
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

	DisplayEntry( NULL );
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

void CASW_Mission_Chooser_Details::DisplayEntry( TGD_Entry *pBaseEntry )
{
	m_nForceReLayout = 1;

	if ( !pBaseEntry )
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

	CASW_Mission_Chooser_Entry *pEntry = assert_cast< CASW_Mission_Chooser_Entry * >( pBaseEntry );

	if ( pEntry->m_szMission[0] )
	{
		const RD_Mission_t *pMission = ReactiveDropMissions::GetMission( pEntry->m_szMission );
		Assert( pMission );
		if ( !pMission )
		{
			DisplayEntry( NULL );
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

		InvalidateLayout();

		return;
	}

	if ( pEntry->m_szCampaign[0] )
	{
		const RD_Campaign_t *pCampaign = ReactiveDropMissions::GetCampaign( pEntry->m_szCampaign );
		Assert( pCampaign );
		if ( !pCampaign )
		{
			DisplayEntry( NULL );
			return;
		}

		m_pImage->SetImage( STRING( pCampaign->ChooseCampaignTexture ) );
		m_pTitle->SetText( STRING( pCampaign->CampaignName ) );
		m_pDescription->SetText( STRING( pCampaign->CampaignDescription ) );
		m_pMapBase->SetImage( STRING( pCampaign->CampaignTextureName ) );
		m_pMapLayer[0]->SetVisible( !rd_reduce_motion.GetBool() );
		m_pMapLayer[1]->SetVisible( !rd_reduce_motion.GetBool() );
		m_pMapLayer[2]->SetVisible( !rd_reduce_motion.GetBool() );
		m_pMapLayer[0]->SetImage( STRING( pCampaign->CampaignTextureLayer[0] ) );
		m_pMapLayer[1]->SetImage( STRING( pCampaign->CampaignTextureLayer[1] ) );
		m_pMapLayer[2]->SetImage( STRING( pCampaign->CampaignTextureLayer[2] ) );
		m_pSearchLights->SetVisible( true );
		m_pSearchLights->SetCampaign( pCampaign );

		InvalidateLayout();

		return;
	}

	// not a campaign or a mission; remove overview (it's a placeholder)
	DisplayEntry( NULL );
}

CASW_Mission_Chooser_Entry::CASW_Mission_Chooser_Entry( TGD_Grid *parent, const char *pElementName, const RD_Campaign_t *pCampaign, const RD_Mission_t *pMission ) : BaseClass( parent, pElementName )
{
	m_szCampaign[0] = '\0';
	m_szMission[0] = '\0';
	m_WorkshopChooserType = ASW_CHOOSER_TYPE::NUM_TYPES;

	if ( pCampaign )
	{
		V_strncpy( m_szCampaign, pCampaign->BaseName, sizeof( m_szCampaign ) );
	}
	if ( pMission )
	{
		V_strncpy( m_szMission, pMission->BaseName, sizeof( m_szMission ) );
	}

	m_pImage = new vgui::ImagePanel( this, "Image" );
	m_pTitle = new vgui::Label( this, "Title", "" );
}

CASW_Mission_Chooser_Entry::CASW_Mission_Chooser_Entry( TGD_Grid *parent, const char *pElementName, ASW_CHOOSER_TYPE iChooserType ) : BaseClass( parent, pElementName )
{
	m_szCampaign[0] = '\0';
	m_szMission[0] = '\0';
	m_WorkshopChooserType = iChooserType;

	m_pImage = new vgui::ImagePanel( this, "Image" );
	m_pTitle = new vgui::Label( this, "Title", "" );
}

void CASW_Mission_Chooser_Entry::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	if ( m_WorkshopChooserType != ASW_CHOOSER_TYPE::NUM_TYPES )
	{
		m_pImage->SetImage( "swarm/missionpics/downloadfromworkshopplaceholder" );
		m_pTitle->SetText( s_WorkshopChooserTypeName[int( m_WorkshopChooserType )] );
		SetAlpha( 255 );
		return;
	}

	Assert( m_szCampaign[0] || m_szMission[0] );

	const RD_Campaign_t *pCampaign = NULL;
	const RD_Mission_t *pMission = NULL;

	if ( m_szCampaign[0] )
	{
		pCampaign = ReactiveDropMissions::GetCampaign( m_szCampaign );
		Assert( pCampaign );
	}
	if ( m_szMission[0] )
	{
		pMission = ReactiveDropMissions::GetMission( m_szMission );
		Assert( pMission );
	}

	bool bIsSubscribed = false;
	if ( pMission )
	{
		m_pImage->SetImage( STRING( pMission->Image ) );
		m_pTitle->SetText( STRING( pMission->MissionTitle ) );
		bIsSubscribed = pMission->WorkshopID == k_PublishedFileIdInvalid || g_ReactiveDropWorkshop.IsSubscribedToFile( pMission->WorkshopID );
	}
	else if ( pCampaign )
	{
		m_pImage->SetImage( STRING( pCampaign->ChooseCampaignTexture ) );
		m_pTitle->SetText( STRING( pCampaign->CampaignName ) );
		bIsSubscribed = pCampaign->WorkshopID == k_PublishedFileIdInvalid || g_ReactiveDropWorkshop.IsSubscribedToFile( pCampaign->WorkshopID );
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

void CASW_Mission_Chooser_Entry::ApplyEntry()
{
	CASW_Mission_Chooser_Tab *pTab = assert_cast< CASW_Mission_Chooser_Tab * >( m_pParent->m_pParent );
	CASW_Mission_Chooser_Frame *pFrame = assert_cast< CASW_Mission_Chooser_Frame * >( pTab->m_pParent );

	if ( m_szMission[0] )
	{
		PublishedFileId_t iWorkshopID = ReactiveDropMissions::MissionWorkshopID( ReactiveDropMissions::GetMissionIndex( m_szMission ) );
		if ( iWorkshopID && !g_ReactiveDropWorkshop.IsAddonEnabled( iWorkshopID ) )
		{
			g_ReactiveDropWorkshop.OpenWorkshopPageForFile( iWorkshopID );
			return;
		}

		if ( pFrame->m_HostType == ASW_HOST_TYPE::CALLVOTE )
		{
			if ( m_szCampaign[0] )
			{
				engine->ServerCmd( CFmtStr( "asw_vote_campaign %d %s\n", ReactiveDropMissions::GetCampaignIndex( m_szCampaign ), m_szMission ) );
			}
			else
			{
				engine->ServerCmd( CFmtStr( "asw_vote_mission %s\n", m_szMission ) );
			}

			pFrame->MarkForDeletion();
			return;
		}

		KeyValues::AutoDelete pSettings( "Settings" );

		if ( pFrame->m_HostType == ASW_HOST_TYPE::SINGLEPLAYER )
		{
			pSettings->SetString( "system/network", "offline" );
		}
		else
		{
			Assert( pFrame->m_HostType == ASW_HOST_TYPE::CREATESERVER );
			pSettings->SetString( "system/network", "LIVE" );
			pSettings->SetString( "system/access", rd_last_game_access.GetString() );
			pSettings->SetString( "options/action", "create" );
		}

		if ( m_szCampaign[0] )
		{
			pSettings->SetString( "game/mode", "campaign" );
			pSettings->SetString( "game/campaign", m_szCampaign );
		}
		else
		{
			pSettings->SetString( "game/mode", "single_mission" );
		}

		pSettings->SetString( "game/mission", m_szMission );
		pSettings->SetInt( "members/numSlots", rd_last_game_maxplayers.GetInt() );

		if ( pTab->m_ChooserType != ASW_CHOOSER_TYPE::DEATHMATCH )
		{
			pSettings->SetString( "game/difficulty", rd_last_game_difficulty.GetString() );
			pSettings->SetString( "game/challenge", rd_last_game_challenge.GetString() );
			pSettings->SetBool( "game/onslaught", rd_last_game_onslaught.GetBool() );
			pSettings->SetBool( "game/hardcoreFF", rd_last_game_hardcoreff.GetBool() );
		}

		BaseModUI::CBaseModPanel::GetSingleton().PlayUISound( BaseModUI::UISOUND_ACCEPT );
		BaseModUI::CBaseModPanel::GetSingleton().CloseAllWindows();

		if ( pFrame->m_HostType == ASW_HOST_TYPE::SINGLEPLAYER )
		{
			g_pMatchFramework->CreateSession( pSettings );
		}
		else
		{
			BaseModUI::CBaseModPanel::GetSingleton().OpenWindow( BaseModUI::WT_GAMESETTINGS, NULL, true, pSettings );
		}

		pFrame->MarkForDeletion();
		return;
	}

	Assert( m_szCampaign[0] );
	if ( !m_szCampaign[0] )
		return;

	pFrame->ApplyCampaign( pTab->m_ChooserType, m_szCampaign );
}
