#pragma once

#include "GameAction.h"
#include "TileMap.h"

class ShootAction :
	public GameAction
{
protected:
	MapVec3 m_target;

	short m_damage;

	virtual void _Execute(float dTime);

public:
	ShootAction(Character* owner, MapVec3 target, short damage);
	~ShootAction();
};

