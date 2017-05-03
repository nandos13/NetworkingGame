#include "GunBase.h"

#include <cstdio>



GunBase::GunBase(unsigned int clipSize, unsigned int minDamage, unsigned int maxDamage, int aimModifier, int critModifier)
{
	m_clipSize = clipSize;
	m_remainingAmmo = m_clipSize;
	m_damageMin = minDamage;
	m_damageMax = maxDamage;

	m_aimModifier = aimModifier;
	m_critModifier = critModifier;
}

GunBase::~GunBase()
{
}

unsigned int GunBase::GetDamageHigh() const
{
	return m_damageMax;
}

unsigned int GunBase::GetDamageLow() const
{
	return m_damageMin;
}

int GunBase::GetAimModifier() const
{
	// TODO: Account for attachments
	return m_aimModifier;
}

int GunBase::GetCritModifier() const
{
	// TODO: Account for attachments
	return m_critModifier;
}

void GunBase::UseAmmo(const unsigned int amount)
{
	if (m_remainingAmmo < amount)
	{
		printf("Error: Tried to use more ammo than currently loaded in gun.\n");
		m_remainingAmmo = 0;
	}
	else
		m_remainingAmmo -= amount;
}

unsigned int GunBase::RemainingAmmo() const
{
	return m_remainingAmmo;
}
