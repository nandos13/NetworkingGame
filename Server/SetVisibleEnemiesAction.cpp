#include "SetVisibleEnemiesAction.h"
#include "Character.h"
#include "Game.h"



void SetVisibleEnemiesAction::_Execute(float dTime)
{
	if (m_owner != nullptr)
	{
		m_owner->SetVisibleEnemyList(m_visibleEnemies);
	}
	CompleteSelf();
}

SetVisibleEnemiesAction::SetVisibleEnemiesAction(Character* owner, const std::list<Character*> enemies)
	: BaseAction(owner), m_visibleEnemies(enemies) {
	m_actionType = 6;
}

SetVisibleEnemiesAction::~SetVisibleEnemiesAction()
{
}

#ifndef NETWORK_SERVER
SetVisibleEnemiesAction * SetVisibleEnemiesAction::Read(RakNet::BitStream & bsIn)
{
	Game* game = Game::GetInstance();

	// Read character index
	short characterID = 0;
	bsIn.Read(characterID);

	Character* owner = game->FindCharacterByID(characterID);

	// Read number of enemies
	int visibleAmount = 0;
	bsIn.Read(visibleAmount);

	std::list<Character*> visibleEnemies;

	for (int i = 0; i < visibleAmount; i++)
	{
		// Read visible enemy's index
		short enemyID = 0;
		bsIn.Read(enemyID);

		// Find character via this index
		Character* enemy = game->FindCharacterByID(enemyID);

		if (enemy != nullptr)
			visibleEnemies.push_back(enemy);
	}

	if (owner != nullptr)
	{
		SetVisibleEnemiesAction* sveA = new SetVisibleEnemiesAction(owner, visibleEnemies);
		return sveA;
	}
	return nullptr;
}
#endif

#ifdef NETWORK_SERVER
void SetVisibleEnemiesAction::Write(RakNet::BitStream & bs)
{
	// Write character index
	bs.Write(m_owner->GetID());

	// Write number of enemies visible
	bs.Write((int)m_visibleEnemies.size());

	// Iterate through & write each visible enemy's index
	for (auto& iter = m_visibleEnemies.begin(); iter != m_visibleEnemies.end(); iter++)
	{
		if ((*iter) == nullptr)
			bs.Write(-1);
		else
			bs.Write((*iter)->GetID());
	}
}
#endif
