"Resource/UI/BaseModUI/CRD_VGUI_Loadout_Marine.res"
{
	"LoadoutMarine"
	{
		"ControlName"		"CRD_VGUI_Loadout_Marine"
		"fieldName"			"LoadoutMarine"
		"xpos"				"0"
		"ypos"				"0"
		"wide"				"f0"
		"tall"				"f0"
		"zpos"				"10"
	}

	"Background"
	{
		"ControlName"		"Panel"
		"fieldName"			"Background"
		"xpos"				"0"
		"ypos"				"0"
		"zpos"				"-5"
		"wide"				"f0"
		"tall"				"f0"
		"bgcolor_override"	"16 20 24 255"
	}

	"ModelPanel"
	{
		"ControlName"		"CRD_Swarmopedia_Model_Panel"
		"fieldName"			"ModelPanel"
		"xpos"				"5"
		"ypos"				"5"
		"zpos"				"-2"
		"wide"				"f10"
		"tall"				"340"
	}

	"BtnBack"
	{
		"ControlName"		"BaseModHybridButton"
		"fieldName"			"BtnBack"
		"xpos"				"8"
		"ypos"				"8"
		"wide"				"100"
		"tall"				"16"
		"labelText"			"#asw_menu_back"
		"command"			"Back"
		"style"				"DialogButton"
		"navDown"			"MarineSlot"
	}

	"BiographyBackground"
	{
		"ControlName"		"Panel"
		"fieldName"			"BiographyBackground"
		"xpos"				"-10"
		"ypos"				"245"
		"zpos"				"-1"
		"wide"				"330"
		"tall"				"120"
		"bgcolor_override"	"16 20 24 255"
		"paintbackgroundtype"	"2"
	}

	"LblBiography"
	{
		"ControlName"		"RichText"
		"fieldName"			"LblBiography"
		"xpos"				"10"
		"ypos"				"250"
		"wide"				"300"
		"tall"				"100"
		"scrollbar"			"0"
		"paintbackground"	"0"
		"fgcolor_override"	"255 255 255 255"
	}

	"ImgClass"
	{
		"ControlName"		"ImagePanel"
		"fieldName"			"ImgClass"
		"xpos"				"c-300"
		"ypos"				"340"
		"wide"				"20"
		"tall"				"20"
		"scaleImage"		"1"
	}

	"LblClass"
	{
		"ControlName"		"Label"
		"fieldName"			"LblClass"
		"xpos"				"c-277"
		"ypos"				"340"
		"wide"				"100"
		"tall"				"20"
		"font"				"DefaultBold"
		"textAlignment"		"west"
		"fgcolor_override"	"192 192 192 255"
	}

	"SkillPanelBackground"
	{
		"ControlName"		"Panel"
		"fieldName"			"SkillPanelBackground"
		"xpos"				"r170"
		"ypos"				"190"
		"zpos"				"-1"
		"wide"				"180"
		"tall"				"170"
		"bgcolor_override"	"16 20 24 255"
		"paintbackgroundtype"	"2"
	}

	"SkillPanel0"
	{
		"ControlName"		"CNB_Skill_Panel"
		"fieldName"			"SkillPanel0"
		"xpos"				"r170"
		"ypos"				"200"
		"wide"				"160"
		"tall"				"30"
	}

	"SkillPanel1"
	{
		"ControlName"		"CNB_Skill_Panel"
		"fieldName"			"SkillPanel1"
		"xpos"				"r170"
		"ypos"				"230"
		"wide"				"160"
		"tall"				"30"
	}

	"SkillPanel2"
	{
		"ControlName"		"CNB_Skill_Panel"
		"fieldName"			"SkillPanel2"
		"xpos"				"r170"
		"ypos"				"260"
		"wide"				"160"
		"tall"				"30"
	}

	"SkillPanel3"
	{
		"ControlName"		"CNB_Skill_Panel"
		"fieldName"			"SkillPanel3"
		"xpos"				"r170"
		"ypos"				"290"
		"wide"				"160"
		"tall"				"30"
	}

	"SkillPanel4"
	{
		"ControlName"		"CNB_Skill_Panel"
		"fieldName"			"SkillPanel4"
		"xpos"				"r170"
		"ypos"				"320"
		"wide"				"160"
		"tall"				"30"
	}

	"MarineSlot"
	{
		"ControlName"		"CRD_VGUI_Loadout_Slot_Marine"
		"fieldName"			"MarineSlot"
		"xpos"				"c-280"
		"ypos"				"360"
		"wide"				"75"
		"tall"				"75"
		"navUp"				"BtnBack"
		"navRight"			"WeaponSlot0"
	}

	"LblMarineSlot"
	{
		"ControlName"		"Label"
		"fieldName"			"LblMarineSlot"
		"xpos"				"c-315"
		"ypos"				"435"
		"wide"				"146"
		"tall"				"60"
		"textAlignment"		"north"
		"font"				"DefaultBold"
		"centerWrap"		"1"
		"fgcolor_override"	"192 192 192 255"
	}

	"WeaponSlot0"
	{
		"ControlName"		"CRD_VGUI_Loadout_Slot_Weapon"
		"fieldName"			"WeaponSlot0"
		"xpos"				"c-180"
		"ypos"				"360"
		"wide"				"150"
		"tall"				"75"
		"navUp"				"BtnBack"
		"navLeft"			"MarineSlot"
		"navRight"			"WeaponSlot1"
	}

	"LblWeaponSlot0"
	{
		"ControlName"		"Label"
		"fieldName"			"LblWeaponSlot0"
		"xpos"				"c-178"
		"ypos"				"435"
		"wide"				"146"
		"tall"				"60"
		"textAlignment"		"north"
		"centerWrap"		"1"
		"font"				"DefaultBold"
		"fgcolor_override"	"192 192 192 255"
	}

	"WeaponSlot1"
	{
		"ControlName"		"CRD_VGUI_Loadout_Slot_Weapon"
		"fieldName"			"WeaponSlot1"
		"xpos"				"c-5"
		"ypos"				"360"
		"wide"				"150"
		"tall"				"75"
		"navUp"				"BtnBack"
		"navLeft"			"WeaponSlot0"
		"navRight"			"WeaponSlot2"
	}

	"LblWeaponSlot1"
	{
		"ControlName"		"Label"
		"fieldName"			"LblWeaponSlot1"
		"xpos"				"c-3"
		"ypos"				"435"
		"wide"				"146"
		"tall"				"60"
		"textAlignment"		"north"
		"centerWrap"		"1"
		"font"				"DefaultBold"
		"fgcolor_override"	"192 192 192 255"
	}

	"WeaponSlot2"
	{
		"ControlName"		"CRD_VGUI_Loadout_Slot_Weapon"
		"fieldName"			"WeaponSlot2"
		"xpos"				"c170"
		"ypos"				"360"
		"wide"				"75"
		"tall"				"75"
		"navUp"				"BtnBack"
		"navLeft"			"WeaponSlot1"
	}

	"LblWeaponSlot2"
	{
		"ControlName"		"Label"
		"fieldName"			"LblWeaponSlot2"
		"xpos"				"c135"
		"ypos"				"435"
		"wide"				"146"
		"tall"				"60"
		"textAlignment"		"north"
		"centerWrap"		"1"
		"font"				"DefaultBold"
		"fgcolor_override"	"192 192 192 255"
	}
}