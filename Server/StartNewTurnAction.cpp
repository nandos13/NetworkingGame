#include "StartNewTurnAction.h"
#include "Character.h"
#include "Game.h"



void StartNewTurnAction::_Execute(float dTime)
{
	Game* game = Game::GetInstance();

	game->SetTurn(m_playerOneTurn);
	CompleteSelf();
}

StartNewTurnAction::StartNewTurnAction(Character* owner, const bool switchToPlayerOne)
	: BaseAction(owner), m_playerOneTurn(switchToPlayerOne) {
	m_requiresOwner = false;
	m_actionType = 7;
}

StartNewTurnAction::~StartNewTurnAction()
{
}

#ifndef NETWORK_SERVER
StartNewTurnAction * StartNewTurnAction::Read(RakNet::BitStream & bsIn)
{
	// Read info
	bool switchToPlayerOne = false;

	bsIn.Read(switchToPlayerOne);

	// Create & return action
	StartNewTurnAction* sntA = new StartNewTurnAction(nullptr, switchToPlayerOne);
	return sntA;
}
#endif

#ifdef NETWORK_SERVER
void StartNewTurnAction::Write(RakNet::BitStream & bs)
{
	// Write player turn bool
	bs.Write(m_playerOneTurn);
}
#endif
