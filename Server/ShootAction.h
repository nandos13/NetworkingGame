#pragma once

#include "BaseAction.h"
#include "TileMap.h"
#include "Game.h"

class ShootAction : public BaseAction
{
protected:
	MapVec3 m_target;
	unsigned int m_ammo;
	SHOT_STATUS m_shotState;

	short m_damage;
	bool m_shred;

	virtual void _Execute(float dTime);
	void OnKill(Character* victim);

public:
	ShootAction(Character* owner, MapVec3 target, short damage, SHOT_STATUS shotState, unsigned int ammoUse = 1, bool armourShred = false);
	~ShootAction();
	
#ifndef NETWORK_SERVER
	static ShootAction* Read(RakNet::BitStream& bsIn);
#endif

#ifdef NETWORK_SERVER
	virtual void Write(RakNet::BitStream& bs);
#endif
};

