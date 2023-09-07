#include "cbase.h"
#include "rd_vgui_settings.h"
#include "gameui/swarm/vgenericconfirmation.h"
#include "gameui/swarm/vhybridbutton.h"
#include "c_gameinstructor.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace BaseModUI;

DECLARE_BUILD_FACTORY( CRD_VGUI_Settings_Options );

extern ConVar asw_crosshair_use_perspective;
extern ConVar asw_fast_reload_under_marine;
extern ConVar rd_draw_timer;

CRD_VGUI_Settings_Options::CRD_VGUI_Settings_Options( vgui::Panel *parent, const char *panelName ) :
	BaseClass( parent, panelName )
{
	// Players
	m_pSettingPlayerNameMode = new CRD_VGUI_Option( this, "SettingPlayerNameMode", "#rd_option_player_name_mode" );
	m_pSettingPlayerNameMode->AddOption( 0, "#rd_option_player_name_mode_none", "" );
	m_pSettingPlayerNameMode->AddOption( 1, "#rd_option_player_name_mode_player", "" );
	m_pSettingPlayerNameMode->AddOption( 2, "#rd_option_player_name_mode_marine", "" );
	m_pSettingPlayerNameMode->AddOption( 3, "#rd_option_player_name_mode_both", "" );
	m_pSettingPlayerNameMode->LinkToConVar( "asw_player_names", true );
	m_pSettingPlayerChatColor = new CRD_VGUI_Option( this, "SettingPlayerChatColor", "#rd_option_player_chat_color", CRD_VGUI_Option::MODE_COLOR );
	m_pSettingPlayerChatColor->LinkToConVar( "cl_chatcolor", true );
	m_pSettingPlayerChatNamesUseColors = new CRD_VGUI_Option( this, "SettingPlayerChatNameUsesColors", "#rd_option_player_chat_name_uses_colors", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingPlayerChatNamesUseColors->LinkToConVar( "rd_chat_colorful_player_names", true );
	m_pSettingPlayerDeathmatchTeamColorMode = new CRD_VGUI_Option( this, "SettingPlayerDeathmatchTeamColorMode", "#rd_option_player_deathmatch_team_color_mode" );
	m_pSettingPlayerDeathmatchTeamColorMode->AddOption( 1, "#rd_option_player_deathmatch_team_color_mode_ally_enemy", "" );
	m_pSettingPlayerDeathmatchTeamColorMode->AddOption( 2, "#rd_option_player_deathmatch_team_color_mode_counter_strike", "" );
	m_pSettingPlayerDeathmatchTeamColorMode->LinkToConVar( "rd_deathmatch_team_colors", false );
	m_pSettingPlayerDeathmatchDrawTopScoreboard = new CRD_VGUI_Option( this, "SettingPlayerDeathmatchDrawTopScoreboard", "#rd_option_player_deathmatch_draw_top_scoreboard", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingPlayerDeathmatchDrawTopScoreboard->LinkToConVar( "rd_draw_avatars_with_frags", true );

	// Hints
	m_pSettingHintsFailAdvice = new CRD_VGUI_Option( this, "SettingHintsFailAdvice", "#rd_option_hints_fail_advice", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingHintsFailAdvice->LinkToConVar( "rd_fail_advice", true );
	m_pSettingHintsGameInstructor = new CRD_VGUI_Option( this, "SettingHintsGameInstructor", "#rd_option_hints_game_instructor", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingHintsGameInstructor->LinkToConVar( "gameinstructor_enable", true );
	m_pBtnResetGameInstructor = new BaseModHybridButton( this, "BtnResetGameInstructor", "#rd_reset_game_instructor_progress", this, "ResetGameInstructorCounts" );
	m_pSettingHintsDeathmatchRespawn = new CRD_VGUI_Option( this, "SettingHintsDeathmatchRespawn", "#rd_option_hints_deathmatch_respawn", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingHintsDeathmatchRespawn->LinkToConVar( "rd_deathmatch_respawn_hints", true );
	m_pSettingHintsSwarmopediaGrid = new CRD_VGUI_Option( this, "SettingHintsSwarmopediaGrid", "#rd_option_hints_swarmopedia_grid", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingHintsSwarmopediaGrid->LinkToConVar( "rd_swarmopedia_grid", true );
	m_pSettingHintsSwarmopediaUnits = new CRD_VGUI_Option( this, "SettingHintsSwarmopediaUnits", "#RD_AdvancedSettings_Misc_SwarmopediaUnits", CRD_VGUI_Option::MODE_DROPDOWN );
	m_pSettingHintsSwarmopediaUnits->AddOption( 0, "#RD_AdvancedSettings_Misc_SwarmopediaUnits_Hammer", "" );
	m_pSettingHintsSwarmopediaUnits->AddOption( 1, "#RD_AdvancedSettings_Misc_SwarmopediaUnits_Metric", "" );
	m_pSettingHintsSwarmopediaUnits->AddOption( 2, "#RD_AdvancedSettings_Misc_SwarmopediaUnits_Imperial", "" );
	m_pSettingHintsSwarmopediaUnits->LinkToConVar( "rd_swarmopedia_units_preference", false );

	// Death
	m_pSettingDeathCamTakeover = new CRD_VGUI_Option( this, "SettingDeathCamTakeover", "#rd_option_death_cam_takeover", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingDeathCamTakeover->LinkToConVar( "asw_marine_death_cam", true );
	m_pSettingDeathCamSlowdown = new CRD_VGUI_Option( this, "SettingDeathCamSlowdown", "#rd_option_death_cam_slowdown", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingDeathCamSlowdown->LinkToConVar( "asw_marine_death_cam_slowdown", true );
	m_pSettingDeathMarineGibs = new CRD_VGUI_Option( this, "SetitngDeathMarineGibs", "#rd_option_death_marine_gibs", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingDeathMarineGibs->LinkToConVar( "rd_marine_explodes_into_gibs", true );

	// Advanced Controls
	m_pSettingControlsRightClickWireHack = new CRD_VGUI_Option( this, "SettingControlsRightClickWireHack", "#RD_AdvancedSettings_Misc_WireHackClick", CRD_VGUI_Option::MODE_DROPDOWN );
	m_pSettingControlsRightClickWireHack->AddOption( 0, "#RD_AdvancedSettings_Misc_WireHackClick_BothSame", "" );
	m_pSettingControlsRightClickWireHack->AddOption( 1, "#RD_AdvancedSettings_Misc_WireHackClick_RightRev", "" );
	m_pSettingControlsRightClickWireHack->AddOption( 2, "#RD_AdvancedSettings_Misc_WireHackClick_LeftRev", "" );
	m_pSettingControlsRightClickWireHack->LinkToConVar( "rd_wire_tile_alternate", true );
	m_pSettingControlsSniperSwapWeapons = new CRD_VGUI_Option( this, "SettingControlsSniperSwapWeapons", "#rd_option_controls_sniper_swap_weapons", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingControlsSniperSwapWeapons->LinkToConVar( "rd_sniper_scope_weapon_switch", false );
	m_pSettingControlsLockMouseToWindow = new CRD_VGUI_Option( this, "SettingControlsLockMouseToWindow", "#rd_option_controls_lock_mouse_to_window", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingControlsLockMouseToWindow->LinkToConVar( "in_lock_mouse_to_window", true );
	m_pSettingControlsHorizontalAutoAim = new CRD_VGUI_Option( this, "SettingControlsHorizontalAutoAim", "#rd_optioncontrols_horizontal_auto_aim", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingControlsHorizontalAutoAim->LinkToConVar( "asw_horizontal_autoaim", true );

	// Crosshair
	m_pSettingCrosshairMarineLabelDist = new CRD_VGUI_Option( this, "SettingCrosshairMarineLabelDist", "#rd_setting_crosshair_marine_label_dist", CRD_VGUI_Option::MODE_SLIDER );
	m_pSettingCrosshairMarineLabelDist->AddOption( -1, "#rd_setting_crosshair_marine_label_dist_unlimited", "" );
	m_pSettingCrosshairMarineLabelDist->SetSliderMinMax( 0.0f, 500.0f );
	m_pSettingCrosshairMarineLabelDist->LinkToConVar( "asw_marine_labels_cursor_maxdist", true );
	m_pSettingCrosshairType = new CRD_VGUI_Option( this, "SettingCrosshairType", "#RD_AdvancedSettings_HUD_CrosshairStyle", CRD_VGUI_Option::MODE_DROPDOWN );
	m_pSettingCrosshairType->AddOption( 0, "#RD_AdvancedSettings_HUD_CrosshairStyle_Cross", "" );
	m_pSettingCrosshairType->AddOption( 1, "#RD_AdvancedSettings_HUD_CrosshairStyle_Crescent", "" );
	m_pSettingCrosshairType->LinkToConVar( "asw_crosshair_use_perspective", true );
	m_pSettingCrosshairSize = new CRD_VGUI_Option( this, "SettingCrosshairSize", "#rd_setting_crosshair_size", CRD_VGUI_Option::MODE_SLIDER );
	m_pSettingCrosshairSize->SetSliderMinMax( 5.0f, 50.0f );
	m_pSettingCrosshairSize->LinkToConVar( "asw_crosshair_progress_size", true );
	m_pSettingCrosshairLaserSight = new CRD_VGUI_Option( this, "SettingCrosshairLaserSight", "#RD_AdvancedSettings_HUD_LaserSight", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingCrosshairLaserSight->LinkToConVar( "asw_laser_sight", true );

	// Reloading
	m_pSettingReloadAuto = new CRD_VGUI_Option( this, "SettingReloadAuto", "#rd_option_reload_auto", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingReloadAuto->LinkToConVar( "asw_auto_reload", true );
	m_pSettingReloadFastUnderMarine = new CRD_VGUI_Option( this, "SettingReloadFastUnderMarine", "#RD_AdvancedSettings_Ammo_FastReloadUnderMarine", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingReloadFastUnderMarine->LinkToConVar( "asw_fast_reload_under_marine", false );
	m_pSettingReloadFastWide = new CRD_VGUI_Option( this, "SettingReloadFastWide", "#RD_AdvancedSettings_Ammo_FastReloadUnderMarineWScale", CRD_VGUI_Option::MODE_SLIDER );
	m_pSettingReloadFastWide->SetSliderMinMax( 0.1f, 15.0f );
	m_pSettingReloadFastWide->LinkToConVar( "rd_fast_reload_under_marine_scale", true );
	m_pSettingReloadFastTall = new CRD_VGUI_Option( this, "SettingReloadFastTall", "#RD_AdvancedSettings_Ammo_FastReloadUnderMarineHScale", CRD_VGUI_Option::MODE_SLIDER );
	m_pSettingReloadFastTall->SetSliderMinMax( 0.1f, 15.0f );
	m_pSettingReloadFastTall->AddOption( -1, "#RD_AdvancedSettings_Ammo_FastReloadUnderMarineHScale_SameAsW", "" );
	m_pSettingReloadFastTall->LinkToConVar( "rd_fast_reload_under_marine_height_scale", true );

	// Floating Text
	m_pSettingDamageNumbers = new CRD_VGUI_Option( this, "SettingDamageNumbers", "#rd_option_damage_numbers", CRD_VGUI_Option::MODE_DROPDOWN );
	m_pSettingDamageNumbers->AddOption( 0, "#rd_option_damge_numbers_disabled", "" );
	m_pSettingDamageNumbers->LinkToConVarAdvanced( 0, "asw_floating_number_type", 0 );
	m_pSettingDamageNumbers->AddOption( 1, "#rd_option_damge_numbers_separate", "" );
	m_pSettingDamageNumbers->LinkToConVarAdvanced( 1, "asw_floating_number_type", 2 );
	m_pSettingDamageNumbers->LinkToConVarAdvanced( 1, "asw_floating_number_combine", 0 );
	m_pSettingDamageNumbers->AddOption( 2, "#rd_option_damge_numbers_combined", "" );
	m_pSettingDamageNumbers->LinkToConVarAdvanced( 2, "asw_floating_number_type", 2 );
	m_pSettingDamageNumbers->LinkToConVarAdvanced( 2, "asw_floating_number_combine", 1 );
	m_pSettingDamageNumbers->SetCurrentUsingConVars();
	m_pSettingStrangeRankUp = new CRD_VGUI_Option( this, "SettingStrangeRankUp", "#rd_option_strange_rank_up", CRD_VGUI_Option::MODE_DROPDOWN );
	m_pSettingStrangeRankUp->AddOption( 0, "#rd_option_strange_rank_up_disabled", "" );
	m_pSettingStrangeRankUp->AddOption( 1, "#rd_option_strange_rank_up_enabled", "" );
	m_pSettingStrangeRankUp->AddOption( 2, "#rd_option_strange_rank_up_only_self", "" );
	m_pSettingStrangeRankUp->LinkToConVar( "rd_strange_device_tier_notifications", false );

	// Speed Running
	m_pSettingSpeedTimer = new CRD_VGUI_Option( this, "SettingSpeedTimer", "#rd_option_speed_timer", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingSpeedTimer->LinkToConVar( "rd_draw_timer", false );
	m_pSettingSpeedTimerColor = new CRD_VGUI_Option( this, "SettingSpeedTimerColor", "#rd_option_speed_timer_color", CRD_VGUI_Option::MODE_COLOR );
	m_pSettingSpeedTimerColor->LinkToConVar( "rd_draw_timer_color", false );
	m_pSettingSpeedObjectivesInChat = new CRD_VGUI_Option( this, "SettingSpeedObjectivesInChat", "#rd_option_speed_objectives_in_chat", CRD_VGUI_Option::MODE_DROPDOWN );
	m_pSettingSpeedObjectivesInChat->AddOption( 0, "#rd_option_speed_objectives_disabled", "" );
	m_pSettingSpeedObjectivesInChat->LinkToConVarAdvanced( 0, "rda_print_chat_objective_completion_time", 0 );
	m_pSettingSpeedObjectivesInChat->LinkToConVarAdvanced( 0, "rda_print_console_objective_completion_time", 0 );
	m_pSettingSpeedObjectivesInChat->AddOption( 1, "#rd_option_speed_objectives_in_console", "" );
	m_pSettingSpeedObjectivesInChat->LinkToConVarAdvanced( 1, "rda_print_chat_objective_completion_time", 0 );
	m_pSettingSpeedObjectivesInChat->LinkToConVarAdvanced( 1, "rda_print_console_objective_completion_time", 1 );
	m_pSettingSpeedObjectivesInChat->AddOption( 2, "#rd_option_speed_objectives_in_chat", "" );
	m_pSettingSpeedObjectivesInChat->LinkToConVarAdvanced( 2, "rda_print_chat_objective_completion_time", 1 );
	m_pSettingSpeedAutoRestartMission = new CRD_VGUI_Option( this, "SettingSpeedAutoRestartMission", "#rd_option_speed_auto_restart_mission", CRD_VGUI_Option::MODE_CHECKBOX);
	m_pSettingSpeedAutoRestartMission->LinkToConVar( "cl_auto_restart_mission", false );

	// Leaderboards
	m_pSettingLeaderboardSend = new CRD_VGUI_Option( this, "SettingLeaderboardSend", "#rd_option_leaderboard_send", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingLeaderboardSend->LinkToConVar( "rd_leaderboard_enabled_client", true );
	m_pSettingLeaderboardLoading = new CRD_VGUI_Option( this, "SettingLeaderboardLoading", "#rd_option_leaderboard_loading", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingLeaderboardLoading->LinkToConVar( "rd_show_leaderboard_loading", false );
	m_pSettingLeaderboardDebrief = new CRD_VGUI_Option( this, "SettingLeaderboardDebrief", "#rd_option_leaderboard_debrief", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingLeaderboardDebrief->LinkToConVar( "rd_show_leaderboard_debrief", false );

	// Loading
	m_pSettingLoadingMissionIcons = new CRD_VGUI_Option( this, "SettingLoadingMissionIcons", "#rd_option_loading_mission_icons", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingLoadingMissionIcons->LinkToConVar( "rd_show_mission_icon_loading", true );
	m_pSettingLoadingMissionScreens = new CRD_VGUI_Option( this, "SettingLoadingMissionScreens", "#rd_option_loading_mission_screens", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingLoadingMissionScreens->LinkToConVar( "rd_loading_image_per_map", true );
	m_pSettingLoadingStatusText = new CRD_VGUI_Option( this, "SettingLoadingStatusText", "#rd_option_loading_status_text", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingLoadingStatusText->LinkToConVar( "rd_loading_status_text_visible", true );

	// Accessibility
	m_pSettingAccessibilityTracerTintSelf = new CRD_VGUI_Option( this, "SettingAccessibilityTracerTintSelf", "#rd_option_accessibility_tracer_tint_self", CRD_VGUI_Option::MODE_COLOR );
	m_pSettingAccessibilityTracerTintSelf->LinkToConVar( "rd_tracer_tint_self", true );
	m_pSettingAccessibilityTracerTintOther = new CRD_VGUI_Option( this, "SettingAccessibilityTracerTintOther", "#rd_option_accessibility_tracer_tint_other", CRD_VGUI_Option::MODE_COLOR );
	m_pSettingAccessibilityTracerTintOther->LinkToConVar( "rd_tracer_tint_other", true );
	m_pSettingAccessibilityHighlightActiveCharacter = new CRD_VGUI_Option( this, "SettingAccessibilityHighlightActiveCharacter", "#RD_AdvancedSettings_Misc_HighlightActiveCharacter", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingAccessibilityHighlightActiveCharacter->LinkToConVar( "rd_highlight_active_character", false );
	m_pSettingAccessibilityReduceMotion = new CRD_VGUI_Option( this, "SettingAccessibilityReduceMotion", "#RD_AdvancedSettings_VisualEffects_ReduceMotion", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingAccessibilityReduceMotion->LinkToConVar( "rd_reduce_motion", false );
	m_pSettingAccessibilityCameraShake = new CRD_VGUI_Option( this, "SettingAccessibilityCameraShake", "#RD_AdvancedSettings_VisualEffects_ScreenShake" );
	m_pSettingAccessibilityCameraShake->AddOption( 0, "#rd_video_effect_disabled", "#rd_option_accessibility_camera_shake_disabled_hint" );
	m_pSettingAccessibilityCameraShake->AddOption( 1, "#RD_AdvancedSettings_VisualEffects_ScreenShake_Reduced", "#rd_option_accessibility_camera_shake_reduced_hint" );
	m_pSettingAccessibilityCameraShake->AddOption( 2, "#rd_video_effect_enabled", "#rd_option_accessibility_camera_shake_enabled_hint" );
	m_pSettingAccessibilityCameraShake->LinkToConVar( "rd_camera_shake", false );
	m_pSettingAccessibilityCameraShift = new CRD_VGUI_Option( this, "SettingAccessibilityCameraShift", "#rd_option_accessibility_camera_shift", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingAccessibilityCameraShift->LinkToConVar( "asw_cam_marine_shift_enable", false );
	m_pSettingAccessibilityMinimapClicks = new CRD_VGUI_Option( this, "SettingAccessibilityMinimapClicks", "#rd_option_accessibility_minimap_clicks", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingAccessibilityMinimapClicks->LinkToConVar( "asw_minimap_clicks", false );
	m_pSettingAccessibilityMoveRelativeToAim = new CRD_VGUI_Option( this, "SettingAccessibilityMoveRelativeToAim", "#rd_option_accessibility_move_relative_to_aim", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingAccessibilityMoveRelativeToAim->LinkToConVar( "rd_movement_relative_to_aim", true );

	// Network
	m_pSettingNetworkInterpolation = new CRD_VGUI_Option( this, "SettingNetworkInterpolation", "#rd_option_network_interpolation", CRD_VGUI_Option::MODE_SLIDER );
	m_pSettingNetworkInterpolation->SetSliderMinMax( 0.0f, 0.5f );
	m_pSettingNetworkInterpolation->LinkToConVar( "cl_interp", true );
	m_pSettingNetworkAllowRelay = new CRD_VGUI_Option( this, "SettingNetworkAllowRelay", "#rd_option_network_allow_relay" );
	m_pSettingNetworkAllowRelay->AddOption( 0, "#rd_option_network_allow_relay_disabled", "" );
	m_pSettingNetworkAllowRelay->AddOption( 1, "#rd_option_network_allow_relay_enabled", "" );
	m_pSettingNetworkAllowRelay->LinkToConVarAdvanced( 0, "net_steamcnx_allowrelay", 0 );
	m_pSettingNetworkAllowRelay->LinkToConVarAdvanced( 0, "net_steamcnx_enabled", 1 );
	m_pSettingNetworkAllowRelay->LinkToConVarAdvanced( 1, "net_steamcnx_allowrelay", 1 );
	m_pSettingNetworkAllowRelay->LinkToConVarAdvanced( 1, "net_steamcnx_enabled", 2 );
	m_pSettingNetworkAllowRelay->SetCurrentUsingConVars();
	m_pSettingNetworkAllowRelay->SetRecommendedUsingConVars();
}

void CRD_VGUI_Settings_Options::Activate()
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
	m_pSettingSpeedTimerColor->SetEnabled( rd_draw_timer.GetBool() );
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
		pSettings->m_pPnlOptions->Activate();
	}
}

void CRD_VGUI_Settings_Options::OnCommand( const char *command )
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

void CRD_VGUI_Settings_Options::OnCurrentOptionChanged( vgui::Panel *panel )
{
	Activate();
}
