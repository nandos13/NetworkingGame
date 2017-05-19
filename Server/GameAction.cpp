#include "GameAction.h"

#include "MovementAction.h"
#include "ShootAction.h"
#include "OverwatchShotAction.h"
#include "RefreshWalkableTilesAction.h"
#include "SetPointsAction.h"
#include "SetVisibleEnemiesAction.h"
#include "StartNewTurnAction.h"

#include <BitStream.h>



void GameAction::CompleteSelf()
{
	m_completed = true;
}

GameAction::GameAction()
{
	m_iter = m_queue.begin();
	m_completed = true;
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
	if (a != nullptr)
	{
		m_queue.push_back(a);
		if (m_queue.size() == 1)
		{
			// This was the first action to be added. Set current action
			m_iter = m_queue.begin();
		}
		m_completed = false;
	}
}

#ifndef NETWORK_SERVER
void GameAction::Read(RakNet::BitStream & bsIn)
{
	// Read action queue size
	unsigned int queueSize = 0;
	bsIn.Read(queueSize);

	for (unsigned int i = 0; i < queueSize; i++)
	{
		// Read action type
		int id = 0;
		bsIn.Read(id);

		BaseAction* a = nullptr;

		// Use the correct Read function depending on the type.
		switch (id)
		{
		case 1:		a = MovementAction::Read(bsIn);
			break;
		case 2:		a = ShootAction::Read(bsIn);
			break;
		case 3:		a = OverwatchShotAction::Read(bsIn);
			break;
		case 4:		a = RefreshWalkableTilesAction::Read(bsIn);
			break;
		case 5:		a = SetPointsAction::Read(bsIn);
			break;
		case 6:		a = SetVisibleEnemiesAction::Read(bsIn);
			break;
		case 7:		a = StartNewTurnAction::Read(bsIn);
			break;

		default:	printf("Error: Action type did not match any expected types.\n");
			break;
		}

		// Add the action to the queue
		AddToQueue(a);
	}
}
#endif

#ifdef NETWORK_SERVER
std::list<BaseAction*>* GameAction::GetActionQueue()
{
	return &m_queue;
}

void GameAction::Write(RakNet::BitStream & bs)
{
	// Write number of child actions
	bs.Write((unsigned int)m_queue.size());

	std::list<BaseAction*>::iterator iter;
	for (iter = m_queue.begin(); iter != m_queue.end(); iter++)
	{
		/* Write action-type ID here.
		 * This is needed when reading, as each action type has it's own Read implementation.
		 */
		if (*iter != nullptr)
		{
			int id = (*iter)->GetActionTypeID();
			bs.Write(id);

			(*iter)->Write(bs);
		}
	}
}
#endif
