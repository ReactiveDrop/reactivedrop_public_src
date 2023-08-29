#include "cbase.h"
#include "rd_vgui_settings.h"
#include "engine/IEngineSound.h"
#include <vgui_controls/ImagePanel.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

DECLARE_BUILD_FACTORY( CRD_VGUI_Settings_Audio );

class CRD_VGUI_Option_Mixer : public CRD_VGUI_Option
{
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Option_Mixer, CRD_VGUI_Option );
public:
	CRD_VGUI_Option_Mixer( vgui::Panel *parent, const char *panelName, const char *szLabel, const char *szExampleAudioLoop ) :
		BaseClass{ parent, panelName, szLabel, MODE_SLIDER }
	{
		m_pImgSoundPlaying = new vgui::ImagePanel( this, "ImgSoundPlaying" );

		V_strncpy( m_szExampleAudioLoop, szExampleAudioLoop, sizeof( m_szExampleAudioLoop ) );
		m_hPlayingSound = 0;
	}

	~CRD_VGUI_Option_Mixer()
	{
		StopSound();
	}

	void NavigateTo() override
	{
		BaseClass::NavigateTo();

		StartSound();
	}

	void NavigateFrom() override
	{
		StopSound();

		BaseClass::NavigateFrom();
	}

	void StartSound()
	{
		m_pImgSoundPlaying->SetVisible( true );

		if ( !m_hPlayingSound )
		{
			Assert( enginesound->IsLoopingSound( m_szExampleAudioLoop ) );

			enginesound->EmitAmbientSound( m_szExampleAudioLoop, 1.0f );
			m_hPlayingSound = enginesound->GetGuidForLastSoundEmitted();
			Assert( m_hPlayingSound );
		}
	}

	void StopSound()
	{
		m_pImgSoundPlaying->SetVisible( false );

		if ( m_hPlayingSound )
		{
			enginesound->StopSoundByGuid( m_hPlayingSound );
			m_hPlayingSound = 0;
		}
	}

	vgui::ImagePanel *m_pImgSoundPlaying;

	char m_szExampleAudioLoop[MAX_PATH];
	int m_hPlayingSound;
};

CRD_VGUI_Settings_Audio::CRD_VGUI_Settings_Audio( vgui::Panel *parent, const char *panelName ) :
	BaseClass( parent, panelName )
{
	m_pMixerOverallVolume = new CRD_VGUI_Option( this, "MixerOverallVolume", "#GameUI_SoundEffectVolume", CRD_VGUI_Option::MODE_SLIDER );
	m_pMixerOverallVolume->SetDefaultHint( "#L4D360UI_AudioOptions_Tooltip_Volume" );
	m_pMixerOverallVolume->LinkToConVar( "volume", false );
	m_pMixerMusicMenus = new CRD_VGUI_Option_Mixer( this, "MixerMusicMenus", "#rd_audio_mixer_music_menus_label", "music/solo/volume_preview.wav");
	m_pMixerMusicMenus->SetDefaultHint( "#rd_audio_mixer_music_menus_tooltip" );
	m_pMixerMusicMenus->SetSliderMinMax( 0.0f, 1.5f );
	m_pMixerMusicMenus->LinkToConVar( "rd_mixer_volume_music_menus", true );
	m_pMixerMusicInGame = new CRD_VGUI_Option_Mixer( this, "MixerMusicInGame", "#rd_audio_mixer_music_ingame_label", "music/volume_preview.wav" );
	m_pMixerMusicInGame->SetDefaultHint( "#rd_audio_mixer_music_ingame_tooltip" );
	m_pMixerMusicInGame->SetSliderMinMax( 0.0f, 1.5f );
	m_pMixerMusicInGame->LinkToConVar( "rd_mixer_volume_music_ingame", true );
	m_pMixerVoiceDialogue = new CRD_VGUI_Option_Mixer( this, "MixerVoiceDialogue", "#rd_audio_mixer_voice_dialogue_label", "vo/command/volume_preview.wav" );
	m_pMixerVoiceDialogue->SetDefaultHint( "#rd_audio_mixer_voice_dialogue_tooltip" );
	m_pMixerVoiceDialogue->SetSliderMinMax( 0.0f, 1.5f );
	m_pMixerVoiceDialogue->LinkToConVar( "rd_mixer_volume_voice_dialogue", true );
	m_pMixerVoicePlayers = new CRD_VGUI_Option( this, "MixerVoicePlayers", "#rd_audio_mixer_voice_players_label", CRD_VGUI_Option::MODE_SLIDER );
	m_pMixerVoicePlayers->SetDefaultHint( "#rd_audio_mixer_voice_players_tooltip" );
	m_pMixerVoicePlayers->SetSliderMinMax( 0.0f, 1.5f );
	m_pMixerVoicePlayers->LinkToConVar( "rd_mixer_volume_voice_players", true );
	m_pMixerVoiceAliens = new CRD_VGUI_Option_Mixer( this, "MixerVoiceAliens", "#rd_audio_mixer_voice_aliens_label", "aliens/voc/volume_preview.wav" );
	m_pMixerVoiceAliens->SetDefaultHint( "#rd_audio_mixer_voice_aliens_tooltip" );
	m_pMixerVoiceAliens->SetSliderMinMax( 0.0f, 1.5f );
	m_pMixerVoiceAliens->LinkToConVar( "rd_mixer_volume_voice_aliens", true );
	m_pMixerInterface = new CRD_VGUI_Option_Mixer( this, "MixerInterface", "#rd_audio_mixer_interface_label", "swarm/interface/volume_preview.wav" );
	m_pMixerInterface->SetDefaultHint( "#rd_audio_mixer_interface_tooltip" );
	m_pMixerInterface->SetSliderMinMax( 0.0f, 1.5f );
	m_pMixerInterface->LinkToConVar( "rd_mixer_volume_interface", true );
	m_pMixerEnvironmentAmbient = new CRD_VGUI_Option_Mixer( this, "MixerEnvironmentAmbient", "#rd_audio_mixer_environment_ambient_label", "ambient/volume_preview.wav" );
	m_pMixerEnvironmentAmbient->SetDefaultHint( "#rd_audio_mixer_environment_ambient_tooltip" );
	m_pMixerEnvironmentAmbient->SetSliderMinMax( 0.0f, 1.5f );
	m_pMixerEnvironmentAmbient->LinkToConVar( "rd_mixer_volume_environment_ambient", true );
	m_pMixerEnvironmentPhysics = new CRD_VGUI_Option_Mixer( this, "MixerEnvironmentPhysics", "#rd_audio_mixer_environment_physics_label", "physics/volume_preview.wav" );
	m_pMixerEnvironmentPhysics->SetDefaultHint( "#rd_audio_mixer_environment_physics_tooltip" );
	m_pMixerEnvironmentPhysics->SetSliderMinMax( 0.0f, 1.5f );
	m_pMixerEnvironmentPhysics->LinkToConVar( "rd_mixer_volume_environment_physics", true );
	m_pMixerExplosions = new CRD_VGUI_Option_Mixer( this, "MixerExplosions", "#rd_audio_mixer_explosions_label", "explosions/volume_preview.wav" );
	m_pMixerExplosions->SetDefaultHint( "#rd_audio_mixer_explosions_tooltip" );
	m_pMixerExplosions->SetSliderMinMax( 0.0f, 1.5f );
	m_pMixerExplosions->LinkToConVar( "rd_mixer_volume_explosions", true );
	m_pMixerCombatDamage = new CRD_VGUI_Option_Mixer( this, "MixerCombatDamage", "#rd_audio_mixer_combat_damage_label", "physics/2D/damage/volume_preview.wav" );
	m_pMixerCombatDamage->SetDefaultHint( "#rd_audio_mixer_combat_damage_tooltip" );
	m_pMixerCombatDamage->SetSliderMinMax( 0.0f, 1.5f );
	m_pMixerCombatDamage->LinkToConVar( "rd_mixer_volume_combat_damage", true );
	m_pMixerCombatWeapons = new CRD_VGUI_Option_Mixer( this, "MixerCombatWeapons", "#rd_audio_mixer_combat_weapons_label", "weapons/volume_preview.wav" );
	m_pMixerCombatWeapons->SetDefaultHint( "#rd_audio_mixer_combat_weapons_tooltip" );
	m_pMixerCombatWeapons->SetSliderMinMax( 0.0f, 1.5f );
	m_pMixerCombatWeapons->LinkToConVar( "rd_mixer_volume_combat_weapons", true );
	m_pMixerOther = new CRD_VGUI_Option_Mixer( this, "MixerOther", "#rd_audio_mixer_other_label", "swarm/volume_preview.wav" );
	m_pMixerOther->SetDefaultHint( "#rd_audio_mixer_other_tooltip" );
	m_pMixerOther->SetSliderMinMax( 0.0f, 1.5f );
	m_pMixerOther->LinkToConVar( "rd_mixer_volume_other", true );

	// snd_surround_speakers / dsp_enhance_stereo
	// #GameUI_SpeakerConfiguration
	// #L4D_spkr_config_tip

	// snd_pitch_quality / dsp_slow_cpu / dsp_enhance_stereo
	// #GameUI_SoundQuality
	// #L4D_sound_qual_tip

	// closecaption / cc_subtitles
	// #GameUI_Captioning
	// #L4D360UI_AudioOptions_Tooltip_Caption

	// rd_skip_all_dialogue

	// rd_hearbeat

	// -

	// voice_modenable / voice_enable / voice_vox
	// #GameUI_EnableVoice
	// #L4D_enable_voice_tip

	// asw_voice_side_icon

	// [voice sensitivity]

	// [set voice receive volume to 100% as we use the mixer now]

	// voice_mixer_boost

	// [test microphone]

	// -

	// [customize combat music]

	// -

	// tf_dingalingaling / tf_dingalingaling_effect

	// tf_dingaling_volume

	// -

	// tf_dingalingaling_lasthit / tf_dingalingaling_last_effect

	// tf_dingaling_lasthit_volume
}

void CRD_VGUI_Settings_Audio::Activate()
{
}

void CRD_VGUI_Settings_Audio::OnCommand( const char *command )
{
	if ( !V_stricmp( command, "Jukebox" ) )
	{
		BaseModUI::CBaseModPanel::GetSingleton().OpenWindow( BaseModUI::WT_JUKEBOX, assert_cast< BaseModUI::CRD_VGUI_Settings * >( GetParent() ), true );

		return;
	}

	BaseClass::OnCommand( command );
}
