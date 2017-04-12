#pragma once

#include "BaseAction.h"
#include "TileMap.h"

class MovementAction : public BaseAction
{
protected:
	MapVec3 m_destination;

	virtual void _Execute(float dTime);

public:
	MovementAction(Character* owner, MapVec3 destination);
	~MovementAction();
	
#ifdef NETWORK_SERVER
	virtual void Write(RakNet::BitStream& bs);
#endif
};

