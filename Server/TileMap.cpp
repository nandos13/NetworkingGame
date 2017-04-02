#include "TileMap.h"



TileMap::MapTile * TileMap::FindTile(MapVec3 pos)
{
	MapPlane* p = &( m_planes[pos.m_y] );
	if (p != nullptr)
	{
		return ( p->m_tiles[std::make_pair(pos.m_x, pos.m_z)] );
	}
	return nullptr;
}

TileMap::TileMap()
{
}


TileMap::~TileMap()
{
}

void TileMap::AddTile(MapVec3 pos, bool autoConnect)
{
	// Insert a plane at specified y-level if it does not exist
	m_planes.insert(std::make_pair(pos.m_y, MapPlane()));

	// Insert a tile at (x, z) in the plane
	MapTile* newTile = new MapTile(pos);

	typedef std::unordered_map<std::pair<short, short>, MapTile*> xyTile;
	std::pair<xyTile::iterator, bool> tileCreated;

	tileCreated = m_planes[pos.m_y].m_tiles.insert
				(std::make_pair(std::make_pair(pos.m_x, pos.m_z), newTile));

	// If the tile already existed
	if (tileCreated.second == false)
	{
		delete newTile;
	}
	else
	{
		// Auto-connect functionality
		if (autoConnect)
		{
			// Offsets for tiles connected in same plane. These values correspond to MAP_CONNECTION_DIR values
			MapVec3 m_offsetVecs[] = 
			{ 
				MapVec3(-1,0,0),  MapVec3(1,0,0), MapVec3(0,0,1), MapVec3(0,0,-1), 
				MapVec3(-1,0,1), MapVec3(1,0,1), MapVec3(1,0,-1), MapVec3(-1,0,-1) 
			};

			// Loop through adjacent tiles & add connections
			MapTile* temp = nullptr;
			for (int i = 0; i < 8; i++)		// 8 = number of directions in MAP_CONNECTION_DIR enum
			{
				MapVec3 offset = m_offsetVecs[i];
				temp = FindTile(offset + newTile->GetTilePos());
				if (temp != nullptr)
				{
					// Link the new tile and the adjacent tile
					temp->AddConnection(newTile);
					newTile->AddConnection(temp);

				}
			}
		}
	}
}

void TileMap::AddTile(short x, short y, short z, bool autoConnect)
{
	AddTile(MapVec3(x,y,z), autoConnect);
}

std::vector<MapVec3> TileMap::FindPath(MapVec3 from, MapVec3 to)
{
	MapTile* origin = FindTile(from);
	if (origin)
	{
		MapTile* destination = FindTile(to);
		if (destination)
		{
			// TODO: Code here
		}
	}
	return std::vector<MapVec3>();
}
