"Resource/UI/CRD_VGUI_Notifications_Filters.res"
{
	"Filters"
	{
		"ControlName"		"CRD_VGUI_Notifications_Filters"
		"fieldName"			"Filters"
		"xpos"				"r375"
		"ypos"				"268"
		"zpos"				"120"
		"wide"				"154"
		"tall"				"100"
	}

	"Background1"
	{
		"ControlName"		"Panel"
		"fieldName"			"Background1"
		"xpos"				"1"
		"ypos"				"1"
		"zpos"				"-1"
		"wide"				"152"
		"tall"				"98"
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
		"wide"				"154"
		"tall"				"100"
		"paintbackgroundtype"	"2"
		"bgcolor_override"	"70 80 90 255"
	}

	"SettingCrafting"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingCrafting"
		"xpos"				"2"
		"ypos"				"2"
		"wide"				"150"
		"tall"				"24"
		"ResourceFile"		"resource/ui/option_notifications_checkbox.res"
		"navLeft"			"SettingCrafting"
		"navUp"				"SettingCrafting"
		"navDown"			"SettingHoIAF"
	}

	"SettingHoIAF"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingHoIAF"
		"xpos"				"2"
		"ypos"				"34"
		"wide"				"150"
		"tall"				"32"
		"ResourceFile"		"resource/ui/option_notifications_checkbox.res"
		"navLeft"			"SettingHoIAF"
		"navUp"				"SettingCrafting"
		"navDown"			"SettingReports"
	}

	"SettingReports"
	{
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"SettingReports"
		"xpos"				"2"
		"ypos"				"66"
		"wide"				"150"
		"tall"				"32"
		"ResourceFile"		"resource/ui/option_notifications_checkbox.res"
		"navLeft"			"SettingReports"
		"navUp"				"SettingHoIAF"
		"navDown"			"SettingReports"
	}
}
