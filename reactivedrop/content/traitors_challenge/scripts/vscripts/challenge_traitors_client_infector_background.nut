/*参数说明
	int		2  - 总人数
	int		3  - 字符串终止1
	int		4  - 字符串终止2
	int		5  - 字符串终止3
	int		6  - 字符串终止4
	int		7  - 字符串终止5

	float	0  - 允许使用技能的开始时间

	string	0  - 文本

	entity 0 - marine

	DefaultVerySmall
	DefaultVerySmallBlur
	DefaultSmall
	DefaultSmallBlur
	DefaultSmallOutline
	Default
	DefaultBlur
	DefaultShadowed
	DefaultUnderline
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
getconsttable()["infector_next_active_time"] <- 0.0;
getconsttable()["infector_is_skill_used"] <- false;
getconsttable()["infector_is_skill_active"] <- false;
// 点击后的短暂视觉反馈，不参与服务端技能状态判定。
getconsttable()["infector_click_feedback_until"] <- 0.0;
activeTime <- 0.0;
isSkillUsed <- false;
isSkillActive <- false;
str1 <- "";
str2 <- "";
str3 <- "";
str4 <- "";
str5 <- "";
str0 <- "";

function Paint() {
	local hMarine = self.GetEntity(0);
	//如果士兵存活且正在控制士兵，显示信息。
	if (hMarine != null && hMarine.IsValid() && self.GetInteracter() != null && hMarine == GetLocalPlayer().GetNPC()) {
		self.PaintRectangle(x0, y0, x1, y1, 0, 0, 0, 200);
		self.PaintRectangle(x2, y2, x3, y3, r0, g0, b0, 200);
		self.PaintText(x2, y2, r1, g1, b1, 255, FONT_DEFAULTLARGE, str0);
	}
}

function Control(tbl) {
	activeTime = getconsttable()["infector_next_active_time"];
	isSkillUsed = getconsttable()["infector_is_skill_used"];
	isSkillActive = getconsttable()["infector_is_skill_active"];

	str0 = "";
	if (isSkillUsed == false) {
		if (Time() < activeTime) {
			str0 = str3 + (activeTime - Time() + 1).tointeger().tostring() + str4;
		} else if (!isSkillActive) {
			str0 = str2;
		} else {
			str0 = str1;
		}
	} else {
		str0 = str5;
	}
	x2 = ScreenWidth() * 0.5 - 0.5 * self.GetTextWide(FONT_DEFAULTLARGE, str0);
	y2 = y1 - rowMargin - rowHeight - rowSep;
	x3 = ScreenWidth() * 0.5 + 0.5 * self.GetTextWide(FONT_DEFAULTLARGE, str0);
	y3 = y2 + rowHeight;
	b0 = 0;
	g0 = 0;
	r0 = 0;
	b1 = 255;
	g1 = 255;
	r1 = 255;
}

function OnUpdate() {
	text = self.GetString(0);
	local a = self.GetInt(3);
	local b = a + self.GetInt(4);
	local c = b + self.GetInt(5);
	local d = c + self.GetInt(6);
	local e = d + self.GetInt(7);
	str1 = a > 0 ? text.slice(0, a) : "";
	str2 = b > a ? text.slice(a, b) : "";
	str3 = c > b ? text.slice(b, c) : "";
	str4 = d > c ? text.slice(c, d) : "";
	str5 = e > d ? text.slice(d, e) : "";
	rowHeight = self.GetFontTall(FONT_DEFAULTLARGE);
	nRows = (self.GetInt(2) / nCols + 1).tointeger();
	x0 = ScreenWidth() * 0.5 - colWidth * nCols / 2.0 - colSep * (nCols / 2.0 + 0.5) - colMargin;
	y0 = ScreenHeight() * 0.4 - rowHeight * nRows / 2.0 - rowSep * (nRows / 2.0 + 0.5) - rowMargin;
	x1 = x0 + colMargin * 2 + colWidth * nCols + colSep * (nCols + 1);
	y1 = y0 + rowMargin * 2 + rowHeight * (nRows + 1) + rowSep * (nRows + 4);
	x2 = ScreenWidth() * 0.5 - 0.5 * self.GetTextWide(FONT_DEFAULTLARGE, text);
	y2 = y1 - rowMargin - rowHeight - rowSep;
	x3 = ScreenWidth() * 0.5 + 0.5 * self.GetTextWide(FONT_DEFAULTLARGE, text);
	y3 = y2 + rowHeight;
}
