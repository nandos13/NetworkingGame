#pragma once

#include "BaseAction.h"
#include "TileMap.h"

class ShootAction : public BaseAction
{
protected:
	MapVec3 m_target;
	unsigned int m_ammo;

	short m_damage;
	bool m_shred;

	virtual void _Execute(float dTime);

public:
	ShootAction(Character* owner, MapVec3 target, short damage, unsigned int ammoUse = 1, bool armourShred = false);
	~ShootAction();

#ifdef NETWORK_SERVER
	virtual void Write(RakNet::BitStream& bs);
#endif
};

