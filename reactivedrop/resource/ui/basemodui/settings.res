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
		"xpos"				"c-303"
		"ypos"				"42"
		"zpos"				"5"
		"wide"				"64"
		"tall"				"24"
		"labelText"			"#rd_settings_controls"
		"command"			"Controls"
		"navLeft"			"BtnControls"
		"navRight"			"BtnOptions"
		"navUp"				"TopBar"
	}

	"BtnOptions"
	{
		"ControlName"		"BaseModHybridButton"
		"fieldName"			"BtnOptions"
		"xpos"				"c-237"
		"ypos"				"42"
		"zpos"				"5"
		"wide"				"64"
		"tall"				"24"
		"labelText"			"#rd_settings_options"
		"command"			"Options"
		"navLeft"			"BtnControls"
		"navRight"			"BtnAudio"
		"navUp"				"TopBar"
	}

	"BtnAudio"
	{
		"ControlName"		"BaseModHybridButton"
		"fieldName"			"BtnAudio"
		"xpos"				"c-171"
		"ypos"				"42"
		"zpos"				"5"
		"wide"				"64"
		"tall"				"24"
		"labelText"			"#rd_settings_audio"
		"command"			"Audio"
		"navLeft"			"BtnOptions"
		"navRight"			"BtnVideo"
		"navUp"				"TopBar"
	}

	"BtnVideo"
	{
		"ControlName"		"BaseModHybridButton"
		"fieldName"			"BtnVideo"
		"xpos"				"c-105"
		"ypos"				"42"
		"zpos"				"5"
		"wide"				"64"
		"tall"				"24"
		"labelText"			"#rd_settings_video"
		"command"			"Video"
		"navLeft"			"BtnAudio"
		"navRight"			"BtnAbout"
		"navUp"				"TopBar"
	}

	"BtnAbout"
	{
		"ControlName"		"BaseModHybridButton"
		"fieldName"			"BtnAbout"
		"xpos"				"c239"
		"ypos"				"42"
		"zpos"				"5"
		"wide"				"64"
		"tall"				"24"
		"labelText"			"#rd_settings_about"
		"command"			"About"
		"navLeft"			"BtnVideo"
		"navRight"			"BtnAbout"
		"navUp"				"TopBar"
	}

	"PnlControls"
	{
		"ControlName"		"CRD_VGUI_Settings_Controls"
		"fieldName"			"PnlControls"
		"xpos"				"c-305"
		"ypos"				"40"
		"wide"				"610"
		"tall"				"400"
		"bgcolor_override"	"16 20 24 224"
		"paintbackgroundtype"	"2"
		"navUp"				"BtnControls"
	}

	"PnlOptions"
	{
		"ControlName"		"CRD_VGUI_Settings_Options"
		"fieldName"			"PnlOptions"
		"xpos"				"c-305"
		"ypos"				"40"
		"wide"				"610"
		"tall"				"400"
		"bgcolor_override"	"16 20 24 224"
		"paintbackgroundtype"	"2"
		"navUp"				"BtnOptions"
	}

	"PnlAudio"
	{
		"ControlName"		"CRD_VGUI_Settings_Audio"
		"fieldName"			"PnlAudio"
		"xpos"				"c-305"
		"ypos"				"40"
		"wide"				"610"
		"tall"				"400"
		"bgcolor_override"	"16 20 24 224"
		"paintbackgroundtype"	"2"
		"navUp"				"BtnAudio"
	}

	"PnlVideo"
	{
		"ControlName"		"CRD_VGUI_Settings_Video"
		"fieldName"			"PnlVideo"
		"xpos"				"c-305"
		"ypos"				"40"
		"wide"				"610"
		"tall"				"400"
		"bgcolor_override"	"16 20 24 224"
		"paintbackgroundtype"	"2"
		"navUp"				"BtnVideo"
	}

	"PnlAbout"
	{
		"ControlName"		"CRD_VGUI_Settings_About"
		"fieldName"			"PnlAbout"
		"xpos"				"c-305"
		"ypos"				"40"
		"wide"				"610"
		"tall"				"400"
		"bgcolor_override"	"16 20 24 224"
		"paintbackgroundtype"	"2"
		"navUp"				"BtnAbout"
	}
}
