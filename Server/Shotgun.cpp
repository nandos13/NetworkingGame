#include "Shotgun.h"
#include <math.h>



Shotgun::Shotgun()
{
}

Shotgun::~Shotgun()
{
}

int Shotgun::GetRangeBonus(float distance)
{
	float distSq = distance * distance;
	return 56 - ( 4 * floor( sqrt(2 * distSq) ) );
}
