#pragma once
#include "TileMap.h"
#include "Player.h"
#include "GameAction.h"
#include "Squad.h"

#include <vector>
#include <map>

class Game
{
private:

	/* Private member variables */

	TileMap*		m_map;
	Player*			m_player1;
	Player*			m_player2;

	Squad m_squads[2];
	std::map<short, GameAction*>	m_actionQueue;

	/* Private methods */
	void Setup();

public:
	Game();
	~Game();

	/* GENERAL */
	void Update(float deltaTime);

	/* CLIENT-ONLY FUNCTIONALITY */
#ifndef NETWORK_SERVER

	void QueueAction(short uniqueID, GameAction* action);	// Should the server have access to this also??

#endif
};

