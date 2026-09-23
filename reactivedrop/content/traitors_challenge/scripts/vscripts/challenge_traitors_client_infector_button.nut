/*参数说明
	int		0  - 按钮序号
	int		1  - 士兵entindex
	int		2  - 总人数

	float	0  - 允许使用技能的开始时间

	string	0  - 名字

	entity 0 - marine

	DefaultVerySmall
	DefaultVerySmallBlur
	DefaultSmall
	DefaultSmallBlur
	DefaultSmallOutline
	Default
	DefaultBlur
	DefaultShadowed
efaultUnderline
	DefaultMedium
	DefaultMediumBlur
	DefaultLarge
	DefaultLargeBlur
	DefaultBold
	DefaultBoldBlur
	DefaultExtraLarge
	DefaultExtraLargeBlur
	Countdown
	CountdownBlur
	CRD_VGui_VScript
*/
isServer <- false;
IncludeScript("challenge_traitors_enums");
IncludeScript("challenge_traitors_client_shared");

FONT_DEFAULTLARGE <- self.LookupFont("DefaultLarge");
rowHeight <- self.GetFontTall(FONT_DEFAULTLARGE);
nRows <- (self.GetInt(2) / nCols + 1).tointeger();
col <- self.GetInt(0) % nCols;
row <- self.GetInt(0) / nCols;
x0 <- 0;
y0 <- 0;
x1 <- 0;
y1 <- 0;
x2 <- 0;
y2 <- 0;
x3 <- 0;
y3 <- 0;
text <- "";
b0 <- 0;
g0 <- 0;
r0 <- 0;
b1 <- 0;
g1 <- 0;
r1 <- 0;
activeTime <- 0.0;
role <- ROLE.NONE;
isMouseDown <- false;
isSkillUsed <- false;
isAlive <- true;
isSkillActive <- false;
INFECTOR_CLICK_FEEDBACK_DURATION <- 0.5;

function Paint() {
	local hMarine = self.GetEntity(0);
	//如果士兵存活且正在控制士兵，显示信息。
	if (hMarine != null && hMarine.IsValid() && self.GetInteracter() != null && hMarine == GetLocalPlayer().GetNPC()) {
		self.PaintRectangle(x0, y0, x1, y1, r0, g0, b0, 200);
		self.PaintText(x0, y0, r1, g1, b1, 255, FONT_DEFAULTLARGE, text);
		if (!isAlive) {
			self.PaintRectangle(x2, y2, x3, y3, r1, g1, b1, 255);
		}
	}
}

function Control(tbl) {
	UpdateButton();

	if (!getconsttable()["marine_info"][self.GetInt(0)].infectorIsAbeted && ((role > ROLE.MAX_IAF_TEAM && role < ROLE.MAX_TRAITOR_TEAM) || !isAlive)) {
		b1 = 50;
		g1 = 50;
		r1 = 50;
		b0 = 10;
		g0 = 10;
		r0 = 10;
		return;
	} else {
		b0 = 0;
		g0 = 0;
		r0 = 0;
		b1 = 255;
		g1 = 255;
		r1 = 255;
	}

	local isShowingClickFeedback = Time() < getconsttable()["infector_click_feedback_until"];
	if (isShowingClickFeedback) {
		isMouseDown = false;
	} else if (Time() > activeTime && isSkillActive && !isSkillUsed) {
		if (tbl.mouse_left) {
			isMouseDown = true;
		} else if (isMouseDown == true) {
			if (tbl.mouse_x > x0 && tbl.mouse_x < x1 && tbl.mouse_y > y0 && tbl.mouse_y < y1) {
				getconsttable()["infector_click_feedback_until"] = Time() + INFECTOR_CLICK_FEEDBACK_DURATION;
				self.SendInput(VGUI_ACTION.INFECTOR_ABET | self.GetInt(1));
			}
			isMouseDown = false;
		}
	}
	if (getconsttable()["marine_info"][self.GetInt(0)].infectorIsAbeted) {
		b0 = 0;
		g0 = 0;
		r0 = 255;
		b1 = 0;
		g1 = 0;
		r1 = 0;
	} else {
		if (Time() > activeTime && isSkillActive && !isSkillUsed && !isShowingClickFeedback && tbl.mouse_x > x0 && tbl.mouse_x < x1 && tbl.mouse_y > y0 && tbl.mouse_y < y1) {
			b0 = 255;
			g0 = 255;
			r0 = 255;
			b1 = 0;
			g1 = 0;
			r1 = 0;
		} else {
			b0 = 0;
			g0 = 0;
			r0 = 0;
			b1 = 255;
			g1 = 255;
			r1 = 255;
		}
	}
}

function OnUpdate() {
	rowHeight = self.GetFontTall(FONT_DEFAULTLARGE);
	nRows = (self.GetInt(2) / nCols + 1).tointeger();
	col = self.GetInt(0) % nCols;
	row = self.GetInt(0) / nCols;
	x0 = ScreenWidth() * 0.5 - colWidth * nCols / 2.0 - colSep * (nCols / 2.0 + 0.5) + (col + 1) * colSep + col * colWidth;
	y0 = ScreenHeight() * 0.4 - rowHeight * nRows / 2.0 - rowSep * (nRows / 2.0 + 0.5) + (row + 1) * rowSep + row * rowHeight;
	x1 = x0 + colWidth;
	y1 = y0 + rowHeight;
	UpdateButton();
}

function UpdateButton() {
	local idx = self.GetInt(0);
	activeTime = getconsttable()["infector_next_active_time"];
	isSkillUsed = getconsttable()["infector_is_skill_used"];
	isSkillActive = getconsttable()["infector_is_skill_active"];

	text = self.GetString(0);
	isAlive = true;

	if (("marine_info" in getconsttable()) && getconsttable()["marine_info"] != null && (idx < getconsttable()["marine_info"].len()) && getconsttable()["marine_info"][idx] != null) {
		isAlive = getconsttable()["marine_info"][idx].isAlive;
		if (isAlive) {
			text = getconsttable()["marine_info"][idx].name;
		}

	}

	text = TrimString(text, FONT_DEFAULTLARGE);
	x2 = x0;
	y2 = y0 + rowHeight / 2 - 5;
	x3 = x2 + self.GetTextWide(FONT_DEFAULTLARGE, text);
	y3 = y2 + 5;
}
