#include "GunBase.h"



GunBase::GunBase()
{
}

GunBase::~GunBase()
{
}

unsigned int GunBase::GetDamageHigh()
{
	return m_damageMax;
}

unsigned int GunBase::GetDamageLow()
{
	return m_damageMin;
}
