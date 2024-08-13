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
		"Gray"				"64 64 64 255"
		"MediumGray"		"145 145 145 255"
		"DarkGrey"			"128 128 128 255"
		"AshGray"			"16 16 16 255"
		"AshGrayHighAlpha"	"16 16 16 192"
		"DarkGrayLowAlpha"	"32 32 32 64"
		"DarkRed"			"65 0 0 255"
		"DeepRed"			"168 26 26 255"
		"Orange"			"255 155 0 255"
		"Red"				"255 0 0 255"
		//"LightBlue"			"68 140 203 255"
		"LightBlue"			"66 142 192 255"
		"GreyBlue"			"65 74 96 255"
		"DarkBlueTrans"			"65 74 96 64"
		"Blue"				"83 148 192 255"
		"HighlightBlue"		"169 213 255 255"
		"ObjectiveBackground"	"30 66 89 164"

		"TransparentBlack"	"0 0 0 128"
		"Black"				"0 0 0 255"

		"Blank"				"0 0 0 0"
		"Green"				"0 128 0 255"
		"LightBrown"		"120 69 24 255"
		"DarkBrown"			"57 49 38 255"
		
		"ScrollBarGrey"		"51 51 51 255"
		"ScrollBarHilight"	"110 110 110 255"
		"ScrollBarDark"		"38 38 38 255"
		
		"BrightYellow"		"242 237 0 255"
		"DarkYellow"		"136 133 0 255"
		"TextYellow"		"110 110 84 255"
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
		Border.DarkRed					"DarkRed"
		Border.DeepRed					"DeepRed"
		Border.LightBrown				"LightBrown"
		Border.DarkBrown				"DarkBrown"
		Border.White					"White"

		Button.TextColor				"LightBlue"
		Button.BgColor					"GreyBlue"
		Button.ArmedTextColor			"White"
		Button.ArmedBgColor				"Blank"
		Button.DepressedTextColor		"White"
		Button.DepressedBgColor			"Blank"
		Button.FocusBorderColor			"Black"
		
		CheckButton.TextColor			"LightBlue"
		CheckButton.SelectedTextColor	"White"
		CheckButton.BgColor				"TransparentBlack"
		CheckButton.Border1				"Border.Dark"		// the left checkbutton border
		CheckButton.Border2				"Border.Bright"		// the right checkbutton border
		CheckButton.Check				"White"				// color of the check itself

		ComboBoxButton.ArrowColor		"DullWhite"
		ComboBoxButton.ArmedArrowColor	"White"
		ComboBoxButton.BgColor			"Blank"
		ComboBoxButton.DisabledBgColor	"Blank"

		Frame.TitleTextInsetX			12
		Frame.ClientInsetX				6
		Frame.ClientInsetY				4
		//Frame.BgColor					"160 160 160 128"
		Frame.BgColor					"0 0 0 255"
		Frame.OutOfFocusBgColor			"0 0 0 255"
		//Frame.OutOfFocusBgColor			"160 160 160 32"
		Frame.FocusTransitionEffectTime	"0.3"	// time it takes for a window to fade in/out on focus/out of focus
		Frame.TransitionEffectTime		"0.3"	// time it takes for a window to fade in/out on open/close
		Frame.AutoSnapRange				"0"
		FrameGrip.Color1				"200 200 200 196"
		FrameGrip.Color2				"0 0 0 196"
		FrameTitleButton.FgColor		"200 200 200 196"
		FrameTitleButton.BgColor		"Blank"
		FrameTitleButton.DisabledFgColor	"255 255 255 192"
		FrameTitleButton.DisabledBgColor	"Blank"
		FrameSystemButton.FgColor		"Blank"
		FrameSystemButton.BgColor		"Blank"
		FrameSystemButton.Icon			""
		FrameSystemButton.DisabledIcon	""
		FrameTitleBar.TextColor			"White"
		FrameTitleBar.BgColor			"Blank"
		FrameTitleBar.DisabledTextColor	"255 255 255 192"
		FrameTitleBar.DisabledBgColor	"Blank"

		GraphPanel.FgColor				"White"
		GraphPanel.BgColor				"TransparentBlack"

		Label.TextDullColor				"Black"
		Label.TextColor					"Blue"
		Label.TextBrightColor			"LightBlue"
		Label.SelectedTextColor			"White"
		Label.BgColor					"Blank"
		Label.DisabledFgColor1			"117 117 117 255"
		Label.DisabledFgColor2			"30 30 30 255"

		ListPanel.TextColor					"OffWhite"
		ListPanel.BgColor					"TransparentBlack"
		ListPanel.SelectedTextColor			"Black"
		ListPanel.SelectedBgColor			"LightBlue"
		ListPanel.SelectedOutOfFocusBgColor	"LightBlue"
		ListPanel.EmptyListInfoTextColor	"LightBlue"
		
		ImagePanel.fillcolor			"Blank"

		Menu.TextColor					"White"
		Menu.BgColor					"160 160 160 64"
		Menu.ArmedTextColor				"Black"
		Menu.ArmedBgColor				"LightBlue"
		Menu.TextInset					"6"

		Panel.FgColor					"DullWhite"
		Panel.BgColor					"Blank"

		ProgressBar.FgColor				"White"
		ProgressBar.BgColor				"TransparentBlack"

		PropertySheet.TextColor			"LightBlue"
		PropertySheet.SelectedTextColor	"White"
		PropertySheet.TransitionEffectTime	"0.6"	// time to change from one tab to another
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
		SectionedListPanel.HeaderBgColor	"Blank"
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
		TextEntry.DisabledBgColor	"Blank"
		TextEntry.SelectedTextColor	"Black"
		TextEntry.SelectedBgColor	"LightBlue"
		TextEntry.OutOfFocusSelectedBgColor	"LightBlue"
		TextEntry.FocusEdgeColor	"0 0 0 196"

		ToggleButton.SelectedTextColor	"White"

		Tooltip.TextColor			"0 0 0 196"
		Tooltip.BgColor				"LightBlue"

		TreeView.BgColor			"TransparentBlack"

		WizardSubPanel.BgColor		"Blank"

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
	//////////////////////// HYBRID BUTTON STYLES /////////////////////////////
	//
	// Custom styles for use with L4D360HybridButtons

		HybridButton.BorderColor					"GreyBlue"
		HybridButton.BlotchColor					"DarkBlueTrans"

		// These bypass all of CA's horrific style.  Look/Feel is code based
			
		// main or ingame menu only
		MainMenuButton.Style						"1"
		MainMenuButton.TextInsetY					"0"		[$WIN32]
		MainMenuButton.TextInsetY					"1"		[$X360HIDEF]
		MainMenuButton.TextInsetY					"0"		[$X360LODEF]
		MainMenuButton.Font							"MainBold"
		MainMenuButton.FontBlur						"MainBoldBlur"
		MainMenuButton.ColorEnabled					"83 148 192 255"
		MainMenuButton.ColorDisabled				"32 59 82 255"
		MainMenuButton.ColorFocusDisabled			"182 189 194 255"
		MainMenuButton.ColorOpen					"169 213 255 255"
		MainMenuButton.ColorFocus					"255 255 255 255"
		
		// inside of a flyout menu only
		FlyoutMenuButton.Style						"2"
		FlyoutMenuButton.TextInsetX					"8"
		FlyoutMenuButton.TextInsetY					"2"		[$WIN32]
		FlyoutMenuButton.TextInsetY					"5"		[$X360]
		FlyoutMenuButton.Font						"DefaultMedium"
		FlyoutMenuButton.FontBlur					"DefaultMediumBlur"
		FlyoutMenuButton.ColorEnabled				"83 148 192 255"
		FlyoutMenuButton.ColorDisabled				"32 59 82 255"
		FlyoutMenuButton.ColorFocusDisabled			"182 189 194 255"
		FlyoutMenuButton.ColorOpen					"169 213 255 255"
		FlyoutMenuButton.ColorFocus					"255 255 255 255"

		// inside a dialog, contains a RHS value, usually causes a flyout
		DropDownButton.Style						"3"
		DropDownButton.TextInsetY					"0"		[$WIN32HIDEF]
		DropDownButton.TextInsetY					"-1"	[$WIN32LODEF]
		DropDownButton.TextInsetY					"2"		[$X360HIDEF]
		DropDownButton.TextInsetY					"1"		[$X360LODEF]
		DropDownButton.Font							"DefaultBold"
		DropDownButton.FontBlur						"DefaultBoldBlur"
		DropDownButton.FontSelected					"DefaultMedium"
		DropDownButton.FontSelectedBlur				"DefaultMediumBlur"
		DropDownButton.ColorEnabled					"83 148 192 255"
		DropDownButton.ColorDisabled				"32 59 82 255"
		DropDownButton.ColorFocusDisabled			"182 189 194 255"
		DropDownButton.ColorOpen					"169 213 255 255"
		DropDownButton.ColorFocus					"255 255 255 255"

		// centers within the focus
		DialogButton.Style							"4"
		DialogButton.TextInsetY						"0"		[$WIN32HIDEF]
		DialogButton.TextInsetY						"-1"	[$WIN32LODEF]
		DialogButton.TextInsetY						"2"		[$X360HIDEF]
		DialogButton.TextInsetY						"1"		[$X360LODEF]
		DialogButton.Font							"DefaultBold"
		DialogButton.FontBlur						"DefaultBoldBlur"
		DialogButton.ColorEnabled					"83 148 192 255"
		DialogButton.ColorDisabled					"32 59 82 255"
		DialogButton.ColorFocusDisabled				"182 189 194 255"
		DialogButton.ColorOpen						"169 213 255 255"
		DialogButton.ColorFocus						"255 255 255 255"
		
		// left aligned within the focus
		DefaultButton.Style							"0"
		DefaultButton.TextInsetY					"0"		[$WIN32HIDEF]
		DefaultButton.TextInsetY					"-1"	[$WIN32LODEF]
		DefaultButton.TextInsetY					"2"		[$X360HIDEF]
		DefaultButton.TextInsetY					"1"		[$X360LODEF]
		DefaultButton.Font							"DefaultBold"
		DefaultButton.FontBlur						"DefaultBoldBlur"
		DefaultButton.ColorEnabled					"83 148 192 255"
		DefaultButton.ColorDisabled					"32 59 82 255"
		DefaultButton.ColorFocusDisabled			"182 189 194 255"
		DefaultButton.ColorOpen						"169 213 255 255"
		DefaultButton.ColorFocus					"255 255 255 255"
		
		// left aligned within the focus
		RedButton.Style								"5"
		RedButton.TextInsetY						"0"		[$WIN32HIDEF]
		RedButton.TextInsetY						"-1"	[$WIN32LODEF]
		RedButton.Font								"DefaultBold"
		RedButton.FontBlur							"DefaultBoldBlur"
		RedButton.ColorEnabled						"169 213 255 255"
		RedButton.ColorDisabled						"32 59 82 255"
		RedButton.ColorFocusDisabled				"182 189 194 255"
		RedButton.ColorOpen							"169 213 255 255"
		RedButton.ColorFocus						"255 255 255 255"

		// left aligned within the focus
		RedMainButton.Style							"6"
		RedMainButton.TextInsetY					"0"		[$WIN32HIDEF]
		RedMainButton.TextInsetY					"-1"	[$WIN32LODEF]
		RedMainButton.Font							"DefaultBold"
		RedMainButton.FontBlur						"DefaultBoldBlur"
		RedMainButton.ColorEnabled					"169 213 255 255"
		RedMainButton.ColorDisabled					"32 59 82 255"
		RedMainButton.ColorFocusDisabled			"182 189 194 255"
		RedMainButton.ColorOpen						"169 213 255 255"
		RedMainButton.ColorFocus					"255 255 255 255"
		
		// left aligned within the focus
		SmallButton.Style							"7"
		SmallButton.TextInsetY						"1"
		SmallButton.Font							"DefaultVerySmall"
		SmallButton.FontBlur						"DefaultVerySmall"
		SmallButton.ColorEnabled					"83 148 192 255"
		SmallButton.ColorDisabled					"32 59 82 255"
		SmallButton.ColorFocusDisabled				"182 189 194 255"
		SmallButton.ColorOpen						"169 213 255 255"
		SmallButton.ColorFocus						"255 255 255 255"

		MediumButton.Style							"8"
		MediumButton.Font							"DefaultMedium"
		MediumButton.FontBlur						"DefaultMediumBlur"
		MediumButton.ColorEnabled					"83 148 192 255"
		MediumButton.ColorDisabled					"32 59 82 255"
		MediumButton.ColorFocusDisabled				"182 189 194 255"
		MediumButton.ColorOpen						"169 213 255 255"
		MediumButton.ColorFocus						"255 255 255 255"

		// specialized button, only appears in game mode carousel
		GameModeButton.Style						"9"
		GameModeButton.TextInsetY					"0"		[$WIN32]
		GameModeButton.TextInsetY					"1"		[$X360HIDEF]
		GameModeButton.TextInsetY					"0"		[$X360LODEF]
		GameModeButton.Font							"MainBold"
		GameModeButton.FontBlur						"MainBoldBlur"
		GameModeButton.FontHint						"Default"
		GameModeButton.ColorEnabled					"83 148 192 255"
		GameModeButton.ColorDisabled				"32 59 82 255"
		GameModeButton.ColorFocusDisabled			"182 189 194 255"
		GameModeButton.ColorOpen					"169 213 255 255"
		GameModeButton.ColorFocus					"255 255 255 255"

		// main or ingame menu only
		MainMenuSmallButton.Style					"10"
		MainMenuSmallButton.Font					"DefaultBold"
		MainMenuSmallButton.FontBlur				"DefaultBoldBlur"
		MainMenuSmallButton.ColorEnabled			"83 148 192 255"
		MainMenuSmallButton.ColorDisabled			"32 59 82 255"
		MainMenuSmallButton.ColorFocusDisabled		"182 189 194 255"
		MainMenuSmallButton.ColorOpen				"169 213 255 255"
		MainMenuSmallButton.ColorFocus				"255 255 255 255"

		// who invented this crazy style system anyway?
		AlienSwarmMenuButton.Style					"11"
		AlienSwarmMenuButton.Font					"DefaultBold"
		AlienSwarmMenuButton.FontBlur				"DefaultBoldBlur"
		AlienSwarmMenuButton.ColorEnabled			"135 170 193 255"
		AlienSwarmMenuButton.ColorDisabled			"32 59 82 255"
		AlienSwarmMenuButton.ColorFocusDisabled		"182 189 194 255"
		AlienSwarmMenuButton.ColorOpen				"169 213 255 255"
		AlienSwarmMenuButton.ColorFocus				"255 255 255 255"

		AlienSwarmMenuButtonSmall.Style				"12"
		AlienSwarmMenuButtonSmall.Font				"DefaultMedium"
		AlienSwarmMenuButtonSmall.FontBlur			"DefaultMediumBlur"
		AlienSwarmMenuButtonSmall.ColorEnabled		"135 170 193 255"
		AlienSwarmMenuButtonSmall.ColorDisabled		"32 59 82 255"
		AlienSwarmMenuButtonSmall.ColorFocusDisabled	"182 189 194 255"
		AlienSwarmMenuButtonSmall.ColorOpen			"169 213 255 255"
		AlienSwarmMenuButtonSmall.ColorFocus		"255 255 255 255"

		AlienSwarmDefault.Style						"13"
		AlienSwarmDefault.Font						"Default"
		AlienSwarmDefault.FontBlur					"DefaultBlur"
		AlienSwarmDefault.ColorEnabled				"83 148 192 255"
		AlienSwarmDefault.ColorDisabled				"32 59 82 255"
		AlienSwarmDefault.ColorFocusDisabled		"182 189 194 255"
		AlienSwarmDefault.ColorOpen					"169 213 255 255"
		AlienSwarmDefault.ColorFocus				"255 255 255 255"

		ReactiveDropMainMenu.Style					"14"
		ReactiveDropMainMenu.Font					"DefaultLarge"
		ReactiveDropMainMenu.FontBlur				"DefaultLargeBlur"
		ReactiveDropMainMenu.ColorEnabled			"192 192 192 255"
		ReactiveDropMainMenu.ColorDisabled			"32 59 82 255"
		ReactiveDropMainMenu.ColorFocusDisabled		"182 189 194 255"
		ReactiveDropMainMenu.ColorOpen				"169 213 255 255"
		ReactiveDropMainMenu.ColorFocus				"255 255 255 255"

		ReactiveDropMainMenuBig.Style				"15"
		ReactiveDropMainMenuBig.Font				"DefaultExtraLarge"
		ReactiveDropMainMenuBig.FontBlur			"DefaultExtraLargeBlur"
		ReactiveDropMainMenuBig.ColorEnabled		"192 192 192 255"
		ReactiveDropMainMenuBig.ColorDisabled		"32 59 82 255"
		ReactiveDropMainMenuBig.ColorFocusDisabled	"182 189 194 255"
		ReactiveDropMainMenuBig.ColorOpen			"169 213 255 255"
		ReactiveDropMainMenuBig.ColorFocus			"255 255 255 255"

		ReactiveDropMainMenuTop.Style				"16"
		ReactiveDropMainMenuTop.Font				"DefaultMedium"
		ReactiveDropMainMenuTop.FontBlur			"DefaultMediumBlur"
		ReactiveDropMainMenuTop.ColorEnabled		"96 96 96 255"
		ReactiveDropMainMenuTop.ColorDisabled		"32 59 82 255"
		ReactiveDropMainMenuTop.ColorFocusDisabled	"182 189 194 255"
		ReactiveDropMainMenuTop.ColorOpen			"224 224 224 255"
		ReactiveDropMainMenuTop.ColorFocus			"255 255 255 255"

		ReactiveDropMainMenuShowcase.Style			"17"
		ReactiveDropMainMenuShowcase.TextInsetX		"3"
		ReactiveDropMainMenuShowcase.TextInsetY		"5"
		ReactiveDropMainMenuShowcase.Font			"Default"
		ReactiveDropMainMenuShowcase.FontBlur		"DefaultBlur"
		ReactiveDropMainMenuShowcase.ColorEnabled	"192 192 192 255"
		ReactiveDropMainMenuShowcase.ColorDisabled	"32 59 82 255"
		ReactiveDropMainMenuShowcase.ColorFocusDisabled	"182 189 194 255"
		ReactiveDropMainMenuShowcase.ColorOpen		"169 213 255 255"
		ReactiveDropMainMenuShowcase.ColorFocus		"255 255 255 255"

		ReactiveDropMainMenuTimer.Style				"18"
		ReactiveDropMainMenuTimer.TextInsetX		"2"
		ReactiveDropMainMenuTimer.TextInsetY		"1"
		ReactiveDropMainMenuTimer.Font				"Default"
		ReactiveDropMainMenuTimer.FontBlur			"DefaultBlur"
		ReactiveDropMainMenuTimer.ColorEnabled		"192 192 192 255"
		ReactiveDropMainMenuTimer.ColorDisabled		"32 59 82 255"
		ReactiveDropMainMenuTimer.ColorFocusDisabled	"182 189 194 255"
		ReactiveDropMainMenuTimer.ColorOpen			"169 213 255 255"
		ReactiveDropMainMenuTimer.ColorFocus		"255 255 255 255"

		ReactiveDropMainMenuHoIAF.Style				"19"
		ReactiveDropMainMenuHoIAF.Font				"DefaultMedium"
		ReactiveDropMainMenuHoIAF.FontBlur			"DefaultMediumBlur"
		ReactiveDropMainMenuHoIAF.FontRank			"DefaultVerySmall"
		ReactiveDropMainMenuHoIAF.FontScore			"Default"
		ReactiveDropMainMenuHoIAF.FontLarge			"DefaultBold"
		ReactiveDropMainMenuHoIAF.FontLargeBlur		"DefaultBoldBlur"
		ReactiveDropMainMenuHoIAF.FontLargeRank		"DefaultSmall"
		ReactiveDropMainMenuHoIAF.FontLargeScore	"DefaultMedium"
		ReactiveDropMainMenuHoIAF.ColorEnabled		"192 192 192 255"
		ReactiveDropMainMenuHoIAF.ColorDisabled		"32 59 82 255"
		ReactiveDropMainMenuHoIAF.ColorFocusDisabled	"182 189 194 255"
		ReactiveDropMainMenuHoIAF.ColorOpen			"169 213 255 255"
		ReactiveDropMainMenuHoIAF.ColorFocus		"255 255 255 255"
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
		
		"DefaultSystemUI" [$WIN32]
		{
			"1"
			{
				"name"		"Veranda" //"Verdana" //"Neo Sans"
				"tall"		"14"
				"weight"	"100"
				"antialias"	"1"
			}
		}
		
		// default font used for regular text throughout
		"Default"
		{
			"1"
			{
				"name"		"Neo Sans"
				"tall"		"10"
				"weight"	"100"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"antialias"	"1"
			}
		}

		"DefaultNoMinRes"
		{
			"1"
			{
				"name"		"Neo Sans"
				"tall"		"10"
				"weight"	"100"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"antialias"	"1"
			}
		}
		"DefaultBlur"
		{
			"1"
			{
				"name"		"Neo Sans"
				"tall"		"10"
				"weight"	"100"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"antialias"	"1"
				"blur"		"2"
			}
		}
		"DefaultShadowed"
		{
			"1"
			{
				"name"		"Neo Sans"
				"tall"		"10"
				"weight"	"100"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"antialias"	"1"
				"dropshadow"	"2"
			}
		}
		"DefaultUnderline"
		{
			"1"
			{
				"name"		"Neo Sans"
				"tall"		"10"
				"weight"	"100"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"antialias"	"1"
				"underline"	"1"
			}
		}
		"DefaultVerySmall"
		{
			"1"
			{
				"name"		"Veranda" //"Verdana" //"Neo Sans"
				"tall"		"7"
				"weight"	"100"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"antialias"	"1"
			}
		}
		"DefaultVerySmallBlur"
		{
			"1"
			{
				"name"		"Veranda" //"Verdana" //"Neo Sans"
				"tall"		"7"
				"weight"	"100"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				//"antialias"	"1"
				"blur"		"5"
			}
		}
		"DefaultSmall"
		{
			"1"
			{
				"name"		"Veranda" //"Verdana" //"Neo Sans"
				"tall"		"8"
				"weight"	"100"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"antialias"	"1"
			}
		}
		"DefaultSmallBlur"
		{
			"1"
			{
				"name"		"Veranda" //"Verdana" //"Neo Sans"
				"tall"		"8"
				"weight"	"100"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"antialias"	"1"
				"blur"		"5"
			}
		}
		"DefaultSmallOutline"
		{
			"1"
			{
				"name"		"Veranda" //"Verdana"
				"tall"		"8"
				"weight"	"100"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"antialias"	"1"
				"outline"	"1"
			}
		}
		"DefaultMedium"
		{
			"1"
			{
				"name"		"Neo Sans"
				"tall"		"12"
				"weight"	"100"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"antialias"	"1"
			}
		}
		"DefaultMediumBlur"
		{
			"1"
			{
				"name"		"Neo Sans"
				"tall"		"12"
				"weight"	"100"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"blur"		"3"
			}
		}
		// DefaultLarge is used for headers
		"DefaultLarge"
		{
			"1"
			{
				"name"		"Neo Sans"
				"tall"		"15"
				"weight"	"100"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"antialias"	"1"
			}
		}
		"DefaultLargeBlur"
		{
			"1"
			{
				"name"		"Neo Sans"
				"tall"		"15"
				"weight"	"100"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"antialias"	"1"
				"blur"		"3"
			}
		}
		// DefaultExtraLarge
		"DefaultExtraLarge"
		{
			"1"
			{
				"name"		"Neo Sans"
				"tall"		"20"
				"weight"	"100"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"antialias"	"1"
			}
		}
		"DefaultExtraLargeBlur"
		{
			"1"
			{
				"name"		"Neo Sans"
				"tall"		"20"
				"weight"	"100"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"antialias"	"1"
				"blur"		"3"
			}
		}

		"DefaultBold"
		{
			"1"
			{
				"name"		"Neo Sans"
				"tall"		"16"
				"weight"	"100"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"antialias"	"1"
			}
		}

		"DefaultBoldBlur"
		{
			"1"
			{
				"name"		"Neo Sans"
				"tall"		"16"
				"weight"	"100"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"antialias"	"1"
				"blur"		"3"
			}
		}

		"DefaultTextBold"
		{
			"1"
			{
				"name"		"Neo Sans"
				"tall"		"10"
				"weight"	"700"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"antialias"	"1"
			}
		}

		"DefaultTextItalic"
		{
			"1"
			{
				"name"		"Neo Sans"
				"tall"		"10"
				"weight"	"100"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"antialias"	"1"
				"italic"	"1"
			}
		}

		"DefaultTextBoldItalic"
		{
			"1"
			{
				"name"		"Neo Sans"
				"tall"		"10"
				"weight"	"700"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"antialias"	"1"
				"italic"	"1"
			}
		}

		"Combine"
		{
			"1"
			{
				"name"		"CMB Combine"
				"tall"		"10"
				"weight"	"400"
				"range"		"0x0000 0x007F" //	ASCII
				"antialias"	"1"
			}
		}

		"CombineExtraLarge"
		{
			"1"
			{
				"name"		"CMB Combine"
				"tall"		"20"
				"weight"	"400"
				"range"		"0x0000 0x007F" //	ASCII
				"antialias"	"1"
			}
		}

		"Countdown"
		{
			"1"
			{
				"name"		"Veranda" //"Verdana"
				"tall"		"20"
				"weight"	"900"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"antialias"	"1"
			}
		}
		"CountdownSmall"
		{
			"1"
			{
				"name"		"Veranda" //"Verdana"
				"tall"		"18"
				"weight"	"900"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"antialias"	"1"
			}
		}
		"CountdownBlur"
		{
			"1"
			{
				"name"		"Veranda" //"Verdana"
				"tall"		"18"
				"weight"	"900"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"scanlines"	"1"
				"blur"	"2"
			}
		}

		//===================================
		// L4D MATCHMAKING FONTS
		//===================================
		
		"FrameTitle"
		{
			"1"
			{
				"name"			"Neo Sans"
				"tall"			"24"
				"weight"		"400"
				"antialias"		"1"
			}
		}

		"FrameTitleBlur"
		{
			"1"
			{
				"name"			"Neo Sans"
				"tall"			"24"
				"weight"		"400"
				"blur"			"3"					[$WIN32 || $X360LODEF]
				"blur"			"5"					[$X360HIDEF]
				"antialias"		"1"
			}
		}

		"MainBold"
		{
			"1"
			{
				"name"			"Neo Sans"
				"tall"			"20"
				"weight"		"400"				[$WIN32]
				"weight"		"800"				[$X360]
				"antialias"		"1"
			}
		}

		"MainBoldBlur"
		{
			"1"
			{
				"name"			"Neo Sans"
				"tall"			"20"
				"weight"		"400"				[$WIN32]
				"weight"		"800"				[$X360]
				"blur"			"3"
				"antialias"		"1"
			}
		}

		GameUIButtons
		{
			"1"
			{
				"bitmap"	"1"
				"name"		"Buttons"
				"scalex"	"0.80"
				"scaley"	"0.80"
			}
		}

		GameUIButtonsMini
		{
			"1"
			{
				"bitmap"	"1"
				"name"		"Buttons"
				"scalex"	"0.65"
				"scaley"	"0.65"
			}
		}
		
		GameUIButtonsTiny
		{
			"1"
			{
				"bitmap"	"1"
				"name"		"Buttons"
				"scalex"	"0.5"
				"scaley"	"0.5"
			}
		}
		
		GameUIButtonsTinier
		{
			"1"
			{
				"bitmap"	"1"
				"name"		"Buttons"
				"scalex"	"0.25"
				"scaley"	"0.25"
			}
		}
		
		//
		// ScreenTitle is the super large font ONLY used as THE major screen title heading.
		//
		"ScreenTitle"
		{
			"1"
			{
				"name"		"Neo Sans"
				"tall"		"28"
				"weight"	"400"
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
		"MarlettHalf"
		{
			"1"
			{
				"name"		"Marlett"
				"tall"		"9"
				"weight"	"0"
				"symbol"	"1"
				"range"		"0x0000 0x007F"	//	Basic Latin
				"antialias"	"1"
			}
		}
	}

	//
	//////////////////// BORDERS //////////////////////////////
	//
	// describes all the border types
	Borders
	{
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
			"inset" "0 0 1 1"
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
				"4"
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

		TitleButtonDisabledBorder
		{
			"inset" "0 0 1 1"
			Left
			{
				"1"
				{
					"color" "BgColor"
					"offset" "0 1"
				}
			}

			Right
			{
				"1"
				{
					"color" "BgColor"
					"offset" "1 0"
				}
			}
			Top
			{
				"1"
				{
					"color" "BgColor"
					"offset" "0 0"
				}
			}

			Bottom
			{
				"1"
				{
					"color" "BgColor"
					"offset" "0 0"
				}
			}
		}

		TitleButtonDepressedBorder
		{
			"inset" "1 1 1 1"
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

		ButtonBorder
		{
			"inset" "0 0 1 1"
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
					"offset" "0 0"
				}
			}

			Top
			{
				"1"
				{
					"color" "BorderBright"
					"offset" "0 1"
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

		TabBorder
		{
			"inset" "0 0 1 1"
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
					"color" "BorderBright"
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
					"color" "BorderBright"
					"offset" "0 0"
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
					"color" "ControlBG"
					"offset" "6 2"
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
					"color" "BorderDark"
					"offset" "0 0"
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
					"color" "BorderDark"
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

		// this is the border used for default buttons (the button that gets pressed when you hit enter)
		ButtonKeyFocusBorder
		{
			"inset" "0 0 1 1"
			Left
			{
				"1"
				{
					"color" "BorderSelection"
					"offset" "0 0"
				}
				"2"
				{
					"color" "BorderBright"
					"offset" "0 1"
				}
			}
			Top
			{
				"1"
				{
					"color" "BorderSelection"
					"offset" "0 0"
				}
				"2"
				{
					"color" "BorderBright"
					"offset" "1 0"
				}
			}
			Right
			{
				"1"
				{
					"color" "BorderSelection"
					"offset" "0 0"
				}
				"2"
				{
					"color" "BorderDark"
					"offset" "1 0"
				}
			}
			Bottom
			{
				"1"
				{
					"color" "BorderSelection"
					"offset" "0 0"
				}
				"2"
				{
					"color" "BorderDark"
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

		ComboBoxBorder
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

		MenuBorder
		{
			"inset" "1 1 1 1"
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
	BitmapFontFiles
	{
		"Buttons"		"materials/vgui/fonts/buttons_32.vbf"
	}
	CustomFontFiles
	{
		"1"		"resource/neosans.vfont"
		"2"		"resource/veranda.vfont"
		"3"		"resource/cmb-combine.vfont"
	}
}
