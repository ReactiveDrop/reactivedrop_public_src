"Resource/UI/RDWorkshop.res"
{
	"RDWorkshop"
	{
		"ControlName"			"Frame"
		"fieldName"				"RDWorkshop"
		"xpos"					"0"
		"ypos"					"0"
		"wide"					"f0"
		"tall"					"f0"
		"visible"				"1"
		"enabled"				"1"
	}

	"HeaderFooter"
	{
		"ControlName"			"CNB_HeaderFooter"
		"fieldName"				"HeaderFooter"
		"xpos"					"0"
		"ypos"					"0"
		"zpos"					"-3"
		"wide"					"f0"
		"tall"					"f0"
		"visible"				"1"
		"enabled"				"1"
	}

	"BtnCancel"
	{
		"ControlName"			"CNB_Button"
		"fieldName"				"BtnCancel"
		"xpos"					"c147"
		"ypos"					"r23"
		"wide"					"117"
		"tall"					"27"
		"zpos"					"1"
		"visible"				"0"
		"enabled"				"1"
		"tabPosition"			"0"
		"command"				"CancelEdit"
		"textAlignment"			"center"
		"font"					"DefaultMedium"
		"fgcolor_override"		"113 142 181 255"
	}

	"BtnSubmit"
	{
		"ControlName"			"CNB_Button"
		"fieldName"				"BtnSubmit"
		"xpos"					"c25"
		"ypos"					"r23"
		"wide"					"117"
		"tall"					"27"
		"zpos"					"1"
		"visible"				"0"
		"enabled"				"1"
		"tabPosition"			"0"
		"command"				"SubmitEdit"
		"textAlignment"			"center"
		"font"					"DefaultMedium"
		"fgcolor_override"		"113 142 181 255"
	}

	"BtnOpen"
	{
		"ControlName"			"CNB_Button"
		"fieldName"				"BtnOpen"
		"xpos"					"c25"
		"ypos"					"r23"
		"wide"					"117"
		"tall"					"27"
		"zpos"					"1"
		"visible"				"0"
		"enabled"				"1"
		"tabPosition"			"0"
		"command"				"OpenInWorkshop"
		"textAlignment"			"center"
		"font"					"DefaultMedium"
		"fgcolor_override"		"113 142 181 255"
	}

	"BtnEdit"
	{
		"ControlName"			"CNB_Button"
		"fieldName"				"BtnEdit"
		"xpos"					"c147"
		"ypos"					"r23"
		"wide"					"117"
		"tall"					"27"
		"zpos"					"1"
		"visible"				"0"
		"enabled"				"1"
		"tabPosition"			"0"
		"command"				"EditWorkshopItem"
		"textAlignment"			"center"
		"font"					"DefaultMedium"
		"fgcolor_override"		"113 142 181 255"
	}

	"BtnBack"
	{
		"ControlName"			"CNB_Button"
		"fieldName"				"BtnBack"
		"xpos"					"c-264"
		"ypos"					"r23"
		"wide"					"117"
		"tall"					"27"
		"zpos"					"1"
		"visible"				"1"
		"enabled"				"1"
		"tabPosition"			"0"
		"labelText"				"#L4D360UI_Back_Caps"
		"command"				"Back"
		"textAlignment"			"center"
		"font"					"DefaultMedium"
		"fgcolor_override"		"113 142 181 255"
	}

	"LblWaiting"
	{
		"ControlName"			"Label"
		"fieldName"				"LblWaiting"
		"xpos"					"c-300"
		"ypos"					"100"
		"wide"					"600"
		"tall"					"100"
		"zpos"					"5"
		"wrap"					"1"
		"centerWrap"			"1"
		"visible"				"1"
		"enabled"				"1"
		"textAlignment"			"south"
		"font"					"DefaultExtraLarge"
		"fgcolor_override"		"113 142 181 255"
	}

	"PrgWaiting"
	{
		"ControlName"			"ProgressBar"
		"fieldName"				"PrgWaiting"
		"xpos"					"c-200"
		"ypos"					"210"
		"wide"					"400"
		"tall"					"20"
		"zpos"					"5"
		"visible"				"1"
		"enabled"				"1"
	}

	"GplWorkshopItems"
	{
		"ControlName"			"GenericPanelList"
		"fieldName"				"GplWorkshopItems"
		"xpos"					"c-320"
		"ypos"					"50"
		"wide"					"200"
		"tall"					"390"
		"zpos"					"3"
		"visible"				"1"
		"enabled"				"1"
		"bgcolor_override"		"0 0 0 0"
	}

	"LblEditingTitle"
	{
		"ControlName"			"Label"
		"fieldName"				"LblEditingTitle"
		"xpos"					"c-110"
		"ypos"					"50"
		"wide"					"200"
		"tall"					"15"
		"visible"				"1"
		"enabled"				"1"
	}

	"TxtEditingTitle"
	{
		"ControlName"			"TextEntry"
		"fieldName"				"TxtEditingTitle"
		"xpos"					"c-110"
		"ypos"					"65"
		"wide"					"200"
		"tall"					"20"
		"visible"				"1"
		"enabled"				"1"
	}

	"LblEditingTags"
	{
		"ControlName"			"Label"
		"fieldName"				"LblEditingTags"
		"xpos"					"c-110"
		"ypos"					"90"
		"wide"					"200"
		"tall"					"15"
		"visible"				"1"
		"enabled"				"1"
	}

	"TxtEditingTags"
	{
		"ControlName"			"TextEntry"
		"fieldName"				"TxtEditingTags"
		"xpos"					"c-110"
		"ypos"					"105"
		"wide"					"200"
		"tall"					"20"
		"visible"				"1"
		"enabled"				"1"
	}

	"LblEditingPreview"
	{
		"ControlName"			"Label"
		"fieldName"				"LblEditingPreview"
		"xpos"					"c100"
		"ypos"					"50"
		"wide"					"160"
		"tall"					"15"
		"visible"				"1"
		"enabled"				"1"
	}

	"ImgEditingPreview"
	{
		"ControlName"			"ImagePanel"
		"fieldName"				"ImgEditingPreview"
		"xpos"					"c100"
		"ypos"					"65"
		"wide"					"160"
		"tall"					"90"
		"scaleImage"			"1"
		"visible"				"1"
		"enabled"				"1"
	}

	"BtnEditPreview"
	{
		"ControlName"			"CNB_Button"
		"fieldName"				"BtnEditPreview"
		"xpos"					"c160"
		"ypos"					"130"
		"wide"					"100"
		"tall"					"25"
		"zpos"					"3"
		"textAlignment"			"center"
		"visible"				"1"
		"enabled"				"1"
	}

	"BtnEditContent"
	{
		"ControlName"			"CNB_Button"
		"fieldName"				"BtnEditContent"
		"xpos"					"c-75"
		"ypos"					"130"
		"wide"					"140"
		"tall"					"25"
		"textAlignment"			"center"
		"visible"				"1"
		"enabled"				"1"
	}

	"BtnCreateWorkshopItem"
	{
		"ControlName"			"CNB_Button"
		"fieldName"				"BtnCreateWorkshopItem"
		"xpos"					"c-142"
		"ypos"					"r23"
		"wide"					"117"
		"tall"					"27"
		"textAlignment"			"center"
		"font"					"DefaultMedium"
		"fgcolor_override"		"113 142 181 255"
		"visible"				"1"
		"enabled"				"1"
	}

	"LblEditingDescription"
	{
		"ControlName"			"Label"
		"fieldName"				"LblEditingDescription"
		"xpos"					"c-110"
		"ypos"					"160"
		"wide"					"370"
		"tall"					"15"
		"visible"				"1"
		"enabled"				"1"
	}

	"TxtEditingDescription"
	{
		"ControlName"			"TextEntry"
		"fieldName"				"TxtEditingDescription"
		"xpos"					"c-110"
		"ypos"					"175"
		"wide"					"370"
		"tall"					"120"
		"visible"				"1"
		"enabled"				"1"
	}

	"LblEditingChangeDescription"
	{
		"ControlName"			"Label"
		"fieldName"				"LblEditingChangeDescription"
		"xpos"					"c-110"
		"ypos"					"300"
		"wide"					"370"
		"tall"					"15"
		"visible"				"1"
		"enabled"				"1"
	}

	"TxtEditingChangeDescription"
	{
		"ControlName"			"TextEntry"
		"fieldName"				"TxtEditingChangeDescription"
		"xpos"					"c-110"
		"ypos"					"315"
		"wide"					"370"
		"tall"					"120"
		"visible"				"1"
		"enabled"				"1"
	}

	"BtnInstalled"
	{
		"ControlName"			"CNB_Button"
		"fieldName"				"BtnInstalled"
		"xpos"					"c147"
		"ypos"					"r23"
		"wide"					"117"
		"tall"					"27"
		"zpos"					"1"
		"enabled"				"1"
		"tabPosition"			"0"
		"labelText"				"#rd_workshop_installed_short"
		"command"				"Installed"
		"textAlignment"			"center"
		"allcaps"				"1"
		"font"					"DefaultMedium"
		"fgcolor_override"		"113 142 181 255"
	}
}
