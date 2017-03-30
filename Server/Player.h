#pragma once
#include "Character.h"
#include <vector>

class Player
{
private:
	std::vector<Character*> m_squad;	// Player's squad characters
public:
	Player();
	~Player();
};

