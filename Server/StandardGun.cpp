#include "StandardGun.h"



StandardGun::StandardGun(unsigned int clipSize, unsigned int minDamage, unsigned int maxDamage, int aimModifier, int critModifier)
	: GunBase(clipSize, minDamage, maxDamage, aimModifier, critModifier) {
	m_gunType = 3;
}

StandardGun::~StandardGun()
{
}

int StandardGun::GetRangeBonus(const float distance) const
{
	float bonus = (float)(42 - (4.5f * distance));
	return (bonus > 0) ? (int)bonus : 0;
}
