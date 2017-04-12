#pragma once

#include "BaseAction.h"
#include "TileMap.h"

class ShootAction : public BaseAction
{
protected:
	MapVec3 m_target;
	short m_ammo;

	short m_damage;
	bool m_crit;

	virtual void _Execute(float dTime);

public:
	ShootAction(Character* owner, MapVec3 target, short damage, bool crit, short ammoUse = 1);
	~ShootAction();

#ifdef NETWORK_SERVER
	virtual void Write(RakNet::BitStream& bs);
#endif
};

