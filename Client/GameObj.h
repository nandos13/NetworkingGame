#pragma once

#include <glm\ext.hpp>

struct MapVec3;

class GameObj
{
private:

	glm::vec3 m_position;
	glm::vec4 m_colour;
	float m_radius;

public:
	GameObj();
	~GameObj();

	bool LerpMove(const float x, const float y, const float z, const float speed, const float dTime);
	bool LerpMove(const glm::vec3 destination, const float speed, const float dTime);

	void GetWorldPosition(float& x, float& y, float& z) const;
	void SetPosition(const glm::vec3 position);
	void SetPosition(const float x, const float y, const float z);
	void SetColour(const glm::vec4 colour);
	void SetColour(const float r, const float g, const float b, const float a);
	void SetRadius(const float r);

	void Draw() const;
};

