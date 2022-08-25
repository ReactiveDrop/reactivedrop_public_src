"Resource/UI/CollectionPanelSwarmopedia.res"
{
	"SwarmopediaPanel"
	{
		"fieldName"			"SwarmopediaPanel"
		"xpos"				"c-310"
		"ypos"				"40"
		"wide"				"630" // 450, but it expands over the details pane too
		"tall"				"440" // 410, but it expands to the bottom of the screen
		"ControlName"		"CRD_Collection_Panel_Swarmopedia"
	}

	"ModelPanel"
	{
		"fieldName"			"ModelPanel"
		"xpos"				"460"
		"ypos"				"250"
		"wide"				"170"
		"tall"				"170"
		"ControlName"		"CRD_Swarmopedia_Model_Panel"
	}

	"ModelButton"
	{
		"fieldName"			"ModelButton"
		"xpos"				"476"
		"ypos"				"417"
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
		"wide"				"450"
		"tall"				"410"
		"ControlName"		"RichText"
		"bgcolor_override"	"0 0 0 0"
	}
}
