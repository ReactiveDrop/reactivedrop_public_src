"Resource/UI/BaseModUI/CRD_VGUI_Settings_Video.res"
{
	"BackgroundResolution"
	{
		"ControlName"		"Panel"
		"fieldName"			"BackgroundResolution"
		"xpos"				"14"
		"ypos"				"39"
		"zpos"				"-1"
		"wide"				"274"
		"tall"				"19"
		"bgcolor_override"	"0 0 0 192"
	}

	"BackgroundMode"
	{
		"ControlName"		"Panel"
		"fieldName"			"BackgroundMode"
		"xpos"				"14"
		"ypos"				"60"
		"zpos"				"-1"
		"wide"				"274"
		"tall"				"151"
		"bgcolor_override"	"0 0 0 192"
	}

	"BackgroundDetail"
	{
		"ControlName"		"Panel"
		"fieldName"			"BackgroundDetail"
		"xpos"				"290"
		"ypos"				"50"
		"zpos"				"-1"
		"wide"				"306"
		"tall"				"161"
		"bgcolor_override"	"0 0 0 192"
	}

	"BackgroundDetail1"
	{
		"ControlName"		"Panel"
		"fieldName"			"BackgroundDetail1"
		"xpos"				"353"
		"ypos"				"39"
		"zpos"				"-1"
		"wide"				"243"
		"tall"				"12"
		"bgcolor_override"	"0 0 0 192"
	}

	"BackgroundExtra"
	{
		"ControlName"		"Panel"
		"fieldName"			"BackgroundExtra"
		"xpos"				"14"
		"ypos"				"214"
		"zpos"				"-1"
		"wide"				"582"
		"tall"				"178"
		"bgcolor_override"	"0 0 0 192"
	}

	"SettingScreenResolution"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingScreenResolution"
		"xpos"				"16"
		"ypos"				"40"
		"wide"				"270"
		"tall"				"17"
		"ResourceFile"		"resource/ui/option_video_resolution.res"
		"navLeft"			"SettingScreenResolution"
		"navRight"			"SettingEffectDetail"
		"navDown"			"SettingDisplayMode"
	}

	"SettingDisplayMode"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingDisplayMode"
		"xpos"				"16"
		"ypos"				"62"
		"wide"				"270"
		"tall"				"36"
		"ResourceFile"		"resource/ui/option_video_mode.res"
		"navLeft"			"SettingDisplayMode"
		"navRight"			"SettingShaderDetail"
		"navUp"				"SettingScreenResolution"
		"navDown"			"SettingScreenBrightness"
	}

	"SettingScreenBrightness"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingScreenBrightness"
		"xpos"				"16"
		"ypos"				"99"
		"wide"				"270"
		"tall"				"36"
		"ResourceFile"		"resource/ui/option_video_mode_slider.res"
		"navLeft"			"SettingScreenBrightness"
		"navRight"			"SettingTextureDetail"
		"navUp"				"SettingDisplayMode"
		"navDown"			"SettingRenderingPipeline"
	}

	"SettingRenderingPipeline"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingRenderingPipeline"
		"xpos"				"16"
		"ypos"				"136"
		"wide"				"270"
		"tall"				"36"
		"ResourceFile"		"resource/ui/option_video_mode.res"
		"navLeft"			"SettingRenderingPipeline"
		"navRight"			"SettingAntiAliasing"
		"navUp"				"SettingScreenBrightness"
		"navDown"			"SettingVSync"
	}

	"SettingVSync"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingVSync"
		"xpos"				"16"
		"ypos"				"173"
		"wide"				"270"
		"tall"				"36"
		"ResourceFile"		"resource/ui/option_video_mode.res"
		"navLeft"			"SettingVSync"
		"navRight"			"SettingFiltering"
		"navUp"				"SettingRenderingPipeline"
		"navDown"			"SettingFilmGrain"
	}

	"LblFaster"
	{
		"ControlName"		"Label"
		"fieldName"			"LblFaster"
		"xpos"				"356"
		"ypos"				"40"
		"wide"				"96"
		"tall"				"12"
		"labelText"			"#rd_video_faster"
		"textAlignment"		"west"
		"fgcolor_override"	"128 128 128 255"
	}

	"LblHigherQuality"
	{
		"ControlName"		"Label"
		"fieldName"			"LblHigherQuality"
		"xpos"				"498"
		"ypos"				"40"
		"wide"				"96"
		"tall"				"12"
		"labelText"			"#rd_video_higher_quality"
		"textAlignment"		"east"
		"fgcolor_override"	"128 128 128 255"
	}

	"SettingEffectDetail"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingEffectDetail"
		"xpos"				"292"
		"ypos"				"52"
		"wide"				"302"
		"tall"				"30"
		"ResourceFile"		"resource/ui/option_video_detail.res"
		"navLeft"			"SettingScreenResolution"
		"navRight"			"SettingEffectDetail"
		"navDown"			"SettingShaderDetail"
	}

	"SettingShaderDetail"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingShaderDetail"
		"xpos"				"292"
		"ypos"				"83"
		"wide"				"302"
		"tall"				"30"
		"ResourceFile"		"resource/ui/option_video_detail.res"
		"navLeft"			"SettingDisplayMode"
		"navRight"			"SettingShaderDetail"
		"navUp"				"SettingEffectDetail"
		"navDown"			"SettingTextureDetail"
	}

	"SettingTextureDetail"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingTextureDetail"
		"xpos"				"292"
		"ypos"				"115"
		"wide"				"302"
		"tall"				"30"
		"ResourceFile"		"resource/ui/option_video_detail.res"
		"navLeft"			"SettingScreenBrightness"
		"navRight"			"SettingTextureDetail"
		"navUp"				"SettingShaderDetail"
		"navDown"			"SettingAntiAliasing"
	}

	"SettingAntiAliasing"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingAntiAliasing"
		"xpos"				"292"
		"ypos"				"147"
		"wide"				"302"
		"tall"				"30"
		"ResourceFile"		"resource/ui/option_video_detail.res"
		"navLeft"			"SettingRenderingPipeline"
		"navRight"			"SettingAntiAliasing"
		"navUp"				"SettingTextureDetail"
		"navDown"			"SettingFiltering"
	}

	"SettingFiltering"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingFiltering"
		"xpos"				"292"
		"ypos"				"179"
		"wide"				"302"
		"tall"				"30"
		"ResourceFile"		"resource/ui/option_video_detail.res"
		"navLeft"			"SettingVSync"
		"navRight"			"SettingFiltering"
		"navUp"				"SettingAntiAliasing"
		"navDown"			"SettingHighQualityBeacons"
	}

	"SettingFilmGrain"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingFilmGrain"
		"xpos"				"16"
		"ypos"				"216"
		"wide"				"187"
		"tall"				"42"
		"ResourceFile"		"resource/ui/option_video_extra.res"
		"navLeft"			"SettingFilmGrain"
		"navRight"			"SettingBloomScale"
		"navUp"				"SettingVSync"
		"navDown"			"SettingLocalContrast"
	}

	"SettingLocalContrast"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingLocalContrast"
		"xpos"				"16"
		"ypos"				"260"
		"wide"				"187"
		"tall"				"42"
		"ResourceFile"		"resource/ui/option_video_extra.res"
		"navLeft"			"SettingLocalContrast"
		"navRight"			"SettingProjectedTextures"
		"navUp"				"SettingFilmGrain"
		"navDown"			"SettingDepthBlur"
	}

	"SettingDepthBlur"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingDepthBlur"
		"xpos"				"16"
		"ypos"				"304"
		"wide"				"187"
		"tall"				"42"
		"ResourceFile"		"resource/ui/option_video_extra.res"
		"navLeft"			"SettingDepthBlur"
		"navRight"			"SettingFlashlightShadows"
		"navUp"				"SettingLocalContrast"
		"navDown"			"SettingWeatherEffects"
	}

	"SettingWeatherEffects"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingWeatherEffects"
		"xpos"				"16"
		"ypos"				"348"
		"wide"				"187"
		"tall"				"42"
		"ResourceFile"		"resource/ui/option_video_extra.res"
		"navLeft"			"SettingWeatherEffects"
		"navRight"			"SettingFlashlightLightSpill"
		"navUp"				"SettingDepthBlur"
		"navDown"			"SettingWeatherEffects"
	}

	"SettingBloomScale"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingBloomScale"
		"xpos"				"211"
		"ypos"				"216"
		"wide"				"188"
		"tall"				"42"
		"ResourceFile"		"resource/ui/option_video_extra_slider.res"
		"navLeft"			"SettingFilmGrain"
		"navRight"			"SettingHighQualityBeacons"
		"navUp"				"SettingFiltering"
		"navDown"			"SettingProjectedTextures"
	}

	"SettingProjectedTextures"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingProjectedTextures"
		"xpos"				"211"
		"ypos"				"260"
		"wide"				"188"
		"tall"				"42"
		"ResourceFile"		"resource/ui/option_video_extra.res"
		"navLeft"			"SettingLocalContrast"
		"navRight"			"SettingMuzzleFlashLights"
		"navUp"				"SettingBloomScale"
		"navDown"			"SettingFlashlightShadows"
	}

	"SettingFlashlightShadows"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingFlashlightShadows"
		"xpos"				"211"
		"ypos"				"304"
		"wide"				"188"
		"tall"				"42"
		"ResourceFile"		"resource/ui/option_video_extra.res"
		"navLeft"			"SettingDepthBlur"
		"navRight"			"SettingAlienShadows"
		"navUp"				"SettingProjectedTextures"
		"navDown"			"SettingFlashlightLightSpill"
	}

	"SettingFlashlightLightSpill"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingFlashlightLightSpill"
		"xpos"				"211"
		"ypos"				"348"
		"wide"				"188"
		"tall"				"42"
		"ResourceFile"		"resource/ui/option_video_extra.res"
		"navLeft"			"SettingWeatherEffects"
		"navRight"			"SettingLowHealthEffect"
		"navUp"				"SettingFlashlightShadows"
		"navDown"			"SettingFlashlightLightSpill"
	}

	"SettingHighQualityBeacons"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingHighQualityBeacons"
		"xpos"				"407"
		"ypos"				"216"
		"wide"				"187"
		"tall"				"42"
		"ResourceFile"		"resource/ui/option_video_extra.res"
		"navLeft"			"SettingBloomScale"
		"navRight"			"SettingHighQualityBeacons"
		"navUp"				"SettingFiltering"
		"navDown"			"SettingMuzzleFlashLights"
	}

	"SettingMuzzleFlashLights"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingMuzzleFlashLights"
		"xpos"				"407"
		"ypos"				"260"
		"wide"				"187"
		"tall"				"42"
		"ResourceFile"		"resource/ui/option_video_extra.res"
		"navLeft"			"SettingProjectedTextures"
		"navRight"			"SettingMuzzleFlashLights"
		"navUp"				"SettingHighQualityBeacons"
		"navDown"			"SettingAlienShadows"
	}

	"SettingAlienShadows"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingAlienShadows"
		"xpos"				"407"
		"ypos"				"304"
		"wide"				"187"
		"tall"				"42"
		"ResourceFile"		"resource/ui/option_video_extra.res"
		"navLeft"			"SettingFlashlightShadows"
		"navRight"			"SettingAlienShadows"
		"navUp"				"SettingMuzzleFlashLights"
		"navDown"			"SettingLowHealthEffect"
	}

	"SettingLowHealthEffect"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingLowHealthEffect"
		"xpos"				"407"
		"ypos"				"348"
		"wide"				"187"
		"tall"				"42"
		"ResourceFile"		"resource/ui/option_video_extra.res"
		"navLeft"			"SettingFlashlightLightSpill"
		"navRight"			"SettingLowHealthEffect"
		"navUp"				"SettingAlienShadows"
		"navDown"			"SettingLowHealthEffect"
	}
}
