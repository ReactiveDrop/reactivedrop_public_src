"Resource/UI/BaseModUI/CRD_VGUI_Settings_Options_2.res"
{
	"SettingDamageNumbers"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingDamageNumbers"
		"xpos"				"16"
		"ypos"				"40"
		"wide"				"260"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"tabIndex"			"1"
		"ResourceFile"		"resource/ui/option_setting_dropdown.res"
		"navLeft"			"SettingDamageNumbers"
		"navRight"			"SettingAccessibilityTracerTintSelf"
		"navDown"			"SettingStrangeRankUp"
	}

	"SettingStrangeRankUp"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingStrangeRankUp"
		"wide"				"260"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"pin_to_sibling"	"SettingDamageNumbers"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"2"
		"ResourceFile"		"resource/ui/option_setting_dropdown.res"
		"navLeft"			"SettingStrangeRankUp"
		"navRight"			"SettingAccessibilityTracerTintOther"
		"navUp"				"SettingDamageNumbers"
		"navDown"			"SettingSpeedTimer"
	}

	"SettingSpeedTimer"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingSpeedTimer"
		"xpos"				"16"
		"ypos"				"72"
		"wide"				"260"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"ResourceFile"		"resource/ui/option_setting_checkbox.res"
		"navLeft"			"SettingSpeedTimer"
		"navRight"			"SettingAccessibilityTracerTintOther"
		"navUp"				"SettingStrangeRankUp"
		"navDown"			"SettingSpeedTimerColor"
	}

	"SettingSpeedTimerColor"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingSpeedTimerColor"
		"wide"				"260"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"pin_to_sibling"	"SettingSpeedTimer"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"2"
		"ResourceFile"		"resource/ui/option_setting_color.res"
		"navLeft"			"SettingSpeedTimerColor"
		"navRight"			"SettingAccessibilityHighlightActiveCharacter"
		"navUp"				"SettingSpeedTimer"
		"navDown"			"SettingSpeedObjectivesInChat"
	}

	"SettingSpeedObjectivesInChat"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingSpeedObjectivesInChat"
		"wide"				"260"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"pin_to_sibling"	"SettingSpeedTimerColor"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"2"
		"ResourceFile"		"resource/ui/option_setting_dropdown.res"
		"navLeft"			"SettingSpeedObjectivesInChat"
		"navRight"			"SettingAccessibilityReduceMotion"
		"navUp"				"SettingSpeedTimerColor"
		"navDown"			"SettingSpeedAutoRestartMission"
	}

	"SettingSpeedAutoRestartMission"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingSpeedAutoRestartMission"
		"wide"				"260"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"pin_to_sibling"	"SettingSpeedObjectivesInChat"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"2"
		"ResourceFile"		"resource/ui/option_setting_checkbox.res"
		"navLeft"			"SettingSpeedAutoRestartMission"
		"navRight"			"SettingAccessibilityCameraShake"
		"navUp"				"SettingSpeedObjectivesInChat"
		"navDown"			"SettingLeaderboardPrivateStats"
	}

	"SettingLeaderboardPrivateStats"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingLeaderboardPrivateStats"
		"xpos"				"16"
		"ypos"				"176"
		"wide"				"260"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"ResourceFile"		"resource/ui/option_setting_checkbox.res"
		"navLeft"			"SettingLeaderboardPrivateStats"
		"navRight"			"SettingAccessibilityMinimapClicks"
		"navUp"				"SettingSpeedAutoRestartMission"
		"navDown"			"SettingLeaderboardSend"
	}

	"SettingLeaderboardSend"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingLeaderboardSend"
		"wide"				"260"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"pin_to_sibling"	"SettingLeaderboardPrivateStats"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"2"
		"ResourceFile"		"resource/ui/option_setting_checkbox.res"
		"navLeft"			"SettingLeaderboardSend"
		"navRight"			"SettingAccessibilityMoveRelativeToAim"
		"navUp"				"SettingLeaderboardPrivateStats"
		"navDown"			"SettingLeaderboardLoading"
	}

	"SettingLeaderboardLoading"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingLeaderboardLoading"
		"wide"				"260"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"pin_to_sibling"	"SettingLeaderboardSend"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"2"
		"ResourceFile"		"resource/ui/option_setting_checkbox.res"
		"navLeft"			"SettingLeaderboardLoading"
		"navRight"			"SettingNetworkInterpolation"
		"navUp"				"SettingLeaderboardSend"
		"navDown"			"SettingLeaderboardDebrief"
	}

	"SettingLeaderboardDebrief"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingLeaderboardDebrief"
		"wide"				"260"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"pin_to_sibling"	"SettingLeaderboardLoading"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"2"
		"ResourceFile"		"resource/ui/option_setting_checkbox.res"
		"navLeft"			"SettingLeaderboardDebrief"
		"navRight"			"SettingNetworkRate"
		"navUp"				"SettingLeaderboardLoading"
		"navDown"			"SettingLoadingMissionIcons"
	}

	"SettingLoadingMissionIcons"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingLoadingMissionIcons"
		"xpos"				"16"
		"ypos"				"280"
		"wide"				"260"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"ResourceFile"		"resource/ui/option_setting_checkbox.res"
		"navLeft"			"SettingLoadingMissionIcons"
		"navRight"			"SettingNetworkAllowRelay"
		"navUp"				"SettingLeaderboardDebrief"
		"navDown"			"SettingLoadingMissionScreens"
	}

	"SettingLoadingMissionScreens"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingLoadingMissionScreens"
		"wide"				"260"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"pin_to_sibling"	"SettingLoadingMissionIcons"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"2"
		"ResourceFile"		"resource/ui/option_setting_checkbox.res"
		"navLeft"			"SettingLoadingMissionScreens"
		"navRight"			"SettingNetworkAllowRelay"
		"navUp"				"SettingLoadingMissionIcons"
		"navDown"			"SettingLoadingStatusText"
	}

	"SettingLoadingStatusText"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingLoadingStatusText"
		"wide"				"260"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"pin_to_sibling"	"SettingLoadingMissionScreens"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"2"
		"ResourceFile"		"resource/ui/option_setting_checkbox.res"
		"navLeft"			"SettingLoadingStatusText"
		"navRight"			"SettingNetworkAllowRelay"
		"navUp"				"SettingLoadingMissionScreens"
		"navDown"			"SettingLoadingStatusText"
	}

	"SettingAccessibilityTracerTintSelf"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingAccessibilityTracerTintSelf"
		"xpos"				"300"	[!$WIN32WIDE]
		"xpos"				"310"	[$WIN32WIDE]
		"ypos"				"40"
		"wide"				"260"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"ResourceFile"		"resource/ui/option_setting_color.res"
		"navLeft"			"SettingDamageNumbers"
		"navRight"			"SettingAccessibilityTracerTintSelf"
		"navDown"			"SettingAccessibilityTracerTintOther"
	}

	"SettingAccessibilityTracerTintOther"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingAccessibilityTracerTintOther"
		"wide"				"260"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"pin_to_sibling"	"SettingAccessibilityTracerTintSelf"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"2"
		"ResourceFile"		"resource/ui/option_setting_color.res"
		"navLeft"			"SettingSpeedTimer"
		"navRight"			"SettingAccessibilityTracerTintOther"
		"navUp"				"SettingAccessibilityTracerTintSelf"
		"navDown"			"SettingAccessibilityHighlightActiveCharacter"
	}

	"SettingAccessibilityHighlightActiveCharacter"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingAccessibilityHighlightActiveCharacter"
		"wide"				"260"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"pin_to_sibling"	"SettingAccessibilityTracerTintOther"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"2"
		"ResourceFile"		"resource/ui/option_setting_checkbox.res"
		"navLeft"			"SettingSpeedTimerColor"
		"navRight"			"SettingAccessibilityHighlightActiveCharacter"
		"navUp"				"SettingAccessibilityTracerTintOther"
		"navDown"			"SettingAccessibilityReduceMotion"
	}

	"SettingAccessibilityReduceMotion"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingAccessibilityReduceMotion"
		"wide"				"260"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"pin_to_sibling"	"SettingAccessibilityHighlightActiveCharacter"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"2"
		"ResourceFile"		"resource/ui/option_setting_checkbox.res"
		"navLeft"			"SettingSpeedObjectivesInChat"
		"navRight"			"SettingAccessibilityReduceMotion"
		"navUp"				"SettingAccessibilityHighlightActiveCharacter"
		"navDown"			"SettingAccessibilityCameraShake"
	}

	"SettingAccessibilityCameraShake"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingAccessibilityCameraShake"
		"wide"				"260"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"pin_to_sibling"	"SettingAccessibilityReduceMotion"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"2"
		"ResourceFile"		"resource/ui/option_setting_dropdown.res"
		"navLeft"			"SettingSpeedAutoRestartMission"
		"navRight"			"SettingAccessibilityCameraShake"
		"navUp"				"SettingAccessibilityReduceMotion"
		"navDown"			"SettingAccessibilityCameraShift"
	}

	"SettingAccessibilityCameraShift"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingAccessibilityCameraShift"
		"wide"				"260"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"pin_to_sibling"	"SettingAccessibilityCameraShake"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"2"
		"ResourceFile"		"resource/ui/option_setting_checkbox.res"
		"navLeft"			"SettingSpeedAutoRestartMission"
		"navRight"			"SettingAccessibilityCameraShift"
		"navUp"				"SettingAccessibilityCameraShake"
		"navDown"			"SettingAccessibilityMinimapClicks"
	}

	"SettingAccessibilityMinimapClicks"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingAccessibilityMinimapClicks"
		"wide"				"260"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"pin_to_sibling"	"SettingAccessibilityCameraShift"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"2"
		"ResourceFile"		"resource/ui/option_setting_checkbox.res"
		"navLeft"			"SettingLeaderboardPrivateStats"
		"navRight"			"SettingAccessibilityMinimapClicks"
		"navUp"				"SettingAccessibilityCameraShift"
		"navDown"			"SettingAccessibilityMoveRelativeToAim"
	}

	"SettingAccessibilityMoveRelativeToAim"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingAccessibilityMoveRelativeToAim"
		"wide"				"260"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"pin_to_sibling"	"SettingAccessibilityMinimapClicks"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"2"
		"ResourceFile"		"resource/ui/option_setting_checkbox.res"
		"navLeft"			"SettingLeaderboardSend"
		"navRight"			"SettingAccessibilityMoveRelativeToAim"
		"navUp"				"SettingAccessibilityMinimapClicks"
		"navDown"			"SettingNetworkInterpolation"
	}

	"SettingNetworkInterpolation"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingNetworkInterpolation"
		"xpos"				"300"	[!$WIN32WIDE]
		"xpos"				"310"	[$WIN32WIDE]
		"ypos"				"240"
		"wide"				"260"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"ResourceFile"		"resource/ui/option_setting_slider.res"
		"displayMultiplier"	"1000"
		"decimalDigits"		"0"
		"displaySuffix"		"#rd_option_suffix_milliseconds"
		"navLeft"			"SettingLeaderboardLoading"
		"navRight"			"SettingNetworkInterpolation"
		"navUp"				"SettingAccessibilityMoveRelativeToAim"
		"navDown"			"SettingNetworkRate"
	}

	"SettingNetworkRate"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingNetworkRate"
		"wide"				"260"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"pin_to_sibling"	"SettingNetworkInterpolation"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"2"
		"ResourceFile"		"resource/ui/option_setting_slider.res"
		"displayMultiplier"	"0.0078125"
		"navLeft"			"SettingLeaderboardDebrief"
		"navRight"			"SettingNetworkRate"
		"navUp"				"SettingNetworkInterpolation"
		"navDown"			"SettingNetworkAllowRelay"
	}

	"SettingNetworkAllowRelay"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingNetworkAllowRelay"
		"wide"				"260"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"pin_to_sibling"	"SettingNetworkRate"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"2"
		"ResourceFile"		"resource/ui/option_setting_dropdown.res"
		"navLeft"			"SettingLoadingMissionIcons"
		"navRight"			"SettingNetworkAllowRelay"
		"navUp"				"SettingNetworkRate"
		"navDown"			"SettingNetworkAllowRelay"
	}
}
