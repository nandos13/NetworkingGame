#include "GameAction.h"
#include "Character.h"



void GameAction::NoOwnerError()
{
	printf("Error: Action does not have an owner assigned.\n");
	m_completed = true;
}

GameAction::GameAction(Character * owner)
{
	m_completed = false;
	m_owner = owner;
}


GameAction::~GameAction()
{
}

void GameAction::Execute(float dTime)
{
	_Execute(dTime);
}
