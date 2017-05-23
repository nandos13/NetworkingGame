#pragma once

#ifdef NETWORK_SERVER

#include <RakPeerInterface.h>
#include "Game.h"

#include <string>
#include <map>

class Server
{
protected:

	/**
	 * Small data structure to store info about each connection.
	 * Used to handle time-outs, etc.
	 */
	struct ConnectionInfo
	{
	private:

		unsigned int m_id;
		std::string m_address;

	public:

		ConnectionInfo(const unsigned int id, const std::string address)
			: m_id(id), m_address(address) {}

		unsigned int GetID() const
		{
			return m_id;
		}

		void SetAddress(std::string address)
		{
			m_address = address;
		}

		std::string GetAddress() const
		{
			return m_address;
		}

		static ConnectionInfo* FindClient(const std::vector<ConnectionInfo*> connections, const std::string address)
		{
			for (auto& iter = connections.cbegin(); iter != connections.cend(); iter++)
			{
				if ((*iter)->GetAddress() == address)
					return *iter;
			}

			return nullptr;
		}
	};

	void Setup();

	// Connection info
	unsigned int nextClientID = 0;
	std::map<unsigned int, ConnectionInfo*> m_clientConnections;
	const ConnectionInfo* GetClientInfo(std::string address) const;
	const ConnectionInfo* GetClientInfo(unsigned int id) const;
	const std::string GetClientAddress(const ConnectionInfo* connection) const;
	const std::vector<ConnectionInfo*> GetConnections() const;

	void SendNewClientID(RakNet::RakPeerInterface* pPeerInterface, RakNet::SystemAddress& address);
	void SendGameData(RakNet::RakPeerInterface* pPeerInterface, RakNet::SystemAddress& address);

	void HandleClientShoot	(RakNet::RakPeerInterface * pPeerInterface, RakNet::Packet* packet);
	void HandleClientMove	(RakNet::RakPeerInterface * pPeerInterface, RakNet::Packet* packet);
	void HandleClientHunker	(RakNet::RakPeerInterface * pPeerInterface, RakNet::Packet* packet);

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