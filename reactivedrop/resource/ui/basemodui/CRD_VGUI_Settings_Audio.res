"Resource/UI/BaseModUI/CRD_VGUI_Settings_Audio.res"
{
	"BackgroundOverall"
	{
		"ControlName"		"Panel"
		"fieldName"			"BackgroundOverall"
		"xpos"				"14"
		"ypos"				"39"
		"zpos"				"-1"
		"wide"				"274"
		"tall"				"19"
		"bgcolor_override"	"0 0 0 192"
	}

	"BackgroundMusic"
	{
		"ControlName"		"Panel"
		"fieldName"			"BackgroundMusic"
		"xpos"				"14"
		"ypos"				"62"
		"zpos"				"-1"
		"wide"				"274"
		"tall"				"48"
		"bgcolor_override"	"0 0 0 192"
	}

	"BackgroundVoice"
	{
		"ControlName"		"Panel"
		"fieldName"			"BackgroundVoice"
		"xpos"				"14"
		"ypos"				"118"
		"zpos"				"-1"
		"wide"				"274"
		"tall"				"72"
		"bgcolor_override"	"0 0 0 192"
	}

	"BackgroundMenus"
	{
		"ControlName"		"Panel"
		"fieldName"			"BackgroundMenus"
		"xpos"				"14"
		"ypos"				"198"
		"zpos"				"-1"
		"wide"				"274"
		"tall"				"24"
		"bgcolor_override"	"0 0 0 192"
	}

	"BackgroundEnvironment"
	{
		"ControlName"		"Panel"
		"fieldName"			"BackgroundEnvironment"
		"xpos"				"14"
		"ypos"				"230"
		"zpos"				"-1"
		"wide"				"274"
		"tall"				"72"
		"bgcolor_override"	"0 0 0 192"
	}

	"BackgroundCombat"
	{
		"ControlName"		"Panel"
		"fieldName"			"BackgroundCombat"
		"xpos"				"14"
		"ypos"				"310"
		"zpos"				"-1"
		"wide"				"274"
		"tall"				"48"
		"bgcolor_override"	"0 0 0 192"
	}

	"BackgroundOther"
	{
		"ControlName"		"Panel"
		"fieldName"			"BackgroundOther"
		"xpos"				"14"
		"ypos"				"366"
		"zpos"				"-1"
		"wide"				"274"
		"tall"				"24"
		"bgcolor_override"	"0 0 0 192"
	}

	"BackgroundConfig"
	{
		"ControlName"		"Panel"
		"fieldName"			"BackgroundConfig"
		"xpos"				"298"	[!$WIN32WIDE]
		"xpos"				"322"	[$WIN32WIDE]
		"ypos"				"38"
		"zpos"				"-1"
		"wide"				"264"	[!$WIN32WIDE]
		"wide"				"274"	[$WIN32WIDE]
		"tall"				"72"
		"bgcolor_override"	"0 0 0 192"
	}

	"BackgroundPreferences"
	{
		"ControlName"		"Panel"
		"fieldName"			"BackgroundPreferences"
		"xpos"				"298"	[!$WIN32WIDE]
		"xpos"				"322"	[$WIN32WIDE]
		"ypos"				"118"
		"zpos"				"-1"
		"wide"				"264"	[!$WIN32WIDE]
		"wide"				"274"	[$WIN32WIDE]
		"tall"				"48"
		"bgcolor_override"	"0 0 0 192"
	}

	"BackgroundVoiceChat"
	{
		"ControlName"		"Panel"
		"fieldName"			"BackgroundVoiceChat"
		"xpos"				"298"	[!$WIN32WIDE]
		"xpos"				"322"	[$WIN32WIDE]
		"ypos"				"174"
		"zpos"				"-1"
		"wide"				"264"	[!$WIN32WIDE]
		"wide"				"274"	[$WIN32WIDE]
		"tall"				"84"
		"bgcolor_override"	"0 0 0 192"
	}

	"BackgroundHitSounds"
	{
		"ControlName"		"Panel"
		"fieldName"			"BackgroundHitSounds"
		"xpos"				"298"	[!$WIN32WIDE]
		"xpos"				"322"	[$WIN32WIDE]
		"ypos"				"286"
		"zpos"				"-1"
		"wide"				"264"	[!$WIN32WIDE]
		"wide"				"274"	[$WIN32WIDE]
		"tall"				"48"
		"bgcolor_override"	"0 0 0 192"
	}

	"BackgroundKillSounds"
	{
		"ControlName"		"Panel"
		"fieldName"			"BackgroundKillSounds"
		"xpos"				"298"	[!$WIN32WIDE]
		"xpos"				"322"	[$WIN32WIDE]
		"ypos"				"342"
		"zpos"				"-1"
		"wide"				"264"	[!$WIN32WIDE]
		"wide"				"274"	[$WIN32WIDE]
		"tall"				"48"
		"bgcolor_override"	"0 0 0 192"
	}

	"MixerOverallVolume"
	{
		"ControlName"		"CRD_VGUI_Option_Mixer"
		"fieldName"			"MixerOverallVolume"
		"xpos"				"16"
		"ypos"				"40"
		"wide"				"270"
		"tall"				"15"
		"displayMultiplier"	"100"
		"displaySuffix"		"#rd_option_suffix_percent"
		"tabPosition"		"1"
		"ResourceFile"		"resource/ui/option_audio_main.res"
		"navLeft"			"MixerOverallVolume"
		"navRight"			""
		"navUp"				""
		"navDown"			"MixerMusicMenus"
	}

	"MixerMusicMenus"
	{
		"ControlName"		"CRD_VGUI_Option_Mixer"
		"fieldName"			"MixerMusicMenus"
		"xpos"				"16"
		"ypos"				"64"
		"wide"				"270"
		"tall"				"24"
		"displayMultiplier"	"100"
		"displaySuffix"		"#rd_option_suffix_percent"
		"ResourceFile"		"resource/ui/option_audio_mixer.res"
		"navLeft"			"MixerMusicMenus"
		"navRight"			""
		"navUp"				"MixerOverallVolume"
		"navDown"			"MixerMusicInGame"
	}

	"MixerMusicInGame"
	{
		"ControlName"		"CRD_VGUI_Option_Mixer"
		"fieldName"			"MixerMusicInGame"
		"xpos"				"16"
		"ypos"				"88"
		"wide"				"270"
		"tall"				"24"
		"displayMultiplier"	"100"
		"displaySuffix"		"#rd_option_suffix_percent"
		"ResourceFile"		"resource/ui/option_audio_mixer.res"
		"navLeft"			"MixerMusicInGame"
		"navRight"			""
		"navUp"				"MixerMusicMenus"
		"navDown"			"MixerVoiceDialogue"
	}

	"MixerVoiceDialogue"
	{
		"ControlName"		"CRD_VGUI_Option_Mixer"
		"fieldName"			"MixerVoiceDialogue"
		"xpos"				"16"
		"ypos"				"120"
		"wide"				"270"
		"tall"				"24"
		"displayMultiplier"	"100"
		"displaySuffix"		"#rd_option_suffix_percent"
		"ResourceFile"		"resource/ui/option_audio_mixer.res"
		"navLeft"			"MixerVoiceDialogue"
		"navRight"			""
		"navUp"				"MixerMusicInGame"
		"navDown"			"MixerVoicePlayers"
	}

	"MixerVoicePlayers"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"MixerVoicePlayers"
		"xpos"				"16"
		"ypos"				"144"
		"wide"				"270"
		"tall"				"24"
		"displayMultiplier"	"100"
		"displaySuffix"		"#rd_option_suffix_percent"
		"ResourceFile"		"resource/ui/option_audio_mixer.res"
		"navLeft"			"MixerVoicePlayers"
		"navRight"			""
		"navUp"				"MixerVoiceDialogue"
		"navDown"			"MixerVoiceAliens"
	}

	"MixerVoiceAliens"
	{
		"ControlName"		"CRD_VGUI_Option_Mixer"
		"fieldName"			"MixerVoiceAliens"
		"xpos"				"16"
		"ypos"				"168"
		"wide"				"270"
		"tall"				"24"
		"displayMultiplier"	"100"
		"displaySuffix"		"#rd_option_suffix_percent"
		"ResourceFile"		"resource/ui/option_audio_mixer.res"
		"navLeft"			"MixerVoiceAliens"
		"navRight"			""
		"navUp"				"MixerVoicePlayers"
		"navDown"			"MixerInterface"
	}

	"MixerInterface"
	{
		"ControlName"		"CRD_VGUI_Option_Mixer"
		"fieldName"			"MixerInterface"
		"xpos"				"16"
		"ypos"				"200"
		"wide"				"270"
		"tall"				"24"
		"displayMultiplier"	"100"
		"displaySuffix"		"#rd_option_suffix_percent"
		"ResourceFile"		"resource/ui/option_audio_mixer.res"
		"navLeft"			"MixerInterface"
		"navRight"			""
		"navUp"				"MixerVoiceAliens"
		"navDown"			"MixerEnvironmentAmbient"
	}

	"MixerEnvironmentAmbient"
	{
		"ControlName"		"CRD_VGUI_Option_Mixer"
		"fieldName"			"MixerEnvironmentAmbient"
		"xpos"				"16"
		"ypos"				"232"
		"wide"				"270"
		"tall"				"24"
		"displayMultiplier"	"100"
		"displaySuffix"		"#rd_option_suffix_percent"
		"ResourceFile"		"resource/ui/option_audio_mixer.res"
		"navLeft"			"MixerEnvironmentAmbient"
		"navRight"			""
		"navUp"				"MixerInterface"
		"navDown"			"MixerEnvironmentPhysics"
	}

	"MixerEnvironmentPhysics"
	{
		"ControlName"		"CRD_VGUI_Option_Mixer"
		"fieldName"			"MixerEnvironmentPhysics"
		"xpos"				"16"
		"ypos"				"256"
		"wide"				"270"
		"tall"				"24"
		"displayMultiplier"	"100"
		"displaySuffix"		"#rd_option_suffix_percent"
		"ResourceFile"		"resource/ui/option_audio_mixer.res"
		"navLeft"			"MixerEnvironmentPhysics"
		"navRight"			""
		"navUp"				"MixerEnvironmentAmbient"
		"navDown"			"MixerExplosions"
	}

	"MixerExplosions"
	{
		"ControlName"		"CRD_VGUI_Option_Mixer"
		"fieldName"			"MixerExplosions"
		"xpos"				"16"
		"ypos"				"280"
		"wide"				"270"
		"tall"				"24"
		"displayMultiplier"	"100"
		"displaySuffix"		"#rd_option_suffix_percent"
		"ResourceFile"		"resource/ui/option_audio_mixer.res"
		"navLeft"			"MixerExplosions"
		"navRight"			""
		"navUp"				"MixerEnvironmentPhysics"
		"navDown"			"MixerCombatDamage"
	}

	"MixerCombatDamage"
	{
		"ControlName"		"CRD_VGUI_Option_Mixer"
		"fieldName"			"MixerCombatDamage"
		"xpos"				"16"
		"ypos"				"312"
		"wide"				"270"
		"tall"				"24"
		"displayMultiplier"	"100"
		"displaySuffix"		"#rd_option_suffix_percent"
		"ResourceFile"		"resource/ui/option_audio_mixer.res"
		"navLeft"			"MixerCombatDamage"
		"navRight"			""
		"navUp"				"MixerExplosions"
		"navDown"			"MixerCombatWeapons"
	}

	"MixerCombatWeapons"
	{
		"ControlName"		"CRD_VGUI_Option_Mixer"
		"fieldName"			"MixerCombatWeapons"
		"xpos"				"16"
		"ypos"				"336"
		"wide"				"270"
		"tall"				"24"
		"displayMultiplier"	"100"
		"displaySuffix"		"#rd_option_suffix_percent"
		"ResourceFile"		"resource/ui/option_audio_mixer.res"
		"navLeft"			"MixerCombatWeapons"
		"navRight"			""
		"navUp"				"MixerCombatDamage"
		"navDown"			"MixerOther"
	}

	"MixerOther"
	{
		"ControlName"		"CRD_VGUI_Option_Mixer"
		"fieldName"			"MixerOther"
		"xpos"				"16"
		"ypos"				"368"
		"wide"				"270"
		"tall"				"24"
		"displayMultiplier"	"100"
		"displaySuffix"		"#rd_option_suffix_percent"
		"ResourceFile"		"resource/ui/option_audio_mixer.res"
		"navLeft"			"MixerOther"
		"navRight"			""
		"navUp"				"MixerCombatWeapons"
		"navDown"			"MixerOther"
	}

	"SettingSpeakerConfiguration"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingSpeakerConfiguration"
		"xpos"				"300"	[!$WIN32WIDE]
		"xpos"				"324"	[$WIN32WIDE]
		"ypos"				"40"
		"wide"				"260"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"ResourceFile"		"resource/ui/option_audio_generic.res"
		"navLeft"			""
		"navRight"			"SettingSpeakerConfiguration"
		"navDown"			"SettingSoundQuality"
	}

	"SettingSoundQuality"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingSoundQuality"
		"xpos"				"300"	[!$WIN32WIDE]
		"xpos"				"324"	[$WIN32WIDE]
		"ypos"				"64"
		"wide"				"260"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"ResourceFile"		"resource/ui/option_audio_generic.res"
		"navLeft"			""
		"navRight"			"SettingSoundQuality"
		"navUp"				"SettingSpeakerConfiguration"
		"navDown"			"SettingCaptioning"
	}

	"SettingCaptioning"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingCaptioning"
		"xpos"				"300"	[!$WIN32WIDE]
		"xpos"				"324"	[$WIN32WIDE]
		"ypos"				"88"
		"wide"				"260"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"ResourceFile"		"resource/ui/option_audio_generic.res"
		"navLeft"			""
		"navRight"			"SettingCaptioning"
		"navUp"				"SettingSoundQuality"
		"navDown"			"SettingStoryDialogue"
	}

	"SettingStoryDialogue"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingStoryDialogue"
		"xpos"				"300"	[!$WIN32WIDE]
		"xpos"				"324"	[$WIN32WIDE]
		"ypos"				"120"
		"wide"				"260"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"ResourceFile"		"resource/ui/option_audio_checkbox.res"
		"navLeft"			""
		"navRight"			"SettingStoryDialogue"
		"navUp"				"SettingCaptioning"
		"navDown"			"SettingLowHealthSound"
	}

	"SettingLowHealthSound"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingLowHealthSound"
		"xpos"				"300"	[!$WIN32WIDE]
		"xpos"				"324"	[$WIN32WIDE]
		"ypos"				"144"
		"wide"				"260"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"ResourceFile"		"resource/ui/option_audio_checkbox.res"
		"navLeft"			""
		"navRight"			"SettingLowHealthSound"
		"navUp"				"SettingStoryDialogue"
		"navDown"			"SettingVoiceChat"
	}

	"SettingVoiceChat"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingVoiceChat"
		"xpos"				"300"	[!$WIN32WIDE]
		"xpos"				"324"	[$WIN32WIDE]
		"ypos"				"176"
		"wide"				"260"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"ResourceFile"		"resource/ui/option_audio_generic.res"
		"navLeft"			""
		"navRight"			"SettingVoiceChat"
		"navUp"				"SettingLowHealthSound"
		"navDown"			"SettingVoiceSensitivity"
	}

	"SettingVoiceSensitivity"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingVoiceSensitivity"
		"xpos"				"300"	[!$WIN32WIDE]
		"xpos"				"324"	[$WIN32WIDE]
		"ypos"				"200"
		"wide"				"260"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"ResourceFile"		"resource/ui/option_audio_slider.res"
		"navLeft"			""
		"navRight"			"SettingVoiceSensitivity"
		"navUp"				"SettingVoiceChat"
		"navDown"			"SettingVoiceIconPosition"
	}

	"SettingVoiceIconPosition"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingVoiceIconPosition"
		"xpos"				"300"	[!$WIN32WIDE]
		"xpos"				"324"	[$WIN32WIDE]
		"ypos"				"224"
		"wide"				"260"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"15"
		"ResourceFile"		"resource/ui/option_audio_generic.res"
		"navLeft"			""
		"navRight"			"SettingVoiceIconPosition"
		"navUp"				"SettingVoiceSensitivity"
		"navDown"			"TestMicrophone"
	}

	"LblTestMicrophone"
	{
		"ControlName"		"Label"
		"fieldName"			"LblTestMicrophone"
		"xpos"				"300"	[!$WIN32WIDE]
		"xpos"				"324"	[$WIN32WIDE]
		"ypos"				"240"
		"wide"				"120"	[!$WIN32WIDE]
		"wide"				"140"	[$WIN32WIDE]
		"tall"				"12"
		"fgcolor_override"	"192 192 192 255"
		"labelText"			"#GameUI_TestMicrophone"
	}

	"TestMicrophone"
	{
		"ControlName"		"CRD_VGUI_Microphone_Tester"
		"fieldName"			"TestMicrophone"
		"xpos"				"431"	[!$WIN32WIDE]
		"xpos"				"465"	[$WIN32WIDE]
		"ypos"				"240"
		"wide"				"128"
		"tall"				"16"
		"meter_x"			"0"
		"meter_y"			"0"
		"meter_wide"		"16"
		"meter_tall"		"16"
		"inactive_texture"	"vgui/resource/mic_meter_dead"
		"active_texture"	"vgui/resource/mic_meter_live"
		"meter_texture"		"vgui/resource/mic_meter_indicator"
		"paintbackground"	"0"
		"navLeft"			""
		"navRight"			"TestMicrophone"
		"navUp"				"SettingVoiceIconPosition"
		"navDown"			"BtnCustomizeCombatMusic"
	}

	"BtnCustomizeCombatMusic"
	{
		"ControlName"		"BaseModHybridButton"
		"fieldName"			"BtnCustomizeCombatMusic"
		"xpos"				"300"	[!$WIN32WIDE]
		"xpos"				"324"	[$WIN32WIDE]
		"ypos"				"264"
		"wide"				"260"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"16"
		"labelText"			"#GameUI_Jukebox"
		"command"			"Jukebox"
		"centerwrap"		"1"
		"navLeft"			""
		"navRight"			"BtnCustomizeCombatMusic"
		"navUp"				"TestMicrophone"
		"navDown"			"SettingHitSoundType"
	}

	"SettingHitSoundType"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingHitSoundType"
		"xpos"				"300"	[!$WIN32WIDE]
		"xpos"				"324"	[$WIN32WIDE]
		"ypos"				"288"
		"wide"				"260"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"ResourceFile"		"resource/ui/option_audio_generic.res"
		"navLeft"			""
		"navRight"			"SettingHitSoundType"
		"navUp"				"BtnCustomizeCombatMusic"
		"navDown"			"SettingHitSoundVolume"
	}

	"SettingHitSoundVolume"
	{
		"ControlName"		"CRD_VGUI_Option_HitSound"
		"fieldName"			"SettingHitSoundVolume"
		"xpos"				"300"	[!$WIN32WIDE]
		"xpos"				"324"	[$WIN32WIDE]
		"ypos"				"312"
		"wide"				"260"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"displayMultiplier"	"100"
		"displaySuffix"		"#rd_option_suffix_percent"
		"ResourceFile"		"resource/ui/option_audio_slider.res"
		"navLeft"			""
		"navRight"			"SettingHitSoundVolume"
		"navUp"				"SettingHitSoundType"
		"navDown"			"SettingKillSoundType"
	}

	"SettingKillSoundType"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingKillSoundType"
		"xpos"				"300"	[!$WIN32WIDE]
		"xpos"				"324"	[$WIN32WIDE]
		"ypos"				"344"
		"wide"				"260"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"ResourceFile"		"resource/ui/option_audio_generic.res"
		"navLeft"			""
		"navRight"			"SettingKillSoundType"
		"navUp"				"SettingHitSoundVolume"
		"navDown"			"SettingKillSoundVolume"
	}

	"SettingKillSoundVolume"
	{
		"ControlName"		"CRD_VGUI_Option_HitSound"
		"fieldName"			"SettingKillSoundVolume"
		"xpos"				"300"	[!$WIN32WIDE]
		"xpos"				"324"	[$WIN32WIDE]
		"ypos"				"368"
		"wide"				"260"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"displayMultiplier"	"100"
		"displaySuffix"		"#rd_option_suffix_percent"
		"ResourceFile"		"resource/ui/option_audio_slider.res"
		"navLeft"			""
		"navRight"			"SettingKillSoundVolume"
		"navUp"				"SettingKillSoundType"
		"navDown"			"SettingKillSoundVolume"
	}
}
