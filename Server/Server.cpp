#include "Server.h"

#ifdef NETWORK_SERVER

#include <iostream>
#include <string>
#include <MessageIdentifiers.h>
#include <BitStream.h>
#include "GameMessages.h"
#include <thread>
#include "Game.h"
#include "MovementAction.h"


void Server::Setup()
{
	// Seed random functionality
	srand((unsigned int)time(NULL));

	// Initialize the game instance
	m_game = Game::GetInstance();
}

const Server::ConnectionInfo * Server::GetClientInfo(std::string address) const
{
	for (auto& iter = m_clientConnections.cbegin(); iter != m_clientConnections.cend(); iter++)
	{
		if (iter->second->GetAddress() == address)
			return iter->second;
	}

	return nullptr;
}

const Server::ConnectionInfo * Server::GetClientInfo(unsigned int id) const
{
	auto& iter = m_clientConnections.find(id);

	if (iter != m_clientConnections.cend())
		return iter->second;
	return nullptr;
}

const std::string Server::GetClientAddress(const ConnectionInfo * connection) const
{
	if (connection != nullptr)
		return connection->GetAddress();
	return "";
}

const std::vector<Server::ConnectionInfo*> Server::GetConnections() const
{
	std::vector<ConnectionInfo*> returnList;
	for (auto& iter = m_clientConnections.cbegin(); iter != m_clientConnections.cend(); iter++)
	{
		returnList.push_back((*iter).second);
	}

	return returnList;
}

void Server::SendNewClientID(RakNet::RakPeerInterface * pPeerInterface, RakNet::SystemAddress & address)
{
	// TODO: Send bool for spectator mode if two players are already connected
	RakNet::BitStream bs;
	bs.Write((RakNet::MessageID)GameMessages::ID_SERVER_SET_CLIENT_ID);

	std::string address_str(address.ToString());

	const ConnectionInfo* c = GetClientInfo(address_str);
	if (c == nullptr)
	{
		// This is a new connection
		printf("Connection is a new client.\n");
		ConnectionInfo* newC = new ConnectionInfo(nextClientID, address_str);
		m_clientConnections[nextClientID] = newC;

		// Write the new client's ID
		bs.Write(nextClientID);
		nextClientID++;

		// Write spectator-mode state (true == playing, false == forced spectator)
		if (newC->GetID() == 0 || newC->GetID() == 1)
			bs.Write(true);
		else
			bs.Write(false);
	}
	else
	{
		// This client has connected before
		printf("Previous client (%i) reconnecting.\n", c->GetID());

		// Write the client's ID
		bs.Write(c->GetID());

		// Write spectator-mode state (true == playing, false == forced spectator)
		if (c->GetID() == 0 || c->GetID() == 1)
			bs.Write(true);
		else
			bs.Write(false);
	}
	// TODO: address says it may not be reliable. Look into: will this treat two connections on the same pc as the same client? etc.

	pPeerInterface->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, address, false);
}

/* Used to initialize the game when a new client connects. */
void Server::SendGameData(RakNet::RakPeerInterface * pPeerInterface, RakNet::SystemAddress & address)
{
	if (m_game != nullptr)
	{
		RakNet::BitStream bs;
		bs.Write((RakNet::MessageID)GameMessages::ID_SERVER_INITIALISE_GAME);

		m_game->Write(bs);

		pPeerInterface->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, address, false);
	}
}

void Server::HandleClientShoot(RakNet::RakPeerInterface * pPeerInterface, RakNet::Packet * packet)
{
	printf("Receiving request to shoot.\n");
	// Find sender connection
	std::string senderAddress = packet->systemAddress.ToString();
	auto connectionVec = GetConnections();
	ConnectionInfo* sender = ConnectionInfo::FindClient(connectionVec, senderAddress);

	if (sender != nullptr && m_game != nullptr)
	{
		// Verify the sender is currently playing
		unsigned int senderID = sender->GetID();

		if (m_game->IsPlayersTurn(senderID))
		{
			// Get shoot command data
			RakNet::BitStream bsIn(packet->data, packet->length, false);
			bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
			short characterID;
			bsIn.Read(characterID);
			MapVec3 targetTile;
			bsIn.Read(targetTile);

			// Create a new shoot action
			Character* victim = m_game->FindCharacterAtCoords(targetTile);
			if (victim != nullptr)
			{
				GameAction* action = nullptr;
				action = m_game->CreateShootAction(characterID, victim->GetID());

				if (action == nullptr) { return; };

				// Send action to all clients
				RakNet::BitStream bs;
				bs.Write((RakNet::MessageID)GameMessages::ID_SERVER_SEND_ACTION);
				action->Write(bs);

				// Send bitstream
				pPeerInterface->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
			}
		}
	}
}

void Server::HandleClientMove(RakNet::RakPeerInterface * pPeerInterface, RakNet::Packet* packet)
{
	printf("Receiving request to move a character.\n");
	// Find sender connection
	std::string senderAddress = packet->systemAddress.ToString();
	auto connectionVec = GetConnections();
	ConnectionInfo* sender = ConnectionInfo::FindClient(connectionVec, senderAddress);
	
	if (sender != nullptr && m_game != nullptr)
	{
		// Verify the sender is currently playing
		unsigned int senderID = sender->GetID();

		if (m_game->IsPlayersTurn(senderID))
		{
			// Get move command data
			RakNet::BitStream bsIn(packet->data, packet->length, false);
			bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
			short characterID;
			bsIn.Read(characterID);
			MapVec3 destination;
			bsIn.Read(destination);

			// Process move command on the server-side game
			GameAction* action;
			action = m_game->CreateMoveAction(characterID, destination);

			// Send action back to all clients
			if (action == nullptr) { return; };

			RakNet::BitStream bs;
			bs.Write((RakNet::MessageID)GameMessages::ID_SERVER_SEND_ACTION);
			action->Write(bs);

			// Send bitstream
			pPeerInterface->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
		}
		else
			printf("Not player's turn.\n");
	}
}

void Server::HandleClientHunker(RakNet::RakPeerInterface * pPeerInterface, RakNet::Packet * packet)
{
	printf("Receiving request to hunker a character.\n");
	// Find sender connection
	std::string senderAddress = packet->systemAddress.ToString();
	auto connectionVec = GetConnections();
	ConnectionInfo* sender = ConnectionInfo::FindClient(connectionVec, senderAddress);

	if (sender != nullptr && m_game != nullptr)
	{
		// Verify the sender is currently playing
		unsigned int senderID = sender->GetID();

		if (m_game->IsPlayersTurn(senderID))
		{
			// Get move command data
			RakNet::BitStream bsIn(packet->data, packet->length, false);
			bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
			short characterID;
			bsIn.Read(characterID);

			// Process move command on the server-side game
			GameAction* action;
			action = m_game->CreateHunkerAction(characterID);

			// Send action back to all clients
			if (action == nullptr) 
			{
				printf("Warning: Client requested a hunker-down action on a character which is not in cover.\n");
				return; 
			}

			RakNet::BitStream bs;
			bs.Write((RakNet::MessageID)GameMessages::ID_SERVER_SEND_ACTION);
			action->Write(bs);

			// Send bitstream
			pPeerInterface->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
		}
		else
			printf("Not player's turn.\n");
	}
}

Server::Server()
{
	Setup();
}

Server::~Server()
{
}

void Server::Run()
{
	const unsigned short PORT = 5456;
	RakNet::RakPeerInterface* pPeerInterface = nullptr;

	// Start up the server, and start it listening to clients
	std::cout << "Starting up the server..." << std::endl;

	// Initialize the Raknet peer interface first
	pPeerInterface = RakNet::RakPeerInterface::GetInstance();

	// Create a socket descriptor to describe this connection
	RakNet::SocketDescriptor sd(PORT, 0);

	// Now call startup - max of 32 connections, on the assigned port
	pPeerInterface->Startup(10, &sd, 1);
	pPeerInterface->SetMaximumIncomingConnections(10);

	RakNet::Packet* packet = nullptr;

	// Start a new thread for client pings
	//std::thread pingThread(SendClientPing, pPeerInterface);

	// Handle connection info here
	HandleConnections(pPeerInterface, packet);
}

void Server::HandleConnections(RakNet::RakPeerInterface* pPeerInterface, RakNet::Packet* packet)
{
	while (true)
	{
		for (packet = pPeerInterface->Receive(); packet;
			pPeerInterface->DeallocatePacket(packet), packet = pPeerInterface->Receive())
		{
			switch (packet->data[0])
			{
			case ID_NEW_INCOMING_CONNECTION:
				std::cout << "A connection is incoming.\n";
				SendNewClientID(pPeerInterface, packet->systemAddress);
				SendGameData(pPeerInterface, packet->systemAddress);
				break;
			case ID_DISCONNECTION_NOTIFICATION:
				std::cout << "A client has disconnected.\n";
				break;
			case ID_CONNECTION_LOST:
				std::cout << "A client lost connection.\n";
				break;
			case ID_CLIENT_SHOOT:
				HandleClientShoot(pPeerInterface, packet);
				break;
			case ID_CLIENT_MOVE:
				HandleClientMove(pPeerInterface, packet);
				break;
			case ID_CLIENT_HUNKER:
				HandleClientHunker(pPeerInterface, packet);
				break;

			default:
				std::cout << "Received a message with unknown id: " << packet->data[0];
				break;
			}
		}
	}
}

void Server::SendClientPing(RakNet::RakPeerInterface * pPeerInterface)
{
	while (true)
	{
		RakNet::BitStream bs;
		bs.Write((RakNet::MessageID)GameMessages::ID_SERVER_TEXT_MESSAGE);
		bs.Write("Ping!");

		pPeerInterface->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}


#endif