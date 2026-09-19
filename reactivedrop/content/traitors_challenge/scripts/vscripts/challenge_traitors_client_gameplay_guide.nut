// Public, client-only gameplay guide for the Traitors challenge.
//
// The server creates one rd_vgui_vscript per player and sends only the
// player's language in string slot 0.  Keeping the text in the client-side
// translation table avoids the 256-byte rd_vgui_vscript network-string limit
// and, importantly, keeps this guide identical for spectators and Marines.

isServer <- false;
IncludeScript("challenge_traitors_enums");
IncludeScript("challenge_traitors_client_shared");

// The generated translation file creates a g_localizations table in the scope
// passed to IncludeScript.  Keep that table in a private child scope instead
// of the VGUI script scope (or the client root), because one guide entity is
// created for every player and all of them execute this script independently.
// The table is loaded lazily only for the local player's open entity: parsing
// the generated file for every remote spectator would be wasteful.
g_guideTranslationLoaded <- false;
g_guideTranslationScope <- {};

GUIDE_PAGE_MAIN <- 0;
GUIDE_PAGE_TEAM_OBJECTIVES <- 1;
GUIDE_PAGE_MECHANICS <- 2;
GUIDE_PAGE_ROLES <- 3;
GUIDE_PAGE_TIPS <- 4;
GUIDE_MAIN_BUTTON_COUNT <- 4;
GUIDE_BACK_BUTTON_INDEX <- 4;
GUIDE_TIP_COUNT <- 11;

// These are the complete public guide vocabulary.  Do not put guide prose in
// this script: all text comes from challenge_traitors_translations_all.nut.
// Keys are stored without '#'.  GetGuideTableString also accepts a leading
// '#' so older callers and both formal/test scripts use the same lookup path.
GUIDE_KEY_TITLE <- "challenge_traitors_guide_title";
GUIDE_KEY_INTRO <- "challenge_traitors_guide_intro";
GUIDE_KEY_INTRO_PARTS <- [
	"challenge_traitors_guide_intro_01",
	"challenge_traitors_guide_intro_02",
	"challenge_traitors_guide_intro_03",
	"challenge_traitors_guide_intro_04",
	"challenge_traitors_guide_intro_05"
];
GUIDE_KEY_TEAM_BUTTON <- "challenge_traitors_guide_team_objectives_button";
GUIDE_KEY_MECHANICS_BUTTON <- "challenge_traitors_guide_mechanics_button";
GUIDE_KEY_ROLES_BUTTON <- "challenge_traitors_guide_roles_button";
GUIDE_KEY_TIPS_BUTTON <- "challenge_traitors_guide_tips_button";
GUIDE_KEY_TEAM_TITLE <- "challenge_traitors_guide_team_objectives_title";
GUIDE_KEY_TEAM_TEXT <- "challenge_traitors_guide_team_objectives_text";
GUIDE_KEY_TEAM_TEXT_PARTS <- [
	"challenge_traitors_guide_team_objectives_text_01",
	"challenge_traitors_guide_team_objectives_text_02",
	"challenge_traitors_guide_team_objectives_text_03",
	"challenge_traitors_guide_team_objectives_text_04",
	"challenge_traitors_guide_team_objectives_text_05",
	"challenge_traitors_guide_team_objectives_text_06",
	"challenge_traitors_guide_team_objectives_text_07",
	"challenge_traitors_guide_team_objectives_text_08",
	"challenge_traitors_guide_team_objectives_text_09",
	"challenge_traitors_guide_team_objectives_text_10"
];
GUIDE_KEY_MECHANICS_TITLE <- "challenge_traitors_guide_mechanics_title";
GUIDE_KEY_MECHANICS_TEXT <- "challenge_traitors_guide_mechanics_text";
GUIDE_KEY_MECHANICS_TEXT_PARTS <- [
	"challenge_traitors_guide_mechanics_text_01",
	"challenge_traitors_guide_mechanics_text_02",
	"challenge_traitors_guide_mechanics_text_03",
	"challenge_traitors_guide_mechanics_text_04",
	"challenge_traitors_guide_mechanics_text_05",
	"challenge_traitors_guide_mechanics_text_06",
	"challenge_traitors_guide_mechanics_text_07",
	"challenge_traitors_guide_mechanics_text_08",
	"challenge_traitors_guide_mechanics_text_09",
	"challenge_traitors_guide_mechanics_text_10",
	"challenge_traitors_guide_mechanics_text_11",
	"challenge_traitors_guide_mechanics_text_12"
];
GUIDE_KEY_ROLES_TITLE <- "challenge_traitors_guide_roles_title";
GUIDE_KEY_ROLES_TEXT <- "challenge_traitors_guide_roles_text";
GUIDE_KEY_ROLES_TEXT_PARTS <- [
	"challenge_traitors_guide_roles_text_01",
	"challenge_traitors_guide_roles_text_02",
	"challenge_traitors_guide_roles_text_03",
	"challenge_traitors_guide_roles_text_04",
	"challenge_traitors_guide_roles_text_05",
	"challenge_traitors_guide_roles_text_06",
	"challenge_traitors_guide_roles_text_07",
	"challenge_traitors_guide_roles_text_08",
	"challenge_traitors_guide_roles_text_09",
	"challenge_traitors_guide_roles_text_10",
	"challenge_traitors_guide_roles_text_11",
	"challenge_traitors_guide_roles_text_12",
	"challenge_traitors_guide_roles_text_13",
	"challenge_traitors_guide_roles_text_14",
	"challenge_traitors_guide_roles_text_15",
	"challenge_traitors_guide_roles_text_16"
];
GUIDE_KEY_TIPS_TITLE <- "challenge_traitors_guide_tips_title";
GUIDE_KEY_TIP_01 <- "challenge_traitors_guide_tip_01";
GUIDE_KEY_TIP_02 <- "challenge_traitors_guide_tip_02";
GUIDE_KEY_TIP_03 <- "challenge_traitors_guide_tip_03";
GUIDE_KEY_TIP_04 <- "challenge_traitors_guide_tip_04";
GUIDE_KEY_TIP_05 <- "challenge_traitors_guide_tip_05";
GUIDE_KEY_TIP_06 <- "challenge_traitors_guide_tip_06";
GUIDE_KEY_TIP_07 <- "challenge_traitors_guide_tip_07";
GUIDE_KEY_TIP_08 <- "challenge_traitors_guide_tip_08";
GUIDE_KEY_TIP_09 <- "challenge_traitors_guide_tip_09";
GUIDE_KEY_TIP_10 <- "challenge_traitors_guide_tip_10";
GUIDE_KEY_TIP_11 <- "challenge_traitors_guide_tip_11";

FONT_GUIDE_TITLE <- self.LookupFont("DefaultLarge");
FONT_GUIDE_BODY <- self.LookupFont("DefaultSmall");
FONT_GUIDE_BUTTON <- self.LookupFont("DefaultLarge");

page <- GUIDE_PAGE_MAIN;
wasVisible <- false;
seenOpenGeneration <- -1;
mouseDown <- false;
hotButton <- -1;
buttonPressed <- false;

panelX <- 0.0;
panelY <- 0.0;
panelRight <- 0.0;
panelBottom <- 0.0;
panelWidth <- 0.0;
panelHeight <- 0.0;
margin <- 0.0;
spacing <- 0.0;
titleHeight <- 0.0;
bodyLineHeight <- 0.0;
buttonHeight <- 0.0;
bodyX <- 0.0;
bodyY <- 0.0;
bodyRight <- 0.0;
backX0 <- 0.0;
backX1 <- 0.0;
bottomButtonY <- 0.0;

guideTitle <- "";
guideIntro <- "";
guideTeamButton <- "";
guideMechanicsButton <- "";
guideRolesButton <- "";
guideTipsButton <- "";
guideTeamTitle <- "";
guideTeamText <- "";
guideMechanicsTitle <- "";
guideMechanicsText <- "";
guideRolesTitle <- "";
guideRolesText <- "";
guideTipsTitle <- "";
guideTips <- [];
detailTitle <- "";
detailText <- "";
bodyLines <- [];

function GuideLanguage() {
	local strLanguage = self.GetString(0);
	try {
		strLanguage = strLanguage.tolower();
	} catch (exception) {
		strLanguage = "english";
	}
	if (strLanguage != "english" && strLanguage != "schinese") {
		return "english";
	}
	return strLanguage;
}

function EnsureGuideTranslations() {
	if (g_guideTranslationLoaded) {
		return;
	}
	try {
		// The generated file is data, not a module: it assigns g_localizations
		// in the scope supplied here.  Never include it into getroottable(),
		// where it would collide with challenge/server localization globals.
		if (!IncludeScript("challenge_traitors_translations_all.nut", g_guideTranslationScope)) {
			return;
		}
		g_guideTranslationLoaded = true;
	} catch (exception) {
		g_guideTranslationLoaded = false;
	}
}

function GetGuideTableString(language, key) {
	try {
		if (!g_guideTranslationScope.rawin("g_localizations")) {
			return null;
		}
		local localizations = g_guideTranslationScope.g_localizations;
		if (localizations == null || !localizations.rawin(language)) {
			return null;
		}
		local token = key;
		if (token.len() > 0 && token.slice(0, 1) == "#") {
			token = token.slice(1);
		}
		local languageTable = localizations[language];
		if (languageTable == null || !languageTable.rawin(token)) {
			return null;
		}
		local result = languageTable[token];
		if (result == null || result == "") {
			return null;
		}
		return result;
	} catch (exception) {
		return null;
	}
}

function ResolveGuideString(key) {
	EnsureGuideTranslations();
	local language = GuideLanguage();
	// The generated table is keyed by token without the leading '#'.  Guide
	// prose has no role-substitution parameters, so reading this table directly
	// is equivalent to GetLocalizedString while remaining safe in the private
	// scope above.  If a supported-language entry is absent, retry English;
	// a missing token remains visible as '#token' for diagnosis.
	local result = GetGuideTableString(language, key);
	if (result == null && language != "english") {
		result = GetGuideTableString("english", key);
	}
	return result == null ? "#" + (key.len() > 0 && key.slice(0, 1) == "#" ? key.slice(1) : key) : result;
}

function ResolveGuideParts(partKeys, separator = "\n") {
	local result = "";
	for (local i = 0; i < partKeys.len(); i++) {
		if (i > 0) {
			result += separator;
		}
		// Resolve each part independently.  This deliberately keeps the helper
		// small: a missing translation falls back to English (or visibly shows
		// its token) without suppressing the other translated parts.
		result += ResolveGuideString(partKeys[i]);
	}
	return result;
}

function IsLocalGuideOwner() {
	local hPlayer = GetLocalPlayer();
	return hPlayer != null && hPlayer.IsValid() && self.GetEntity(0) == hPlayer;
}

function IsGuideOpenForLocalPlayer() {
	return self.GetInt(0) != 0 && IsLocalGuideOwner();
}

function ResetGuidePage() {
	page = GUIDE_PAGE_MAIN;
	hotButton = -1;
	buttonPressed = false;
	mouseDown = false;
}

function CalculateLayout() {
	local screenWidth = ScreenWidth().tofloat();
	local screenHeight = ScreenHeight().tofloat();
	local scale = screenHeight / 768.0;
	if (scale < 0.75) {
		scale = 0.75;
	}

	panelWidth = screenWidth * 0.86;
	local maxWidth = 980.0 * scale;
	if (panelWidth > maxWidth) {
		panelWidth = maxWidth;
	}
	panelHeight = screenHeight * 0.84;
	panelX = (screenWidth - panelWidth) * 0.5;
	panelY = (screenHeight - panelHeight) * 0.5;
	panelRight = panelX + panelWidth;
	panelBottom = panelY + panelHeight;
	margin = 18.0 * scale;
	spacing = 8.0 * scale;
	titleHeight = self.GetFontTall(FONT_GUIDE_TITLE);
	bodyLineHeight = self.GetFontTall(FONT_GUIDE_BODY) + 2.0 * scale;
	buttonHeight = self.GetFontTall(FONT_GUIDE_BUTTON) + 16.0 * scale;

	local backWidth = 180.0 * scale;
	backX0 = panelX + margin;
	backX1 = backX0 + backWidth;

	// The home page has fixed category buttons at the bottom.  The body
	// text uses the full width available inside the panel.
	local headerHeight = titleHeight;
	if (page == GUIDE_PAGE_MAIN && buttonHeight > headerHeight) {
		headerHeight = buttonHeight;
	}
	bodyX = panelX + margin;
	bodyY = panelY + margin + headerHeight + margin;
	local bottomRows = page == GUIDE_PAGE_MAIN ? GUIDE_MAIN_BUTTON_COUNT : 1;
	local bottomSpacing = page == GUIDE_PAGE_MAIN ? spacing * (GUIDE_MAIN_BUTTON_COUNT - 1) : 0.0;
	bottomButtonY = panelBottom - margin - buttonHeight * bottomRows - bottomSpacing;
	bodyRight = panelRight - margin;
}

function GetUtf8CharLength(text, index) {
	local length = 1;
	try {
		local byte = text[index];
		if (typeof(byte) == "string") {
			byte = byte[0];
		}
		if (typeof(byte) == "integer") {
			if ((byte & 0xE0) == 0xC0) {
				length = 2;
			} else if ((byte & 0xF0) == 0xE0) {
				length = 3;
			} else if ((byte & 0xF8) == 0xF0) {
				length = 4;
			}
		}
	} catch (exception) {}
	if (index + length > text.len()) {
		return 1;
	}
	return length;
}

function WrapGuideText(text, font, width) {
	local result = [];
	local current = "";
	local index = 0;
	while (index < text.len()) {
		local charLength = GetUtf8CharLength(text, index);
		local character = text.slice(index, index + charLength);
		index += charLength;
		if (character == "\r") {
			continue;
		}
		if (character == "\n") {
			result.append(current);
			current = "";
			continue;
		}
		if (current != "" && self.GetTextWide(font, current + character) > width) {
			result.append(current);
			current = "";
		}
		current += character;
	}
	if (current != "" || result.len() == 0) {
		result.append(current);
	}
	return result;
}

function RefreshGuideContent() {
	guideTitle = ResolveGuideString(GUIDE_KEY_TITLE);
	guideIntro = ResolveGuideParts(GUIDE_KEY_INTRO_PARTS);
	guideTeamButton = ResolveGuideString(GUIDE_KEY_TEAM_BUTTON);
	guideMechanicsButton = ResolveGuideString(GUIDE_KEY_MECHANICS_BUTTON);
	guideRolesButton = ResolveGuideString(GUIDE_KEY_ROLES_BUTTON);
	guideTipsButton = ResolveGuideString(GUIDE_KEY_TIPS_BUTTON);
	guideTeamTitle = ResolveGuideString(GUIDE_KEY_TEAM_TITLE);
	guideTeamText = ResolveGuideParts(GUIDE_KEY_TEAM_TEXT_PARTS);
	guideMechanicsTitle = ResolveGuideString(GUIDE_KEY_MECHANICS_TITLE);
	guideMechanicsText = ResolveGuideParts(GUIDE_KEY_MECHANICS_TEXT_PARTS);
	guideRolesTitle = ResolveGuideString(GUIDE_KEY_ROLES_TITLE);
	guideRolesText = ResolveGuideParts(GUIDE_KEY_ROLES_TEXT_PARTS);
	guideTipsTitle = ResolveGuideString(GUIDE_KEY_TIPS_TITLE);
	guideTips = [
		ResolveGuideString(GUIDE_KEY_TIP_01),
		ResolveGuideString(GUIDE_KEY_TIP_02),
		ResolveGuideString(GUIDE_KEY_TIP_03),
		ResolveGuideString(GUIDE_KEY_TIP_04),
		ResolveGuideString(GUIDE_KEY_TIP_05),
		ResolveGuideString(GUIDE_KEY_TIP_06),
		ResolveGuideString(GUIDE_KEY_TIP_07),
		ResolveGuideString(GUIDE_KEY_TIP_08),
		ResolveGuideString(GUIDE_KEY_TIP_09),
		ResolveGuideString(GUIDE_KEY_TIP_10),
		ResolveGuideString(GUIDE_KEY_TIP_11)
	];

	if (page == GUIDE_PAGE_TEAM_OBJECTIVES) {
		detailTitle = guideTeamTitle;
		detailText = guideTeamText;
	} else if (page == GUIDE_PAGE_MECHANICS) {
		detailTitle = guideMechanicsTitle;
		detailText = guideMechanicsText;
	} else if (page == GUIDE_PAGE_ROLES) {
		detailTitle = guideRolesTitle;
		detailText = guideRolesText;
	} else if (page == GUIDE_PAGE_TIPS) {
		detailTitle = guideTipsTitle;
		detailText = "";
	} else {
		detailTitle = guideTitle;
		detailText = guideIntro;
	}
	CalculateLayout();
	local wrapWidth = bodyRight - bodyX;
	if (page == GUIDE_PAGE_TIPS) {
		// Wrap each tip independently so token order is stable without adding
		// blank separator lines that would consume the no-scroll layout.
		bodyLines = [];
		for (local i = 0; i < GUIDE_TIP_COUNT; i++) {
			local tipLines = WrapGuideText(guideTips[i], FONT_GUIDE_BODY, wrapWidth);
			foreach (line in tipLines) {
				if (line != "") {
					bodyLines.append(line);
				}
			}
		}
	} else {
		bodyLines = WrapGuideText(detailText, FONT_GUIDE_BODY, wrapWidth);
	}
}

function IsInside(x, y, rect) {
	return x >= rect[0] && x <= rect[2] && y >= rect[1] && y <= rect[3];
}

function GetMainButtonRect(index) {
	local x0 = panelX + margin;
	local x1 = panelRight - margin;
	local y0 = bottomButtonY + index * (buttonHeight + spacing);
	return [x0, y0, x1, y0 + buttonHeight];
}

function GetGuideButtonAt(x, y) {
	if (page == GUIDE_PAGE_MAIN) {
		for (local i = 0; i < GUIDE_MAIN_BUTTON_COUNT; i++) {
			if (IsInside(x, y, GetMainButtonRect(i))) {
				return i;
			}
		}
		return -1;
	}
	if (y >= bottomButtonY && y <= bottomButtonY + buttonHeight) {
		if (x >= backX0 && x <= backX1) {
			return GUIDE_BACK_BUTTON_INDEX;
		}
	}
	return -1;
}

function DrawGuideButton(x0, y0, x1, y1, text, index, enabled = true) {
	local isHot = enabled && hotButton == index;
	local isPressed = isHot && buttonPressed;
	local r = 35;
	local g = 45;
	local b = 55;
	local textR = 255;
	local textG = 255;
	local textB = 255;
	if (!enabled) {
		r = 18;
		g = 22;
		b = 26;
		textR = 100;
		textG = 100;
		textB = 100;
	} else if (isPressed) {
		r = 115;
		g = 105;
		b = 45;
		textR = 0;
		textG = 0;
		textB = 0;
	} else if (isHot) {
		r = 210;
		g = 210;
		b = 120;
		textR = 0;
		textG = 0;
		textB = 0;
	}
	self.PaintRectangle(x0, y0, x1, y1, r, g, b, 230);
	local textWidth = self.GetTextWide(FONT_GUIDE_BUTTON, text);
	local textX = (x0 + x1 - textWidth) * 0.5;
	local textY = y0 + (y1 - y0 - self.GetFontTall(FONT_GUIDE_BUTTON)) * 0.5;
	self.PaintText(textX, textY, textR, textG, textB, 255, FONT_GUIDE_BUTTON, text);
}

function PaintGuideText(x, y) {
	for (local i = 0; i < bodyLines.len(); i++) {
		self.PaintText(x, y + i * bodyLineHeight, 235, 235, 235, 255, FONT_GUIDE_BODY, bodyLines[i]);
	}
}

function Paint() {
	// Hidden screens must not draw or consume input.  The explicit player and
	// Int(0) checks also prevent another client's VGUI entity from appearing in
	// this client's view.
	if (!IsGuideOpenForLocalPlayer()) {
		return;
	}
	CalculateLayout();
	self.PaintRectangle(panelX, panelY, panelRight, panelBottom, 0, 0, 0, 225);
	self.PaintRectangle(panelX + 3, panelY + 3, panelRight - 3, panelBottom - 3, 12, 18, 24, 225);
	local titleText = page == GUIDE_PAGE_MAIN ? guideTitle : detailTitle;
	local titleWidth = self.GetTextWide(FONT_GUIDE_TITLE, titleText);
	local titleX = (panelX + panelRight - titleWidth) * 0.5;
	self.PaintText(titleX, panelY + margin, 255, 255, 210, 255, FONT_GUIDE_TITLE, page == GUIDE_PAGE_MAIN ? guideTitle : detailTitle);

	if (page == GUIDE_PAGE_MAIN) {
		PaintGuideText(bodyX, bodyY);
		for (local i = 0; i < GUIDE_MAIN_BUTTON_COUNT; i++) {
			local button = GetMainButtonRect(i);
			local buttonText = i == 0 ? guideTeamButton : (i == 1 ? guideMechanicsButton : (i == 2 ? guideRolesButton : guideTipsButton));
			DrawGuideButton(button[0], button[1], button[2], button[3], buttonText, i);
		}
	} else {
		PaintGuideText(bodyX, bodyY);
		DrawGuideButton(backX0, bottomButtonY, backX1, bottomButtonY + buttonHeight, "<<", GUIDE_BACK_BUTTON_INDEX);
	}
}

function ActivateGuideButton(index) {
	if (page == GUIDE_PAGE_MAIN) {
		if (index == 0) {
			page = GUIDE_PAGE_TEAM_OBJECTIVES;
		} else if (index == 1) {
			page = GUIDE_PAGE_MECHANICS;
		} else if (index == 2) {
			page = GUIDE_PAGE_ROLES;
		} else if (index == 3) {
			page = GUIDE_PAGE_TIPS;
		} else {
			return;
		}
		RefreshGuideContent();
		return;
	}
	if (index == GUIDE_BACK_BUTTON_INDEX) {
		ResetGuidePage();
		RefreshGuideContent();
	}
}

function Control(tbl) {
	// Returning before touching mouse/key state is intentional: when hidden,
	// the engine's normal controls continue receiving every input event.
	if (!IsGuideOpenForLocalPlayer()) {
		hotButton = -1;
		mouseDown = false;
		buttonPressed = false;
		return;
	}
	CalculateLayout();
	hotButton = GetGuideButtonAt(tbl.mouse_x, tbl.mouse_y);
	buttonPressed = false;

	if (tbl.mouse_left) {
		mouseDown = true;
		buttonPressed = hotButton >= 0;
	} else if (mouseDown) {
		local clickedButton = hotButton;
		mouseDown = false;
		buttonPressed = false;
		if (clickedButton >= 0) {
			ActivateGuideButton(clickedButton);
		}
	}
}

function OnUpdate() {
	self.ForceSync();
	local openForLocalPlayer = IsGuideOpenForLocalPlayer();
	local openGeneration = self.GetInt(1);
	if (!openForLocalPlayer) {
		if (wasVisible) {
			// A close (Int(0) becoming zero) resets the page.  The next open
			// therefore always starts at MAIN.
			ResetGuidePage();
		}
		wasVisible = false;
		seenOpenGeneration = openGeneration;
		return;
	}
	if (!wasVisible || seenOpenGeneration != openGeneration) {
		ResetGuidePage();
	}
	wasVisible = true;
	seenOpenGeneration = openGeneration;
	RefreshGuideContent();
}
