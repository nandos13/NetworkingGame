#pragma once
#include "BaseAction.h"
class StartNewTurnAction : public BaseAction
{
private:
	bool m_playerOneTurn;

	virtual void _Execute(float dTime);

public:
	StartNewTurnAction(Character* owner, const bool switchToPlayerOne);
	~StartNewTurnAction();

#ifndef NETWORK_SERVER
	static StartNewTurnAction* Read(RakNet::BitStream& bsIn);
#endif

#ifdef NETWORK_SERVER
	virtual void Write(RakNet::BitStream& bs);
#endif
};

