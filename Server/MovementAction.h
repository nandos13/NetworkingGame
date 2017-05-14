#pragma once

#include "BaseAction.h"
#include "TileMap.h"

class MovementAction : public BaseAction
{
protected:
	MapVec3 m_destination;

#ifndef NETWORK_SERVER
	float m_lerpSpeed;
#endif

	virtual void _Execute(float dTime);

public:
	MovementAction(Character* owner, MapVec3 destination);
	~MovementAction();
	
#ifndef NETWORK_SERVER
	static MovementAction* Read(RakNet::BitStream& bsIn);
#endif

#ifdef NETWORK_SERVER
	virtual void Write(RakNet::BitStream& bs);
#endif
};

