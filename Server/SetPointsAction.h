#pragma once
#include "BaseAction.h"
class SetPointsAction : public BaseAction
{
protected:
	unsigned int m_newPoints;

	virtual void _Execute(float dTime);

public:
	SetPointsAction(Character* owner, const unsigned int points);
	~SetPointsAction();

#ifndef NETWORK_SERVER
	static SetPointsAction* Read(RakNet::BitStream& bsIn);
#endif

#ifdef NETWORK_SERVER
	virtual void Write(RakNet::BitStream& bs);
#endif
};

