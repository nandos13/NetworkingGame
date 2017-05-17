#pragma once
#include "BaseAction.h"
class SetVisibleEnemiesAction : public BaseAction
{
private:
	std::list<Character*> m_visibleEnemies;

	virtual void _Execute(float dTime);

public:
	SetVisibleEnemiesAction(Character* owner, const std::list<Character*> enemies);
	~SetVisibleEnemiesAction();

#ifndef NETWORK_SERVER
	static SetVisibleEnemiesAction* Read(RakNet::BitStream& bsIn);
#endif

#ifdef NETWORK_SERVER
	virtual void Write(RakNet::BitStream& bs);
#endif
};

