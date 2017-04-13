#include "StandardGun.h"



StandardGun::StandardGun()
{
}

StandardGun::~StandardGun()
{
}

int StandardGun::GetRangeBonus(float distance)
{
	float bonus = (float)42 - 4.5 * distance;
	return (bonus > 0) ? (int)bonus : 0;
}
