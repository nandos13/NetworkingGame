#pragma once

#include "GunBase.h"

class StandardGun : public GunBase
{
public:
	StandardGun(unsigned int clipSize, unsigned int minDamage, unsigned int maxDamage, int aimModifier = 0, int critModifier = 0);
	virtual ~StandardGun();

	virtual int GetRangeBonus(const float distance);
};

