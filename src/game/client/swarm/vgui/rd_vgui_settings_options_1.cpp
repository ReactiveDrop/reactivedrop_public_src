#include "cbase.h"
#include "rd_vgui_settings.h"
#include "gameui/swarm/vgenericconfirmation.h"
#include "gameui/swarm/vhybridbutton.h"
#include "c_gameinstructor.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace BaseModUI;

DECLARE_BUILD_FACTORY( CRD_VGUI_Settings_Options_1 );

extern ConVar asw_crosshair_use_perspective;
extern ConVar asw_fast_reload_under_marine;

CRD_VGUI_Settings_Options_1::CRD_VGUI_Settings_Options_1( vgui::Panel *parent, const char *panelName ) :
	BaseClass( parent, panelName )
{
	// Players
	m_pSettingPlayerNameMode = new CRD_VGUI_Option( this, "SettingPlayerNameMode", "#rd_option_player_name_mode", CRD_VGUI_Option::MODE_DROPDOWN );
	m_pSettingPlayerNameMode->AddOption( 0, "#rd_option_player_name_mode_none", "#rd_option_player_name_mode_none_hint" );
	m_pSettingPlayerNameMode->AddOption( 1, "#rd_option_player_name_mode_player", "#rd_option_player_name_mode_player_hint" );
	m_pSettingPlayerNameMode->AddOption( 2, "#rd_option_player_name_mode_marine", "#rd_option_player_name_mode_marine_hint" );
	m_pSettingPlayerNameMode->AddOption( 3, "#rd_option_player_name_mode_both", "#rd_option_player_name_mode_both_hint" );
	m_pSettingPlayerNameMode->LinkToConVar( "asw_player_names", true );
	m_pSettingPlayerChatColor = new CRD_VGUI_Option( this, "SettingPlayerChatColor", "#rd_option_player_chat_color", CRD_VGUI_Option::MODE_COLOR );
	m_pSettingPlayerChatColor->SetDefaultHint( "#rd_option_player_chat_color_hint" );
	m_pSettingPlayerChatColor->LinkToConVar( "cl_chatcolor", true );
	m_pSettingPlayerChatNamesUseColors = new CRD_VGUI_Option( this, "SettingPlayerChatNamesUseColors", "#rd_option_player_chat_names_use_colors", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingPlayerChatNamesUseColors->SetDefaultHint( "#rd_option_player_chat_names_use_colors_hint" );
	m_pSettingPlayerChatNamesUseColors->LinkToConVar( "rd_chat_colorful_player_names", true );
	m_pSettingPlayerDeathmatchTeamColorMode = new CRD_VGUI_Option( this, "SettingPlayerDeathmatchTeamColorMode", "#rd_option_player_deathmatch_team_color_mode", CRD_VGUI_Option::MODE_DROPDOWN );
	m_pSettingPlayerDeathmatchTeamColorMode->AddOption( 1, "#rd_option_player_deathmatch_team_color_mode_ally_enemy", "#rd_option_player_deathmatch_team_color_mode_ally_enemy_hint" );
	m_pSettingPlayerDeathmatchTeamColorMode->AddOption( 2, "#rd_option_player_deathmatch_team_color_mode_counter_strike", "#rd_option_player_deathmatch_team_color_mode_counter_strike_hint" );
	m_pSettingPlayerDeathmatchTeamColorMode->LinkToConVar( "rd_deathmatch_team_colors", false );
	m_pSettingPlayerDeathmatchDrawTopScoreboard = new CRD_VGUI_Option( this, "SettingPlayerDeathmatchDrawTopScoreboard", "#rd_option_player_deathmatch_draw_top_scoreboard", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingPlayerDeathmatchDrawTopScoreboard->SetDefaultHint( "#rd_option_player_deathmatch_draw_top_scoreboard_hint" );
	m_pSettingPlayerDeathmatchDrawTopScoreboard->LinkToConVar( "rd_draw_avatars_with_frags", true );

	// Hints
	m_pSettingHintsFailAdvice = new CRD_VGUI_Option( this, "SettingHintsFailAdvice", "#rd_option_hints_fail_advice", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingHintsFailAdvice->SetDefaultHint( "#rd_option_hints_fail_advice_hint" );
	m_pSettingHintsFailAdvice->LinkToConVar( "rd_fail_advice", true );
	m_pSettingHintsGameInstructor = new CRD_VGUI_Option( this, "SettingHintsGameInstructor", "#rd_option_hints_game_instructor", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingHintsGameInstructor->SetDefaultHint( "#rd_option_hints_game_instructor_hint" );
	m_pSettingHintsGameInstructor->LinkToConVar( "gameinstructor_enable", true );
	m_pBtnResetGameInstructor = new BaseModHybridButton( this, "BtnResetGameInstructor", "#rd_reset_game_instructor_progress", this, "ResetGameInstructorCounts" );
	m_pSettingHintsDeathmatchRespawn = new CRD_VGUI_Option( this, "SettingHintsDeathmatchRespawn", "#rd_option_hints_deathmatch_respawn", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingHintsDeathmatchRespawn->SetDefaultHint( "#rd_option_hints_deathmatch_respawn_hint" );
	m_pSettingHintsDeathmatchRespawn->LinkToConVar( "rd_deathmatch_respawn_hints", true );
	m_pSettingHintsSwarmopediaGrid = new CRD_VGUI_Option( this, "SettingHintsSwarmopediaGrid", "#rd_option_hints_swarmopedia_grid", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingHintsSwarmopediaGrid->SetDefaultHint( "#rd_option_hints_swarmopedia_grid_hint" );
	m_pSettingHintsSwarmopediaGrid->LinkToConVar( "rd_swarmopedia_grid", true );
	m_pSettingHintsSwarmopediaUnits = new CRD_VGUI_Option( this, "SettingHintsSwarmopediaUnits", "#RD_AdvancedSettings_Misc_SwarmopediaUnits", CRD_VGUI_Option::MODE_DROPDOWN );
	m_pSettingHintsSwarmopediaUnits->AddOption( 0, "#RD_AdvancedSettings_Misc_SwarmopediaUnits_Hammer", "#rd_option_hints_swarmopedia_units_hint" );
	m_pSettingHintsSwarmopediaUnits->AddOption( 1, "#RD_AdvancedSettings_Misc_SwarmopediaUnits_Metric", "#rd_option_hints_swarmopedia_units_hint" );
	m_pSettingHintsSwarmopediaUnits->AddOption( 2, "#RD_AdvancedSettings_Misc_SwarmopediaUnits_Imperial", "#rd_option_hints_swarmopedia_units_hint" );
	m_pSettingHintsSwarmopediaUnits->LinkToConVar( "rd_swarmopedia_units_preference", false );

	// Death
	m_pSettingDeathCamTakeover = new CRD_VGUI_Option( this, "SettingDeathCamTakeover", "#rd_option_death_cam_takeover", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingDeathCamTakeover->SetDefaultHint( "#rd_option_death_cam_takeover_hint" );
	m_pSettingDeathCamTakeover->LinkToConVar( "asw_marine_death_cam", true );
	m_pSettingDeathCamSlowdown = new CRD_VGUI_Option( this, "SettingDeathCamSlowdown", "#rd_option_death_cam_slowdown", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingDeathCamSlowdown->SetDefaultHint( "#rd_option_death_cam_slowdown_hint" );
	m_pSettingDeathCamSlowdown->LinkToConVar( "asw_marine_death_cam_slowdown", true );
	m_pSettingDeathMarineGibs = new CRD_VGUI_Option( this, "SettingDeathMarineGibs", "#rd_option_death_marine_gibs", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingDeathMarineGibs->SetDefaultHint( "#rd_option_death_marine_gibs_hint" );
	m_pSettingDeathMarineGibs->LinkToConVar( "rd_marine_explodes_into_gibs", true );

	// Advanced Controls
	m_pSettingControlsRightClickWireHack = new CRD_VGUI_Option( this, "SettingControlsRightClickWireHack", "#RD_AdvancedSettings_Misc_WireHackClick", CRD_VGUI_Option::MODE_DROPDOWN );
	m_pSettingControlsRightClickWireHack->AddOption( 0, "#RD_AdvancedSettings_Misc_WireHackClick_BothSame", "#rd_option_controls_right_click_wire_hack_hint" );
	m_pSettingControlsRightClickWireHack->AddOption( 1, "#RD_AdvancedSettings_Misc_WireHackClick_RightRev", "#rd_option_controls_right_click_wire_hack_hint" );
	m_pSettingControlsRightClickWireHack->AddOption( 2, "#RD_AdvancedSettings_Misc_WireHackClick_LeftRev", "#rd_option_controls_right_click_wire_hack_hint" );
	m_pSettingControlsRightClickWireHack->LinkToConVar( "rd_wire_tile_alternate", true );
	m_pSettingControlsSniperSwapWeapons = new CRD_VGUI_Option( this, "SettingControlsSniperSwapWeapons", "#rd_option_controls_sniper_swap_weapons", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingControlsSniperSwapWeapons->SetDefaultHint( "#rd_option_controls_sniper_swap_weapons" );
	m_pSettingControlsSniperSwapWeapons->LinkToConVar( "rd_sniper_scope_weapon_switch", false );
	m_pSettingControlsLockMouseToWindow = new CRD_VGUI_Option( this, "SettingControlsLockMouseToWindow", "#rd_option_controls_lock_mouse_to_window", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingControlsLockMouseToWindow->SetDefaultHint( "#rd_option_controls_lock_mouse_to_window_hint" );
	m_pSettingControlsLockMouseToWindow->LinkToConVar( "in_lock_mouse_to_window", true );
	m_pSettingControlsHorizontalAutoAim = new CRD_VGUI_Option( this, "SettingControlsHorizontalAutoAim", "#rd_option_controls_horizontal_auto_aim", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingControlsHorizontalAutoAim->SetDefaultHint( "#rd_option_controls_horizontal_auto_aim_hint" );
	m_pSettingControlsHorizontalAutoAim->LinkToConVar( "asw_horizontal_autoaim", true );

	// Crosshair
	m_pSettingCrosshairMarineLabelDist = new CRD_VGUI_Option( this, "SettingCrosshairMarineLabelDist", "#rd_option_crosshair_marine_label_dist", CRD_VGUI_Option::MODE_SLIDER );
	m_pSettingCrosshairMarineLabelDist->SetDefaultHint( "#rd_option_crosshair_marine_label_dist_hint" );
	m_pSettingCrosshairMarineLabelDist->AddOption( -1, "#rd_option_crosshair_marine_label_dist_unlimited", "#rd_option_crosshair_marine_label_dist_unlimited_hint" );
	m_pSettingCrosshairMarineLabelDist->SetSliderMinMax( 0.0f, 500.0f );
	m_pSettingCrosshairMarineLabelDist->LinkToConVar( "asw_marine_labels_cursor_maxdist", true );
	m_pSettingCrosshairType = new CRD_VGUI_Option( this, "SettingCrosshairType", "#RD_AdvancedSettings_HUD_CrosshairStyle", CRD_VGUI_Option::MODE_DROPDOWN );
	m_pSettingCrosshairType->AddOption( 0, "#RD_AdvancedSettings_HUD_CrosshairStyle_Cross", "#rd_option_crosshair_type_cross_hint" );
	m_pSettingCrosshairType->AddOption( 1, "#RD_AdvancedSettings_HUD_CrosshairStyle_Crescent", "#rd_option_crosshair_type_crescent_hint" );
	m_pSettingCrosshairType->LinkToConVar( "asw_crosshair_use_perspective", true );
	m_pSettingCrosshairSize = new CRD_VGUI_Option( this, "SettingCrosshairSize", "#rd_option_crosshair_size", CRD_VGUI_Option::MODE_SLIDER );
	m_pSettingCrosshairSize->SetDefaultHint( "#rd_option_crosshair_size_hint" );
	m_pSettingCrosshairSize->SetSliderMinMax( 5.0f, 50.0f );
	m_pSettingCrosshairSize->LinkToConVar( "asw_crosshair_progress_size", true );
	m_pSettingCrosshairLaserSight = new CRD_VGUI_Option( this, "SettingCrosshairLaserSight", "#RD_AdvancedSettings_HUD_LaserSight", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingCrosshairLaserSight->SetDefaultHint( "#rd_option_crosshair_laser_sight_hint" );
	m_pSettingCrosshairLaserSight->LinkToConVar( "asw_laser_sight", true );

	// Reloading
	m_pSettingReloadAuto = new CRD_VGUI_Option( this, "SettingReloadAuto", "#rd_option_reload_auto", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingReloadAuto->SetDefaultHint( "#rd_option_reload_auto_hint" );
	m_pSettingReloadAuto->LinkToConVar( "asw_auto_reload", true );
	m_pSettingReloadFastUnderMarine = new CRD_VGUI_Option( this, "SettingReloadFastUnderMarine", "#RD_AdvancedSettings_Ammo_FastReloadUnderMarine", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingReloadFastUnderMarine->SetDefaultHint( "#rd_option_reload_fast_under_marine_hint" );
	m_pSettingReloadFastUnderMarine->LinkToConVar( "asw_fast_reload_under_marine", false );
	m_pSettingReloadFastWide = new CRD_VGUI_Option( this, "SettingReloadFastWide", "#RD_AdvancedSettings_Ammo_FastReloadUnderMarineWScale", CRD_VGUI_Option::MODE_SLIDER );
	m_pSettingReloadFastWide->SetDefaultHint( "#rd_option_reload_fast_wide_hint" );
	m_pSettingReloadFastWide->SetSliderMinMax( 0.1f, 12.0f );
	m_pSettingReloadFastWide->LinkToConVar( "rd_fast_reload_under_marine_scale", true );
	m_pSettingReloadFastTall = new CRD_VGUI_Option( this, "SettingReloadFastTall", "#RD_AdvancedSettings_Ammo_FastReloadUnderMarineHScale", CRD_VGUI_Option::MODE_SLIDER );
	m_pSettingReloadFastTall->SetDefaultHint( "#rd_option_reload_fast_tall_hint" );
	m_pSettingReloadFastTall->SetSliderMinMax( 0.1f, 12.0f );
	m_pSettingReloadFastTall->AddOption( 0, "#RD_AdvancedSettings_Ammo_FastReloadUnderMarineHScale_SameAsW", "#rd_option_reload_fast_tall_hint" );
	m_pSettingReloadFastTall->LinkToConVar( "rd_fast_reload_under_marine_height_scale", true );
}

void CRD_VGUI_Settings_Options_1::Activate()
{
	bool bEverShowedLesson = false;
	for ( int i = 0; i < MAX_SPLITSCREEN_PLAYERS; ++i )
	{
		ACTIVE_SPLITSCREEN_PLAYER_GUARD( i );
		bEverShowedLesson = GetGameInstructor().EverShowedAnyLesson();
		if ( bEverShowedLesson )
			break;
	}
	m_pBtnResetGameInstructor->SetEnabled( bEverShowedLesson );

	m_pSettingCrosshairSize->SetEnabled( asw_crosshair_use_perspective.GetBool() );
	m_pSettingReloadFastWide->SetEnabled( asw_fast_reload_under_marine.GetBool() );
	m_pSettingReloadFastTall->SetEnabled( asw_fast_reload_under_marine.GetBool() );
}

static void ResetGameInstructorCountsHelper()
{
	for ( int i = 0; i < MAX_SPLITSCREEN_PLAYERS; i++ )
	{
		ACTIVE_SPLITSCREEN_PLAYER_GUARD( i );
		GetGameInstructor().ResetDisplaysAndSuccesses();
		GetGameInstructor().WriteSaveData();
	}

	if ( CRD_VGUI_Settings *pSettings = assert_cast< CRD_VGUI_Settings * >( CBaseModPanel::GetSingleton().GetWindow( WT_SETTINGS ) ) )
	{
		pSettings->m_pPnlOptions1->Activate();
	}
}

void CRD_VGUI_Settings_Options_1::OnCommand( const char *command )
{
	if ( !V_stricmp( command, "ResetGameInstructorCounts" ) )
	{
		GenericConfirmation *pConfirm = assert_cast< GenericConfirmation * >( CBaseModPanel::GetSingleton().OpenWindow( WT_GENERICCONFIRMATION, assert_cast< CRD_VGUI_Settings * >( GetParent() ), false ) );
		Assert( pConfirm );
		if ( pConfirm )
		{
			CBaseModPanel::GetSingleton().PlayUISound( UISOUND_CLICK );
			GenericConfirmation::Data_t data;
			data.pWindowTitle = "#rd_reset_game_instructor_progress_title";
			data.pMessageText = "#rd_reset_game_instructor_progress_desc";
			data.bOkButtonEnabled = true;
			data.pfnOkCallback = &ResetGameInstructorCountsHelper;
			data.bCancelButtonEnabled = true;

			pConfirm->SetUsageData( data );
		}

		return;
	}

	BaseClass::OnCommand( command );
}

void CRD_VGUI_Settings_Options_1::OnCurrentOptionChanged( vgui::Panel *panel )
{
	Activate();
}
