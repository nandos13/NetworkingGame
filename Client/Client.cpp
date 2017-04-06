#include "Client.h"
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
}

Client::~Client() 
{
}

bool Client::startup() 
{
	handleNetworkConnection();

	// Initialize game
	m_game = new Game();
	
	setBackgroundColour(0.25f, 0.25f, 0.25f);

	// initialise gizmo primitive counts
	Gizmos::create(10000, 10000, 10000, 10000);

	return true;
}

void Client::shutdown() {

	Gizmos::destroy();
}

void Client::update(float deltaTime) 
{

	handleNetworkMessages();

	// query time since application started
	//float time = getTime();

	Gizmos::clear();

	// Update the game
	m_game->Update(deltaTime);

	// Update camera
	//cam.Update(deltaTime);
	
	// quit if we press escape
	aie::Input* input = aie::Input::getInstance();

	if (input->isKeyDown(aie::INPUT_KEY_ESCAPE))
		quit();
}

void Client::draw() {

	// wipe the screen to the background colour
	clearScreen();

	// do drawing here

	Gizmos::draw(cam.GetProjectionMatrix(getWindowWidth(), getWindowHeight()) * cam.GetViewMatrix());
}

glm::mat4 Client::GetCameraTransform()
{
	return (cam.GetProjectionMatrix(getWindowWidth(), getWindowHeight()) * cam.GetViewMatrix());
}

void Client::handleNetworkConnection()
{
	// Initialise the Raknet peer interface first
	m_pPeerInterface = RakNet::RakPeerInterface::GetInstance();
	initialiseClientConnection();
}

void Client::initialiseClientConnection()
{
	// Create a socket descriptor to describe this connection
	// No data needed, as we wil be connecting to a server
	RakNet::SocketDescriptor sd;

	// Now call startup - max of 1 connections (to the server)
	m_pPeerInterface->Startup(1, &sd, 1);
	std::cout << "Connecting to server at: " << IP << std::endl;

	// Now call connect to attempt to connect to the given server
	RakNet::ConnectionAttemptResult res = m_pPeerInterface->Connect(IP, PORT, nullptr, 0);

	// Finally, check to see if we connection. If not, throw error
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
		case ID_SERVER_TEXT_MESSAGE:
		{
			RakNet::BitStream bsIn(packet->data, packet->length, false);
			bsIn.IgnoreBytes(sizeof(RakNet::MessageID));

			RakNet::RakString str;
			bsIn.Read(str);
			std::cout << str.C_String() << std::endl;

			break;
		}

		default:
			std::cout << "Received a message with unknown id: " << (int)packet->data[0] << std::endl;
			break;
		}
	}
}

void Client::sendCharacterMove(short characterID, MapVec3 destination)
{
	RakNet::BitStream bs;
	bs.Write((RakNet::MessageID)GameMessages::ID_CLIENT_MOVE);
	bs.Write(characterID);
	bs.Write((char*)&destination, sizeof(MapVec3));

	m_pPeerInterface->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, 
		RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
}

