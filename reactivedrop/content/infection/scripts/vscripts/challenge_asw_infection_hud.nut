FONT_DEFAULT <- self.LookupFont( "Default" );
FONT_DEFAULTLARGE <- self.LookupFont( "DefaultLarge" );
FONT_DEFAULTEXTRALARGE <- self.LookupFont( "DefaultExtraLarge" );
FONT_COUNTDOWN <- self.LookupFont( "Countdown" );
FONT_COUNTDOWNSMALL <- self.LookupFont( "CountdownSmall" );
FONT_COUNTDOWNBLUR <- self.LookupFont( "CountdownBlur" );

function Paint()
{
	if ( self.GetEntity(0) != GetLocalPlayer() )
	{
		return;
	}

	local sec = self.GetInt(0);
	local min = self.GetInt(1);
	local win = self.GetInt(2);
	local humanCount = self.GetInt(3);
	local zombieCount = self.GetInt(4);
	local virus = self.GetInt(5);

	if (win > 0)
	{
		local winText = TryLocalize("#asw_infection_win_zombie_hud");
		local winColor = [200, 0, 0];
		local winColorAlt = [100, 0, 0];
		if (win == 2)
		{
			winText = TryLocalize("#asw_infection_win_human_hud");
			winColor = [0, 100, 200];
			winColorAlt = [0, 50, 100];
		}
		local winTextWide = self.GetTextWide(FONT_COUNTDOWN, winText);
		self.PaintRectangle
		(
			XRes( 312 ) - winTextWide*0.5,
			YRes( 144 ) - XRes( 8 ),
			XRes( 328 ) + winTextWide*0.5,
			YRes( 144 ) + XRes( 8 ) + self.GetFontTall(FONT_COUNTDOWN),
			200, 200, 200, 255
		);
		self.PaintRectangleFade
		(
			XRes( 312 ) - winTextWide*0.5,
			YRes( 144 ) - XRes( 8 ),
			XRes( 328 ) + winTextWide*0.5,
			YRes( 144 ) + XRes( 8 ) + self.GetFontTall(FONT_COUNTDOWN),
			0, 0, 0, 0, 255,
			YRes( 144 ) - XRes( 8 ),
			YRes( 144 ) + XRes( 8 ) + self.GetFontTall(FONT_COUNTDOWN),
			false
		);
		self.PaintRectangle
		(
			XRes( 313.6 ) - winTextWide*0.5,
			YRes( 144 ) - XRes( 6.4 ),
			XRes( 326.4 ) + winTextWide*0.5,
			YRes( 144 ) + XRes( 6.4 ) + self.GetFontTall(FONT_COUNTDOWN),
			winColorAlt[0], winColorAlt[1], winColorAlt[2], 255
		);
		self.PaintRectangleFade
		(
			XRes( 313.6 ) - winTextWide*0.5,
			YRes( 144 ) - XRes( 6.4 ),
			XRes( 326.4 ) + winTextWide*0.5,
			YRes( 144 ) + XRes( 6.4 ) + self.GetFontTall(FONT_COUNTDOWN),
			winColor[0], winColor[1], winColor[2], 255, 0,
			YRes( 144 ) - XRes( 6.4 ),
			YRes( 144 ) + XRes( 6.4 ) + self.GetFontTall(FONT_COUNTDOWN),
			false
		);
		PaintScanlineText(XRes(320), YRes(145), 255, 255, 255, 255, winText);
	}
	else
	{
		if (virus > 0)
		{
			local virusText = TryLocalize("#asw_infection_starts_in");
			virusText = virusText.slice(0, virusText.find("%s1")) + virus.tostring() + virusText.slice(virusText.find("%s1") + 3);
			local virusTextWide = self.GetTextWide(FONT_DEFAULT, virusText);
			self.PaintRectangle
			(
				XRes( 318.4 ) - virusTextWide*0.5,
				YRes( 336 ) - XRes( 1.6 ),
				XRes( 321.6 ) + virusTextWide*0.5,
				YRes( 336 ) + XRes( 1.6 ) + self.GetFontTall(FONT_DEFAULT),
				0, 0, 0, 200
			);
			self.PaintRectangleFade
			(
				XRes( 286.4 ) - virusTextWide*0.5,
				YRes( 336 ) - XRes( 1.6 ),
				XRes( 318.4 ) - virusTextWide*0.5,
				YRes( 336 ) + XRes( 1.6 ) + self.GetFontTall(FONT_DEFAULT),
				0, 0, 0, 0, 200,
				XRes( 286.4 ) - virusTextWide*0.5,
				XRes( 318.4 ) - virusTextWide*0.5,
				true
			);
			self.PaintRectangleFade
			(
				XRes( 321.6 ) + virusTextWide*0.5,
				YRes( 336 ) - XRes( 1.6 ),
				XRes( 353.6 ) + virusTextWide*0.5,
				YRes( 336 ) + XRes( 1.6 ) + self.GetFontTall(FONT_DEFAULT),
				0, 0, 0, 225, 0,
				XRes( 321.6 ) + virusTextWide*0.5,
				XRes( 353.6 ) + virusTextWide*0.5,
				true
			);
			self.PaintText(
				XRes( 320 ) - virusTextWide*0.5,
				YRes( 336 ),
				255, 200, 0, 255,
				FONT_DEFAULT,
				virusText
			);
		}
		else
		{
			if (humanCount > 0)
			{
				local minText = "";
				if (min < 10)
				{
					minText = "0";
				}
				minText = minText + min.tostring();
				local secText = "";
				if (sec < 10)
				{
					secText = "0";
				}
				secText = secText + sec.tostring();
				local timeText = minText + ":" + secText;
				local zombieText = zombieCount.tostring();
				local humanText = humanCount.tostring();
				local timeTextWide = self.GetTextWide(FONT_COUNTDOWNBLUR, timeText);
				local zombieTextWide = self.GetTextWide(FONT_COUNTDOWNBLUR, zombieText);
				local humanTextWide = self.GetTextWide(FONT_COUNTDOWNBLUR, humanText);
				local textTall = self.GetFontTall(FONT_COUNTDOWNBLUR);
				self.PaintRectangle
				(
					XRes( 299.2 ) - timeTextWide*0.5 - zombieTextWide,
					YRes( 96 ) - textTall - XRes( 6.4 ),
					XRes( 340.8 ) + timeTextWide*0.5 + humanTextWide,
					YRes( 96 ) + XRes( 6.4 ),
					0, 0, 0, 100
				);
				self.PaintRectangle
				(
					XRes( 315.2 ) - timeTextWide*0.5,
					YRes( 96 ) - textTall - XRes( 3.2 ),
					XRes( 324.8 ) + timeTextWide*0.5,
					YRes( 96 ) + XRes( 3.2 ),
					50, 50, 50, 200
				);
				self.PaintRectangle
				(
					XRes( 302.4 ) - timeTextWide*0.5 - zombieTextWide,
					YRes( 96 ) - textTall - XRes( 3.2 ),
					XRes( 312 ) - timeTextWide*0.5,
					YRes( 96 ) + XRes( 3.2 ),
					50, 0, 0, 200
				);
				self.PaintRectangle
				(
					XRes( 328 ) + timeTextWide*0.5,
					YRes( 96 ) - textTall - XRes( 3.2 ),
					XRes( 337.6 ) + timeTextWide*0.5 + humanTextWide,
					YRes( 96 ) + XRes( 3.2 ),
					0, 50, 50, 200
				);
				PaintScanlineText(XRes(320), YRes(96) - textTall, 255, 255, 255, 255, timeText);
				PaintScanlineText(XRes(307.2) - timeTextWide * 0.5 - zombieTextWide * 0.5, YRes(96) - textTall, 255, 225, 225, 255, zombieText);
				PaintScanlineText(XRes(332.8) + timeTextWide * 0.5 + humanTextWide * 0.5, YRes(96) - textTall, 225, 255, 255, 255, humanText);
			}

			local teamIndex = self.GetInt(6);
			if (teamIndex == 2)
			{
				local rage = self.GetFloat(0);
				local rageText = TryLocalize("#asw_infection_rage_meter");
				local rageMeter = "";
				for (local i = 0; i < (10*rage).tointeger(); i++)
				{
					rageMeter = rageMeter + "█";
				}
				for (local i = 0; i < 10-(10*rage).tointeger(); i++)
				{
					rageMeter = rageMeter + "  ";
				}
				rageText = rageText.slice(0, rageText.find("%s1")) + rageMeter + rageText.slice(rageText.find("%s1") + 3);
				rageText = rageText.slice(0, rageText.find("%s2")) + (100*rage).tointeger().tostring() + rageText.slice(rageText.find("%s2") + 3);
				PaintScanlineText(XRes(320), YRes(384), 255, 0, (255*rage).tointeger(), 200, rageText);
			}
			else if (teamIndex == 1)
			{
				local shieldMax = self.GetFloat(1);
				if (shieldMax > 0)
				{
					local shield = self.GetFloat(0);
					local a = 255;
					if (shield <= 0)
					{
						a = 125;
					}
					self.PaintRectangle
					(
						XRes( 303.36 ),
						YRes( 312 ) - XRes( 0.64 ),
						XRes( 336.64 ),
						YRes( 314.4 ) + XRes( 0.64 ),
						0, 0, 0, a
					);
					self.PaintRectangle
					(
						XRes( 304 ),
						YRes( 312 ),
						XRes( 304 ) + XRes( 32 * (shield/shieldMax) ),
						YRes( 314.4 ),
						200, 255, 255, 255
					);
				}
				local dist = self.GetInt(7);
				if (dist >= 0)
				{
					local distText = "⌕ " + dist;
					PaintScanlineText(XRes( 320 ), YRes( 396 ), 0, 255, 0, 200, distText);
				}
			}
		}
	}

	local creditScript = "Scripter: ModdedMarionette";
	local creditModel = "Modeler: Beka";
	self.PaintText(
		XRes( 569.6 ),
		YRes( 384 ) - self.GetFontTall(FONT_DEFAULT)*2,
		255, 255, 255, 255,
		FONT_DEFAULT,
		creditScript
	);
	self.PaintText(
		XRes( 569.6 ),
		YRes( 384.96 ) - self.GetFontTall(FONT_DEFAULT),
		255, 255, 255, 255,
		FONT_DEFAULT,
		creditModel
	);
}

function PaintScanlineText(x, y, r, g, b, a, text) {
	local wide = self.GetTextWide(FONT_COUNTDOWNBLUR, text);
	self.PaintText(
		x - wide * 0.5, y,
		r, g, b, a,
		FONT_COUNTDOWNBLUR,
		text
	);
	self.PaintText(
		x - wide * 0.5, y,
		r, g, b, a,
		FONT_COUNTDOWNSMALL,
		text
	);
}
