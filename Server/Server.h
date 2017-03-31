#pragma once

#include <RakPeerInterface.h>
#include "Game.h"

class Server
{
protected:
	void Setup();

	// Connection info
	int nextClientID = 1;
	void sendNewClientID(RakNet::RakPeerInterface* pPeerInterface, RakNet::SystemAddress& address);

	void handleClientMove(RakNet::Packet* packet);

	// Game instance
	Game* m_game;

public:
	Server();
	~Server();

	void Run();
	void HandleConnections(RakNet::RakPeerInterface * pPeerInterface, RakNet::Packet * packet);
	static void SendClientPing(RakNet::RakPeerInterface * pPeerInterface);
};

