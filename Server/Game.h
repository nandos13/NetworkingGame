#pragma once
#include "TileMap.h"
#include "GameAction.h"
#include "MovementAction.h"
#include "Squad.h"

#include <list>
#include <map>
#include <unordered_map>

struct MapVec3;

class Game
{
private:

	/* Private member variables */

	TileMap*		m_map;

	Squad m_squads[2];
	std::unordered_map<short, Character>	m_characters;
	std::list<GameAction*>					m_actionQueue;
	std::list<GameAction*>::iterator		m_currentAction;

	/* Private methods */
	void Setup();

	MovementAction* CreateMoveAction(short characterID, std::list<MapVec3> path);

public:
	Game();
	~Game();

	void Update(float dTime);

	/* CLIENT-ONLY FUNCTIONALITY */
#ifndef NETWORK_SERVER

	void QueueAction(short uniqueID, GameAction* action);	// Should the server have access to this also??

#endif

	/* SERVER-ONLY FUNCTIONALITY */
#ifdef NETWORK_SERVER

	GameAction* TakeShot(short shooterID, short victimID);
	MovementAction* MoveCharacter(short characterID, MapVec3 coords);

#endif
};

