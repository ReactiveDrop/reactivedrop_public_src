"Resource/UI/BaseModUI/Demos.res" {
	"Demos" {
		"ControlName"		"Frame"
		"fieldName"		"Demos"
		
		"xpos"		"0"
		"ypos"		"0"
		"wide"		"f0"
		"tall"		"f0"
		"autoResize"		"0"
		"pinCorner"		"0"
		"visible"		"1"
		"enabled"		"1"
		"tabPosition"		"0"
	}

	"Title" {
		"fieldName"		"Title"
		"xpos"		"c-266"
		"ypos"		"56"
		"wide"		"250"
		"tall"		"19"
		"zpos"		"5"
		"font"		"DefaultExtraLarge"
		"textAlignment"		"west"
		"ControlName"		"Label"
		"labelText"		"#rd_demo_list_title"
		"fgcolor_override"		"224 224 224 255"
	}

	"GplRecordingList" {
		"ControlName"		"GenericPanelList"
		"fieldName"		"GplRecordingList"
		"xpos"		"c-226"
		"ypos"		"90"
		"wide"		"450"
		"tall"		"270"
		"zpos"		"1"
		"tabPosition"		"1"
		"NoDrawPanel"		"1"
	}

	"LblNoRecordings" {
		"ControlName"		"Label"
		"fieldName"		"LblNoRecordings"
		"xpos"		"c-150"
		"ypos"		"210"
		"wide"		"300"
		"tall"		"24"
		"zpos"		"1"
		"labelText"		"#rd_demo_list_empty"
		"font"		"DefaultExtraLarge"
		"textAlignment"		"center"
	}

	"LblAutoRecordWarning" {
		"ControlName"		"Label"
		"fieldName"		"LblAutoRecordWarning"
		"xpos"		"c-226"
		"ypos"		"390"
		"wide"		"380"
		"tall"		"20"
		"zpos"		"1"
		"labelText"		""
	}
	"DrpAutoRecord" {
		"ControlName"		"DropDownMenu"
		"fieldName"		"DrpAutoRecord"
		"xpos"		"c-226"
		"ypos"		"380"
		"zpos"		"1"
		"wide"		"380"
		"tall"		"15"
		"navUp"		"GplRecordingList"
		"navDown"		"BtnCancel"

		"BtnDropButton" {
			"ControlName"		"BaseModHybridButton"
			"fieldName"		"BtnDropButton"
			"xpos"		"0"
			"ypos"		"0"
			"zpos"		"2"
			"wide"		"380"
			"wideatopen"		"270"
			"tall"		"15"
			"autoResize"		"1"
			"pinCorner"		"0"
			"tabPosition"		"0"
			"labelText"		"#rd_demo_auto_setting_name"
			"style"		"DropDownButton"
			"command"		"FlmAutoRecord"
			"allcaps"		"1"
		}
	}

	"FlmAutoRecord" {
		"ControlName"		"FlyoutMenu"
		"fieldName"		"FlmAutoRecord"
		"zpos"		"4"
		"ExpandUp"		"1"
		"visible"		"0"
		"ResourceFile"		"resource/UI/BaseModUI/DropDownAutoRecord.res"
	}

	"BtnCancel" {
		"ControlName"		"CNB_Button"
		"fieldName"		"BtnCancel"
		"xpos"		"c-264"
		"ypos"		"r23"
		"wide"		"117"
		"tall"		"27"
		"zpos"		"1"
		"visible"		"1"
		"enabled"		"1"
		"tabPosition"		"0"
		"labelText"		"#nb_back"
		"command"		"Back"
		"textAlignment"		"center"
		"font"		"DefaultMedium"
		"fgcolor_override"		"113 142 181 255"
	}
	"BtnDelete"
	{
		"fieldName"		"BtnDelete"
		"xpos"		"c-80"
		"ypos"		"r23"
		"wide"		"107"
		"tall"		"27"
		"ControlName"		"CNB_Button"
		"allcaps"		"1"
		"labelText"		"#rd_demo_action_delete"
		"command"		"Delete"
		"textAlignment"		"center"
		"font"		"DefaultMedium"
		"fgcolor_override"		"113 142 181 255"
		"zpos"		"3"
	}
	"BtnRename"
	{
		"fieldName"		"BtnRename"
		"xpos"		"c34"
		"ypos"		"r23"
		"wide"		"107"
		"tall"		"27"
		"ControlName"		"CNB_Button"
		"allcaps"		"1"
		"labelText"		"#rd_demo_action_rename"
		"command"		"Rename"
		"textAlignment"		"center"
		"font"		"DefaultMedium"
		"fgcolor_override"		"113 142 181 255"
		"zpos"		"3"
	}
	"BtnWatch"
	{
		"fieldName"		"BtnWatch"
		"xpos"		"c147"
		"ypos"		"r23"
		"wide"		"117"
		"tall"		"27"
		"ControlName"		"CNB_Button"
		"allcaps"		"1"
		"labelText"		"#rd_demo_action_watch"
		"command"		"Watch"
		"textAlignment"		"center"
		"font"		"DefaultMedium"
		"fgcolor_override"		"113 142 181 255"
		"zpos"		"3"
	}
}
