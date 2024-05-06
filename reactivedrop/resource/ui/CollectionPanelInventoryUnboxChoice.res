"Resource/UI/CollectionPanelInventoryUnboxChoice.res"
{
	"OptionsModal"
	{
		"ControlName"		"CRD_Collection_Panel_Inventory_Unbox_Choice"
		"fieldName"			"OptionsModal"
		"xpos"				"c-280"	[!$WIN32WIDE]
		"xpos"				"c-310"	[$WIN32WIDE]
		"ypos"				"40"
		"wide"				"450"
		"tall"				"410"
	}
	"LblTitle"
	{
		"ControlName"	"Label"
		"fieldName"		"LblTitle"
		"xpos"			"0"
		"ypos"			"0"
		"wide"			"450"
		"tall"			"15"
		"font"			"DefaultLarge"
		"textAlignment"	"north"
		"fgcolor_override"	"255 255 255 255"
	}
	"LblFlavor"
	{
		"ControlName"	"Label"
		"fieldName"		"LblFlavor"
		"xpos"			"50"
		"ypos"			"15"
		"wide"			"350"
		"tall"			"25"
		"textAlignment"	"north"
		"wrap"			"1"
		"centerwrap"	"1"
		"fgcolor_override"	"180 180 180 255"
	}
	"LblOptionNumber"
	{
		"ControlName"	"Label"
		"fieldName"		"LblOptionNumber"
		"xpos"			"100"
		"ypos"			"40"
		"wide"			"250"
		"tall"			"12"
		"textAlignment"	"north"
		"fgcolor_override"	"96 96 96 255"
	}
	"LblItemName"
	{
		"ControlName"	"Label"
		"fieldName"		"LblItemName"
		"xpos"			"50"
		"ypos"			"50"
		"wide"			"350"
		"tall"			"20"
		"font"			"DefaultExtraLarge"
		"textAlignment"	"north"
		"wrap"			"1"
		"centerwrap"	"1"
	}
	"LblItemType"
	{
		"ControlName"	"Label"
		"fieldName"		"LblItemType"
		"xpos"			"50"
		"ypos"			"70"
		"wide"			"350"
		"tall"			"10"
		"textAlignment"	"north"
		"fgcolor_override"	"170 170 170 255"
	}
	"ImgItemIcon"
	{
		"ControlName"	"ImagePanel"
		"fieldName"		"ImgItemIcon"
		"xpos"			"150"
		"ypos"			"80"
		"wide"			"150"
		"tall"			"150"
		"scaleImage"	"1"
	}
	"LblDescription"
	{
		"ControlName"	"MultiFontRichText"
		"fieldName"		"LblDescription"
		"xpos"			"50"
		"ypos"			"230"
		"wide"			"350"
		"tall"			"70"
		"scrollbar"		"0"
	}
	"BtnPrevious"
	{
		"ControlName"	"CBitmapButton"
		"fieldName"		"BtnPrevious"
		"xpos"			"70"
		"ypos"			"80"
		"wide"			"75"
		"tall"			"150"
		"command"		"PreviousSelection"
		"enabledImage"
		{
			"material"		"vgui/TODO"
			"color"			"224 224 224 255"
		}
		"mouseOverImage"
		{
			"material"		"vgui/TODO"
			"color"			"255 255 255 255"
		}
		"pressedImage"
		{
			"material"		"vgui/TODO"
			"color"			"255 255 255 255"
		}
		"disabledImage"
		{
			"material"		"vgui/TODO"
			"color"			"96 96 96 255"
		}
	}
	"BtnNext"
	{
		"ControlName"	"CBitmapButton"
		"fieldName"		"BtnNext"
		"xpos"			"305"
		"ypos"			"80"
		"wide"			"75"
		"tall"			"150"
		"command"		"NextSelection"
		"enabledImage"
		{
			"material"		"vgui/TODO"
			"color"			"224 224 224 255"
		}
		"mouseOverImage"
		{
			"material"		"vgui/TODO"
			"color"			"255 255 255 255"
		}
		"pressedImage"
		{
			"material"		"vgui/TODO"
			"color"			"255 255 255 255"
		}
		"disabledImage"
		{
			"material"		"vgui/TODO"
			"color"			"96 96 96 255"
		}
	}
	"PnlDetails"
	{
		"ControlName"	"Panel"
		"fieldName"		"PnlDetails"
		"xpos"			"50"
		"ypos"			"300"
		"wide"			"350"
		"tall"			"80"
	}
	"BtnConfirm"
	{
		"ControlName"	"CNB_Button"
		"fieldName"		"BtnConfirm"
		"xpos"			"250"
		"ypos"			"385"
		"wide"			"150"
		"tall"			"23"
		"font"			"DefaultMedium"
		"textAlignment"	"center"
	}
	"BtnCancel"
	{
		"ControlName"	"CNB_Button"
		"fieldName"		"BtnCancel"
		"xpos"			"50"
		"ypos"			"385"
		"wide"			"150"
		"tall"			"23"
		"font"			"DefaultMedium"
		"textAlignment"	"center"
	}
}
