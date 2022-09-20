#pragma once
//These can be values anywhere from 0x20 to 0x7E - BLEND_NONE is an exception as it isn't inserted anywhere.
enum CHATCOLORBLENDMODE
{
	BLEND_NONE = 0,				//This can't be used as ASCII code
	BLEND_NORMAL = 32,				//Normal 2 color blend mode
	BLEND_INVERT = 33,		//Inverts color blend as it reaches tail end.
	BLEND_3COLOR = 34,		//Blend with 3 colors
	BLEND_CYCLE = 35,		//Blend with 2 colors that repeats at fixed length rather than by text length.
	BLEND_SMOOTHCYCLE = 36 //Blend with 2 colors that repeats blending with smooth return.
};

//This is moved here so the consistent enum can be loaded into both client and server
enum TextColor
{
	COLOR_NORMAL = 1,
	COLOR_USEOLDCOLORS = 2,
	COLOR_PLAYERNAME = 3,
	COLOR_LOCATION = 4,
	COLOR_ACHIEVEMENT = 5,
	COLOR_MOD_CUSTOM = 6,
	COLOR_MOD_CUSTOM2 = 7,
	COLOR_INPUTCUSTOMCOL = 8,
	COLOR_MAX
};

