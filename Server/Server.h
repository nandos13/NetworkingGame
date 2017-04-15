#pragma once

#ifdef NETWORK_SERVER

#include <RakPeerInterface.h>
#include "Game.h"

#include <map>

class Server
{
protected:
	void Setup();

	// Connection info
	int nextClientID = 1;
	std::map<int, const char*> m_clientConnections;
	void SendNewClientID(RakNet::RakPeerInterface* pPeerInterface, RakNet::SystemAddress& address);
	void SendGameData(RakNet::RakPeerInterface* pPeerInterface, RakNet::SystemAddress& address);

	void HandleClientShoot(RakNet::Packet* packet);
	void HandleClientMove(RakNet::Packet* packet);

	// Game instance
	Game* m_game;

public:
	Server();
	~Server();

	void Run();
	void HandleConnections(RakNet::RakPeerInterface * pPeerInterface, RakNet::Packet * packet);
	static void SendClientPing(RakNet::RakPeerInterface * pPeerInterface);
};


#endif