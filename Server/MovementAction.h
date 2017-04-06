#pragma once

#include "GameAction.h"
#include "TileMap.h"

class MovementAction : public GameAction
{
protected:
	MapVec3 m_destination;

	virtual void _Execute(float dTime);

public:
	MovementAction(Character* owner, MapVec3 destination);
	~MovementAction();
	
};

