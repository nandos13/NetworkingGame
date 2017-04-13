#include "Server.h"
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
	srand(time(NULL));
	// Initialize the game instnace
	m_game = new Game();
}

void Server::sendNewClientID(RakNet::RakPeerInterface * pPeerInterface, RakNet::SystemAddress & address)
{
	RakNet::BitStream bs;
	bs.Write((RakNet::MessageID)GameMessages::ID_SERVER_SET_CLIENT_ID);
	bs.Write(nextClientID);
	nextClientID++;

	pPeerInterface->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, address, false);
}

void Server::handleClientShoot(RakNet::Packet * packet)
{
	// Get shoot command data
	RakNet::BitStream bsIn(packet->data, packet->length, false);
	bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
	short characterID;
	bsIn.Read(characterID);
	MapVec3 targetTile;
	bsIn.Read(targetTile);

	// Create a new shoot action
	GameAction* action;
#ifdef NETWORK_SERVER
	//action = m_game->TakeShot(characterID, targetTile);	// TODO
#endif

	if (action == nullptr) { return; };

	// Send action to all clients
	RakNet::BitStream bs;
	bs.Write((RakNet::MessageID)GameMessages::ID_SERVER_SEND_ACTION);
	// TODO
}

void Server::handleClientMove(RakNet::Packet* packet)
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
#ifdef NETWORK_SERVER
	action = m_game->CreateMoveAction(characterID, destination);
#endif

	// Send action back to all clients
	if (action == nullptr) { return; };

	RakNet::BitStream bs;
	bs.Write((RakNet::MessageID)GameMessages::ID_SERVER_SEND_ACTION);
	bs.Write((char*)action, sizeof(MovementAction));
	// TODO: Implement Write function within MovementAction to avoid pointers, etc.
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
	pPeerInterface->Startup(32, &sd, 1);
	pPeerInterface->SetMaximumIncomingConnections(32);

	RakNet::Packet* packet = nullptr;

	// Start a new thread for client pings
	std::thread pingThread(SendClientPing, pPeerInterface);

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
				sendNewClientID(pPeerInterface, packet->systemAddress);
				break;
			case ID_DISCONNECTION_NOTIFICATION:
				std::cout << "A client has disconnected.\n";
				break;
			case ID_CONNECTION_LOST:
				std::cout << "A client lost connection.\n";
				break;
			case ID_CLIENT_SHOOT:
				handleClientShoot(packet);
				break;
			case ID_CLIENT_MOVE:
				handleClientMove(packet);
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
