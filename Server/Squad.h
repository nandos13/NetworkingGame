#pragma once
#include "Character.h"
#include "GameAction.h"

#include <unordered_map>
#include <list>

class Squad
{
private:

	std::unordered_map<short, Character*> m_squaddies;

public:
	Squad();
	~Squad();

	void StartTurn();

	Character* FindCharacter(const MapVec3 pos) const;

#ifdef NETWORK_SERVER

	void QueryOverwatch(GameAction* action, Character* mover, TileMap& map);

#endif

#ifndef NETWORK_SERVER
	void Draw();
#endif
};

