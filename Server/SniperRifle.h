#pragma once

#include "GunBase.h"

class SniperRifle : public GunBase
{
public:
	SniperRifle();
	virtual ~SniperRifle();

	virtual int GetRangeBonus(float distance);
};

