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

Squad * Game::GetPlayingSquad()
{
	return &m_squads[m_currentTurn];
}

Squad * Game::GetWaitingSquad()
{
	unsigned int currentWaiting = (m_currentTurn == 0) ? 1 : 0;
	return &m_squads[currentWaiting];
}

Game::Game()
{
	m_currentTurn = 0;
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

/**
 * Add an action to the queue. Actions will be executed one by one
 * by the client, in the order they were sent by the server.
 */
void Game::QueueAction(short uniqueID, GameAction* action)
{
	if (action == nullptr) { return; };
	m_actionQueue.push_back(action);
}

#ifndef NETWORK_SERVER
void Game::Draw()
{
	m_squads[0].Draw();
	m_squads[1].Draw();
}
#endif

#ifdef NETWORK_SERVER
void Game::GetShotVariables(short & damage, bool & crit, Character * shooter, MapVec3 target)
{
	// TODO: Use random seed to decide if the shot hits, output this info
}

/**
 * Handle character movement to a specified map coordinate. 
 * Returns null if the character or map coordinate could not be found, or if the
 * move is not valid.
 */
GameAction * Game::CreateMoveAction(short characterID, MapVec3 coords)
{
	// Find the referenced character
	auto& cIter = m_characters.find(characterID);
	if (cIter != m_characters.end())
	{
		Character* c = &(cIter->second);
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

			// Create an action
			GameAction* g = new GameAction();

			// Get some basic info
			Squad* waitingSquad = GetWaitingSquad();

			// Iterate through tiles in the path
			std::list<MapVec3>::iterator pathIter;
			for (pathIter = path.begin(); pathIter != path.end(); pathIter++)
			{
				MapVec3 thisPos = c->GetPosition();
				MapVec3 nextPos = (*pathIter);

				// Check for overwatch triggers. If any are triggered, actions will be added to the list
				waitingSquad->QueryOverwatch(g, c, *m_map);

				// Create movement to next tile in the path
				MovementAction* mA = new MovementAction(c, nextPos);
				g->AddToQueue(mA);

				// Simulate on the server
				// NOTE: Execute takes a float, but this does not do anything when simulated on the server, 
				// as everything is done instantly
				while (!g->IsCompleted())
					g->Execute(0);
			}

			// Check overwatch triggers for final tile move
			waitingSquad->QueryOverwatch(g, c, *m_map);

			// Simulate last overwatch
			while (!g->IsCompleted())
				g->Execute(0);

			// Reset the game action. 
			// (Reverses the effects of simulating it on the server, so it is again ready to use on a client)
			g->Reset();

			return g;
		}
		else
			printf("Error: MoveCharacter function could not find character with id %d.\n", characterID);
	}
	else
		printf("Error: Could not find character");

	return nullptr;
}

#endif