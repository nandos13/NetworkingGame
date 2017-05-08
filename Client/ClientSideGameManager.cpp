#include "ClientSideGameManager.h"

#include <glm/ext.hpp>
#include <Input.h>



void ClientSideGameManager::RefreshSelectableCharList()
{
	Game* g = Game::GetInstance();
	// TODO: Find which player this GM belongs to. Probably needs to be specified in constructor
}

MapVec3 ClientSideGameManager::GetClickedTile(bool& missedTiles) const
{
	glm::vec3 camPos = m_cam->GetPosition();
	glm::vec3 camFwd = m_cam->GetForwardVec();

	TileMap* map = Game::GetMap();
	float tileScale = Game::GetMapTileScale();

	// Get a list of tiles under the mouse
	std::list<MapVec3> rayCastHits =
		map->Raycast(camPos.x, camPos.y, camPos.z, camFwd.x, camFwd.y, camFwd.z, tileScale);

	if (rayCastHits.size() == 0)
	{
		missedTiles = true;
		return MapVec3(0);
	}
	else
	{
		missedTiles = false;
		return *(rayCastHits.begin());
	}
}

ClientSideGameManager::ClientSideGameManager(Camera* cam, const bool forceSpectator)
	: m_cam(cam), m_forceSpectator(forceSpectator)
{
	m_camSpeed = 1.0f;
	m_selectedCharacter = nullptr;
}

ClientSideGameManager::~ClientSideGameManager()
{
}

void ClientSideGameManager::Update(float dTime)
{
	aie::Input* input = aie::Input::getInstance();

	if (input->wasMouseButtonPressed(0))
	{
		bool missedTiles = false;
		MapVec3 clicked = GetClickedTile(missedTiles);

		if (!missedTiles)
		{
			// TODO: Left click, find character on tile clicked and switch if they are in the list
		}
	}

	if (input->wasMouseButtonPressed(1))
	{
		bool missedTiles = false;
		MapVec3 clicked = GetClickedTile(missedTiles);

		if (!missedTiles)
		{
			// TODO: Right click, try to move if tile is within range of current selected character
		}
	}
}

void ClientSideGameManager::SetCameraSpeed(const float speed)
{
	m_camSpeed = speed;
}

void ClientSideGameManager::SetSelectedCharacter(Character * c)
{
	m_selectedCharacter = c;
}
