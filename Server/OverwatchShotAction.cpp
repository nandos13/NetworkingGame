#include "OverwatchShotAction.h"
#include "Character.h"



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

#ifdef NETWORK_SERVER
void OverwatchShotAction::Write(RakNet::BitStream & bs)
{
}
#endif
