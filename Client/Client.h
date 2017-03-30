#pragma once

#include "Application.h"

#include <RakPeerInterface.h>
#include <glm/mat4x4.hpp>

class Camera;
class ClientSideGameManager;

class Client : public aie::Application {
public:

	Client();
	virtual ~Client();

	virtual bool startup();
	virtual void shutdown();

	virtual void update(float deltaTime);
	virtual void draw();

protected:
	
	/* Member variables */
	
	Camera cam;
	ClientSideGameManager gm;

	/* Member functions */

	glm::mat4 GetCameraTransform();

	/* Networking functions */
	void handleNetworkConnection();
	void initialiseClientConnection();
	void handleNetworkMessages();

	/* Networking varibles */
	RakNet::RakPeerInterface* m_pPeerInterface;
	const char* IP = "127.0.0.1";
	const unsigned short PORT = 5456;
};