#include "cbase.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

namespace LaserHelper
{
	char* g_LaserSightStyleParticles[] =
	{
		"laser_rgb_main",
		"laser_rgb_main_style2",
		"laser_rgb_main_style3",
		"laser_rgb_main_style4",
	};

	constexpr int CLAMP_BYTERANGE(int value)
	{
		return value < 0 ? 0 : value > 255 ? 255 : value;
	}
	constexpr int CLAMP_NIBBLERANGE(int value)
	{
		return value < 0 ? 0 : value > 15 ? 15 : value;
	}
	void GetDecodedLaserColor(int laser, int& outRed, int& outGreen, int& outBlue, int& outLaserStyle, int& outLaserSize)
	{
		outRed = laser & 0xFF;
		outGreen = (laser >> 8) & 0xFF;
		outBlue = (laser >> 16) & 0xFF;
		outLaserStyle = (laser >> 24) & 0xF;
		outLaserSize = (laser >> 28) & 0xF;
	}
	int GetEncodedLaserColor(int red, int green, int blue, int laserStyle, int laserSize)
	{
		red = CLAMP_BYTERANGE(red);
		green = CLAMP_BYTERANGE(green);
		blue = CLAMP_BYTERANGE(blue);
		laserStyle = CLAMP_NIBBLERANGE(laserStyle);
		laserSize = CLAMP_NIBBLERANGE(laserSize);
		return red | green << 8 | blue << 16 | laserStyle << 24 | laserSize << 28;
	}
	void LaserToConvarString(int laser, char* outBuffer)
	{
		int outRed, outGreen, outBlue, outLaserStyle, outLaserSize;
		GetDecodedLaserColor(laser, outRed, outGreen, outBlue, outLaserStyle, outLaserSize);
		Q_snprintf(outBuffer, sizeof(outBuffer), "%d %d %d %d %d");
	}

	int ConvarStringToLaser(char* szLSValue)
	{
		if (szLSValue)
		{
			CUtlStringList szSplitFloats;
			V_SplitString(szLSValue, " ", szSplitFloats);

			if (szSplitFloats.Count() >= 3)
			{
					int laserStyle = 0, laserSize = 0;
					if (szSplitFloats.Count() >= 4)
					{
						laserStyle = atof(szSplitFloats[3]);
						if (szSplitFloats.Count() >= 5)
						{
							laserSize = atof(szSplitFloats[4]);
						}
					}
					return LaserHelper::GetEncodedLaserColor(atof(szSplitFloats[0]), atof(szSplitFloats[1]), atof(szSplitFloats[2]), laserStyle, laserSize);
			}
		}
		return LaserHelper::GetEncodedLaserColor(255, 0, 0, 0, 0);
	}

	void SetLaserConvar(ConVar* convar, int Red, int Green, int Blue, int Style, int Size)
	{
		char buffer[256]{};
		Q_snprintf(buffer, sizeof(buffer), "%d %d %d %d %d", Red, Green, Blue, Style, Size);
		convar->SetValue(buffer);
	}

	void SplitLaserConvar(const char* szLSValue, int& outRed, int& outGreen, int& outBlue, int& outStyle, int& outSize)
	{
		if (szLSValue)
		{
			CUtlStringList szSplitFloats;
			V_SplitString(szLSValue, " ", szSplitFloats);

			if (szSplitFloats.Count() >= 3)
			{
				outRed = atof(szSplitFloats[0]);
				outGreen = atof(szSplitFloats[1]);
				outBlue = atof(szSplitFloats[2]);

				if (szSplitFloats.Count() >= 4)
				{
					outStyle = atof(szSplitFloats[3]);
					if (szSplitFloats.Count() >= 5)
					{
						outSize = atof(szSplitFloats[4]);
					}
					else
					{
						outSize = 0;
					}
				}
				else
				{
					outStyle = 0;
					outSize = 0;
				}
				return;
			}
		}
		outRed = 255;
		outGreen = 0;
		outBlue = 0;
		outStyle = 0;
		outSize = 0;
	}

	void SplitLaserConvar(ConVar* convar, int& outRed, int& outGreen, int& outBlue, int& outStyle, int& outSize)
	{
		const char* szLSValue = convar->GetString();
		SplitLaserConvar(szLSValue, outRed, outGreen, outBlue, outStyle, outSize);
	}

	int ConvarToLaser(ConVar* convar)
	{
		int outRed = 0, outGreen = 0, outBlue = 0, outStyle = 0, outSize = 0;
		SplitLaserConvar(convar, outRed, outGreen, outBlue, outStyle, outSize);
		return GetEncodedLaserColor(outRed, outGreen, outBlue, outStyle, outSize);
	}

	char* GetParticleStyle(int index)
	{
		if (index > 3 || index < 0)
			return g_LaserSightStyleParticles[0];
		return g_LaserSightStyleParticles[index];
	}
}