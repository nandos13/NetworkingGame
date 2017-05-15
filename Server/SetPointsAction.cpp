#include "SetPointsAction.h"
#include "Character.h"
#include "Game.h"



void SetPointsAction::_Execute(float dTime)
{
	printf("Setting character's action points to: %d.\n", m_newPoints);
	m_owner->SetActionPoints(m_newPoints);
	CompleteSelf();
}

SetPointsAction::SetPointsAction(Character * owner, const unsigned int points)
	: BaseAction(owner), m_newPoints(points){
	m_actionType = 5;
}

SetPointsAction::~SetPointsAction()
{
}

#ifndef NETWORK_SERVER
SetPointsAction * SetPointsAction::Read(RakNet::BitStream & bsIn)
{
	// Read info
	short characterID = 0;
	unsigned int pointsVal = 0;

	bsIn.Read(characterID);
	bsIn.Read(pointsVal);

	// Find character by ID
	Character* c = Game::GetInstance()->FindCharacterByID(characterID);

	// Error check
	if (c == nullptr)
	{
		printf("Error: Could not find character with id: %d\n", characterID);
		return nullptr;
	}

	// Create & return action
	SetPointsAction* spA = new SetPointsAction(c, pointsVal);
	return spA;
}
#endif

#ifdef NETWORK_SERVER
void SetPointsAction::Write(RakNet::BitStream & bs)
{
	// Write character index
	bs.Write(m_owner->GetID());

	// Write new point value
	bs.Write(m_newPoints);
}
#endif
