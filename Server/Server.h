#pragma once

#include <RakPeerInterface.h>

class Server
{
protected:
	void Setup();

	int nextClientID = 1;
	void sendNewClientID(RakNet::RakPeerInterface* pPeerInterface, RakNet::SystemAddress& address);
public:
	Server();
	~Server();

	void Run();
	void HandleConnections(RakNet::RakPeerInterface * pPeerInterface, RakNet::Packet * packet);
	static void SendClientPing(RakNet::RakPeerInterface * pPeerInterface);
};

