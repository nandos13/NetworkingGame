#include "TileMap.h"



TileMap::Tile * TileMap::FindTile(MapVec3 pos)
{
	return nullptr;
}

TileMap::TileMap(unsigned int width, unsigned int height)
{
	m_width = (width > m_minWidth) ? width : m_minWidth;
	m_height = (height > m_minHeight) ? height : m_minHeight;
	m_tiles.resize( (size_t)(m_width*m_height) );
}


TileMap::~TileMap()
{
}

std::vector<MapVec3> TileMap::FindPath(MapVec3 from, MapVec3 to)
{
	Tile* origin = FindTile(from);
	if (origin)
	{
		Tile* destination = FindTile(to);
		if (destination)
		{
			// TODO: Code here
		}
	}
	return std::vector<MapVec3>();
}
