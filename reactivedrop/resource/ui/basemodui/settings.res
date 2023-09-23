"Resource/UI/BaseModUI/Settings.res"
{
	"Settings"
	{
		"ControlName"		"Frame"
		"fieldName"			"Settings"
		"xpos"				"0"
		"ypos"				"0"
		"wide"				"f0"
		"tall"				"f0"
		"visible"			"1"
		"enabled"			"1"
	}

	"TopBar"
	{
		"ControlName"			"CRD_VGUI_Main_Menu_Top_Bar"
		"fieldName"				"TopBar"
		"visible"				"1"
		"enabled"				"1"
		"navUp"					"BtnLogo"
		"navDown"				"BtnControls"
	}

	"StockTickerHelper"
	{
		"ControlName"		"CRD_VGUI_Stock_Ticker_Helper"
		"fieldName"			"StockTickerHelper"
		"visible"			"1"
		"enabled"			"1"
	}

	"BtnControls"
	{
		"ControlName"		"BaseModHybridButton"
		"fieldName"			"BtnControls"
		"xpos"				"c-283"	[!$WIN32WIDE]
		"xpos"				"c-303"	[$WIN32WIDE]
		"ypos"				"42"
		"zpos"				"5"
		"wide"				"64"
		"tall"				"20"
		"labelText"			"#rd_settings_controls"
		"style"				"ReactiveDropMainMenuTop"
		"command"			"Controls"
		"navLeft"			"BtnControls"
		"navRight"			"BtnOptions1"
		"navUp"				"TopBar"
	}

	"BtnOptions1"
	{
		"ControlName"		"BaseModHybridButton"
		"fieldName"			"BtnOptions1"
		"xpos"				"c-217"	[!$WIN32WIDE]
		"xpos"				"c-237"	[$WIN32WIDE]
		"ypos"				"42"
		"zpos"				"5"
		"wide"				"64"
		"tall"				"20"
		"labelText"			"#rd_settings_options_1"
		"style"				"ReactiveDropMainMenuTop"
		"command"			"Options1"
		"navLeft"			"BtnControls"
		"navRight"			"BtnOptions2"
		"navUp"				"TopBar"
	}

	"BtnOptions2"
	{
		"ControlName"		"BaseModHybridButton"
		"fieldName"			"BtnOptions2"
		"xpos"				"c-151"	[!$WIN32WIDE]
		"xpos"				"c-171"	[$WIN32WIDE]
		"ypos"				"42"
		"zpos"				"5"
		"wide"				"64"
		"tall"				"20"
		"labelText"			"#rd_settings_options_2"
		"style"				"ReactiveDropMainMenuTop"
		"command"			"Options2"
		"navLeft"			"BtnOptions1"
		"navRight"			"BtnAudio"
		"navUp"				"TopBar"
	}

	"BtnAudio"
	{
		"ControlName"		"BaseModHybridButton"
		"fieldName"			"BtnAudio"
		"xpos"				"c-85"	[!$WIN32WIDE]
		"xpos"				"c-105"	[$WIN32WIDE]
		"ypos"				"42"
		"zpos"				"5"
		"wide"				"64"
		"tall"				"20"
		"labelText"			"#rd_settings_audio"
		"style"				"ReactiveDropMainMenuTop"
		"command"			"Audio"
		"navLeft"			"BtnOptions2"
		"navRight"			"BtnVideo"
		"navUp"				"TopBar"
	}

	"BtnVideo"
	{
		"ControlName"		"BaseModHybridButton"
		"fieldName"			"BtnVideo"
		"xpos"				"c-19"	[!$WIN32WIDE]
		"xpos"				"c-39"	[$WIN32WIDE]
		"ypos"				"42"
		"zpos"				"5"
		"wide"				"64"
		"tall"				"20"
		"labelText"			"#rd_settings_video"
		"style"				"ReactiveDropMainMenuTop"
		"command"			"Video"
		"navLeft"			"BtnAudio"
		"navRight"			"BtnAbout"
		"navUp"				"TopBar"
	}

	"BtnAbout"
	{
		"ControlName"		"BaseModHybridButton"
		"fieldName"			"BtnAbout"
		"xpos"				"c219"	[!$WIN32WIDE]
		"xpos"				"c239"	[$WIN32WIDE]
		"ypos"				"42"
		"zpos"				"5"
		"wide"				"64"
		"tall"				"20"
		"labelText"			"#rd_settings_about"
		"style"				"ReactiveDropMainMenuTop"
		"command"			"About"
		"navLeft"			"BtnVideo"
		"navRight"			"BtnAbout"
		"navUp"				"TopBar"
	}

	"PnlControls"
	{
		"ControlName"		"CRD_VGUI_Settings_Controls"
		"fieldName"			"PnlControls"
		"xpos"				"c-290"	[!$WIN32WIDE]
		"xpos"				"c-305"	[$WIN32WIDE]
		"ypos"				"40"
		"wide"				"580"	[!$WIN32WIDE]
		"wide"				"610"	[$WIN32WIDE]
		"tall"				"400"
		"bgcolor_override"	"16 20 24 224"
		"paintbackgroundtype"	"2"
		"navUp"				"BtnControls"
	}

	"PnlOptions1"
	{
		"ControlName"		"CRD_VGUI_Settings_Options_1"
		"fieldName"			"PnlOptions1"
		"xpos"				"c-290"	[!$WIN32WIDE]
		"xpos"				"c-305"	[$WIN32WIDE]
		"ypos"				"40"
		"wide"				"580"	[!$WIN32WIDE]
		"wide"				"610"	[$WIN32WIDE]
		"tall"				"400"
		"bgcolor_override"	"16 20 24 224"
		"paintbackgroundtype"	"2"
		"navUp"				"BtnOptions1"
	}

	"PnlOptions2"
	{
		"ControlName"		"CRD_VGUI_Settings_Options_2"
		"fieldName"			"PnlOptions2"
		"xpos"				"c-290"	[!$WIN32WIDE]
		"xpos"				"c-305"	[$WIN32WIDE]
		"ypos"				"40"
		"wide"				"580"	[!$WIN32WIDE]
		"wide"				"610"	[$WIN32WIDE]
		"tall"				"400"
		"bgcolor_override"	"16 20 24 224"
		"paintbackgroundtype"	"2"
		"navUp"				"BtnOptions2"
	}

	"PnlAudio"
	{
		"ControlName"		"CRD_VGUI_Settings_Audio"
		"fieldName"			"PnlAudio"
		"xpos"				"c-290"	[!$WIN32WIDE]
		"xpos"				"c-305"	[$WIN32WIDE]
		"ypos"				"40"
		"wide"				"580"	[!$WIN32WIDE]
		"wide"				"610"	[$WIN32WIDE]
		"tall"				"400"
		"bgcolor_override"	"16 20 24 224"
		"paintbackgroundtype"	"2"
		"navUp"				"BtnAudio"
	}

	"PnlVideo"
	{
		"ControlName"		"CRD_VGUI_Settings_Video"
		"fieldName"			"PnlVideo"
		"xpos"				"c-290"	[!$WIN32WIDE]
		"xpos"				"c-305"	[$WIN32WIDE]
		"ypos"				"40"
		"wide"				"580"	[!$WIN32WIDE]
		"wide"				"610"	[$WIN32WIDE]
		"tall"				"400"
		"bgcolor_override"	"16 20 24 224"
		"paintbackgroundtype"	"2"
		"navUp"				"BtnVideo"
	}

	"PnlAbout"
	{
		"ControlName"		"CRD_VGUI_Settings_About"
		"fieldName"			"PnlAbout"
		"xpos"				"c-290"	[!$WIN32WIDE]
		"xpos"				"c-305"	[$WIN32WIDE]
		"ypos"				"40"
		"wide"				"580"	[!$WIN32WIDE]
		"wide"				"610"	[$WIN32WIDE]
		"tall"				"400"
		"bgcolor_override"	"16 20 24 224"
		"paintbackgroundtype"	"2"
		"navUp"				"BtnAbout"
	}
}
