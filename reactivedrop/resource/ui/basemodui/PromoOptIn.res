"Resource/UI/PromoOptIn.res"
{
	"PromoOptIn"
	{
		"ControlName"	"PromoOptIn"
		"fieldName"		"PromoOptIn"
		"xpos"			"0"
		"ypos"			"0"
		"wide"			"f0"
		"tall"			"f0"
		"bgcolor_override"	"0 0 0 255"
	}

	"LblFlavor"
	{
		"ControlName"	"Label"
		"fieldName"		"LblFlavor"
		"xpos"			"c-200"
		"ypos"			"20"
		"wide"			"400"
		"tall"			"210"
		"wrap"			"1"
		"textAlignment"	"north-west"
		"font"			"DefaultMedium"
		"fgcolor_override"	"0 255 0 255"
	}

	"LblExplanationTitle"
	{
		"ControlName"	"Label"
		"fieldName"		"LblExplanationTitle"
		"xpos"			"c-250"
		"ypos"			"230"
		"wide"			"500"
		"tall"			"20"
		"labelText"		"#rd_crafting_beta1_signup_title"
		"textAlignment"	"north"
		"font"			"DefaultLarge"
	}

	"LblExplanation"
	{
		"ControlName"	"Label"
		"fieldName"		"LblExplanation"
		"xpos"			"c-250"
		"ypos"			"250"
		"wide"			"500"
		"tall"			"180"
		"labelText"		"#rd_crafting_beta1_signup_explanation"
		"wrap"			"1"
	}

	"BtnDecline"
	{
		"ControlName"	"CNB_Button"
		"fieldName"		"BtnDecline"
		"xpos"			"c-230"
		"ypos"			"440"
		"wide"			"200"
		"tall"			"27"
		"labelText"		"#rd_crafting_beta1_signup_decline"
		"textAlignment"	"center"
		"font"			"DefaultMedium"
		"command"		"Decline"
	}

	"BtnAccept"
	{
		"ControlName"	"CNB_Button"
		"fieldName"		"BtnAccept"
		"xpos"			"c30"
		"ypos"			"440"
		"wide"			"200"
		"tall"			"27"
		"labelText"		"#rd_crafting_beta1_signup_accept"
		"textAlignment"	"center"
		"font"			"DefaultMedium"
		"command"		"Accept"
	}

	"BtnAlready"
	{
		"ControlName"	"CNB_Button"
		"fieldName"		"BtnAccept"
		"xpos"			"c-230"
		"ypos"			"440"
		"wide"			"460"
		"tall"			"27"
		"labelText"		"#rd_crafting_beta1_signup_already"
		"textAlignment"	"center"
		"font"			"DefaultMedium"
		"command"		"Back"
	}
}
