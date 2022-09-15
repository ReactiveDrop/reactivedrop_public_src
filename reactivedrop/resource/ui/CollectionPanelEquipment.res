"Resource/UI/CollectionPanelEquipment.res"
{
	"EquipmentPanel"
	{
		"fieldName"			"EquipmentPanel"
		"xpos"				"c-310"
		"ypos"				"40"
		"wide"				"630" // 450, but it expands over the details pane too
		"tall"				"440" // 410, but it expands to the bottom of the screen
		"ControlName"		"EditablePanel"
	}

	"GplFacts"
	{
		"fieldName"			"GplFacts"
		"wide"				"300"
		"tall"				"410"
		"ControlName"		"GenericPanelList"
		"bgcolor_override"	"0 0 0 0"
	}

	"BtnEquip"
	{
		"fieldName"			"BtnEquip"
		"xpos"				"476"
		"ypos"				"385"
		"wide"				"117"
		"tall"				"27"
		"ControlName"		"CNB_Button"
		"font"				"DefaultMedium"
		"labelText"			"#asw_equip"
		"Command"			"AcceptEquip"
		"textAlignment"		"center"
	}
}
