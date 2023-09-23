#include "cbase.h"
#include "rd_vgui_settings.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace BaseModUI;

DECLARE_BUILD_FACTORY( CRD_VGUI_Settings_Options_2 );

extern ConVar rd_draw_timer;

CRD_VGUI_Settings_Options_2::CRD_VGUI_Settings_Options_2( vgui::Panel *parent, const char *panelName ) :
	BaseClass( parent, panelName )
{
	// Floating Text
	m_pSettingDamageNumbers = new CRD_VGUI_Option( this, "SettingDamageNumbers", "#rd_option_damage_numbers", CRD_VGUI_Option::MODE_DROPDOWN );
	m_pSettingDamageNumbers->AddOption( 0, "#rd_option_damage_numbers_disabled", "#rd_option_damage_numbers_hint" );
	m_pSettingDamageNumbers->LinkToConVarAdvanced( 0, "asw_floating_number_type", 0 );
	m_pSettingDamageNumbers->AddOption( 1, "#rd_option_damage_numbers_separate", "#rd_option_damage_numbers_hint" );
	m_pSettingDamageNumbers->LinkToConVarAdvanced( 1, "asw_floating_number_type", 2 );
	m_pSettingDamageNumbers->LinkToConVarAdvanced( 1, "asw_floating_number_combine", 0 );
	m_pSettingDamageNumbers->AddOption( 2, "#rd_option_damage_numbers_combined", "#rd_option_damage_numbers_hint" );
	m_pSettingDamageNumbers->LinkToConVarAdvanced( 2, "asw_floating_number_type", 2 );
	m_pSettingDamageNumbers->LinkToConVarAdvanced( 2, "asw_floating_number_combine", 1 );
	m_pSettingDamageNumbers->SetCurrentUsingConVars();
	m_pSettingStrangeRankUp = new CRD_VGUI_Option( this, "SettingStrangeRankUp", "#rd_option_strange_rank_up", CRD_VGUI_Option::MODE_DROPDOWN );
	m_pSettingStrangeRankUp->AddOption( 0, "#rd_option_strange_rank_up_disabled", "#rd_option_strange_rank_up_hint" );
	m_pSettingStrangeRankUp->AddOption( 1, "#rd_option_strange_rank_up_enabled", "#rd_option_strange_rank_up_hint" );
	m_pSettingStrangeRankUp->AddOption( 2, "#rd_option_strange_rank_up_only_self", "#rd_option_strange_rank_up_hint" );
	m_pSettingStrangeRankUp->LinkToConVar( "rd_strange_device_tier_notifications", false );

	// Speed Running
	m_pSettingSpeedTimer = new CRD_VGUI_Option( this, "SettingSpeedTimer", "#rd_option_speed_timer", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingSpeedTimer->SetDefaultHint( "#rd_option_speed_timer_hint" );
	m_pSettingSpeedTimer->LinkToConVar( "rd_draw_timer", false );
	m_pSettingSpeedTimerColor = new CRD_VGUI_Option( this, "SettingSpeedTimerColor", "#rd_option_speed_timer_color", CRD_VGUI_Option::MODE_COLOR );
	m_pSettingSpeedTimerColor->SetDefaultHint( "#rd_option_speed_timer_color_hint" );
	m_pSettingSpeedTimerColor->LinkToConVar( "rd_draw_timer_color", false );
	m_pSettingSpeedObjectivesInChat = new CRD_VGUI_Option( this, "SettingSpeedObjectivesInChat", "#rd_option_speed_objectives", CRD_VGUI_Option::MODE_DROPDOWN );
	m_pSettingSpeedObjectivesInChat->AddOption( 0, "#rd_option_speed_objectives_disabled", "#rd_option_speed_objectives_hint" );
	m_pSettingSpeedObjectivesInChat->LinkToConVarAdvanced( 0, "rda_print_chat_objective_completion_time", 0 );
	m_pSettingSpeedObjectivesInChat->LinkToConVarAdvanced( 0, "rda_print_console_objective_completion_time", 0 );
	m_pSettingSpeedObjectivesInChat->AddOption( 1, "#rd_option_speed_objectives_in_console", "#rd_option_speed_objectives_hint" );
	m_pSettingSpeedObjectivesInChat->LinkToConVarAdvanced( 1, "rda_print_chat_objective_completion_time", 0 );
	m_pSettingSpeedObjectivesInChat->LinkToConVarAdvanced( 1, "rda_print_console_objective_completion_time", 1 );
	m_pSettingSpeedObjectivesInChat->AddOption( 2, "#rd_option_speed_objectives_in_chat", "#rd_option_speed_objectives_hint" );
	m_pSettingSpeedObjectivesInChat->LinkToConVarAdvanced( 2, "rda_print_chat_objective_completion_time", 1 );
	m_pSettingSpeedObjectivesInChat->SetCurrentUsingConVars();
	m_pSettingSpeedAutoRestartMission = new CRD_VGUI_Option( this, "SettingSpeedAutoRestartMission", "#rd_option_speed_auto_restart_mission", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingSpeedAutoRestartMission->SetDefaultHint( "#rd_option_speed_auto_restart_mission_hint" );
	m_pSettingSpeedAutoRestartMission->LinkToConVar( "cl_auto_restart_mission", false );

	// Leaderboards
	m_pSettingLeaderboardPrivateStats = new CRD_VGUI_Option( this, "SettingLeaderboardPrivateStats", "#rd_option_leaderboard_private_stats", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingLeaderboardPrivateStats->SetDefaultHint( "#rd_option_leaderboard_private_stats" );
	m_pSettingLeaderboardPrivateStats->SetEnabled( false );
	m_pSettingLeaderboardSend = new CRD_VGUI_Option( this, "SettingLeaderboardSend", "#rd_option_leaderboard_send", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingLeaderboardSend->SetDefaultHint( "#rd_option_leaderboard_send_hint" );
	m_pSettingLeaderboardSend->LinkToConVar( "rd_leaderboard_enabled_client", true );
	m_pSettingLeaderboardLoading = new CRD_VGUI_Option( this, "SettingLeaderboardLoading", "#rd_option_leaderboard_loading", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingLeaderboardLoading->SetDefaultHint( "#rd_option_leaderboard_loading_hint" );
	m_pSettingLeaderboardLoading->LinkToConVar( "rd_show_leaderboard_loading", false );
	m_pSettingLeaderboardDebrief = new CRD_VGUI_Option( this, "SettingLeaderboardDebrief", "#rd_option_leaderboard_debrief", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingLeaderboardDebrief->SetDefaultHint( "#rd_option_leaderboard_debrief_hint" );
	m_pSettingLeaderboardDebrief->LinkToConVar( "rd_show_leaderboard_debrief", false );

	// Loading
	m_pSettingLoadingMissionIcons = new CRD_VGUI_Option( this, "SettingLoadingMissionIcons", "#rd_option_loading_mission_icons", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingLoadingMissionIcons->SetDefaultHint( "#rd_option_loading_mission_icons_hint" );
	m_pSettingLoadingMissionIcons->LinkToConVar( "rd_show_mission_icon_loading", true );
	m_pSettingLoadingMissionScreens = new CRD_VGUI_Option( this, "SettingLoadingMissionScreens", "#rd_option_loading_mission_screens", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingLoadingMissionScreens->SetDefaultHint( "#rd_option_loading_mission_screens_hint" );
	m_pSettingLoadingMissionScreens->LinkToConVar( "rd_loading_image_per_map", true );
	m_pSettingLoadingStatusText = new CRD_VGUI_Option( this, "SettingLoadingStatusText", "#rd_option_loading_status_text", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingLoadingStatusText->SetDefaultHint( "#rd_option_loading_status_text_hint" );
	m_pSettingLoadingStatusText->LinkToConVar( "rd_loading_status_text_visible", true );

	// Accessibility
	m_pSettingAccessibilityTracerTintSelf = new CRD_VGUI_Option( this, "SettingAccessibilityTracerTintSelf", "#rd_option_accessibility_tracer_tint_self", CRD_VGUI_Option::MODE_COLOR );
	m_pSettingAccessibilityTracerTintSelf->SetDefaultHint( "#rd_option_accessibility_tracer_tint_self_hint" );
	m_pSettingAccessibilityTracerTintSelf->LinkToConVar( "rd_tracer_tint_self", true );
	m_pSettingAccessibilityTracerTintOther = new CRD_VGUI_Option( this, "SettingAccessibilityTracerTintOther", "#rd_option_accessibility_tracer_tint_other", CRD_VGUI_Option::MODE_COLOR );
	m_pSettingAccessibilityTracerTintOther->SetDefaultHint( "#rd_option_accessibility_tracer_tint_other_hint" );
	m_pSettingAccessibilityTracerTintOther->LinkToConVar( "rd_tracer_tint_other", true );
	m_pSettingAccessibilityHighlightActiveCharacter = new CRD_VGUI_Option( this, "SettingAccessibilityHighlightActiveCharacter", "#RD_AdvancedSettings_Misc_HighlightActiveCharacter", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingAccessibilityHighlightActiveCharacter->SetDefaultHint( "#rd_option_accessibility_highlight_active_character_hint" );
	m_pSettingAccessibilityHighlightActiveCharacter->LinkToConVar( "rd_highlight_active_character", false );
	m_pSettingAccessibilityReduceMotion = new CRD_VGUI_Option( this, "SettingAccessibilityReduceMotion", "#RD_AdvancedSettings_VisualEffects_ReduceMotion", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingAccessibilityReduceMotion->SetDefaultHint( "#rd_option_accessibility_reduce_motion_hint" );
	m_pSettingAccessibilityReduceMotion->LinkToConVar( "rd_reduce_motion", false );
	m_pSettingAccessibilityCameraShake = new CRD_VGUI_Option( this, "SettingAccessibilityCameraShake", "#RD_AdvancedSettings_VisualEffects_ScreenShake", CRD_VGUI_Option::MODE_DROPDOWN );
	m_pSettingAccessibilityCameraShake->AddOption( 0, "#rd_video_effect_disabled", "#rd_option_accessibility_camera_shake_disabled_hint" );
	m_pSettingAccessibilityCameraShake->AddOption( 1, "#RD_AdvancedSettings_VisualEffects_ScreenShake_Reduced", "#rd_option_accessibility_camera_shake_reduced_hint" );
	m_pSettingAccessibilityCameraShake->AddOption( 2, "#rd_video_effect_enabled", "#rd_option_accessibility_camera_shake_enabled_hint" );
	m_pSettingAccessibilityCameraShake->LinkToConVar( "rd_camera_shake", false );
	m_pSettingAccessibilityCameraShift = new CRD_VGUI_Option( this, "SettingAccessibilityCameraShift", "#rd_option_accessibility_camera_shift", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingAccessibilityCameraShift->SetDefaultHint( "#rd_option_accessibility_camera_shift_hint" );
	m_pSettingAccessibilityCameraShift->LinkToConVar( "asw_cam_marine_shift_enable", false );
	m_pSettingAccessibilityMinimapClicks = new CRD_VGUI_Option( this, "SettingAccessibilityMinimapClicks", "#rd_option_accessibility_minimap_clicks", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingAccessibilityMinimapClicks->SetDefaultHint( "#rd_option_accessibility_minimap_clicks_hint" );
	m_pSettingAccessibilityMinimapClicks->LinkToConVar( "asw_minimap_clicks", false );
	m_pSettingAccessibilityMoveRelativeToAim = new CRD_VGUI_Option( this, "SettingAccessibilityMoveRelativeToAim", "#rd_option_accessibility_move_relative_to_aim", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingAccessibilityMoveRelativeToAim->SetDefaultHint( "#rd_option_accessibility_move_relative_to_aim_hint" );
	m_pSettingAccessibilityMoveRelativeToAim->LinkToConVar( "rd_movement_relative_to_aim", true );

	// Network
	m_pSettingNetworkInterpolation = new CRD_VGUI_Option( this, "SettingNetworkInterpolation", "#rd_option_network_interpolation", CRD_VGUI_Option::MODE_SLIDER );
	m_pSettingNetworkInterpolation->SetDefaultHint( "#rd_option_network_interpolation_hint" );
	m_pSettingNetworkInterpolation->SetSliderMinMax( 0.0f, 0.5f );
	m_pSettingNetworkInterpolation->LinkToConVar( "cl_interp", true );
	m_pSettingNetworkRate = new CRD_VGUI_Option( this, "SettingNetworkRate", "#rd_option_network_rate", CRD_VGUI_Option::MODE_SLIDER );
	m_pSettingNetworkRate->SetDefaultHint( "#rd_option_network_rate_hint" );
	m_pSettingNetworkRate->SetSliderMinMax( 24576, 786432 );
	m_pSettingNetworkRate->LinkToConVar( "rate", true );
	m_pSettingNetworkAllowRelay = new CRD_VGUI_Option( this, "SettingNetworkAllowRelay", "#rd_option_network_allow_relay", CRD_VGUI_Option::MODE_DROPDOWN );
	m_pSettingNetworkAllowRelay->AddOption( 0, "#rd_option_network_allow_relay_disabled", "#rd_option_network_allow_relay_disabled_hint" );
	m_pSettingNetworkAllowRelay->LinkToConVarAdvanced( 0, "net_steamcnx_allowrelay", 0 );
	m_pSettingNetworkAllowRelay->LinkToConVarAdvanced( 0, "net_steamcnx_enabled", 1 );
	m_pSettingNetworkAllowRelay->AddOption( 1, "#rd_option_network_allow_relay_enabled", "#rd_option_network_allow_relay_enabled_hint" );
	m_pSettingNetworkAllowRelay->LinkToConVarAdvanced( 1, "net_steamcnx_allowrelay", 1 );
	m_pSettingNetworkAllowRelay->LinkToConVarAdvanced( 1, "net_steamcnx_enabled", 2 );
	m_pSettingNetworkAllowRelay->SetCurrentUsingConVars();
}

void CRD_VGUI_Settings_Options_2::Activate()
{
	NavigateToChild( m_pSettingDamageNumbers );

	m_pSettingSpeedTimerColor->SetEnabled( rd_draw_timer.GetBool() );

	ISteamUserStats *pUserStats = SteamUserStats();
	Assert( pUserStats );
	int32_t iStatsOptOut = 0;
	if ( !m_pSettingLeaderboardPrivateStats->IsEnabled() && pUserStats && pUserStats->GetStat( "stats_display_opt_out", &iStatsOptOut ) )
	{
		m_pSettingLeaderboardPrivateStats->SetCurrentSliderValue( iStatsOptOut );
		m_pSettingLeaderboardPrivateStats->SetEnabled( true );
	}

	// not ready yet
	m_pSettingStrangeRankUp->SetVisible( false );
}

void CRD_VGUI_Settings_Options_2::OnCurrentOptionChanged( vgui::Panel *panel )
{
	if ( panel == m_pSettingLeaderboardPrivateStats )
	{
		ISteamUserStats *pUserStats = SteamUserStats();
		Assert( pUserStats );
		if ( pUserStats )
		{
			int32_t iStatsOptOutNew = int32_t( m_pSettingLeaderboardPrivateStats->GetCurrentSliderValue() );
			int32_t iStatsOptOutOld = 0;
			if ( pUserStats->GetStat( "stats_display_opt_out", &iStatsOptOutOld ) && iStatsOptOutOld != iStatsOptOutNew )
			{
				Msg( "Setting stats_display_opt_out to %d\n", iStatsOptOutNew );
				pUserStats->SetStat( "stats_display_opt_out", iStatsOptOutNew );
				pUserStats->StoreStats();
				m_pSettingLeaderboardPrivateStats->SetEnabled( false );
			}
		}
	}

	Activate();
}
