Convars.SetValue( "asw_batch_interval", 3 );
Convars.SetValue( "asw_realistic_death_chatter", 1 );
Convars.SetValue( "asw_marine_ff", 2 );
Convars.SetValue( "asw_marine_ff_dmg_base", 3 );
Convars.SetValue( "asw_custom_skill_points", 0 );
Convars.SetValue( "asw_marine_death_cam_slowdown", 0 );
Convars.SetValue( "asw_marine_death_protection", 0 );
Convars.SetValue( "asw_marine_collision", 1 );
Convars.SetValue( "asw_horde_override", 1 );
Convars.SetValue( "asw_wanderer_override", 1 );
Convars.SetValue( "asw_difficulty_alien_health_step", 0.2 );
Convars.SetValue( "asw_difficulty_alien_damage_step", 0.2 );
Convars.SetValue( "asw_marine_time_until_ignite", 0 );
Convars.SetValue( "rd_marine_ignite_immediately", 1 );
Convars.SetValue( "asw_marine_burn_time_easy", 60 );
Convars.SetValue( "asw_marine_burn_time_normal", 60 );
Convars.SetValue( "asw_marine_burn_time_hard", 60 );
Convars.SetValue( "asw_marine_burn_time_insane", 60 );

if ( Convars.GetFloat( "asw_skill" ) == 1 ) { // easy
	Convars.SetValue( "asw_marine_speed_scale_easy", 0.96 );
	Convars.SetValue( "asw_alien_speed_scale_easy", 0.7 );
	Convars.SetValue( "asw_drone_acceleration", 5 );
	Convars.SetValue( "asw_horde_interval_min", 10 );
	Convars.SetValue( "asw_horde_interval_max", 30 );
	Convars.SetValue( "asw_director_peak_min_time", 2 );
	Convars.SetValue( "asw_director_peak_max_time", 4 );
	Convars.SetValue( "asw_director_relaxed_min_time", 15 );
	Convars.SetValue( "asw_director_relaxed_max_time", 30 );
} else if ( Convars.GetFloat( "asw_skill" ) == 2 ) { // normal
	Convars.SetValue( "asw_marine_speed_scale_normal", 1.0 );
	Convars.SetValue( "asw_alien_speed_scale_normal", 1.0 );
	Convars.SetValue( "asw_drone_acceleration", 5 );
	Convars.SetValue( "asw_horde_interval_min", 15 );
	Convars.SetValue( "asw_horde_interval_max", 60 );
	Convars.SetValue( "asw_director_peak_min_time", 2 );
	Convars.SetValue( "asw_director_peak_max_time", 4 );
	Convars.SetValue( "asw_director_relaxed_min_time", 15 );
	Convars.SetValue( "asw_director_relaxed_max_time", 30 );
} else if ( Convars.GetFloat( "asw_skill" ) == 3 ) { // hard
	Convars.SetValue( "asw_marine_speed_scale_hard", 1.024 );
	Convars.SetValue( "asw_alien_speed_scale_hard", 1.7 );
	Convars.SetValue( "asw_drone_acceleration", 8 );
	Convars.SetValue( "asw_horde_interval_min", 15 );
	Convars.SetValue( "asw_horde_interval_max", 120 );
	Convars.SetValue( "asw_director_peak_min_time", 2 );
	Convars.SetValue( "asw_director_peak_max_time", 4 );
	Convars.SetValue( "asw_director_relaxed_min_time", 15 );
	Convars.SetValue( "asw_director_relaxed_max_time", 30 );
} else if ( Convars.GetFloat( "asw_skill" ) == 4 ) { // insane
	Convars.SetValue( "asw_marine_speed_scale_insane", 1.048 );
	Convars.SetValue( "asw_alien_speed_scale_insane", 1.8 );
	Convars.SetValue( "asw_drone_acceleration", 9 );
	Convars.SetValue( "asw_horde_interval_min", 15 );
	Convars.SetValue( "asw_horde_interval_max", 80 );
	Convars.SetValue( "asw_director_peak_min_time", 2 );
	Convars.SetValue( "asw_director_peak_max_time", 4 );
	Convars.SetValue( "asw_director_relaxed_min_time", 15 );
	Convars.SetValue( "asw_director_relaxed_max_time", 30 );	
} else if ( Convars.GetFloat( "asw_skill" ) == 5 ) { // brutal
	Convars.SetValue( "asw_marine_speed_scale_insane", 1.048 );
	Convars.SetValue( "asw_alien_speed_scale_insane", 1.9 );
	Convars.SetValue( "asw_drone_acceleration", 10 );
	Convars.SetValue( "asw_horde_interval_min", 15 );
	Convars.SetValue( "asw_horde_interval_max", 60 );
	Convars.SetValue( "asw_director_peak_min_time", 2 );
	Convars.SetValue( "asw_director_peak_max_time", 4 );
	Convars.SetValue( "asw_director_relaxed_min_time", 10 );
	Convars.SetValue( "asw_director_relaxed_max_time", 30 );
}
