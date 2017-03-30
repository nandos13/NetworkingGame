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

	float		getFovY(unsigned int width, unsigned int height);
	glm::vec3	forward();
	glm::vec3	right();
	glm::vec3	up();

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
	glm::mat4	GetProjectionMatrix(unsigned int w, unsigned int h);
	glm::mat4	GetViewMatrix();
	glm::vec3	GetPosition();
	glm::vec2	GetViewAngle();
	float		GetFov();
};

