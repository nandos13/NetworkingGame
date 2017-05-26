#include "GunBase.h"

#include "Shotgun.h"
#include "SniperRifle.h"
#include "StandardGun.h"

#include <BitStream.h>
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

unsigned int GunBase::MaxAmmo() const
{
	return m_clipSize;
}

void GunBase::Reload()
{
	m_remainingAmmo = m_clipSize;
}

#ifndef NETWORK_SERVER
GunBase * GunBase::Read(RakNet::BitStream & bsIn)
{
	// Read gun type
	int id = 0;
	bsIn.Read(id);

	unsigned int clipSize = 0;
	unsigned int damageMin = 0;
	unsigned int damageMax = 0;
	int aimModifier = 0;
	int critModifier = 0;

	bsIn.Read(clipSize);
	bsIn.Read(damageMin);
	bsIn.Read(damageMax);
	bsIn.Read(aimModifier);
	bsIn.Read(critModifier);

	switch (id)
	{
	case 1:		return new Shotgun(clipSize, damageMin, damageMax, aimModifier, critModifier);
	case 2:		return new SniperRifle(clipSize, damageMin, damageMax, aimModifier, critModifier);
	case 3:		return new StandardGun(clipSize, damageMin, damageMax, aimModifier, critModifier);

	default:	return nullptr;
	}

}
#endif

#ifdef NETWORK_SERVER
void GunBase::Write(RakNet::BitStream & bs)
{
	// Write gun type
	bs.Write(m_gunType);

	// Write gun stats
	bs.Write(m_clipSize);
	bs.Write(m_damageMin);
	bs.Write(m_damageMax);
	bs.Write(m_aimModifier);
	bs.Write(m_critModifier);
}
#endif
