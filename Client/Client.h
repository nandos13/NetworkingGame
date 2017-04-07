#pragma once

#include "Application.h"

#include <RakPeerInterface.h>
#include <glm/mat4x4.hpp>

#include "Camera.h"
#include "ClientSideGameManager.h"

struct MapVec3;

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
	Game* m_game;

	/* Member functions */

	glm::mat4 GetCameraTransform();

	/* Networking functions */
	void handleNetworkConnection();
	void initialiseClientConnection();
	void handleNetworkMessages();

	void sendCharacterShoot(short characterID, MapVec3 target);
	void sendCharacterMove(short characterID, MapVec3 destination);

	/* Networking varibles */
	RakNet::RakPeerInterface* m_pPeerInterface;
	const char* IP = "127.0.0.1";
	const unsigned short PORT = 5456;
};