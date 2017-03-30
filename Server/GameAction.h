#pragma once

/**
 * BASE CLASS.
 * Actions inherit from this class. 
 */
class GameAction
{
protected:

public:
	GameAction();
	virtual ~GameAction();

	virtual void Execute() = 0;
};

