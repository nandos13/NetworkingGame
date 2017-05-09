#include "Camera.h"
#include <glm\ext.hpp>
#include "Input.h"


float Camera::getFovY(const unsigned int width, const unsigned int height) const
{
	float asp = (float)width / (float)height;
	float fovXRad = glm::radians(m_fov);
	return ( atan(tan(fovXRad / 2) / asp) * 2 );
}

glm::vec3 Camera::forward() const
{
	float thetaR = glm::radians(m_theta);
	float phiR = glm::radians(m_phi);
	glm::vec3 forward(cos(phiR)*cos(thetaR), sin(phiR), cos(phiR)*sin(thetaR));
	return forward;
}

glm::vec3 Camera::right() const
{
	return glm::cross(forward(), glm::vec3(0, 1, 0));
}

glm::vec3 Camera::up() const
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

void Camera::Update(float deltaTime, glm::vec3& lookTarget, bool& lockMovement, const float rotateTo, const float rotateSpeed, const int windowWidth, const int windowHeight)
{
	// Lerp rotation
	{
		float camLerpAmount = deltaTime * rotateSpeed;
		if ((m_theta - rotateTo) < camLerpAmount)
			m_theta = rotateTo;
		else
		{
			if (m_theta - rotateTo > 0)
				camLerpAmount *= -1.0f;

			m_theta += camLerpAmount;
		}
	}

	aie::Input* input = aie::Input::getInstance();
	float thetaR = glm::radians(m_theta);
	float phiR = glm::radians(m_phi);
	int mouseX = 0, mouseY = 0;
	input->getMouseXY(&mouseX, &mouseY);
	glm::vec2 mousePos = glm::vec2(mouseX, mouseY);

	/** 
	 * Rather than using the camera's forward direction, we use a direction
	 * which ignores phi. This causes the camera to glide across the floor at
	 * the same height rather than following it's phi angle into the ground.
	 */
	glm::vec3 forwardVec(cos(0)*cos(thetaR), sin(0), cos(0)*sin(thetaR));

	// WASD Key movement
	const float camMoveSpeed = 2.5f;
	if (!lockMovement)
	{
		if (input->isKeyDown(aie::INPUT_KEY_W) || mouseY == windowHeight)
			m_currentLookTarget += forwardVec	* deltaTime * camMoveSpeed;
		if (input->isKeyDown(aie::INPUT_KEY_S) || mouseY == 0)
			m_currentLookTarget += -forwardVec	* deltaTime * camMoveSpeed;
		if (input->isKeyDown(aie::INPUT_KEY_A) || mouseX == 0)
			m_currentLookTarget += -right()		* deltaTime * camMoveSpeed;
		if (input->isKeyDown(aie::INPUT_KEY_D) || mouseX == windowWidth)
			m_currentLookTarget += right()		* deltaTime * camMoveSpeed;

		lookTarget = m_currentLookTarget;
	}
	else
	{
		// Movement input is locked. Lerp position until camera is looking at the intended target position

		float moveLerpAmount = deltaTime * camMoveSpeed;
		glm::vec3 moveVector = lookTarget - m_currentLookTarget;
		glm::vec3 moveDir = glm::normalize(moveVector) * moveLerpAmount;

		if (glm::length(moveDir) > glm::length(moveVector))
		{
			// Movement hits it's target. Unlock movement
			m_currentLookTarget = lookTarget;
			lockMovement = false;
		}
		else
			m_currentLookTarget += moveDir;
	}

	// TODO: Move camera to currentLookTarget plus an offset based on angle, and look at the lookTarget
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

glm::mat4 Camera::GetProjectionMatrix(const unsigned int w, const unsigned int h) const
{
	return glm::perspective(getFovY(w, h), (float)w / (float)h, m_nearPlane, m_farPlane);
}

glm::mat4 Camera::GetViewMatrix() const
{
	return glm::lookAt(m_position, m_position + forward(), glm::vec3(0,1,0));
}

glm::mat4 Camera::GetMVP(const unsigned int w, const unsigned int h) const
{
	return GetProjectionMatrix(w, h) * GetViewMatrix();
}

glm::vec3 Camera::Get3DPointFromScreenSpace(const glm::vec2 screenSpacePoint, const unsigned int width, const unsigned int height) const
{
	// TODO: Verify this method works. May need to change the z component in point3D to 1 rather than 0?
	double x = 2.0 * screenSpacePoint.x / width - 1;
	double y = 2.0 * screenSpacePoint.y / height + 1;
	glm::mat4 viewProjInverse = glm::inverse(GetProjectionMatrix(width, height) * GetViewMatrix());

	glm::vec3 point3D = glm::vec3(x, y, 0);
	return glm::vec3(viewProjInverse * glm::vec4(point3D, 1));
}

glm::vec3 Camera::GetPosition() const
{
	return m_position;
}

glm::vec3 Camera::GetForwardVec() const
{
	return forward();
}

glm::vec3 Camera::GetRightVec() const
{
	return right();
}

glm::vec3 Camera::GetUpVec() const
{
	return up();
}

glm::vec2 Camera::GetViewAngle() const
{
	return glm::vec2(m_phi, m_theta);
}

float Camera::GetFov() const
{
	return m_fov;
}
