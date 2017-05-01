#include "MovementAction.h"
#include "Character.h"



#ifndef NETWORK_SERVER
// Client-side execution
void MovementAction::_Execute(float dTime)
{
	// Move character
	if (m_owner->Move(m_destination, dTime))
		CompleteSelf();
}
#endif

#ifdef NETWORK_SERVER
// Server-side execution
void MovementAction::_Execute(float dTime)
{
	// Move character
	m_owner->Move(m_destination);

	// Complete action
	CompleteSelf();

}
#endif

MovementAction::MovementAction(Character * owner, MapVec3 destination) : BaseAction(owner)
{
	m_destination = destination;
}

MovementAction::~MovementAction()
{
}

#ifndef NETWORK_SERVER
MovementAction * MovementAction::Read(RakNet::BitStream & bsIn)
{
	// Read info
	short characterID = 0;
	MapVec3 destination = MapVec3(0);

	bsIn.Read(characterID);
	bsIn.Read(destination);

	// Find character by ID
	Character* c = Game::GetInstance()->FindCharacterByID(characterID);

	// Error check
	if (c == nullptr)
	{
		printf("Error: Could not find character with id: %d\n", characterID);
		return nullptr;
	}

	// Create & return action
	MovementAction mA = new MovementAction(c, destination);
	return mA;
}
#endif

#ifdef NETWORK_SERVER
void MovementAction::Write(RakNet::BitStream & bs)
{
	// Write character index
	bs.Write(m_owner->GetID());

	// Write destination
	bs.Write((char*)&m_destination, sizeof(MapVec3));
}
#endif
