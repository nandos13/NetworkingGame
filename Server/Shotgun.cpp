#include "Shotgun.h"
#include <math.h>



Shotgun::Shotgun(unsigned int clipSize, unsigned int minDamage, unsigned int maxDamage, int aimModifier, int critModifier) 
	: GunBase(clipSize, minDamage, maxDamage, aimModifier, critModifier)
{
}

Shotgun::~Shotgun()
{
}

int Shotgun::GetRangeBonus(const float distance) const
{
	float distSq = distance * distance;
	return 56 - ( 4 * floor( sqrt(2 * distSq) ) );
}
