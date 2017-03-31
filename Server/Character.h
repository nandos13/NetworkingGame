#pragma once

#include "TileMap.h"

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

	// Variable Stats
	// TODO: Actually, these should probably be calculated when needed based
	// on base-stats and active debuffs

	// Other
	MapVec3 m_currentPosition = MapVec3(-1);

public:
	Character();
	~Character();

	unsigned int GetMoveDistance();
	unsigned int GetDashDistance();
	MapVec3 GetMapTileCoords();

	unsigned int RemainingActionPoints();
	unsigned int PointsToMove(short moveTiles);
};

