#include "ClientSideGameManager.h"
#include "Client.h"

#include <glm/ext.hpp>
#include <Input.h>



/* Cycles to next available character. If reverse == true, cycles backwards. */
void ClientSideGameManager::SelectNextCharacter(const bool reverse)
{
	// TODO
}

void ClientSideGameManager::RefreshSelectableCharList()
{
	Game* g = Game::GetInstance();
	// TODO: Find which player this GM belongs to. Probably needs to be specified in constructor
	// Should be able to use Game's m_mySquad variable here somehow
}

MapVec3 ClientSideGameManager::GetClickedTile(glm::vec2 clickPointSS, bool& missedTiles) const
{
	glm::vec3 camPos = m_cam->GetPosition();
	glm::vec3 clickPoint = 
		m_cam->Get3DPointFromScreenSpace(clickPointSS, m_thisClient->getWindowWidth(), m_thisClient->getWindowHeight());

	// Find vector between camera's pos & the click point
	glm::vec3 clickDir = clickPoint - camPos;
	clickDir = glm::normalize(clickDir);

	// Get map info
	TileMap* map = Game::GetMap();
	float tileScale = Game::GetMapTileScale();

	// Get a list of tiles under the mouse
	std::list<MapVec3> rayCastHits =
		map->Raycast(camPos.x, camPos.y, camPos.z, clickDir.x, clickDir.y, clickDir.z, tileScale);

	if (rayCastHits.size() == 0)
	{
		missedTiles = true;
		return MapVec3(0);
	}
	else
	{
		missedTiles = false;
		// TODO: Need to iterate through and find the first MapVec3 that actually corresponds to a tile
		// in the tilemap. This is currently just returning the MapVec3 which the camera is positioned inside.
		return *(rayCastHits.begin());
	}
}

ClientSideGameManager::ClientSideGameManager(Client* client, Camera* cam)
	: m_thisClient(client), m_cam(cam)
{
	m_camRotationSpeed = 1.0f;
	m_camRotationDestination = 0.0f;
	m_selectedCharacter = nullptr;
}

ClientSideGameManager::~ClientSideGameManager()
{
}

void ClientSideGameManager::Update(const float dTime)
{
	/* Handle input */
	aie::Input* input = aie::Input::getInstance();
	int mouseX, mouseY;
	input->getMouseXY(&mouseX, &mouseY);
	glm::vec2 mousePos = glm::vec2(mouseX, mouseY);

	if (input->wasMouseButtonPressed(0))
	{
		bool missedTiles = false;
		MapVec3 clicked = GetClickedTile(mousePos, missedTiles);

		if (!missedTiles)
		{
			// TODO: Left click, find character on tile clicked and switch if they are in the list
		}
	}
	else if (input->wasMouseButtonPressed(1))
	{
		bool missedTiles = false;
		MapVec3 clicked = GetClickedTile(mousePos, missedTiles);

		if (!missedTiles)
		{
			// TODO: Right click, try to move if tile is within range of current selected character
		}
	}
	else if (input->wasKeyPressed(aie::INPUT_KEY_TAB))
		SelectNextCharacter(false);
	else if (input->wasKeyPressed(aie::INPUT_KEY_LEFT_SHIFT))
		SelectNextCharacter(true);
	else if (input->wasKeyPressed(aie::INPUT_KEY_Q))
	{
		m_camRotationDestination += 45.0f;
		if (m_camRotationDestination >= 360)	m_camRotationDestination -= 360;
	}
	else if (input->wasKeyPressed(aie::INPUT_KEY_E))
	{
		m_camRotationDestination -= 45.0f;
		if (m_camRotationDestination <= 0)		m_camRotationDestination += 360;
	}

	/* Update scene stuff */

	m_cam->Update(dTime, m_camCurrentLookTarget, m_camPosLerping, 
		m_camRotationDestination, m_camRotationSpeed, m_thisClient->getWindowWidth(), m_thisClient->getWindowHeight());
}

void ClientSideGameManager::SetCameraSpeed(const float speed)
{
	m_camRotationSpeed = speed;
}

void ClientSideGameManager::SetSpectatorMode(const bool forceSpectator)
{
	m_forceSpectator = forceSpectator;
}

void ClientSideGameManager::SetSelectedCharacter(Character * c)
{
	m_selectedCharacter = c;
}
