"Resource/UI/MainMenuStub.res"
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
		"PaintBackgroundType"	"0"
		"navUp"					"BtnStub"
		"navDown"				"BtnStub"
		"navLeft"				"BtnStub"
		"navRight"				"BtnStub"
	}

	"BtnStub"
	{
		"ControlName"			"BaseModHybridButton"
		"fieldName"				"BtnStub"
		"xpos"					"100"
		"ypos"					"175"
		"wide"					"180"
		"tall"					"20"
		"autoResize"			"1"
		"pinCorner"				"0"
		"visible"				"1"
		"enabled"				"1"
		"tabPosition"			"1"
		"navDown"				"BtnQuit"
		"labelText"				"NO STEAM"
		"tooltiptext"			"Failed to connect to Steam"
		"style"					"MainMenuButton"
		"command"				"BtnStub"
		"ActivationType"		"1"
	}

	"BtnQuit"
	{
		"ControlName"			"BaseModHybridButton"
		"fieldName"				"BtnQuit"
		"xpos"					"100"
		"ypos"					"200"
		"wide"					"180"
		"tall"					"20"
		"autoResize"			"1"
		"pinCorner"				"0"
		"visible"				"1"
		"enabled"				"1"
		"tabPosition"			"1"
		"navUp"					"BtnStub"
		"labelText"				"#L4D360UI_MainMenu_Quit"
		"tooltiptext"			"#L4D360UI_MainMenu_Quit_Tip"
		"style"					"MainMenuButton"
		"command"				"QuitGame"
		"ActivationType"		"1"
	}
}
