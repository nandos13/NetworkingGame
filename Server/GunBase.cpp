#include "GunBase.h"



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
