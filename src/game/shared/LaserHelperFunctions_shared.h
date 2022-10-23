#ifndef LASERHELPER_FUNCTIONS_H
#define LASERHELPER_FUNCTIONS_H

namespace LaserHelper
{
	void GetDecodedLaserColor(int laser, int& outRed, int& outGreen, int& outBlue, int& outUnused);
	int GetEncodedLaserColor(int red, int green, int blue, int unused);
}
#endif