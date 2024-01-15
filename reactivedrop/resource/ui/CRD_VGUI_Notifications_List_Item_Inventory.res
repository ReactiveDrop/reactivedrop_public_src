"Resource/UI/CRD_VGUI_Notifications_List_Item_Inventory.res"
{
	"Notification"
	{
		"ControlName"		"CRD_VGUI_Notifications_List_Item_Inventory"
		"fieldName"			"Notification"
		"wide"				"185"
		"tall"				"20"
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
		"tall"				"14"
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
		"tall"				"10"
		"textAlignment"		"north-west"
	}
}
