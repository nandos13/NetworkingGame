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

	TileMap* m_map;

	Squad m_squads[2];
	unsigned int m_currentTurn;
	std::unordered_map<short, Character>	m_characters;
	std::list<GameAction*>					m_actionQueue;
	std::list<GameAction*>::iterator		m_currentAction;

	/* Private methods */
	void Setup();
	Squad* GetPlayingSquad();
	Squad* GetWaitingSquad();

public:
	Game();
	~Game();

	void Update(float dTime);

	void QueueAction(short uniqueID, GameAction* action);

	/* CLIENT-ONLY FUNCTIONALITY */
#ifndef NETWORK_SERVER
	void Draw();
#endif

	/* SERVER-ONLY FUNCTIONALITY */

#ifdef NETWORK_SERVER

	static void GetShotVariables(short& damage, bool& crit, Character* shooter, MapVec3 target);
	//GameAction* TakeShot(const short shooterID, short victimID);
	GameAction* CreateMoveAction(const short characterID, MapVec3 coords);

#endif
};

