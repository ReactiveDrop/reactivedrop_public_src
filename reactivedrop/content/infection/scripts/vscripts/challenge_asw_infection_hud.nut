FONT_DEFAULT <- self.LookupFont( "Default" );
FONT_DEFAULTLARGE <- self.LookupFont( "DefaultLarge" );
FONT_DEFAULTEXTRALARGE <- self.LookupFont( "DefaultExtraLarge" );
FONT_COUNTDOWN <- self.LookupFont( "Countdown" );
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
		local winText = "Zombies Win";
		local winColor = [200, 0, 0];
		local winColorAlt = [100, 0, 0];
		if (win == 2)
		{
			winText = "Humans Win";
			winColor = [0, 100, 200];
			winColorAlt = [0, 50, 100];
		}
		self.PaintRectangle
		(
			ScreenPosX( 0.4875 ) - self.GetTextWide(FONT_COUNTDOWN, winText)*0.5,
			ScreenPosY( 0.3000 ) - ScreenPosX( 0.0125 ),
			ScreenPosX( 0.5125 ) + self.GetTextWide(FONT_COUNTDOWN, winText)*0.5,
			ScreenPosY( 0.3000 ) + ScreenPosX( 0.0125 ) + self.GetFontTall(FONT_COUNTDOWN),
			200, 200, 200, 255
		);
		self.PaintRectangleFade
		(
			ScreenPosX( 0.4875 ) - self.GetTextWide(FONT_COUNTDOWN, winText)*0.5,
			ScreenPosY( 0.3000 ) - ScreenPosX( 0.0125 ),
			ScreenPosX( 0.5125 ) + self.GetTextWide(FONT_COUNTDOWN, winText)*0.5,
			ScreenPosY( 0.3000 ) + ScreenPosX( 0.0125 ) + self.GetFontTall(FONT_COUNTDOWN),
			0, 0, 0, 0, 255,
			ScreenPosY( 0.3000 ) - ScreenPosX( 0.0125 ),
			ScreenPosY( 0.3000 ) + ScreenPosX( 0.0125 ) + self.GetFontTall(FONT_COUNTDOWN),
			false
		);
		self.PaintRectangle
		(
			ScreenPosX( 0.4900 ) - self.GetTextWide(FONT_COUNTDOWN, winText)*0.5,
			ScreenPosY( 0.3000 ) - ScreenPosX( 0.0100 ),
			ScreenPosX( 0.5100 ) + self.GetTextWide(FONT_COUNTDOWN, winText)*0.5,
			ScreenPosY( 0.3000 ) + ScreenPosX( 0.0100 ) + self.GetFontTall(FONT_COUNTDOWN),
			winColorAlt[0], winColorAlt[1], winColorAlt[2], 255
		);
		self.PaintRectangleFade
		(
			ScreenPosX( 0.4900 ) - self.GetTextWide(FONT_COUNTDOWN, winText)*0.5,
			ScreenPosY( 0.3000 ) - ScreenPosX( 0.0100 ),
			ScreenPosX( 0.5100 ) + self.GetTextWide(FONT_COUNTDOWN, winText)*0.5,
			ScreenPosY( 0.3000 ) + ScreenPosX( 0.0100 ) + self.GetFontTall(FONT_COUNTDOWN),
			winColor[0], winColor[1], winColor[2], 255, 0,
			ScreenPosY( 0.3000 ) - ScreenPosX( 0.0100 ),
			ScreenPosY( 0.3000 ) + ScreenPosX( 0.0100 ) + self.GetFontTall(FONT_COUNTDOWN),
			false
		);
		self.PaintText(
			ScreenPosX( 0.5000 ) - self.GetTextWide(FONT_COUNTDOWN, winText)*0.5,
			ScreenPosY( 0.3000 ),
			255, 255, 255, 255,
			FONT_COUNTDOWN,
			winText
		);
	}
	else
	{
		if (virus > 0)
		{
			local virusText = "First zombie appears in: " + virus.tostring();
			self.PaintRectangle
			(
				ScreenPosX( 0.4975 ) - self.GetTextWide(FONT_DEFAULT, virusText)*0.5,
				ScreenPosY( 0.7000 ) - ScreenPosX( 0.0025 ),
				ScreenPosX( 0.5025 ) + self.GetTextWide(FONT_DEFAULT, virusText)*0.5,
				ScreenPosY( 0.7000 ) + ScreenPosX( 0.0025 ) + self.GetFontTall(FONT_DEFAULT),
				0, 0, 0, 200
			);
			self.PaintRectangleFade
			(
				ScreenPosX( 0.4475 ) - self.GetTextWide(FONT_DEFAULT, virusText)*0.5,
				ScreenPosY( 0.7000 ) - ScreenPosX( 0.0025 ),
				ScreenPosX( 0.4975 ) - self.GetTextWide(FONT_DEFAULT, virusText)*0.5,
				ScreenPosY( 0.7000 ) + ScreenPosX( 0.0025 ) + self.GetFontTall(FONT_DEFAULT),
				0, 0, 0, 0, 200,
				ScreenPosX( 0.4475 ) - self.GetTextWide(FONT_DEFAULT, virusText)*0.5,
				ScreenPosX( 0.4975 ) - self.GetTextWide(FONT_DEFAULT, virusText)*0.5,
				true
			);
			self.PaintRectangleFade
			(
				ScreenPosX( 0.5025 ) + self.GetTextWide(FONT_DEFAULT, virusText)*0.5,
				ScreenPosY( 0.7000 ) - ScreenPosX( 0.0025 ),
				ScreenPosX( 0.5525 ) + self.GetTextWide(FONT_DEFAULT, virusText)*0.5,
				ScreenPosY( 0.7000 ) + ScreenPosX( 0.0025 ) + self.GetFontTall(FONT_DEFAULT),
				0, 0, 0, 225, 0,
				ScreenPosX( 0.5025 ) + self.GetTextWide(FONT_DEFAULT, virusText)*0.5,
				ScreenPosX( 0.5525 ) + self.GetTextWide(FONT_DEFAULT, virusText)*0.5,
				true
			);
			self.PaintText(
				ScreenPosX( 0.5000 ) - self.GetTextWide(FONT_DEFAULT, virusText)*0.5,
				ScreenPosY( 0.7000 ),
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
				self.PaintRectangle
				(
					ScreenPosX( 0.4675 ) - self.GetTextWide(FONT_COUNTDOWNBLUR, timeText)*0.5 - self.GetTextWide(FONT_COUNTDOWNBLUR, zombieText),
					ScreenPosY( 0.2000 ) - self.GetFontTall(FONT_COUNTDOWNBLUR) - ScreenPosX( 0.0100 ),
					ScreenPosX( 0.5325 ) + self.GetTextWide(FONT_COUNTDOWNBLUR, timeText)*0.5 + self.GetTextWide(FONT_COUNTDOWNBLUR, humanText),
					ScreenPosY( 0.2000 ) + ScreenPosX( 0.0100 ),
					0, 0, 0, 100
				);
				self.PaintRectangle
				(
					ScreenPosX( 0.4925 ) - self.GetTextWide(FONT_COUNTDOWNBLUR, timeText)*0.5,
					ScreenPosY( 0.2000 ) - self.GetFontTall(FONT_COUNTDOWNBLUR) - ScreenPosX( 0.0050 ),
					ScreenPosX( 0.5075 ) + self.GetTextWide(FONT_COUNTDOWNBLUR, timeText)*0.5,
					ScreenPosY( 0.2000 ) + ScreenPosX( 0.0050 ),
					50, 50, 50, 200
				);
				self.PaintRectangle
				(
					ScreenPosX( 0.4725 ) - self.GetTextWide(FONT_COUNTDOWNBLUR, timeText)*0.5 - self.GetTextWide(FONT_COUNTDOWNBLUR, zombieText),
					ScreenPosY( 0.2000 ) - self.GetFontTall(FONT_COUNTDOWNBLUR) - ScreenPosX( 0.0050 ),
					ScreenPosX( 0.4875 ) - self.GetTextWide(FONT_COUNTDOWNBLUR, timeText)*0.5,
					ScreenPosY( 0.2000 ) + ScreenPosX( 0.0050 ),
					50, 0, 0, 200
				);
				self.PaintRectangle
				(
					ScreenPosX( 0.5125 ) + self.GetTextWide(FONT_COUNTDOWNBLUR, timeText)*0.5,
					ScreenPosY( 0.2000 ) - self.GetFontTall(FONT_COUNTDOWNBLUR) - ScreenPosX( 0.0050 ),
					ScreenPosX( 0.5275 ) + self.GetTextWide(FONT_COUNTDOWNBLUR, timeText)*0.5 + self.GetTextWide(FONT_COUNTDOWNBLUR, humanText),
					ScreenPosY( 0.2000 ) + ScreenPosX( 0.0050 ),
					0, 50, 50, 200
				);
				for (local i = 0; i < 2; i++)
				{
					self.PaintText(
						ScreenPosX( 0.5000 ) - self.GetTextWide(FONT_COUNTDOWNBLUR, timeText)*0.5,
						ScreenPosY( 0.2000 ) - self.GetFontTall(FONT_COUNTDOWNBLUR),
						255, 255, 255, 255,
						FONT_COUNTDOWNBLUR,
						timeText
					);
					self.PaintText(
						ScreenPosX( 0.4800 ) - self.GetTextWide(FONT_COUNTDOWNBLUR, timeText)*0.5 - self.GetTextWide(FONT_COUNTDOWNBLUR, zombieText),
						ScreenPosY( 0.2000 ) - self.GetFontTall(FONT_COUNTDOWNBLUR),
						255, 225, 225, 255,
						FONT_COUNTDOWNBLUR,
						zombieText
					);
					self.PaintText(
						ScreenPosX( 0.5200 ) + self.GetTextWide(FONT_COUNTDOWNBLUR, timeText)*0.5,
						ScreenPosY( 0.2000 ) - self.GetFontTall(FONT_COUNTDOWNBLUR),
						225, 255, 255, 255,
						FONT_COUNTDOWNBLUR,
						humanText
					);
				}
			}

			local teamIndex = self.GetInt(6);
			if (teamIndex == 2)
			{
				local rage = self.GetFloat(0);
				local rageText = "Rage: ";
				for (local i = 0; i < (10*rage).tointeger(); i++)
				{
					rageText = rageText + "█";
				}
				for (local i = 0; i < 10-(10*rage).tointeger(); i++)
				{
					rageText = rageText + "  ";
				}
				rageText = rageText + " " + (100*rage).tointeger().tostring() + "%";
				for (local i = 0; i < 2; i++)
				{
					self.PaintText(
						ScreenPosX( 0.5000 ) - self.GetTextWide(FONT_COUNTDOWNBLUR, rageText)*0.5,
						ScreenPosY( 0.8000 ),
						255, 0, (255*rage).tointeger(), 255,
						FONT_COUNTDOWNBLUR,
						rageText
					);
				}
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
						ScreenPosX( 0.4740 ),
						ScreenPosY( 0.6500 ) - ScreenPosX( 0.0010 ),
						ScreenPosX( 0.5260 ),
						ScreenPosY( 0.6550 ) + ScreenPosX( 0.0010 ),
						0, 0, 0, a
					);
					self.PaintRectangle
					(
						ScreenPosX( 0.4750 ),
						ScreenPosY( 0.6500 ),
						ScreenPosX( 0.4750 ) + ScreenPosX( 0.0500 * (shield/shieldMax) ),
						ScreenPosY( 0.6550 ),
						200, 255, 255, 255
					);
					local dist = self.GetInt(7);
					local distText = "⌕ " + dist;
					for (local i = 0; i < 2; i++)
					{
						self.PaintText(
							ScreenPosX( 0.5000 ) - self.GetTextWide(FONT_COUNTDOWNBLUR, distText)*0.5,
							ScreenPosY( 0.9000 ),
							0, 255, 0, 255,
							FONT_COUNTDOWNBLUR,
							distText
						);
					}
				}
			}
		}
	}

	local creditScript = "Scripter: ModdedMarionette";
	local creditModel = "Modeler: Beka";
	self.PaintText(
		ScreenPosX( 0.8900 ),
		ScreenPosY( 0.8000 ) - self.GetFontTall(FONT_DEFAULT)*2,
		255, 255, 255, 255,
		FONT_DEFAULT,
		creditScript
	);
	self.PaintText(
		ScreenPosX( 0.8900 ),
		ScreenPosY( 0.8020 ) - self.GetFontTall(FONT_DEFAULT),
		255, 255, 255, 255,
		FONT_DEFAULT,
		creditModel
	);
}

function ScreenPosX( fFraction )
{
	return ScreenWidth() * fFraction;
}

function ScreenPosY( fFraction )
{
	return ScreenHeight() * fFraction;
}
