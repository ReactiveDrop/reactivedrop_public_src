"Resource/UI/BaseModUI/Loadouts.res"
{
	"Loadouts"
	{
		"ControlName"		"Frame"
		"fieldName"			"Loadouts"
		"xpos"				"0"
		"ypos"				"0"
		"wide"				"f0"
		"tall"				"f0"
		"visible"			"1"
		"enabled"			"1"
	}

	"TopBar"
	{
		"ControlName"			"CRD_VGUI_Main_Menu_Top_Bar"
		"fieldName"				"TopBar"
		"visible"				"1"
		"enabled"				"1"
		"navUp"					"BtnLoadouts"
		"navDown"				"BtnMarine0"
	}

	"StockTickerHelper"
	{
		"ControlName"		"CRD_VGUI_Stock_Ticker_Helper"
		"fieldName"			"StockTickerHelper"
		"visible"			"1"
		"enabled"			"1"
	}

	"GplSavedLoadouts"
	{
		"ControlName"		"GenericPanelList"
		"fieldName"			"GplSavedLoadouts"
		"xpos"				"c-290"
		"ypos"				"60"
		"wide"				"155"
		"tall"				"354"
		"bgcolor_override"	"0 0 0 0"
		"navRight"			"BtnMarine0"
		"navUp"				"BtnCreateLoadout"
		"navDown"			"BtnBrowseWorkshop"
	}

	"LblMedals"
	{
		"ControlName"		"Label"
		"fieldName"			"LblMedals"
		"xpos"				"c-100"
		"ypos"				"320"
		"wide"				"375"
		"tall"				"20"
		"labelText"			"#rd_loadout_equipped_medals"
		"textAlignment"		"center"
		"fgcolor_override"	"128 128 128 255"
		"font"				"DefaultExtraLarge"
	}

	"MedalSlot0"
	{
		"ControlName"		"CRD_VGUI_Loadout_Slot_Inventory"
		"fieldName"			"MedalSlot0"
		"xpos"				"c-80"
		"ypos"				"345"
		"wide"				"75"
		"tall"				"75"
		"navLeft"			"BtnBrowseWorkshop"
		"navRight"			"BtnDeleteLoadout"
		"navUp"				"BtnMarine4"
		"navDown"			"BtnDeleteLoadout"
	}

	"LblMedal0"
	{
		"ControlName"		"Label"
		"fieldName"			"LblMedal0"
		"xpos"				"c-80"
		"ypos"				"425"
		"wide"				"75"
		"tall"				"24"
		"wrap"				"1"
		"centerWrap"		"1"
		"textAlignment"		"north"
		"font"				"Default"
		"fgcolor_override"	"192 192 192 255"
	}

	"MedalSlot1"
	{
		"ControlName"		"CRD_VGUI_Loadout_Slot_Inventory"
		"fieldName"			"MedalSlot1"
		"xpos"				"c0"
		"ypos"				"345"
		"wide"				"75"
		"tall"				"75"
		"navLeft"			"BtnViewOnWorkshop"
		"navRight"			"BtnCopyToLive"
		"navUp"				"BtnMarine5"
		"navDown"			"BtnCopyToLive"
	}

	"LblMedal1"
	{
		"ControlName"		"Label"
		"fieldName"			"LblMedal1"
		"xpos"				"c0"
		"ypos"				"425"
		"wide"				"75"
		"tall"				"24"
		"wrap"				"1"
		"centerWrap"		"1"
		"textAlignment"		"north"
		"font"				"Default"
		"fgcolor_override"	"192 192 192 255"
	}

	"MedalSlot2"
	{
		"ControlName"		"CRD_VGUI_Loadout_Slot_Inventory"
		"fieldName"			"MedalSlot2"
		"xpos"				"c80"
		"ypos"				"345"
		"wide"				"75"
		"tall"				"75"
		"navLeft"			"BtnCopyToLive"
		"navRight"			"BtnCopyFromLive"
		"navUp"				"BtnMarine6"
		"navDown"			"BtnCopyFromLive"
	}

	"LblMedal2"
	{
		"ControlName"		"Label"
		"fieldName"			"LblMedal2"
		"xpos"				"c80"
		"ypos"				"425"
		"wide"				"75"
		"tall"				"24"
		"wrap"				"1"
		"centerWrap"		"1"
		"textAlignment"		"north"
		"font"				"Default"
		"fgcolor_override"	"192 192 192 255"
	}

	"ImgPromotionIcon"
	{
		"ControlName"		"ImagePanel"
		"fieldName"			"ImgPromotionIcon"
		"xpos"				"c175"
		"ypos"				"345"
		"wide"				"75"
		"tall"				"75"
		"scaleImage"		"1"
	}

	"LblPromotion"
	{
		"ControlName"		"Label"
		"fieldName"			"LblPromotion"
		"xpos"				"c175"
		"ypos"				"425"
		"wide"				"75"
		"tall"				"24"
		"wrap"				"1"
		"centerWrap"		"1"
		"textAlignment"		"north"
		"font"				"Default"
		"fgcolor_override"	"192 192 192 255"
	}

	"LblMarines"
	{
		"ControlName"		"Label"
		"fieldName"			"LblMarines"
		"xpos"				"c-100"
		"ypos"				"40"
		"wide"				"375"
		"tall"				"20"
		"labelText"			"#rd_loadout_equipped_marines"
		"textAlignment"		"center"
		"fgcolor_override"	"128 128 128 255"
		"font"				"DefaultExtraLarge"
	}

	"ImgMarineClassOfficer"
	{
		"ControlName"		"ImagePanel"
		"fieldName"			"ImgMarineClassOfficer"
		"xpos"				"c-100"
		"ypos"				"70"
		"wide"				"15"
		"tall"				"15"
		"scaleImage"		"1"
		"image"				"swarm/ClassIcons/NCOClassIcon"
	}

	"LblMarineClassOfficer"
	{
		"ControlName"		"Label"
		"fieldName"			"LblMarineClassOfficer"
		"xpos"				"c-83"
		"ypos"				"70"
		"wide"				"58"
		"tall"				"15"
		"textAlignment"		"center"
		"font"				"DefaultMedium"
		"labelText"			"#marine_class_officer"
		"fgcolor_override"	"192 192 192 255"
	}

	"ImgMarineClassSpecialWeapons"
	{
		"ControlName"		"ImagePanel"
		"fieldName"			"ImgMarineClassSpecialWeapons"
		"xpos"				"c0"
		"ypos"				"70"
		"wide"				"15"
		"tall"				"15"
		"scaleImage"		"1"
		"image"				"swarm/ClassIcons/SpecialWeaponsClassIcon"
	}

	"LblMarineClassSpecialWeapons"
	{
		"ControlName"		"Label"
		"fieldName"			"LblMarineClassSpecialWeapons"
		"xpos"				"c17"
		"ypos"				"70"
		"wide"				"58"
		"tall"				"15"
		"textAlignment"		"center"
		"font"				"DefaultMedium"
		"labelText"			"#marine_class_sw_short"
		"fgcolor_override"	"192 192 192 255"
	}

	"ImgMarineClassMedic"
	{
		"ControlName"		"ImagePanel"
		"fieldName"			"ImgMarineClassMedic"
		"xpos"				"c100"
		"ypos"				"70"
		"wide"				"15"
		"tall"				"15"
		"scaleImage"		"1"
		"image"				"swarm/ClassIcons/MedicClassIcon"
	}

	"LblMarineClassMedic"
	{
		"ControlName"		"Label"
		"fieldName"			"LblMarineClassMedic"
		"xpos"				"c117"
		"ypos"				"70"
		"wide"				"58"
		"tall"				"15"
		"textAlignment"		"center"
		"font"				"DefaultMedium"
		"labelText"			"#marine_class_medic"
		"fgcolor_override"	"192 192 192 255"
	}

	"ImgMarineClassTech"
	{
		"ControlName"		"ImagePanel"
		"fieldName"			"ImgMarineClassTech"
		"xpos"				"c200"
		"ypos"				"70"
		"wide"				"15"
		"tall"				"15"
		"scaleImage"		"1"
		"image"				"swarm/ClassIcons/TechClassIcon"
	}

	"LblMarineClassTech"
	{
		"ControlName"		"Label"
		"fieldName"			"LblMarineClassTech"
		"xpos"				"c217"
		"ypos"				"70"
		"wide"				"58"
		"tall"				"15"
		"textAlignment"		"center"
		"font"				"DefaultMedium"
		"labelText"			"#marine_class_tech"
		"fgcolor_override"	"192 192 192 255"
	}

	"BtnMarine0"
	{
		"ControlName"		"CBitmapButton"
		"fieldName"			"BtnMarine0"
		"xpos"				"c-100"
		"ypos"				"100"
		"wide"				"75"
		"tall"				"75"
		"command"			"MarineLoadout0"
		"enabledImage" {
			"material"	"vgui/briefing/face_sarge"
			"color"		"255 255 255 255"
		}
		"mouseOverImage" {
			"material"	"vgui/briefing/face_sarge_lit"
			"color"		"255 255 255 255"
		}
		"pressedImage" {
			"material"	"vgui/briefing/face_sarge_lit"
			"color"		"255 255 255 255"
		}
		"disabledImage" {
			"material"	"vgui/briefing/face_sarge"
			"color"		"255 255 255 255"
		}
		"navLeft"			"GplSavedLoadouts"
		"navRight"			"BtnMarine1"
		"navUp"				"TopBar"
		"navDown"			"BtnMarine4"
		"tabindex"			"0"
	}

	"LblMarine0"
	{
		"ControlName"		"Label"
		"fieldName"			"LblMarine0"
		"xpos"				"c-100"
		"ypos"				"177"
		"wide"				"75"
		"tall"				"15"
		"textAlignment"		"center"
		"font"				"DefaultMedium"
		"labelText"			"#asw_name_sarge"
		"fgcolor_override"	"192 192 192 255"
	}

	"BtnMarine1"
	{
		"ControlName"		"CBitmapButton"
		"fieldName"			"BtnMarine1"
		"xpos"				"c0"
		"ypos"				"100"
		"wide"				"75"
		"tall"				"75"
		"command"			"MarineLoadout1"
		"enabledImage" {
			"material"	"vgui/briefing/face_wildcat"
			"color"		"255 255 255 255"
		}
		"mouseOverImage" {
			"material"	"vgui/briefing/face_wildcat_lit"
			"color"		"255 255 255 255"
		}
		"pressedImage" {
			"material"	"vgui/briefing/face_wildcat_lit"
			"color"		"255 255 255 255"
		}
		"disabledImage" {
			"material"	"vgui/briefing/face_wildcat"
			"color"		"255 255 255 255"
		}
		"navLeft"			"BtnMarine0"
		"navRight"			"BtnMarine2"
		"navUp"				"TopBar"
		"navDown"			"BtnMarine5"
	}

	"LblMarine1"
	{
		"ControlName"		"Label"
		"fieldName"			"LblMarine1"
		"xpos"				"c0"
		"ypos"				"177"
		"wide"				"75"
		"tall"				"15"
		"textAlignment"		"center"
		"font"				"DefaultMedium"
		"labelText"			"#asw_name_wildcat"
		"fgcolor_override"	"192 192 192 255"
	}

	"BtnMarine2"
	{
		"ControlName"		"CBitmapButton"
		"fieldName"			"BtnMarine2"
		"xpos"				"c100"
		"ypos"				"100"
		"wide"				"75"
		"tall"				"75"
		"command"			"MarineLoadout2"
		"enabledImage" {
			"material"	"vgui/briefing/face_faith"
			"color"		"255 255 255 255"
		}
		"mouseOverImage" {
			"material"	"vgui/briefing/face_faith_lit"
			"color"		"255 255 255 255"
		}
		"pressedImage" {
			"material"	"vgui/briefing/face_faith_lit"
			"color"		"255 255 255 255"
		}
		"disabledImage" {
			"material"	"vgui/briefing/face_faith"
			"color"		"255 255 255 255"
		}
		"navLeft"			"BtnMarine1"
		"navRight"			"BtnMarine3"
		"navUp"				"TopBar"
		"navDown"			"BtnMarine6"
	}

	"LblMarine2"
	{
		"ControlName"		"Label"
		"fieldName"			"LblMarine2"
		"xpos"				"c100"
		"ypos"				"177"
		"wide"				"75"
		"tall"				"15"
		"textAlignment"		"center"
		"font"				"DefaultMedium"
		"labelText"			"#asw_name_faith"
		"fgcolor_override"	"192 192 192 255"
	}

	"BtnMarine3"
	{
		"ControlName"		"CBitmapButton"
		"fieldName"			"BtnMarine3"
		"xpos"				"c200"
		"ypos"				"100"
		"wide"				"75"
		"tall"				"75"
		"command"			"MarineLoadout3"
		"enabledImage" {
			"material"	"vgui/briefing/face_crash"
			"color"		"255 255 255 255"
		}
		"mouseOverImage" {
			"material"	"vgui/briefing/face_crash_lit"
			"color"		"255 255 255 255"
		}
		"pressedImage" {
			"material"	"vgui/briefing/face_crash_lit"
			"color"		"255 255 255 255"
		}
		"disabledImage" {
			"material"	"vgui/briefing/face_crash"
			"color"		"255 255 255 255"
		}
		"navLeft"			"BtnMarine2"
		"navRight"			"BtnMarine3"
		"navUp"				"TopBar"
		"navDown"			"BtnMarine7"
	}

	"LblMarine3"
	{
		"ControlName"		"Label"
		"fieldName"			"LblMarine3"
		"xpos"				"c200"
		"ypos"				"177"
		"wide"				"75"
		"tall"				"15"
		"textAlignment"		"center"
		"font"				"DefaultMedium"
		"labelText"			"#asw_name_crash"
		"fgcolor_override"	"192 192 192 255"
	}

	"BtnMarine4"
	{
		"ControlName"		"CBitmapButton"
		"fieldName"			"BtnMarine4"
		"xpos"				"c-100"
		"ypos"				"200"
		"wide"				"75"
		"tall"				"75"
		"command"			"MarineLoadout4"
		"enabledImage" {
			"material"	"vgui/briefing/face_jaeger"
			"color"		"255 255 255 255"
		}
		"mouseOverImage" {
			"material"	"vgui/briefing/face_jaeger_lit"
			"color"		"255 255 255 255"
		}
		"pressedImage" {
			"material"	"vgui/briefing/face_jaeger_lit"
			"color"		"255 255 255 255"
		}
		"disabledImage" {
			"material"	"vgui/briefing/face_jaeger"
			"color"		"255 255 255 255"
		}
		"navLeft"			"GplSavedLoadouts"
		"navRight"			"BtnMarine5"
		"navUp"				"BtnMarine0"
		"navDown"			"MedalSlot0"
	}

	"LblMarine4"
	{
		"ControlName"		"Label"
		"fieldName"			"LblMarine4"
		"xpos"				"c-100"
		"ypos"				"277"
		"wide"				"75"
		"tall"				"15"
		"textAlignment"		"center"
		"font"				"DefaultMedium"
		"labelText"			"#asw_name_jaeger"
		"fgcolor_override"	"192 192 192 255"
	}

	"BtnMarine5"
	{
		"ControlName"		"CBitmapButton"
		"fieldName"			"BtnMarine5"
		"xpos"				"c0"
		"ypos"				"200"
		"wide"				"75"
		"tall"				"75"
		"command"			"MarineLoadout5"
		"enabledImage" {
			"material"	"vgui/briefing/face_wolfe"
			"color"		"255 255 255 255"
		}
		"mouseOverImage" {
			"material"	"vgui/briefing/face_wolfe_lit"
			"color"		"255 255 255 255"
		}
		"pressedImage" {
			"material"	"vgui/briefing/face_wolfe_lit"
			"color"		"255 255 255 255"
		}
		"disabledImage" {
			"material"	"vgui/briefing/face_wolfe"
			"color"		"255 255 255 255"
		}
		"navLeft"			"BtnMarine4"
		"navRight"			"BtnMarine6"
		"navUp"				"BtnMarine1"
		"navDown"			"MedalSlot1"
	}

	"LblMarine5"
	{
		"ControlName"		"Label"
		"fieldName"			"LblMarine5"
		"xpos"				"c0"
		"ypos"				"277"
		"wide"				"75"
		"tall"				"15"
		"textAlignment"		"center"
		"font"				"DefaultMedium"
		"labelText"			"#asw_name_wolfe"
		"fgcolor_override"	"192 192 192 255"
	}

	"BtnMarine6"
	{
		"ControlName"		"CBitmapButton"
		"fieldName"			"BtnMarine6"
		"xpos"				"c100"
		"ypos"				"200"
		"wide"				"75"
		"tall"				"75"
		"command"			"MarineLoadout6"
		"enabledImage" {
			"material"	"vgui/briefing/face_bastille"
			"color"		"255 255 255 255"
		}
		"mouseOverImage" {
			"material"	"vgui/briefing/face_bastille_lit"
			"color"		"255 255 255 255"
		}
		"pressedImage" {
			"material"	"vgui/briefing/face_bastille_lit"
			"color"		"255 255 255 255"
		}
		"disabledImage" {
			"material"	"vgui/briefing/face_bastille"
			"color"		"255 255 255 255"
		}
		"navLeft"			"BtnMarine5"
		"navRight"			"BtnMarine7"
		"navUp"				"BtnMarine2"
		"navDown"			"MedalSlot2"
	}

	"LblMarine6"
	{
		"ControlName"		"Label"
		"fieldName"			"LblMarine6"
		"xpos"				"c100"
		"ypos"				"277"
		"wide"				"75"
		"tall"				"15"
		"textAlignment"		"center"
		"font"				"DefaultMedium"
		"labelText"			"#asw_name_bastille"
		"fgcolor_override"	"192 192 192 255"
	}

	"BtnMarine7"
	{
		"ControlName"		"CBitmapButton"
		"fieldName"			"BtnMarine7"
		"xpos"				"c200"
		"ypos"				"200"
		"wide"				"75"
		"tall"				"75"
		"command"			"MarineLoadout7"
		"enabledImage" {
			"material"	"vgui/briefing/face_vegas"
			"color"		"255 255 255 255"
		}
		"mouseOverImage" {
			"material"	"vgui/briefing/face_vegas_lit"
			"color"		"255 255 255 255"
		}
		"pressedImage" {
			"material"	"vgui/briefing/face_vegas_lit"
			"color"		"255 255 255 255"
		}
		"disabledImage" {
			"material"	"vgui/briefing/face_vegas"
			"color"		"255 255 255 255"
		}
		"navLeft"			"BtnMarine6"
		"navRight"			"BtnMarine7"
		"navUp"				"BtnMarine3"
		"navDown"			"MedalSlot2"
	}

	"LblMarine7"
	{
		"ControlName"		"Label"
		"fieldName"			"LblMarine7"
		"xpos"				"c200"
		"ypos"				"277"
		"wide"				"75"
		"tall"				"15"
		"textAlignment"		"center"
		"font"				"DefaultMedium"
		"labelText"			"#asw_name_vegas"
		"fgcolor_override"	"192 192 192 255"
	}

	"BtnCreateLoadout"
	{
		"ControlName"		"BaseModHybridButton"
		"fieldName"			"BtnCreateLoadout"
		"xpos"				"c-290"
		"ypos"				"40"
		"wide"				"155"
		"tall"				"20"
		"labelText"			"#rd_loadout_new"
		"command"			"CreateNewLoadout"
		"navUp"				"TopBar"
		"navDown"			"GplSavedLoadouts"
		"navLeft"			"BtnCreateLoadout"
		"navRight"			"BtnMarine0"
	}

	"BtnDeleteLoadout"
	{
		"ControlName"		"BaseModHybridButton"
		"fieldName"			"BtnDeleteLoadout"
		"xpos"				"c-100"
		"ypos"				"345"
		"wide"				"110"
		"tall"				"20"
		"labelText"			"#rd_loadout_delete"
		"command"			"DeleteLoadout"
		"navUp"				"MedalSlot0"
		"navDown"			"BtnViewOnWorkshop"
		"navLeft"			"MedalSlot0"
		"navRight"			"BtnViewOnWorkshop"
	}

	"BtnCopyToLive"
	{
		"ControlName"		"BaseModHybridButton"
		"fieldName"			"BtnCopyToLive"
		"xpos"				"c20"
		"ypos"				"345"
		"wide"				"110"
		"tall"				"20"
		"labelText"			"#rd_loadout_load"
		"command"			"CopyToLive"
		"navUp"				"MedalSlot1"
		"navLeft"			"MedalSlot1"
		"navRight"			"MedalSlot2"
	}

	"BtnCopyFromLive"
	{
		"ControlName"		"BaseModHybridButton"
		"fieldName"			"BtnCopyFromLive"
		"xpos"				"c150"
		"ypos"				"345"
		"wide"				"110"
		"tall"				"20"
		"labelText"			"#rd_loadout_save"
		"command"			"CopyFromLive"
		"navUp"				"MedalSlot2"
		"navLeft"			"MedalSlot2"
	}

	"BtnViewOnWorkshop"
	{
		"ControlName"		"BaseModHybridButton"
		"fieldName"			"BtnViewOnWorkshop"
		"xpos"				"c-100"
		"ypos"				"345"
		"wide"				"110"
		"tall"				"20"
		"labelText"			"#rd_loadout_view_on_workshop"
		"command"			"ViewOnWorkshop"
		"navUp"				"BtnDeleteLoadout"
		"navLeft"			"BtnDeleteLoadout"
		"navRight"			"MedalSlot1"
	}

	"BtnBrowseWorkshop"
	{
		"ControlName"		"BaseModHybridButton"
		"fieldName"			"BtnBrowseWorkshop"
		"xpos"				"c-290"
		"ypos"				"414"
		"wide"				"155"
		"tall"				"20"
		"labelText"			"#rd_loadout_browse_workshop"
		"command"			"FindLoadoutsOnWorkshop"
		"navUp"				"GplSavedLoadouts"
		"navDown"			"BtnShare"
		"navRight"			"MedalSlot0"
	}

	"BtnShare"
	{
		"ControlName"		"BaseModHybridButton"
		"fieldName"			"BtnShare"
		"xpos"				"c-290"
		"ypos"				"434"
		"wide"				"155"
		"tall"				"20"
		"labelText"			"#rd_loadout_share_start"
		"command"			"ShareLoadoutsOnWorkshop"
		"navUp"				"BtnBrowseWorkshop"
		"navRight"			"MedalSlot0"
	}
}
