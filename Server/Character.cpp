#include "Character.h"
#include "OverwatchShotAction.h"
#include "Game.h"



unsigned int Character::CurrentMobility() const
{
	// TODO: Iterate through buffs + debuffs
	return m_baseMobility;
}

Character::Character(short ID, short HomeSquad)
{
	m_ID = ID;
	m_homeSquad = HomeSquad;
}

Character::~Character()
{
}

/* Amount of tiles the character is able to move, using a single action point */
unsigned int Character::GetMoveDistance() const
{
	return (unsigned int)(CurrentMobility() * 0.625);
}

/* Amount of tiles the character is able to dash, using both action points */
unsigned int Character::GetDashDistance() const
{
	return (unsigned int)(CurrentMobility() * 0.625 * 2);
}

MapVec3 Character::GetMapTileCoords() const
{
	return m_currentPosition;
}

std::pair<unsigned int, unsigned int> Character::GetWeaponDamage() const
{
	if (m_gun != nullptr)
		return std::make_pair(m_gun->GetDamageLow(), m_gun->GetDamageHigh());
	return std::pair<unsigned int, unsigned int>();
}

void Character::UseAmmo(const unsigned int amount)
{
	if (m_gun)
		m_gun->UseAmmo(amount);
}

unsigned int Character::GetCurrentAimStat() const
{
	unsigned int aim = m_baseAim;

	if (m_gun)
		aim += m_gun->GetAimModifier();

	// TODO: Account for gear bonuses, debuffs, etc

	return aim;
}

unsigned int Character::GetCurrentDefenseStat() const
{
	unsigned int crit = m_baseCritChance;

	if (m_gun)
		crit += m_gun->GetCritModifier();

	// TODO: Account for gear bonuses, debuffs, etc

	return crit;
}

int Character::GetAimBonus(float distance) const
{
	if (m_gun)
		return m_gun->GetRangeBonus(distance);
	return 0;
}

#ifdef NETWORK_SERVER
void Character::QueryOverwatch(GameAction* action, Character * mover, TileMap& map)
{
	if (mover != nullptr && m_inOverwatch)
	{
		// Only attempt to take the shot if the mover is alive.
		// This will prevent multiple units taking overwatch at the same time and wasting the shot
		// if the first one connects
		if (mover->Alive())
		{
			MapVec3 moverPos = mover->GetPosition();

			// Check if the character can see the mover's position tile
			if (map.CheckTileSight(m_currentPosition, moverPos, m_sightRadius))
			{
				// Create an overwatch-shot action

				Game* game = Game::GetInstance();

				short damage = 0;
				SHOT_STATUS shotType = MISS;
				game->GetShotVariables(damage, shotType, this, moverPos);

				ShootAction* sa = new ShootAction(this, moverPos, damage);
				OverwatchShotAction* oa = new OverwatchShotAction(this, sa);
				action->AddToQueue(oa);
			}
		}
	}
}
#endif

unsigned int Character::RemainingActionPoints() const
{
	return m_remainingPoints;
}

/**
 * Returns the amount of action points that will be used to move 'moveTiles' tiles
 * from the current position. 
 * Returns 0 if the character does not have enough remaining points to make the move. 
 */
unsigned int Character::PointsToMove(short moveTiles) const
{
	if (m_remainingPoints <= 0 || moveTiles <= 0) { return 0; };	// Can not legally make the move

	if (moveTiles <= (int)GetMoveDistance() && (int)m_remainingPoints > 0) { return 1; };	// One point to move
	if (moveTiles <= (int)GetDashDistance() && (int)m_remainingPoints > 1) { return 2; };	// Two points to move

	return 0;	// 
}

void Character::ResetActionPoints()
{
	m_remainingPoints = 2;
}

#ifndef NETWORK_SERVER
/**
 * Lerps character's position towards it's destination.
 * Returns true if the character reaches destination this frame.
 */
bool Character::Move(MapVec3 destination, float dTime)
{
	TileMap* map = Game::GetMap();

	// Find world coordinates of the specified destination & move character's gameobject
	float x, y, z;
	map->GetTileWorldCoords(x, y, z, destination, Game::GetMapTileScale());
	bool reachedDestination = m_gameObject.LerpMove(x, y, z, dTime);

	// Update character's position
	m_gameObject.GetWorldPosition(x, y, z);
	m_currentPosition = map->FindTileAtWorldCoords(x, y, z, Game::GetMapTileScale());
	
	return reachedDestination;
}

void Character::Read(RakNet::BitStream & bsIn)
{
	// TODO
}

void Character::Draw()
{
	m_gameObject.Draw();
}
#endif

#ifdef NETWORK_SERVER
/* Instantly move character's position for use on server-side */
void Character::Move(MapVec3 destination)
{
	m_currentPosition = destination;
}

void Character::Write(RakNet::BitStream & bs)
{
	// TODO: Finish implementation
}
#endif

void Character::EndOverwatch()
{
	m_inOverwatch = false;
}

/* Returns the character's unique ID, which corresponds to the map key it belongs to in the Game class. */
short Character::GetID() const
{
	return m_ID;
}

/** 
 * Returns the character's initial squad. 
 * This may not be the same as the current squad if the unit is under mind control, etc.
 */
short Character::GetHomeSquad() const
{
	return m_homeSquad;
}

MapVec3 Character::GetPosition() const
{
	return m_currentPosition;
}

void Character::ApplyDamage(const int amount, const bool armourShred)
{
	// TODO: Check for armour, shredding, etc
	if ((int)m_remainingHealth < amount)
		m_remainingHealth = 0;
	else
		m_remainingHealth -= amount;

}

bool Character::Alive() const
{
	return m_remainingHealth > 0;
}
