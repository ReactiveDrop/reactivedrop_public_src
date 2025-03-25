"Resource/UI/CraftingPanel.res" {
	"CraftingPanel" {
		"ControlName"		"CRD_Crafting_Panel"
		"fieldName"			"CraftingPanel"
		"xpos"				"c-285"	[!$WIN32WIDE]
		"xpos"				"c-315"	[$WIN32WIDE]
		"ypos"				"60"
		"wide"				"640"
		"tall"				"390"
	}

	"Backdrop"
	{
		"ControlName"		"Panel"
		"fieldName"			"Backdrop"
		"xpos"				"0"
		"ypos"				"0"
		"wide"				"640"
		"tall"				"390"
		"zpos"				"-1"
		"paintbackgroundenabled"	"1"
		"paintbackgroundtype"	"2"
		"bgcolor_override"	"16 20 24 224"
	}

	"LblChooseRecipe" {
		"ControlName"		"Label"
		"fieldName"			"LblChooseRecipe"
		"xpos"				"5"
		"ypos"				"0"
		"wide"				"200"
		"tall"				"25"
		"labelText"			"#rd_crafting_select_recipe"
		"font"				"DefaultBold"
		"allcaps"			"1"
	}

	"GplRecipes" {
		"ControlName"		"GenericPanelList"
		"fieldName"			"GplRecipes"
		"xpos"				"5"
		"ypos"				"30"
		"wide"				"200"
		"tall"				"350"
		"bgcolor_override"	"0 0 0 0"
	}

	"LblRecipeTitle" {
		"ControlName"		"Label"
		"fieldName"			"LblRecipeTitle"
		"xpos"				"220"
		"ypos"				"0"
		"wide"				"410"
		"tall"				"25"
		"labelText"			""
		"font"				"DefaultBold"
	}

	"LblFlavor" {
		"ControlName"		"Label"
		"fieldName"			"LblFlavor"
		"xpos"				"220"
		"ypos"				"30"
		"wide"				"410"
		"tall"				"60"
		"wrap"				"1"
		"labelText"			""
		"fgcolor_override"	"224 224 224 255"
	}

	"LblWarning" {
		"ControlName"		"Label"
		"fieldName"			"LblWarning"
		"xpos"				"220"
		"ypos"				"280"
		"wide"				"410"
		"tall"				"60"
		"wrap"				"1"
		"labelText"			""
		"fgcolor_override"	"255 255 0 255"
	}

	"BtnCraft" {
		"ControlName"		"CNB_Button"
		"fieldName"			"BtnCraft"
		"xpos"				"250"
		"ypos"				"360"
		"wide"				"340"
		"tall"				"24"
		"textAlignment"		"center"
		"font"				"DefaultMedium"
		"fgcolor_override"	"113 142 181 255"
	}

	"LblIngredients" {
		"ControlName"		"Label"
		"fieldName"			"LblIngredients"
		"xpos"				"220"
		"ypos"				"90"
		"wide"				"410"
		"tall"				"15"
		"labelText"			"#rd_crafting_required_items"
	}

	"GridIngredients" {
		"ControlName"		"CRD_Crafting_Item_Grid"
		"fieldName"			"GridIngredients"
		"xpos"				"220"
		"ypos"				"105"
		"wide"				"70"
		"tall"				"15"
	}

	"GridIngredientsTall" {
		"ControlName"		"CRD_Crafting_Item_Grid"
		"fieldName"			"GridIngredientsTall"
		"xpos"				"220"
		"ypos"				"105"
		"wide"				"410"
		"tall"				"175"
	}

	"LblOutputs" {
		"ControlName"		"Label"
		"fieldName"			"LblOutputs"
		"xpos"				"220"
		"ypos"				"175"
		"wide"				"410"
		"tall"				"15"
		"labelText"			"#rd_crafting_output_items"
	}

	"GridOutputs" {
		"ControlName"		"CRD_Crafting_Item_Grid"
		"fieldName"			"GridOutputs"
		"xpos"				"220"
		"ypos"				"190"
		"wide"				"410"
		"tall"				"90"
	}
}
