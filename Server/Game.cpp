#include "Game.h"


/** 
 * Set up the Game instance. This handles initialization of squads,
 * tile map, etc.
 */
void Game::Setup()
{
	// TODO: create a squad for each player. One squad humans, one aliens
}

Game::Game()
{
	Setup();
}


Game::~Game()
{
}

void Game::Update(float deltaTime)
{
}

#ifndef NETWORK_SERVER
/**
 * Add an action to the queue. Actions will be executed one by one
 * by the client, in the order they were sent by the server.
 */
void Game::QueueAction(short uniqueID, GameAction* action)
{
	m_actionQueue.insert(std::map<int, GameAction*>::value_type(uniqueID, action));
}







#endif
