"Resource/HudLayout.res"
{
	ASWHudMinimap
	{
		"fieldName"		"ASWHudMinimap"
		"xpos"	"r160"
		"ypos"	"r160"
		"wide"	"160"
		"tall"  "160"
		"visible" "1"
		"enabled" "1"

		"PaintBackgroundType"	"1"
		"BgAlpha"	"255"
		"NumberFont"	"DefaultLarge"
		"TextFont"	"Default"
		
		"text_xpos" "16"
		"text_ypos" "9"
		"digit_xpos" "31"
		"digit_ypos" "29"
	}
	
	ASWHudHealth
	{
		"fieldName"		"ASWHudHealth"
		"xpos"	"0"
		"ypos"	"0"
		"wide"	"2450"
		"tall"  "480"
		"visible" "1"
		"enabled" "1"

		"PaintBackgroundType"	"0"
		"BgAlpha"	"0"		
	}
	
	ASWHud3DMarineNames
	{
		"fieldName"		"ASWHud3DMarineNames"
		"xpos"	"0"
		"ypos"	"0"
		"wide"	"2450"	// a lot of hud elements are oversized to fill the screen in widescreen modes
		"tall"  "480"
		"visible" "1"
		"enabled" "1"

		"PaintBackgroundType"	"0"
		"BgAlpha"	"0"
		"NumberFont"	"DefaultLarge"
		"TextFont"	"Default"
		"SmallNumberFont"	"Default"
	}
	
	ASWHudEmotes
	{
		"fieldName"		"ASWHudEmotes"
		"xpos"	"0"
		"ypos"	"0"
		"wide"	"2450"
		"tall"  "480"
		"visible" "1"
		"enabled" "1"

		"PaintBackgroundType"	"0"
		"BgAlpha"	"0"
	}
	
	ASWHudUseArea
	{
		"fieldName"		"ASWHudUseArea"
		"xpos"	"0"
		"ypos"	"290"
		"wide"	"2450"
		"tall"  "160"
		//"xpos"      "0"
		//"ypos"      "0"
		//"wide"      "640"
		//"tall"      "480"
		"visible" "1"
		"enabled" "1"

		"PaintBackgroundType"	"0"
		"BgAlpha"	"0"
		"NumberFont"	"DefaultLarge"
		"TextFont"	"Default"
		"UseAreaFont"	"Default"
	}
	
	ASWHudOverlayMessages
	{
		"fieldName"		"ASWHudOverlayMessages"
		"xpos"	"0"
		"ypos"	"0"
		"wide"	"2450"
		"tall"  "480"
		"visible" "1"
		"enabled" "1"

		"PaintBackgroundType"	"0"
		"BgAlpha"	"0"
		"NumberFont"	"DefaultLarge"
		"TextFont"	"Default"
		"SmallNumberFont"	"Default"
		"OverlayFont"	"DefaultLarge"
	}
	
	ASWHudCrosshair
	{
		"fieldName" "ASWHudCrosshair"
		"visible" "1"
		"enabled" "1"
		"wide"	 "2450"
		"tall"	 "480"		
	}
	
	RadialMenu
	{
		"fieldName" "RadialMenu"
		"visible" "1"
		"enabled" "1"
		"wide"	 "2450"
		"tall"	 "480"		
	}
	
	ASWHudStylinCam
	{
		"fieldName" "ASWHudStylinCam"
		"visible" "1"
		"enabled" "1"
		"wide"	 "640"
		"tall"	 "480"		
	}
	
	ASWHudObjective
	{
		"fieldName" "ASWHudObjective"
		"visible" "1"
		"enabled" "1"
		"xpos"	"0"
		"wide"	 "540"
		"tall"	 "480"		
	}
	
	HudDamageIndicator
	{
		"fieldName" "HudDamageIndicator"
		"visible" "0"
		"enabled" "0"
		"DmgColorLeft" "255 0 0 0"
		"DmgColorRight" "255 0 0 0"
		
		"dmg_xpos" "30"
		"dmg_ypos" "100"
		"dmg_wide" "36"
		"dmg_tall1" "240"
		"dmg_tall2" "200"
	}

	HudMessage
	{
		"fieldName" "HudMessage"
		"visible" "1"
		"enabled" "1"
		"wide"	 "640"
		"tall"	 "480"
	}

	HudMenu
	{
		"fieldName" "HudMenu"
		"visible" "1"
		"enabled" "1"
		"wide"	 "640"
		"tall"	 "480"
	}

	HudCloseCaption
	{
		"fieldName" "HudCloseCaption"
		"visible"	"1"
		"enabled"	"1"
		"xpos"		"c-120"
		"ypos"		"0"
		"wide"		"240" //"500"
		"tall"		"100"
		"topoffset" "2"

		"BgAlpha"	"128"

		"GrowTime"		"0.25"
		"ItemHiddenTime"	"0.2"  // Nearly same as grow time so that the item doesn't start to show until growth is finished
		"ItemFadeInTime"	"0.15"	// Once ItemHiddenTime is finished, takes this much longer to fade in
		"ItemFadeOutTime"	"0.3"

	}

	HudPredictionDump
	{
		"fieldName" "HudPredictionDump"
		"visible" "1"
		"enabled" "1"
		"wide"	 "640"
		"tall"	 "480"
	}

	HudCommentary
	{
		"fieldName" "HudCommentary"
		"xpos"	"c-190"
		"ypos"	"350"
		"wide"	"380"
		"tall"  "40"
		"visible" "1"
		"enabled" "1"
		
		"PaintBackgroundType"	"2"
		
		"bar_xpos"		"50"
		"bar_ypos"		"20"
		"bar_height"	"8"
		"bar_width"		"320"
		"speaker_xpos"	"50"
		"speaker_ypos"	"8"
		"count_xpos_from_right"	"10"	// Counts from the right side
		"count_ypos"	"8"
		
		"icon_texture"	"vgui/hud/icon_commentary"
		"icon_xpos"		"0"
		"icon_ypos"		"0"		
		"icon_width"	"40"
		"icon_height"	"40"
	}
	
	HudChat
	{
		"fieldName" "HudChat"
		"visible" "1"
		"enabled" "1"
		"xpos"	"110"
		"ypos"	"345" // "280"   (asw changed)
		"wide"	 "400"
		"tall"	 "100"
	}
	
	HudHistoryResource	[$WIN32]
	{
		"fieldName" "HudHistoryResource"
		"visible" "0"
		"enabled" "1"
		"xpos"	"r252"
		"ypos"	"40"
		"wide"	 "248"
		"tall"	 "320"

		"history_gap"	"56"
		"icon_inset"	"38"
		"text_inset"	"36"
		"text_inset"	"26"
		"NumberFont"	"HudNumbersSmall"
	}
	HudHistoryResource	[$X360]
	{
		"fieldName" "HudHistoryResource"
		"visible" "0"
		"enabled" "1"
		"xpos"	"r300"
		"ypos"	"40" 
		"wide"	 "248"
		"tall"	 "240"

		"history_gap"	"50"
		"icon_inset"	"38"
		"text_inset"	"36"
		"NumberFont"	"HudNumbersSmall"
	}

	ASWHudVoiceStatus
	{
		"fieldName" "ASWHudVoiceStatus"
		"visible" "1"
		"enabled" "1"
		"xpos" "r180"
		"ypos" "6"
		"wide" "100"
		"tall" "400"

		"item_tall"	"24"
		"item_wide"	"100"

		"item_spacing" "2"

		"icon_ypos"	"0"
		"icon_xpos"	"0"
		"icon_tall"	"24"
		"icon_wide"	"24"

		"text_xpos"	"26"
	}

	ASWHudKills
	{
		"fieldName"		"ASWHudKills"
		"xpos"	"r150"
		"ypos"	"100"
		"wide"	"140"
		"tall"  "200"
		"visible" "1"
		"enabled" "1"
		"BgColor" "0 0 0 255"
		"PaintBackgroundType"	"2"
		"BgColor_override" "0 0 0 192"
	}
	
	ASWHudCounters
	{
		"fieldName"		"ASWHudCounters"
		"xpos"	"r100"
		"ypos"	"300"
		"wide"	"90"
		"tall"  "20"
		"visible" "1"
		"enabled" "1"
		"BgColor" "0 0 0 255"
		"PaintBackgroundType"	"2"
		"BgColor_override" "0 0 0 192"
	}
	
	ASW_Hud_Holdout
	{
		"fieldName" "ASW_Hud_Holdout"
		"visible" "1"
		"enabled" "1"
		"xpos"	"15"
		"ypos"	"35"
		"wide"	"f0"
		"tall"	"480"
	}

	AchievementNotificationPanel
	{
	}

	RD_Hud_Boss_Bars
	{
		"fieldName" "RD_Hud_Boss_Bars"
		"visible" "1"
		"enabled" "1"
	}

	ASW_Hud_Master
	{
		"fieldName" "ASW_Hud_Master"
		"visible" "1"
		"enabled" "1"
		
		"xpos" "0"
		"ypos" "0"
		"wide" "f0"
		"tall" "480"
		
		"PrimaryWeapon_x" "18"
		"PrimaryWeapon_y" "r160"
		"SecondaryWeapon_x" "18"
		"SecondaryWeapon_y" "r130"
		"TertiaryWeapon_x" "18"
		"TertiaryWeapon_y" "r190"
		
		"Weapon_w" "80"
		"Weapon_t" "40"
		
		"MarinePortrait_x" "5"
		"MarinePortrait_y" "r80"
		
		// these coords are relative to the top left of the marine portrait
		"MarinePortrait_circle_bg_x" "0"
		"MarinePortrait_circle_bg_y" "0"
		"MarinePortrait_circle_bg_w" "75"
		"MarinePortrait_circle_bg_t" "75"
		"MarinePortrait_bar_bg_w" "75"
		"MarinePortrait_face_x" "38"
		"MarinePortrait_face_y" "38"
		"MarinePortrait_face_radius" "27"
		"MarinePortrait_class_icon_x" "39"
		"MarinePortrait_class_icon_y" "50"
		"MarinePortrait_class_icon_w" "22"
		"MarinePortrait_class_icon_t" "22"
		"MarinePortrait_weapon_name_x" "65"
		"MarinePortrait_weapon_name_y" "39"
		"MarinePortrait_low_ammo_x"	"81"
		"MarinePortrait_low_ammo_y"	"27"
		"MarinePortrait_bullets_icon_x" "63"
		"MarinePortrait_bullets_icon_y" "48"
		"MarinePortrait_bullets_icon_w" "11"
		"MarinePortrait_bullets_icon_t" "11"
		"MarinePortrait_bullets_x" "75"
		"MarinePortrait_bullets_y" "45"
		
		"MarinePortrait_grenades_icon_x" "98"
		"MarinePortrait_grenades_icon_y" "48"
		"MarinePortrait_grenades_icon_w" "11"
		"MarinePortrait_grenades_icon_t" "11"
		"MarinePortrait_grenades_x" "109"
		"MarinePortrait_grenades_y" "45"
		
		"MarinePortrait_clips_x" "60"
		"MarinePortrait_clips_y" "60"
		"MarinePortrait_clips_w" "12"
		"MarinePortrait_clips_t" "12"
		"MarinePortrait_clips_spacing" "8"
		
		"MarinePortrait_health_counter_color" "66 142 192 255"
		"MarinePortrait_health_counter_x" "80"
		"MarinePortrait_health_counter_y" "11"
		"MarinePortrait_health_counter_icon_x" "70"
		"MarinePortrait_health_counter_icon_y" "14"
		"MarinePortrait_health_counter_icon_w" "10"
		"MarinePortrait_health_counter_icon_t" "10"
		
		"ExtraItem_x"	"115"		// relative to local portrait too
		"ExtraItem_y"	"39"
		"ExtraItem_w"	"35"
		"ExtraItem_t"	"35"
		"ExtraItem_hotkey_x"	"149"		// top right of text
		"ExtraItem_hotkey_y"	"40"
		"ExtraItem_quantity_x"	"148"			// lower right of text
		"ExtraItem_quantity_y"	"74"
		"ExtraItem_battery_x"	"3"		// relative to extra item top left
		"ExtraItem_battery_y"	"28"
		"ExtraItem_battery_w"	"29"
		"ExtraItem_battery_t"	"5"
		
		
		
		"FastReload_x" "99"
		"FastReload_y" "r69"
		"FastReload_w" "84"
		"FastReload_t" "11"
		
		"SquadMates_x"	"151"		// spacing to the right of the currently active marine
		"SquadMates_y"	"r41"
		"SquadMates_spacing"	"86"
		
		// these coords are relative to the top left of the squadmate's panel
		"SquadMate_name_color" "255 255 255 255"
		"SquadMate_name_dead_color" "128 128 128 255"
		"SquadMate_name_x"	"0"
		"SquadMate_name_y"	"0"
		"SquadMate_bg_x"	"0"
		"SquadMate_bg_y"	"9"
		"SquadMate_bg_w"	"85" // Measured size:"78"
		"SquadMate_bg_t"	"27"	
		"SquadMate_class_icon_x" "2"
		"SquadMate_class_icon_y" "11"
		"SquadMate_class_icon_w" "23"
		"SquadMate_class_icon_t" "23"
		"SquadMate_health_x" "19"
		"SquadMate_health_y" "11"
		"SquadMate_health_w" "40"
		"SquadMate_health_t" "8"
		"SquadMate_bullets_x" "26"
		"SquadMate_bullets_y" "21"
		"SquadMate_bullets_w" "33"
		"SquadMate_bullets_t" "6"
		"SquadMate_bullets_bg_color" "96 96 96 255"
		"SquadMate_bullets_fg_color" "200 200 200 255"
		"SquadMate_clips_x" "23"
		"SquadMate_clips_y" "28"
		"SquadMate_clips_w" "7"
		"SquadMate_clips_t" "7"
		"SquadMate_clips_spacing" "5"
		"SquadMate_clips_full_color" "200 200 200 255"
		"SquadMate_clips_empty_color" "96 96 96 255"
		"SquadMate_ExtraItem_x" "60"
		"SquadMate_ExtraItem_y" "10"
		"SquadMate_ExtraItem_w" "25"
		"SquadMate_ExtraItem_t" "25"
		"SquadMate_ExtraItem_hotkey_color" "255 255 255 255"
		"SquadMate_ExtraItem_hotkey_x" "83"		// top right of text
		"SquadMate_ExtraItem_hotkey_y" "10"
		"SquadMate_ExtraItem_quantity_color" "66 142 192 255"
		"SquadMate_ExtraItem_quantity_x" "83"	// lower right of text
		"SquadMate_ExtraItem_quantity_y" "36"
		
		"SquadMate_health_counter_color" "255 255 255 255"
		"SquadMate_health_counter_x" "26"
		"SquadMate_health_counter_y" "11"
	}
}
