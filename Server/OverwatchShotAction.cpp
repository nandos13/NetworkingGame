#include "OverwatchShotAction.h"
#include "Character.h"
#include "Game.h"



#ifndef NETWORK_SERVER
// Client-side execution
void OverwatchShotAction::_Execute(float dTime)
{
	m_shoot->Execute(dTime);

	if (m_shoot->IsCompleted())
	{
		m_owner->EndOverwatch();
		CompleteSelf();
	}
}
#endif

#ifdef NETWORK_SERVER
// Server-side execution
void OverwatchShotAction::_Execute(float dTime)
{
	m_shoot->Execute(dTime);
	m_owner->EndOverwatch();
	CompleteSelf();
}
#endif

OverwatchShotAction::OverwatchShotAction(Character* owner, ShootAction* shootAction) : BaseAction(owner)
{
	m_shoot = shootAction;
}

OverwatchShotAction::~OverwatchShotAction()
{
}

#ifndef NETWORK_SERVER
OverwatchShotAction * OverwatchShotAction::Read(RakNet::BitStream & bsIn)
{
	// Read info
	short characterID = 0;

	bsIn.Read(characterID);
	ShootAction* sA = ShootAction::Read(bsIn);

	// Find character by ID
	Character* c = Game::GetInstance()->FindCharacterByID(characterID);

	// Error check
	if (c == nullptr)
	{
		printf("Error: Could not find character with id: %d\n", characterID);
		return nullptr;
	}

	// Create & return action
	OverwatchShotAction* oA = new OverwatchShotAction(c, sA);
	return oA;
}
#endif

#ifdef NETWORK_SERVER
void OverwatchShotAction::Write(RakNet::BitStream & bs)
{
	// Write character index
	bs.Write(m_owner->GetID());

	// Write target tile
	m_shoot->Write(bs);
}
#endif
