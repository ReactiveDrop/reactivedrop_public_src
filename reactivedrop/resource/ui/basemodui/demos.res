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
}
