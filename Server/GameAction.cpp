#include "GameAction.h"



void GameAction::CompleteSelf()
{
	m_completed = true;
}

GameAction::GameAction()
{
	m_iter = m_queue.begin();
}

GameAction::~GameAction()
{
}

bool GameAction::IsCompleted()
{
	return m_completed;
}

void GameAction::Execute(float dTime)
{
	if (!m_completed)
	{
		if (m_iter != m_queue.end())
		{
			// Execute current action
			(*m_iter)->Execute(dTime);

			// Advance iterator if the current action is complete
			if ((*m_iter)->IsCompleted())
				m_iter++;
		}
		else
			CompleteSelf();
	}
}

void GameAction::Reset()
{
	for each (BaseAction* a in m_queue)
		a->Reset();
	m_iter = m_queue.begin();
	m_completed = false;
}

void GameAction::AddToQueue(BaseAction * a)
{
	m_queue.push_back(a);
	m_completed = false;
}

#ifdef NETWORK_SERVER
std::list<BaseAction*>* GameAction::GetActionQueue()
{
	return &m_queue;
}

void GameAction::Write(RakNet::BitStream & bs)
{
	std::list<BaseAction*>::iterator iter;
	for (iter = m_queue.begin(); iter != m_queue.end(); iter++)
	{
		(*iter)->Write(bs);
	}
}
#endif
