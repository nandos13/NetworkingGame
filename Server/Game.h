#pragma once
#include "TileMap.h"
#include "GameAction.h"
#include "MovementAction.h"
#include "Squad.h"

#include <list>
#include <map>
#include <unordered_map>

struct MapVec3;

enum SHOT_STATUS { MISS, GRAZE, HIT, CRITICAL };

class Game
{
private:

	/* Private member variables */

	Game* m_singleton;

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

	int GetShotChance(Character* shooter, MapVec3 target);
	int GetCritChance(Character* shooter, MapVec3 target);
	int GetDamage(Character* shooter);

public:
	Game();
	~Game();

	Game* GetInstance();

	void Update(float dTime);

	void QueueAction(short uniqueID, GameAction* action);

	/* CLIENT-ONLY FUNCTIONALITY */
#ifndef NETWORK_SERVER
	void Draw();
#endif

	/* SERVER-ONLY FUNCTIONALITY */

#ifdef NETWORK_SERVER

	void GetShotVariables(short& damage, SHOT_STATUS& shotType, const Character* shooter, const MapVec3 target);
	//GameAction* TakeShot(const short shooterID, short victimID);
	GameAction* CreateMoveAction(const short characterID, MapVec3 coords);

#endif
};

