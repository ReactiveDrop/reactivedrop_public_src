#include "cbase.h"
#include "rd_vgui_settings.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

DECLARE_BUILD_FACTORY( CRD_VGUI_Settings_Audio );

CRD_VGUI_Settings_Audio::CRD_VGUI_Settings_Audio( vgui::Panel *parent, const char *panelName ) :
	BaseClass( parent, panelName )
{
	// TODO:
	// volume
	// rd_mixer_volume_music_menus
	// rd_mixer_volume_music_ingame
	// rd_mixer_volume_voice_dialogue
	// rd_mixer_volume_voice_players
	// rd_mixer_volume_voice_aliens
	// rd_mixer_volume_interface
	// rd_mixer_volume_environment_ambient
	// rd_mixer_volume_environment_physics
	// rd_mixer_volume_combat_damage
	// rd_mixer_volume_combat_weapons
	// rd_mixer_volume_explosions
	// rd_mixer_volume_other
	//
	// snd_surround_speakers / dsp_enhance_stereo
	// snd_pitch_quality / dsp_slow_cpu / dsp_enhance_stereo
	// closecaption / cc_subtitles
	// rd_skip_all_dialogue
	// rd_hearbeat
	//
	// voice_modenable / voice_enable / voice_vox
	// asw_voice_side_icon
	// [voice sensitivity]
	// [set voice receive volume to 100% as we use the mixer now]
	// voice_mixer_boost
	// [test microphone]
	//
	// [customize combat music]
	//
	// tf_dingalingaling / tf_dingalingaling_effect
	// tf_dingaling_volume
	//
	// tf_dingalingaling_lasthit / tf_dingalingaling_last_effect
	// tf_dingaling_lasthit_volume
}

void CRD_VGUI_Settings_Audio::Activate()
{
	Assert( !"TODO" );
}
