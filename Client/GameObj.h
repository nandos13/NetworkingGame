#pragma once

#include <glm\ext.hpp>

struct MapVec3;

class GameObj
{
private:

	glm::vec3 m_position;

public:
	GameObj();
	~GameObj();

	bool LerpMove(const float x, const float y, const float z, const float speed, const float dTime);
	bool LerpMove(const glm::vec3 destination, const float speed, const float dTime);

	void GetWorldPosition(float& x, float& y, float& z) const;
	void SetPosition(const glm::vec3 position);
	void SetPosition(const float x, const float y, const float z);

	void Draw() const;
};

