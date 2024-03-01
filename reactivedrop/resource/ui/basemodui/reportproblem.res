"Resource/UI/BaseModUI/ReportProblem.res"
{
	"ReportProblem"
	{
		"ControlName"		"ReportProblem"
		"fieldName"			"ReportProblem"
		"xpos"				"0"
		"ypos"				"0"
		"wide"				"f0"
		"tall"				"f0"
	}

	"BtnBack"
	{
		"ControlName"		"CNB_Button"
		"fieldName"			"BtnBack"
		"xpos"				"c-264"
		"ypos"				"r23"
		"zpos"				"3"
		"wide"				"117"
		"tall"				"27"
		"labelText"			"#nb_back"
		"command"			"Back"
		"textAlignment"		"center"
		"font"				"DefaultMedium"
		"fgcolor_override"	"113 142 181 255"
	}

	"LblTitle"
	{
		"ControlName"		"Label"
		"fieldName"			"LblTitle"
		"xpos"				"c-266"
		"ypos"				"60"
		"wide"				"250"
		"tall"				"24"
		"zpos"				"5"
		"labelText"			"#rd_reporting_title"
		"textAlignment"		"north-west"
		"font"				"DefaultExtraLarge"
		"fgcolor_override"	"224 224 224 255"
	}

	"HeaderFooter_Category"
	{
		"ControlName"		"CNB_Header_Footer"
		"fieldName"			"HeaderFooter_Category"
		"xpos"				"0"
		"ypos"				"0"
		"wide"				"f0"
		"tall"				"f0"
		"gradient_bar_enabled"	"1"
		"gradient_bar_ypos"	"80"
		"gradient_bar_tall"	"330"
	}

	"BtnMyAccount"
	{
		"ControlName"		"CBitmapButton"
		"fieldName"			"BtnMyAccount"
		"xpos"				"c-225"
		"ypos"				"100"
		"wide"				"100"
		"tall"				"115"
		"icon_xpos"			"0"
		"icon_ypos"			"0"
		"icon_wide"			"100"
		"icon_tall"			"100"
		"textAlignment"		"south"
		"font"				"DefaultMedium"
		"navRight"			"BtnServer"
		"navDown"			"BtnGameBug"
		"labelText"			"#rd_reporting_category_my_account"
		"command"			"MyAccount"
		"enabledImage"
		{
			"material"		"vgui/reporting/category_my_account"
			"color"			"83 148 192 255"
		}
		"mouseOverImage"
		{
			"material"		"vgui/reporting/category_my_account"
			"color"			"255 255 255 255"
		}
		"pressedImage"
		{
			"material"		"vgui/reporting/category_my_account"
			"color"			"255 255 255 255"
		}
		"disabledImage"
		{
			"material"		"vgui/reporting/category_my_account"
			"color"			"96 96 96 255"
		}
	}

	"LblMyAccount"
	{
		"ControlName"		"Label"
		"fieldName"			"LblMyAccount"
		"xpos"				"c-250"
		"ypos"				"215"
		"wide"				"150"
		"tall"				"30"
		"centerwrap"		"1"
		"labelText"			"#rd_reporting_category_my_account_desc"
		"fgcolor_override"	"192 192 192 255"
	}

	"BtnServer"
	{
		"ControlName"		"CBitmapButton"
		"fieldName"			"BtnServer"
		"xpos"				"c-50"
		"ypos"				"100"
		"wide"				"100"
		"tall"				"115"
		"icon_xpos"			"0"
		"icon_ypos"			"0"
		"icon_wide"			"100"
		"icon_tall"			"100"
		"textAlignment"		"south"
		"font"				"DefaultMedium"
		"navLeft"			"BtnMyAccount"
		"navRight"			"BtnPlayer"
		"navDown"			"BtnGameBug"
		"labelText"			"#rd_reporting_category_server"
		"command"			"Server"
		"enabledImage"
		{
			"material"		"vgui/reporting/category_server"
			"color"			"83 148 192 255"
		}
		"mouseOverImage"
		{
			"material"		"vgui/reporting/category_server"
			"color"			"255 255 255 255"
		}
		"pressedImage"
		{
			"material"		"vgui/reporting/category_server"
			"color"			"255 255 255 255"
		}
		"disabledImage"
		{
			"material"		"vgui/reporting/category_server"
			"color"			"96 96 96 255"
		}
	}

	"LblServer"
	{
		"ControlName"		"Label"
		"fieldName"			"LblServer"
		"xpos"				"c-75"
		"ypos"				"215"
		"wide"				"150"
		"tall"				"30"
		"centerwrap"		"1"
		"labelText"			"#rd_reporting_category_server_desc"
		"fgcolor_override"	"192 192 192 255"
	}

	"BtnPlayer"
	{
		"ControlName"		"CBitmapButton"
		"fieldName"			"BtnPlayer"
		"xpos"				"c125"
		"ypos"				"100"
		"wide"				"100"
		"tall"				"115"
		"icon_xpos"			"0"
		"icon_ypos"			"0"
		"icon_wide"			"100"
		"icon_tall"			"100"
		"textAlignment"		"south"
		"font"				"DefaultMedium"
		"navLeft"			"BtnServer"
		"navDown"			"BtnOther"
		"labelText"			"#rd_reporting_category_player"
		"command"			"Player"
		"enabledImage"
		{
			"material"		"vgui/reporting/category_player"
			"color"			"83 148 192 255"
		}
		"mouseOverImage"
		{
			"material"		"vgui/reporting/category_player"
			"color"			"255 255 255 255"
		}
		"pressedImage"
		{
			"material"		"vgui/reporting/category_player"
			"color"			"255 255 255 255"
		}
		"disabledImage"
		{
			"material"		"vgui/reporting/category_player"
			"color"			"96 96 96 255"
		}
	}

	"LblPlayer"
	{
		"ControlName"		"Label"
		"fieldName"			"LblPlayer"
		"xpos"				"c100"
		"ypos"				"215"
		"wide"				"150"
		"tall"				"30"
		"centerwrap"		"1"
		"labelText"			"#rd_reporting_category_player_desc"
		"fgcolor_override"	"192 192 192 255"
	}

	"BtnGameBug"
	{
		"ControlName"		"CBitmapButton"
		"fieldName"			"BtnGameBug"
		"xpos"				"c-137.5"
		"ypos"				"250"
		"wide"				"100"
		"tall"				"115"
		"icon_xpos"			"0"
		"icon_ypos"			"0"
		"icon_wide"			"100"
		"icon_tall"			"100"
		"textAlignment"		"south"
		"font"				"DefaultMedium"
		"navRight"			"BtnOther"
		"navUp"				"BtnMyAccount"
		"navDown"			"BtnResume"
		"labelText"			"#rd_reporting_category_game_bug"
		"command"			"GameBug"
		"enabledImage"
		{
			"material"		"vgui/reporting/category_game_bug"
			"color"			"83 148 192 255"
		}
		"mouseOverImage"
		{
			"material"		"vgui/reporting/category_game_bug"
			"color"			"255 255 255 255"
		}
		"pressedImage"
		{
			"material"		"vgui/reporting/category_game_bug"
			"color"			"255 255 255 255"
		}
		"disabledImage"
		{
			"material"		"vgui/reporting/category_game_bug"
			"color"			"96 96 96 255"
		}
	}

	"LblGameBug"
	{
		"ControlName"		"Label"
		"fieldName"			"LblGameBug"
		"xpos"				"c-162.5"
		"ypos"				"365"
		"wide"				"150"
		"tall"				"30"
		"centerwrap"		"1"
		"labelText"			"#rd_reporting_category_game_bug_desc"
		"fgcolor_override"	"192 192 192 255"
	}

	"BtnOther"
	{
		"ControlName"		"CBitmapButton"
		"fieldName"			"BtnOther"
		"xpos"				"c37.5"
		"ypos"				"250"
		"wide"				"100"
		"tall"				"115"
		"icon_xpos"			"0"
		"icon_ypos"			"0"
		"icon_wide"			"100"
		"icon_tall"			"100"
		"textAlignment"		"south"
		"font"				"DefaultMedium"
		"navLeft"			"BtnGameBug"
		"navUp"				"BtnPlayer"
		"navDown"			"BtnResume"
		"labelText"			"#rd_reporting_category_other"
		"command"			"Other"
		"enabledImage"
		{
			"material"		"vgui/reporting/category_other"
			"color"			"83 148 192 255"
		}
		"mouseOverImage"
		{
			"material"		"vgui/reporting/category_other"
			"color"			"255 255 255 255"
		}
		"pressedImage"
		{
			"material"		"vgui/reporting/category_other"
			"color"			"255 255 255 255"
		}
		"disabledImage"
		{
			"material"		"vgui/reporting/category_other"
			"color"			"96 96 96 255"
		}
	}

	"LblOther"
	{
		"ControlName"		"Label"
		"fieldName"			"LblOther"
		"xpos"				"c12.5"
		"ypos"				"365"
		"wide"				"150"
		"tall"				"30"
		"centerwrap"		"1"
		"labelText"			"#rd_reporting_category_other_desc"
		"fgcolor_override"	"192 192 192 255"
	}

	"LblLastProgress"
	{
		"ControlName"		"Label"
		"fieldName"			"LblLastProgress"
		"xpos"				"c-150"
		"ypos"				"420"
		"wide"				"300"
		"tall"				"20"
		"centerwrap"		"1"
		"fgcolor_override"	"224 224 224 255"
	}

	"BtnResume"
	{
		"ControlName"		"CNB_Button"
		"fieldName"			"BtnResume"
		"xpos"				"c-100"
		"ypos"				"r23"
		"zpos"				"3"
		"wide"				"200"
		"tall"				"27"
		"labelText"			"#rd_reporting_resume"
		"command"			"Resume"
		"textAlignment"		"center"
		"font"				"DefaultMedium"
		"fgcolor_override"	"113 142 181 255"
	}

	"HeaderFooter_Wait"
	{
		"ControlName"		"CNB_Header_Footer"
		"fieldName"			"HeaderFooter_Wait"
		"xpos"				"0"
		"ypos"				"0"
		"wide"				"f0"
		"tall"				"f0"
		"gradient_bar_enabled"	"1"
		"gradient_bar_ypos"	"80"
		"gradient_bar_tall"	"140"
	}

	"ImgSpinner"
	{
		"ControlName"		"ImagePanel"
		"fieldName"			"ImgSpinner"
		"xpos"				"r50"
		"ypos"				"100"
		"wide"				"32"
		"tall"				"32"
		"image"				"common/swarm_cycle_anim"
		"scaleImage"		"1"
	}

	"LblWait"
	{
		"ControlName"		"Label"
		"fieldName"			"LblWait"
		"xpos"				"c-150"
		"ypos"				"100"
		"wide"				"300"
		"tall"				"60"
		"centerwrap"		"1"
		"font"				"DefaultExtraLarge"
	}

	"LblDontWait"
	{
		"ControlName"		"Label"
		"fieldName"			"LblDontWait"
		"xpos"				"c-150"
		"ypos"				"180"
		"wide"				"300"
		"tall"				"40"
		"centerwrap"		"1"
		"labelText"			"#rd_reporting_continues_in_background"
		"fgcolor_override"	"192 192 192 255"
	}

	"HeaderFooter_Player"
	{
		"ControlName"		"CNB_Header_Footer"
		"fieldName"			"HeaderFooter_Player"
		"xpos"				"0"
		"ypos"				"0"
		"wide"				"f0"
		"tall"				"f0"
		"gradient_bar_enabled"	"1"
		"gradient_bar_ypos"	"80"
		"gradient_bar_tall"	"320"
	}

	"GplPlayerChoices"
	{
		"ControlName"		"GenericPanelList"
		"fieldName"			"GplPlayerChoices"
		"xpos"				"c-109"
		"ypos"				"90"
		"wide"				"217"
		"tall"				"300"
		"bgcolor_override"	"0 0 0 64"
	}

	"BtnSelectPlayer"
	{
		"ControlName"		"CNB_Button"
		"fieldName"			"BtnSelectPlayer"
		"xpos"				"c147"
		"ypos"				"r23"
		"zpos"				"3"
		"wide"				"117"
		"tall"				"27"
		"labelText"			"#asw_button_next"
		"command"			"SelectPlayer"
		"textAlignment"		"center"
		"font"				"DefaultMedium"
		"fgcolor_override"	"113 142 181 255"
	}

	"HeaderFooter_Report"
	{
		"ControlName"		"CNB_Header_Footer"
		"fieldName"			"HeaderFooter_Report"
		"xpos"				"0"
		"ypos"				"0"
		"wide"				"f0"
		"tall"				"f0"
		"gradient_bar_enabled"	"1"
		"gradient_bar_ypos"	"80"
		"gradient_bar_tall"	"310"
	}

	"LblReportingPlayer"
	{
		"ControlName"		"Label"
		"fieldName"			"LblReportingPlayer"
		"xpos"				"c115"
		"ypos"				"62"
		"wide"				"185"
		"tall"				"10"
		"labelText"			"#rd_reporting_player_label"
		"fgcolor_override"	"224 224 224 255"
		"font"				"DefaultVerySmall"
		"textAlignment"		"north-west"
	}

	"ImgPlayerAvatar"
	{
		"ControlName"		"CAvatarImagePanel"
		"fieldName"			"ImgPlayerAvatar"
		"xpos"				"c93"
		"ypos"				"64"
		"wide"				"20"
		"tall"				"20"
		"legacy_padding"	"0"
	}

	"LblPlayerName"
	{
		"ControlName"		"Label"
		"fieldName"			"LblPlayerName"
		"xpos"				"c115"
		"ypos"				"68"
		"wide"				"185"
		"tall"				"15"
		"fgcolor_override"	"224 224 224 255"
		"font"				"DefaultMedium"
		"textAlignment"		"north-west"
	}

	"SettingSubCategory"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingSubCategory"
		"xpos"				"c-250"
		"ypos"				"90"
		"wide"				"500"
		"tall"				"10"
		"ResourceFile"		"resource/ui/option_report_radio.res"
	}

	"LblInstructions"
	{
		"ControlName"		"Label"
		"fieldName"			"LblInstructions"
		"xpos"				"c-250"
		"ypos"				"105"
		"wide"				"500"
		"tall"				"30"
		"wrap"				"1"
		"fgcolor_override"	"255 255 255 255"
	}

	"TxtDescription"
	{
		"ControlName"		"TextEntry"
		"fieldName"			"TxtDescription"
		"xpos"				"c-250"
		"ypos"				"140"
		"wide"				"500"
		"tall"				"150"
		"unicode"			"1"
		"maxchars"			"8000"
	}

	"ImgScreenshot0"
	{
		"ControlName"		"ImagePanel"
		"fieldName"			"ImgScreenshot0"
		"xpos"				"c-250"
		"ypos"				"300"
		"wide"				"48"
		"tall"				"27"
		"scaleImage"		"1"
	}

	"ChkScreenshot0"
	{
		"ControlName"		"CheckButton"
		"fieldName"			"ChkScreenshot0"
		"xpos"				"c-200"
		"ypos"				"303"
		"wide"				"145"
		"tall"				"20"
		"labelText"			"#rd_reporting_include_this_screenshot"
	}

	"LblReportContents"
	{
		"ControlName"		"MultiFontRichText"
		"fieldName"			"LblReportContents"
		"xpos"				"c-50"
		"ypos"				"300"
		"wide"				"300"
		"tall"				"80"
	}

	"BtnSubmit"
	{
		"ControlName"		"CNB_Button"
		"fieldName"			"BtnSubmit"
		"xpos"				"c147"
		"ypos"				"r23"
		"zpos"				"3"
		"wide"				"117"
		"tall"				"27"
		"labelText"			"#rd_reporting_submit"
		"command"			"Submit"
		"textAlignment"		"center"
		"font"				"DefaultMedium"
		"fgcolor_override"	"113 142 181 255"
	}
}
