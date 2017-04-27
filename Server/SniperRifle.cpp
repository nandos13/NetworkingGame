#include "SniperRifle.h"
#include <math.h>



SniperRifle::SniperRifle(unsigned int clipSize, unsigned int minDamage, unsigned int maxDamage, int aimModifier, int critModifier)
	: GunBase(clipSize, minDamage, maxDamage, aimModifier, critModifier)
{
}

SniperRifle::~SniperRifle()
{
}

int SniperRifle::GetRangeBonus(const float distance)
{
	float distSq = distance * distance;
	float bonus = -27.5 + ( 2 * sqrt(2 * distSq) );
	return (bonus < 0) ? bonus : 0;
}
