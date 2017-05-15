#include "MovementAction.h"
#include "Character.h"
#include "Game.h"



#ifndef NETWORK_SERVER
// Client-side execution
void MovementAction::_Execute(float dTime)
{
	if (m_lerpSpeed <= 0)
	{
		float tileScale = Game::GetMapTileScale();
		float x = 0, y = 0, z = 0;

		// Get owner's position
		MapVec3 ownerTilePos = m_owner->GetPosition();
		MapVec3::GetTileWorldCoords(x, y, z, ownerTilePos, tileScale);
		glm::vec3 ownerPos = glm::vec3(x, y, z);

		MapVec3::GetTileWorldCoords(x, y, z, m_destination, tileScale);
		glm::vec3 destinationPos = glm::vec3(x, y, z);

		float distance = glm::distance(ownerPos, destinationPos);
		m_lerpSpeed = distance / 0.8f;
	}

	// Move character
	if (m_owner->Move(m_destination, m_lerpSpeed, dTime))
	{
		m_lerpSpeed = 0.0f;
		CompleteSelf();
	}
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
	m_actionType = 1;

	m_destination = destination;

#ifndef NETWORK_SERVER
	m_lerpSpeed = 0.0f;
#endif
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
	MovementAction* mA = new MovementAction(c, destination);
	return mA;
}
#endif

#ifdef NETWORK_SERVER
void MovementAction::Write(RakNet::BitStream & bs)
{
	// Write character index
	bs.Write(m_owner->GetID());

	// Write destination
	m_destination.Write(bs);
}
#endif
