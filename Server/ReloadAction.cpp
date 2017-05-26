#include "ReloadAction.h"
#include "Character.h"
#include "Game.h"



void ReloadAction::_Execute(float dTime)
{
	m_owner->Reload();
	CompleteSelf();
}

ReloadAction::ReloadAction(Character* owner)
	: BaseAction(owner) {
	m_actionType = 9;
}

ReloadAction::~ReloadAction()
{
}

#ifndef NETWORK_SERVER
ReloadAction * ReloadAction::Read(RakNet::BitStream & bsIn)
{
	// Read info
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
	ReloadAction* rA = new ReloadAction(c);
	return rA;
}
#endif

#ifdef NETWORK_SERVER
void ReloadAction::Write(RakNet::BitStream & bs)
{
	// Write character index
	bs.Write(m_owner->GetID());
}
#endif
