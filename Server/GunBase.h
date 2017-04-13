#pragma once
class GunBase
{
protected:
	unsigned int m_clipSize;
	unsigned int m_remainingAmmo;

	unsigned int m_damageMin;
	unsigned int m_damageMax;
public:
	GunBase();
	virtual ~GunBase();

	virtual int GetRangeBonus(float distance) = 0;

	unsigned int GetDamageHigh();
	unsigned int GetDamageLow();
};

