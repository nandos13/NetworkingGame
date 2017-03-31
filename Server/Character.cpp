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

unsigned int Character::RemainingActionPoints()
{
	// TODO
	return 0;
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
