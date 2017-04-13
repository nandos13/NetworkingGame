#pragma once

#include "TileMap.h"
#include "BaseAction.h"
#include "GameAction.h"
#include "GearPieceBase.h"
#include "GunBase.h"

class Character
{
private:
	// Base Stats
	unsigned int m_baseHealth;
	unsigned int m_baseAim;
	unsigned int m_baseMobility;
	unsigned int m_baseDefense;
	unsigned int m_baseCritChance;

	unsigned int CurrentMobility();

	const unsigned int m_sightRadius = 27;
	unsigned int m_remainingPoints = 0;
	unsigned int m_remainingHealth;

	// Gear
	std::unordered_map<short, GearPieceBase> m_gear;
	GunBase* m_gun;

	// Other
	MapVec3 m_currentPosition;
	bool m_inOverwatch = false;

public:
	Character();
	~Character();

	unsigned int GetMoveDistance();
	unsigned int GetDashDistance();
	MapVec3 GetMapTileCoords();

	std::pair<unsigned int, unsigned int> GetWeaponDamage();
	unsigned int GetCurrentAimStat();
	unsigned int GetCurrentDefenseStat();
	int GetAimBonus(float distance);

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
	void Move(MapVec3 destination);
#endif

	MapVec3 GetPosition();
	bool Alive();
	void EndOverwatch();

};

