#pragma once

#include "GunBase.h"

class Shotgun : public GunBase
{
public:
	Shotgun(unsigned int clipSize, unsigned int minDamage, unsigned int maxDamage, int aimModifier = 0, int critModifier = 0);
	virtual ~Shotgun();

	virtual int GetRangeBonus(float distance);
};

