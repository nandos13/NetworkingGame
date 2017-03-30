#include "TileMap.h"



TileMap::TileMap(unsigned int width, unsigned int height)
{
	m_width = (width > m_minWidth) ? width : m_minWidth;
	m_height = (height > m_minHeight) ? height : m_minHeight;
	m_tiles.resize( (size_t)(m_width*m_height) );
}


TileMap::~TileMap()
{
}
