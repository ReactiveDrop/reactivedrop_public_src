#include "cbase.h"
#include "rd_vgui_settings.h"
#include "engine/IEngineSound.h"
#include "gameui/swarm/vhybridbutton.h"
#include "vgui_controls/ImagePanel.h"
#include "vgui/ISurface.h"
#include "asw_util_shared.h"
#include "ivoicetweak.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

DECLARE_BUILD_FACTORY( CRD_VGUI_Settings_Audio );

extern ConVar tf_dingalingaling_effect[2];
extern ConVar tf_dingaling_pitch_override[2];
extern ConVar tf_dingaling_pitchmindmg[2];
extern ConVar tf_dingaling_pitchmaxdmg[2];

static IVoiceTweak *g_pVoiceTweak = NULL;

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

		if ( IsEnabled() && ( !g_pVoiceTweak || !g_pVoiceTweak->IsStillTweaking() ) )
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

class CRD_VGUI_Option_HitSound : public CRD_VGUI_Option
{
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Option_HitSound, CRD_VGUI_Option );
public:
	CRD_VGUI_Option_HitSound( vgui::Panel *parent, const char *panelName, const char *szLabel, const char *szGameSoundPrefix, bool bIsLastHit ) :
		BaseClass{ parent, panelName, szLabel, MODE_SLIDER }
	{
		m_pImgSoundPlaying = new vgui::ImagePanel( this, "ImgSoundPlaying" );

		m_szGameSoundPrefix = szGameSoundPrefix;
		m_bIsLastHit = bIsLastHit;

		m_bPlaying = false;
		m_hLastSound = 0;
	}

	~CRD_VGUI_Option_HitSound()
	{
		StopSound();
	}

	void NavigateTo() override
	{
		BaseClass::NavigateTo();

		if ( IsEnabled() && ( !g_pVoiceTweak || !g_pVoiceTweak->IsStillTweaking() ) )
			StartSound();
	}

	void NavigateFrom() override
	{
		StopSound();

		BaseClass::NavigateFrom();
	}

	void OnThink() override
	{
		BaseClass::OnThink();

		if ( m_bPlaying && ( m_hLastSound == 0 || !enginesound->IsSoundStillPlaying( m_hLastSound ) ) )
		{
			CFmtStr szSoundName{ "%s%s", m_szGameSoundPrefix, s_DingalingSounds[clamp( tf_dingalingaling_effect[m_bIsLastHit].GetInt(), 0, int( NELEMS( s_DingalingSounds ) ) - 1 )] };
			const char *szWavFile = soundemitterbase->GetWavFileForSound( szSoundName, GENDER_NONE );
			float flVolume = GetCurrentSliderValue();
			int iPitch = tf_dingaling_pitch_override[m_bIsLastHit].GetFloat() > 0 ?
				tf_dingaling_pitch_override[m_bIsLastHit].GetFloat() :
				RandomFloat( tf_dingaling_pitchmindmg[m_bIsLastHit].GetFloat(),
					tf_dingaling_pitchmaxdmg[m_bIsLastHit].GetFloat() );
			enginesound->EmitAmbientSound( szWavFile, flVolume, iPitch );
			m_hLastSound = enginesound->GetGuidForLastSoundEmitted();
		}
	}

	void StartSound()
	{
		m_pImgSoundPlaying->SetVisible( true );

		m_bPlaying = true;
	}

	void StopSound()
	{
		m_pImgSoundPlaying->SetVisible( false );

		m_bPlaying = false;

		if ( m_hLastSound != 0 )
		{
			enginesound->StopSoundByGuid( m_hLastSound );
			m_hLastSound = 0;
		}
	}

	vgui::ImagePanel *m_pImgSoundPlaying;

	const char *m_szGameSoundPrefix;
	bool m_bIsLastHit;

	bool m_bPlaying;
	int m_hLastSound;
};

class CRD_VGUI_Microphone_Tester : public vgui::Button
{
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Microphone_Tester, vgui::Button );
public:
	CRD_VGUI_Microphone_Tester( vgui::Panel *parent, const char *panelName ) :
		BaseClass( parent, panelName, "", this, "ToggleVoiceTweak" )
	{
		SetConsoleStylePanel( true );

#ifndef NO_VOICE
		g_pVoiceTweak = engine->GetVoiceTweakAPI();
#else
		g_pVoiceTweak = NULL;
#endif

		if ( g_pVoiceTweak )
		{
			// Set voice receive volume to 100% as we use the mixer now.
			// All other numbers we can change through this API mess with Windows settings, so don't touch them.
			g_pVoiceTweak->SetControlFloat( OtherSpeakerScale, 1.0f );
		}
	}

	~CRD_VGUI_Microphone_Tester()
	{
		if ( g_pVoiceTweak && g_pVoiceTweak->IsStillTweaking() )
			g_pVoiceTweak->EndVoiceTweakMode();
	}

	void NavigateTo() override
	{
		BaseClass::NavigateTo();

		RequestFocus();

		BaseModUI::CBaseModPanel::GetSingleton().PlayUISound( BaseModUI::UISOUND_FOCUS );
	}

	void OnCommand( const char *command ) override
	{
		if ( !V_stricmp( command, "ToggleVoiceTweak" ) )
		{
			if ( !g_pVoiceTweak )
				return;

			if ( g_pVoiceTweak->IsStillTweaking() )
				g_pVoiceTweak->EndVoiceTweakMode();
			else
				g_pVoiceTweak->StartVoiceTweakMode();

			BaseModUI::CBaseModPanel::GetSingleton().PlayUISound( BaseModUI::UISOUND_CLICK );

			return;
		}

		BaseClass::OnCommand( command );
	}

	void Paint() override
	{
		int wide, tall;
		GetSize( wide, tall );

		if ( !g_pVoiceTweak || !g_pVoiceTweak->IsStillTweaking() )
		{
			vgui::surface()->DrawSetColor( 255, 255, 255, 255 );
			vgui::surface()->DrawSetTexture( m_iInactiveTexture );
			vgui::surface()->DrawTexturedRect( 0, 0, wide, tall );

			return;
		}

		vgui::surface()->DrawSetColor( 255, 255, 255, 255 );
		vgui::surface()->DrawSetTexture( m_iActiveTexture );
		vgui::surface()->DrawTexturedRect( 0, 0, wide, tall );

		float flSpeakingVolume = g_pVoiceTweak->GetControlFloat( SpeakingVolume );
		int x = m_iMeterX - ( m_iMeterWide / 2 ) + GetWide() * flSpeakingVolume;

		vgui::surface()->DrawSetTexture( m_iMeterTexture );
		vgui::surface()->DrawTexturedRect( x, m_iMeterY, x + m_iMeterWide, m_iMeterY + m_iMeterTall );
	}

	CPanelAnimationVarAliasType( int, m_iMeterX, "meter_x", "0", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iMeterY, "meter_y", "0", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iMeterWide, "meter_wide", "16", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iMeterTall, "meter_tall", "16", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iInactiveTexture, "inactive_texture", "vgui/resource/mic_meter_dead", "textureid" );
	CPanelAnimationVarAliasType( int, m_iActiveTexture, "active_texture", "vgui/resource/mic_meter_live", "textureid" );
	CPanelAnimationVarAliasType( int, m_iMeterTexture, "meter_texture", "vgui/resource/mic_meter_indicator", "textureid" );
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

	m_pSettingSpeakerConfiguration = new CRD_VGUI_Option( this, "SettingSpeakerConfiguration", "#rd_audio_speaker_configuration", CRD_VGUI_Option::MODE_DROPDOWN );
	m_pSettingSpeakerConfiguration->SetDefaultHint( "#L4D_spkr_config_tip" );
	m_pSettingSpeakerConfiguration->AddOption( 2, "#GameUI_2Speakers", "#rd_audio_speaker_configuration_2_hint" );
	m_pSettingSpeakerConfiguration->AddOption( 0, "#GameUI_Headphones", "#rd_audio_speaker_configuration_headphones_hint" );
	m_pSettingSpeakerConfiguration->AddOption( 4, "#GameUI_4Speakers", "#rd_audio_speaker_configuration_4_hint" );
	m_pSettingSpeakerConfiguration->AddOption( 5, "#GameUI_5Speakers", "#rd_audio_speaker_configuration_5_hint" );
	m_pSettingSpeakerConfiguration->AddOption( 7, "#GameUI_7Speakers", "#rd_audio_speaker_configuration_7_hint" );
	m_pSettingSpeakerConfiguration->LinkToConVar( "snd_surround_speakers", false );

	m_pSettingSoundQuality = new CRD_VGUI_Option( this, "SettingSoundQuality", "#rd_audio_sound_quality", CRD_VGUI_Option::MODE_DROPDOWN );
	m_pSettingSoundQuality->SetDefaultHint( "#L4D_sound_qual_tip" );
	m_pSettingSoundQuality->AddOption( 0, "#rd_audio_sound_quality_low", "#rd_audio_sound_quality_low_hint" );
	m_pSettingSoundQuality->LinkToConVarAdvanced( 0, "snd_pitchquality", 0 );
	m_pSettingSoundQuality->LinkToConVarAdvanced( 0, "dsp_slow_cpu", 1 );
	m_pSettingSoundQuality->AddOption( 1, "#rd_audio_sound_quality_medium", "#rd_audio_sound_quality_medium_hint" );
	m_pSettingSoundQuality->LinkToConVarAdvanced( 1, "snd_pitchquality", 0 );
	m_pSettingSoundQuality->LinkToConVarAdvanced( 1, "dsp_slow_cpu", 0 );
	m_pSettingSoundQuality->AddOption( 2, "#rd_audio_sound_quality_high", "#rd_audio_sound_quality_high_hint" );
	m_pSettingSoundQuality->LinkToConVarAdvanced( 2, "snd_pitchquality", 1 );
	m_pSettingSoundQuality->LinkToConVarAdvanced( 2, "dsp_slow_cpu", 0 );
	m_pSettingSoundQuality->SetCurrentUsingConVars();
	m_pSettingSoundQuality->SetRecommendedOption( 2 );

	m_pSettingCaptioning = new CRD_VGUI_Option( this, "SettingCaptioning", "#GameUI_Captioning", CRD_VGUI_Option::MODE_DROPDOWN );
	m_pSettingCaptioning->SetDefaultHint( "#L4D360UI_AudioOptions_Tooltip_Caption" );
	m_pSettingCaptioning->AddOption( 0, "#GameUI_NoClosedCaptions", "#L4D360UI_AudioOptions_CaptionOff_Tooltip" );
	m_pSettingCaptioning->LinkToConVarAdvanced( 0, "closecaption", 0 );
	m_pSettingCaptioning->AddOption( 1, "#GameUI_Subtitles", "#L4D360UI_AudioOptions_Tooltip_CaptionSubtitles" );
	m_pSettingCaptioning->LinkToConVarAdvanced( 1, "closecaption", 1 );
	m_pSettingCaptioning->LinkToConVarAdvanced( 1, "cc_subtitles", 1 );
	m_pSettingCaptioning->AddOption( 2, "#GameUI_SubtitlesAndSoundEffects", "#L4D360UI_AudioOptions_Tooltip_CaptionOn" );
	m_pSettingCaptioning->LinkToConVarAdvanced( 2, "closecaption", 1 );
	m_pSettingCaptioning->LinkToConVarAdvanced( 2, "cc_subtitles", 0 );
	m_pSettingCaptioning->SetCurrentUsingConVars();

	m_pSettingStoryDialogue = new CRD_VGUI_Option( this, "SettingStoryDialogue", "#rd_audio_skip_all_story_dialogue", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingStoryDialogue->SetDefaultHint( "#rd_audio_skip_all_story_dialogue_hint" );
	m_pSettingStoryDialogue->LinkToConVar( "rd_skip_all_dialogue", false );

	m_pSettingLowHealthSound = new CRD_VGUI_Option( this, "SettingLowHealthSound", "#rd_audio_low_health_sound", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingLowHealthSound->SetDefaultHint( "#rd_audio_low_health_sound_hint" );
	m_pSettingLowHealthSound->LinkToConVar( "rd_hearbeat", false );

	m_pSettingVoiceChat = new CRD_VGUI_Option( this, "SettingVoiceChat", "#GameUI_VoiceCommStyle", CRD_VGUI_Option::MODE_DROPDOWN );
	m_pSettingVoiceChat->SetDefaultHint( "#L4D_voice_comm_tip" );
	m_pSettingVoiceChat->AddOption( 0, "#rd_audio_disable_voice_chat", "#L4D_voice_comm_tip" );
	m_pSettingVoiceChat->LinkToConVarAdvanced( 0, "voice_modenable", 0 );
	m_pSettingVoiceChat->LinkToConVarAdvanced( 0, "voice_enable", 0 );
	m_pSettingVoiceChat->AddOption( 1, "#L4D360UI_PushToTalk", "#L4D_voice_comm_tip" );
	m_pSettingVoiceChat->LinkToConVarAdvanced( 1, "voice_modenable", 1 );
	m_pSettingVoiceChat->LinkToConVarAdvanced( 1, "voice_enable", 1 );
	m_pSettingVoiceChat->LinkToConVarAdvanced( 1, "voice_vox", 0 );
	m_pSettingVoiceChat->AddOption( 2, "#L4D360UI_OpenMic", "#L4D_voice_comm_tip" );
	m_pSettingVoiceChat->LinkToConVarAdvanced( 2, "voice_modenable", 1 );
	m_pSettingVoiceChat->LinkToConVarAdvanced( 2, "voice_enable", 1 );
	m_pSettingVoiceChat->LinkToConVarAdvanced( 2, "voice_vox", 1 );
	m_pSettingVoiceChat->SetCurrentUsingConVars();
	m_pSettingVoiceChat->SetRecommendedOption( 1 );

	m_pSettingVoiceSensitivity = new CRD_VGUI_Option( this, "SettingVoiceSensitivity", "#GameUI_VoiceSensitivity", CRD_VGUI_Option::MODE_SLIDER );
	m_pSettingVoiceSensitivity->SetDefaultHint( "#L4D_mic_sens_tip" );
	m_pSettingVoiceSensitivity->SetSliderMinMax( 1.0f, 2000.0f );
	m_pSettingVoiceSensitivity->LinkToConVar( "voice_threshold", true );

	m_pSettingVoiceIconPosition = new CRD_VGUI_Option( this, "SettingVoiceIconPosition", "#RD_AdvancedSettings_HUD_VoiceIcon", CRD_VGUI_Option::MODE_DROPDOWN );
	m_pSettingVoiceIconPosition->AddOption( 0, "#RD_AdvancedSettings_HUD_VoiceIcon_Marine", "" );
	m_pSettingVoiceIconPosition->AddOption( 1, "#RD_AdvancedSettings_HUD_VoiceIcon_Side", "" );
	m_pSettingVoiceIconPosition->LinkToConVar( "asw_voice_side_icon", false );

	m_pTestMicrophone = new CRD_VGUI_Microphone_Tester( this, "TestMicrophone" );

	m_pBtnCustomizeCombatMusic = new BaseModUI::BaseModHybridButton( this, "BtnCustomizeCombatMusic", "#GameUI_Jukebox", this, "Jukebox" );

	m_pSettingHitSoundType = new CRD_VGUI_Option( this, "SettingHitSoundType", "#tf_dingalingaling_effect", CRD_VGUI_Option::MODE_DROPDOWN );
	m_pSettingHitSoundType->AddOption( -1, "#rd_video_effect_disabled", "#TF_Hitbeeps" );
	m_pSettingHitSoundType->LinkToConVarAdvanced( -1, "tf_dingalingaling", 0 );
	m_pSettingHitSoundVolume = new CRD_VGUI_Option_HitSound( this, "SettingHitSoundVolume", "#TF_Dingaling_Volume", "Player.HitSound", false );
	m_pSettingHitSoundVolume->SetDefaultHint( "#Tooltip_Dingaling_Volume" );
	m_pSettingHitSoundVolume->SetSliderMinMax( 0.0f, 1.5f );
	m_pSettingHitSoundVolume->LinkToConVar( "tf_dingaling_volume", true );

	m_pSettingKillSoundType = new CRD_VGUI_Option( this, "SettingKillSoundType", "#tf_dingalingaling_last_effect", CRD_VGUI_Option::MODE_DROPDOWN );
	m_pSettingKillSoundType->AddOption( -1, "#rd_video_effect_disabled", "#TF_LastHitbeeps" );
	m_pSettingKillSoundType->LinkToConVarAdvanced( -1, "tf_dingalingaling_lasthit", 0 );
	m_pSettingKillSoundVolume = new CRD_VGUI_Option_HitSound( this, "SettingKillSoundVolume", "#TF_Dingaling_LastHit_Volume", "Player.KillSound", true );
	m_pSettingKillSoundVolume->SetDefaultHint( "" );
	m_pSettingKillSoundVolume->SetSliderMinMax( 0.0f, 1.5f );
	m_pSettingKillSoundVolume->LinkToConVar( "tf_dingaling_lasthit_volume", true );

	for ( int i = 0; i < NELEMS( s_DingalingSounds ); i++ )
	{
		m_pSettingHitSoundType->AddOption( i, VarArgs( "#tf_dingalingaling_%s", s_DingalingSounds[i] ), "#TF_Hitbeeps" );
		m_pSettingHitSoundType->LinkToConVarAdvanced( i, "tf_dingalingaling", 1 );
		m_pSettingHitSoundType->LinkToConVarAdvanced( i, "tf_dingalingaling_effect", i );
		m_pSettingKillSoundType->AddOption( i, VarArgs( "#tf_dingalingaling_%s", s_DingalingSounds[i] ), "#TF_LastHitbeeps" );
		m_pSettingKillSoundType->LinkToConVarAdvanced( i, "tf_dingalingaling_lasthit", 1 );
		m_pSettingKillSoundType->LinkToConVarAdvanced( i, "tf_dingalingaling_last_effect", i );
	}

	m_pSettingHitSoundType->SetCurrentUsingConVars();
	m_pSettingKillSoundType->SetCurrentUsingConVars();
}

void CRD_VGUI_Settings_Audio::Activate()
{
	if ( m_bActivateWithoutNavigate )
		m_bActivateWithoutNavigate = false;
	else
		NavigateToChild( m_pMixerOverallVolume );

	m_pSettingVoiceSensitivity->SetEnabled( m_pSettingVoiceChat->GetCurrentOption() == 2 );
	m_pSettingVoiceIconPosition->SetEnabled( m_pSettingVoiceChat->GetCurrentOption() != 0 );
	m_pTestMicrophone->SetEnabled( m_pSettingVoiceChat->GetCurrentOption() != 0 );
	m_pSettingHitSoundVolume->SetEnabled( m_pSettingHitSoundType->GetCurrentOption() != -1 );
	m_pSettingKillSoundVolume->SetEnabled( m_pSettingKillSoundType->GetCurrentOption() != -1 );
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

void CRD_VGUI_Settings_Audio::OnCurrentOptionChanged( vgui::Panel *panel )
{
	ConVarRef dsp_enhance_stereo{ "dsp_enhance_stereo" };
	if ( dsp_enhance_stereo.IsValid() )
	{
		// headphones at high quality get enhanced stereo turned on (this affects reverb mostly)
		dsp_enhance_stereo.SetValue( m_pSettingSpeakerConfiguration->GetCurrentOption() == 0 && m_pSettingSoundQuality->GetCurrentOption() == 2 );
	}

	m_bActivateWithoutNavigate = true;
	Activate();
}
