"Resource/UI/CollectionEntryEquipmentExtra.res"
{
	"CollectionEntryEquipmentExtra"
	{
		"fieldName"			"CollectionEntryEquipmentExtra"
		"xpos"				"0"
		"ypos"				"0"
		"wide"				"90"
		"tall"				"90"
		"ControlName"		"CRD_Collection_Entry_Equipment"
	}

	"FocusHolder"
	{
		"fieldName"			"FocusHolder"
		"xpos"				"0"
		"ypos"				"0"
		"wide"				"90"
		"tall"				"90"
		"zpos"				"999"
		"ControlName"		"TGD_Entry_FocusHolder"
	}

	"Background"
	{
		"fieldName"			"Background"
		"xpos"				"1"
		"ypos"				"2"
		"wide"				"88"
		"tall"				"88"
		"ControlName"		"Panel"
		"zpos"				"-2"
		"bgcolor_override"	"0 0 0 192"
	}

	"Highlight"
	{
		"fieldName"			"Highlight"
		"xpos"				"2"
		"ypos"				"3"
		"wide"				"86"
		"tall"				"86"
		"ControlName"		"Panel"
		"zpos"				"-1"
		"bgcolor_override"	"65 74 96 255"
		"visible"			"0"
	}

	"Icon"
	{
		"fieldName"			"Icon"
		"xpos"				"2"
		"ypos"				"3"
		"wide"				"86"
		"tall"				"86"
		"ControlName"		"ImagePanel"
		"scaleImage"		"1"
	}

	"LockedIcon"
	{
		"fieldName"			"LockedIcon"
		"xpos"				"2"
		"ypos"				"3"
		"wide"				"86"
		"tall"				"86"
		"ControlName"		"ImagePanel"
		"alpha"				"64"
		"scaleImage"		"1"
		"image"				"swarm/EquipIcons/Locked"
	}

	"LockedOverlay"
	{
		"fieldName"			"LockedOverlay"
		"xpos"				"0"
		"ypos"				"30"
		"wide"				"90"
		"tall"				"40"
		"ControlName"		"Label"
		"textAlignment"		"north"
		"fgcolor_override"	"255 255 255 255"
		"font"				"DefaultMedium"
		"labelText"			"#asw_weapon_details_required_level"
	}

	"LockedLabel"
	{
		"fieldName"			"LockedLabel"
		"xpos"				"2"
		"ypos"				"40"
		"wide"				"86"
		"tall"				"20"
		"ControlName"		"Label"
		"textAlignment"		"center"
		"fgcolor_override"	"255 255 255 255"
		"font"				"DefaultExtraLarge"
	}

	"NewLabel"
	{
		"fieldName"			"NewLabel"
		"xpos"				"60"
		"ypos"				"4"
		"wide"				"25"
		"tall"				"15"
		"ControlName"		"Label"
		"textAlignment"		"north-east"
		"allcaps"			"1"
		"fgcolor_override"	"255 255 255 255"
		"font"				"DefaultMedium"
	}

	"CantEquipLabel"
	{
		"fieldName"			"CantEquipLabel"
		"xpos"				"2"
		"ypos"				"3"
		"wide"				"86"
		"tall"				"86"
		"ControlName"		"Label"
		"textAlignment"		"center"
		"fgcolor_override"	"255 255 255 255"
		"bgcolor_override"	"0 0 0 224"
		"font"				"DefaultSmall"
		"wrap"				"1"
		"centerwrap"		"1"
	}

	"ClassIcon"
	{
		"fieldName"			"ClassIcon"
		"xpos"				"4"
		"ypos"				"77"
		"wide"				"10"
		"tall"				"10"
		"ControlName"		"ImagePanel"
		"zpos"				"1"
		"scaleImage"		"1"
	}

	"ClassLabel"
	{
		"fieldName"			"ClassLabel"
		"xpos"				"9"
		"ypos"				"77"
		"wide"				"79"
		"tall"				"10"
		"ControlName"		"Label"
		"textAlignment"		"west"
		"fgcolor_override"	"255 255 255 255"
		"bgcolor_override"	"0 0 0 224"
		"font"				"DefaultSmall"
		"use_proportional_insets"	"1"
		"textinsetx"		"18"
	}

	"InfoButton"
	{
		"fieldName"			"InfoButton"
		"xpos"				"63"
		"ypos"				"63"
		"wide"				"25"
		"tall"				"25"
		"ControlName"		"CBitmapButton"
		"zpos"				"1000"
		"command"			"ShowInfo"
		"scaleImage"		"1"
	}
}
