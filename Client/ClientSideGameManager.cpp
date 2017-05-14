#include "ClientSideGameManager.h"

#ifndef NETWORK_SERVER

#include "Client.h"

#include <glm/ext.hpp>
#include <Input.h>
#include <Gizmos.h>
#include <imgui.h>



/* Verifies the specified character a part of the player's team, and selects them if true. */
void ClientSideGameManager::SelectCharacter(Character * c)
{
	if (c != nullptr)
	{
		// Get a list of all characters on team
		Game* g = Game::GetInstance();
		std::list<Character*> teamCharacters = g->GetCharactersByHomeSquad(m_thisClient->GetID());

		// Iterate through & check if the specified character is in the list
		for (auto& iter = teamCharacters.cbegin(); iter != teamCharacters.cend(); iter++)
		{
			if ((*iter) == c)
			{
				m_selectedCharacter = c;

				// Set camera to lerp to the character
				m_camPosLerping = true;
				m_camPosFollow = true;
				float x = 0, y = 0, z = 0;
				MapVec3::GetTileWorldCoords(x, y, z, m_selectedCharacter->GetPosition(), Game::GetMapTileScale());
				glm::vec3 characterPos = glm::vec3(x, y, z);
				m_camCurrentLookTarget = characterPos;

				break;
			}
		}
	}
}

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
		m_camPosFollow = true;
		float x = 0, y = 0, z = 0;
		MapVec3::GetTileWorldCoords(x, y, z, m_selectedCharacter->GetPosition(), Game::GetMapTileScale());
		glm::vec3 characterPos = glm::vec3(x, y, z);
		m_camCurrentLookTarget = characterPos;
	}
}

MapVec3 ClientSideGameManager::GetTileUnderMouse(bool& missedTiles) const
{
	glm::vec3 camPos = m_cam->GetPosition();

	// Get click point in world space (This is the world coordinate on the near plane.)
	glm::vec3 mousePoint = 
		m_cam->Get3DPointFromScreenSpace(m_thisClient->getWindowPtr(), m_thisClient->getWindowWidth(), m_thisClient->getWindowHeight());
	
	// Find vector between camera's pos & the click point
	glm::vec3 rayDir = glm::normalize(mousePoint - camPos);

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

void ClientSideGameManager::DrawHUD()
{
	// Draw character info
	ImGui::Begin("Current Character", (bool*)0, ImGuiWindowFlags_NoCollapse);

	if (m_selectedCharacter == nullptr)
	{
		ImGui::Text("No character selected.");
	}
	else
	{
		int actionPoints = (int)m_selectedCharacter->RemainingActionPoints();
		ImGui::InputInt("Action Points", &actionPoints);
	}

	ImGui::End();

	ImGui::DragFloat3("", &m_camCurrentLookTarget.x);
}

ClientSideGameManager::ClientSideGameManager(Client* client, Camera* cam)
	: m_thisClient(client), m_cam(cam)
{
	m_camPosLerping = false;
	m_camPosFollow = false;
	m_camRotLerping = false;
	m_camRotationSpeed = 1.0f;
	m_camRotationDestination = 0.0f;
	m_selectedCharacter = nullptr;

	m_hoveredTile = MapVec3(0);
	m_mouseIsOverVoidSpace = true;
}

ClientSideGameManager::~ClientSideGameManager()
{
}

void ClientSideGameManager::Update(const float dTime)
{
	/* Update mouse-hover-point */
	m_hoveredTile = GetTileUnderMouse(m_mouseIsOverVoidSpace);

	// Draw a translucent gizmo over the hovered tile
	if (!m_mouseIsOverVoidSpace)
	{
		float tileScale = Game::GetMapTileScale();

		float x = 0, y = 0, z = 0;
		MapVec3::GetTileWorldCoords(x, y, z, m_hoveredTile, tileScale);
		glm::vec3 worldSpaceTilePos = glm::vec3(x, y, z);

		aie::Gizmos::addAABBFilled(worldSpaceTilePos, glm::vec3(tileScale / 2, 0, tileScale / 2), glm::vec4(1, 0.5f, 1, 0.4f));
	}

	// Get lists of tiles the selected character is able to move to
	std::list<MapVec3> walkTiles;
	std::list<MapVec3> dashTiles;
	if (m_selectedCharacter != nullptr)
	{
		walkTiles = m_selectedCharacter->Get1PointWalkTiles();
		dashTiles= m_selectedCharacter->Get2PointWalkTiles();
	}

	/* Handle input */
	aie::Input* input = aie::Input::getInstance();

	if (input->wasMouseButtonPressed(0))	// Left click: Select friendly character at clicked tile
	{
		if (!m_mouseIsOverVoidSpace)
		{
			// Find character at the clicked position
			Game* game = Game::GetInstance();
			Character* charAtHoverTile = game->FindCharacterAtCoords(m_hoveredTile);
			
			// Select the character. (SelectCharacter function will also verify if the character is friendly)
			if (charAtHoverTile != nullptr)
				SelectCharacter(charAtHoverTile);
		}
	}
	else if (input->wasMouseButtonPressed(1))	// Right click: Move selected character to clicked target
	{
		if (!m_mouseIsOverVoidSpace && Game::GetInstance()->IsPlayersTurn(m_thisClient->GetID()))
		{
			if (m_selectedCharacter != nullptr)
			{
				bool moveable = false;

				// Check the walk list for the clicked tile
				for (auto& iter = walkTiles.cbegin(); iter != walkTiles.cend(); iter++)
				{
					if ((*iter) == m_hoveredTile)
					{
						moveable = true;
						break;
					}
				}

				// Check the dash list for the clicked tile
				if (!moveable)
				{
					for (auto& iter = dashTiles.cbegin(); iter != dashTiles.cend(); iter++)
					{
						if ((*iter) == m_hoveredTile)
						{
							moveable = true;
							break;
						}
					}
				}

				if (moveable)
				{
					// This seems to be a legal move. Send a message to the server
					m_thisClient->sendCharacterMove(m_selectedCharacter->GetID(), m_hoveredTile);
				}
			}
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
	glm::vec3 followTarget = glm::vec3(0);
	if (m_camPosFollow && m_selectedCharacter != nullptr)
	{
		// Follow the character as they move
		float x = 0, y = 0, z = 0;
		m_selectedCharacter->GetGameObjPosition(x, y, z);

		followTarget = glm::vec3(x, y, z);
	}

	m_cam->Update(dTime, m_camCurrentLookTarget, m_camPosLerping, followTarget, m_camPosFollow,
		m_camRotationDestination, m_camRotLerping, m_camRotationSpeed, m_thisClient->getWindowWidth(), m_thisClient->getWindowHeight());

	DrawHUD();
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