#include "SniperRifle.h"
#include <math.h>



SniperRifle::SniperRifle()
{
}

SniperRifle::~SniperRifle()
{
}

int SniperRifle::GetRangeBonus(float distance)
{
	float distSq = distance * distance;
	float bonus = -27.5 + ( 2 * sqrt(2 * distSq) );
	return (bonus < 0) ? bonus : 0;
}
