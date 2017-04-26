#pragma once
class GunBase
{
protected:

	unsigned int m_clipSize;
	unsigned int m_remainingAmmo;

	unsigned int m_damageMin;
	unsigned int m_damageMax;

	int m_aimModifier;
	int m_critModifier;

public:

	GunBase(unsigned int clipSize, unsigned int minDamage, unsigned int maxDamage, int aimModifier = 0, int critModifier = 0);
	virtual ~GunBase();

	virtual int GetRangeBonus(float distance) const = 0;

	unsigned int GetDamageHigh() const;
	unsigned int GetDamageLow() const;

	int GetAimModifier() const;
	int GetCritModifier() const;

};

