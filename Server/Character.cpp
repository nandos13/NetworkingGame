#include "Character.h"
#include "OverwatchShotAction.h"
#include "Game.h"



unsigned int Character::CurrentMobility() const
{
	// TODO: Iterate through buffs + debuffs
	return m_baseMobility;
}

Character::Character(short ID, short HomeSquad, unsigned int health, unsigned int aim, unsigned int mobility)
{
	m_ID = ID;
	m_homeSquad = HomeSquad;

	m_baseHealth = health;
	m_remainingHealth = m_baseHealth;
	m_baseAim = aim;
	m_baseMobility = mobility;
}

Character::~Character()
{
	// TODO: Delete gun, etc
}

unsigned int Character::GetSightRadius() const
{
	return m_sightRadius;
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

std::list<MapVec3> Character::Get1PointWalkTiles() const
{
	return m_1PointWalkableTiles;
}

std::list<MapVec3> Character::Get2PointWalkTiles() const
{
	return m_2PointWalkableTiles;
}

void Character::Set1PointWalkableTiles(std::list<MapVec3> tiles)
{
	m_1PointWalkableTiles = tiles;
}

void Character::Set2PointWalkableTiles(std::list<MapVec3> tiles)
{
	m_2PointWalkableTiles = tiles;
}

MapVec3 Character::GetMapTileCoords() const
{
	return m_currentPosition;
}

std::list<Character*> Character::GetVisibleEnemies() const
{
	return m_visibleEnemies;
}

void Character::SetVisibleEnemyList(const std::list<Character*> enemies)
{
	m_visibleEnemies = enemies;
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

unsigned int Character::GetCurrentDefense() const
{
	// TODO: Factor in gear, hunker-state, etc to get base stat.
	// TODO: Get defense from position based on cover
	return 0;
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
			if (map.CheckTileSight(m_currentPosition, moverPos, (float)m_sightRadius))
			{
				// Create an overwatch-shot action

				Game* game = Game::GetInstance();

				short damage = 0;
				SHOT_STATUS shotType = MISS;
				game->GetShotVariables(damage, shotType, this, moverPos);

				ShootAction* sa = new ShootAction(this, moverPos, damage, shotType/*, ammoUse, shred*/);	// TODO: Get armour shred & ammo use
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

void Character::SetActionPoints(const unsigned int newPoints)
{
	if (newPoints >= 0 && newPoints <= 2)
	{
		m_remainingPoints = newPoints;
	}
	else
		printf("Warning: Character SetActionPoints function was called with a newPoints value of %d. Only values of 0, 1 or 2 are allowed.\n", newPoints);
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

	return 0;
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
bool Character::Move(MapVec3 destination, const float speed, const float dTime)
{
	// Find world coordinates of the specified destination & move character's gameobject
	float x, y, z;
	MapVec3::GetTileWorldCoords(x, y, z, destination, Game::GetMapTileScale());
	bool reachedDestination = m_gameObject.LerpMove(x, y, z, speed, dTime);

	// Update character's position
	m_gameObject.GetWorldPosition(x, y, z);
	m_currentPosition = MapVec3::FindTileAtWorldCoords(x, y, z, Game::GetMapTileScale());
	
	return reachedDestination;
}

void Character::GetGameObjPosition(float & x, float & y, float & z)
{
	m_gameObject.GetWorldPosition(x, y, z);
}

Character* Character::Read(RakNet::BitStream & bsIn)
{
	// Read ID info
	short id, squad;
	bsIn.Read(id);
	bsIn.Read(squad);

	// Read base stats
	unsigned int health, aim, mobility;
	bsIn.Read(health);
	bsIn.Read(aim);
	bsIn.Read(mobility);

	// Create character based on stats
	Character* c = new Character(id, squad, health, aim, mobility);

	// Read position
	MapVec3 pos(0);
	pos.Read(bsIn);
	c->SetPosition(pos);

	// Set GameObject's position
	float x = 0, y = 0, z = 0;
	MapVec3::GetTileWorldCoords(x, y, z, pos, Game::GetMapTileScale());
	c->m_gameObject.SetPosition(x, y, z);

	// TODO: Implement rest of function along with Write function

	return c;
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

void Character::SetPrimaryGun(GunBase * gun)
{
	m_gun = gun;
}

void Character::SetHealth(const unsigned int baseHealth, const bool setCurrentHealth)
{
	m_baseHealth = baseHealth;
	if (setCurrentHealth)
		m_remainingHealth = baseHealth;
}

void Character::SetAim(const unsigned int baseAim)
{
	m_baseAim = baseAim;
}

void Character::SetMobility(const unsigned int baseMobility)
{
	m_baseMobility = baseMobility;
}

void Character::Write(RakNet::BitStream & bs)
{
	// Write IDs
	bs.Write(m_ID);
	bs.Write(m_homeSquad);

	// Write base stats
	bs.Write(m_baseHealth);
	bs.Write(m_baseAim);
	bs.Write(m_baseMobility);

	// Write position
	m_currentPosition.Write(bs);

	// TODO: Finish implementation with gun, gear, abilities, etc
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

void Character::SetPosition(const MapVec3 pos)
{
	m_currentPosition = pos;
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

bool Character::HasRemainingPoints() const
{
	return m_remainingPoints > 0;
}

bool Character::IsSelectable() const
{
	// TODO: Loop through debuffs, find if the character is stunned, etc.
	if (m_remainingPoints <= 0 || !Alive())
		return false;

	return true;
}
