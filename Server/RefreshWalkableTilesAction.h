#pragma once
#include "BaseAction.h"
#include "TileMap.h"
#include <list>

class RefreshWalkableTilesAction : public BaseAction
{
protected:
	std::list<MapVec3> m_1PTileList;
	std::list<MapVec3> m_2PTileList;

	virtual void _Execute(float dTime);

public:
	RefreshWalkableTilesAction(Character* owner, std::list<MapVec3> list1P, std::list<MapVec3> list2P);
	~RefreshWalkableTilesAction();

#ifndef NETWORK_SERVER
	static RefreshWalkableTilesAction* Read(RakNet::BitStream& bsIn);
#endif

#ifdef NETWORK_SERVER
	virtual void Write(RakNet::BitStream& bs);
#endif
};

