#include "GameObj.h"
#include <Gizmos.h>

#include "TileMap.h"



GameObj::GameObj()
{
}

GameObj::~GameObj()
{
}

bool GameObj::LerpMove(const float x, const float y, const float z, const float speed, const float dTime)
{
	glm::vec3 dest = glm::vec3(x, y, z);
	return LerpMove(dest, speed, dTime);
}

bool GameObj::LerpMove(const glm::vec3 destination, const float speed, const float dTime)
{
	// Get vector from current pos to destination
	glm::vec3 moveDistance = destination - m_position;
	glm::vec3 moveDir = glm::normalize(moveDistance);
	glm::vec3 moveVector = moveDir * speed * dTime;

	if (glm::length(moveVector) > glm::length(moveDistance) || speed <= 0)
	{
		// Movement reached destination
		m_position = destination;
		return true;
	}
	
	m_position += moveVector;
	return false;
}

void GameObj::GetWorldPosition(float & x, float & y, float & z) const
{
	x = m_position.x;
	y = m_position.y;
	z = m_position.z;
}

void GameObj::SetPosition(const glm::vec3 position)
{
	m_position = position;
}

void GameObj::SetPosition(const float x, const float y, const float z)
{
	glm::vec3 pos = glm::vec3(x, y, z);
	SetPosition(pos);
}

void GameObj::Draw() const
{
	// TODO: Probably needs to take in camera matrix, etc
	aie::Gizmos::addSphere(m_position, 0.4f, 8, 10, glm::vec4(1, 0, 0, 0.5f));
}
