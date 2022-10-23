#include "cbase.h"

namespace LaserHelper
{
	constexpr int CLAMP_BYTERANGE(int value)
	{
		return value < 0 ? 0 : value > 255 ? 255 : value;
	}
	void GetDecodedLaserColor(int laser, int& outRed, int& outGreen, int& outBlue, int& outUnused)
	{
		outRed = laser & 0xFF;
		outGreen = (laser >> 8) & 0xFF;
		outBlue = (laser >> 16) & 0xFF;
		outUnused = (laser >> 24) & 0xFF;
	}
	int GetEncodedLaserColor(int red, int green, int blue, int unused)
	{
		//Remember that color channels need to be ranged 0 - 255
		red = CLAMP_BYTERANGE(red);
		green = CLAMP_BYTERANGE(green);
		blue = CLAMP_BYTERANGE(blue);
		unused = CLAMP_BYTERANGE(unused);
		return red | green << 8 | blue << 16 | unused << 24;
	}
}