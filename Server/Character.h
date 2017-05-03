#pragma once

#include "TileMap.h"
#include "BaseAction.h"
#include "GameAction.h"
#include "GearPieceBase.h"
#include "GunBase.h"

#ifndef NETWORK_SERVER
#include "../Client/GameObj.h"
#endif

class Character
{
private:
	// Base Stats
	unsigned int m_baseHealth;
	unsigned int m_baseAim;
	unsigned int m_baseMobility;
	//unsigned int m_baseDefense;
	unsigned int m_baseCritChance;

	unsigned int CurrentMobility() const;

	const unsigned int m_sightRadius = 27;
	unsigned int m_remainingPoints = 0;
	unsigned int m_remainingHealth;

	// Gear
	std::unordered_map<short, GearPieceBase> m_gear;
	GunBase* m_gun;

	// Other
	MapVec3 m_currentPosition;
	bool m_inOverwatch = false;
	short m_ID;
	short m_homeSquad;

#ifndef NETWORK_SERVER
	GameObj m_gameObject;
#endif

public:
	Character(short ID, short HomeSquad);
	~Character();

	unsigned int GetMoveDistance() const;
	unsigned int GetDashDistance() const;
	MapVec3 GetMapTileCoords() const;

	void UseAmmo(const unsigned int amount);
	std::pair<unsigned int, unsigned int> GetWeaponDamage() const;
	unsigned int GetCurrentAimStat() const;
	unsigned int GetCurrentDefenseStat() const;
	int GetAimBonus(float distance) const;

#ifdef NETWORK_SERVER
	void QueryOverwatch(GameAction* action, Character* mover, TileMap& map);
#endif

	unsigned int RemainingActionPoints() const;
	unsigned int PointsToMove(short moveTiles) const;
	void ResetActionPoints();

	/* CLIENT-ONLY FUNCTIONALITY */
#ifndef NETWORK_SERVER

	bool Move(MapVec3 destination, float dTime);

	void Read(RakNet::BitStream& bsIn);
	void Draw();

#endif

	/* SERVER-ONLY FUNCTIONALITY */
#ifdef NETWORK_SERVER

	void Move(MapVec3 destination);

	void SetPrimaryGun(GunBase* gun);
	void SetHealth(const unsigned int baseHealth, const bool setCurrentHealth = true);
	void SetAim(const unsigned int baseAim);
	void SetMobility(const unsigned int baseMobility);
	//void SetDefense(const unsigned int baseDefense);
	void SetCritChance(const unsigned int baseCritChance);

	void Write(RakNet::BitStream& bs);

#endif

	short GetID() const;
	short GetHomeSquad() const;
	MapVec3 GetPosition() const;
	void ApplyDamage(const int amount, const bool armourShred = false);
	bool Alive() const;
	void EndOverwatch();

};

