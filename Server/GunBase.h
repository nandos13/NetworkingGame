#pragma once

#include <RakPeerInterface.h>

class GunBase
{
protected:

	int m_gunType = 0;

	unsigned int m_clipSize;
	unsigned int m_remainingAmmo;

	unsigned int m_damageMin;
	unsigned int m_damageMax;

	int m_aimModifier;
	int m_critModifier;

public:

	GunBase(unsigned int clipSize, unsigned int minDamage, unsigned int maxDamage, int aimModifier = 0, int critModifier = 0);
	virtual ~GunBase();

	virtual int GetRangeBonus(const float distance) const = 0;

	unsigned int GetDamageHigh() const;
	unsigned int GetDamageLow() const;

	int GetAimModifier() const;
	int GetCritModifier() const;

	void UseAmmo(const unsigned int amount = 1);
	unsigned int RemainingAmmo() const;
	unsigned int MaxAmmo() const;

	void Reload();

#ifndef NETWORK_SERVER
	static GunBase* Read(RakNet::BitStream& bsIn);
#endif

#ifdef NETWORK_SERVER
	virtual void Write(RakNet::BitStream& bs);
#endif

};

