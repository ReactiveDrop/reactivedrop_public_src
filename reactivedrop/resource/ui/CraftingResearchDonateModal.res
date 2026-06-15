"Resource/UI/CraftingResearchDonateModal.res" {
	"DonateModal" {
		"ControlName"		"CRD_Crafting_Research_Donate_Modal"
		"fieldName"			"DonateModal"
		"xpos"				"150"
		"ypos"				"60"
		"zpos"				"10"
		"wide"				"340"
		"tall"				"240"
	}

	"Backdrop" {
		"ControlName"		"Panel"
		"fieldName"			"Backdrop"
		"xpos"				"1"
		"ypos"				"1"
		"wide"				"338"
		"tall"				"238"
		"zpos"				"-1"
		"paintbackgroundenabled"	"1"
		"paintbackgroundtype"	"2"
		"bgcolor_override"	"10 14 18 224"
	}

	"Backdrop2" {
		"ControlName"		"Panel"
		"fieldName"			"Backdrop2"
		"xpos"				"0"
		"ypos"				"0"
		"wide"				"340"
		"tall"				"240"
		"zpos"				"-2"
		"paintbackgroundenabled"	"1"
		"paintbackgroundtype"	"2"
		"bgcolor_override"	"78 94 110 224"
	}

	"LblTitle" {
		"ControlName"		"Label"
		"fieldName"			"LblTitle"
		"xpos"				"5"
		"ypos"				"5"
		"wide"				"330"
		"tall"				"30"
		"labelText"			"#rd_crafting_research_donation_title"
		"textAlignment"		"center"
		"font"				"DefaultExtraLarge"
	}

	"ImgItemIcon" {
		"ControlName"		"ImagePanel"
		"fieldName"			"ImgItemIcon"
		"xpos"				"30"
		"ypos"				"30"
		"wide"				"64"
		"tall"				"64"
		"scaleImage"		"1"
	}

	"LblItemName" {
		"ControlName"		"Label"
		"fieldName"			"LblItemName"
		"xpos"				"105"
		"ypos"				"30"
		"wide"				"190"
		"tall"				"40"
		"font"				"DefaultLarge"
		"wrap"				"1"
	}

	"LblDisplayType" {
		"ControlName"		"Label"
		"fieldName"			"LblDisplayType"
		"xpos"				"105"
		"ypos"				"70"
		"wide"				"190"
		"tall"				"10"
		"fgcolor_override"	"128 128 128 255"
	}

	"QuantitySlider" {
		"ControlName"		"CRD_VGUI_Option"
		"fieldName"			"QuantitySlider"
		"xpos"				"30"
		"ypos"				"100"
		"wide"				"270"
		"tall"				"12"
		"ResourceFile"		"resource/ui/option_setting_slider.res"
		"decimalDigits"		"0"
		"navDown"			"BtnConfirm"
	}

	"BtnConfirm" {
		"ControlName"		"CNB_Button"
		"fieldName"			"BtnConfirm"
		"xpos"				"85"
		"ypos"				"125"
		"wide"				"170"
		"tall"				"23"
		"font"				"DefaultMedium"
		"labelText"			"#rd_crafting_research_donate_button"
		"navUp"				"QuantitySlider"
		"navDown"			"BtnCancel"
		"textAlignment"		"center"
	}

	"BtnCancel" {
		"ControlName"		"CNB_Button"
		"fieldName"			"BtnCancel"
		"xpos"				"85"
		"ypos"				"155"
		"wide"				"170"
		"tall"				"23"
		"font"				"DefaultMedium"
		"labelText"			"#asw_button_cancel"
		"navUp"				"BtnConfirm"
		"textAlignment"		"center"
	}

	"LblExplanation" {
		"ControlName"		"Label"
		"fieldName"			"LblExplanation"
		"xpos"				"30"
		"ypos"				"185"
		"wide"				"280"
		"tall"				"50"
		"labelText"			"#rd_crafting_research_donation_explanation"
		"textAlignment"		"center"
		"wrap"				"1"
		"centerwrap"		"1"
	}
}
