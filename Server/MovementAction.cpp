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

#ifdef NETWORK_SERVER
void MovementAction::Write(RakNet::BitStream & bs)
{
}
#endif
