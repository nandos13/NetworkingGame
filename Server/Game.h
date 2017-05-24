#pragma once

#include "TileMap.h"
#include "GameAction.h"
#include "Squad.h"

#include <list>
#include <map>
#include <unordered_map>

struct MapVec3;

enum SHOT_STATUS 
{	MISS = 0, 
	GRAZE = 1, 
	HIT = 2, 
	CRITICAL = 3 
};
enum GAME_STATE { PLAYING, PAUSED, WAITINGFORPLAYER };

using namespace JakePerry;
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
	int m_mySquad = -1;		// Tracks which squad a client has control over
#endif
	unsigned int m_currentTurn;
	std::unordered_map<short, Character*>	m_characters;
	std::list<GameAction*>					m_actionQueue;
	std::list<GameAction*>::iterator		m_currentAction;

	/* Private methods */
#ifdef NETWORK_SERVER
	void Setup();
	GameAction* CreateInitialWalkableTilesAction();
	GameAction* CreateInitialVisibleEnemiesAction();
	std::list<Character*> GetVisibleEnemies(Character* lookUnit);
#endif
	Squad* GetPlayingSquad();
	Squad* GetWaitingSquad();
	void QueryTurnEnd(GameAction* g);
	void CreateTurnEndAction(GameAction* g);
	
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

	void QueueAction(GameAction* action);

	Character* FindCharacterAtCoords(const MapVec3 position) const;
	Character* FindCharacterByID(const short id) const;
	std::list<Character*> GetCharactersByHomeSquad(const unsigned int squad) const;
	std::list<MapVec3> GetAllCharacterPositions(const bool ignoreDeadCharacters = true) const;

	int GetShotChance(const Character* shooter, MapVec3 target);
	int GetCritChance(const Character* shooter, MapVec3 target);

	void SetSpectatorMode(const bool state);
	void SetTurn(const bool playerOne);
	bool IsSpectator() const;
	bool IsPlayersTurn(const unsigned int playerID) const;

	/* CLIENT-ONLY FUNCTIONALITY */
#ifndef NETWORK_SERVER

	void Draw();

	void Read(RakNet::Packet* packet);
	void TakeControlOfSquad(const int squad);
	std::list<Character*> GetSelectableCharacters() const;

	bool IsMyTurn() const;
	Squad* GetMySquad();

#endif

	/* SERVER-ONLY FUNCTIONALITY */
#ifdef NETWORK_SERVER

	void GetShotVariables(short& damage, SHOT_STATUS& shotType, const Character* shooter, const MapVec3 target);
	GameAction* CreateShootAction(const short shooterID, short victimID);
	GameAction* CreateMoveAction(const short characterID, MapVec3 coords);
	GameAction* CreateHunkerAction(const short characterID);

	void TempGameSetup();

	void Write(RakNet::BitStream& bs);

#endif
};

