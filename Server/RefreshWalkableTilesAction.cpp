#include "RefreshWalkableTilesAction.h"
#include "Character.h"
#include "Game.h"



void RefreshWalkableTilesAction::_Execute(float dTime)
{
	m_owner->Set1PointWalkableTiles(m_1PTileList);
	m_owner->Set2PointWalkableTiles(m_2PTileList);

	// Complete action
	CompleteSelf();
}

RefreshWalkableTilesAction::RefreshWalkableTilesAction(Character* owner, std::list<MapVec3> list1P, std::list<MapVec3> list2P)
	: BaseAction(owner)
{
	m_actionType = 4;

	m_1PTileList = list1P;
	m_2PTileList = list2P;
}


RefreshWalkableTilesAction::~RefreshWalkableTilesAction()
{
}

#ifndef NETWORK_SERVER
RefreshWalkableTilesAction * RefreshWalkableTilesAction::Read(RakNet::BitStream & bsIn)
{
	// Read character ID
	short characterID = 0;
	bsIn.Read(characterID);

	// Find character by ID
	Character* c = Game::GetInstance()->FindCharacterByID(characterID);

	// Error check
	if (c == nullptr)
	{
		printf("Error: Could not find character with id: %d\n", characterID);
		return nullptr;
	}

	// Read 1-point move list
	std::list<MapVec3> list1P;
	unsigned int size1P = 0;
	bsIn.Read(size1P);

	for (unsigned int i = 0; i < size1P; i++)
	{
		MapVec3 pos = MapVec3(0);
		pos.Read(bsIn);
		list1P.push_back(pos);
	}

	// Read 2-point move list
	std::list<MapVec3> list2P;
	unsigned int size2P = 0;
	bsIn.Read(size2P);

	for (unsigned int i = 0; i < size2P; i++)
	{
		MapVec3 pos = MapVec3(0);
		pos.Read(bsIn);
		list2P.push_back(pos);
	}

	RefreshWalkableTilesAction* rwtA = new RefreshWalkableTilesAction(c, list1P, list2P);
	return rwtA;
}
#endif

#ifdef NETWORK_SERVER
void RefreshWalkableTilesAction::Write(RakNet::BitStream & bs)
{
	// Write character index
	bs.Write(m_owner->GetID());

	// Write 1-point distance list
	bs.Write((unsigned int)m_1PTileList.size());
	for (auto& iter = m_1PTileList.begin(); iter != m_1PTileList.end(); iter++)
		(*iter).Write(bs);

	// Write 2-point distance list
	bs.Write((unsigned int)m_2PTileList.size());
	for (auto& iter = m_2PTileList.begin(); iter != m_2PTileList.end(); iter++)
		(*iter).Write(bs);
}
#endif
