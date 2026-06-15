"Resource/UI/CraftingResearchSlot.res" {
	"ResearchSlot" {
		"ControlName"		"CRD_Crafting_Research_Slot"
		"fieldName"			"ResearchSlot"
		"wide"				"210"
		"tall"				"350"
	}

	"Backdrop" {
		"ControlName"		"Panel"
		"fieldName"			"Backdrop"
		"xpos"				"0"
		"ypos"				"0"
		"wide"				"210"
		"tall"				"350"
		"zpos"				"-1"
		"paintbackgroundenabled"	"1"
		"paintbackgroundtype"	"2"
		"bgcolor_override"	"16 20 24 224"
	}

	"LblTitle" {
		"ControlName"		"Label"
		"fieldName"			"LblTitle"
		"xpos"				"5"
		"ypos"				"0"
		"wide"				"200"
		"tall"				"30"
		"font"				"DefaultLarge"
		"textAlignment"		"center"
	}

	"LblWaitDescription" {
		"ControlName"		"Label"
		"fieldName"			"LblWaitDescription"
		"xpos"				"0"
		"ypos"				"70"
		"wide"				"210"
		"tall"				"50"
		"textAlignment"		"center"
		"wrap"				"1"
		"centerwrap"		"1"
	}

	"LblWaitTimer" {
		"ControlName"		"Label"
		"fieldName"			"LblWaitTimer"
		"xpos"				"5"
		"ypos"				"130"
		"wide"				"200"
		"tall"				"23"
		"font"				"Countdown"
		"textAlignment"		"center"
	}

	"ImgItemIcon" {
		"ControlName"		"ImagePanel"
		"fieldName"			"ImgItemIcon"
		"xpos"				"0"
		"ypos"				"30"
		"wide"				"40"
		"tall"				"40"
		"scaleImage"		"1"
	}

	"LblProjectTitle" {
		"ControlName"		"Label"
		"fieldName"			"LblProjectTitle"
		"xpos"				"42"
		"ypos"				"30"
		"wide"				"165"
		"tall"				"40"
		"font"				"DefaultMedium"
		"textAlignment"		"west"
		"wrap"				"1"
	}

	"LblProjectFlavorText" {
		"ControlName"		"Label"
		"fieldName"			"LblProjectFlavorText"
		"xpos"				"0"
		"ypos"				"70"
		"wide"				"210"
		"tall"				"50"
		"textAlignment"		"center"
		"wrap"				"1"
		"centerwrap"		"1"
	}

	"BtnFetchProjectComponents" {
		"ControlName"		"CNB_Button"
		"fieldName"			"BtnFetchProjectComponents"
		"xpos"				"30"
		"ypos"				"130"
		"wide"				"150"
		"tall"				"23"
		"font"				"DefaultMedium"
		"labelText"			"#rd_crafting_research_force_refresh"
		"textAlignment"		"center"
		"command"			"ForceRefresh"
	}

	"GplComponents" {
		"ControlName"		"GenericPanelList"
		"fieldName"			"GplComponents"
		"xpos"				"0"
		"ypos"				"120"
		"wide"				"210"
		"tall"				"230"
		"bgcolor_override"	"0 0 0 0"
	}
}
