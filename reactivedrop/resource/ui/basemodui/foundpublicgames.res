"Resource/UI/FoundPublicGames.res"
{
	"FoundPublicGames"
	{
		"ControlName"					"Frame"
		"fieldName"						"FoundPublicGames"
		"xpos"							"0"
		"ypos"							"0"
		"wide"							"f0"
		"tall"							"f0"	[$WIN32]
		"tall"							"335"	[$X360]
		"autoResize"					"0"
		"pinCorner"						"0"
		"visible"						"1"
		"enabled"						"1"
		"tabPosition"					"0"
	}

	"Title"
	{
		"fieldName"		"Title"
		"xpos"		"c-266"
		"ypos"		"61"
		"wide"		"450"
		"tall"		"19"
		"zpos"		"5"
		"font"		"DefaultExtraLarge"
		"textAlignment"		"west"
		"ControlName"		"Label"
		"fgcolor_override"		"224 224 224 255"
	}

	"SubTitle"
	{
		"fieldName"		"SubTitle"
		"xpos"		"c-266"
		"ypos"		"61"
		"wide"		"450"
		"tall"		"19"
		"zpos"		"5"
		"font"		"Default"
		"textAlignment"		"south-east"
		"ControlName"		"Label"
		"fgcolor_override"		"224 224 224 255"
	}

	"ImgBackground" [$WIN32]
	{
		"ControlName"			"L4DMenuBackground"
		"fieldName"				"ImgBackground"
		"xpos"					"0"
		"ypos"					"94"
		"zpos"					"-1"
		"wide"					"f0"
		"tall"					"312"
		"autoResize"			"0"
		"pinCorner"				"0"
		"visible"				"1"
		"enabled"				"1"
		"tabPosition"			"0"
		"fillColor"				"0 0 0 0"
	}

	"LblCampaign"
	{
		"ControlName"					"Label"
		"fieldName"						"LblCampaign"
		"xpos"							"c130"	[$WIN32]
		"ypos"							"245"	[$WIN32]
		"xpos"							"c107"	[$X360]
		"ypos"							"110"	[$X360]
		"zpos"							"2"
		"wide"							"200"
		"tall"							"12"
		"autoResize"					"0"
		"pinCorner"						"0"
		"visible"						"1"
		"enabled"						"1"
		"tabPosition"					"0"
		"labelText"						""
		"textAlignment"					"north-west"
		"Font"							"DefaultMedium"
		//"fgcolor_override"				"TextYellow"
	}

	"ImgLevelImage"
	{
		"ControlName"			"ImagePanel"
		"fieldName"				"ImgLevelImage"
		"xpos"					"c131"
		"ypos"					"139"
		"wide"					"133"
		"tall"					"100"
		"zois"					"1"
		"scaleImage"			"1"
		"pinCorner"				"0"
		"visible"				"0"
		"enabled"				"1"
		"tabPosition"			"0"
		"image"					"maps/any"
		"scaleImage"			"1"
	}
	"ImgFrame"
	{
		"ControlName"			"Panel"
		"fieldName"				"ImgFrame"
		"xpos"					"c130"
		"ypos"					"138"
		"wide"					"135"
		"tall"					"102"
		"scaleImage"			"1"
		"pinCorner"				"0"
		"visible"				"0"
		"enabled"				"0"
		"tabPosition"			"0"
		"bgcolor_override"		"0 0 0 128"
		//"image"					"campaignFrame"
		//"scaleImage"			"1"
	}

	"LblAuthor" [$WIN32]
	{
		"ControlName"				"Label"
		"fieldName"					"LblAuthor"
		"xpos"						"c130"
		"zpos"						"2"
		"ypos"						"258"
		"wide"						"200"
		"tall"						"12"
		"autoResize"				"0"
		"pinCorner"					"0"
		"visible"					"1"
		"enabled"					"1"
		"tabPosition"				"0"
		"labelText"					""
		"textAlignment"				"north-west"
		"Font"						"Default"
		//"fgcolor_override"			"TextYellow"
	}

	"LblGameDifficulty"
	{
		"ControlName"				"Label"
		"fieldName"					"LblGameDifficulty"
		"xpos"						"c130"	[$WIN32]
		"ypos"						"270"	[$WIN32]
		"xpos"						"c107"	[$X360]
		"ypos"						"230"	[$X360]
		"zpos"						"2"
		"wide"						"200"
		"tall"						"12"
		"autoResize"				"0"
		"pinCorner"					"0"
		"visible"					"0"
		"enabled"					"1"
		"tabPosition"				"0"
		"labelText"					""
		"textAlignment"				"north-west"
		"Font"						"Default"
		//"fgcolor_override"			"TextYellow"
	}

	"LblGameChallenge"
	{
		"ControlName"				"Label"
		"fieldName"					"LblGameChallenge"
		"xpos"						"c130"
		"ypos"						"282"
		"zpos"						"2"
		"wide"						"200"
		"tall"						"12"
		"autoResize"				"0"
		"pinCorner"					"0"
		"visible"					"0"
		"enabled"					"1"
		"tabPosition"				"0"
		"labelText"					""
		"textAlignment"				"north-west"
		"Font"						"Default"
		//"fgcolor_override"			"TextYellow"
	}

// 	"LblGameStatus"
// 	{
// 		"ControlName"				"Label"
// 		"fieldName"					"LblGameStatus"
// 		"xpos"						"c90"	[$WIN32]
// 		"ypos"						"280"	[$WIN32]
// 		"xpos"						"c107"	[$X360]
// 		"ypos"						"250"	[$X360]
// 		"zpos"						"2"
// 		"wide"						"200"
// 		"tall"						"12"
// 		"autoResize"				"0"
// 		"pinCorner"					"0"
// 		"visible"					"1"
// 		"enabled"					"1"
// 		"tabPosition"				"0"
// 		"labelText"					""
// 		"textAlignment"				"north-west"
// 		"Font"						"Default"
// 		//"fgcolor_override"			"TextYellow"
// 	}
// 
// 	"LblGameStatus2"	[$X360]
// 	{
// 		"ControlName"				"Label"
// 		"fieldName"					"LblGameStatus2"
// 		"xpos"						"c107"
// 		"ypos"						"270"
// 		"zpos"						"2"
// 		"wide"						"200"
// 		"tall"						"12"
// 		"autoResize"				"0"
// 		"pinCorner"					"0"
// 		"visible"					"1"
// 		"enabled"					"1"
// 		"tabPosition"				"0"
// 		"labelText"					""
// 		"textAlignment"				"north-west"
// 		"Font"						"Default"
// 		//"fgcolor_override"			"TextYellow"
// 	}

	"LblNewVersion" [$WIN32]
	{
		"ControlName"				"Label"
		"fieldName"					"LblNewVersion"
		"xpos"							"c130"
		"ypos"							"325"
		"zpos"						"2"
		"wide"						"200"
		"tall"						"12"
		"autoResize"				"0"
		"pinCorner"					"0"
		"visible"					"1"
		"enabled"					"1"
		"tabPosition"				"0"
		"labelText"					"#L4D360UI_FoundGames_DownloadNewVersion"
		"textAlignment"				"north-west"
		"Font"						"Default"
		//"fgcolor_override"			"TextYellow"
	}

	"BtnWebsite" [$WIN32]
	{
		"ControlName"				"BaseModHybridButton"
		"fieldName"					"BtnWebsite"
		"xpos"							"c122"
		"ypos"							"310"
		"zpos"						"2"
		"wide"						"200"
		"tall"						"15"
		"autoResize"				"0"
		"pinCorner"					"0"
		"visible"					"0"
		"enabled"					"1"
		"tabPosition"				"0"
		"command"					"Website"
		"labelText"					""
		"textAlignment"				"north-west"
		"style"						"FlyoutMenuButton"
		"navLeft"					"GplGames"
		"navDown"					"BtnJoinSelected"
	}

// 	"BtnJoinSelected" [$WIN32]
// 	{
// 		"ControlName"			"BaseModHybridButton"
// 		"fieldName"				"BtnJoinSelected"
// 		"xpos"					"c90"
// 		"ypos"					"345"
// 		"zpos"					"2"
// 		"wide"					"200"
// 		"tall"					"15"
// 		"autoResize"			"1"
// 		"pinCorner"				"0"
// 		"visible"				"0"
// 		"enabled"				"1"
// 		"tabPosition"			"0"
// 		"wrap"					"1"
// 		"labelText"				"#L4D360UI_FoundGames_JoinGame"
// 		"tooltiptext"			"#L4D360UI_JoinGame"
// 		"style"					"MainMenuSmallButton"
// 		"command"				"JoinSelected"
// 		EnabledTextInsetX		"2"
// 		DisabledTextInsetX		"2"
// 		FocusTextInsetX			"2"
// 		OpenTextInsetX			"2"
// 		"navLeft"				"GplGames"
// 		"navUp"					"BtnWebsite"
// 		"navDown"				"BtnDownloadSelected"
// 	}

	"BtnDownloadSelected" [$WIN32]
	{
		"ControlName"			"BaseModHybridButton"
		"fieldName"				"BtnDownloadSelected"
		"xpos"					"c130"
		"ypos"					"325"
		"zpos"					"2"
		"wide"					"200"
		"tall"					"15"
		"autoResize"			"1"
		"pinCorner"				"0"
		"visible"				"0"
		"enabled"				"0"
		"tabPosition"			"0"
		"wrap"					"1"
		"labelText"				"#L4D360UI_FoundGames_DownloadAddon"
		"tooltiptext"			"#L4D360UI_FoundGames_Join_Download"
		"style"					"RedMainButton"
		"command"				"DownloadSelected"
		EnabledTextInsetX		"2"
		DisabledTextInsetX		"2"
		FocusTextInsetX			"2"
		OpenTextInsetX			"2"
		"navLeft"				"GplGames"
		"navUp"					"BtnJoinSelected"
		"navDown"				"BtnWebsite"
	}

	"SearchingIcon"
	{
		"ControlName"			"ImagePanel"
		"fieldName"				"SearchingIcon"
		"xpos"					"r106"		[$WIN32]
		"xpos"					"r128"		[$X360]
		"ypos"					"27"
		"zpos"					"2"
		"wide"					"32"
		"tall"					"32"
		"pinCorner"				"0"
		"visible"				"1"
		"enabled"				"1"
		"tabPosition"			"0"
		"scaleImage"			"1"
		"image"					"common/swarm_cycle"
	}

	"LblPressX"		[$X360]
	{
		"ControlName"					"Label"
		"fieldName"						"LblPressX"
		"xpos"							"82"
		"ypos"							"295"
		"wide"							"200"
		"tall"							"15"
		"zpos"							"2"
		"autoResize"					"1"
		"pinCorner"						"0"
		"visible"						"1"
		"enabled"						"1"
		"tabPosition"					"0"
		"Font"							"GameUIButtonsTiny"
		"labelText"						"#GameUI_Icons_X_3DButton"
	}

	"LblNoGamesFound"
	{
		"ControlName"					"Label"
		"fieldName"						"LblNoGamesFound"
		"xpos"							"c-142"		[$WIN32]
		"ypos"							"90"		[$WIN32]
		"xpos"							"c-285" [$X360]
		"ypos"							"80"    [$X360]
		"wide"							"380"
		"tall"							"20"
		"zpos"							"2"
		"autoResize"					"0"
		"pinCorner"						"0"
		"visible"						"1" 
		"enabled"						"1"
		"tabPosition"					"0"
		"labelText"						""	//"No Campaign Games Found"
		"textAlignment"					"center" [$X360]
		"textAlignment"					"west" [$WIN32]
		"Font"							"DefaultBold"
	}

	"LblSearching"
	{
		"ControlName"					"Label"
		"fieldName"						"LblSearching"
		"xpos"							"c-320"		[$WIN32]
		"ypos"							"38"		[$WIN32]
		"xpos"							"80"	[$X360]
		"ypos"							"110"	[$X360]
		"zpos"							"0"
		"wide"							"380"		[$WIN32]
		"wide"							"370"		[$X360]
		"tall"							"195"
		"zpos"							"2"
		"autoResize"					"0"
		"pinCorner"						"0"
		"visible"						"0" 
		"enabled"						"1"
		"tabPosition"					"0"
		"labelText"						""
		"textAlignment"					"center"
		"Font"							"MainBold"		[$WIN32]
		"Font"							"FrameTitle"	[$X360]
	}

	// top line
	"Divider1" [$WIN32]
	{
		"ControlName"			"ImagePanel"
		"fieldName"				"Divider1"
		"xpos"					"c-320"
		"ypos"					"110"
		"zpos"					"-1"
		"wide"					"400"
		"tall"					"2"
		"autoResize"			"0"
		"pinCorner"				"0"
		"visible"				"1"
		"enabled"				"1"
		"tabPosition"			"0"
		"image"					"divider_gradient"
		"scaleImage"			"1"
	}

	// bottom line
	"Divider2" [$WIN32]
	{
		"ControlName"			"ImagePanel"
		"fieldName"				"Divider2"
		"xpos"					"c-320"
		"ypos"					"325"
		"zpos"					"-1"
		"wide"					"400"
		"tall"					"2"
		"autoResize"			"0"
		"pinCorner"				"0"
		"visible"				"1"
		"enabled"				"1"
		"tabPosition"			"0"
		"image"					"divider_gradient"
		"scaleImage"			"1"
	}

	"GplGames"
	{
		"ControlName"			"GenericPanelList"
		"fieldName"				"GplGames"
		"xpos"					"c-320"		[$WIN32]
		"ypos"					"110"		[$WIN32]
		"zpos"					"0"
		"wide"					"440"		[$WIN32]
		"tall"					"260"		[$WIN32]
		"xpos"					"45"		[$X360 && $X360WIDE]
		"xpos"					"20"		[$X360 && !$X360WIDE]
		"ypos"					"124"		[$X360]
		"wide"					"440"		[$X360 && $X360WIDE]
		"wide"					"405"		[$X360 && !$X360WIDE]
		"tall"					"170"		[$X360]
		"autoResize"			"1"
		"pinCorner"				"0"
		"visible"				"1"
		"enabled"				"1"
		"tabPosition"			"0"
		"bgcolor_override" 		"0 0 0 128"
		"NoWrap"				"1"
		"panelBorder"			"2" [$WIN32]
		"navRight"				"BtnJoinSelected" [$WIN32]
		"navDown"				"BtnFilters" [$WIN32]
		"navUp"					"BtnCancel" [$WIN32]
	}

	"BtnCreateNewGame" [$WIN32]
	{
		"ControlName"			"CNB_Button"
		"fieldName"				"BtnCreateNewGame"
		"xpos"		"c147"
		"ypos"		"r23"
		"wide"		"117"
		"tall"		"27"
		"zpos"		"1"
		"visible"				"1"
		"enabled"				"1"
		"tabPosition"			"0"
		"command"				"CreateGame"
		"textAlignment"		"center"
		"font"		"DefaultMedium"
		"fgcolor_override"		"113 142 181 255"
	}

	"BtnJoinSelected" [$WIN32]
	{
		"ControlName"			"CNB_Button"
		"fieldName"				"BtnJoinSelected"
		"xpos"		"c10"
		"ypos"		"r23"
		"wide"		"117"
		"tall"		"27"
		"zpos"		"1"
		"visible"				"1"
		"enabled"				"1"
		"tabPosition"			"0"
		"command"				"JoinSelected"
		"textAlignment"		"center"
		"font"		"DefaultMedium"
		"labelText"	"#L4D360UI_FoundGames_JoinGame"
		"fgcolor_override"		"113 142 181 255"
	}

	"BtnCancel" [$WIN32]
	{
		"ControlName"			"CNB_Button"
		"fieldName"				"BtnCancel"
		"xpos"		"c-264"
		"ypos"		"r23"
		"wide"		"117"
		"tall"		"27"
		"zpos"		"1"
		"visible"				"1"
		"enabled"				"1"
		"tabPosition"			"0"
		"labelText"				"#nb_back"
		"command"				"Back"
		"textAlignment"		"center"
		"font"		"DefaultMedium"
		"fgcolor_override"		"113 142 181 255"
	}

	"LblSupportRequiredDetails"
	{
		"ControlName"		"Label"
		"fieldName"			"LblSupportRequiredDetails"
		"xpos"				"c90"
		"ypos"				"300"
		"wide"				"220"
		"tall"				"50" 
		"zpos"				"1"
		"autoResize"		"0"
		"pinCorner"			"0"
		"visible"			"0"
		"enabled"			"1"
		"tabPosition"		"0"
		"font"				"DefaultMedium"
		"textAlignment"		"north-west"
		"labelText"			"#L4D360UI_FOUNDGAMES_ADDON_SUPPORT_REQUIRED"	//"Add-on support is required to play third party campaigns"
		"fgcolor_override"	"MediumGray"
		"wrap"				"1"
	}

	"BtnInstallSupport"
	{
		"ControlName"			"BaseModHybridButton"
		"fieldName"				"BtnInstallSupport"
		"xpos"					"c90"
		"ypos"					"340"
		"zpos"					"2"
		"wide"					"250"
		"tall"					"15"
		"autoResize"			"1"
		"pinCorner"				"0"
		"visible"				"0"
		"enabled"				"1"
		"tabPosition"			"0"
		"wrap"					"1"
		"labelText"				"#L4D360UI_ADDON_SUPPORT_INSTALL"			//"INSTALL ADD-ON SUPPORT"
		"style"					"RedButton"		// actually teal!
		"command"				"InstallSupport"
		"proportionalToParent"	"1"
		"usetitlesafe" 			"0"
		EnabledTextInsetX		"2"
		DisabledTextInsetX		"2"
		FocusTextInsetX			"2"
		OpenTextInsetX			"2"
		"allcaps"				"1"
	}

	"LblInstalling"
	{
		"ControlName"		"Label"
		"fieldName"			"LblInstalling"
		"xpos"				"c100"
		"ypos"				"340"
		"zpos"				"3"
		"wide"				"250"
		"tall"				"18" 
		"zpos"				"1"
		"autoResize"		"0"
		"pinCorner"			"0"
		"visible"			"0"
		"enabled"			"1"
		"tabPosition"		"0"
		"font"				"DefaultLarge"
		"textAlignment"		"west"
		"labelText"			"#L4D360UI_ADDON_SUPPORT_INSTALLING"	//"INSTALLING ADD-ON SUPPORT..."
	}

	"LblInstallingDetails"
	{
		"ControlName"		"Label"
		"fieldName"			"LblInstallingDetails"
		"xpos"				"c100"
		"ypos"				"340"
		"zpos"				"3"
		"wide"				"250"
		"tall"				"50" 
		"autoResize"		"0"
		"pinCorner"			"0"
		"visible"			"0"
		"enabled"			"1"
		"tabPosition"		"0"
		"font"				"DefaultVerySmall"
		"textAlignment"		"west"
		"labelText"			"#L4D360UI_ADDON_SUPPORT_INSTALLING_DETAILS"	//"Check download progress in the Steam Tools tab."
		"fgcolor_override"			"MediumGray"
	}

	"PnlFiltersBackground"
	{
		"fieldName"			"PnlFiltersBackground"
		"ControlName"		"Panel"
		"xpos"				"c-140"
		"ypos"				"110"
		"zpos"				"5"
		"wide"				"405"
		"tall"				"260"
		"visible"			"0"
		"paintbackgroundtype"	"2"
		"bgcolor_override"	"12 34 56 255"
	}
	"BtnFilters"
	{
		"fieldName"			"BtnFilters"
		"xpos"				"c130"
		"ypos"				"343"
		"zpos"				"6"
		"wide"				"135"
		"tall"				"27"
		"textAlignment"		"center"
		"font"				"DefaultMedium"
		"fgcolor_override"	"113 142 181 255"
	}
	"OptionNameMode"
	{
		"fieldName"			"OptionNameMode"
		"xpos"				"c-130"
		"ypos"				"115"
		"zpos"				"6"
		"wide"				"190"
		"tall"				"52"
		"navLeft"			"OptionNameMode"
		"navRight"			"OptionInstalled"
		"navUp"				"OptionNameMode"
		"navDown"			"OptionDifficultyMin"
		"ResourceFile"		"resource/ui/option_lobby_radio.res"
	}
	"OptionDifficultyMin"
	{
		"fieldName"			"OptionDifficultyMin"
		"xpos"				"c-130"
		"ypos"				"160"
		"zpos"				"6"
		"wide"				"190"
		"tall"				"52"
		"navLeft"			"OptionDifficultyMin"
		"navRight"			"OptionChallenge"
		"navUp"				"OptionNameMode"
		"navDown"			"OptionDifficultyMax"
		"ResourceFile"		"resource/ui/option_lobby_radio.res"
	}
	"OptionDifficultyMax"
	{
		"fieldName"			"OptionDifficultyMax"
		"xpos"				"c-130"
		"ypos"				"205"
		"zpos"				"6"
		"wide"				"190"
		"tall"				"52"
		"navLeft"			"OptionDifficultyMax"
		"navRight"			"OptionDedicated"
		"navUp"				"OptionDifficultyMin"
		"navDown"			"OptionOnslaught"
		"ResourceFile"		"resource/ui/option_lobby_radio.res"
	}
	"OptionOnslaught"
	{
		"fieldName"			"OptionOnslaught"
		"xpos"				"c-130"
		"ypos"				"250"
		"zpos"				"6"
		"wide"				"190"
		"tall"				"52"
		"navLeft"			"OptionOnslaught"
		"navRight"			"OptionDistance"
		"navUp"				"OptionDifficultyMax"
		"navDown"			"OptionHardcoreFF"
		"ResourceFile"		"resource/ui/option_lobby_radio.res"
	}
	"OptionHardcoreFF"
	{
		"fieldName"			"OptionHardcoreFF"
		"xpos"				"c-130"
		"ypos"				"295"
		"zpos"				"6"
		"wide"				"190"
		"tall"				"52"
		"navLeft"			"OptionHardcoreFF"
		"navRight"			"OptionAlwaysFriends"
		"navUp"				"OptionOnslaught"
		"navDown"			"OptionHardcoreFF"
		"ResourceFile"		"resource/ui/option_lobby_radio.res"
	}
	"OptionInstalled"
	{
		"fieldName"			"OptionInstalled"
		"xpos"				"c70"
		"ypos"				"115"
		"zpos"				"6"
		"wide"				"190"
		"tall"				"52"
		"navLeft"			"OptionNameMode"
		"navRight"			"OptionInstalled"
		"navUp"				"OptionInstalled"
		"navDown"			"OptionChallenge"
		"ResourceFile"		"resource/ui/option_lobby_radio.res"
	}
	"OptionChallenge"
	{
		"fieldName"			"OptionChallenge"
		"xpos"				"c70"
		"ypos"				"160"
		"zpos"				"6"
		"wide"				"190"
		"tall"				"52"
		"navLeft"			"OptionDifficultyMin"
		"navRight"			"OptionChallenge"
		"navUp"				"OptionInstalled"
		"navDown"			"OptionDedicated"
		"ResourceFile"		"resource/ui/option_lobby_radio.res"
	}
	"OptionDedicated"
	{
		"fieldName"			"OptionDedicated"
		"xpos"				"c70"
		"ypos"				"205"
		"zpos"				"6"
		"wide"				"190"
		"tall"				"52"
		"navLeft"			"OptionDifficultyMax"
		"navRight"			"OptionDedicated"
		"navUp"				"OptionChallenge"
		"navDown"			"OptionDistance"
		"ResourceFile"		"resource/ui/option_lobby_radio.res"
	}
	"OptionDistance"
	{
		"fieldName"			"OptionDistance"
		"xpos"				"c70"
		"ypos"				"250"
		"zpos"				"6"
		"wide"				"190"
		"tall"				"52"
		"navLeft"			"OptionOnslaught"
		"navRight"			"OptionDistance"
		"navUp"				"OptionDedicated"
		"navDown"			"OptionAlwaysFriends"
		"ResourceFile"		"resource/ui/option_lobby_radio.res"
	}
	"OptionAlwaysFriends"
	{
		"fieldName"			"OptionAlwaysFriends"
		"xpos"				"c70"
		"ypos"				"295"
		"zpos"				"6"
		"wide"				"190"
		"tall"				"40"
		"navLeft"			"OptionHardcoreFF"
		"navRight"			"OptionAlwaysFriends"
		"navUp"				"OptionDistance"
		"navDown"			"OptionAlwaysFriends"
		"ResourceFile"		"resource/ui/option_lobby_checkbox.res"
	}

	"WorkshopDownloadProgress"
	{
		"ControlName"		"CRD_VGUI_Workshop_Download_Progress"
		"fieldName"			"WorkshopDownloadProgress"
		"xpos"				"r150"
		"ypos"				"r50"
		"wide"				"145"
		"tall"				"45"
		"zpos"				"20"
		"visible"			"1"
		"enabled"			"1"
	}
}
