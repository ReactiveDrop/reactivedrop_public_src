"Resource/UI/CraftingResearchPanel.res" {
	"CraftingResearchPanel" {
		"ControlName"		"CRD_Crafting_Research_Panel"
		"fieldName"			"CraftingResearchPanel"
		"xpos"				"c-285"	[!$WIN32WIDE]
		"xpos"				"c-315"	[$WIN32WIDE]
		"ypos"				"60"
		"wide"				"640"
		"tall"				"390"
	}

	"Backdrop" {
		"ControlName"		"Panel"
		"fieldName"			"Backdrop"
		"xpos"				"2"
		"ypos"				"0"
		"wide"				"638"
		"tall"				"12"
		"zpos"				"-1"
		"paintbackgroundenabled"	"1"
		"paintbackgroundtype"	"2"
		"bgcolor_override"	"16 20 24 224"
	}

	"BackdropError" {
		"ControlName"		"Panel"
		"fieldName"			"BackdropError"
		"xpos"				"2"
		"ypos"				"16"
		"wide"				"638"
		"tall"				"350"
		"zpos"				"-1"
		"paintbackgroundenabled"	"1"
		"paintbackgroundtype"	"2"
		"bgcolor_override"	"16 20 24 224"
	}

	"LblUpdateTimer"
	{
		"ControlName"		"Label"
		"fieldName"			"LblUpdateTimer"
		"xpos"				"489"
		"ypos"				"1"
		"wide"				"150"
		"tall"				"10"
		"textAlignment"		"north-east"
	}

	"LblErrorMessage"
	{
		"ControlName"		"Label"
		"fieldName"			"LblErrorMessage"
		"xpos"				"120"
		"ypos"				"20"
		"wide"				"400"
		"tall"				"160"
		"textAlignment"		"south"
		"font"				"DefaultExtraLarge"
		"wrap"				"1"
		"centerwrap"		"1"
	}

	"BtnRetryAfterError"
	{
		"ControlName"		"CNB_Button"
		"fieldName"			"BtnRetryAfterError"
		"xpos"				"245"
		"ypos"				"200"
		"wide"				"150"
		"tall"				"23"
		"font"				"DefaultMedium"
		"labelText"			"#rd_crafting_research_retry_connection"
		"textAlignment"		"center"
		"command"			"RetryConnection"
	}
}
