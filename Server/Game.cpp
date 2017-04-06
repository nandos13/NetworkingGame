#include "Game.h"
#include "TileMap.h"


/** 
 * Set up the Game instance. This handles initialization of squads,
 * tile map, etc.
 */
void Game::Setup()
{
	// TODO: create a squad for each player. One squad humans, one aliens
}

MovementAction * Game::CreateMoveAction(short characterID, std::list<MapVec3> path)
{
	// TODO
	// STEPS:
	// Simulate on the server
	// Check for any overwatch triggers, etc
	// Create action & return
	// USE TAKENACTION CLASS WITH VEC OF GAMEACTIONS

	return nullptr;
}

Game::Game()
{
	m_currentAction = m_actionQueue.begin();

	Setup();
}


Game::~Game()
{
}

void Game::Update(float dTime)
{
	if (m_currentAction != m_actionQueue.end())
	{
		if ((*m_currentAction)->IsCompleted())
			m_currentAction++;	// Current action is complete. Advance iterator
		else
			(*m_currentAction)->Execute(dTime);	// Play current action
	}
}

#ifndef NETWORK_SERVER
/**
 * Add an action to the queue. Actions will be executed one by one
 * by the client, in the order they were sent by the server.
 */
void Game::QueueAction(short uniqueID, GameAction* action)
{
	if (action == nullptr) { return; };
	m_actionQueue.insert(std::map<int, GameAction*>::value_type(uniqueID, action));
}

#endif

#ifdef NETWORK_SERVER
/**
 * Handle character movement to a specified map coordinate. 
 * Returns null if the character or map coordinate could not be found, or if the
 * move is not valid.
 */
MovementAction * Game::MoveCharacter(short characterID, MapVec3 coords)
{
	// Find the referenced character
	Character* c = &(m_characters[characterID]);
	if (c != nullptr)
	{
		// Find the tile the character is standing on
		MapVec3 characterPos = c->GetMapTileCoords();
		if (c->RemainingActionPoints() == 0)
		{
			printf("Warning: Tried to move character (id: %d) with no action points.\n", characterID);
			return nullptr;
		}

		// Find a path
		std::list<MapVec3> path = m_map->FindPath(characterPos, coords);
		if (path.size() == 0) 
		{ 
			printf("Error: Found no path between tile (%d, %d, %d) and tile (%d, %d, %d).\n", 
				characterPos.m_x, characterPos.m_y, characterPos.m_z, coords.m_x, coords.m_y, coords.m_z);
			return nullptr;
		}

		// Check if the character can legally move this far
		if (c->PointsToMove(path.size()) == 0)
			printf("Error: Character (id: %d) cannot legally move %d tiles.\n", characterID, path.size());

		// Create a movement action
		return CreateMoveAction(characterID, path);
	}
	else
		printf("Error: MoveCharacter function could not find character with id %d.\n", characterID);

	return nullptr;
}

#endif