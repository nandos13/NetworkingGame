#pragma once

#include <list>

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

	/**
	 * A list of actions currently affecting playback time.
	 * Using the SlowTime and RemoveSlowTime methods, an action can
	 * add itself as a time-slowing action, forcing all actions to play
	 * back at the slowest time present in the list. This allows 
	 * shooting actions to activate slow motion as a character is running, etc.
	 */
	static std::list<GameAction*> m_timeManipulators;
	// Track the current slowest time scale in effect
	static float m_currentSlowestTime;

	static void UpdateTimeManipulators();

	// An action's individual time-scale. 
	float m_thisTimeScale;

	void NoOwnerError();

	void SlowTime(float timeScale);
	void RemoveSlowTime();

	virtual void _Execute(float dTime) = 0;

	// Override function to specify extra functionality when the action completes & exits
	virtual void CompleteSelf();

public:
	GameAction(Character* owner);
	virtual ~GameAction();

	void Execute(float dTime);

	bool IsCompleted();
};

