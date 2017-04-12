#pragma once

#include "TileMap.h"
#include "BaseAction.h"
#include "GameAction.h"
#include "GearPieceBase.h"

class Character
{
private:
	// Base Stats
	unsigned int m_baseHealth;
	unsigned int m_baseAim;
	unsigned int m_baseMobility;
	unsigned int m_baseDefense;
	unsigned int m_baseCritChance;

	unsigned int CurrentAim();
	unsigned int CurrentMobility();
	unsigned int CurrentDefense();

	const unsigned int m_sightRadius = 27;
	unsigned int m_remainingPoints = 0;
	unsigned int m_remainingHealth;

	// Gear
	std::unordered_map<short, GearPieceBase> m_gear;

	// Other
	MapVec3 m_currentPosition;
	bool m_inOverwatch = false;

public:
	Character();
	~Character();

	// Amount of tiles the character is able to move, using a single action point
	unsigned int GetMoveDistance();
	// Amount of tiles the character is able to dash, using both action points
	unsigned int GetDashDistance();
	MapVec3 GetMapTileCoords();

#ifdef NETWORK_SERVER
	void QueryOverwatch(GameAction* action, Character* mover, TileMap& map);
#endif

	unsigned int RemainingActionPoints();
	unsigned int PointsToMove(short moveTiles);
	void ResetActionPoints();

	/* CLIENT-ONLY FUNCTIONALITY */
#ifndef NETWORK_SERVER
	// Smoothly lerp character's position
	void Move(MapVec3 destination, float dTime);

	void Draw();
#endif

	/* SERVER-ONLY FUNCTIONALITY */
#ifdef NETWORK_SERVER
	// Instantly move character's position
	void MoveTo(MapVec3 destination);
#endif

	MapVec3 GetPosition();
	bool Alive();
	void EndOverwatch();

};

