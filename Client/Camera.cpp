#include "Camera.h"
#include <glm\ext.hpp>
#include "Input.h"


float Camera::getFovY(unsigned int width, unsigned int height)
{
	float asp = (float)width / (float)height;
	float fovXRad = glm::radians(m_fov);
	return ( atan(tan(fovXRad / 2) / asp) * 2 );
}

glm::vec3 Camera::forward()
{
	float thetaR = glm::radians(m_theta);
	float phiR = glm::radians(m_phi);
	glm::vec3 forward(cos(phiR)*cos(thetaR), sin(phiR), cos(phiR)*sin(thetaR));
	return forward;
}

glm::vec3 Camera::right()
{
	return glm::cross(forward(), glm::vec3(0, 1, 0));
}

glm::vec3 Camera::up()
{
	return glm::cross(right(), forward());
}

void Camera::clampPhi()
{
	if (m_phi > 70)
		m_phi = 70;
	else if (m_phi < -70)
		m_phi = -70;
}

Camera::Camera()
{
	m_position = glm::vec3(0);
	m_fov = 70;
	m_phi = 0;
	m_theta = 0;
	m_nearPlane = 0.1f;
	m_farPlane = 1000.0f;
}

Camera::Camera(glm::vec3 pos, glm::vec2 euler, float fov)
{
	m_position = pos;
	m_fov = fov;
	m_phi = euler.x;
	clampPhi();
	m_theta = euler.y;
	m_nearPlane = 0.1f;
	m_farPlane = 1000.0f;
}


Camera::~Camera()
{
}

void Camera::Update(float deltaTime)
{
	aie::Input* input = aie::Input::getInstance();
	float thetaR = glm::radians(m_theta);
	float phiR = glm::radians(m_phi);

	/** 
	 * Rather than using the camera's forward direction, we use a direction
	 * which ignores phi. This causes the camera to glide across the floor at
	 * the same height rather than following it's phi angle into the ground.
	 */
	glm::vec3 forwardVec(cos(0)*cos(thetaR), sin(0), cos(0)*sin(thetaR));

	// WASD Key movement
	const float camSpeed = 2.5f;
	if (input->isKeyDown(aie::INPUT_KEY_W))
		m_position += forwardVec	* deltaTime * camSpeed;
	if (input->isKeyDown(aie::INPUT_KEY_S))
		m_position += -forwardVec	* deltaTime * camSpeed;
	if (input->isKeyDown(aie::INPUT_KEY_A))
		m_position += -right()		* deltaTime * camSpeed;
	if (input->isKeyDown(aie::INPUT_KEY_D))
		m_position += right()		* deltaTime * camSpeed;

}

void Camera::SetViewAngle(float phi, float theta)
{
	m_phi = phi;
	clampPhi();
	m_theta = theta;
}

void Camera::SetViewAngle(glm::vec2 phiTheta)
{
	m_phi = phiTheta.x;
	clampPhi();
	m_theta = phiTheta.y;
}

void Camera::SetPosition(glm::vec3 pos)
{
	m_position = pos;
}

void Camera::SetFov(float fov)
{
	m_fov = fov;
}

glm::mat4 Camera::GetProjectionMatrix(unsigned int w, unsigned int h)
{
	return glm::perspective(getFovY(w, h), (float)w / (float)h, m_nearPlane, m_farPlane);
}

glm::mat4 Camera::GetViewMatrix()
{
	return glm::lookAt(m_position, m_position + forward(), glm::vec3(0,1,0));
}

glm::vec3 Camera::GetPosition()
{
	return m_position;
}

glm::vec2 Camera::GetViewAngle()
{
	return glm::vec2(m_phi, m_theta);
}

float Camera::GetFov()
{
	return m_fov;
}
