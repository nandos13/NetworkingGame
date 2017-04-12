#include "Character.h"
#include "OverwatchShotAction.h"
#include "Game.h"



unsigned int Character::CurrentMobility()
{
	// TODO: Iterate through buffs + debuffs
	return m_baseMobility;
}

Character::Character()
{
}


Character::~Character()
{
}

unsigned int Character::GetMoveDistance()
{
	return (unsigned int)(CurrentMobility() * 0.625);
}

unsigned int Character::GetDashDistance()
{
	return (unsigned int)(CurrentMobility() * 0.625 * 2);
}

MapVec3 Character::GetMapTileCoords()
{
	return m_currentPosition;
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

				short damage = 0;
				bool crit = false;
				Game::GetShotVariables(damage, crit, this, moverPos);

				ShootAction* sa = new ShootAction(this, moverPos, damage, crit);
				OverwatchShotAction* oa = new OverwatchShotAction(this, sa);
				action->AddToQueue(oa);
			}
		}
	}
}
#endif

unsigned int Character::RemainingActionPoints()
{
	return m_remainingPoints;
}

/**
 * Returns the amount of action points that will be used to move 'moveTiles' tiles
 * from the current position. 
 * Returns 0 if the character does not have enough remaining points to make the move. 
 */
unsigned int Character::PointsToMove(short moveTiles)
{
	if (m_remainingPoints <= 0 || moveTiles <= 0) { return 0; };	// Can not legally make the move

	if (moveTiles <= GetMoveDistance() && m_remainingPoints > 0) { return 1; };	// One point to move
	if (moveTiles <= GetDashDistance() && m_remainingPoints > 1) { return 2; };	// Two points to move

	return 0;	// 
}

void Character::ResetActionPoints()
{
	m_remainingPoints = 2;
}

#ifndef NETWORK_SERVER
void Character::Move(MapVec3 destination, float dTime)
{

}

void Character::Draw()
{
	// TODO
}
#endif

#ifdef NETWORK_SERVER
void Character::MoveTo(MapVec3 destination)
{
	// TODO
}
#endif

void Character::EndOverwatch()
{
	m_inOverwatch = false;
}

MapVec3 Character::GetPosition()
{
	return m_currentPosition;
}

bool Character::Alive()
{
	return m_remainingHealth > 0;
}
