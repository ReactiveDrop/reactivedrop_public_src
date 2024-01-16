"Resource/UI/CRD_VGUI_Notifications_List_Item_HoIAF_Bounty.res"
{
	"Notification"
	{
		"ControlName"		"CRD_VGUI_Notifications_List_Item_HoIAF_Bounty"
		"fieldName"			"Notification"
		"wide"				"185"
		"tall"				"55"
		"font_title"		"DefaultTextBold"
		"font_desc"			"Default"
		"title_color"		"224 224 224 255"
		"description_color"	"160 160 160 255"
		"bgcolor_hover"		"20 59 96 255"
		"bgcolor_fresh"		"24 43 66 255"
		"bgcolor_viewed"	"40 48 56 255"
	}

	"NotificationText"
	{
		"ControlName"		"MultiFontRichText"
		"fieldName"			"NotificationText"
		"xpos"				"3"
		"ypos"				"3"
		"wide"				"179"
		"tall"				"15"
		"scrollbar"			"0"
	}

	"LblAge"
	{
		"ControlName"		"Label"
		"fieldName"			"LblAge"
		"xpos"				"162"
		"ypos"				"3"
		"wide"				"20"
		"tall"				"10"
		"textAlignment"		"north-east"
		"font"				"DefaultVerySmall"
	}

	"LblExpires"
	{
		"ControlName"		"Label"
		"fieldName"			"LblExpires"
		"pin_to_sibling"	"NotificationText"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"2"
		"wide"				"179"
		"tall"				"12"
		"textAlignment"		"north-west"
	}

	"ImgCompleted0"
	{
		"ControlName"		"ImagePanel"
		"fieldName"			"ImgCompleted0"
		"pin_to_sibling"	"LblExpires"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"2"
		"zpos"				"1"
		"wide"				"20"
		"tall"				"15"
		"scaleImage"		"1"
		"image"				"swarm/bounty_complete_overlay"
	}

	"ImgMissionIcon0"
	{
		"ControlName"		"ImagePanel"
		"fieldName"			"ImgMissionIcon0"
		"pin_to_sibling"	"LblExpires"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"2"
		"wide"				"20"
		"tall"				"15"
		"scaleImage"		"1"
	}

	"LblMissionPoints0"
	{
		"ControlName"		"Label"
		"fieldName"			"LblMissionPoints0"
		"pin_to_sibling"	"ImgMissionIcon0"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"2"
		"wide"				"20"
		"tall"				"10"
		"fgcolor_override"	"255 255 255 255"
		"textAlignment"		"center"
		"font"				"DefaultVerySmall"
	}

	"ImgCompleted1"
	{
		"ControlName"		"ImagePanel"
		"fieldName"			"ImgCompleted1"
		"pin_to_sibling"	"ImgMissionIcon0"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"1"
		"zpos"				"1"
		"wide"				"20"
		"tall"				"15"
		"scaleImage"		"1"
		"image"				"swarm/bounty_complete_overlay"
	}

	"ImgMissionIcon1"
	{
		"ControlName"		"ImagePanel"
		"fieldName"			"ImgMissionIcon1"
		"pin_to_sibling"	"ImgMissionIcon0"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"1"
		"wide"				"20"
		"tall"				"15"
		"scaleImage"		"1"
	}

	"LblMissionPoints1"
	{
		"ControlName"		"Label"
		"fieldName"			"LblMissionPoints1"
		"pin_to_sibling"	"LblMissionPoints0"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"1"
		"wide"				"20"
		"tall"				"10"
		"fgcolor_override"	"255 255 255 255"
		"textAlignment"		"center"
		"font"				"DefaultVerySmall"
	}

	"ImgCompleted2"
	{
		"ControlName"		"ImagePanel"
		"fieldName"			"ImgCompleted2"
		"pin_to_sibling"	"ImgMissionIcon1"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"1"
		"zpos"				"1"
		"wide"				"20"
		"tall"				"15"
		"scaleImage"		"1"
		"image"				"swarm/bounty_complete_overlay"
	}

	"ImgMissionIcon2"
	{
		"ControlName"		"ImagePanel"
		"fieldName"			"ImgMissionIcon2"
		"pin_to_sibling"	"ImgMissionIcon1"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"1"
		"wide"				"20"
		"tall"				"15"
		"scaleImage"		"1"
	}

	"LblMissionPoints2"
	{
		"ControlName"		"Label"
		"fieldName"			"LblMissionPoints2"
		"pin_to_sibling"	"LblMissionPoints1"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"1"
		"wide"				"20"
		"tall"				"10"
		"fgcolor_override"	"255 255 255 255"
		"textAlignment"		"center"
		"font"				"DefaultVerySmall"
	}

	"ImgCompleted3"
	{
		"ControlName"		"ImagePanel"
		"fieldName"			"ImgCompleted3"
		"pin_to_sibling"	"ImgMissionIcon2"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"1"
		"zpos"				"1"
		"wide"				"20"
		"tall"				"15"
		"scaleImage"		"1"
		"image"				"swarm/bounty_complete_overlay"
	}

	"ImgMissionIcon3"
	{
		"ControlName"		"ImagePanel"
		"fieldName"			"ImgMissionIcon3"
		"pin_to_sibling"	"ImgMissionIcon2"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"1"
		"wide"				"20"
		"tall"				"15"
		"scaleImage"		"1"
	}

	"LblMissionPoints3"
	{
		"ControlName"		"Label"
		"fieldName"			"LblMissionPoints3"
		"pin_to_sibling"	"LblMissionPoints2"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"1"
		"wide"				"20"
		"tall"				"10"
		"fgcolor_override"	"255 255 255 255"
		"textAlignment"		"center"
		"font"				"DefaultVerySmall"
	}

	"ImgCompleted4"
	{
		"ControlName"		"ImagePanel"
		"fieldName"			"ImgCompleted4"
		"pin_to_sibling"	"ImgMissionIcon3"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"1"
		"zpos"				"1"
		"wide"				"20"
		"tall"				"15"
		"scaleImage"		"1"
		"image"				"swarm/bounty_complete_overlay"
	}

	"ImgMissionIcon4"
	{
		"ControlName"		"ImagePanel"
		"fieldName"			"ImgMissionIcon4"
		"pin_to_sibling"	"ImgMissionIcon3"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"1"
		"wide"				"20"
		"tall"				"15"
		"scaleImage"		"1"
	}

	"LblMissionPoints4"
	{
		"ControlName"		"Label"
		"fieldName"			"LblMissionPoints4"
		"pin_to_sibling"	"LblMissionPoints3"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"1"
		"wide"				"20"
		"tall"				"10"
		"fgcolor_override"	"255 255 255 255"
		"textAlignment"		"center"
		"font"				"DefaultVerySmall"
	}

	"ImgCompleted5"
	{
		"ControlName"		"ImagePanel"
		"fieldName"			"ImgCompleted5"
		"pin_to_sibling"	"ImgMissionIcon4"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"1"
		"zpos"				"1"
		"wide"				"20"
		"tall"				"15"
		"scaleImage"		"1"
		"image"				"swarm/bounty_complete_overlay"
	}

	"ImgMissionIcon5"
	{
		"ControlName"		"ImagePanel"
		"fieldName"			"ImgMissionIcon5"
		"pin_to_sibling"	"ImgMissionIcon4"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"1"
		"wide"				"20"
		"tall"				"15"
		"scaleImage"		"1"
	}

	"LblMissionPoints5"
	{
		"ControlName"		"Label"
		"fieldName"			"LblMissionPoints5"
		"pin_to_sibling"	"LblMissionPoints4"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"1"
		"wide"				"20"
		"tall"				"10"
		"fgcolor_override"	"255 255 255 255"
		"textAlignment"		"center"
		"font"				"DefaultVerySmall"
	}

	"ImgCompleted6"
	{
		"ControlName"		"ImagePanel"
		"fieldName"			"ImgCompleted6"
		"pin_to_sibling"	"ImgMissionIcon5"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"1"
		"zpos"				"1"
		"wide"				"20"
		"tall"				"15"
		"scaleImage"		"1"
		"image"				"swarm/bounty_complete_overlay"
	}

	"ImgMissionIcon6"
	{
		"ControlName"		"ImagePanel"
		"fieldName"			"ImgMissionIcon6"
		"pin_to_sibling"	"ImgMissionIcon5"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"1"
		"wide"				"20"
		"tall"				"15"
		"scaleImage"		"1"
	}

	"LblMissionPoints6"
	{
		"ControlName"		"Label"
		"fieldName"			"LblMissionPoints6"
		"pin_to_sibling"	"LblMissionPoints5"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"1"
		"wide"				"20"
		"tall"				"10"
		"fgcolor_override"	"255 255 255 255"
		"textAlignment"		"center"
		"font"				"DefaultVerySmall"
	}

	"ImgCompleted7"
	{
		"ControlName"		"ImagePanel"
		"fieldName"			"ImgCompleted7"
		"pin_to_sibling"	"ImgMissionIcon6"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"1"
		"zpos"				"1"
		"wide"				"20"
		"tall"				"15"
		"scaleImage"		"1"
		"image"				"swarm/bounty_complete_overlay"
	}

	"ImgMissionIcon7"
	{
		"ControlName"		"ImagePanel"
		"fieldName"			"ImgMissionIcon7"
		"pin_to_sibling"	"ImgMissionIcon6"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"1"
		"wide"				"20"
		"tall"				"15"
		"scaleImage"		"1"
	}

	"LblMissionPoints7"
	{
		"ControlName"		"Label"
		"fieldName"			"LblMissionPoints7"
		"pin_to_sibling"	"LblMissionPoints6"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"1"
		"wide"				"20"
		"tall"				"10"
		"fgcolor_override"	"255 255 255 255"
		"textAlignment"		"center"
		"font"				"DefaultVerySmall"
	}

	"ImgCompleted8"
	{
		"ControlName"		"ImagePanel"
		"fieldName"			"ImgCompleted8"
		"pin_to_sibling"	"ImgMissionIcon7"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"1"
		"zpos"				"1"
		"wide"				"20"
		"tall"				"15"
		"scaleImage"		"1"
		"image"				"swarm/bounty_complete_overlay"
	}

	"ImgMissionIcon8"
	{
		"ControlName"		"ImagePanel"
		"fieldName"			"ImgMissionIcon8"
		"pin_to_sibling"	"ImgMissionIcon7"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"1"
		"wide"				"20"
		"tall"				"15"
		"scaleImage"		"1"
	}

	"LblMissionPoints8"
	{
		"ControlName"		"Label"
		"fieldName"			"LblMissionPoints8"
		"pin_to_sibling"	"LblMissionPoints7"
		"pin_corner_to_sibling"	"0"
		"pin_to_sibling_corner"	"1"
		"wide"				"20"
		"tall"				"10"
		"fgcolor_override"	"255 255 255 255"
		"textAlignment"		"center"
		"font"				"DefaultVerySmall"
	}
}
