#include "StandardGun.h"



StandardGun::StandardGun(unsigned int clipSize, unsigned int minDamage, unsigned int maxDamage, int aimModifier, int critModifier)
	: GunBase(clipSize, minDamage, maxDamage, aimModifier, critModifier)
{
}

StandardGun::~StandardGun()
{
}

int StandardGun::GetRangeBonus(const float distance)
{
	float bonus = (float)42 - 4.5 * distance;
	return (bonus > 0) ? (int)bonus : 0;
}
