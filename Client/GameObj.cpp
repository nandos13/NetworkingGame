#include "GameObj.h"

#include "TileMap.h"



GameObj::GameObj()
{
}

GameObj::~GameObj()
{
}

bool GameObj::LerpMove(const float x, const float y, const float z, const float dTime)
{
	glm::vec3 dest = glm::vec3(x, y, z);
	return LerpMove(dest, dTime);
}

bool GameObj::LerpMove(const glm::vec3 destination, const float dTime)
{
	// Get vector from current pos to destination
	glm::vec3 toDest = destination - m_position;

	if (toDest.length() < dTime)
	{
		// Movement reached destination
		m_position = destination;
		return true;
	}
	
	m_position += destination * dTime;
	return false;
}

void GameObj::GetWorldPosition(float & x, float & y, float & z)
{
	x = m_position.x;
	y = m_position.y;
	z = m_position.z;
}
