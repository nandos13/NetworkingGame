#pragma once

#include "BaseAction.h"

#include <list>

/**
 * Game-end class containing a list of Actions.
 * These actions are simulated in order on the game & server.
 */
class GameAction
{
protected:
	std::list<BaseAction*>				m_queue;
	std::list<BaseAction*>::iterator	m_iter;

	bool m_completed;
	void CompleteSelf();

public:
	GameAction();
	~GameAction();

	bool IsCompleted();

	void Execute(float dTime);

	void Reset();
	void AddToQueue(BaseAction* a);

#ifdef NETWORK_SERVER
	std::list<BaseAction*> * GetActionQueue();
#endif
};

