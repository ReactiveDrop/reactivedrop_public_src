"Resource/UI/BaseModUI/CRD_VGUI_Settings_Controls.res"
{
	"BindMoveForward"
	{
		"ControlName"		"CRD_VGUI_Bind"
		"fieldName"			"BindMoveForward"
		"xpos"				"50"
		"ypos"				"50"
		"wide"				"32"
		"tall"				"32"
		"tabPosition"		"1"
		"navLeft"			"BindMoveForward"
		"navRight"			"BindWalk"
		"navDown"			"BindMoveBack"
	}

	"BindMoveLeft"
	{
		"ControlName"		"CRD_VGUI_Bind"
		"fieldName"			"BindMoveLeft"
		"xpos"				"16"
		"ypos"				"84"
		"wide"				"32"
		"tall"				"32"
		"navLeft"			"BindMoveLeft"
		"navRight"			"BindMoveBack"
		"navUp"				"BindMoveForward"
		"navDown"			"BindTextChat"
	}

	"BindMoveBack"
	{
		"ControlName"		"CRD_VGUI_Bind"
		"fieldName"			"BindMoveBack"
		"xpos"				"50"
		"ypos"				"84"
		"wide"				"32"
		"tall"				"32"
		"navLeft"			"BindMoveLeft"
		"navRight"			"BindMoveRight"
		"navUp"				"BindMoveForward"
		"navDown"			"BindTextChat"
	}

	"BindMoveRight"
	{
		"ControlName"		"CRD_VGUI_Bind"
		"fieldName"			"BindMoveRight"
		"xpos"				"84"
		"ypos"				"84"
		"wide"				"32"
		"tall"				"32"
		"navLeft"			"BindMoveBack"
		"navRight"			"BindJump"
		"navUp"				"BindMoveForward"
		"navDown"			"BindTextChat"
	}

	"BindWalk"
	{
		"ControlName"		"CRD_VGUI_Bind"
		"fieldName"			"BindWalk"
		"xpos"				"126"
		"ypos"				"50"
		"wide"				"32"
		"tall"				"32"
		"navLeft"			"BindMoveForward"
		"navRight"			"SettingAutoWalk"
		"navDown"			"BindJump"
	}

	"BindJump"
	{
		"ControlName"		"CRD_VGUI_Bind"
		"fieldName"			"BindJump"
		"xpos"				"126"
		"ypos"				"84"
		"wide"				"32"
		"tall"				"32"
		"navLeft"			"BindMoveRight"
		"navRight"			"SettingControllerGlyphs"
		"navUp"				"BindWalk"
		"navDown"			"BindTextChat"
	}

	"LblLeftStickAction"
	{
		"ControlName"		"Label"
		"fieldName"			"LblLeftStickAction"
		"xpos"				"16"
		"ypos"				"52"
		"wide"				"32"
		"tall"				"28"
		"textAlignment"		"south"
		"font"				"DefaultVerySmall"
		"fgcolor_override"	"128 128 128 255"
	}

	"LblRightStickAction"
	{
		"ControlName"		"Label"
		"fieldName"			"LblRightStickAction"
		"xpos"				"84"
		"ypos"				"52"
		"wide"				"32"
		"tall"				"28"
		"textAlignment"		"south"
		"font"				"DefaultVerySmall"
		"fgcolor_override"	"128 128 128 255"
	}

	"SettingAutoWalk"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingAutoWalk"
		"xpos"				"167"	[!$WIN32WIDE]
		"xpos"				"182"	[$WIN32WIDE]
		"ypos"				"50"
		"wide"				"220"
		"tall"				"12"
		"ResourceFile"		"resource/ui/option_simple_checkbox.res"
		"navLeft"			"BindWalk"
		"navRight"			"BindPrimaryAttack"
		"navDown"			"SettingAutoAttack"
	}

	"SettingAutoAttack"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingAutoAttack"
		"xpos"				"167"	[!$WIN32WIDE]
		"xpos"				"182"	[$WIN32WIDE]
		"ypos"				"62"
		"wide"				"220"
		"tall"				"12"
		"ResourceFile"		"resource/ui/option_simple_checkbox.res"
		"navLeft"			"BindWalk"
		"navRight"			"BindPrimaryAttack"
		"navUp"				"SettingAutoWalk"
		"navDown"			"SettingAimToMovement"
	}

	"SettingAimToMovement"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingAimToMovement"
		"xpos"				"167"	[!$WIN32WIDE]
		"xpos"				"182"	[$WIN32WIDE]
		"ypos"				"74"
		"wide"				"220"
		"tall"				"12"
		"ResourceFile"		"resource/ui/option_simple_checkbox.res"
		"navLeft"			"BindJump"
		"navRight"			"BindSwapWeapons"
		"navUp"				"SettingAutoAttack"
		"navDown"			"SettingControllerGlyphs"
	}

	"SettingControllerGlyphs"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingControllerGlyphs"
		"xpos"				"167"	[!$WIN32WIDE]
		"xpos"				"182"	[$WIN32WIDE]
		"ypos"				"86"
		"wide"				"220"
		"tall"				"12"
		"ResourceFile"		"resource/ui/option_simple_dropdown.res"
		"navLeft"			"BindJump"
		"navRight"			"BindSwapWeapons"
		"navUp"				"SettingAimToMovement"
		"navDown"			"BindActivatePrimary"
	}

	"BindPrimaryAttack"
	{
		"ControlName"		"CRD_VGUI_Bind"
		"fieldName"			"BindPrimaryAttack"
		"xpos"				"396"	[!$WIN32WIDE]
		"xpos"				"426"	[$WIN32WIDE]
		"ypos"				"50"
		"wide"				"32"
		"tall"				"32"
		"navLeft"			"SettingAutoWalk"
		"navRight"			"BindSecondaryAttack"
		"navDown"			"BindSwapWeapons"
	}

	"BindSecondaryAttack"
	{
		"ControlName"		"CRD_VGUI_Bind"
		"fieldName"			"BindSecondaryAttack"
		"xpos"				"430"	[!$WIN32WIDE]
		"xpos"				"460"	[$WIN32WIDE]
		"ypos"				"50"
		"wide"				"32"
		"tall"				"32"
		"navLeft"			"BindPrimaryAttack"
		"navRight"			"BindUse"
		"navDown"			"BindMeleeAttack"
	}

	"BindUse"
	{
		"ControlName"		"CRD_VGUI_Bind"
		"fieldName"			"BindUse"
		"xpos"				"464"	[!$WIN32WIDE]
		"xpos"				"494"	[$WIN32WIDE]
		"ypos"				"50"
		"wide"				"32"
		"tall"				"32"
		"navLeft"			"BindSecondaryAttack"
		"navRight"			"BindSelectPrimary"
		"navDown"			"BindReload"
	}

	"BindSelectPrimary"
	{
		"ControlName"		"CRD_VGUI_Bind"
		"fieldName"			"BindSelectPrimary"
		"xpos"				"498"	[!$WIN32WIDE]
		"xpos"				"528"	[$WIN32WIDE]
		"ypos"				"50"
		"wide"				"32"
		"tall"				"32"
		"navLeft"			"BindUse"
		"navRight"			"BindDropWeapon"
		"navDown"			"BindSelectSecondary"
	}

	"BindDropWeapon"
	{
		"ControlName"		"CRD_VGUI_Bind"
		"fieldName"			"BindDropWeapon"
		"xpos"				"532"	[!$WIN32WIDE]
		"xpos"				"562"	[$WIN32WIDE]
		"ypos"				"50"
		"wide"				"32"
		"tall"				"32"
		"navLeft"			"BindSelectPrimary"
		"navRight"			"BindDropWeapon"
		"navDown"			"BindDropEquipment"
	}

	"BindSwapWeapons"
	{
		"ControlName"		"CRD_VGUI_Bind"
		"fieldName"			"BindSwapWeapons"
		"xpos"				"396"	[!$WIN32WIDE]
		"xpos"				"426"	[$WIN32WIDE]
		"ypos"				"84"
		"wide"				"32"
		"tall"				"32"
		"navLeft"			"SettingControllerGlyphs"
		"navRight"			"BindMeleeAttack"
		"navUp"				"BindPrimaryAttack"
		"navDown"			"BindSelectMarine0"
	}

	"BindMeleeAttack"
	{
		"ControlName"		"CRD_VGUI_Bind"
		"fieldName"			"BindMeleeAttack"
		"xpos"				"430"	[!$WIN32WIDE]
		"xpos"				"460"	[$WIN32WIDE]
		"ypos"				"84"
		"wide"				"32"
		"tall"				"32"
		"navLeft"			"BindSwapWeapons"
		"navRight"			"BindReload"
		"navUp"				"BindSecondaryAttack"
		"navDown"			"BindSelectMarine0"
	}

	"BindReload"
	{
		"ControlName"		"CRD_VGUI_Bind"
		"fieldName"			"BindReload"
		"xpos"				"464"	[!$WIN32WIDE]
		"xpos"				"494"	[$WIN32WIDE]
		"ypos"				"84"
		"wide"				"32"
		"tall"				"32"
		"navLeft"			"BindMeleeAttack"
		"navRight"			"BindSelectSecondary"
		"navUp"				"BindUse"
		"navDown"			"BindSelectMarine0"
	}

	"BindSelectSecondary"
	{
		"ControlName"		"CRD_VGUI_Bind"
		"fieldName"			"BindSelectSecondary"
		"xpos"				"498"	[!$WIN32WIDE]
		"xpos"				"528"	[$WIN32WIDE]
		"ypos"				"84"
		"wide"				"32"
		"tall"				"32"
		"navLeft"			"BindReload"
		"navRight"			"BindDropEquipment"
		"navUp"				"BindSelectPrimary"
		"navDown"			"BindSelectMarine0"
	}

	"BindDropEquipment"
	{
		"ControlName"		"CRD_VGUI_Bind"
		"fieldName"			"BindDropEquipment"
		"xpos"				"532"	[!$WIN32WIDE]
		"xpos"				"562"	[$WIN32WIDE]
		"ypos"				"84"
		"wide"				"32"
		"tall"				"32"
		"navLeft"			"BindSelectSecondary"
		"navRight"			"BindDropEquipment"
		"navUp"				"BindDropWeapon"
		"navDown"			"BindSelectMarine0"
	}

	"BindTextChat"
	{
		"ControlName"		"CRD_VGUI_Bind"
		"fieldName"			"BindTextChat"
		"xpos"				"16"
		"ypos"				"148"
		"wide"				"176"
		"tall"				"12"
		"navLeft"			"BindTextChat"
		"navRight"			"BindActivatePrimary"
		"navUp"				"BindMoveBack"
		"navDown"			"BindVoiceChat"
	}

	"BindVoiceChat"
	{
		"ControlName"		"CRD_VGUI_Bind"
		"fieldName"			"BindVoiceChat"
		"xpos"				"16"
		"ypos"				"160"
		"wide"				"176"
		"tall"				"12"
		"navLeft"			"BindVoiceChat"
		"navRight"			"BindActivateSecondary"
		"navUp"				"BindTextChat"
		"navDown"			"BindWheelDefault"
	}

	"BindWheelDefault"
	{
		"ControlName"		"CRD_VGUI_Bind"
		"fieldName"			"BindWheelDefault"
		"xpos"				"16"
		"ypos"				"172"
		"wide"				"176"
		"tall"				"12"
		"navLeft"			"BindWheelDefault"
		"navRight"			"BindActivateEquipment0"
		"navUp"				"BindVoiceChat"
		"navDown"			"BindEmoteGo"
	}

	"BindEmoteGo"
	{
		"ControlName"		"CRD_VGUI_Bind"
		"fieldName"			"BindEmoteGo"
		"xpos"				"16"
		"ypos"				"184"
		"wide"				"176"
		"tall"				"12"
		"navLeft"			"BindEmoteGo"
		"navRight"			"BindActivateEquipment1"
		"navUp"				"BindWheelDefault"
		"navDown"			"BindEmoteStop"
	}

	"BindEmoteStop"
	{
		"ControlName"		"CRD_VGUI_Bind"
		"fieldName"			"BindEmoteStop"
		"xpos"				"16"
		"ypos"				"196"
		"wide"				"176"
		"tall"				"12"
		"navLeft"			"BindEmoteStop"
		"navRight"			"BindActivateEquipment2"
		"navUp"				"BindEmoteGo"
		"navDown"			"BindMarinePosition"
	}

	"BindMarinePosition"
	{
		"ControlName"		"CRD_VGUI_Bind"
		"fieldName"			"BindMarinePosition"
		"xpos"				"16"
		"ypos"				"208"
		"wide"				"176"
		"tall"				"12"
		"navLeft"			"BindMarinePosition"
		"navRight"			"BindActivateEquipment3"
		"navUp"				"BindEmoteStop"
		"navDown"			"BindEmoteMedic"
	}

	"BindEmoteMedic"
	{
		"ControlName"		"CRD_VGUI_Bind"
		"fieldName"			"BindEmoteMedic"
		"xpos"				"16"
		"ypos"				"220"
		"wide"				"176"
		"tall"				"12"
		"navLeft"			"BindEmoteMedic"
		"navRight"			"BindActivateEquipment4"
		"navUp"				"BindMarinePosition"
		"navDown"			"BindEmoteAmmo"
	}

	"BindEmoteAmmo"
	{
		"ControlName"		"CRD_VGUI_Bind"
		"fieldName"			"BindEmoteAmmo"
		"xpos"				"16"
		"ypos"				"232"
		"wide"				"176"
		"tall"				"12"
		"navLeft"			"BindEmoteAmmo"
		"navRight"			"BindActivateEquipment5"
		"navUp"				"BindEmoteMedic"
		"navDown"			"BindEmoteQuestion"
	}

	"BindEmoteQuestion"
	{
		"ControlName"		"CRD_VGUI_Bind"
		"fieldName"			"BindEmoteQuestion"
		"xpos"				"16"
		"ypos"				"244"
		"wide"				"176"
		"tall"				"12"
		"navLeft"			"BindEmoteQuestion"
		"navRight"			"BindActivateEquipment6"
		"navUp"				"BindEmoteAmmo"
		"navDown"			"BindEmoteExclaim"
	}

	"BindEmoteExclaim"
	{
		"ControlName"		"CRD_VGUI_Bind"
		"fieldName"			"BindEmoteExclaim"
		"xpos"				"16"
		"ypos"				"256"
		"wide"				"176"
		"tall"				"12"
		"navLeft"			"BindEmoteExclaim"
		"navRight"			"BindActivateEquipment7"
		"navUp"				"BindEmoteQuestion"
		"navDown"			"BindVoteYes"
	}

	"BindVoteYes"
	{
		"ControlName"		"CRD_VGUI_Bind"
		"fieldName"			"BindVoteYes"
		"xpos"				"16"
		"ypos"				"268"
		"wide"				"176"
		"tall"				"12"
		"navLeft"			"BindVoteYes"
		"navRight"			"BindWheelEquipment"
		"navUp"				"BindEmoteExclaim"
		"navDown"			"BindVoteNo"
	}

	"BindVoteNo"
	{
		"ControlName"		"CRD_VGUI_Bind"
		"fieldName"			"BindVoteNo"
		"xpos"				"16"
		"ypos"				"280"
		"wide"				"176"
		"tall"				"12"
		"navLeft"			"BindVoteNo"
		"navRight"			"BindWheelEquipment1"
		"navUp"				"BindVoteYes"
		"navDown"			"BindMissionOverview"
	}

	"BindMissionOverview"
	{
		"ControlName"		"CRD_VGUI_Bind"
		"fieldName"			"BindMissionOverview"
		"xpos"				"16"
		"ypos"				"292"
		"wide"				"176"
		"tall"				"12"
		"navLeft"			"BindMissionOverview"
		"navRight"			"BindWheelEquipment2"
		"navUp"				"BindVoteNo"
		"navDown"			"BindPlayerList"
	}

	"BindPlayerList"
	{
		"ControlName"		"CRD_VGUI_Bind"
		"fieldName"			"BindPlayerList"
		"xpos"				"16"
		"ypos"				"304"
		"wide"				"176"
		"tall"				"12"
		"navLeft"			"BindPlayerList"
		"navRight"			"BindWheelEquipment2"
		"navUp"				"BindMissionOverview"
		"navDown"			"BindRotateCameraLeft"
	}

	"BindRotateCameraLeft"
	{
		"ControlName"		"CRD_VGUI_Bind"
		"fieldName"			"BindRotateCameraLeft"
		"xpos"				"16"
		"ypos"				"316"
		"wide"				"176"
		"tall"				"12"
		"navLeft"			"BindRotateCameraLeft"
		"navRight"			"BindWheelEquipment2"
		"navUp"				"BindPlayerList"
		"navDown"			"BindRotateCameraRight"
	}

	"BindRotateCameraRight"
	{
		"ControlName"		"CRD_VGUI_Bind"
		"fieldName"			"BindRotateCameraRight"
		"xpos"				"16"
		"ypos"				"328"
		"wide"				"176"
		"tall"				"12"
		"navLeft"			"BindRotateCameraRight"
		"navRight"			"BindWheelEquipment2"
		"navUp"				"BindRotateCameraLeft"
		"navDown"			"BindSecondaryAttackAlt"
	}

	"BindSecondaryAttackAlt"
	{
		"ControlName"		"CRD_VGUI_Bind"
		"fieldName"			"BindSecondaryAttackAlt"
		"xpos"				"16"
		"ypos"				"340"
		"wide"				"176"
		"tall"				"12"
		"navLeft"			"BindSecondaryAttackAlt"
		"navRight"			"BindWheelEquipment2"
		"navUp"				"BindRotateCameraRight"
		"navDown"			"BindChooseMarine"
	}

	"BindChooseMarine"
	{
		"ControlName"		"CRD_VGUI_Bind"
		"fieldName"			"BindChooseMarine"
		"xpos"				"16"
		"ypos"				"352"
		"wide"				"176"
		"tall"				"12"
		"navLeft"			"BindChooseMarine"
		"navRight"			"BindWheelEquipment2"
		"navUp"				"BindSecondaryAttackAlt"
		"navDown"			"BtnResetDefaults"
	}

	"BindActivatePrimary"
	{
		"ControlName"		"CRD_VGUI_Bind"
		"fieldName"			"BindActivatePrimary"
		"xpos"				"202"	[!$WIN32WIDE]
		"xpos"				"217"	[$WIN32WIDE]
		"ypos"				"148"
		"wide"				"176"
		"tall"				"12"
		"navLeft"			"BindTextChat"
		"navRight"			"BindSelectMarine0"
		"navUp"				"SettingControllerGlyphs"
		"navDown"			"BindActivateSecondary"
	}

	"BindActivateSecondary"
	{
		"ControlName"		"CRD_VGUI_Bind"
		"fieldName"			"BindActivateSecondary"
		"xpos"				"202"	[!$WIN32WIDE]
		"xpos"				"217"	[$WIN32WIDE]
		"ypos"				"160"
		"wide"				"176"
		"tall"				"12"
		"navLeft"			"BindVoiceChat"
		"navRight"			"BindSelectMarine1"
		"navUp"				"BindActivatePrimary"
		"navDown"			"BindActivateEquipment0"
	}

	"BindActivateEquipment0"
	{
		"ControlName"		"CRD_VGUI_Bind"
		"fieldName"			"BindActivateEquipment0"
		"xpos"				"202"	[!$WIN32WIDE]
		"xpos"				"217"	[$WIN32WIDE]
		"ypos"				"172"
		"wide"				"176"
		"tall"				"12"
		"navLeft"			"BindWheelDefault"
		"navRight"			"BindSelectMarine2"
		"navUp"				"BindActivateSecondary"
		"navDown"			"BindActivateEquipment1"
	}

	"BindActivateEquipment1"
	{
		"ControlName"		"CRD_VGUI_Bind"
		"fieldName"			"BindActivateEquipment1"
		"xpos"				"202"	[!$WIN32WIDE]
		"xpos"				"217"	[$WIN32WIDE]
		"ypos"				"184"
		"wide"				"176"
		"tall"				"12"
		"navLeft"			"BindEmoteGo"
		"navRight"			"BindSelectMarine3"
		"navUp"				"BindActivateEquipment0"
		"navDown"			"BindActivateEquipment2"
	}

	"BindActivateEquipment2"
	{
		"ControlName"		"CRD_VGUI_Bind"
		"fieldName"			"BindActivateEquipment2"
		"xpos"				"202"	[!$WIN32WIDE]
		"xpos"				"217"	[$WIN32WIDE]
		"ypos"				"196"
		"wide"				"176"
		"tall"				"12"
		"navLeft"			"BindEmoteStop"
		"navRight"			"BindSelectMarine4"
		"navUp"				"BindActivateEquipment1"
		"navDown"			"BindActivateEquipment3"
	}

	"BindActivateEquipment3"
	{
		"ControlName"		"CRD_VGUI_Bind"
		"fieldName"			"BindActivateEquipment3"
		"xpos"				"202"	[!$WIN32WIDE]
		"xpos"				"217"	[$WIN32WIDE]
		"ypos"				"208"
		"wide"				"176"
		"tall"				"12"
		"navLeft"			"BindMarinePosition"
		"navRight"			"BindSelectMarine5"
		"navUp"				"BindActivateEquipment2"
		"navDown"			"BindActivateEquipment4"
	}

	"BindActivateEquipment4"
	{
		"ControlName"		"CRD_VGUI_Bind"
		"fieldName"			"BindActivateEquipment4"
		"xpos"				"202"	[!$WIN32WIDE]
		"xpos"				"217"	[$WIN32WIDE]
		"ypos"				"220"
		"wide"				"176"
		"tall"				"12"
		"navLeft"			"BindEmoteMedic"
		"navRight"			"BindSelectMarine6"
		"navUp"				"BindActivateEquipment3"
		"navDown"			"BindActivateEquipment5"
	}

	"BindActivateEquipment5"
	{
		"ControlName"		"CRD_VGUI_Bind"
		"fieldName"			"BindActivateEquipment5"
		"xpos"				"202"	[!$WIN32WIDE]
		"xpos"				"217"	[$WIN32WIDE]
		"ypos"				"232"
		"wide"				"176"
		"tall"				"12"
		"navLeft"			"BindEmoteAmmo"
		"navRight"			"BindSelectMarine7"
		"navUp"				"BindActivateEquipment4"
		"navDown"			"BindActivateEquipment6"
	}

	"BindActivateEquipment6"
	{
		"ControlName"		"CRD_VGUI_Bind"
		"fieldName"			"BindActivateEquipment6"
		"xpos"				"202"	[!$WIN32WIDE]
		"xpos"				"217"	[$WIN32WIDE]
		"ypos"				"244"
		"wide"				"176"
		"tall"				"12"
		"navLeft"			"BindEmoteQuestion"
		"navRight"			"BindWheelMarine"
		"navUp"				"BindActivateEquipment5"
		"navDown"			"BindActivateEquipment7"
	}

	"BindActivateEquipment7"
	{
		"ControlName"		"CRD_VGUI_Bind"
		"fieldName"			"BindActivateEquipment7"
		"xpos"				"202"	[!$WIN32WIDE]
		"xpos"				"217"	[$WIN32WIDE]
		"ypos"				"256"
		"wide"				"176"
		"tall"				"12"
		"navLeft"			"BindEmoteExclaim"
		"navRight"			"BindWheelMarine"
		"navUp"				"BindActivateEquipment6"
		"navDown"			"BindWheelEquipment"
	}

	"BindWheelEquipment"
	{
		"ControlName"		"CRD_VGUI_Bind"
		"fieldName"			"BindWheelEquipment"
		"xpos"				"202"	[!$WIN32WIDE]
		"xpos"				"217"	[$WIN32WIDE]
		"ypos"				"268"
		"wide"				"176"
		"tall"				"12"
		"navLeft"			"BindVoteYes"
		"navRight"			"BtnCustomWheels"
		"navUp"				"BindActivateEquipment7"
		"navDown"			"BindWheelEquipment1"
	}

	"BindWheelEquipment1"
	{
		"ControlName"		"CRD_VGUI_Bind"
		"fieldName"			"BindWheelEquipment1"
		"xpos"				"202"	[!$WIN32WIDE]
		"xpos"				"217"	[$WIN32WIDE]
		"ypos"				"280"
		"wide"				"176"
		"tall"				"12"
		"navLeft"			"BindVoteNo"
		"navRight"			"BtnCustomWheels"
		"navUp"				"BindWheelEquipment"
		"navDown"			"BindWheelEquipment2"
	}

	"BindWheelEquipment2"
	{
		"ControlName"		"CRD_VGUI_Bind"
		"fieldName"			"BindWheelEquipment2"
		"xpos"				"202"	[!$WIN32WIDE]
		"xpos"				"217"	[$WIN32WIDE]
		"ypos"				"292"
		"wide"				"176"
		"tall"				"12"
		"navLeft"			"BindMissionOverview"
		"navRight"			"BtnCustomWheels"
		"navUp"				"BindWheelEquipment1"
		"navDown"			"BindWheelEquipment2"
	}

	"BindSelectMarine0"
	{
		"ControlName"		"CRD_VGUI_Bind"
		"fieldName"			"BindSelectMarine0"
		"xpos"				"388"	[!$WIN32WIDE]
		"xpos"				"418"	[$WIN32WIDE]
		"ypos"				"148"
		"wide"				"176"
		"tall"				"12"
		"navLeft"			"BindActivatePrimary"
		"navRight"			"BindSelectMarine0"
		"navUp"				"BindReload"
		"navDown"			"BindSelectMarine1"
	}

	"BindSelectMarine1"
	{
		"ControlName"		"CRD_VGUI_Bind"
		"fieldName"			"BindSelectMarine1"
		"xpos"				"388"	[!$WIN32WIDE]
		"xpos"				"418"	[$WIN32WIDE]
		"ypos"				"160"
		"wide"				"176"
		"tall"				"12"
		"navLeft"			"BindActivateSecondary"
		"navRight"			"BindSelectMarine1"
		"navUp"				"BindSelectMarine0"
		"navDown"			"BindSelectMarine2"
	}

	"BindSelectMarine2"
	{
		"ControlName"		"CRD_VGUI_Bind"
		"fieldName"			"BindSelectMarine2"
		"xpos"				"388"	[!$WIN32WIDE]
		"xpos"				"418"	[$WIN32WIDE]
		"ypos"				"172"
		"wide"				"176"
		"tall"				"12"
		"navLeft"			"BindActivateEquipment0"
		"navRight"			"BindSelectMarine2"
		"navUp"				"BindSelectMarine1"
		"navDown"			"BindSelectMarine3"
	}

	"BindSelectMarine3"
	{
		"ControlName"		"CRD_VGUI_Bind"
		"fieldName"			"BindSelectMarine3"
		"xpos"				"388"	[!$WIN32WIDE]
		"xpos"				"418"	[$WIN32WIDE]
		"ypos"				"184"
		"wide"				"176"
		"tall"				"12"
		"navLeft"			"BindActivateEquipment1"
		"navRight"			"BindSelectMarine3"
		"navUp"				"BindSelectMarine2"
		"navDown"			"BindSelectMarine4"
	}

	"BindSelectMarine4"
	{
		"ControlName"		"CRD_VGUI_Bind"
		"fieldName"			"BindSelectMarine4"
		"xpos"				"388"	[!$WIN32WIDE]
		"xpos"				"418"	[$WIN32WIDE]
		"ypos"				"196"
		"wide"				"176"
		"tall"				"12"
		"navLeft"			"BindActivateEquipment2"
		"navRight"			"BindSelectMarine4"
		"navUp"				"BindSelectMarine3"
		"navDown"			"BindSelectMarine5"
	}

	"BindSelectMarine5"
	{
		"ControlName"		"CRD_VGUI_Bind"
		"fieldName"			"BindSelectMarine5"
		"xpos"				"388"	[!$WIN32WIDE]
		"xpos"				"418"	[$WIN32WIDE]
		"ypos"				"208"
		"wide"				"176"
		"tall"				"12"
		"navLeft"			"BindActivateEquipment3"
		"navRight"			"BindSelectMarine5"
		"navUp"				"BindSelectMarine4"
		"navDown"			"BindSelectMarine6"
	}

	"BindSelectMarine6"
	{
		"ControlName"		"CRD_VGUI_Bind"
		"fieldName"			"BindSelectMarine6"
		"xpos"				"388"	[!$WIN32WIDE]
		"xpos"				"418"	[$WIN32WIDE]
		"ypos"				"220"
		"wide"				"176"
		"tall"				"12"
		"navLeft"			"BindActivateEquipment4"
		"navRight"			"BindSelectMarine6"
		"navUp"				"BindSelectMarine5"
		"navDown"			"BindSelectMarine7"
	}

	"BindSelectMarine7"
	{
		"ControlName"		"CRD_VGUI_Bind"
		"fieldName"			"BindSelectMarine7"
		"xpos"				"388"	[!$WIN32WIDE]
		"xpos"				"418"	[$WIN32WIDE]
		"ypos"				"232"
		"wide"				"176"
		"tall"				"12"
		"navLeft"			"BindActivateEquipment5"
		"navRight"			"BindSelectMarine7"
		"navUp"				"BindSelectMarine6"
		"navDown"			"BindWheelMarine"
	}

	"BindWheelMarine"
	{
		"ControlName"		"CRD_VGUI_Bind"
		"fieldName"			"BindWheelMarine"
		"xpos"				"388"	[!$WIN32WIDE]
		"xpos"				"418"	[$WIN32WIDE]
		"ypos"				"244"
		"wide"				"176"
		"tall"				"12"
		"navLeft"			"BindActivateEquipment6"
		"navRight"			"BindWheelMarine"
		"navUp"				"BindSelectMarine7"
		"navDown"			"BtnCustomWheels"
	}

	"BtnCustomWheels"
	{
		"ControlName"		"BaseModHybridButton"
		"fieldName"			"BtnCustomWheels"
		"xpos"				"412"	[!$WIN32WIDE]
		"xpos"				"442"	[$WIN32WIDE]
		"ypos"				"300"
		"wide"				"128"
		"tall"				"34"
		"labelText"			"#rd_manage_custom_chat_wheels"
		"command"			"ManageWheels"
		"centerwrap"		"1"
		"navLeft"			"BindWheelEquipment2"
		"navRight"			"BtnCustomWheels"
		"navUp"				"BindWheelMarine"
		"navDown"			"SettingDeveloperConsole"
	}

	"BtnResetDefaults"
	{
		"ControlName"		"BaseModHybridButton"
		"fieldName"			"BtnResetDefaults"
		"xpos"				"8"
		"ypos"				"380"
		"wide"				"150"
		"tall"				"18"
		"labelText"			"#L4D360UI_Controller_Default"
		"style"				"DialogButton"
		"command"			"ResetDefaults"
		"navLeft"			"BtnResetDefaults"
		"navRight"			"SettingDeveloperConsole"
		"navUp"				"BindChooseMarine"
		"navDown"			"BtnResetDefaults"
	}

	"SettingDeveloperConsole"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingDeveloperConsole"
		"xpos"				"422"	[!$WIN32WIDE]
		"xpos"				"452"	[$WIN32WIDE]
		"ypos"				"380"
		"wide"				"150"
		"tall"				"18"
		"ResourceFile"		"resource/ui/option_simple_checkbox.res"
		"navLeft"			"BtnResetDefaults"
		"navRight"			"SettingDeveloperConsole"
		"navUp"				"BtnCustomWheels"
		"navDown"			"SettingDeveloperConsole"
	}

	"LblMovement"
	{
		"ControlName"		"Label"
		"fieldName"			"LblMovement"
		"xpos"				"8"
		"ypos"				"32"
		"zpos"				"-1"
		"wide"				"158"
		"tall"				"92"
		"textAlignment"		"north-west"
		"labelText"			"#rd_controls_category_movement"
		"font"				"DefaultMedium"
	}

	"LblController"
	{
		"ControlName"		"Label"
		"fieldName"			"LblController"
		"xpos"				"159"	[!$WIN32WIDE]
		"xpos"				"174"	[$WIN32WIDE]
		"ypos"				"32"
		"zpos"				"-1"
		"wide"				"236"
		"tall"				"92"
		"textAlignment"		"north-west"
		"labelText"			"#rd_controls_category_controller"
		"font"				"DefaultMedium"
	}

	"LblActions"
	{
		"ControlName"		"Label"
		"fieldName"			"LblActions"
		"xpos"				"388"	[!$WIN32WIDE]
		"xpos"				"418"	[$WIN32WIDE]
		"ypos"				"32"
		"zpos"				"-1"
		"wide"				"184"
		"tall"				"92"
		"textAlignment"		"north-west"
		"labelText"			"#rd_controls_category_actions"
		"font"				"DefaultMedium"
	}

	"LblSocialExtras"
	{
		"ControlName"		"Label"
		"fieldName"			"LblSocialExtras"
		"xpos"				"8"
		"ypos"				"132"
		"zpos"				"-1"
		"wide"				"192"
		"tall"				"240"
		"textAlignment"		"north-west"
		"labelText"			"#rd_controls_category_social_and_extras"
		"font"				"DefaultMedium"
	}

	"LblUseEquipment"
	{
		"ControlName"		"Label"
		"fieldName"			"LblUseEquipment"
		"xpos"				"194"	[!$WIN32WIDE]
		"xpos"				"209"	[$WIN32WIDE]
		"ypos"				"132"
		"zpos"				"-1"
		"wide"				"192"
		"tall"				"180"
		"textAlignment"		"north-west"
		"labelText"			"#rd_controls_category_use_equipment"
		"font"				"DefaultMedium"
	}

	"LblSelectMarine"
	{
		"ControlName"		"Label"
		"fieldName"			"LblSelectMarine"
		"xpos"				"380"	[!$WIN32WIDE]
		"xpos"				"410"	[$WIN32WIDE]
		"ypos"				"132"
		"zpos"				"-1"
		"wide"				"192"
		"tall"				"132"
		"textAlignment"		"north-west"
		"labelText"			"#rd_controls_category_select_marine"
		"font"				"DefaultMedium"
	}
}
