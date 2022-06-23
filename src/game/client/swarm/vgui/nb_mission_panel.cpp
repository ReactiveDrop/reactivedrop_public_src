#include "cbase.h"
#include "nb_mission_panel.h"
#include "vgui_controls/Label.h"
#include "vgui_controls/ImagePanel.h"
#include "vgui_controls/Button.h"
#include <vgui/ILocalize.h>
#include "ObjectivePanel.h"
#include "ObjectiveListBox.h"
#include "ObjectiveDetailsPanel.h"
#include "ObjectiveMap.h"
#include "ObjectiveIcons.h"
#include "asw_hud_minimap.h"
#include "asw_gamerules.h"
#include "c_asw_campaign_save.h"
#include "nb_header_footer.h"
#include "nb_button.h"
#include "gameui/swarm/vdropdownmenu.h"
#include "c_asw_player.h"
#include "c_asw_game_resource.h"
#include "asw_input.h"
#include "nb_island.h"
#include "gameui/swarm/basemodpanel.h"
#include "gameui/swarm/VFooterPanel.h"
#include "asw_hud_chat.h"
#include "asw_deathmatch_mode.h"
#include "rd_challenges_shared.h"
#include "gameui/swarm/vflyoutmenu.h"
#include "gameui/swarm/vhybridbutton.h"
#include "gameui/swarm/rd_challenge_selection.h"
#include "controller_focus.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

#define MAP_SIZE 0.66666f

CNB_Mission_Panel::CNB_Mission_Panel( vgui::Panel *parent, const char *name ) : BaseClass( parent, name )
{
	m_pHeaderFooter = new CNB_Header_Footer( this, "HeaderFooter" );
	m_pObjectiveListBoxIsland = new CNB_Island( this, "ObjectiveListBoxIsland" );
	m_pObjectiveListBoxIsland->m_pTitle->SetText( "#nb_objectives" );
	m_pObjectiveMapIsland = new CNB_Island( this, "ObjectiveMapIsland" );
	m_pObjectiveMapIsland->m_pTitle->SetText( "#nb_overview_map" );
	m_pObjectiveDetailsIsland = new CNB_Island( this, "ObjectiveDetailsIsland" );
	m_pObjectiveDetailsIsland->m_pTitle->SetText( "#nb_objective_details" );
	m_pDifficultyIsland = new CNB_Island( this, "DifficultyIsland" );
	m_pDifficultyIsland->m_pTitle->SetText( "#nb_mission_options" );
	m_pDifficultyDescription = new vgui::Label( this, "DifficultyDescription", "" );
	// == MANAGED_MEMBER_CREATION_START: Do not edit by hand ==
	m_pRetriesLabel = new vgui::Label( this, "RetriesLabel", "" );
	// == MANAGED_MEMBER_CREATION_END ==
	m_pBackButton = new CNB_Button( this, "BackButton", "", this, "BackButton" );
	m_drpDifficulty = new BaseModUI::DropDownMenu( this, "DrpDifficulty" );
	m_drpGameMode = new BaseModUI::DropDownMenu( this, "DrpGameMode" );
	m_drpFriendlyFire = new BaseModUI::DropDownMenu( this, "DrpFriendlyFire" );
	m_drpOnslaught = new BaseModUI::DropDownMenu( this, "DrpOnslaught" );
	m_drpChallenge = new BaseModUI::DropDownMenu(this, "DrpSelectChallenge");
	m_drpFixedSkillPoints = new BaseModUI::DropDownMenu( this, "DrpFixedSkillPoints" );

	m_pHeaderFooter->SetTitle( "#nb_mission_details" );

	m_pObjectiveList = new ObjectiveListBox(this, "objectivelistbox");
	m_pObjectiveDetails = new ObjectiveDetailsPanel(this, "objectivedetails");
	m_pObjectiveMap = new ObjectiveMap(this, "objectivemap");

	m_pObjectiveMap->SetDetailsPanel(m_pObjectiveDetails);
	m_pObjectiveList->SetDetailsPanel(m_pObjectiveDetails);
	m_pObjectiveList->SetMapPanel(m_pObjectiveMap);

	m_bSelectedFirstObjective = false;

	m_iLastSkillLevel = -1;
	m_iLastGameMode = -1;
	m_iLastFixedSkillPoints = -1;
	m_iLastHardcoreFF = -1;
	m_iLastOnslaught = -1;
	m_bIgnoreSelections = false;
	m_bAddedDropdownsToControllerFocus = false;

	// back button was auto-added, but we need it to be marked as modal
	GetControllerFocus()->RemoveFromFocusList( m_pBackButton );
	GetControllerFocus()->AddToFocusList( m_pBackButton, false, true );
}

CNB_Mission_Panel::~CNB_Mission_Panel()
{
	if ( m_bAddedDropdownsToControllerFocus )
	{
		GetControllerFocus()->RemoveFromFocusList( m_drpDifficulty->GetButton() );
		GetControllerFocus()->RemoveFromFocusList( m_drpGameMode->GetButton() );
		GetControllerFocus()->RemoveFromFocusList( m_drpFriendlyFire->GetButton() );
		GetControllerFocus()->RemoveFromFocusList( m_drpOnslaught->GetButton() );
		GetControllerFocus()->RemoveFromFocusList( m_drpChallenge->GetButton() );
	}
}

void CNB_Mission_Panel::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );
	
	LoadControlSettings( "resource/ui/nb_mission_panel.res" );

	if ( ASWGameRules()->GetGameState() == ASW_GS_INGAME )
	{
		m_pObjectiveMap->m_pMapImage->SetAlpha( 90 );
		m_pHeaderFooter->SetBackgroundStyle( NB_BACKGROUND_TRANSPARENT_BLUE );
		m_pHeaderFooter->SetMovieEnabled( false );
// 		m_pObjectiveDetails->SetVisible( false );
// 		m_pObjectiveMap->SetWide( m_pObjectiveMap->GetWide() * 1.5f );
// 		m_pObjectiveMap->SetTall( m_pObjectiveMap->GetTall() * 1.5f );
		//m_drpDifficulty->SetVisible( false );
		m_drpFixedSkillPoints->SetVisible( false );
	}

	if ( !m_bAddedDropdownsToControllerFocus )
	{
		m_bAddedDropdownsToControllerFocus = true;
		GetControllerFocus()->AddToFocusList( m_drpDifficulty->GetButton(), false, true );
		GetControllerFocus()->AddToFocusList( m_drpGameMode->GetButton(), false, true );
		GetControllerFocus()->AddToFocusList( m_drpFriendlyFire->GetButton(), false, true );
		GetControllerFocus()->AddToFocusList( m_drpOnslaught->GetButton(), false, true );
		GetControllerFocus()->AddToFocusList( m_drpChallenge->GetButton(), false, true );
	}
}

void CNB_Mission_Panel::PerformLayout()
{
	BaseClass::PerformLayout();

	m_pObjectiveList->InvalidateLayout( true );
	m_pObjectiveDetails->InvalidateLayout( true );
	m_pObjectiveMap->InvalidateLayout( true );

	int border = YRES( 2 );
	int title_size = YRES( 30 );
	int x, y, w, t;
	m_pObjectiveList->GetBounds( x, y, w, t );
	m_pObjectiveListBoxIsland->SetBounds( x - border, y - border - title_size, w + border * 2, t + border * 2 + title_size );

	m_pObjectiveMap->GetBounds( x, y, w, t );
	m_pObjectiveMapIsland->SetBounds( x - border, y - border - title_size, w + border * 2, t + border * 2 + title_size );

	m_pObjectiveDetails->GetBounds( x, y, w, t );
	m_pObjectiveDetailsIsland->SetBounds( x - border, y - border - title_size, w + border * 2, t + border * 2 + title_size );
}

void CNB_Mission_Panel::OnThink()
{
	BaseClass::OnThink();

	if (!m_bSelectedFirstObjective && GetAlpha()>=200)
	{
		if ( m_pObjectiveList->ClickFirstTitle() )
		{
			m_bSelectedFirstObjective = true;
			m_pObjectiveDetails->InvalidateLayout( true, true );
		}
	}

	CASWHudMinimap *pMap = GET_HUDELEMENT( CASWHudMinimap );
	if ( pMap )
	{
		wchar_t wszMissionTitle[ 128 ];
		if ( pMap->m_szMissionTitle[0] == '#' )
		{
			V_snwprintf( wszMissionTitle, ARRAYSIZE( wszMissionTitle ), L"%s", g_pVGuiLocalize->FindSafe( pMap->m_szMissionTitle ) );
		}
		else
		{
			g_pVGuiLocalize->ConvertANSIToUnicode( pMap->m_szMissionTitle, wszMissionTitle, sizeof( wszMissionTitle ) );
		}
		wchar_t wszTitleText[ 255 ];
		g_pVGuiLocalize->ConstructString( wszTitleText, sizeof( wszTitleText ), g_pVGuiLocalize->Find( "#nb_mission_details" ), 1, wszMissionTitle );

		m_pHeaderFooter->SetTitle( wszTitleText );
	}
	if ( ASWGameRules() && ASWGameRules()->GetCampaignSave() )
	{
		int iRetries = ASWGameRules()->GetCampaignSave()->GetRetries();

		if ( ASWGameRules()->GetGameState() != ASW_GS_BRIEFING )	// #8 Number of retries in UI shows a number that is bigger by 1
			iRetries -= 1;	// since retries counter is incremented on mission start it is safer to -1 it here in UI

		if ( iRetries > 0 )
		{
			m_pRetriesLabel->SetVisible( true );
			wchar_t number[20];
			V_snwprintf( number, sizeof( number ), L"%d", iRetries );
			wchar_t buffer[64];
			g_pVGuiLocalize->ConstructString( buffer, sizeof( buffer ), g_pVGuiLocalize->Find( "#asw_campaign_retries_label" ), 1, number );
			m_pRetriesLabel->SetText( buffer );
		}
		else
		{
			m_pRetriesLabel->SetVisible( false );
		}
	}
	else
	{	
		m_pRetriesLabel->SetVisible(false);
	}

	if ( !ASWGameRules() )
		return;

	m_bIgnoreSelections = true;

	// disable mission settings flyouts if not leader or the mission has started
	int iLeaderIndex = ASWGameResource() ? ASWGameResource()->GetLeaderEntIndex() : -1;
	C_ASW_Player *pPlayer = C_ASW_Player::GetLocalASWPlayer();
	bool bLeader = ( pPlayer && (pPlayer->entindex() == iLeaderIndex ) );

	m_drpDifficulty->SetEnabled( ASWGameRules()->GetGameState() == ASW_GS_BRIEFING && bLeader );
	m_drpFriendlyFire->SetEnabled( ASWGameRules()->GetGameState() == ASW_GS_BRIEFING && bLeader && !ForceHardcoreFF() );
	const bool bDeathmatchIngme =  ASWDeathmatchMode() && ASWGameRules()->GetGameState() == ASW_GS_INGAME;
	const bool bInBriefing = ASWGameRules()->GetGameState() == ASW_GS_BRIEFING;
	m_drpGameMode->SetEnabled( ( bDeathmatchIngme || bInBriefing ) && bLeader );
	m_drpOnslaught->SetEnabled( (bDeathmatchIngme || bInBriefing) && bLeader && !ForceOnslaught() );
	m_drpChallenge->SetEnabled( ASWGameRules()->GetGameState() == ASW_GS_BRIEFING && bLeader);
	m_drpFixedSkillPoints->SetEnabled( false ); //ASWGameRules()->GetGameState() == ASW_GS_BRIEFING && bLeader );

	if (m_iLastSkillLevel != ASWGameRules()->GetSkillLevel())
	{
		m_iLastSkillLevel = ASWGameRules()->GetSkillLevel();
		if (m_iLastSkillLevel == 4)
		{
			m_drpDifficulty->SetCurrentSelection("#L4D360UI_Difficulty_insane");
			//m_pDifficultyDescription->SetText( "#asw_difficulty_chooser_insaned" );
		}
		else if (m_iLastSkillLevel == 3)
		{
			m_drpDifficulty->SetCurrentSelection("#L4D360UI_Difficulty_hard");
			//m_pDifficultyDescription->SetText( "#asw_difficulty_chooser_hardd" );
		}
		else if (m_iLastSkillLevel == 1)
		{
			m_drpDifficulty->SetCurrentSelection("#L4D360UI_Difficulty_easy");
			//m_pDifficultyDescription->SetText( "#asw_difficulty_chooser_easyd" );
		}
		else if (m_iLastSkillLevel == 5)
		{
			m_drpDifficulty->SetCurrentSelection("#L4D360UI_Difficulty_imba");
			//m_pDifficultyDescription->SetText( "#asw_difficulty_chooser_imbad" );
		}
		else 
		{
			m_drpDifficulty->SetCurrentSelection("#L4D360UI_Difficulty_normal");
			//m_pDifficultyDescription->SetText( "#asw_difficulty_chooser_normald" );
		}
	}

	if (ASWDeathmatchMode() && m_iLastGameMode != ASWDeathmatchMode()->GetGameMode())
	{
		m_iLastGameMode = ASWDeathmatchMode()->GetGameMode();
		if (m_iLastGameMode == GAMEMODE_TEAMDEATHMATCH)
		{
			m_drpGameMode->SetCurrentSelection("#rdui_tdm_title");
		}
		else if (m_iLastGameMode == GAMEMODE_INSTAGIB)
		{
			m_drpGameMode->SetCurrentSelection("#rdui_instagib_title");
		}
		else if (m_iLastGameMode == GAMEMODE_GUNGAME)
		{
			m_drpGameMode->SetCurrentSelection("#rdui_gungame_title");
		}
		else if (m_iLastGameMode == GAMEMODE_DEATHMATCH)
		{
			m_drpGameMode->SetCurrentSelection("#rdui_deathmatch_title");
		}
	}

	extern ConVar asw_sentry_friendly_fire_scale;
	extern ConVar asw_marine_ff_absorption;
	int nHardcoreFF = ( asw_sentry_friendly_fire_scale.GetFloat() != 0.0f || asw_marine_ff_absorption.GetInt() != 1 ) ? 1 : 0;
	if ( m_iLastHardcoreFF != nHardcoreFF )
	{
		m_iLastHardcoreFF = nHardcoreFF;
		if ( nHardcoreFF == 1 )
		{
			m_drpFriendlyFire->SetCurrentSelection( "#L4D360UI_HardcoreFF" );
		}
		else
		{
			m_drpFriendlyFire->SetCurrentSelection( "#L4D360UI_RegularFF" );
		}
	}
	extern ConVar asw_horde_override;
	extern ConVar asw_wanderer_override;
	int nOnslaught = ( asw_horde_override.GetBool() || asw_wanderer_override.GetBool() ) ? 1 : 0;
	if ( m_iLastOnslaught != nOnslaught )
	{
		m_iLastOnslaught = nOnslaught;
		if ( nOnslaught == 1 )
		{
			m_drpOnslaught->SetCurrentSelection( "#L4D360UI_OnslaughtEnabled" );
		}
		else
		{
			m_drpOnslaught->SetCurrentSelection( "#L4D360UI_OnslaughtDisabled" );
		}
	}

	extern ConVar rd_challenge;
	m_drpChallenge->SetCurrentSelection( ReactiveDropChallenges::DisplayName( rd_challenge.GetString() ) );

	BaseModUI::CBaseModFooterPanel *footer = BaseModUI::CBaseModPanel::GetSingleton().GetFooterPanel();
	if ( footer )
	{
		m_pDifficultyDescription->SetText( footer->GetHelpText() );
	}

	// reactivedrop: commented. To enable Insane and Brutal for singleplayer
	// // only show insane in multiplayer
	// m_drpDifficulty->SetFlyoutItemEnabled( "BtnImpossible", gpGlobals->maxClients > 1 );
	// m_drpDifficulty->SetFlyoutItemEnabled( "BtnImba", gpGlobals->maxClients > 1 );

	// reactivedrop: hide difficulty drop down for deathmatch
	// and show game mode selection instead
	m_drpDifficulty->SetVisible( ASWDeathmatchMode() == NULL );
	m_drpGameMode->SetVisible( ASWDeathmatchMode() != NULL );

	if ( ASWGameRules()->GetCampaignSave() && ASWGameRules()->GetGameState() != ASW_GS_INGAME )
	{
		int iFixedSkillPoints = ASWGameRules()->GetCampaignSave()->UsingFixedSkillPoints() ? 1 : 0;
		if ( iFixedSkillPoints != m_iLastFixedSkillPoints )
		{
			if ( iFixedSkillPoints == 1 )
			{
				m_drpFixedSkillPoints->SetCurrentSelection( "#nb_skill_points_fixed" );
				m_drpFixedSkillPoints->SetVisible( false );
			}
			else
			{
				m_drpFixedSkillPoints->SetCurrentSelection( "#nb_skill_points_custom" );
				m_drpFixedSkillPoints->SetVisible( true );
			}
			m_iLastFixedSkillPoints = iFixedSkillPoints;
		}
	}
	else
	{
		m_drpFixedSkillPoints->SetVisible( false );
	}

	m_bIgnoreSelections = false;
}

void CNB_Mission_Panel::OnCommand( const char *command )
{
	if ( !Q_stricmp( command, "BackButton" ) )
	{
		InGameMissionPanelFrame *pParent = dynamic_cast<InGameMissionPanelFrame*>( GetParent() );
		if ( pParent )
		{
			pParent->MarkForDeletion();
		}
		else
		{
			MarkForDeletion();
		}
		return;
	}
	else if ( !Q_stricmp( command, "cmd_change_challenge" ) )
	{
		extern ConVar rd_challenge;
		BaseModUI::ReactiveDropChallengeSelection *pPanel = new BaseModUI::ReactiveDropChallengeSelection( this, "ReactiveDropChallengeSelection" );
		pPanel->SetSelectedChallenge( rd_challenge.GetString() );
		pPanel->MoveToFront();

		return;
	}
	else if ( !Q_stricmp( command, "fixed_skill_points" ) )
	{
		if ( !m_bIgnoreSelections )
			engine->ClientCmd( "cl_fixedskills 1" );
		return;
	}
	else if ( !Q_stricmp( command, "custom_skill_points" ) )
	{
		if ( !m_bIgnoreSelections )
			engine->ClientCmd( "cl_fixedskills 0" );
		return;
	}
	else if ( !Q_stricmp( command, "#L4D360UI_Difficulty_easy" ) )
	{
		if ( !m_bIgnoreSelections )
			engine->ClientCmd( "cl_skill 1" );
		return;
	}
	else if ( !Q_stricmp( command, "#L4D360UI_Difficulty_normal" ) )
	{
		if ( !m_bIgnoreSelections )
			engine->ClientCmd( "cl_skill 2" );
		return;
	}
	else if ( !Q_stricmp( command, "#L4D360UI_Difficulty_hard" ) )
	{
		if ( !m_bIgnoreSelections )
			engine->ClientCmd( "cl_skill 3" );
		return;
	}
	else if ( !Q_stricmp( command, "#L4D360UI_Difficulty_insane" ) )
	{
		if ( !m_bIgnoreSelections )
			engine->ClientCmd( "cl_skill 4" );
		return;
	}
	else if ( !Q_stricmp( command, "#L4D360UI_Difficulty_imba" ) )
	{
		if ( !m_bIgnoreSelections )
			engine->ClientCmd( "cl_skill 5" );
		return;
	}
	else if ( !Q_stricmp( command, "#rdui_deathmatch_title" ) )
	{
		if ( !m_bIgnoreSelections )
			engine->ClientCmd( "rd_Deathmatch_enable" );
		return;
	}
	else if ( !Q_stricmp( command, "#rdui_gungame_title" ) )
	{
		if ( !m_bIgnoreSelections )
			engine->ClientCmd( "rd_gungame_enable" );
		return;
	}
	else if ( !Q_stricmp( command, "#rdui_instagib_title" ) )
	{
		if ( !m_bIgnoreSelections )
			engine->ClientCmd( "rd_instagib_enable" );
		return;
	}
	else if ( !Q_stricmp( command, "#rdui_tdm_title" ) )
	{
		if ( !m_bIgnoreSelections )
			engine->ClientCmd( "rd_TeamDeathmatch_enable" );
		return;
	}
	else if ( !Q_stricmp( command, "#L4D360UI_RegularFF" ) )
	{
		if ( !m_bIgnoreSelections )
			engine->ClientCmd( "cl_hardcore_ff 0" );
		return;
	}
	else if ( !Q_stricmp( command, "#L4D360UI_HardcoreFF" ) )
	{
		if ( !m_bIgnoreSelections )
			engine->ClientCmd( "cl_hardcore_ff 1" );
		return;
	}
	else if ( !Q_stricmp( command, "#L4D360UI_OnslaughtDisabled" ) )
	{
		if ( !m_bIgnoreSelections )
			engine->ClientCmd( "cl_onslaught 0" );
		return;
	}
	else if ( !Q_stricmp( command, "#L4D360UI_OnslaughtEnabled" ) )
	{
		if ( !m_bIgnoreSelections )
			engine->ClientCmd( "cl_onslaught 1" );
		return;
	}
	else if ( const char *szChallengeName = StringAfterPrefix( command, "cmd_challenge_selected_" ) )
	{
		if ( !m_bIgnoreSelections )
		{
			m_drpChallenge->SetCurrentSelection( ReactiveDropChallenges::DisplayName( szChallengeName ) );
			engine->ClientCmd( VarArgs( "rd_set_challenge %s", szChallengeName ) );
		}
		return;
	}
	BaseClass::OnCommand( command );
}

bool CNB_Mission_Panel::ForceHardcoreFF()
{
	extern ConVar rd_challenge;

	if ( const RD_Challenge_t *pChallenge = ReactiveDropChallenges::GetSummary( rd_challenge.GetString() ) )
	{
		return pChallenge->ForceHardcore;
	}

	return false;
}

bool CNB_Mission_Panel::ForceOnslaught()
{
	extern ConVar rd_challenge;

	if ( const RD_Challenge_t *pChallenge = ReactiveDropChallenges::GetSummary( rd_challenge.GetString() ) )
	{
		return pChallenge->ForceOnslaught;
	}

	return false;
}


// ================================== Frame container ==================================================

InGameMissionPanelFrame::InGameMissionPanelFrame(Panel *parent, const char *panelName, bool showTaskbarIcon) :
vgui::Frame(parent, panelName, showTaskbarIcon)
{
	SetMoveable(false);
	SetSizeable(false);
	SetMenuButtonVisible(false);
	SetMaximizeButtonVisible(false);
	SetMinimizeToSysTrayButtonVisible(false);
	SetCloseButtonVisible(true);
	SetTitleBarVisible(false);
	m_iAnimated = 0;

	ASWInput()->SetCameraFixed( true );
}

InGameMissionPanelFrame::~InGameMissionPanelFrame()
{
	ASWInput()->SetCameraFixed( false );
}

void InGameMissionPanelFrame::PerformLayout()
{
	BaseClass::PerformLayout();

	int border = 0;
	int x = border;
	int y = border;
	int w = ScreenWidth() - border * 2;
	int h = ScreenHeight() - border * 2;


// 	if ( m_iAnimated < 1 )
// 	{
// 	Msg("Starting animation m_iAnimated=%d\n", m_iAnimated);
// 	int cx, cy, cw, ct;
// 	vgui::Panel *pContainer = GetClientMode()->GetViewport()->FindChildByName("MapBackdrop", true);
// 	pContainer->GetBounds( cx, cy, cw, ct);
// 	SetBounds( cx, cy, cw, ct );
// 
// 	float fDuration = 0.5f;
// 	GetAnimationController()->RunAnimationCommand(this, "xpos", x, 0, fDuration, AnimationController::INTERPOLATOR_LINEAR);
// 	GetAnimationController()->RunAnimationCommand(this, "ypos", y, 0, fDuration, AnimationController::INTERPOLATOR_LINEAR);
// 	GetAnimationController()->RunAnimationCommand(this, "wide", w, 0, fDuration, AnimationController::INTERPOLATOR_LINEAR);
// 	GetAnimationController()->RunAnimationCommand(this, "tall", h, 0, fDuration, AnimationController::INTERPOLATOR_LINEAR);
// 	m_iAnimated++;
// 	}

	SetBounds( x, y, w, h );
}

void InGameMissionPanelFrame::ApplySchemeSettings(vgui::IScheme *scheme)
{
	BaseClass::ApplySchemeSettings(scheme);

	SetPaintBackgroundEnabled(false);
	SetBgColor(Color(0,0,0,64));
}