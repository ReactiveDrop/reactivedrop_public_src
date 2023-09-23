"Resource/UI/BaseModUI/CRD_VGUI_Settings_Options_1.res"
{
	"SettingPlayerNameMode"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingPlayerNameMode"
		"xpos"				"16"
		"ypos"				"40"
		"wide"				"260"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"tabIndex"			"1"
		"ResourceFile"		"resource/ui/option_setting_dropdown.res"
		"navLeft"			"SettingPlayerNameMode"
		"navRight"			"SettingControlsRightClickWireHack"
		"navDown"			"SettingPlayerChatColor"
	}

	"SettingPlayerChatColor"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingPlayerChatColor"
		"wide"				"260"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"pin_to_sibling"	"SettingPlayerNameMode"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"2"
		"ResourceFile"		"resource/ui/option_setting_color.res"
		"navLeft"			"SettingPlayerChatColor"
		"navRight"			"SettingControlsSniperSwapWeapons"
		"navUp"				"SettingPlayerNameMode"
		"navDown"			"SettingPlayerChatNamesUseColors"
	}

	"SettingPlayerChatNamesUseColors"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingPlayerChatNamesUseColors"
		"wide"				"260"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"pin_to_sibling"	"SettingPlayerChatColor"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"2"
		"ResourceFile"		"resource/ui/option_setting_checkbox.res"
		"navLeft"			"SettingPlayerChatNamesUseColors"
		"navRight"			"SettingControlsLockMouseToWindow"
		"navUp"				"SettingPlayerChatColor"
		"navDown"			"SettingPlayerDeathmatchTeamColorMode"
	}

	"SettingPlayerDeathmatchTeamColorMode"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingPlayerDeathmatchTeamColorMode"
		"wide"				"260"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"pin_to_sibling"	"SettingPlayerChatNamesUseColors"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"2"
		"ResourceFile"		"resource/ui/option_setting_dropdown.res"
		"navLeft"			"SettingPlayerDeathmatchTeamColorMode"
		"navRight"			"SettingControlsHorizontalAutoAim"
		"navUp"				"SettingPlayerChatNamesUseColors"
		"navDown"			"SettingPlayerDeathmatchDrawTopScoreboard"
	}

	"SettingPlayerDeathmatchDrawTopScoreboard"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingPlayerDeathmatchDrawTopScoreboard"
		"wide"				"260"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"pin_to_sibling"	"SettingPlayerDeathmatchTeamColorMode"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"2"
		"ResourceFile"		"resource/ui/option_setting_checkbox.res"
		"navLeft"			"SettingPlayerDeathmatchDrawTopScoreboard"
		"navRight"			"SettingCrosshairMarineLabelDist"
		"navUp"				"SettingPlayerDeathmatchTeamColorMode"
		"navDown"			"SettingHintsFailAdvice"
	}

	"SettingHintsFailAdvice"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingHintsFailAdvice"
		"xpos"				"16"
		"ypos"				"168"
		"wide"				"260"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"ResourceFile"		"resource/ui/option_setting_checkbox.res"
		"navLeft"			"SettingHintsFailAdvice"
		"navRight"			"SettingCrosshairType"
		"navUp"				"SettingPlayerDeathmatchDrawTopScoreboard"
		"navDown"			"SettingHintsGameInstructor"
	}

	"SettingHintsGameInstructor"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingHintsGameInstructor"
		"wide"				"260"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"pin_to_sibling"	"SettingHintsFailAdvice"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"2"
		"ResourceFile"		"resource/ui/option_setting_checkbox.res"
		"navLeft"			"SettingHintsGameInstructor"
		"navRight"			"SettingCrosshairSize"
		"navUp"				"SettingHintsFailAdvice"
		"navDown"			"BtnResetGameInstructor"
	}

	"BtnResetGameInstructor"
	{
		"ControlName"		"BaseModHybridButton"
		"fieldName"			"BtnResetGameInstructor"
		"wide"				"260"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"18"
		"pin_to_sibling"	"SettingHintsGameInstructor"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"2"
		"navLeft"			"BtnResetGameInstructor"
		"navRight"			"SettingCrosshairLaserSight"
		"navUp"				"SettingHintsGameInstructor"
		"navDown"			"SettingHintsDeathmatchRespawn"
	}

	"SettingHintsDeathmatchRespawn"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingHintsDeathmatchRespawn"
		"wide"				"260"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"pin_to_sibling"	"BtnResetGameInstructor"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"2"
		"ResourceFile"		"resource/ui/option_setting_checkbox.res"
		"navLeft"			"SettingHintsDeathmatchRespawn"
		"navRight"			"SettingReloadAuto"
		"navUp"				"BtnResetGameInstructor"
		"navDown"			"SettingHintsSwarmopediaGrid"
	}

	"SettingHintsSwarmopediaGrid"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingHintsSwarmopediaGrid"
		"wide"				"260"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"pin_to_sibling"	"SettingHintsDeathmatchRespawn"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"2"
		"ResourceFile"		"resource/ui/option_setting_checkbox.res"
		"navLeft"			"SettingHintsSwarmopediaGrid"
		"navRight"			"SettingReloadAuto"
		"navUp"				"SettingHintsDeathmatchRespawn"
		"navDown"			"SettingHintsSwarmopediaUnits"
	}

	"SettingHintsSwarmopediaUnits"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingHintsSwarmopediaUnits"
		"wide"				"260"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"pin_to_sibling"	"SettingHintsSwarmopediaGrid"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"2"
		"ResourceFile"		"resource/ui/option_setting_dropdown.res"
		"navLeft"			"SettingHintsSwarmopediaUnits"
		"navRight"			"SettingReloadFastUnderMarine"
		"navUp"				"SettingHintsSwarmopediaGrid"
		"navDown"			"SettingDeathCamTakeover"
	}

	"SettingDeathCamTakeover"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingDeathCamTakeover"
		"xpos"				"16"
		"ypos"				"314"
		"wide"				"260"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"ResourceFile"		"resource/ui/option_setting_checkbox.res"
		"navLeft"			"SettingDeathCamTakeover"
		"navRight"			"SettingReloadFastTall"
		"navUp"				"SettingHintsSwarmopediaUnits"
		"navDown"			"SettingDeathCamSlowdown"
	}

	"SettingDeathCamSlowdown"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingDeathCamSlowdown"
		"wide"				"260"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"pin_to_sibling"	"SettingDeathCamTakeover"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"2"
		"ResourceFile"		"resource/ui/option_setting_checkbox.res"
		"navLeft"			"SettingDeathCamSlowdown"
		"navRight"			"SettingReloadFastTall"
		"navUp"				"SettingDeathCamTakeover"
		"navDown"			"SettingDeathMarineGibs"
	}

	"SettingDeathMarineGibs"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingDeathMarineGibs"
		"wide"				"260"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"pin_to_sibling"	"SettingDeathCamSlowdown"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"2"
		"ResourceFile"		"resource/ui/option_setting_checkbox.res"
		"navLeft"			"SettingDeathMarineGibs"
		"navRight"			"SettingReloadFastTall"
		"navUp"				"SettingDeathCamSlowdown"
		"navDown"			"SettingDeathMarineGibs"
	}

	"SettingControlsRightClickWireHack"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingControlsRightClickWireHack"
		"xpos"				"300"	[!$WIN32WIDE]
		"xpos"				"310"	[$WIN32WIDE]
		"ypos"				"40"
		"wide"				"260"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"ResourceFile"		"resource/ui/option_setting_dropdown.res"
		"navLeft"			"SettingPlayerNameMode"
		"navRight"			"SettingControlsRightClickWireHack"
		"navDown"			"SettingControlsSniperSwapWeapons"
	}

	"SettingControlsSniperSwapWeapons"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingControlsSniperSwapWeapons"
		"wide"				"260"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"pin_to_sibling"	"SettingControlsRightClickWireHack"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"2"
		"ResourceFile"		"resource/ui/option_setting_checkbox.res"
		"navLeft"			"SettingPlayerChatColor"
		"navRight"			"SettingControlsSniperSwapWeapons"
		"navUp"				"SettingControlsRightClickWireHack"
		"navDown"			"SettingControlsLockMouseToWindow"
	}

	"SettingControlsLockMouseToWindow"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingControlsLockMouseToWindow"
		"wide"				"260"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"pin_to_sibling"	"SettingControlsSniperSwapWeapons"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"2"
		"ResourceFile"		"resource/ui/option_setting_checkbox.res"
		"navLeft"			"SettingPlayerChatNamesUseColors"
		"navRight"			"SettingControlsLockMouseToWindow"
		"navUp"				"SettingControlsSniperSwapWeapons"
		"navDown"			"SettingCrosshairMarineLabelDist"
	}

	"SettingCrosshairMarineLabelDist"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingCrosshairMarineLabelDist"
		"xpos"				"300"	[!$WIN32WIDE]
		"xpos"				"310"	[$WIN32WIDE]
		"ypos"				"120"
		"wide"				"260"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"ResourceFile"		"resource/ui/option_setting_slider.res"
		"navLeft"			"SettingPlayerDeathmatchDrawTopScoreboard"
		"navRight"			"SettingCrosshairMarineLabelDist"
		"navUp"				"SettingControlsLockMouseToWindow"
		"navDown"			"SettingCrosshairType"
	}

	"SettingCrosshairType"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingCrosshairType"
		"wide"				"260"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"pin_to_sibling"	"SettingCrosshairMarineLabelDist"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"2"
		"ResourceFile"		"resource/ui/option_setting_dropdown.res"
		"navLeft"			"SettingHintsFailAdvice"
		"navRight"			"SettingCrosshairType"
		"navUp"				"SettingCrosshairMarineLabelDist"
		"navDown"			"SettingCrosshairSize"
	}

	"SettingCrosshairSize"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingCrosshairSize"
		"wide"				"260"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"pin_to_sibling"	"SettingCrosshairType"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"2"
		"ResourceFile"		"resource/ui/option_setting_slider.res"
		"navLeft"			"SettingHintsGameInstructor"
		"navRight"			"SettingCrosshairSize"
		"navUp"				"SettingCrosshairType"
		"navDown"			"SettingCrosshairLaserSight"
	}

	"SettingCrosshairLaserSight"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingCrosshairLaserSight"
		"wide"				"260"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"pin_to_sibling"	"SettingCrosshairSize"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"2"
		"ResourceFile"		"resource/ui/option_setting_checkbox.res"
		"navLeft"			"BtnResetGameInstructor"
		"navRight"			"SettingCrosshairLaserSight"
		"navUp"				"SettingCrosshairSize"
		"navDown"			"SettingReloadAuto"
	}

	"SettingReloadAuto"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingReloadAuto"
		"xpos"				"300"	[!$WIN32WIDE]
		"xpos"				"310"	[$WIN32WIDE]
		"ypos"				"224"
		"wide"				"260"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"ResourceFile"		"resource/ui/option_setting_checkbox.res"
		"navLeft"			"SettingHintsSwarmopediaGrid"
		"navRight"			"SettingReloadAuto"
		"navUp"				"SettingCrosshairLaserSight"
		"navDown"			"SettingReloadFastUnderMarine"
	}

	"SettingReloadFastUnderMarine"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingReloadFastUnderMarine"
		"wide"				"260"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"pin_to_sibling"	"SettingReloadAuto"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"2"
		"ResourceFile"		"resource/ui/option_setting_checkbox.res"
		"navLeft"			"SettingHintsSwarmopediaUnits"
		"navRight"			"SettingReloadFastUnderMarine"
		"navUp"				"SettingReloadAuto"
		"navDown"			"SettingReloadFastWide"
	}

	"SettingReloadFastWide"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingReloadFastWide"
		"wide"				"260"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"pin_to_sibling"	"SettingReloadFastUnderMarine"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"2"
		"ResourceFile"		"resource/ui/option_setting_slider.res"
		"navLeft"			"SettingHintsSwarmopediaUnits"
		"navRight"			"SettingReloadFastWide"
		"navUp"				"SettingReloadFastUnderMarine"
		"navDown"			"SettingReloadFastTall"
	}

	"SettingReloadFastTall"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingReloadFastTall"
		"wide"				"260"	[!$WIN32WIDE]
		"wide"				"270"	[$WIN32WIDE]
		"tall"				"24"
		"pin_to_sibling"	"SettingReloadFastWide"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"2"
		"ResourceFile"		"resource/ui/option_setting_slider.res"
		"navLeft"			"SettingDeathCamTakeover"
		"navRight"			"SettingReloadFastTall"
		"navUp"				"SettingReloadFastWide"
		"navDown"			"SettingReloadFastTall"
	}
}
