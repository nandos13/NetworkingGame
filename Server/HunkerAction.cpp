#include "HunkerAction.h"
#include "Character.h"
#include "Game.h"



void HunkerAction::_Execute(float dTime)
{
	if (m_owner != nullptr)
		m_owner->SetHunkerState(true);

	CompleteSelf();
}

HunkerAction::HunkerAction(Character* owner) :
	BaseAction(owner) {
	m_actionType = 8;
}

HunkerAction::~HunkerAction()
{
}

#ifndef NETWORK_SERVER
HunkerAction * HunkerAction::Read(RakNet::BitStream & bsIn)
{
	// Read character index
	short characterID = 0;

	bsIn.Read(characterID);

	// Find character by ID
	Character* c = Game::GetInstance()->FindCharacterByID(characterID);

	// Error check
	if (c == nullptr)
	{
		printf("Error: Could not find character with id: %d\n", characterID);
		return nullptr;
	}

	// Create & return action
	HunkerAction* hA = new HunkerAction(c);
	return hA;
}
#endif

#ifdef NETWORK_SERVER
void HunkerAction::Write(RakNet::BitStream & bs)
{
	// Write character index
	bs.Write(m_owner->GetID());
}
#endif
