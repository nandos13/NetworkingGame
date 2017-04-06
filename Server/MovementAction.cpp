#include "MovementAction.h"
#include "Character.h"



void MovementAction::_Execute(float dTime)
{
	if (m_owner != nullptr)
	{
		// Move character
		m_owner->Move(m_destination, dTime);

		// Check if move is complete
		if (m_owner->GetPosition() == m_destination)
			m_completed = true;
	}
	else
		NoOwnerError();
}

MovementAction::MovementAction(Character * owner, MapVec3 destination) : GameAction(owner)
{
	m_destination = destination;
}

MovementAction::~MovementAction()
{
}
