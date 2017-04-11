#include "Character.h"



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
void Character::QueryOverwatch(GameAction* action, Character * mover)
{
	if (mover != nullptr && m_inOverwatch)
	{
		MapVec3 moverPos = mover->GetPosition();

		// TODO
		bool shot = false;
		int damage = 0;
		bool crit = false;
		// TODO: PASS REFERENCES INTO FUNCTION SOMEWHERE.
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

MapVec3 Character::GetPosition()
{
	return m_currentPosition;
}
