#ifndef LASERHELPER_FUNCTIONS_H
#define LASERHELPER_FUNCTIONS_H

namespace LaserHelper
{
	extern char* g_LaserSightStyleParticles[3];
	void GetDecodedLaserColor(int laser, int& outRed, int& outGreen, int& outBlue, int& outLaserStyle, int& outLaserSize);
	int GetEncodedLaserColor(int red, int green, int blue, int laserStyle, int laserSize);
	void LaserToConvarString(int laser, char* outBuffer);
	int ConvarStringToLaser(char* szLSValue);
	void SplitLaserConvar(ConVar* convar, int& outRed, int& outGreen, int& outBlue, int& outStyle, int& outSize);
	void SplitLaserConvar(const char* szLSValue, int& outRed, int& outGreen, int& outBlue, int& outStyle, int& outSize);
	void SetLaserConvar(ConVar* convar, int Red, int Green, int Blue, int Style, int Size);
	char* GetParticleStyle(int index);
	int ConvarToLaser(ConVar* convar);
}
#endif