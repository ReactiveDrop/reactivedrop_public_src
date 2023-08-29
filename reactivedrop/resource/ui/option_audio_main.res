"resource/ui/option_audio_main.res"
{
	"InteractiveArea"
	{
		"ControlName"		"Panel"
		"fieldName"			"InteractiveArea"
		"xpos"				"120"	[!$WIN32WIDE]
		"xpos"				"140"	[$WIN32WIDE]
		"ypos"				"0"
		"wide"				"120"	[!$WIN32WIDE]
		"wide"				"130"	[$WIN32WIDE]
		"tall"				"15"
		"navLeft"			"TextEntry"
	}

	"LblFieldName"
	{
		"ControlName"		"Label"
		"fieldName"			"LblFieldName"
		"xpos"				"0"
		"ypos"				"0"
		"wide"				"90"	[!$WIN32WIDE]
		"wide"				"110"	[$WIN32WIDE]
		"tall"				"15"
		"font"				"DefaultBold"
		"fgcolor_override"	"192 192 192 255"
	}

	"LblHint"
	{
		"ControlName"		"Label"
		"fieldName"			"LblHint"
		"visible"			"0"
	}

	"TextEntry"
	{
		"ControlName"		"TextEntry"
		"fieldName"			"TextEntry"
		"xpos"				"90"	[!$WIN32WIDE]
		"xpos"				"110"	[$WIN32WIDE]
		"ypos"				"0"
		"wide"				"30"
		"tall"				"15"
		"navRight"			"InteractiveArea"
		"bgcolor_override"	"32 32 32 255"
	}
}
