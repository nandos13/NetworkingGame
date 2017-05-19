#pragma once
#include "Character.h"
#include "GameAction.h"

#include <unordered_map>
#include <list>

using namespace JakePerry;
class Squad
{
private:

	std::unordered_map<short, Character*> m_squaddies;

public:
	Squad();
	~Squad();

	void AddMember(Character* c);
	void ClearMembers();

	void StartTurn();

	Character* FindCharacter(const MapVec3 pos) const;
	Character* FindCharacter(const short id) const;

	std::list<Character*> GetAllCharacters() const;
	std::list<Character*> GetSelectableCharacters() const;

#ifdef NETWORK_SERVER

	void QueryOverwatch(GameAction* action, Character* mover, TileMap& map);

#endif

#ifndef NETWORK_SERVER
	void Draw() const;
#endif
};

