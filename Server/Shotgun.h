#pragma once

#include "GunBase.h"

class Shotgun : public GunBase
{
public:
	Shotgun();
	virtual ~Shotgun();

	virtual int GetRangeBonus(float distance);
};

