"Resource/UI/BaseModUI/CRD_VGUI_Settings_Audio.res"
{
	"BackgroundOverall"
	{
		"ControlName"		"Panel"
		"fieldName"			"BackgroundOverall"
		"xpos"				"14"
		"ypos"				"39"
		"zpos"				"-1"
		"wide"				"244"	[!$WIN32WIDE]
		"wide"				"274"	[$WIN32WIDE]
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
		"wide"				"244"	[!$WIN32WIDE]
		"wide"				"274"	[$WIN32WIDE]
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
		"wide"				"244"	[!$WIN32WIDE]
		"wide"				"274"	[$WIN32WIDE]
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
		"wide"				"244"	[!$WIN32WIDE]
		"wide"				"274"	[$WIN32WIDE]
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
		"wide"				"244"	[!$WIN32WIDE]
		"wide"				"274"	[$WIN32WIDE]
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
		"wide"				"244"	[!$WIN32WIDE]
		"wide"				"274"	[$WIN32WIDE]
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
		"wide"				"244"	[!$WIN32WIDE]
		"wide"				"274"	[$WIN32WIDE]
		"tall"				"24"
		"bgcolor_override"	"0 0 0 192"
	}

	"MixerOverallVolume"
	{
		"ControlName"		"CRD_VGUI_Option_Mixer"
		"fieldName"			"MixerOverallVolume"
		"xpos"				"16"
		"ypos"				"40"
		"wide"				"240"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"15"
		"percentage"		"1"
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
		"wide"				"240"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"percentage"		"1"
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
		"wide"				"240"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"percentage"		"1"
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
		"wide"				"240"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"percentage"		"1"
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
		"wide"				"240"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"percentage"		"1"
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
		"wide"				"240"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"percentage"		"1"
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
		"wide"				"240"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"percentage"		"1"
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
		"wide"				"240"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"percentage"		"1"
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
		"wide"				"240"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"percentage"		"1"
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
		"wide"				"240"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"percentage"		"1"
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
		"wide"				"240"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"percentage"		"1"
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
		"wide"				"240"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"percentage"		"1"
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
		"wide"				"240"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"percentage"		"1"
		"ResourceFile"		"resource/ui/option_audio_mixer.res"
		"navLeft"			"MixerOther"
		"navRight"			""
		"navUp"				"MixerCombatWeapons"
		"navDown"			"MixerOther"
	}

	"BtnJukebox"
	{
		"ControlName"		"BaseModHybridButton"
		"fieldName"			"BtnJukebox"
		"xpos"				"412"	[!$WIN32WIDE]
		"xpos"				"442"	[$WIN32WIDE]
		"ypos"				"300"
		"wide"				"128"
		"tall"				"34"
		"labelText"			"#GameUI_Jukebox"
		"command"			"Jukebox"
		"centerwrap"		"1"
		"navLeft"			""
		"navRight"			"BtnJukebox"
		"navUp"				""
		"navDown"			""
	}
}
