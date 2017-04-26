#pragma once

#include "GunBase.h"

class SniperRifle : public GunBase
{
public:
	SniperRifle(unsigned int clipSize, unsigned int minDamage, unsigned int maxDamage, int aimModifier = 0, int critModifier = 0);
	virtual ~SniperRifle();

	virtual int GetRangeBonus(float distance);
};

