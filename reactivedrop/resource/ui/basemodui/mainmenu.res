"Resource/UI/MainMenu.res"
{
	"MainMenu"
	{
		"ControlName"			"Frame"
		"fieldName"				"MainMenu"
		"xpos"					"0"
		"ypos"					"0"
		"wide"					"f0"
		"tall"					"f0"
		"autoResize"			"0"
		"pinCorner"				"0"
		"visible"				"1"
		"enabled"				"1"
		"tabPosition"			"0"
		"PaintBackgroundType"	"0"
		"navUp"					"BtnMultiplayer"
		"navDown"				"BtnMultiplayer"
		"navLeft"				"PnlQuickJoin"
		"navRight"				"PnlQuickJoinGroups"
	}
	
	"BtnMultiplayer"
	{
		"ControlName"			"BaseModHybridButton"
		"fieldName"				"BtnMultiplayer"
		"xpos"					"100"
		"ypos"					"180"
		"wide"					"180"
		"tall"					"17"
		"autoResize"			"1"
		"pinCorner"				"0"
		"visible"				"1"
		"enabled"				"1"
		"tabPosition"			"0"
		"allcaps"				"1"
		"navUp"					"BtnQuit"
		"navDown"				"BtnTraining"
		"labelText"				"#L4D360UI_MainMenu_CoOp"
		"tooltiptext"			"#L4D360UI_MainMenu_CoOp_Tip"
		"style"					"AlienSwarmMenuButton"
		"command"				"FlmCampaignFlyout"	
		"ActivationType"		"1"
	}
	
	"BtnTraining"
	{
		"ControlName"			"BaseModHybridButton"
		"fieldName"				"BtnTraining"
		"xpos"					"100"
		"ypos"					"210"
		"wide"					"180"
		"tall"					"13"
		"autoResize"			"1"
		"pinCorner"				"0"
		"visible"				"1"
		"enabled"				"1"
		"tabPosition"			"0"
		"allcaps"				"1"
		"navUp"					"BtnMultiplayer"
		"navDown"				"BtnSingleplayer"
		"labelText"				"#RD_MainMenu_Training"
		"tooltiptext"			"#RD_MainMenu_Training_Tip"
		"style"					"AlienSwarmMenuButtonSmall"
		"command"				"TrainingPlay"	
		"ActivationType"		"1"
	}

	"BtnSingleplayer"
	{
		"ControlName"			"BaseModHybridButton"
		"fieldName"				"BtnSingleplayer"
		"xpos"					"100"
		"ypos"					"225"
		"wide"					"180"
		"tall"					"13"
		"autoResize"			"1"
		"pinCorner"				"0"
		"visible"				"1"
		"enabled"				"1"
		"tabPosition"			"0"
		"allcaps"				"1"
		"navUp"					"BtnTraining"
		"navDown"				"BtnStatsAndAchievements"
		"labelText"				"#L4D360UI_MainMenu_PlaySolo"
		"tooltiptext"			"#L4D360UI_MainMenu_PlaySolo_Tip"
		"style"					"AlienSwarmMenuButtonSmall"
		"command"				"SoloPlay"	
		"ActivationType"		"1"
	}

	"BtnStatsAndAchievements"
	{
		"ControlName"			"BaseModHybridButton"
		"fieldName"				"BtnStatsAndAchievements"
		"xpos"					"100"
		"ypos"					"240"
		"wide"					"180"
		"tall"					"13"
		"autoResize"			"1"
		"pinCorner"				"0"
		"visible"				"1"
		"enabled"				"1"
		"tabPosition"			"0"
		"allcaps"				"1"
		"navUp"					"BtnSingleplayer"
		"navDown"				"BtnWorkshop"
		"labelText"				"#L4D360UI_MainMenu_StatsAndAchievements"
		"tooltiptext"			"#L4D360UI_MainMenu_StatsAndAchievements_Tip"	[$X360]
		"tooltiptext"			"#L4D360UI_MainMenu_PCStatsAndAchievements_Tip"	[$WIN32]
		"style"					"AlienSwarmMenuButtonSmall"
		"command"				"StatsAndAchievements"
		"ActivationType"		"1"
	}

	"BtnWorkshop"
	{
		"ControlName"			"BaseModHybridButton"
		"fieldName"				"BtnWorkshop"
		"xpos"					"100"
		"ypos"					"255" 
		"wide"					"180"
		"tall"					"13"
		"autoResize"			"1"
		"pinCorner"				"0"
		"visible"				"1"
		"enabled"				"1"
		"tabPosition"			"0"
		"allcaps"				"1"
		"navUp"					"BtnStatsAndAchievements"
		"navDown"				"BtnOptions"
		"style"					"AlienSwarmMenuButtonSmall"
		"ActivationType"		"1"
		"labelText"				"#rd_mainmenu_workshop"
		"tooltiptext"			"#rd_mainmenu_workshop_tip"
		"command"				"FlmWorkshopFlyout"
	}
	
	"BtnOptions"
	{
		"ControlName"			"BaseModHybridButton"
		"fieldName"				"BtnOptions"
		"xpos"					"100"
		"ypos"					"270"
		"wide"					"180"
		"tall"					"13"
		"autoResize"			"1"
		"pinCorner"				"0"
		"visible"				"1"
		"enabled"				"1"
		"tabPosition"			"0"
		"allcaps"				"1"
		"navUp"					"BtnWorkshop"
		"navDown"				"BtnCollections"
		"labelText"				"#L4D360UI_MainMenu_Options"
		"tooltiptext"			"#L4D360UI_MainMenu_Options_Tip"
		"style"					"AlienSwarmMenuButtonSmall"
		"command"				"FlmOptionsFlyout"	
		"ActivationType"		"1"
	}
	
	"BtnCollections"
	{
		"ControlName"			"BaseModHybridButton"
		"fieldName"				"BtnCollections"
		"xpos"					"100"
		"ypos"					"285"
		"wide"					"180"
		"tall"					"13"
		"autoResize"			"1"
		"pinCorner"				"0"
		"visible"				"1"
		"enabled"				"1"
		"tabPosition"			"0"
		"navUp"					"BtnOptions"
		"navDown"				"BtnDemoList"
		"labelText"				"#rd_mainmenu_collections"
		"tooltiptext"			""
		"allcaps"				"1"
		"style"					"AlienSwarmMenuButtonSmall"
		"command"				"#rd_collections"
		"ActivationType"		"1"
	}
	
	"BtnDemoList"
	{
		"ControlName"			"BaseModHybridButton"
		"fieldName"				"BtnDemoList"
		"xpos"					"100"
		"ypos"					"300"
		"wide"					"180"
		"tall"					"13"
		"autoResize"			"1"
		"pinCorner"				"0"
		"visible"				"1"
		"enabled"				"1"
		"tabPosition"			"0"
		"navUp"					"BtnCollections"
		"navDown"				"BtnIafRanks"
		"labelText"				"#rd_demo_list_title"
		"tooltiptext"			""
		"allcaps"				"1"
		"style"					"AlienSwarmMenuButtonSmall"
		"command"				"#rd_auto_record_ui"
		"ActivationType"		"1"
	}
	
	"BtnIafRanks"
	{
		"ControlName"			"BaseModHybridButton"
		"fieldName"				"BtnIafRanks"
		"xpos"					"100"
		"ypos"					"315"
		"wide"					"180"
		"tall"					"13"
		"autoResize"			"1"
		"pinCorner"				"0"
		"visible"				"1"
		"enabled"				"1"
		"tabPosition"			"0"
		"navUp"					"BtnDemoList"
		"navDown"				"BtnQuit"
		"labelText"				"#asw_iafranks"
		"tooltiptext"			""
		"allcaps"				"1"
		"style"					"AlienSwarmMenuButtonSmall"
		"command"				"IafRanks"	
		"ActivationType"		"1"
	}

	"BtnQuit"
	{
		"ControlName"			"BaseModHybridButton"
		"fieldName"				"BtnQuit"
		"xpos"					"100"
		"ypos"					"325"   [$X360]
		"ypos"					"345"	[$WIN32]
		"wide"					"180"
		"tall"					"13"
		"autoResize"			"1"
		"pinCorner"				"0"
		"visible"				"0"		[$X360]
		"visible"				"1"		[$WIN32]
		"enabled"				"1"
		"tabPosition"			"0"
		"allcaps"				"1"
		"navUp"					"BtnIafRanks"
		"navDown"				"BtnMultiplayer"
		"style"					"AlienSwarmMenuButtonSmall"
		"ActivationType"		"1"
		"labelText"				"#L4D360UI_MainMenu_Quit"			[$WIN32]
		"tooltiptext"			"#L4D360UI_MainMenu_Quit_Tip"		[$WIN32]
		"labelText"				"#L4D360UI_MainMenu_QuitDemo"		[$X360]
		"tooltiptext"			"L4D360UI_MainMenu_QuitDemo_Tip"	[$X360]
		"command"				"QuitGame"
	}

	"FlmCampaignFlyout"
	{
		"ControlName"			"FlyoutMenu"
		"fieldName"				"FlmCampaignFlyout"
		"visible"				"0"
		"wide"					"0"
		"tall"					"0"
		"zpos"					"3"
		"InitialFocus"			"BtnCreateGame"
		"ResourceFile"			"resource/UI/basemodui/CampaignFlyout.res"
	}

	"FlmWorkshopFlyout"
	{
		"ControlName"			"FlyoutMenu"
		"fieldName"				"FlmWorkshopFlyout"
		"visible"				"0"
		"wide"					"0"
		"tall"					"0"
		"zpos"					"3"
		"InitialFocus"			"BtnBrowse"
		"ResourceFile"			"resource/UI/basemodui/WorkshopFlyout_rd.res"
	}

	"FlmOptionsFlyout"
	{
		"ControlName"			"FlyoutMenu"
		"fieldName"				"FlmOptionsFlyout"
		"visible"				"0"
		"wide"					"0"
		"tall"					"0"
		"zpos"					"3"
		"InitialFocus"			"BtnAudioVideo"	[$X360]
		"InitialFocus"			"BtnVideo"	[$WIN32]
		"ResourceFile"			"resource/UI/basemodui/OptionsFlyout.res"
	}

	"PnlQuickJoin"
	{
		"ControlName"			"QuickJoinPanel"
		"fieldName"				"PnlQuickJoin"
		"ResourceFile"			"resource/UI/basemodui/QuickJoin.res"
		"visible"				"0"
		"wide"					"500"	[$X360]
		"wide"					"240"	[$WIN32]
		"tall"					"300"
		"xpos"					"r260"	[$X360]
		"xpos"					"80"	[$WIN32]
		"ypos"					"r120"	[$X360]
		"ypos"					"r75"	[$WIN32]
	}
	
	"PnlQuickJoinGroups"	[$WIN32]
	{
		"ControlName"			"QuickJoinGroupsPanel"
		"fieldName"				"PnlQuickJoinGroups"
		"ResourceFile"			"resource/UI/basemodui/QuickJoinGroups.res"
		"visible"				"0"
		"wide"					"500"
		"tall"					"300"
		"xpos"					"c0"
		"ypos"					"r75"
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

	"LblBranchDisclaimer"
	{
		"ControlName"		"Label"
		"fieldName"			"LblBranchDisclaimer"
		"xpos"				"r305"
		"ypos"				"5"
		"wide"				"300"
		"tall"				"150"
		"textAlignment"		"north-east"
		"wrap"				"1"
	}
}
