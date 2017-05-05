#pragma once
#include <glm\glm.hpp>

class Camera
{
private:
	glm::vec3	m_position;
	float		m_fov;
	float		m_phi;
	float		m_theta;
	float		m_nearPlane;
	float		m_farPlane;

	float		getFovY(const unsigned int width, const unsigned int height) const;
	glm::vec3	forward() const;
	glm::vec3	right() const;
	glm::vec3	up() const;

	void clampPhi();

public:
	Camera();
	Camera(glm::vec3 pos, glm::vec2 euler, float fov = 70);
	~Camera();

	void Update(float deltaTime);

	// Set functions
	void SetViewAngle(float phi, float theta);
	void SetViewAngle(glm::vec2 phiTheta);
	void SetPosition(glm::vec3 pos);
	void SetFov(float fov);

	// Get functions
	glm::mat4	GetProjectionMatrix(const unsigned int w, const unsigned int h) const;
	glm::mat4	GetViewMatrix() const;
	glm::mat4	GetMVP(const unsigned int w, const unsigned int h) const;
	glm::vec3	GetPosition() const;
	glm::vec2	GetViewAngle() const;
	float		GetFov() const;
};

