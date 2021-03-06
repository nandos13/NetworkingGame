#include "Squad.h"



Squad::Squad()
{
}


Squad::~Squad()
{
}

void Squad::AddMember(Character * c)
{
	if (c != nullptr)
	{
		short id = c->GetID();
		m_squaddies[id] = c;
	}
	else
		printf("Error: Tried to add a character to squad using null-reference.\n");
}

void Squad::ClearMembers()
{
	m_squaddies.clear();
}

void Squad::StartTurn()
{
	std::unordered_map<short, Character*>::iterator iter;
	for (iter = m_squaddies.begin(); iter != m_squaddies.end(); iter++)
	{
		(*iter).second->ResetActionPoints();
		(*iter).second->SetHunkerState(false);
	}
	// TODO: take one turn from current debuffs, etc
	// TODO: Re-evaluate moveable tiles for each character (this should be a method in character class)
}

Character * Squad::FindCharacter(const MapVec3 pos) const
{
	std::unordered_map<short, Character*>::const_iterator iter;
	for (iter = m_squaddies.begin(); iter != m_squaddies.end(); iter++)
	{
		if (iter->second->GetPosition() == pos)
			return iter->second;
	}
	return nullptr;
}

Character * Squad::FindCharacter(const short id) const
{
	std::unordered_map<short, Character*>::const_iterator iter;
	for (iter = m_squaddies.begin(); iter != m_squaddies.end(); iter++)
	{
		if (iter->second->GetID() == id)
			return iter->second;
	}
	return nullptr;
}

std::list<Character*> Squad::GetAllCharacters() const
{
	std::list<Character*> charList;
	for (auto& iter = m_squaddies.cbegin(); iter != m_squaddies.cend(); iter++)
	{
		if (iter->second != nullptr)
			charList.push_back(iter->second);
	}

	return charList;
}

std::list<Character*> Squad::GetSelectableCharacters() const
{
	std::list<Character*> charList;
	for (auto& iter = m_squaddies.cbegin(); iter != m_squaddies.cend(); iter++)
	{
		if (iter->second != nullptr)
		{
			if (iter->second->IsSelectable())
				charList.push_back(iter->second);
		}
	}

	return charList;
}

#ifdef NETWORK_SERVER
void Squad::QueryOverwatch(GameAction* action, Character * mover, TileMap& map)
{
	if (mover != nullptr)
	{
		// Check each character in this squad
		std::unordered_map<short, Character*>::iterator iter;
		for (iter = m_squaddies.begin(); iter != m_squaddies.end(); iter++)
		{
			(*iter).second->QueryOverwatch(action, mover, map);

			if (!mover->Alive())
				break;	// Moving unit was killed
		}
	}
}
#endif

#ifndef NETWORK_SERVER
void Squad::Draw() const
{
	// Loop through & draw all characters
	std::unordered_map<short, Character*>::const_iterator iter;
	for (iter = m_squaddies.cbegin(); iter != m_squaddies.cend(); iter++)
	{
		(*iter).second->Draw();
	}
}
#endif
