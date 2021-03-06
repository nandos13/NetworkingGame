#pragma once
#include "BaseAction.h"
#include "ShootAction.h"

class OverwatchShotAction : public BaseAction
{
protected:
	ShootAction* m_shoot;

	virtual void _Execute(float dTime);

public:
	OverwatchShotAction(Character* owner, ShootAction* shootAction);
	~OverwatchShotAction();

#ifndef NETWORK_SERVER
	static OverwatchShotAction* Read(RakNet::BitStream& bsIn);
#endif

#ifdef NETWORK_SERVER
	virtual void Write(RakNet::BitStream& bs);
#endif
};

