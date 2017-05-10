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
enum GAME_STATE { PLAYING, PAUSED, WAITINGFORPLAYER };

class Game
{
private:

	/* Private member variables */

	static Game* m_singleton;

	bool m_forcedSpectator;

	TileMap* m_map;
	float m_tileScale;

	Squad m_squads[2];
#ifndef NETWORK_SERVER
	int m_mySquad = -1;
#endif
	unsigned int m_currentTurn;
	std::unordered_map<short, Character*>	m_characters;
	std::list<GameAction*>					m_actionQueue;
	std::list<GameAction*>::iterator		m_currentAction;

	/* Private methods */
#ifdef NETWORK_SERVER
	void Setup();
#endif
	Squad* GetPlayingSquad();
	Squad* GetWaitingSquad();

	int GetShotChance(const Character* shooter, MapVec3 target);
	int GetCritChance(const Character* shooter, MapVec3 target);
	int GetDamage(const Character* shooter, const bool critical);

	void ClearGame();

public:
	Game();
	~Game();
	void SafeDelete();

	static Game* GetInstance();
	static TileMap* GetMap();
	static float GetMapTileScale();

	void Update(float dTime);

	void QueueAction(short uniqueID, GameAction* action);

	Character* FindCharacterAtCoords(const MapVec3 position) const;
	Character* FindCharacterByID(const short id) const;

	void SetSpectatorMode(const bool state);
	bool IsSpectator() const;

	/* CLIENT-ONLY FUNCTIONALITY */
#ifndef NETWORK_SERVER

	void Draw();

	void Read(RakNet::Packet* packet);
	void TakeControlOfSquad(const int squad);
	std::list<Character*> GetSelectableCharacters() const;

#endif

	/* SERVER-ONLY FUNCTIONALITY */
#ifdef NETWORK_SERVER

	void GetShotVariables(short& damage, SHOT_STATUS& shotType, const Character* shooter, const MapVec3 target);
	//GameAction* TakeShot(const short shooterID, short victimID);
	GameAction* CreateMoveAction(const short characterID, MapVec3 coords);

	void TempGameSetup();

	void Write(RakNet::BitStream& bs);

#endif
};

