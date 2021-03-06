#pragma once

#ifndef NETWORK_SERVER

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

	int GetID() const;

	void sendCharacterShoot(short characterID, MapVec3 target) const;
	void sendCharacterMove(short characterID, MapVec3 destination) const;
	void sendCharacterHunker(short characterID) const;
	void sendCharacterReload(short characterID) const;

protected:
	
	/* Member variables */
	
	Camera m_cam;
	ClientSideGameManager* m_gm;
	Game* m_game;

	int m_myID;

	/* Member functions */

	glm::mat4 GetCameraTransform() const;

	/* Networking functions */
	void handleNetworkConnection();
	void initialiseClientConnection();
	void handleNetworkMessages();

	void ReceiveClientID(RakNet::Packet* packet);
	void ReceiveGameInfo(RakNet::Packet* packet);
	void ReceiveAction(RakNet::Packet* packet);

	/* Networking varibles */
	RakNet::RakPeerInterface* m_pPeerInterface;
	const char* IP = "127.0.0.1";	// TODO: Implement method for changing IP to connect from two different PCs
	const unsigned short PORT = 5456;
};


#endif