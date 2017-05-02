#include "GameAction.h"

#include "MovementAction.h"
#include "ShootAction.h"
#include "OverwatchShotAction.h"

#include <BitStream.h>
#include <typeinfo>



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

#ifndef NETWORK_SERVER
void GameAction::Read(RakNet::BitStream & bsIn)
{
	// Read action queue size
	unsigned int queueSize = 0;
	bsIn.Read(queueSize);

	for (unsigned int i = 0; i < queueSize; i++)
	{
		// Read action type
		char* typeName = nullptr;
		bsIn.Read(typeName);

		BaseAction* a = nullptr;

		// Use the correct Read function depending on the type.
		if (typeName == "MovementAction") { a = MovementAction::Read(bsIn); }
		else if (typeName == "ShootAction") { a = ShootAction::Read(bsIn); }
		else if (typeName == "OverwatchShotAction") { a = OverwatchShotAction::Read(bsIn); }
		else { printf("Error: Action type did not match any expected types.\n"); }

		// Add the action to the queue
		m_queue.push_back(a);

		// TODO: Does typeName need to be deleted here? Not sure if bsIn.Read(typeName) allocates memory
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
		/* Write action type here.
		* This is needed when reading, as each action type has it's own Read implementation.
		* Does not seem to be the best way to achieve this. :( Will have to look into it later.
		*/
		std::string typeName = typeid(&(iter)).name();
		bs.Write(typeName.c_str());

		(*iter)->Write(bs);
	}
}
#endif
