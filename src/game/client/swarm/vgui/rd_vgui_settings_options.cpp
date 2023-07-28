#include "cbase.h"
#include "rd_vgui_settings.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

DECLARE_BUILD_FACTORY( CRD_VGUI_Settings_Options );

CRD_VGUI_Settings_Options::CRD_VGUI_Settings_Options( vgui::Panel *parent, const char *panelName ) :
	BaseClass( parent, panelName )
{
	/*
	TODO
	Health
	[asw_medic_under_marine]
	[asw_medic_under_marine_frequency]
	[asw_medic_under_marine_offscreen]
	[asw_medic_under_marine_recall_time]
	[rd_health_counter_under_marine]
	[rd_health_counter_under_marine_alignment]
	[rd_health_counter_under_marine_show_max_health]
	[asw_world_healthbar_class_icon]
	[rd_draw_marine_health_counter]

	Ammo
	[asw_magazine_under_marine]
	[asw_magazine_under_marine_frequency]
	[asw_magazine_under_marine_offscreen]
	[asw_magazine_under_marine_recall_time]
	[rd_ammo_under_marine]
	[rd_ammo_counter_under_marine]
	[rd_ammo_counter_under_marine_alignment]
	[rd_ammo_counter_under_marine_show_max_ammo]
	*/

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
	// TODO: [rd_fail_advice]
	// TODO: [gameinstructor_enable]
	// TODO: [rd_deathmatch_respawn_hints]
	// TODO: [rd_swarmopedia_grid]
	// TODO: [rd_swarmopedia_units_preference]

	// Death
	// TODO: [asw_marine_death_cam]
	// TODO: [asw_marine_death_cam_slowdown]
	// TODO: [rd_marine_explodes_into_gibs]

	// Advanced Controls
	// TODO: [rd_wire_tile_alternate]
	// TODO: [rd_sniper_scope_weapon_switch]
	// TODO: [in_lock_mouse_to_window]
	// TODO: [asw_horizontal_autoaim]

	// Crosshair
	// TODO: [asw_marine_labels_cursor_maxdist]
	// TODO: [asw_crosshair_progress_size]
	// TODO: [asw_crosshair_use_perspective]
	// TODO: [asw_laser_sight]

	// Reloading
	// TODO: [asw_auto_reload]
	// TODO: [asw_fast_reload_under_marine]
	// TODO: [rd_fast_reload_under_marine_scale]
	// TODO: [rd_fast_reload_under_marine_height_scale]

	// Damage Numbers
	// TODO: [asw_floating_number_type]
	// TODO: [asw_floating_number_combine]

	// Speed Running
	// TODO: [rd_draw_timer]
	// TODO: [rd_draw_timer_color]
	// TODO: [rda_print_chat_objective_completion_time]
	// TODO: [rda_print_console_objective_completion_time]
	// TODO: [cl_auto_restart_mission]

	// Leaderboards
	// TODO: [rd_leaderboard_enabled_client]
	// TODO: [rd_show_leaderboard_loading]
	// TODO: [rd_show_leaderboard_debrief]

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
}
