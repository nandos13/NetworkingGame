#include "Client.h"

#ifndef NETWORK_SERVER

#include "Gizmos.h"
#include "Input.h"
#include "GameMessages.h"
#include "TileMap.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <iostream>
#include <MessageIdentifiers.h>
#include <BitStream.h>

using glm::vec3;
using glm::vec4;
using glm::mat4;
using aie::Gizmos;

Client::Client() 
{
	m_myID = -1;
	m_gm = new ClientSideGameManager(this, &m_cam);
}

Client::~Client() 
{
	if (m_game)
		m_game->SafeDelete();
}

bool Client::startup() 
{
	handleNetworkConnection();
	
	setBackgroundColour(0.25f, 0.25f, 0.25f);

	// initialise gizmo primitive counts
	Gizmos::create(10000, 10000, 10000, 10000);

	// Initialize game
	m_game = Game::GetInstance();

	return true;
}

void Client::shutdown() {

	Gizmos::destroy();
}

void Client::update(float deltaTime) 
{

	handleNetworkMessages();

	Gizmos::clear();

	// Update the game
	m_game->Update(deltaTime);

	// Update the game manager
	if (m_gm != nullptr)
		m_gm->Update(deltaTime);

	// Update camera
	//cam.Update(deltaTime);
	
	// quit if we press escape
	aie::Input* input = aie::Input::getInstance();

	if (input->isKeyDown(aie::INPUT_KEY_ESCAPE))
		quit();
}

void Client::draw() {

	// Wipe the screen to the background colour
	clearScreen();

	// Do drawing here
	m_game->Draw();

	Gizmos::draw(GetCameraTransform());
}

glm::mat4 Client::GetCameraTransform() const
{
	return ( m_cam.GetMVP(getWindowWidth(), getWindowHeight()) );
}

void Client::handleNetworkConnection()
{
	m_pPeerInterface = RakNet::RakPeerInterface::GetInstance();
	initialiseClientConnection();
}

void Client::initialiseClientConnection()
{
	// Create a socket descriptor to describe this connection
	// No data needed, as we will be connecting to a server
	RakNet::SocketDescriptor sd;

	// Now call startup - max of 1 connections (to the server)
	m_pPeerInterface->Startup(1, &sd, 1);
	std::cout << "Connecting to server at: " << IP << std::endl;

	// Now call connect to attempt to connect to the given server
	RakNet::ConnectionAttemptResult res = m_pPeerInterface->Connect(IP, PORT, nullptr, 0);

	// Finally, check to see if we connected. If not, throw error
	if (res != RakNet::CONNECTION_ATTEMPT_STARTED)
		std::cout << "Unable to start a connection, Error #: " << res << std::endl;
}

void Client::handleNetworkMessages()
{
	RakNet::Packet* packet;

	for (packet = m_pPeerInterface->Receive(); packet;
		m_pPeerInterface->DeallocatePacket(packet),
		packet = m_pPeerInterface->Receive())
	{
		switch (packet->data[0])
		{
		case ID_REMOTE_DISCONNECTION_NOTIFICATION:
			std::cout << "Another client has disconnected.\n";
			break;
		case ID_REMOTE_CONNECTION_LOST:
			std::cout << "Another client has lost connection.\n";
			break;
		case ID_REMOTE_NEW_INCOMING_CONNECTION:
			std::cout << "Another client has connected.\n";
			break;
		case ID_CONNECTION_REQUEST_ACCEPTED:
			std::cout << "Our connection request has been accepted.\n";
			break;
		case ID_NO_FREE_INCOMING_CONNECTIONS:
			std::cout << "The server is full.\n";
			break;
		case ID_DISCONNECTION_NOTIFICATION:
			std::cout << "We have been disconnected.\n";
			break;
		case ID_CONNECTION_LOST:
			std::cout << "Connection lost.\n";
			break;
		case ID_SERVER_SET_CLIENT_ID:
			ReceiveClientID(packet);
			break;
		case ID_SERVER_TEXT_MESSAGE:
		{
			RakNet::BitStream bsIn(packet->data, packet->length, false);
			bsIn.IgnoreBytes(sizeof(RakNet::MessageID));

			RakNet::RakString str;
			bsIn.Read(str);
			std::cout << str.C_String() << std::endl;

			break;
		}
		case ID_SERVER_INITIALISE_GAME:
			// TODO: Thread this to prevent client from locking up
			ReceiveGameInfo(packet);
			break;
		case ID_SERVER_SEND_ACTION:
			// TODO
			break;

		default:
			std::cout << "Received a message with unknown id: " << (int)packet->data[0] << std::endl;
			break;
		}
	}
}

void Client::ReceiveClientID(RakNet::Packet * packet)
{
	std::cout << "Receiving Client Info..." << std::endl;

	RakNet::BitStream bsIn(packet->data, packet->length, false);
	bsIn.IgnoreBytes(sizeof(RakNet::MessageID));

	// Get Client ID
	unsigned int id = 0;
	bsIn.Read(id);
	m_myID = (int)id;
	std::cout << "ID: " << m_myID << "\t";

	// Get Spectator-mode state
	bool isPlayer = false;
	bsIn.Read(isPlayer);

	std::cout << "Spectator-Mode: ";
	if (!isPlayer) std::cout << "true" << std::endl;
	else std::cout << "false" << std::endl;

	// Set spectator variables
	m_gm->SetSpectatorMode(!isPlayer);
	Game::GetInstance()->SetSpectatorMode(!isPlayer);

	// If this client is a player, take possession of a squad
	if (isPlayer)
		Game::GetInstance()->TakeControlOfSquad((int)id + 1);	// eg. Client id 0 takes squad 1
}

void Client::ReceiveGameInfo(RakNet::Packet * packet)
{
	if (m_game != nullptr)
	{
		std::cout << "Receiving new game info.\n";
		m_game->Read(packet);
	}
}

void Client::sendCharacterShoot(short characterID, MapVec3 target)
{
	RakNet::BitStream bs;
	bs.Write((RakNet::MessageID)GameMessages::ID_CLIENT_SHOOT);
	bs.Write(characterID);
	target.Write(bs);

	m_pPeerInterface->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0,
		RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
}

void Client::sendCharacterMove(short characterID, MapVec3 destination)
{
	RakNet::BitStream bs;
	bs.Write((RakNet::MessageID)GameMessages::ID_CLIENT_MOVE);
	bs.Write(characterID);
	destination.Write(bs);

	m_pPeerInterface->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, 
		RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
}


#endif