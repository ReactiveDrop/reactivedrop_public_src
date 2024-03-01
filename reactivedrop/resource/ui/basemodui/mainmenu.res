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
		"PaintBackground"		"1"
		"hoiaf_timer_offset"	"5"
		"navUp"					"BtnMultiplayer"
		"navDown"				"BtnMultiplayer"
		"navLeft"				"BtnMultiplayer"
		"navRight"				"BtnMultiplayer"
	}

	"TopBar"
	{
		"ControlName"			"CRD_VGUI_Main_Menu_Top_Bar"
		"fieldName"				"TopBar"
		"visible"				"1"
		"enabled"				"1"
		"navUp"					"BtnLogo"
		"navDown"				"CommanderMiniProfile"
	}

	"CommanderMiniProfile"
	{
		"ControlName"			"CRD_VGUI_Commander_Mini_Profile"
		"fieldName"				"CommanderMiniProfile"
		"xpos"					"10"
		"ypos"					"40"
		"wide"					"145"
		"tall"					"72"
		"tabPosition"			"0"
		"navRight"				"BtnHoIAFTimer"
		"navUp"					"TopBar"
		"navDown"				"BtnMultiplayer"
		"showLocalPlayer"		"1"
		"isButton"				"1"
	}

	"BtnMultiplayer"
	{
		"ControlName"			"BaseModHybridButton"
		"fieldName"				"BtnMultiplayer"
		"xpos"					"10"
		"ypos"					"125"
		"wide"					"145"
		"tall"					"30"
		"tabPosition"			"1"
		"navRight"				"BtnServerBrowser"
		"navUp"					"CommanderMiniProfile"
		"navDown"				"BtnSingleplayer"
		"labelText"				"#L4D360UI_FoudGames_CreateNew_campaign"
		"tooltiptext"			"#L4D360UI_MainMenu_CoOp_Tip"
		"style"					"ReactiveDropMainMenuBig"
		"command"				"CreateGame"
	}

	"BtnSingleplayer"
	{
		"ControlName"			"BaseModHybridButton"
		"fieldName"				"BtnSingleplayer"
		"xpos"					"10"
		"ypos"					"160"
		"wide"					"130"
		"tall"					"25"
		"navRight"				"BtnHoIAFTimer"
		"navUp"					"BtnMultiplayer"
		"navDown"				"PnlQuickJoinPublic"
		"labelText"				"#L4D360UI_MainMenu_PlaySolo"
		"tooltiptext"			"#L4D360UI_MainMenu_PlaySolo_Tip"
		"style"					"ReactiveDropMainMenu"
		"command"				"SoloPlay"
	}

	"PnlQuickJoinPublic"
	{
		"ControlName"			"QuickJoinPublicPanel"
		"fieldName"				"PnlQuickJoinPublic"
		"ResourceFile"			"resource/UI/basemodui/QuickJoinGroups.res"
		"navRight"				"BtnNewsShowcase"
		"navUp"					"BtnSingleplayer"
		"navDown"				"PnlQuickJoin"
		"xpos"					"10"
		"ypos"					"190"
		"wide"					"130"
		"tall"					"90"
	}

	"PnlQuickJoin"
	{
		"ControlName"			"QuickJoinPanel"
		"fieldName"				"PnlQuickJoin"
		"ResourceFile"			"resource/UI/basemodui/QuickJoin.res"
		"navRight"				"BtnNewsShowcase"
		"navUp"					"PnlQuickJoinPublic"
		"navDown"				"BtnWorkshopShowcase"
		"xpos"					"10"
		"ypos"					"285"
		"wide"					"130"
		"tall"					"90"
	}

	"BtnWorkshopShowcase"
	{
		"ControlName"			"BaseModHybridButton"
		"fieldName"				"BtnWorkshopShowcase"
		"xpos"					"10"
		"ypos"					"380"
		"wide"					"130"
		"tall"					"74"
		"navRight"				"BtnNewsShowcase"
		"navUp"					"PnlQuickJoin"
		"style"					"ReactiveDropMainMenuShowcase"
		"command"				"WorkshopShowcase"
	}

	"HoIAFTop1"
	{
		"ControlName"			"CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry_Large"
		"fieldName"				"HoIAFTop1"
		"xpos"					"r210"
		"ypos"					"40"
		"wide"					"200"
		"tall"					"20"
		"navLeft"				"BtnMultiplayer"
		"navUp"					"TopBar"
		"navDown"				"HoIAFTop2"
		"command"				"HoIAFTop1"
	}

	"HoIAFTop2"
	{
		"ControlName"			"CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry"
		"fieldName"				"HoIAFTop2"
		"xpos"					"r200"
		"ypos"					"60"
		"wide"					"190"
		"tall"					"15"
		"navLeft"				"BtnMultiplayer"
		"navUp"					"HoIAFTop1"
		"navDown"				"HoIAFTop3"
		"command"				"HoIAFTop2"
	}

	"HoIAFTop3"
	{
		"ControlName"			"CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry"
		"fieldName"				"HoIAFTop3"
		"xpos"					"r200"
		"ypos"					"75"
		"wide"					"190"
		"tall"					"15"
		"navLeft"				"BtnMultiplayer"
		"navUp"					"HoIAFTop2"
		"navDown"				"HoIAFTop4"
		"command"				"HoIAFTop3"
	}

	"HoIAFTop4"
	{
		"ControlName"			"CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry"
		"fieldName"				"HoIAFTop4"
		"xpos"					"r200"
		"ypos"					"90"
		"wide"					"190"
		"tall"					"15"
		"navLeft"				"BtnMultiplayer"
		"navUp"					"HoIAFTop3"
		"navDown"				"HoIAFTop5"
		"command"				"HoIAFTop4"
	}

	"HoIAFTop5"
	{
		"ControlName"			"CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry"
		"fieldName"				"HoIAFTop5"
		"xpos"					"r200"
		"ypos"					"105"
		"wide"					"190"
		"tall"					"15"
		"navLeft"				"BtnMultiplayer"
		"navUp"					"HoIAFTop4"
		"navDown"				"HoIAFTop6"
		"command"				"HoIAFTop5"
	}

	"HoIAFTop6"
	{
		"ControlName"			"CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry"
		"fieldName"				"HoIAFTop6"
		"xpos"					"r200"
		"ypos"					"120"
		"wide"					"190"
		"tall"					"15"
		"navLeft"				"BtnMultiplayer"
		"navUp"					"HoIAFTop5"
		"navDown"				"HoIAFTop7"
		"command"				"HoIAFTop6"
	}

	"HoIAFTop7"
	{
		"ControlName"			"CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry"
		"fieldName"				"HoIAFTop7"
		"xpos"					"r200"
		"ypos"					"135"
		"wide"					"190"
		"tall"					"15"
		"navLeft"				"BtnMultiplayer"
		"navUp"					"HoIAFTop6"
		"navDown"				"HoIAFTop8"
		"command"				"HoIAFTop7"
	}

	"HoIAFTop8"
	{
		"ControlName"			"CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry"
		"fieldName"				"HoIAFTop8"
		"xpos"					"r200"
		"ypos"					"150"
		"wide"					"190"
		"tall"					"15"
		"navLeft"				"BtnMultiplayer"
		"navUp"					"HoIAFTop7"
		"navDown"				"HoIAFTop9"
		"command"				"HoIAFTop8"
	}

	"HoIAFTop9"
	{
		"ControlName"			"CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry"
		"fieldName"				"HoIAFTop9"
		"xpos"					"r200"
		"ypos"					"165"
		"wide"					"190"
		"tall"					"15"
		"navLeft"				"BtnMultiplayer"
		"navUp"					"HoIAFTop8"
		"navDown"				"HoIAFTop10"
		"command"				"HoIAFTop9"
	}

	"HoIAFTop10"
	{
		"ControlName"			"CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry"
		"fieldName"				"HoIAFTop10"
		"xpos"					"r200"
		"ypos"					"180"
		"wide"					"190"
		"tall"					"15"
		"navLeft"				"BtnMultiplayer"
		"navUp"					"HoIAFTop9"
		"navDown"				"BtnHoIAFTimer"
		"command"				"HoIAFTop10"
	}

	"BtnHoIAFTimer"
	{
		"ControlName"			"BaseModHybridButton"
		"fieldName"				"BtnHoIAFTimer"
		"xpos"					"r210"
		"ypos"					"200"
		"wide"					"200"
		"tall"					"20"
		"navLeft"				"BtnMultiplayer"
		"navUp"					"HoIAFTop10"
		"navDown"				"BtnEventTimer3"
		"style"					"ReactiveDropMainMenuTimer"
		"command"				"ShowHoIAF"
	}

	"BtnEventTimer3"
	{
		"ControlName"			"BaseModHybridButton"
		"fieldName"				"BtnEventTimer3"
		"xpos"					"r210"
		"ypos"					"229"
		"wide"					"200"
		"tall"					"25"
		"navLeft"				"BtnWorkshopShowcase"
		"navUp"					"BtnHoIAFTimer"
		"navDown"				"BtnEventTimer2"
		"style"					"ReactiveDropMainMenuTimer"
		"command"				"EventTimer3"
	}

	"BtnEventTimer2"
	{
		"ControlName"			"BaseModHybridButton"
		"fieldName"				"BtnEventTimer2"
		"xpos"					"r210"
		"ypos"					"254"
		"wide"					"200"
		"tall"					"25"
		"navLeft"				"BtnWorkshopShowcase"
		"navUp"					"BtnEventTimer3"
		"navDown"				"BtnEventTimer1"
		"style"					"ReactiveDropMainMenuTimer"
		"command"				"EventTimer2"
	}

	"BtnEventTimer1"
	{
		"ControlName"			"BaseModHybridButton"
		"fieldName"				"BtnEventTimer1"
		"xpos"					"r210"
		"ypos"					"279"
		"wide"					"200"
		"tall"					"25"
		"navLeft"				"BtnWorkshopShowcase"
		"navUp"					"BtnEventTimer2"
		"navDown"				"BtnNewsShowcase"
		"style"					"ReactiveDropMainMenuTimer"
		"command"				"EventTimer1"
	}

	"BtnNewsShowcase"
	{
		"ControlName"			"BaseModHybridButton"
		"fieldName"				"BtnNewsShowcase"
		"xpos"					"r210"
		"ypos"					"309"
		"wide"					"200"
		"tall"					"115"
		"navLeft"				"BtnWorkshopShowcase"
		"navUp"					"BtnEventTimer1"
		"navDown"				"BtnUpdateNotes"
		"style"					"ReactiveDropMainMenuShowcase"
		"command"				"NewsShowcase"
	}

	"BtnUpdateNotes"
	{
		"ControlName"			"BaseModHybridButton"
		"fieldName"				"BtnUpdateNotes"
		"xpos"					"r210"
		"ypos"					"429"
		"wide"					"200"
		"tall"					"25"
		"navLeft"				"BtnWorkshopShowcase"
		"navUp"					"BtnNewsShowcase"
		"style"					"ReactiveDropMainMenu"
		"command"				"UpdateNotes"
	}

	"WorkshopDownloadProgress"
	{
		"ControlName"		"CRD_VGUI_Workshop_Download_Progress"
		"fieldName"			"WorkshopDownloadProgress"
		"xpos"				"145"
		"ypos"				"409"
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
		"xpos"				"160"
		"ypos"				"40"
		"wide"				"225"	[!$WIN32WIDE]
		"wide"				"300"	[$WIN32WIDE]
		"tall"				"60"
		"textAlignment"		"north-west"
		"wrap"				"1"
		"fgcolor_override"	"192 192 192 255"
	}

	"StockTickerHelper"
	{
		"ControlName"		"CRD_VGUI_Stock_Ticker_Helper"
		"fieldName"			"StockTickerHelper"
		"visible"			"1"
		"enabled"			"1"
	}

	"PnlServerBrowserSpacer"
	{
		"ControlName"		"Panel"
		"fieldName"			"PnlServerBrowserSpacer"
		"pin_to_sibling"	"BtnMultiplayer"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"1"
		"wide"					"5"
		"tall"					"0"
	}

	"BtnServerBrowser"
	{
		"ControlName"		"CBitmapButton"
		"fieldName"			"BtnServerBrowser"
		"pin_to_sibling"	"PnlServerBrowserSpacer"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"1"
		"wide"				"30"
		"tall"				"30"
		"labelText"			" "
		"navLeft"			"BtnMultiplayer"
		"navRight"			"BtnHoIAFTimer"
		"navUp"				"CommanderMiniProfile"
		"navDown"			"BtnSingleplayer"
		"auto_focus"		"1"
		"command"			"OpenServerBrowser"
		"enabledImage"
		{
			"material"		"vgui/swarm/community_servers_icon"
			"color"			"255 255 255 64"
		}
		"mouseOverImage"
		{
			"material"		"vgui/swarm/community_servers_icon"
			"color"			"83 148 192 255"
		}
		"pressedImage"
		{
			"material"		"vgui/swarm/community_servers_icon"
			"color"			"169 213 255 255"
		}
		"disabledImage"
		{
			"material"		"vgui/swarm/community_servers_icon"
			"color"			"64 64 64 64"
		}
	}
}
