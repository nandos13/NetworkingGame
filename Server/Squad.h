#pragma once
#include "Character.h"

#include <unordered_map>

class Squad
{
private:

	std::unordered_map<short, Character*> m_squaddies;

public:
	Squad();
	~Squad();

	//bool QueryTurnmThinggg();
};

