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

	/*
	Players
	[asw_player_names]
	[cl_chatcolor]
	[rd_chat_colorful_player_names]
	[rd_deathmatch_team_colors]
	[rd_draw_avatars_with_frags]
	*/

	/*
	Hints
	[rd_fail_advice]
	[gameinstructor_enable]
	[rd_deathmatch_respawn_hints]
	[rd_swarmopedia_grid]
	[rd_swarmopedia_units_preference]
	*/

	/*
	Death
	[asw_marine_death_cam]
	[asw_marine_death_cam_slowdown]
	[rd_marine_explodes_into_gibs]
	*/

	/*
	Advanced Controls
	[rd_wire_tile_alternate]
	[rd_sniper_scope_weapon_switch]
	[in_lock_mouse_to_window]
	[asw_horizontal_autoaim]
	*/

	/*
	Crosshair
	[asw_marine_labels_cursor_maxdist]
	[asw_crosshair_progress_size]
	[asw_crosshair_use_perspective]
	[asw_laser_sight]
	*/

	/*
	Reloading
	[asw_auto_reload]
	[asw_fast_reload_under_marine]
	[rd_fast_reload_under_marine_scale]
	[rd_fast_reload_under_marine_height_scale]
	*/

	/*
	Damage Numbers
	[asw_floating_number_type]
	[asw_floating_number_combine]
	*/

	/*
	Speed Running
	[rd_draw_timer]
	[rd_draw_timer_color]
	[rda_print_chat_objective_completion_time]
	[rda_print_console_objective_completion_time]
	[cl_auto_restart_mission]
	*/

	/*
	Leaderboards
	[rd_leaderboard_enabled_client]
	[rd_show_leaderboard_loading]
	[rd_show_leaderboard_debrief]
	*/

	/*
	Loading
	[rd_show_mission_icon_loading]
	[rd_loading_image_per_map]
	[rd_loading_status_text_visible]
	*/

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
	Assert( !"TODO" );
}
