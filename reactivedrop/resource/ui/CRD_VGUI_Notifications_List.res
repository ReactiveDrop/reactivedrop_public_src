"Resource/UI/CRD_VGUI_Notifications_List.res"
{
	"List"
	{
		"ControlName"		"CRD_VGUI_Notifications_List"
		"fieldName"			"List"
		"xpos"				"r220"
		"ypos"				"28"
		"zpos"				"100"
		"wide"				"210"
		"tall"				"340"
	}

	"Background1"
	{
		"ControlName"		"Panel"
		"fieldName"			"Background1"
		"xpos"				"1"
		"ypos"				"1"
		"zpos"				"-1"
		"wide"				"208"
		"tall"				"338"
		"paintbackgroundtype"	"2"
		"bgcolor_override"	"28 34 40 255"
	}

	"Background2"
	{
		"ControlName"		"Panel"
		"fieldName"			"Background2"
		"xpos"				"0"
		"ypos"				"0"
		"zpos"				"-2"
		"wide"				"210"
		"tall"				"340"
		"paintbackgroundtype"	"2"
		"bgcolor_override"	"70 80 90 255"
	}

	"BtnFilters"
	{
		"ControlName"		"CNB_Button"
		"fieldName"			"BtnFilters"
		"xpos"				"0"
		"ypos"				"313"
		"wide"				"210"
		"tall"				"27"
		"font"				"DefaultMedium"
		"labelText"			"#rd_notification_filter_title"
		"textAlignment"		"center"
		"command"			"NotificationsFiltersClicked"
		"autoFocus"			"1"
	}

	"LblNone"
	{
		"ControlName"		"Label"
		"fieldName"			"LblNone"
		"xpos"				"0"
		"ypos"				"0"
		"wide"				"210"
		"tall"				"312"
		"font"				"DefaultMedium"
		"labelText"			"#rd_notification_none"
		"textAlignment"		"center"
		"alpha"				"128"
		"wrap"				"1"
		"centerwrap"		"1"
	}

	"GplList"
	{
		"ControlName"		"GenericPanelList"
		"fieldName"			"GplList"
		"xpos"				"0"
		"ypos"				"0"
		"wide"				"210"
		"tall"				"312"
		"NoDrawPanel"		"1"
	}
}
