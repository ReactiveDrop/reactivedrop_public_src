"Resource/UI/CRD_VGUI_Quick_Report_Panel.res"
{
	"QuickReportPanel"
	{
		"ControlName"		"CRD_VGUI_Quick_Report_Panel"
		"fieldName"			"QuickReportPanel"
		"xpos"				"0"
		"ypos"				"0"
		"zpos"				"500"
		"wide"				"f0"
		"tall"				"f0"
	}

	"HeaderFooter"
	{
		"ControlName"		"CNB_Header_Footer"
		"fieldName"			"HeaderFooter"
		"xpos"				"0"
		"ypos"				"0"
		"zpos"				"-2"
		"wide"				"f0"
		"tall"				"f0"
	}

	"BackgroundCommend"
	{
		"ControlName"		"Panel"
		"fieldName"			"BackgroundCommend"
		"xpos"				"c-200"
		"ypos"				"115"
		"zpos"				"-1"
		"wide"				"400"
		"tall"				"130"
		"bgcolor_override"	"12 56 34 192"
		"paintbackgroundtype"	"2"
	}

	"BackgroundReport"
	{
		"ControlName"		"Panel"
		"fieldName"			"BackgroundReport"
		"xpos"				"c-200"
		"ypos"				"250"
		"zpos"				"-1"
		"wide"				"400"
		"tall"				"130"
		"bgcolor_override"	"56 34 12 192"
		"paintbackgroundtype"	"2"
	}

	"ImgPlayerAvatar"
	{
		"ControlName"		"CAvatarImagePanel"
		"fieldName"			"ImgPlayerAvatar"
		"xpos"				"c-200"
		"ypos"				"65"
		"wide"				"32"
		"tall"				"32"
		"legacy_padding"	"0"
	}

	"ImgTitle"
	{
		"ControlName"		"ImagePanel"
		"fieldName"			"ImgTitle"
		"xpos"				"c-166"
		"ypos"				"69"
		"wide"				"10"
		"tall"				"10"
		"image"				"briefing/quick_report_icon"
		"scaleImage"		"1"
	}

	"LblTitle"
	{
		"ControlName"		"Label"
		"fieldName"			"LblTitle"
		"xpos"				"c-154"
		"ypos"				"69"
		"wide"				"300"
		"tall"				"10"
		"labelText"			"#rd_quick_report_commend_or_report"
		"textAlignment"		"west"
		"font"				"Default"
		"allcaps"			"1"
		"fgcolor_override"	"128 128 128 255"
	}

	"LblPlayerName"
	{
		"ControlName"		"Label"
		"fieldName"			"LblPlayerName"
		"xpos"				"c-166"
		"ypos"				"75"
		"wide"				"300"
		"tall"				"20"
		"textAlignment"		"west"
		"font"				"DefaultLarge"
		"fgcolor_override"	"255 255 255 255"
	}

	"LblProgress"
	{
		"ControlName"		"Label"
		"fieldName"			"LblProgress"
		"xpos"				"c-125"
		"ypos"				"405"
		"wide"				"250"
		"tall"				"20"
		"textAlignment"		"center"
		"font"				"Default"
		"fgcolor_override"	"255 255 255 255"
	}

	"BtnBack"
	{
		"ControlName"		"CNB_Button"
		"fieldName"			"BtnBack"
		"xpos"				"c-264"
		"ypos"				"r23"
		"wide"				"117"
		"tall"				"27"
		"labelText"			"#nb_back"
		"font"				"DefaultMedium"
		"command"			"Back"
		"textAlignment"		"center"
		"fgcolor_override"	"113 142 181 255"
	}

	"LblCommend0"
	{
		"ControlName"		"Label"
		"fieldName"			"LblCommend0"
		"xpos"				"c-170"
		"ypos"				"120"
		"wide"				"300"
		"tall"				"20"
		"labelText"			"#rd_quick_report_commend"
		"font"				"DefaultExtraLargeBlur"
		"fgcolor_override"	"12 156 34 255"
		"textinsetx"		"5"
		"use_proportional_insets"	"1"
	}

	"LblReport0"
	{
		"ControlName"		"Label"
		"fieldName"			"LblReport0"
		"xpos"				"c-170"
		"ypos"				"255"
		"wide"				"300"
		"tall"				"20"
		"labelText"			"#rd_quick_report_report"
		"font"				"DefaultExtraLargeBlur"
		"fgcolor_override"	"156 34 12 255"
		"textinsetx"		"5"
		"use_proportional_insets"	"1"
	}

	"LblCommend1"
	{
		"ControlName"		"Label"
		"fieldName"			"LblCommend1"
		"xpos"				"c-170"
		"ypos"				"120"
		"zpos"				"1"
		"wide"				"300"
		"tall"				"20"
		"labelText"			"#rd_quick_report_commend"
		"font"				"DefaultExtraLarge"
		"fgcolor_override"	"255 255 255 255"
		"textinsetx"		"5"
		"use_proportional_insets"	"1"
	}

	"LblReport1"
	{
		"ControlName"		"Label"
		"fieldName"			"LblReport1"
		"xpos"				"c-170"
		"ypos"				"255"
		"zpos"				"1"
		"wide"				"300"
		"tall"				"20"
		"labelText"			"#rd_quick_report_report"
		"font"				"DefaultExtraLarge"
		"fgcolor_override"	"255 255 255 255"
		"textinsetx"		"5"
		"use_proportional_insets"	"1"
	}

	"BtnCommendFriendly"
	{
		"ControlName"		"CBitmapButton"
		"fieldName"			"BtnCommendFriendly"
		"xpos"				"c-157"
		"ypos"				"155"
		"wide"				"64"
		"tall"				"64"
		"command"			"CommendFriendly"
		"labelText"			" "
		"enabledImage"
		{
			"material"		"vgui/reporting/quick_friendly"
			"color"			"224 224 224 255"
		}
		"mouseOverImage"
		{
			"material"		"vgui/reporting/quick_friendly"
			"color"			"255 255 255 255"
		}
		"pressedImage"
		{
			"material"		"vgui/reporting/quick_friendly"
			"color"			"255 255 255 255"
		}
		"disabledImage"
		{
			"material"		"vgui/reporting/quick_friendly"
			"color"			"96 96 96 255"
		}
	}

	"LblCommendFriendly"
	{
		"ControlName"		"Label"
		"fieldName"			"LblCommendFriendly"
		"xpos"				"c-175"
		"ypos"				"221"
		"wide"				"100"
		"tall"				"14"
		"labelText"			"#rd_quick_report_friendly"
		"textAlignment"		"north"
		"font"				"DefaultMedium"
		"fgcolor_override"	"128 128 128 255"
	}

	"BtnCommendLeader"
	{
		"ControlName"		"CBitmapButton"
		"fieldName"			"BtnCommendLeader"
		"xpos"				"c-32"
		"ypos"				"155"
		"wide"				"64"
		"tall"				"64"
		"command"			"CommendLeader"
		"labelText"			" "
		"enabledImage"
		{
			"material"		"vgui/reporting/quick_leader"
			"color"			"224 224 224 255"
		}
		"mouseOverImage"
		{
			"material"		"vgui/reporting/quick_leader"
			"color"			"255 255 255 255"
		}
		"pressedImage"
		{
			"material"		"vgui/reporting/quick_leader"
			"color"			"255 255 255 255"
		}
		"disabledImage"
		{
			"material"		"vgui/reporting/quick_leader"
			"color"			"96 96 96 255"
		}
	}

	"LblCommendLeader"
	{
		"ControlName"		"Label"
		"fieldName"			"LblCommendLeader"
		"xpos"				"c-50"
		"ypos"				"221"
		"wide"				"100"
		"tall"				"14"
		"labelText"			"#rd_quick_report_leader"
		"textAlignment"		"north"
		"font"				"DefaultMedium"
		"fgcolor_override"	"128 128 128 255"
	}

	"BtnCommendTeacher"
	{
		"ControlName"		"CBitmapButton"
		"fieldName"			"BtnCommendTeacher"
		"xpos"				"c93"
		"ypos"				"155"
		"wide"				"64"
		"tall"				"64"
		"command"			"CommendTeacher"
		"labelText"			" "
		"enabledImage"
		{
			"material"		"vgui/reporting/quick_teacher"
			"color"			"224 224 224 255"
		}
		"mouseOverImage"
		{
			"material"		"vgui/reporting/quick_teacher"
			"color"			"255 255 255 255"
		}
		"pressedImage"
		{
			"material"		"vgui/reporting/quick_teacher"
			"color"			"255 255 255 255"
		}
		"disabledImage"
		{
			"material"		"vgui/reporting/quick_teacher"
			"color"			"96 96 96 255"
		}
	}

	"LblCommendTeacher"
	{
		"ControlName"		"Label"
		"fieldName"			"LblCommendTeacher"
		"xpos"				"c75"
		"ypos"				"221"
		"wide"				"100"
		"tall"				"14"
		"labelText"			"#rd_quick_report_teacher"
		"textAlignment"		"north"
		"font"				"DefaultMedium"
		"fgcolor_override"	"128 128 128 255"
	}

	"BtnReportCheating"
	{
		"ControlName"		"CBitmapButton"
		"fieldName"			"BtnReportCheating"
		"xpos"				"c-157"
		"ypos"				"290"
		"wide"				"64"
		"tall"				"64"
		"command"			"ReportCheating"
		"labelText"			" "
		"enabledImage"
		{
			"material"		"vgui/reporting/quick_cheating"
			"color"			"224 224 224 255"
		}
		"mouseOverImage"
		{
			"material"		"vgui/reporting/quick_cheating"
			"color"			"255 255 255 255"
		}
		"pressedImage"
		{
			"material"		"vgui/reporting/quick_cheating"
			"color"			"255 255 255 255"
		}
		"disabledImage"
		{
			"material"		"vgui/reporting/quick_cheating"
			"color"			"96 96 96 255"
		}
	}

	"LblReportCheating"
	{
		"ControlName"		"Label"
		"fieldName"			"LblReportCheating"
		"xpos"				"c-175"
		"ypos"				"356"
		"wide"				"100"
		"tall"				"14"
		"labelText"			"#rd_quick_report_cheating"
		"textAlignment"		"north"
		"font"				"DefaultMedium"
		"fgcolor_override"	"128 128 128 255"
	}

	"BtnReportGriefing"
	{
		"ControlName"		"CBitmapButton"
		"fieldName"			"BtnReportGriefing"
		"xpos"				"c-32"
		"ypos"				"290"
		"wide"				"64"
		"tall"				"64"
		"command"			"ReportGriefing"
		"labelText"			" "
		"enabledImage"
		{
			"material"		"vgui/reporting/quick_griefing"
			"color"			"224 224 224 255"
		}
		"mouseOverImage"
		{
			"material"		"vgui/reporting/quick_griefing"
			"color"			"255 255 255 255"
		}
		"pressedImage"
		{
			"material"		"vgui/reporting/quick_griefing"
			"color"			"255 255 255 255"
		}
		"disabledImage"
		{
			"material"		"vgui/reporting/quick_griefing"
			"color"			"96 96 96 255"
		}
	}

	"LblReportGriefing"
	{
		"ControlName"		"Label"
		"fieldName"			"LblReportGriefing"
		"xpos"				"c-50"
		"ypos"				"356"
		"wide"				"100"
		"tall"				"14"
		"labelText"			"#rd_quick_report_gameplay"
		"textAlignment"		"north"
		"font"				"DefaultMedium"
		"fgcolor_override"	"128 128 128 255"
	}

	"BtnReportCommunication"
	{
		"ControlName"		"CBitmapButton"
		"fieldName"			"BtnReportCommunication"
		"xpos"				"c93"
		"ypos"				"290"
		"wide"				"64"
		"tall"				"64"
		"command"			"ReportCommunication"
		"labelText"			" "
		"enabledImage"
		{
			"material"		"vgui/reporting/quick_communication"
			"color"			"224 224 224 255"
		}
		"mouseOverImage"
		{
			"material"		"vgui/reporting/quick_communication"
			"color"			"255 255 255 255"
		}
		"pressedImage"
		{
			"material"		"vgui/reporting/quick_communication"
			"color"			"255 255 255 255"
		}
		"disabledImage"
		{
			"material"		"vgui/reporting/quick_communication"
			"color"			"96 96 96 255"
		}
	}

	"LblReportCommunication"
	{
		"ControlName"		"Label"
		"fieldName"			"LblReportCommunication"
		"xpos"				"c75"
		"ypos"				"356"
		"wide"				"100"
		"tall"				"14"
		"labelText"			"#rd_quick_report_communication"
		"textAlignment"		"north"
		"font"				"DefaultMedium"
		"fgcolor_override"	"128 128 128 255"
	}
}
