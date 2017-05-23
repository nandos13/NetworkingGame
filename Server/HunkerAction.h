#pragma once
#include "BaseAction.h"
class HunkerAction : public BaseAction
{
private:
	virtual void _Execute(float dTime);

public:
	HunkerAction(Character* owner);
	~HunkerAction();

#ifndef NETWORK_SERVER
	static HunkerAction* Read(RakNet::BitStream& bsIn);
#endif

#ifdef NETWORK_SERVER
	virtual void Write(RakNet::BitStream& bs);
#endif
};

