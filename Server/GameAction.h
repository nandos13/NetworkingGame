#pragma once

class Character;

/**
 * BASE CLASS.
 * Actions inherit from this class. 
 */
class GameAction
{
protected:
	bool m_completed;
	Character* m_owner;

	void NoOwnerError();

	virtual void _Execute(float dTime) = 0;

public:
	GameAction(Character* owner);
	virtual ~GameAction();

	void Execute(float dTime);
};

