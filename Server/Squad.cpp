#include "Squad.h"



Squad::Squad()
{
}


Squad::~Squad()
{
}

void Squad::StartTurn()
{
	std::unordered_map<short, Character*>::iterator iter;
	for (iter = m_squaddies.begin(); iter != m_squaddies.end(); iter++)
	{
		(*iter).second->ResetActionPoints();
	}
	// TODO
}

#ifdef NETWORK_SERVER
void Squad::QueryOverwatch(GameAction* action, Character * mover)
{
	if (mover != nullptr)
	{
		std::unordered_map<short, Character*>::iterator iter;
		for (iter = m_squaddies.begin(); iter != m_squaddies.end(); iter++)
		{
			(*iter).second->QueryOverwatch(action, mover);
		}
	}
}
#endif

#ifndef NETWORK_SERVER
void Squad::Draw()
{
	// Loop through & draw all characters
	std::unordered_map<short, Character*>::iterator iter;
	for (iter = m_squaddies.begin(); iter != m_squaddies.end(); iter++)
	{
		(*iter).second->Draw();
	}
}
#endif
