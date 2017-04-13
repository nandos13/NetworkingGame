#pragma once

#include "GunBase.h"

class StandardGun : public GunBase
{
public:
	StandardGun();
	virtual ~StandardGun();

	virtual int GetRangeBonus(float distance);
};

