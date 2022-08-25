#include "cbase.h"
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/CheckButton.h>
#include "c_asw_marine_resource.h"
#include "c_asw_player.h"
#include "c_asw_marine.h"
#include "c_asw_campaign_save.h"
#include "c_asw_game_resource.h"
#include "PlayerListPanel.h"
#include "PlayerListLine.h"
#include "asw_gamerules.h"
#include "c_playerresource.h"
#include "ImageButton.h"
#include <vgui/ILocalize.h>
#include "WrappedLabel.h"
#include <vgui_controls/TextImage.h>
#include <vgui/ISurface.h>
#include "MissionStatsPanel.h"
#include "iinput.h"
#include "input.h"
#include "ForceReadyPanel.h"
#include "iclientmode.h"
#include "hud_element_helper.h"
#include "asw_hud_minimap.h"
#include "nb_header_footer.h"
#include "missionchooser/iasw_mission_chooser.h"
#include "missionchooser/iasw_mission_chooser_source.h"
#include "nb_button.h"
#include "asw_deathmatch_mode.h"
#include "c_team.h"
#include "gameui/swarm/vgenericpanellist.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using BaseModUI::GenericPanelList;

int g_asw_iPlayerListOpen = 0;

PlayerListPanel::PlayerListPanel(vgui::Panel *parent, const char *name) :
	vgui::EditablePanel(parent, name)
{
	//input->MouseEvent(0, false);	// unclick all our mouse buttons when this panel pops up (so firing doesn't get stuck on)

	m_bVoteMapInstalled = true;
	m_PlayerLine.Purge();

	m_pHeaderFooter = new CNB_Header_Footer( this, "HeaderFooter" );
	m_pHeaderFooter->SetTitle( "" );
	m_pHeaderFooter->SetHeaderEnabled( false );
	m_pHeaderFooter->SetFooterEnabled( true );
	m_pHeaderFooter->SetGradientBarEnabled( true );
	m_pHeaderFooter->SetGradientBarPos( 75, 320 );
	if ( ASWGameRules() && ASWGameRules()->GetGameState() != ASW_GS_INGAME )
	{
		m_pHeaderFooter->SetMovieEnabled( true );
		m_pHeaderFooter->SetBackgroundStyle( NB_BACKGROUND_DARK );
	}
	else
	{
		m_pHeaderFooter->SetMovieEnabled( false );
		m_pHeaderFooter->SetBackgroundStyle( NB_BACKGROUND_NONE );
	}

	m_pPlayerHeader = new vgui::Label(this, "PlayerHeader", "#asw_player_list_player");
	m_pMarinesHeader = new vgui::Label(this, "MarinesHeader", "#asw_player_list_marines");
	m_pFragsHeader = new vgui::Label(this, "FragsHeader",		"#asw_player_list_frags");
	m_pDeathsHeader = new vgui::Label(this, "DeathsHeader",		"#asw_player_list_deaths");
	m_pPingHeader = new vgui::Label(this, "PingHeader", "#asw_player_list_ping");

	m_pMissionLabel = new vgui::Label(this, "MissionLabel", "");
	m_pTeam1ScoreLabel = new vgui::Label(this, "Team1Score", "");
	m_pTeam2ScoreLabel = new vgui::Label(this, "Team2Score", "");
	m_pServerLabel = new vgui::Label(this, "ServerLabel", "");
	m_pDifficultyLabel = new vgui::Label(this, "DifficultyLabel", "");	

	m_szMapName[0] = '\0';
	m_iNoCount = -1;
	m_iYesCount = -1;
	m_iSecondsLeft = -1;
	m_fUpdateDifficultyTime = 0;

	m_pVisibilityButton = new CNB_Button( this, "VisibilityButton", "", this, "VisibilityButton" );
	m_pVisibilityLabel = new vgui::Label( this, "VisibilityLabel", "" );
	m_pTipsLabel = new vgui::Label( this, "TipsLabel", "" );

	// voting buttons
	//m_pVoteBackground = new vgui::Panel(this, "VoteBG");
	m_pLeaderButtonsBackground = new vgui::Panel(this, "LeaderBG");
	//m_pStartVoteTitle = new vgui::Label(this, "StartVoteTitle", "#asw_start_vote");
	m_pCurrentVoteTitle = new vgui::Label(this, "CurrentVoteTitle", "#asw_current_vote");	
	m_pYesVotesLabel = new vgui::Label(this, "YesCount", "");
	m_pNoVotesLabel = new vgui::Label(this, "NoCount", "");	
	m_pMapNameLabel = new vgui::WrappedLabel(this, "MapName", " ");
	m_pCounterLabel = new vgui::Label(this, "Counter", "");

	m_pRestartMissionButton = new CNB_Button( this, "RestartButton", "" ); //ImageButton(this, "RestartButton", );
	m_pStartCampaignVoteButton = new CNB_Button(this, "CampaignVoteButton", "#asw_campaign_vote");
	m_pStartSavedCampaignVoteButton = new ImageButton(this, "SavedVoteButton", "#asw_saved_vote");
	m_pStartSavedCampaignVoteButton->SetVisible( false );
	m_pVoteYesButton = new CNB_Button(this, "Yes", "#asw_vote_yes");
	m_pVoteNoButton = new CNB_Button(this, "No", "#asw_vote_no");	

	m_pStartSavedCampaignVoteButton->SetButtonTexture("swarm/Briefing/ShadedButton");
	m_pStartSavedCampaignVoteButton->SetButtonOverTexture("swarm/Briefing/ShadedButton_over");
	
	m_pRestartMissionButton->AddActionSignalTarget( this );
	//m_pStartCampaignVoteButton->AddActionSignalTarget( this );
	m_pStartSavedCampaignVoteButton->AddActionSignalTarget( this );
	m_pVoteYesButton->AddActionSignalTarget( this );
	m_pVoteNoButton->AddActionSignalTarget( this );

	m_pStartSavedCampaignVoteButton->SetContentAlignment(vgui::Label::a_center);

	KeyValues *newcampaignmsg = new KeyValues("Command");
	newcampaignmsg->SetString("command", "NewCampaignVote");
	m_pStartCampaignVoteButton->SetCommand(newcampaignmsg);

	KeyValues *newsavedmsg = new KeyValues("Command");
	newsavedmsg->SetString("command", "NewSavedVote");
	m_pStartSavedCampaignVoteButton->SetCommand(newsavedmsg);

	KeyValues *voteyesmsg = new KeyValues("Command");
	voteyesmsg->SetString("command", "VoteYes");
	m_pVoteYesButton->SetCommand(voteyesmsg);

	KeyValues *votenomsg = new KeyValues("Command");
	votenomsg->SetString("command", "VoteNo");
	m_pVoteNoButton->SetCommand(votenomsg);

	KeyValues *resmsg = new KeyValues("Command");
	resmsg->SetString("command", "RestartMis");
	m_pRestartMissionButton->SetCommand(resmsg);	

	UpdateKickLeaderTicks();

	g_asw_iPlayerListOpen++;

	m_szServerName[0] = '\0';

	m_pPlayerListScroll = new GenericPanelList( this, "PlayerListScroll", GenericPanelList::ISM_ELEVATOR );
	m_pPlayerListScroll->SetScrollBarVisible( IsPC() );
	m_pPlayerListScroll->SetBgColor( Color( 0, 0, 0, 0 ) );
}

PlayerListPanel::~PlayerListPanel()
{
	g_asw_iPlayerListOpen--;
}

void PlayerListPanel::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	LoadControlSettings( "resource/ui/PlayerListPanel.res" );

	vgui::HFont DefaultFont = pScheme->GetFont( "Default", IsProportional() );
	m_pPlayerHeader->SetFont(DefaultFont);
	m_pPlayerHeader->SetFgColor(Color(255,255,255,255));
	m_pMarinesHeader->SetFont(DefaultFont);
	m_pMarinesHeader->SetFgColor(Color(255,255,255,255));
	m_pFragsHeader->SetFont(DefaultFont);
	m_pFragsHeader->SetFgColor(Color(255,255,255,255));
	m_pDeathsHeader->SetFont(DefaultFont);
	m_pDeathsHeader->SetFgColor(Color(255,255,255,255));
	m_pPingHeader->SetFont(DefaultFont);
	m_pPingHeader->SetFgColor(Color(255,255,255,255));
	SetPaintBackgroundEnabled(true);
	SetPaintBackgroundType(0);

	// more translucent when viewed ingame
	if (GetParent() && GetParent()->GetParent() && GetParent()->GetParent() == GetClientMode()->GetViewport())
		SetBgColor(Color(0,0,0,128));
	else
		SetBgColor(Color(0,0,0,220));


	m_pLeaderButtonsBackground->SetPaintBackgroundEnabled(false);	// temp removal of this
	m_pLeaderButtonsBackground->SetPaintBackgroundType(0);
	m_pLeaderButtonsBackground->SetBgColor(Color(0,0,0,128));

	m_pStartSavedCampaignVoteButton->m_pLabel->SetFont(DefaultFont);

	Color col_white(255,255,255,255);
	Color col_grey(128,128,128,255);
	m_pStartSavedCampaignVoteButton->SetColors(col_white, col_grey, col_white, col_grey, col_white);
}

void PlayerListPanel::PerformLayout()
{	
	float fScale = (ScreenHeight() / 768.0f);
	int border = 12.0f * fScale;
	int middle_button_width = 160.0f * fScale;
	int player_list_top = YRES( 85 ); //border * 2 + mission_tall + default_tall * 2;
	int line_top = 35 * fScale + player_list_top;
	int line_height = 48 * fScale;
	int header_height = 32 * fScale;
	int padding = 5 * fScale;	

	if (!GetParent())
		return;

	SetBounds(0, 0, GetParent()->GetWide(),GetParent()->GetTall());
	
	int left_edge = GetWide() * 0.5f - YRES( 250 );

	m_pPlayerHeader->SetBounds( left_edge + PLAYER_LIST_PLAYER_X * fScale, player_list_top, PLAYER_LIST_PLAYER_W * fScale, header_height);
	m_pMarinesHeader->SetBounds( left_edge + PLAYER_LIST_MARINES_X * fScale, player_list_top, PLAYER_LIST_MARINES_W * fScale, header_height);
	m_pFragsHeader->SetBounds( left_edge + PLAYER_LIST_FRAGS_X * fScale, player_list_top, PLAYER_LIST_FRAGS_W * fScale, header_height);
	m_pDeathsHeader->SetBounds( left_edge + PLAYER_LIST_DEATHS_X * fScale, player_list_top, PLAYER_LIST_DEATHS_W * fScale, header_height);
	m_pPingHeader->SetBounds( left_edge + PLAYER_LIST_PING_X * fScale, player_list_top, PLAYER_LIST_PING_W * fScale, header_height);

	m_pPlayerListScroll->SetBounds(left_edge + border - 5, line_top, YRES( 500 ) - 24.0f * fScale + 5, line_height * 8);


	for (int i = m_pPlayerListScroll->GetPanelItemCount(); i < m_PlayerLine.Count(); ++i)
	{
		m_pPlayerListScroll->AddPanelItem(m_PlayerLine[i], true);
		m_PlayerLine[i]->SetBounds( 0, i * (line_height + padding), YRES( 500 ) - 24.0f * fScale, line_height);
	}
	
	int button_height = 32.0f * fScale;
	int button_width = 140.0f * fScale;	
	int wide_button_width = 285.0f * fScale;
	int button_padding = 8.0f * fScale;
	int button_y = YRES( 420 ) - (button_height + button_padding + border);
	int label_height = 32.0f * fScale;
	int button_x = button_padding + border;

	m_pStartSavedCampaignVoteButton->SetBounds( left_edge + button_x + button_width + middle_button_width + button_padding * 2, button_y, wide_button_width, button_height);
	int vote_y = YRES( 420 ) - (button_height * 2 + button_padding * 2 + border);

	bool bVoteActive = (ASWGameRules() && (ASWGameRules()->GetCurrentVoteType() != ASW_VOTE_NONE));
	int bg_top = button_y - (label_height + button_padding);
	if (bVoteActive)
		bg_top = vote_y - button_padding;

	m_pLeaderButtonsBackground->SetBounds( left_edge + border, bg_top - (button_height + border * 3), YRES( 500 ) - (border * 2),
			button_height + border * 2);
}

void PlayerListPanel::OnThink()
{
	// make sure we have enough line panels per player and that each line knows the index of the player its displaying
	int iNumPlayersInGame = 0;
	bool bNeedsLayout = false;
	for ( int j = 1; j <= gpGlobals->maxClients; j++ )
	{	
		if ( g_PR->IsConnected( j ) )
		{
			iNumPlayersInGame++;
			while (m_PlayerLine.Count() <= iNumPlayersInGame)
			{
				// temp comment
				m_PlayerLine.AddToTail(new PlayerListLine(this, "PlayerLine"));
			}
			if ( m_PlayerLine.Count() > ( iNumPlayersInGame - 1 ) && m_PlayerLine[ iNumPlayersInGame - 1 ] )
			{
				m_PlayerLine[iNumPlayersInGame-1]->SetVisible(true);
				if (m_PlayerLine[iNumPlayersInGame-1]->SetPlayerIndex(j))
				{
					bNeedsLayout = true;
				}
			}
		}
	}

	m_pTipsLabel->SetText("");

	IMatchSession *pSession = g_pMatchFramework->GetMatchSession();
	if ( pSession )
	{
		KeyValues *pSettings = pSession->GetSessionSettings();
		if ( pSettings )
		{
			m_pVisibilityLabel->SetVisible( !Q_stricmp( pSettings->GetString( "options/server" ), "listen") );
			m_pVisibilityButton->SetVisible( engine->IsClientLocalToActiveServer() );
			const char* tip;

			if ( !Q_stricmp( pSettings->GetString( "system/access"), "public" ) )
			{
				m_pVisibilityLabel->SetText( "#L4D360UI_Lobby_PublicTitle" );
				m_pVisibilityButton->SetText( "#L4D360UI_Lobby_FriendsTitle" );
				tip = "#L4D360UI_Lobby_MakeFriendOnly_Tip";
			}
			else
			{
				m_pVisibilityLabel->SetText( "#L4D360UI_Lobby_FriendsTitle" );
				m_pVisibilityButton->SetText( "#L4D360UI_Lobby_PublicTitle" );
				tip = "#L4D360UI_Lobby_OpenToPublic_Tip";
			}

			if ( m_pVisibilityButton->IsCursorOver() )
				m_pTipsLabel->SetText( tip );
		}
	}
	else
	{
		m_pVisibilityLabel->SetVisible( false );
		m_pVisibilityButton->SetVisible( false );
	}

	//m_pPlayerListScroll->SetScrollBarVisible( IsPC() && iNumPlayersInGame > 6);

	// hide any extra ones we might have
	for (int i=iNumPlayersInGame;i<m_PlayerLine.Count();i++)
	{
		m_PlayerLine[i]->SetVisible(false);
	}

	UpdateVoteButtons();
	UpdateKickLeaderTicks();
	if (gpGlobals->curtime > m_fUpdateDifficultyTime)
	{
		MissionStatsPanel::SetMissionLabels(m_pMissionLabel, m_pDifficultyLabel);
		m_fUpdateDifficultyTime = gpGlobals->curtime + 1.0f;
	}

	bool is_team_game = ASWDeathmatchMode() && ASWDeathmatchMode()->IsTeamDeathmatchEnabled();
	m_pTeam1ScoreLabel->SetVisible( is_team_game );
	m_pTeam2ScoreLabel->SetVisible( is_team_game );

	if ( is_team_game )
	{
		C_Team *alpha_team = GetGlobalTeam( TEAM_ALPHA );
		C_Team *beta_team = GetGlobalTeam( TEAM_BETA );

		if ( alpha_team && beta_team )
		{
			const char *t1_name = alpha_team->Get_Name();
			const char *t2_name = beta_team->Get_Name();

			if (t1_name && t2_name)
			{
				char t1_score[32];
				char t2_score[32];
				itoa(alpha_team->Get_Score(), t1_score, 10);
				itoa(beta_team->Get_Score(), t2_score, 10);
				char t1_label[1024];
				char t2_label[1024];
				sprintf_s(t1_label, sizeof(t1_label), "%s: %s", t1_name, t1_score);
				sprintf_s(t2_label, sizeof(t2_label), "%s: %s", t2_name, t2_score);

				wchar_t t1_wlabel[1024];
				wchar_t t2_wlabel[1024];
				g_pVGuiLocalize->ConvertANSIToUnicode( t1_label, t1_wlabel, sizeof(t1_wlabel));
				g_pVGuiLocalize->ConvertANSIToUnicode( t2_label, t2_wlabel, sizeof(t2_wlabel));

				m_pTeam1ScoreLabel->SetText( t1_wlabel );
				m_pTeam2ScoreLabel->SetText( t2_wlabel );
			}
		}
		else
		{
			Warning( "Cannot get the needed team for displayin in UI \n" );
		}

	}

	C_ASW_Player *pPlayer = C_ASW_Player::GetLocalASWPlayer();
	bool bShowRestart = pPlayer && ASWGameResource() && (ASWGameResource()->GetLeaderEntIndex() == pPlayer->entindex());
	//Msg("bLeader = %d leaderentindex=%d player entindex=%d\n", bLeader, ASWGameResource()->GetLeaderEntIndex(), pPlayer->entindex());
	m_pRestartMissionButton->SetVisible(bShowRestart);
	m_pLeaderButtonsBackground->SetVisible(bShowRestart);	

	UpdateServerName();

	if (bNeedsLayout)
		InvalidateLayout(true);
}


void PlayerListPanel::KickClicked(PlayerListLine* pLine)
{
	// unselect all the other check boxes
	for (int i=0;i<m_PlayerLine.Count();i++)
	{
		if (m_PlayerLine[i] == pLine)			
		{
			if (m_iKickVoteIndex != m_PlayerLine[i]->m_iPlayerIndex)
			{
				char buffer[64];
				Q_snprintf(buffer, sizeof(buffer), "cl_kickvote %d", m_PlayerLine[i]->m_iPlayerIndex);
				engine->ClientCmd(buffer);
			}
			else	// we were already wanting to kick this player, so toggle it off
			{
				char buffer[64];
				Q_snprintf(buffer, sizeof(buffer), "cl_kickvote -1");
				engine->ClientCmd(buffer);
			}
		}
	}
}

void PlayerListPanel::LeaderClicked(PlayerListLine* pLine)
{
	// unselect all the other check boxes
	for (int i=0;i<m_PlayerLine.Count();i++)
	{
		if (m_PlayerLine[i] == pLine)			
		{
			if (m_iLeaderVoteIndex != m_PlayerLine[i]->m_iPlayerIndex)
			{
				char buffer[64];
				Q_snprintf(buffer, sizeof(buffer), "cl_leadervote %d", m_PlayerLine[i]->m_iPlayerIndex);
				engine->ClientCmd(buffer);
			}
			else	// we were already wanting to kick this play, so toggle it off
			{
				char buffer[64];
				Q_snprintf(buffer, sizeof(buffer), "cl_leadervote -1");
				engine->ClientCmd(buffer);
			}
		}
	}
}

void PlayerListPanel::UpdateVoteButtons()
{
	if (!ASWGameRules())
	{
		SetShowStartVoteElements(false);
		SetShowCurrentVoteElements(false);
		return;
	}

	if (ASWGameRules()->GetCurrentVoteType() == ASW_VOTE_NONE)
	{
		SetShowStartVoteElements(true);
		SetShowCurrentVoteElements(false);
		return;
	}
	
	SetShowStartVoteElements(false);
	SetShowCurrentVoteElements(true);

	// update timer
	int iSecondsLeft = ASWGameRules()->GetCurrentVoteTimeLeft();
	if (iSecondsLeft != m_iSecondsLeft)
	{
		m_iSecondsLeft = iSecondsLeft;
		char buffer[8];
		Q_snprintf(buffer, sizeof(buffer), "%d", iSecondsLeft);

		wchar_t wnumber[8];
		g_pVGuiLocalize->ConvertANSIToUnicode(buffer, wnumber, sizeof( wnumber ));

		wchar_t wbuffer[96];		
		g_pVGuiLocalize->ConstructString( wbuffer, sizeof(wbuffer),
			g_pVGuiLocalize->Find("#asw_time_left"), 1,
				wnumber);
		m_pCounterLabel->SetText(wbuffer);
	}	

	// update count and other labels
	if (m_iYesCount != ASWGameRules()->GetCurrentVoteYes())
	{
		m_iYesCount = ASWGameRules()->GetCurrentVoteYes();
		char buffer[8];
		Q_snprintf(buffer, sizeof(buffer), "%d", m_iYesCount);

		wchar_t wnumber[8];
		g_pVGuiLocalize->ConvertANSIToUnicode(buffer, wnumber, sizeof( wnumber ));

		wchar_t wbuffer[96];		
		g_pVGuiLocalize->ConstructString( wbuffer, sizeof(wbuffer),
			g_pVGuiLocalize->Find("#asw_yes_votes"), 1,
				wnumber);
		m_pYesVotesLabel->SetText(wbuffer);
	}
	if (m_iNoCount != ASWGameRules()->GetCurrentVoteNo())
	{
		m_iNoCount = ASWGameRules()->GetCurrentVoteNo();
		char buffer[8];
		Q_snprintf(buffer, sizeof(buffer), "%d", m_iNoCount);

		wchar_t wnumber[8];
		g_pVGuiLocalize->ConvertANSIToUnicode(buffer, wnumber, sizeof( wnumber ));

		wchar_t wbuffer[96];		
		g_pVGuiLocalize->ConstructString( wbuffer, sizeof(wbuffer),
			g_pVGuiLocalize->Find("#asw_no_votes"), 1,
				wnumber);
		m_pNoVotesLabel->SetText(wbuffer);
	}	
	if (Q_strcmp(m_szMapName, ASWGameRules()->GetCurrentVoteDescription()))
	{
		Q_snprintf(m_szMapName, sizeof(m_szMapName), "%s", ASWGameRules()->GetCurrentVoteDescription());
	
		wchar_t wmapnamebuf[64];
		const wchar_t *wmapname = g_pVGuiLocalize->Find( m_szMapName );
		if ( !wmapname )
		{
			g_pVGuiLocalize->ConvertANSIToUnicode( m_szMapName, wmapnamebuf, sizeof( wmapnamebuf ) );
			wmapname = wmapnamebuf;
		}

		wchar_t wbuffer[96];
		if (ASWGameRules()->GetCurrentVoteType() == ASW_VOTE_CHANGE_MISSION)
		{
			m_bVoteMapInstalled = true;
			if ( missionchooser && missionchooser->LocalMissionSource() )
			{
				if ( !missionchooser->LocalMissionSource()->GetMissionDetails( ASWGameRules()->GetCurrentVoteMapName() ) )
					m_bVoteMapInstalled = false;
			}

			if ( m_bVoteMapInstalled )
			{
				const char *szContainingCampaign = ASWGameRules()->GetCurrentVoteCampaignName();
				if ( !szContainingCampaign || !szContainingCampaign[0] )
				{
					g_pVGuiLocalize->ConstructString( wbuffer, sizeof(wbuffer),
						g_pVGuiLocalize->Find("#asw_current_mission_vote"), 1,
							wmapname);
				}
				else
				{
					// TODO: Show campaign name too?
					g_pVGuiLocalize->ConstructString( wbuffer, sizeof(wbuffer),
						g_pVGuiLocalize->Find("#asw_current_mission_vote"), 1,
						wmapname);
				}
			}
			else
			{
				g_pVGuiLocalize->ConstructString( wbuffer, sizeof(wbuffer),
					g_pVGuiLocalize->Find("#asw_current_mission_vote_not_installed"), 1,
					wmapname);
			}
		}
		else if (ASWGameRules()->GetCurrentVoteType() == ASW_VOTE_SAVED_CAMPAIGN)
		{
			g_pVGuiLocalize->ConstructString( wbuffer, sizeof(wbuffer),
				g_pVGuiLocalize->Find("#asw_current_saved_vote"), 1,
					wmapname);
		}

		int w, t;
		m_pMapNameLabel->GetSize(w, t);
		if (m_pMapNameLabel->GetTextImage())
			m_pMapNameLabel->GetTextImage()->SetSize(w, t);
		m_pMapNameLabel->SetText(wbuffer);
		m_pMapNameLabel->InvalidateLayout(true);
	}		
}

void PlayerListPanel::SetShowStartVoteElements(bool bVisible)
{
	//m_pStartVoteTitle->SetVisible(bVisible);
	m_pStartCampaignVoteButton->SetVisible(bVisible);
	m_pStartSavedCampaignVoteButton->SetVisible(false);	// disable loading saves for now
}

void PlayerListPanel::SetShowCurrentVoteElements(bool bVisible)
{
	m_pCurrentVoteTitle->SetVisible(bVisible);
	m_pVoteYesButton->SetVisible(bVisible);
	m_pVoteNoButton->SetVisible(bVisible);
	m_pYesVotesLabel->SetVisible(bVisible);
	m_pNoVotesLabel->SetVisible(bVisible);
	m_pMapNameLabel->SetVisible(bVisible);
	m_pCounterLabel->SetVisible(bVisible);
}

void PlayerListPanel::OnCommand( char const *cmd )
{
	if ( !V_stricmp( cmd, "NewMissionVote" ) || !V_stricmp( cmd, "NewCampaignVote" ) || !V_stricmp( cmd, "NewSavedVote" ) )
	{
		GetParent()->SetVisible( false );
		GetParent()->MarkForDeletion();
		engine->ClientCmd( "asw_mission_chooser callvote" );
	}
	else if ( !V_stricmp( cmd, "VoteYes" ) )
	{
		GetParent()->SetVisible( false );
		GetParent()->MarkForDeletion();
		engine->ClientCmd( "vote_yes" );
	}
	else if ( !V_stricmp( cmd, "VoteNo" ) )
	{
		GetParent()->SetVisible( false );
		GetParent()->MarkForDeletion();
		engine->ClientCmd( "vote_no" );
	}
	else if ( !V_stricmp( cmd, "Back" ) )
	{
		GetParent()->SetVisible( false );
		GetParent()->MarkForDeletion();
	}
	else if ( !V_stricmp( cmd, "VisibilityButton" ) )
	{
		IMatchSession *pSession = g_pMatchFramework->GetMatchSession();
		if ( pSession )
		{
			KeyValues *pSettings = pSession->GetSessionSettings();
			if ( pSettings )
			{
				if ( !Q_stricmp( pSettings->GetString( "system/access" ), "public" ) )
					engine->ClientCmd( "make_game_friends_only" );
				else
					engine->ClientCmd( "make_game_public" );
			}
		}
		return;
	}
	else if ( !V_stricmp( cmd, "RestartMis" ) )
	{
		C_ASW_Player *pPlayer = C_ASW_Player::GetLocalASWPlayer();
		if ( !pPlayer )
			return;
		bool bLeader = pPlayer && ASWGameResource() && ASWGameResource()->GetLeaderEntIndex() == pPlayer->entindex();
		if ( !bLeader )
			return;

		bool bCanStart = ASWGameRules() &&
			( ( ASWGameRules()->GetGameState() <= ASW_GS_INGAME )
				|| ( ASWGameResource() && ASWGameResource()->AreAllOtherPlayersReady( pPlayer->entindex() ) ) );

		if ( bCanStart )
		{
			GetParent()->SetVisible( false );
			GetParent()->MarkForDeletion();
			engine->ClientCmd( "asw_restart_mission" );
		}
		else
		{
			if ( GetParent() && GetParent()->GetParent() )
			{
				vgui::Panel *p = new ForceReadyPanel( GetParent()->GetParent(), "ForceReady", "#asw_force_restartm", ASW_FR_RESTART );
				p->SetZPos( 201 );	// make sure it's in front of the player list
			}
		}
	}
	else
	{
		BaseClass::OnCommand( cmd );
	}
}

void PlayerListPanel::UpdateKickLeaderTicks()
{
	m_iKickVoteIndex = -1;
	m_iLeaderVoteIndex = -1;
	C_ASW_Player *pPlayer = C_ASW_Player::GetLocalASWPlayer();
	if (pPlayer)
	{
		m_iKickVoteIndex = pPlayer->m_iKickVoteIndex;
		m_iLeaderVoteIndex = pPlayer->m_iLeaderVoteIndex;
	}
}

void PlayerListPanel::UpdateServerName()
{
	const char* szServerName="";
	if (gpGlobals->maxClients > 1)
	{
		CASWHudMinimap *pMinimap = GET_HUDELEMENT( CASWHudMinimap );
		if (pMinimap)
			szServerName = pMinimap->m_szServerName;
	}
	if (Q_strcmp(szServerName, m_szServerName))
	{
		Q_snprintf(m_szServerName, sizeof(m_szServerName), "%s", szServerName);
		if (m_pServerLabel)
		{
			// post message to avoid translation
			PostMessage( m_pServerLabel, new KeyValues( "SetText", "text", m_szServerName ) );
		}
	}
}