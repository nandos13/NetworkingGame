#include "Camera.h"

#ifndef NETWORK_SERVER

#include <glm\ext.hpp>
#include "Input.h"
#include <Gizmos.h>


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

void Camera::WrapThetaTo360()
{
	float thetaDecimal = m_theta - floorf(m_theta);
	int thetaInt = (int)m_theta;
	thetaInt = thetaInt % 360;
	m_theta = (float)thetaInt + thetaDecimal;
	if (m_theta < 0)
	{
		m_theta -= 1;
		m_theta += 360;
	}
}

Camera::Camera()
{
	m_position = glm::vec3(0);
	m_currentLookTarget = glm::vec3(0);
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

void Camera::Update(float deltaTime, glm::vec3& lookTarget, bool& lockMovement, const float rotateTo, bool& lockRotation, const float rotateSpeed, const int windowWidth, const int windowHeight)
{
	// Lerp rotation
	{
		WrapThetaTo360();

		float camLerpAmount = deltaTime * rotateSpeed * 100;

		if (m_theta != rotateTo)
		{
			float turnAngle = rotateTo - m_theta;

			float turnAngleDecimal = turnAngle - floorf(turnAngle);
			int turnAngleInt = (int)turnAngle;
			turnAngleInt = turnAngleInt % 360;
			turnAngle = (float)turnAngleInt + turnAngleDecimal;
			if (turnAngle < 0)
				turnAngle += 360;

			if (fabs(turnAngle) < camLerpAmount)
			{
				// End lerp
				m_theta = rotateTo;
				lockRotation = false;
			}
			else
			{
				if ((turnAngle < 0 && turnAngle > -180) || turnAngle > 180)
					camLerpAmount *= -1.0f;
				//if ((turnAngle < 0 && fabs(turnAngle) <= 180) || fabs(turnAngle) > 180)
				//	camLerpAmount *= -1.0f;

				m_theta += camLerpAmount;
			}
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
	const float camMoveSpeed = 15.0f;
	const float camLerpTime = 0.35f;
	if (!lockMovement)
	{
		// TODO: Re-enable edge scrolling. Maybe accept several pixels?
		if (input->isKeyDown(aie::INPUT_KEY_W) /*|| mouseY == windowHeight*/)
			m_currentLookTarget += forwardVec	* deltaTime * camMoveSpeed;
		if (input->isKeyDown(aie::INPUT_KEY_S) /*|| mouseY == 0*/)
			m_currentLookTarget += -forwardVec	* deltaTime * camMoveSpeed;
		if (input->isKeyDown(aie::INPUT_KEY_A) /*|| mouseX == 0*/)
			m_currentLookTarget += -right()		* deltaTime * camMoveSpeed;
		if (input->isKeyDown(aie::INPUT_KEY_D) /*|| mouseX == windowWidth*/)
			m_currentLookTarget += right()		* deltaTime * camMoveSpeed;

		lookTarget = m_currentLookTarget;
	}
	else
	{
		// Movement input is locked. Lerp position until camera is looking at the intended target position

		if (m_lerpSpeed <= 0)
			m_lerpSpeed = (glm::distance(lookTarget, m_currentLookTarget)) / camLerpTime;

		glm::vec3 moveDistance = lookTarget - m_currentLookTarget;
		glm::vec3 moveDir = glm::normalize(moveDistance);
		glm::vec3 moveVector = moveDir * m_lerpSpeed * deltaTime;

		if (glm::length(moveVector) > glm::length(moveDistance) || m_lerpSpeed <= 0)
		{
			// Movement hits it's target. Unlock movement
			m_currentLookTarget = lookTarget;
			lockMovement = false;
			m_lerpSpeed = 0;
		}
		else
			m_currentLookTarget += moveVector;
	}

	glm::vec3 camOffset = glm::vec3( sin(glm::radians(m_theta - 90) ), 1, cos( glm::radians(m_theta + 90)) );
	const float camDistance = 5.0f;
	camOffset *= camDistance;
	m_position = m_currentLookTarget + camOffset;

	// Adjust phi angle to point at target tile
	{
		glm::vec3 camToTile = glm::normalize(m_currentLookTarget - m_position);
		glm::vec3 rightVec = glm::cross(forwardVec, glm::vec3(0, 1, 0));

		// Get angle from horizontal to the target & convert to degrees
		float anglePhi = glm::orientedAngle(camToTile, forwardVec, rightVec);
		anglePhi = glm::degrees(anglePhi);
		anglePhi = fabs(anglePhi) * -1.0f;
		m_phi = anglePhi;
	}
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

glm::vec3 Camera::Get3DPointFromScreenSpace(GLFWwindow* window, const unsigned int width, const unsigned int height) const
{
	double x = 0, y = 0;
	glfwGetCursorPos(window, &x, &y);
	glm::vec3 windowCoords = glm::vec3(x, height - y, 0);
	glm::vec4 viewport = glm::vec4(0.0f, 0.0f, width, height);
	return glm::unProject(windowCoords, GetViewMatrix(), GetProjectionMatrix(width, height), viewport);
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


#endif