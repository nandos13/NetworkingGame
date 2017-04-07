#include "BaseAction.h"



// Static definitions
std::list<BaseAction*> BaseAction::m_timeManipulators;
float BaseAction::m_currentSlowestTime;

void BaseAction::CompleteSelf()
{
	m_completed = true;
	m_timeManipulators.remove(this);
}

BaseAction::BaseAction(Character * owner)
{
	m_completed = false;
	m_owner = owner;

	m_currentSlowestTime = 1.0f;
	m_thisTimeScale = 1.0f;
}

BaseAction::~BaseAction()
{
}

void BaseAction::Execute(float dTime)
{
	if (m_owner != nullptr)
		_Execute(dTime * m_currentSlowestTime);
	else
		NoOwnerError();
}

bool BaseAction::IsCompleted()
{
	return m_completed;
}

void BaseAction::Reset()
{
	m_completed = false;
	m_thisTimeScale = 1.0f;
}

void BaseAction::UpdateTimeManipulators()
{
	m_currentSlowestTime = 1.0f;

	// Loop through all manipulating actions
	for each (BaseAction* g in m_timeManipulators)
	{
		// Check if the action's time-scale is still valid
		if (g->m_completed == false)
		{
			float gTime = g->m_thisTimeScale;
			if (gTime < m_currentSlowestTime)
			{
				// Check the time scale is valid
				if (gTime >= 0.01f)
					m_currentSlowestTime = gTime;	// This is the new slowest time
			}
		}
		else
			m_timeManipulators.remove(g);
	}
}

void BaseAction::NoOwnerError()
{
	printf("Error: Action does not have an owner assigned.\n");
	m_completed = true;
}

void BaseAction::SlowTime(float timeScale)
{
	if (timeScale <= 1.0f && timeScale >= 0.01f)
	{
		// Add this action to the list of time manipulators
		BaseAction::m_timeManipulators.push_back(this);

		// Update local time scale
		m_thisTimeScale = timeScale;

		// Recalculate the current slowest time-scale, in case this is now the slowest
		UpdateTimeManipulators();
	}
}

void BaseAction::RemoveSlowTime()
{
	// Remove this action from the time manipulators
	m_timeManipulators.remove(this);

	// Reset local time-scale
	m_thisTimeScale = 1.0f;
}
