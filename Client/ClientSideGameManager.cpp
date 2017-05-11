#include "ClientSideGameManager.h"

#ifndef NETWORK_SERVER

#include "Client.h"

#include <glm/ext.hpp>
#include <Input.h>
#include <Gizmos.h>



/* Cycles to next available character. If reverse == true, cycles backwards. */
void ClientSideGameManager::SelectNextCharacter(const bool reverse)
{
	// Get a list of selectable characters
	std::list<Character*> selectableList = Game::GetInstance()->GetSelectableCharacters();

	// Find which character is currently selected
	if (selectableList.size() > 0)
	{
		if (m_selectedCharacter == nullptr)
		{
			m_selectedCharacter = *selectableList.begin();
		}
		else
		{
			std::list<Character*>::iterator selectedIter = selectableList.end();
			for (auto& iter = selectableList.begin(); iter != selectableList.end(); iter++)
			{
				if ((*iter) == m_selectedCharacter)
				{
					selectedIter = iter;
					break;
				}
			}

			if (selectedIter == selectableList.end())
			{
				// Currently selected character is not in the selectable list. Set selected to first character.
				m_selectedCharacter = *selectableList.begin();
			}
			else
			{
				auto nextCharacter = selectedIter;
				std::list<Character*>::iterator lastElement = selectableList.end();
				lastElement--;

				if (selectedIter == selectableList.begin() && reverse)
					nextCharacter = lastElement;	// First character is selected, select last
				else if (selectedIter == lastElement && !reverse)
					nextCharacter = selectableList.begin();
				else
				{
					if (!reverse)
						nextCharacter++;
					else
						nextCharacter--;
				}

				// Select the new character
				m_selectedCharacter = *nextCharacter;
			}
		}

		// Set camera to lerp to the character
		m_camPosLerping = true;
		TileMap* map = Game::GetMap();
		float x = 0, y = 0, z = 0;
		map->GetTileWorldCoords(x, y, z, m_selectedCharacter->GetPosition(), Game::GetMapTileScale());
		glm::vec3 characterPos = glm::vec3(x, y, z);
		m_camCurrentLookTarget = characterPos;
	}
}

MapVec3 ClientSideGameManager::GetClickedTile(bool& missedTiles) const
{
	glm::vec3 camPos = m_cam->GetPosition();

	// Get click point in world space (This is the world coordinate on the near plane.)
	glm::vec3 clickPoint = 
		m_cam->Get3DPointFromScreenSpace(m_thisClient->getWindowPtr(), m_thisClient->getWindowWidth(), m_thisClient->getWindowHeight());
	
	// Find vector between camera's pos & the click point
	glm::vec3 rayDir = glm::normalize(clickPoint - camPos);
	printf("Ray Direction: %f, %f, %f\n", rayDir.x, rayDir.y, rayDir.z);

	// Get map info
	TileMap* map = Game::GetMap();
	float tileScale = Game::GetMapTileScale();

	// Get a list of tiles under the mouse
	std::list<MapVec3> rayCastHits =
		map->Raycast(camPos.x, camPos.y, camPos.z, rayDir.x, rayDir.y, rayDir.z, tileScale);

	if (rayCastHits.size() == 0)
	{
		missedTiles = true;
		return MapVec3(0);
	}
	else
	{
		// Iterate through tile spaces and find the first one with a physical tile in the tilemap.
		for (auto& iter = rayCastHits.begin(); iter != rayCastHits.end(); iter++)
		{
			if (map->TileAt(*iter))
			{
				// The tile-space has a physical tile. Check if the ray hit the bottom of this tile
				auto iter2 = iter;
				iter2++;
				if (iter2 != rayCastHits.end())
				{
					if ((*iter2).m_y != (*iter).m_y)
					{
						missedTiles = false;
						return *iter;
					}
				}
			}
		}

		// All tile spaces hit by the ray were empty
		missedTiles = true;
		return MapVec3();
	}
}

ClientSideGameManager::ClientSideGameManager(Client* client, Camera* cam)
	: m_thisClient(client), m_cam(cam)
{
	m_camPosLerping = false;
	m_camRotLerping = false;
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

	if (input->wasMouseButtonPressed(0))
	{
		bool missedTiles = false;
		MapVec3 clicked = GetClickedTile(missedTiles);

		if (!missedTiles)
		{
			// TODO: Left click, find character on tile clicked and switch if they are in the list
			printf("Clicked tile: %i, %i, %i\n", clicked.m_x, clicked.m_y, clicked.m_z);

			// TEMP CODE:
			Game::GetMap()->GetTileWorldCoords(clickedTile.x, clickedTile.y, clickedTile.z, clicked, Game::GetMapTileScale());
		}
	}
	else if (input->wasMouseButtonPressed(1))
	{
		bool missedTiles = false;
		MapVec3 clicked = GetClickedTile(missedTiles);

		if (!missedTiles)
		{
			// TODO: Right click, try to move if tile is within range of current selected character
		}
	}
	else if (input->wasKeyPressed(aie::INPUT_KEY_TAB))
		SelectNextCharacter(false);
	else if (input->wasKeyPressed(aie::INPUT_KEY_LEFT_SHIFT))
		SelectNextCharacter(true);
	else if (input->wasKeyPressed(aie::INPUT_KEY_Q) && !m_camRotLerping)
	{
		m_camRotLerping = true;
		m_camRotationDestination += 45.0f;

		float camRotDecimal = m_camRotationDestination - floorf(m_camRotationDestination);
		int camRotInt = (int)m_camRotationDestination;
		camRotInt = camRotInt % 360;
		m_camRotationDestination = (float)camRotInt + camRotDecimal;
		if (m_camRotationDestination < 0)
			m_camRotationDestination += 360;
	}
	else if (input->wasKeyPressed(aie::INPUT_KEY_E) && !m_camRotLerping)
	{
		m_camRotLerping = true;
		m_camRotationDestination -= 45.0f;
		
		float camRotDecimal = m_camRotationDestination - floorf(m_camRotationDestination);
		int camRotInt = (int)m_camRotationDestination;
		camRotInt = camRotInt % 360;
		m_camRotationDestination = (float)camRotInt + camRotDecimal;
		if (m_camRotationDestination < 0)
			m_camRotationDestination += 360;
	}

	/* Update scene stuff */

	m_cam->Update(dTime, m_camCurrentLookTarget, m_camPosLerping, 
		m_camRotationDestination, m_camRotLerping, m_camRotationSpeed, m_thisClient->getWindowWidth(), m_thisClient->getWindowHeight());

	// TEMP CODE:
	float tileScale = Game::GetMapTileScale();
	aie::Gizmos::addAABBFilled(clickedTile, glm::vec3(tileScale / 2, 0, tileScale / 2), glm::vec4(1,0.5f,1,0.8f));
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


#endif