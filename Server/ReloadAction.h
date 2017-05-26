#pragma once
#include "BaseAction.h"
class ReloadAction : public BaseAction
{
private:
	virtual void _Execute(float dTime);
public:
	ReloadAction(Character* owner);
	~ReloadAction();

#ifndef NETWORK_SERVER
	static ReloadAction* Read(RakNet::BitStream& bsIn);
#endif

#ifdef NETWORK_SERVER
	virtual void Write(RakNet::BitStream& bs);
#endif
};

