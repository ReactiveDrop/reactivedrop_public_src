#include "cbase.h"

#include "nb_main_panel.h"
#include "vgui_controls/Label.h"
#include "vgui_controls/Button.h"
#include "vgui_controls/Panel.h"
#include "vgui_controls/ImagePanel.h"
#include "nb_lobby_tooltip.h"
#include "nb_lobby_row.h"
#include "nb_lobby_row_small.h"
#include "nb_select_marine_panel.h"
#include "tabbedgriddetails.h"
#include "rd_collections.h"
#include "nb_vote_panel.h"
#include "asw_briefing.h"
#include <vgui/ILocalize.h>
#include <vgui/IVGui.h>
#include "asw_marine_profile.h"
#include "ForceReadyPanel.h"
#include "asw_gamerules.h"
#include "KeyValues.h"
#include "nb_mission_summary.h"
#include "nb_mission_panel.h"
#include "nb_spend_skill_points.h"
#include "nb_header_footer.h"
#include "nb_button.h"
#include "gameui/swarm/uigamedata.h"
#include "gameui/swarm/vgenericpanellist.h"
#include "c_asw_game_resource.h"
#include "vgui_bitmapbutton.h"
#include "clientmode_asw.h"
#include "c_asw_player.h"
#include "nb_promotion_panel.h"
#include "asw_deathmatch_mode.h"
#include "rd_lobby_utils.h"
#include "nb_leaderboard_panel.h"
#include "controller_focus.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


#define CHAT_BUTTON_ICON "vgui/briefing/chat_icon"
#define VOTE_BUTTON_ICON "vgui/briefing/vote_icon"
#define LEADERBOARD_BUTTON_ICON "vgui/briefing/leaderboard_icon"
#define ADDBOT_BUTTON_ICON "vgui/briefing/addbot_icon"
#define DESELECT_MARINES_BUTTON_ICON "vgui/briefing/deselectmarines_icon"

extern ConVar mp_gamemode;
extern ConVar mm_max_players;
extern ConVar rd_player_bots_allowed;
ConVar rd_draw_briefing_ui( "rd_draw_briefing_ui", "1", FCVAR_CHEAT );

using BaseModUI::GenericPanelList;

CUtlVector<int> CNB_Main_Panel::s_QueuedSpendSkillPoints;

void CNB_Main_Panel::QueueSpendSkillPoints( int nProfileIndex )
{
	if ( s_QueuedSpendSkillPoints.Find( nProfileIndex ) != s_QueuedSpendSkillPoints.InvalidIndex() )
		return;

	s_QueuedSpendSkillPoints.AddToTail( nProfileIndex );
}

void CNB_Main_Panel::RemoveFromSpendQueue( int nProfileIndex )
{
	for ( int i = s_QueuedSpendSkillPoints.Count() - 1; i >= 0; i-- )
	{
		if ( s_QueuedSpendSkillPoints[i] == nProfileIndex )
		{
			s_QueuedSpendSkillPoints.Remove( i );
		}
	}
}

CNB_Main_Panel::CNB_Main_Panel( vgui::Panel *parent, const char *name ) : BaseClass( parent, name )
{
	// == MANAGED_MEMBER_CREATION_START: Do not edit by hand ==
	m_pHeaderFooter = new CNB_Header_Footer( this, "HeaderFooter" );
	m_pLeaderLabel = new vgui::Label( this, "LeaderLabel", "" );
    m_pTeamLabel = new vgui::Label( this, "TeamLabel", "" );
	m_pReadyCheckImage = new vgui::ImagePanel( this, "ReadyCheckImage" );
	m_pLobbyRow0 = new CNB_Lobby_Row( this, "LobbyRow0" );
	m_pLobbyTooltip = new CNB_Lobby_Tooltip( this, "LobbyTooltip" );
	m_pMissionSummary = new CNB_Mission_Summary( this, "MissionSummary" );
	// == MANAGED_MEMBER_CREATION_END ==

    m_pLobbyRowsScroll = new GenericPanelList( this, "LobbyRowsScroll", GenericPanelList::ISM_ELEVATOR );
    m_pLobbyRowsScroll->SetBgColor( Color( 0, 0, 0, 0 ) );
	m_pLobbyRowsScroll->SetScrollArrowsVisible( false );

	m_pVotePanel = new CNB_Vote_Panel( this, "VotePanel" );
	m_pReadyButton = new CNB_Button( this, "ReadyButton", "", this, "ReadyButton" );
	m_pMissionDetailsButton = new CNB_Button( this, "MissionDetailsButton", "", this, "MissionDetailsButton" );
	m_pFriendsButton = new CNB_Button( this, "FriendsButton", "", this, "FriendsButton" );
	m_pChatButton = new CBitmapButton( this, "ChatButton", "" );
	m_pChatButton->AddActionSignalTarget( this );
	m_pChatButton->SetCommand( "ChatButton" );
	m_pVoteButton = new CBitmapButton( this, "VoteButton", "" );
	m_pVoteButton->AddActionSignalTarget( this );
	m_pVoteButton->SetCommand( "VoteButton" );
	m_pLeaderboardButton = new CBitmapButton( this, "LeaderboardButton", "" );
	m_pLeaderboardButton->AddActionSignalTarget( this );
	m_pLeaderboardButton->SetCommand( "LeaderboardButton" );
	m_pAddBotButton = new CBitmapButton(this, "AddBotButton", "");
	m_pAddBotButton->AddActionSignalTarget(this);
	m_pAddBotButton->SetCommand("AddBotButton");
	m_pDeselectMarinesButton = new CBitmapButton(this, "DeselectMarines", "");
	m_pDeselectMarinesButton->AddActionSignalTarget(this);
	m_pDeselectMarinesButton->SetCommand("DeselectMarines");

	m_pChangeMissionButton = new CNB_Button( this, "ChangeMissionButton", "", this, "ChangeMissionButton" );

	m_pPromotionButton = new CNB_Button( this, "PromotionButton", "", this, "PromotionButton" );
    m_pTeamChangeButtonButton = new CNB_Button( this, "TeamChangeButton", "", this, "TeamChangeButton" );

	m_pHeaderFooter->SetTitle( "#nb_mission_prep" );
	m_pHeaderFooter->SetBriefingCameraEnabled( true );

	m_bLocalLeader = false;

	if ( ASWDeathmatchMode() )
	{
		m_pReadyButton->SetText( "#nb_ready" );
		m_pReadyCheckImage->SetVisible( false );
	}

	vgui::ivgui()->AddTickSignal( GetVPanel() );

	m_bLobbyValidityChecked = false;

	GetControllerFocus()->AddToFocusList( m_pChatButton );
	GetControllerFocus()->AddToFocusList( m_pVoteButton );
	GetControllerFocus()->AddToFocusList( m_pLeaderboardButton );
	GetControllerFocus()->AddToFocusList( m_pAddBotButton );
	GetControllerFocus()->AddToFocusList( m_pDeselectMarinesButton );
}

CNB_Main_Panel::~CNB_Main_Panel()
{
	GetControllerFocus()->RemoveFromFocusList( m_pChatButton );
	GetControllerFocus()->RemoveFromFocusList( m_pVoteButton );
	GetControllerFocus()->RemoveFromFocusList( m_pLeaderboardButton );
	GetControllerFocus()->RemoveFromFocusList( m_pAddBotButton );
	GetControllerFocus()->RemoveFromFocusList( m_pDeselectMarinesButton );
}

void CNB_Main_Panel::ProcessSkillSpendQueue()
{
	if ( s_QueuedSpendSkillPoints.Count() > 0 )
	{
		SpendSkillPointsOnMarine( s_QueuedSpendSkillPoints[0] );
	}
}

void CNB_Main_Panel::OnFinishedSpendingSkillPoints()
{
	m_hSubScreen = NULL;
	ProcessSkillSpendQueue();
}

void CNB_Main_Panel::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );
	
	LoadControlSettings( "resource/ui/nb_main_panel.res" );

	color32 white;
	white.r = 255;
	white.g = 255;
	white.b = 255;
	white.a = 255;

	color32 grey;
	grey.r = 190;
	grey.g = 190;
	grey.b = 190;
	grey.a = 255;

	m_pChatButton->SetImage( CBitmapButton::BUTTON_ENABLED, CHAT_BUTTON_ICON, grey );
	m_pChatButton->SetImage( CBitmapButton::BUTTON_DISABLED, CHAT_BUTTON_ICON, grey );
	m_pChatButton->SetImage( CBitmapButton::BUTTON_PRESSED, CHAT_BUTTON_ICON, white );		
	m_pChatButton->SetImage( CBitmapButton::BUTTON_ENABLED_MOUSE_OVER, CHAT_BUTTON_ICON, white );

	m_pVoteButton->SetImage( CBitmapButton::BUTTON_ENABLED, VOTE_BUTTON_ICON, grey );
	m_pVoteButton->SetImage( CBitmapButton::BUTTON_DISABLED, VOTE_BUTTON_ICON, grey );
	m_pVoteButton->SetImage( CBitmapButton::BUTTON_PRESSED, VOTE_BUTTON_ICON, white );		
	m_pVoteButton->SetImage( CBitmapButton::BUTTON_ENABLED_MOUSE_OVER, VOTE_BUTTON_ICON, white );

	m_pLeaderboardButton->SetImage( CBitmapButton::BUTTON_ENABLED, LEADERBOARD_BUTTON_ICON, grey );
	m_pLeaderboardButton->SetImage( CBitmapButton::BUTTON_DISABLED, LEADERBOARD_BUTTON_ICON, grey );
	m_pLeaderboardButton->SetImage( CBitmapButton::BUTTON_PRESSED, LEADERBOARD_BUTTON_ICON, white );
	m_pLeaderboardButton->SetImage( CBitmapButton::BUTTON_ENABLED_MOUSE_OVER, LEADERBOARD_BUTTON_ICON, white );

	m_pAddBotButton->SetImage(CBitmapButton::BUTTON_ENABLED, ADDBOT_BUTTON_ICON, grey);
	m_pAddBotButton->SetImage(CBitmapButton::BUTTON_DISABLED, ADDBOT_BUTTON_ICON, grey);
	m_pAddBotButton->SetImage(CBitmapButton::BUTTON_PRESSED, ADDBOT_BUTTON_ICON, white);
	m_pAddBotButton->SetImage(CBitmapButton::BUTTON_ENABLED_MOUSE_OVER, ADDBOT_BUTTON_ICON, white);

	m_pDeselectMarinesButton->SetImage(CBitmapButton::BUTTON_ENABLED, DESELECT_MARINES_BUTTON_ICON, grey);
	m_pDeselectMarinesButton->SetImage(CBitmapButton::BUTTON_DISABLED, DESELECT_MARINES_BUTTON_ICON, grey);
	m_pDeselectMarinesButton->SetImage(CBitmapButton::BUTTON_PRESSED, DESELECT_MARINES_BUTTON_ICON, white);
	m_pDeselectMarinesButton->SetImage(CBitmapButton::BUTTON_ENABLED_MOUSE_OVER, DESELECT_MARINES_BUTTON_ICON, white);
}

void CNB_Main_Panel::PerformLayout()
{
	BaseClass::PerformLayout();
}

void CNB_Main_Panel::OnThink()
{
	BaseClass::OnThink();

	if ( !Briefing() )
		return;

	if ( !m_bLobbyValidityChecked )
	{
		m_bLobbyValidityChecked = true;
		if ( gpGlobals->maxClients > 1 && !UTIL_RD_GetCurrentLobbyID().IsValid() )
		{
			engine->ServerCmd( "cl_lobby_invalid_request" );
		}
	}

	m_pFriendsButton->SetVisible( ! ( ASWGameResource() && ASWGameResource()->IsOfflineGame() ) );
	m_pChatButton->SetVisible( gpGlobals->maxClients > 1 );
	m_pVoteButton->SetVisible( gpGlobals->maxClients > 1 );
	m_pLeaderboardButton->SetVisible( gpGlobals->maxClients > 1 && !ASWDeathmatchMode() && UTIL_RD_GetCurrentLobbyID().IsValid() );
	m_pChangeMissionButton->SetVisible( gpGlobals->maxClients == 1 );

	bool isOffline = ASWGameResource() && ASWGameResource()->IsOfflineGame();
	bool hasMarineSelected = Briefing()->GetMarineProfile(m_pLobbyRow0->m_nLobbySlot) != NULL;
	m_pAddBotButton->SetVisible( !isOffline && hasMarineSelected );
	m_pDeselectMarinesButton->SetVisible( hasMarineSelected );

	C_ASW_Player *pPlayer = C_ASW_Player::GetLocalASWPlayer();
	m_pPromotionButton->SetVisible( pPlayer && pPlayer->GetExperience() >= ( ASW_XP_CAP * g_flPromotionXPScale[ pPlayer->GetPromotion() ] ) && pPlayer->GetPromotion() < ASW_PROMOTION_CAP );

	m_pTeamChangeButtonButton->SetVisible( ASWDeathmatchMode() && ASWDeathmatchMode()->IsTeamDeathmatchEnabled() );
	
	const char *pszLeaderName = Briefing()->GetLeaderName();
	if ( pszLeaderName )
	{
		m_pLeaderLabel->SetVisible( ! ( ASWGameResource() && ASWGameResource()->IsOfflineGame() ) );

		wchar_t wszPlayerName[32];
		g_pVGuiLocalize->ConvertANSIToUnicode( pszLeaderName, wszPlayerName, sizeof(wszPlayerName));

		wchar_t wszBuffer[128];
		g_pVGuiLocalize->ConstructString( wszBuffer, sizeof(wszBuffer), g_pVGuiLocalize->Find( "#nb_leader" ), 1, wszPlayerName );

		m_pLeaderLabel->SetText( wszBuffer );
	}
	else
	{
		m_pLeaderLabel->SetVisible( false );
	}

    const char *pszTeamname = Briefing()->GetTeamName();
    if ( pszTeamname )
    {
        m_pTeamLabel->SetVisible( ASWDeathmatchMode() && GAMEMODE_TEAMDEATHMATCH == ASWDeathmatchMode()->GetGameMode() );

        wchar_t wszTeamName[32];
        g_pVGuiLocalize->ConvertANSIToUnicode( pszTeamname, wszTeamName, sizeof(wszTeamName));

        wchar_t wszBuffer[128];
        g_pVGuiLocalize->ConstructString( wszBuffer, sizeof(wszBuffer), g_pVGuiLocalize->Find( "#rd_str_team" ), 1, wszTeamName );  // Team: %s1

        m_pTeamLabel->SetText( wszBuffer );

        m_pTeamLabel->SetFgColor( Briefing()->GetTeamColor() );
    }
    else
    {
        m_pTeamLabel->SetVisible( false );
    }

	if ( !m_hSubScreen.Get() )
	{
		m_pLobbyRow0->CheckTooltip( m_pLobbyTooltip );
		for ( unsigned short i = 0; i < m_pLobbyRowsScroll->GetPanelItemCount(); i++ )
		{
			CNB_Lobby_Row_Small *pRow = assert_cast<CNB_Lobby_Row_Small *>( m_pLobbyRowsScroll->GetPanelItem( i ) );
			pRow->CheckTooltip( m_pLobbyTooltip );
		}

		ProcessSkillSpendQueue();
	}

	// these checks are set up only for coop, deathmatch sets it up in constructor 
	if ( !ASWDeathmatchMode() )
	{
		bool bLocalLeader = Briefing()->IsLocalPlayerLeader();
		if ( bLocalLeader != m_bLocalLeader )
		{
			if ( bLocalLeader )
			{
				m_pReadyButton->SetText( "#nb_start_mission" );
				m_pReadyCheckImage->SetVisible( false );
			}
			else
			{
				m_pReadyButton->SetText( "#nb_ready" );
				m_pReadyCheckImage->SetVisible( true );
			}
			m_bLocalLeader = bLocalLeader;
		}

		if ( !m_bLocalLeader )
		{
			if ( Briefing()->GetCommanderReady( 0 ) )
			{
				m_pReadyCheckImage->SetImage( "swarm/HUD/TickBoxTicked" );
			}
			else
			{
				m_pReadyCheckImage->SetImage( "swarm/HUD/TickBoxEmpty" );
			}
		}
	}
}

void CNB_Main_Panel::OnTick()
{
	BaseClass::OnTick();

	if ( !rd_draw_briefing_ui.GetBool() )
	{
		SetVisible( false );
		return;
	}

	SetVisible( true );

	char mapName[255];
	Q_FileBase( engine->GetLevelName(), mapName, sizeof(mapName) );
	bool bPracticeMap = ( !Q_strnicmp( mapName, "asi-jac1-landingbay_pract", 25 ) );

	unsigned short nBriefingSlots = 4;
	if ( !Briefing()->IsOfflineGame() )
	{
		for ( nBriefingSlots = 1; nBriefingSlots < NUM_BRIEFING_LOBBY_SLOTS; nBriefingSlots++ )
		{
			if ( !Briefing()->IsLobbySlotOccupied( nBriefingSlots ) )
			{
				break;
			}
		}
	}
	else
	{
		if ( !bPracticeMap )
		{
			if ( !rd_player_bots_allowed.GetBool() )
				nBriefingSlots = 1;
			else if ( V_stricmp( mp_gamemode.GetString(), "single_mission" ) != 0 )
				nBriefingSlots = mm_max_players.GetInt();
		}
	}

	// remove extra rows
	for ( unsigned short i = m_pLobbyRowsScroll->GetPanelItemCount(); i >= nBriefingSlots; i-- )
	{
		m_pLobbyRowsScroll->RemovePanelItem( i - 1 );
	}

	// add missing rows
	for ( unsigned short i = m_pLobbyRowsScroll->GetPanelItemCount() + 1; i < nBriefingSlots; i++ )
	{
		CNB_Lobby_Row_Small *pRow = new CNB_Lobby_Row_Small( this, VarArgs( "LobbyRow%d", i ) );
		pRow->m_nLobbySlot = i;
		m_pLobbyRowsScroll->AddPanelItem( pRow, true );
	}

	m_pLobbyRowsScroll->SetScrollBarVisible( nBriefingSlots > 5 );
}

void CNB_Main_Panel::ChangeMarine( int nLobbySlot )
{
	if ( !Briefing()->IsLobbySlotLocal( nLobbySlot ) )
		return;

	if ( m_hSubScreen.Get() )
	{
		m_hSubScreen->MarkForDeletion();
	}

	CNB_Select_Marine_Panel *pMarinePanel = new CNB_Select_Marine_Panel( this, "Select_Marine_Panel" );
	CASW_Marine_Profile *pProfile = Briefing()->GetMarineProfile( nLobbySlot );
	pMarinePanel->m_nInitialProfileIndex = pProfile ? pProfile->m_ProfileIndex : -1;
	pMarinePanel->m_nPreferredLobbySlot = nLobbySlot;
	pMarinePanel->InitMarineList();
	pMarinePanel->MoveToFront();
	Briefing()->SetChangingWeaponSlot( nLobbySlot, 1 );

	m_hSubScreen = pMarinePanel;

	CLocalPlayerFilter filter;
	C_BaseEntity::EmitSound( filter, -1, "ASWComputer.MenuButton" );
}

void CNB_Main_Panel::AddBot()
{
	if ( !rd_player_bots_allowed.GetBool() )
	{
		C_ASW_Player *pPlayer = C_ASW_Player::GetLocalASWPlayer();
		pPlayer->SendRosterSelectCommand( "cl_selectm", -1, -1 );
		return;
	}

	if (m_hSubScreen.Get())
	{
		m_hSubScreen->MarkForDeletion();
	}

	CNB_Select_Marine_Panel *pMarinePanel = new CNB_Select_Marine_Panel(this, "Select_Marine_Panel");	
	pMarinePanel->m_nInitialProfileIndex = -1;

	if (Briefing()->IsOfflineGame())
	{
		pMarinePanel->m_nPreferredLobbySlot = -1;
	}
	pMarinePanel->m_bAddingBot = true;

	pMarinePanel->InitMarineList();
	pMarinePanel->MoveToFront();

	m_hSubScreen = pMarinePanel;

	CLocalPlayerFilter filter;
	C_BaseEntity::EmitSound(filter, -1, "ASWComputer.MenuButton");
}

void CNB_Main_Panel::ChangeWeapon( int nLobbySlot, int nInventorySlot )
{
	if ( !Briefing()->IsLobbySlotLocal( nLobbySlot ) )
		return;

	if ( m_hSubScreen.Get() )
	{
		m_hSubScreen->MarkForDeletion();
	}

	CASW_Marine_Profile *pProfile = Briefing()->GetMarineProfile( nLobbySlot );
	if ( !pProfile )
		return;

	int nProfileIndex = pProfile->m_ProfileIndex;
	if ( nProfileIndex == -1 )
		return;

	TabbedGridDetails *pWeaponPanel = new TabbedGridDetails();
	if ( nInventorySlot == ASW_INVENTORY_SLOT_PRIMARY )
		pWeaponPanel->SetTitle( "#nb_select_weapon_one", true );
	else if ( nInventorySlot == ASW_INVENTORY_SLOT_SECONDARY )
		pWeaponPanel->SetTitle( "#nb_select_weapon_two", true );
	else if ( nInventorySlot == ASW_INVENTORY_SLOT_EXTRA )
		pWeaponPanel->SetTitle( "#nb_select_offhand", true );

	CRD_Collection_Tab_Equipment *pTab = new CRD_Collection_Tab_Equipment( pWeaponPanel, nInventorySlot == ASW_INVENTORY_SLOT_EXTRA ? "#rd_collection_equipment" : "#rd_collection_weapons", pProfile, nInventorySlot );
	pTab->SetBriefing( Briefing(), nLobbySlot );
	pWeaponPanel->AddTab( pTab );

	pWeaponPanel->ShowFullScreen();

	m_hSubScreen = pWeaponPanel;
}

void CNB_Main_Panel::SpendSkillPointsOnMarine( int nProfileIndex )
{
	if ( m_hSubScreen.Get() )
	{
		if ( dynamic_cast<CNB_Spend_Skill_Points*>( m_hSubScreen.Get() ) != NULL )	// already spending skill points on a marine
			return;

		m_hSubScreen->MarkForDeletion();
	}

	// remove from queue
	RemoveFromSpendQueue( nProfileIndex );

	CNB_Spend_Skill_Points *pPanel = new CNB_Spend_Skill_Points( this, "Spend_Skill_Points" );
	pPanel->m_nProfileIndex = nProfileIndex;
	pPanel->Init();
	pPanel->MoveToFront();

	// TODO: Briefing()->SetChangingWeaponSlot( ???, 1 );

	m_hSubScreen = pPanel;
}

extern vgui::DHANDLE<vgui::Frame> g_hBriefingFrame;

void CNB_Main_Panel::OnCommand( const char *command )
{
	if ( !Q_stricmp( command, "ReadyButton" ) )
	{
		// Firstly check if we are in the deathmatch 
		if ( ASWDeathmatchMode() )
		{
			// because briefing frame fades out slowly a user can click the
			// Ready button one more time and get a crash here, so we do this check
			if ( g_hBriefingFrame.Get() )
			{
				// for DM we only close the briefing panel
				g_hBriefingFrame->SetDeleteSelfOnClose( true );
				g_hBriefingFrame->Close();
				g_hBriefingFrame = NULL;
			}
		}
		else if ( m_bLocalLeader )
		{
			if ( Briefing()->CheckMissionRequirements() )
			{
				if ( Briefing()->AreOtherPlayersReady() )
				{
					Briefing()->StartMission();
				}
				else
				{
					// force other players to be ready?
					engine->ClientCmd( "cl_wants_start" ); // notify other players that we're waiting on them
					new ForceReadyPanel( GetParent(), "ForceReady", "#asw_force_startm", ASW_FR_BRIEFING );		// TODO: this breaks the IBriefing abstraction, fix it if we need that
				}
			}
		}
		else
		{
			Briefing()->ToggleLocalPlayerReady();
		}
	}
	else if ( !Q_stricmp( command, "FriendsButton" ) )
	{
#ifndef _X360 
		if ( BaseModUI::CUIGameData::Get() )
		{
			BaseModUI::CUIGameData::Get()->ExecuteOverlayCommand( "LobbyInvite" );
		}
#endif
	}
	else if ( !Q_stricmp( command, "ChangeMissionButton" ) )
	{
		engine->ClientCmd( "asw_mission_chooser callvote" );
	}
	else if ( !Q_stricmp( command, "MissionDetailsButton" ) )
	{
		ShowMissionDetails();
	}
	else if ( !Q_stricmp( command, "ChatButton" ) )
	{
		if ( GetClientModeASW() )
		{
			GetClientModeASW()->ToggleMessageMode();
		}
	}
	else if ( !Q_stricmp( command, "VoteButton" ) )
	{
		engine->ClientCmd( "playerlist" );
	}
	else if ( !Q_stricmp( command, "LeaderboardButton" ) )
	{
		ShowLeaderboard();
	}
	else if ( !Q_stricmp( command, "AddBotButton" ) )
	{
		AddBot();
	}
	else if ( !Q_stricmp( command, "DeselectMarines" ) )
	{
		engine->ClientCmd( "cl_dselectm 0;cl_dselectm 1;cl_dselectm 2;cl_dselectm 3;cl_dselectm 4;cl_dselectm 5;cl_dselectm 6;cl_dselectm 7;" );
	}
	else if ( !Q_stricmp( command, "PromotionButton" ) )
	{
		ShowPromotionPanel();
	}
	else if ( !Q_stricmp( command, "TeamChangeButton" ) )
	{
		engine->ServerCmd( "rd_team_change" );
	}
	BaseClass::OnCommand( command );
}



void CNB_Main_Panel::ShowMissionDetails()
{
	if ( m_hSubScreen.Get() )
	{
		m_hSubScreen->MarkForDeletion();
	}

	CNB_Mission_Panel *pPanel = new CNB_Mission_Panel( this, "MissionPanel" );
	pPanel->MoveToFront();

	m_hSubScreen = pPanel;
}

void CNB_Main_Panel::ShowPromotionPanel()
{
	if ( m_hSubScreen.Get() )
	{
		m_hSubScreen->MarkForDeletion();
	}

	CNB_Promotion_Panel *pPanel = new CNB_Promotion_Panel( this, "PromotionPanel" );
	pPanel->MoveToFront();

	m_hSubScreen = pPanel;
}

void CNB_Main_Panel::ShowLeaderboard()
{
	if ( m_hSubScreen.Get() )
	{
		m_hSubScreen->MarkForDeletion();
	}

	CNB_Leaderboard_Panel *pPanel = new CNB_Leaderboard_Panel( this, "LeaderboardPanel" );
	pPanel->MoveToFront();

	m_hSubScreen = pPanel;
}
