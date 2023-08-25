"Resource/UI/CollectionPanelSwarmopedia.res"
{
	"SwarmopediaPanel"
	{
		"fieldName"			"SwarmopediaPanel"
		"xpos"				"c-280"	[!$WIN32WIDE]
		"xpos"				"c-310"	[$WIN32WIDE]
		"ypos"				"40"
		"wide"				"630" // 450, but it expands over the details pane too
		"tall"				"440" // 410, but it expands to the bottom of the screen
		"ControlName"		"CRD_Collection_Panel_Swarmopedia"
	}

	"ModelPanel"
	{
		"fieldName"			"ModelPanel"
		"xpos"				"400"	[!$WIN32WIDE]
		"xpos"				"460"	[$WIN32WIDE]
		"ypos"				"250"
		"wide"				"170"
		"tall"				"170"
		"ControlName"		"CRD_Swarmopedia_Model_Panel"
	}

	"ModelButton"
	{
		"fieldName"			"ModelButton"
		"xpos"				"416"	[!$WIN32WIDE]
		"xpos"				"476"	[$WIN32WIDE]
		"ypos"				"390"
		"wide"				"117"
		"tall"				"27"
		"ControlName"		"CNB_Button"
		"font"				"DefaultMedium"
		"Command"			"CycleDisplay"
		"textAlignment"		"center"
	}

	"LblNoModel"
	{
		"fieldName"			"LblNoModel"
		"xpos"				"460"
		"ypos"				"240"
		"wide"				"170"
		"tall"				"170"
		"ControlName"		"Label"
		"text"				"#rd_so_display_no_model"
		"textAlignment"		"center"
		"font"				"DefaultLarge"
		"fgcolor_override"	"255 155 0 255"
	}

	"Content"
	{
		"fieldName"			"Content"
		"xpos"				"0"
		"ypos"				"0"
		"wide"				"380"	[!$WIN32WIDE]
		"wide"				"450"	[$WIN32WIDE]
		"tall"				"410"
		"ControlName"		"RichText"
		"bgcolor_override"	"0 0 0 0"
	}
}
