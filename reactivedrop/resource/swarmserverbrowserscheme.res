///////////////////////////////////////////////////////////
// Object Control Panel scheme resource file
//
// sections:
//		Colors			- all the colors used by the scheme
//		BaseSettings	- contains settings for app to use to draw controls
//		Fonts			- list of all the fonts used by app
//		Borders			- description of all the borders
//
// hit ctrl-alt-shift-R in the app to reload this file
//
///////////////////////////////////////////////////////////
Scheme
{
	//////////////////////// COLORS ///////////////////////////
	// color details
	// this is a list of all the colors used by the scheme
	Colors
	{
		// base colors
		"White"				"255 255 255 255"
		"OffWhite"			"221 221 221 255"
		"DullWhite"			"211 211 211 255"
		"Red"				"255 0 0 255"
		//"LightBlue"			"68 140 203 255"
		"LightBlue"			"66 142 192 255"
		"GreyBlue"			"65 74 96 255"
		"DarkBlueTrans"			"65 74 96 64"
		
		"TransparentBlack"	"0 0 0 128"
		"DarkGrey"			"128 128 128 255"
		"Black"				"0 0 0 255"

		"Blank"				"0 0 0 0"
	}

	///////////////////// BASE SETTINGS ////////////////////////
	//
	// default settings for all panels
	// controls use these to determine their settings
	BaseSettings
	{
		// vgui_controls color specifications
		Border.Bright					"200 200 200 196"	// the lit side of a control
		Border.Dark						"40 40 40 196"		// the dark/unlit side of a control
		Border.Selection				"0 0 0 196"			// the additional border color for displaying the default/selected button

		Button.TextColor				"White"
		Button.BgColor					"65 74 96 255"
		Button.ArmedTextColor			"White"
		Button.ArmedBgColor				"65 74 96 255"
		Button.DepressedTextColor		"White"
		Button.DepressedBgColor			"65 74 96 255"
		Button.FocusBorderColor			"65 74 96 255"		
		
		CheckButton.TextColor			"LightBlue"
		CheckButton.SelectedTextColor	"White"
		CheckButton.BgColor				"TransparentBlack"
		CheckButton.Border1  			"Border.Dark" 		// the left checkbutton border
		CheckButton.Border2  			"Border.Bright"		// the right checkbutton border
		CheckButton.Check				"White"				// color of the check itself

		ComboBoxButton.ArrowColor		"DullWhite"
		ComboBoxButton.ArmedArrowColor	"White"
		ComboBoxButton.BgColor			"65 74 96 255"
		ComboBoxButton.DisabledBgColor	"65 74 96 255"

		Frame.TitleTextInsetX			12
		Frame.ClientInsetX				6
		Frame.ClientInsetY				4
		//Frame.BgColor					"160 160 160 128"
		Frame.BgColor					"65 74 96 255"
		Frame.OutOfFocusBgColor			"65 74 96 255"
		//Frame.OutOfFocusBgColor			"160 160 160 32"
		Frame.FocusTransitionEffectTime	"0.3"	// time it takes for a window to fade in/out on focus/out of focus
		Frame.TransitionEffectTime		"0.3"	// time it takes for a window to fade in/out on open/close
		Frame.AutoSnapRange				"0"
		FrameGrip.Color1				"200 200 200 196"
		FrameGrip.Color2				"0 0 0 196"
		FrameTitleButton.FgColor		"255 255 255 255"
		FrameTitleButton.BgColor		"65 74 96 255"
		FrameTitleButton.DisabledFgColor	"255 255 255 192"
		FrameTitleButton.DisabledBgColor	"65 74 96 255"
		FrameSystemButton.FgColor		"65 74 96 255"
		FrameSystemButton.BgColor		"65 74 96 255"
		FrameSystemButton.Icon			""
		FrameSystemButton.DisabledIcon	""
		FrameTitleBar.TextColor			"White"
		FrameTitleBar.BgColor			"65 74 96 255"
		FrameTitleBar.DisabledTextColor	"255 255 255 192"
		FrameTitleBar.DisabledBgColor	"65 74 96 255"

		GraphPanel.FgColor				"White"
		GraphPanel.BgColor				"TransparentBlack"

		Label.TextDullColor				"DullWhite"
		Label.TextColor					"OffWhite"
		Label.TextBrightColor			"LightBlue"
		Label.SelectedTextColor			"White"
		Label.BgColor					"65 74 96 255"
		Label.DisabledFgColor1			"117 117 117 255"
		Label.DisabledFgColor2			"30 30 30 255"

		ListPanel.TextColor					"OffWhite"
		ListPanel.BgColor					"TransparentBlack"
		ListPanel.SelectedTextColor			"Black"
		ListPanel.SelectedBgColor			"LightBlue"
		ListPanel.SelectedOutOfFocusBgColor	"LightBlue"
		ListPanel.EmptyListInfoTextColor	"LightBlue"
		
		ImagePanel.fillcolor			"65 74 96 255"

		Menu.TextColor					"White"
		Menu.BgColor					"TransparentBlack"
		Menu.ArmedTextColor				"Black"
		Menu.ArmedBgColor				"LightBlue"
		Menu.TextInset					"6"

		Panel.FgColor					"DullWhite"
		Panel.BgColor					"65 74 96 255"

		ProgressBar.FgColor				"White"
		ProgressBar.BgColor				"TransparentBlack"

		PropertySheet.TextColor			"LightBlue"
		PropertySheet.SelectedTextColor	"White"
		PropertySheet.TransitionEffectTime	"0.3"	// time to change from one tab to another
		PropertySheet.TabFont	"DefaultLarge"

		RadioButton.TextColor			"DullWhite"
		RadioButton.SelectedTextColor	"White"

		RichText.TextColor				"OffWhite"
		RichText.BgColor				"TransparentBlack"
		RichText.SelectedTextColor		"Black"
		RichText.SelectedBgColor		"LightBlue"

		ScrollBar.Wide					17

		ScrollBarButton.FgColor				"LightBlue"
		ScrollBarButton.BgColor				"Blank"
		ScrollBarButton.ArmedFgColor		"White"
		ScrollBarButton.ArmedBgColor		"Blank"
		ScrollBarButton.DepressedFgColor	"White"
		ScrollBarButton.DepressedBgColor	"Blank"

		ScrollBarSlider.FgColor				"GreyBlue"			// nob color
		ScrollBarSlider.BgColor				"Black"	// slider background color
		ScrollBarSlider.NobFocusColor		"LightBlue"
		ScrollBarSlider.NobDragColor		"White"

		SectionedListPanel.HeaderTextColor	"White"
		SectionedListPanel.HeaderBgColor	"65 74 96 255"
		SectionedListPanel.DividerColor		"Black"
		SectionedListPanel.TextColor		"LightBlue"
		SectionedListPanel.BrightTextColor	"White"
		SectionedListPanel.BgColor			"TransparentBlack"
		SectionedListPanel.SelectedTextColor			"Black"
		SectionedListPanel.SelectedBgColor				"LightBlue"
		SectionedListPanel.OutOfFocusSelectedTextColor	"Black"
		SectionedListPanel.OutOfFocusSelectedBgColor	"LightBlue"

		Slider.NobColor				"108 108 108 255"
		Slider.TextColor			"127 140 127 255"
		Slider.TrackColor			"31 31 31 255"
		Slider.DisabledTextColor1	"117 117 117 255"
		Slider.DisabledTextColor2	"30 30 30 255"

		TextEntry.TextColor			"OffWhite"
		TextEntry.BgColor			"TransparentBlack"
		TextEntry.CursorColor		"OffWhite"
		TextEntry.DisabledTextColor	"DullWhite"
		TextEntry.DisabledBgColor	"65 74 96 255"
		TextEntry.SelectedTextColor	"Black"
		TextEntry.SelectedBgColor	"LightBlue"
		TextEntry.OutOfFocusSelectedBgColor	"LightBlue"
		TextEntry.FocusEdgeColor	"0 0 0 196"

		ToggleButton.SelectedTextColor	"White"

		Tooltip.TextColor			"0 0 0 196"
		Tooltip.BgColor				"LightBlue"

		TreeView.BgColor			"TransparentBlack"

		WizardSubPanel.BgColor		"65 74 96 255"

		// scheme-specific colors
		MainMenu.TextColor			"White"
		MainMenu.ArmedTextColor		"200 200 200 255"
		MainMenu.DepressedTextColor	"192 186 80 255"
		MainMenu.MenuItemHeight		"30"
		MainMenu.Inset				"32"

		Console.TextColor			"OffWhite"
		Console.DevTextColor		"White"

		NewGame.TextColor			"White"
		NewGame.FillColor			"0 0 0 255"
		NewGame.SelectionColor		"LightBlue"
		NewGame.DisabledColor		"128 128 128 196"
	}

	//
	//////////////////////// FONTS /////////////////////////////
	//
	// describes all the fonts
	// font options: italic, underline, strikeout, antialias, dropshadow, outline, tall, blur, scanlines
	//   ?: custom, bitmap, rotary, additive
	
	Fonts
	{
		// fonts are used in order that they are listed
		// fonts listed later in the order will only be used if they fulfill a range not already filled
		// if a font fails to load then the subsequent fonts will replace
		
		// default font used for regular text throughout
		"Default"
		{
			"1"
			{
				"name"		"Tahoma"
				"tall"		"16"
				"weight"	"500"
			}
			"2"
			{
				"name"		"Manri"
				"tall"		"16"
				"weight"	"500"
				"range"		"0xED80 0xEDBF" //	Zbalermorna
			}
		}
		"DefaultBold"
		{
			"1"
			{
				"name"		"Tahoma"
				"tall"		"16"
				"weight"	"1000"
			}
			"2"
			{
				"name"		"Manri"
				"tall"		"16"
				"weight"	"1000"
				"range"		"0xED80 0xEDBF" //	Zbalermorna
			}
		}
		"DefaultUnderline"
		{
			"1"
			{
				"name"		"Tahoma"
				"tall"		"16"
				"weight"	"500"
				"underline" "1"
			}
			"2"
			{
				"name"		"Manri"
				"tall"		"16"
				"weight"	"500"
				"range"		"0xED80 0xEDBF" //	Zbalermorna
				"underline"	"1"
			}
		}
		"DefaultSmall"
		{
			"1"
			{
				"name"		"Tahoma"
				"tall"		"13"
				"weight"	"0"
			}
			"2"
			{
				"name"		"Manri"
				"tall"		"13"
				"weight"	"0"
				"range"		"0xED80 0xEDBF" //	Zbalermorna
			}
		}
		"DefaultSmallDropShadow"
		{
			"1"
			{
				"name"		"Tahoma"
				"tall"		"13"
				"weight"	"0"
				"dropshadow" "1"
			}
			"2"
			{
				"name"		"Manri"
				"tall"		"13"
				"weight"	"0"
				"range"		"0xED80 0xEDBF" //	Zbalermorna
				"dropshadow" "1"
			}
		}
		"DefaultVerySmall"
		{
			"1"
			{
				"name"		"Tahoma"
				"tall"		"12"
				"weight"	"0"
			}
			"2"
			{
				"name"		"Manri"
				"tall"		"12"
				"weight"	"0"
				"range"		"0xED80 0xEDBF" //	Zbalermorna
			}
		}

		"DefaultLarge"
		{
			"1"
			{
				"name"		"Tahoma"
				"tall"		"18"
				"weight"	"0"
			}
			"2"
			{
				"name"		"Manri"
				"tall"		"18"
				"weight"	"0"
				"range"		"0xED80 0xEDBF" //	Zbalermorna
			}
		}
		// DefaultExtraLarge
		"DefaultExtraLarge"
		{
			"1"
			{
				"name"		"Neo Sans"
				"tall"		"36"
				"weight"	"900"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"antialias"	"1"
			}
			"2"
			{
				"name"		"Manri"
				"tall"		"36"
				"weight"	"900"
				"range"		"0xED80 0xEDBF" //	Zbalermorna
				"antialias"	"1"
			}
		}

		// this is the symbol font
		"Marlett"
		{
			"1"
			{
				"name"		"Marlett"
				"tall"		"14"
				"weight"	"0"
				"symbol"	"1"
				"range"		"0x0000 0x007F"	//	Basic Latin
				"antialias"	"1"
			}
		}
		
		// specific fonts for the main resolutions
		// Resolution	:	Header	:	Regular	:	Small
		// -640			14		10		8
		// 800			16		12		10
		// 1024			18		14		12
		// 1280+		20		16		14		
		"Sansman8"
		{
			"1"
			{
				"name"		"Neo Sans"	
				"tall"		"8"
				"weight"	"900"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"antialias"	"1"
			}
			"2"
			{
				"name"		"Manri"
				"tall"		"8"
				"weight"	"900"
				"range"		"0xED80 0xEDBF" //	Zbalermorna
				"antialias"	"1"
			}
		}
		"Sansman10"
		{
			"1"
			{
				"name"		"Neo Sans"
				"tall"		"10"
				"weight"	"900"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"antialias"	"1"
			}
			"2"
			{
				"name"		"Manri"
				"tall"		"10"
				"weight"	"900"
				"range"		"0xED80 0xEDBF" //	Zbalermorna
				"antialias"	"1"
			}
		}
		"Sansman12"
		{
			"1"
			{
				"name"		"Neo Sans"	
				"tall"		"12"
				"weight"	"900"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"antialias"	"1"
			}
			"2"
			{
				"name"		"Manri"
				"tall"		"12"
				"weight"	"900"
				"range"		"0xED80 0xEDBF" //	Zbalermorna
				"antialias"	"1"
			}
		}
		"Sansman14"
		{
			"1"
			{
				"name"		"Neo Sans"	
				"tall"		"14"
				"weight"	"900"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"antialias"	"1"
			}
			"2"
			{
				"name"		"Manri"
				"tall"		"14"
				"weight"	"900"
				"range"		"0xED80 0xEDBF" //	Zbalermorna
				"antialias"	"1"
			}
		}	
		"Sansman16"
		{
			"1"
			{
				"name"		"Neo Sans"	
				"tall"		"16"
				"weight"	"900"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"antialias"	"1"
			}
			"2"
			{
				"name"		"Manri"
				"tall"		"16"
				"weight"	"900"
				"range"		"0xED80 0xEDBF" //	Zbalermorna
				"antialias"	"1"
			}
		}	
		"Sansman18"
		{
			"1"
			{
				"name"		"Neo Sans"	
				"tall"		"18"
				"weight"	"900"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"antialias"	"1"
			}
			"2"
			{
				"name"		"Manri"
				"tall"		"18"
				"weight"	"900"
				"range"		"0xED80 0xEDBF" //	Zbalermorna
				"antialias"	"1"
			}
		}
		"Sansman20"
		{
			"1"
			{
				"name"		"Neo Sans"	
				"tall"		"20"
				"weight"	"900"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"antialias"	"1"
			}
			"2"
			{
				"name"		"Manri"
				"tall"		"20"
				"weight"	"900"
				"range"		"0xED80 0xEDBF" //	Zbalermorna
				"antialias"	"1"
			}
		}
		"CleanHUD"
		{
			"1"
			{
				"name"		"Microsoft Sans Serif"
				"tall"		"17"
				"weight"	"900"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"antialias"	"1"
			}
			"2"
			{
				"name"		"Manri"
				"tall"		"17"
				"weight"	"900"
				"range"		"0xED80 0xEDBF" //	Zbalermorna
				"antialias"	"1"
			}
		}
		"Courier"
		{
			"1"
			{
				"name"		"Courier New"
				"tall"		"16"
				"weight"	"900"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"antialias"	"1"
			}
			"2"
			{
				"name"		"Manri"
				"tall"		"16"
				"weight"	"900"
				"range"		"0xED80 0xEDBF" //	Zbalermorna
				"antialias"	"1"
			}
		}
		"SmallCourier"
		{
			"1"
			{
				"name"		"Courier New"
				"tall"		"8"
				"weight"	"900"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"antialias"	"1"
			}
			"2"
			{
				"name"		"Manri"
				"tall"		"8"
				"weight"	"900"
				"range"		"0xED80 0xEDBF" //	Zbalermorna
				"antialias"	"1"
			}
		}
		"Verdana"
		{
			"1"
			{
				"name"		"Veranda" //"Verdana"
				"tall"		"16"
				"weight"	"450"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"antialias"	"1"
			}
			"2"
			{
				"name"		"Manri"
				"tall"		"16"
				"weight"	"450"
				"range"		"0xED80 0xEDBF" //	Zbalermorna
				"antialias"	"1"
			}
		}
		"VerdanaSmall"
		{
			"1"
			{
				"name"		"Veranda" //"Verdana"
				"tall"		"9"
				"weight"	"900"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"antialias"	"1"
			}
			"2"
			{
				"name"		"Manri"
				"tall"		"9"
				"weight"	"900"
				"range"		"0xED80 0xEDBF" //	Zbalermorna
				"antialias"	"1"
			}
		}
		"CampaignTitle"
		{
			"1"
			{
				"name"		"Neo Sans"
				"tall"		"64"
				"weight"	"450"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"antialias"	"1"
			}
			"2"
			{
				"name"		"Manri"
				"tall"		"64"
				"weight"	"450"
				"range"		"0xED80 0xEDBF" //	Zbalermorna
				"antialias"	"1"
			}
		}
		"CampaignTitleBlur"
		{
			"1"
			{
				"name"		"Neo Sans"
				"tall"		"64"
				"weight"	"450"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"antialias"	"1"
				"blur"		"5"
			}
			"2"
			{
				"name"		"Manri"
				"tall"		"64"
				"weight"	"450"
				"range"		"0xED80 0xEDBF" //	Zbalermorna
				"antialias"	"1"
				"blur"		"5"
			}
		}
		// asw - infested small font for mission chooser
		"MissionChooserFont"
		{
			"1"
			{
				"name"		"Tahoma"
				"tall"		"11"
				"weight"	"0"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"antialias"	"1"
				"yres"	"1 599"
			}
			"2"
			{
				"name"		"Tahoma"
				"tall"		"12"
				"weight"	"0"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"yres"	"600 767"
			}
			"3"
			{
				"name"		"Tahoma"
				"tall"		"16"
				"weight"	"0"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"yres"	"768 1023"
			}
			"4"
			{
				"name"		"Tahoma"
				"tall"		"16"
				"weight"	"0"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"yres"	"1024 1199"
			}
			"5"
			{
				"name"		"Tahoma"
				"tall"		"16"
				"weight"	"0"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"yres"	"1200 10000"
			}
		}
	}

	//
	//////////////////// BORDERS //////////////////////////////
	//
	// describes all the border types
	Borders
	{
	    BaseBorder		DepressedBorder
		ButtonBorder	RaisedBorder
		ComboBoxBorder	DepressedBorder
		MenuBorder		RaisedBorder
		BrowserBorder	DepressedBorder
		PropertySheetBorder	NoBorder
		
		FrameBorder
		{
			// rounded corners for frames
			"backgroundtype" "2"
		}
		
		NoBorder
		{
			"backgroundtype" "0"
		}

		DepressedBorder
		{
			"inset" "0 0 1 1"
			Left
			{
				"1"
				{
					"color" "Border.Dark"
					"offset" "0 1"
				}
			}

			Right
			{
				"1"
				{
					"color" "Border.Bright"
					"offset" "1 0"
				}
			}

			Top
			{
				"1"
				{
					"color" "Border.Dark"
					"offset" "0 0"
				}
			}

			Bottom
			{
				"1"
				{
					"color" "Border.Bright"
					"offset" "0 0"
				}
			}
		}
		RaisedBorder
		{
			"inset" "0 0 1 1"
			Left
			{
				"1"
				{
					"color" "Border.Bright"
					"offset" "0 1"
				}
			}

			Right
			{
				"1"
				{
					"color" "Border.Dark"
					"offset" "0 0"
				}
			}

			Top
			{
				"1"
				{
					"color" "Border.Bright"
					"offset" "0 1"
				}
			}

			Bottom
			{
				"1"
				{
					"color" "Border.Dark"
					"offset" "0 0"
				}
			}
		}
		
		BaseBorder
		{
			"inset" "0 0 1 1"
			Left
			{
				"1"
				{
					"color" "BorderDark"
					"offset" "0 1"
				}
			}

			Right
			{
				"1"
				{
					"color" "BorderBright"
					"offset" "1 0"
				}
			}

			Top
			{
				"1"
				{
					"color" "BorderDark"
					"offset" "0 0"
				}
			}

			Bottom
			{
				"1"
				{
					"color" "BorderBright"
					"offset" "0 0"
				}
			}
		}
		
		TitleButtonBorder
		{
			"backgroundtype" "0"
		}

		TitleButtonDisabledBorder
		{
			"backgroundtype" "0"
		}

		TitleButtonDepressedBorder
		{
			"backgroundtype" "0"
		}

		ScrollBarButtonBorder
		{
			"inset" "2 2 0 0"
			Left
			{
				"1"
				{
					"color" "BorderBright"
					"offset" "0 1"
				}
			}

			Right
			{
				"1"
				{
					"color" "BorderDark"
					"offset" "1 0"
				}
			}

			Top
			{
				"1"
				{
					"color" "BorderBright"
					"offset" "0 0"
				}
			}

			Bottom
			{
				"1"
				{
					"color" "BorderDark"
					"offset" "0 0"
				}
			}
		}		

		ScrollBarButtonDepressedBorder
		{
			"inset" "2 2 0 0"
			Left
			{
				"1"
				{
					"color" "Border.Dark"
					"offset" "0 1"
				}
			}

			Right
			{
				"1"
				{
					"color" "Border.Bright"
					"offset" "1 0"
				}
			}

			Top
			{
				"1"
				{
					"color" "Border.Dark"
					"offset" "0 0"
				}
			}

			Bottom
			{
				"1"
				{
					"color" "Border.Bright"
					"offset" "0 0"
				}
			}
		}

		TabBorder
		{
			"inset" "0 0 1 1"
			Left
			{
				"1"
				{
					"color" "Border.Bright"
					"offset" "0 1"
				}
			}

			Right
			{
				"1"
				{
					"color" "Border.Dark"
					"offset" "1 0"
				}
			}

			Top
			{
				"1"
				{
					"color" "Border.Bright"
					"offset" "0 0"
				}
			}

		}

		TabActiveBorder
		{
			"inset" "0 0 1 0"
			Left
			{
				"1"
				{
					"color" "Border.Bright"
					"offset" "0 0"
				}
			}

			Right
			{
				"1"
				{
					"color" "Border.Dark"
					"offset" "1 0"
				}
			}

			Top
			{
				"1"
				{
					"color" "Border.Bright"
					"offset" "0 0"
				}
			}

		}


		ToolTipBorder
		{
			"inset" "0 0 1 0"
			Left
			{
				"1"
				{
					"color" "Border.Dark"
					"offset" "0 0"
				}
			}

			Right
			{
				"1"
				{
					"color" "Border.Dark"
					"offset" "1 0"
				}
			}

			Top
			{
				"1"
				{
					"color" "Border.Dark"
					"offset" "0 0"
				}
			}

			Bottom
			{
				"1"
				{
					"color" "Border.Dark"
					"offset" "0 0"
				}
			}
		}

		// this is the border used for default buttons (the button that gets pressed when you hit enter)
		ButtonKeyFocusBorder
		{
			"inset" "0 0 1 1"
			Left
			{
				"1"
				{
					"color" "Border.Selection"
					"offset" "0 0"
				}
				"2"
				{
					"color" "Border.Bright"
					"offset" "0 1"
				}
			}
			Top
			{
				"1"
				{
					"color" "Border.Selection"
					"offset" "0 0"
				}
				"2"
				{
					"color" "Border.Bright"
					"offset" "1 0"
				}
			}
			Right
			{
				"1"
				{
					"color" "Border.Selection"
					"offset" "0 0"
				}
				"2"
				{
					"color" "Border.Dark"
					"offset" "1 0"
				}
			}
			Bottom
			{
				"1"
				{
					"color" "Border.Selection"
					"offset" "0 0"
				}
				"2"
				{
					"color" "Border.Dark"
					"offset" "0 0"
				}
			}
		}

		ButtonDepressedBorder
		{
			"inset" "2 1 1 1"
			Left
			{
				"1"
				{
					"color" "Border.Dark"
					"offset" "0 1"
				}
			}

			Right
			{
				"1"
				{
					"color" "Border.Bright"
					"offset" "1 0"
				}
			}

			Top
			{
				"1"
				{
					"color" "Border.Dark"
					"offset" "0 0"
				}
			}

			Bottom
			{
				"1"
				{
					"color" "Border.Bright"
					"offset" "0 0"
				}
			}
		}
		ASWBriefingButtonBorder
		{
			"inset" "1 1 1 1"
			Left
			{
				"1"
				{
					"color" "LightBlue"
					"offset" "0 0"
				}
			}

			Right
			{
				"1"
				{
					"color" "LightBlue"
					"offset" "0 0"
				}
			}

			Top
			{
				"1"
				{
					"color" "LightBlue"
					"offset" "0 0"
				}
			}

			Bottom
			{
				"1"
				{
					"color" "LightBlue"
					"offset" "0 0"
				}
			}
		}
		ASWBriefingButtonBorderDisabled
		{
			"inset" "1 1 1 1"
			Left
			{
				"1"
				{
					"color" "GreyBlue"
					"offset" "0 0"
				}
			}

			Right
			{
				"1"
				{
					"color" "GreyBlue"
					"offset" "0 0"
				}
			}

			Top
			{
				"1"
				{
					"color" "GreyBlue"
					"offset" "0 0"
				}
			}

			Bottom
			{
				"1"
				{
					"color" "GreyBlue"
					"offset" "0 0"
				}
			}
		}
		ASWMapLabelBorder
		{
			"inset" "1 1 1 1"
			Left
			{
				"1"
				{
					"color" "White"
					"offset" "0 0"
				}
			}

			Right
			{
				"1"
				{
					"color" "White"
					"offset" "0 0"
				}
			}

			Top
			{
				"1"
				{
					"color" "White"
					"offset" "0 0"
				}
			}

			Bottom
			{
				"1"
				{
					"color" "White"
					"offset" "0 0"
				}
			}
		}
	}
	CustomFontFiles
	{
		"1"		"resource/neosans.vfont"
		"2"		"resource/veranda.vfont"
		"3"		"resource/manri.vfont"
	}
}