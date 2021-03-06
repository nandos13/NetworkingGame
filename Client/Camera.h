#pragma once

#ifndef NETWORK_SERVER

#include <glm\glm.hpp>
#include <..\glfw\include\GLFW\glfw3.h>

class Camera
{
private:
	glm::vec3	m_position;
	glm::vec3	m_currentLookTarget;
	float		m_lerpSpeed;
	float		m_zoomDistance;

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
	void WrapThetaTo360();
	void ZoomCamera(const float deltaTime, const bool zoomOut);

public:
	Camera();
	Camera(glm::vec3 pos, glm::vec2 euler, float fov = 70);
	~Camera();

	void Update(float deltaTime, glm::vec3& lookTarget, bool& lockMovement, const glm::vec3 followTarget, bool& camPosFollow, const float rotateTo, bool& lockRotation, const float rotateSpeed, const int windowWidth, const int windowHeight);

	// Set functions
	void SetViewAngle(float phi, float theta);
	void SetViewAngle(glm::vec2 phiTheta);
	void SetPosition(glm::vec3 pos);
	void SetFov(float fov);

	// Get functions
	glm::mat4	GetProjectionMatrix(const unsigned int w, const unsigned int h) const;
	glm::mat4	GetViewMatrix() const;
	glm::mat4	GetMVP(const unsigned int w, const unsigned int h) const;
	glm::vec3	Get3DPointFromScreenSpace(GLFWwindow* window, const unsigned int width, const unsigned int height) const;
	glm::vec3	GetPosition() const;
	glm::vec3	GetForwardVec() const;
	glm::vec3	GetRightVec() const;
	glm::vec3	GetUpVec() const;
	glm::vec2	GetViewAngle() const;
	float		GetFov() const;
};


#endif