/*参数说明
	int	 0  - 士兵的角色
	int	 1  - 除了起点外，信息需要在屏幕上经过的点
	int	 2  - 行号
	int	 3  - 投降，当前人数
	int	 4  - 投降，总人数

	int	 63 - 标识，1和2代表显示的是内鬼提示1和提示2。3代表显示的是内鬼开始投票，4代表投票中，5代表投票停止

	float   0  - HUD信息显示的起始时间
	float   1  - 信息显示的起始位置在屏幕上的相对位置X，范围0-1
	float   2  - 信息显示的起始位置在屏幕上的相对位置Y，范围0-1
	float   3  - HUD信息移动到下个坐标的时间
	float   4  - 信息显示的终止位置在屏幕上的相对位置X，范围0-1
	float   5  - 信息显示的终止位置在屏幕上的相对位置Y，范围0-1
	...
	float   30 - Hud起始时间
	float   31 - HUD信息消失的时间


	entity 0 - marine
	entity 1 - TRAITOR_LEADER 对应的marine
	entity 2 - INFECTOR 对应的marine
	entity 3 - BOOMER 对应的marine
	entity 4 - SILENCER 对应的marine
	entity 5 - MIMIC 对应的marine
	entity 6 - TRAITOR 对应的marine
	...
	entity 15 - TRAITOR 对应的marine

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

IncludeScript("challenge_traitors_enums.nut");

FONT_DEFAULTLARGE <- self.LookupFont("DefaultMedium");

/*TEXTURE_TRAITOR <- self.LookupTexture("vgui/swarm/Emotes/EmoteTraitor");
TEXTURE_TRAITORS <- {
	[1] = self.LookupTexture("vgui/swarm/Emotes/EmoteTraitorLeader"),
	[2] = self.LookupTexture("vgui/swarm/Emotes/EmoteInfector"),
	[3] = self.LookupTexture("vgui/swarm/Emotes/EmoteBoomer"),
	[4] = self.LookupTexture("vgui/swarm/Emotes/EmoteSilencer"),
	[5] = self.LookupTexture("vgui/swarm/Emotes/EmoteMimic"),
	[6] = TEXTURE_TRAITOR,
	[7] = TEXTURE_TRAITOR,
	[8] = TEXTURE_TRAITOR,
	[9] = TEXTURE_TRAITOR,
	[10] = TEXTURE_TRAITOR,
	[11] = TEXTURE_TRAITOR,
	[12] = TEXTURE_TRAITOR,
	[13] = TEXTURE_TRAITOR,
	[14] = TEXTURE_TRAITOR,
	[15] = TEXTURE_TRAITOR,
};*/

xMargin <- 0;

function Paint() {
	//如果正在控制士兵，显示信息。
	if (self.GetEntity(0) == GetLocalPlayer()) {
		/*for (local i = 1; i < 16; i++) {
			PaintTraitorIcon(i);
		}*/
      
		local message = self.GetString(0);
		if (message == "") {
			return;
		}
		//根据时间和坐标显示信息。
		local time = Time() - self.GetFloat(30);
		local i = 0;
		local int1 = self.GetInt(1);
		local point = [0.0, 0.0];
		local role = self.GetInt(0);
		for (; i < int1; i++) {
			if (time > self.GetFloat(0 + 3 * i) && time <= self.GetFloat(3 + 3 * i)) {
				point = interp(i, time);
				PaintMsg(point, role, FONT_DEFAULTLARGE, message);
			}
		}
		if (time > self.GetFloat(0 + 3 * int1) && time <= self.GetFloat(31)) {
			if (self.GetInt(63) == 4) {
				message = message + " " + self.GetInt(3) + "/" + self.GetInt(4) + " (" + (self.GetFloat(31) - time).tointeger() + "s)";
			}
			point = [ScreenPosX(self.GetFloat(1 + 3 * int1)), ScreenPosY(self.GetFloat(2 + 3 * int1))];
			PaintMsg(point, role, FONT_DEFAULTLARGE, message);
		}
	}
}

function PaintMsg(point, role, font, message) {
	local line = self.GetInt(2);
	local textHalfWidth = 0.5 * self.GetTextWide(font, message);
	SetMargin();
	local alpha = 255;
	local xOffset = point[0] >= textHalfWidth + 20 ? 0 : textHalfWidth + 20 - point[0];
	point[0] += xOffset;
	if (role <= ROLE.MAX_IAF_TEAM) {
		self.PaintText(point[0] - textHalfWidth, point[1] + line * (self.GetFontTall(font) + 10), 255, 255, 255, 240, font, message);
		self.PaintRectangle(point[0] - textHalfWidth - 20, point[1] - 5 + line * (self.GetFontTall(font) + 10), point[0] + 20 + textHalfWidth, point[1] - 5 + (line + 1) * (self.GetFontTall(font) + 10), 0, 70, 0, 150);
	} else if (role > ROLE.MAX_IAF_TEAM && role <= ROLE.MAX_TRAITOR_TEAM) {
		self.PaintText(point[0] - textHalfWidth, point[1] + line * (self.GetFontTall(font) + 10), 255, 255, 255, 200, font, message);
		self.PaintRectangle(point[0] - textHalfWidth - 20, point[1] - 5 + line * (self.GetFontTall(font) + 10), point[0] + 20 + textHalfWidth, point[1] - 5 + (line + 1) * (self.GetFontTall(font) + 10), 70, 0, 0, 150);
	} else {
		self.PaintText(point[0] - textHalfWidth, point[1] + line * (self.GetFontTall(font) + 10), 255, 255, 255, 200, font, message);
		self.PaintRectangle(point[0] - textHalfWidth - 20, point[1] - 5 + line * (self.GetFontTall(font) + 10), point[0] + 20 + textHalfWidth, point[1] - 5 + (line + 1) * (self.GetFontTall(font) + 10), 0, 0, 0, 150);
	}

}

function PaintTraitorIcon(idx) {

	if (!self.GetEntity(idx) || TEXTURE_TRAITORS[idx] == -1 || self.GetInt(0) <= ROLE.MAX_IAF_TEAM || self.GetInt(0) > ROLE.MAX_TRAITOR_TEAM) {
		return;
	}
	local screenPos = self.ClientGetEntityScreenPos(self.GetEntity(idx), true);
	if (!screenPos) {
		return;
	}

	local xPos = screenPos.x;
	local yPos = screenPos.y;

	local fScale = (ScreenHeight() / 768.0) * 0.4;
	local HalfW = 128.0 * fScale * 0.5;
	local HalfH = 128.0 * fScale * 0.5;
	yPos += 100 * (ScreenHeight() / 768.0);

	local points = [
		{x = xPos - HalfW, y = yPos - HalfH, s = 0, t = 0},
		{x = xPos + HalfW, y = yPos - HalfH, s = 1, t = 0},
		{x = xPos + HalfW, y = yPos + HalfH, s = 1, t = 1},
		{x = xPos - HalfW, y = yPos + HalfH, s = 0, t = 1}
	];
	self.PaintPolygon(points, 255, 255, 255, 255, TEXTURE_TRAITORS[idx]);
}

function interp(i, t) {
	local t0 = self.GetFloat(0 + 3 * i);
	local t1 = self.GetFloat(3 + 3 * i);
	local x0 = ScreenPosX(self.GetFloat(1 + 3 * i));
	local x1 = ScreenPosX(self.GetFloat(4 + 3 * i));
	local y0 = ScreenPosY(self.GetFloat(2 + 3 * i));
	local y1 = ScreenPosY(self.GetFloat(5 + 3 * i));
	local point = [0.0, 0.0];
	point[0] = x0 + (x1 - x0) / (t1 - t0) * (t - t0);
	point[1] = y0 + (y1 - y0) / (t1 - t0) * (t - t0);
	return point;
}

function SetMargin() {
	if (ScreenWidth().tofloat() / ScreenHeight().tofloat() > 16.0 / 9.0) {
		xMargin = ((ScreenWidth().tofloat() - ScreenHeight().tofloat() * 16.0 / 9.0) / 2.0).tointeger();
	}
}

// pass in a value from 0.0 to 1.0, 0.0 means the left side of the screen, 1.0 means the right, 0.5 in the middle
function ScreenPosX(fraction) {
	return xMargin + (ScreenWidth() - 2 * xMargin) * fraction;
}

// pass in a value from 0.0 to 1.0, 0.0 means the up side of the screen, 1.0 means the down, 0.5 in the middle
function ScreenPosY(fraction) {
	return ScreenHeight() * fraction;
}