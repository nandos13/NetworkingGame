#include "ClientSideGameManager.h"

#ifndef NETWORK_SERVER

#include "Client.h"
#include "Game.h"

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

				m_currentEnemy = nullptr;

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

		m_currentEnemy = nullptr;
	}
}

void ClientSideGameManager::HunkerDown() const
{
	if (m_selectedCharacter != nullptr)
	{
		short charID = m_selectedCharacter->GetID();
		m_thisClient->sendCharacterHunker(charID);
	}
}

void ClientSideGameManager::Reload() const
{
	if (m_selectedCharacter != nullptr)
	{
		short charID = m_selectedCharacter->GetID();
		m_thisClient->sendCharacterReload(charID);
	}
}

void ClientSideGameManager::DrawEnemyTile()
{
	if (m_selectedCharacter == nullptr)
	{
		m_currentEnemy = nullptr;
		return;
	}

	if (m_currentEnemy != nullptr)
	{
		float tileScale = Game::GetMapTileScale();

		float x = 0, y = 0, z = 0;
		MapVec3::GetTileWorldCoords(x, y, z, m_currentEnemy->GetPosition(), tileScale);
		glm::vec3 worldSpaceTilePos = glm::vec3(x, y, z);

		aie::Gizmos::addAABBFilled(worldSpaceTilePos, glm::vec3(tileScale / 2, 0, tileScale / 2), glm::vec4(1, 0, 0, 0.4f));
	}
}

void ClientSideGameManager::AttackEnemy()
{
	if (m_currentEnemy != nullptr && m_selectedCharacter != nullptr)
	{
		if (m_currentEnemy->Alive() && m_selectedCharacter->Alive())
			m_thisClient->sendCharacterShoot(m_selectedCharacter->GetID(), m_currentEnemy->GetPosition());
	}
}

void ClientSideGameManager::DrawHoverTile() const
{
	if (!m_mouseIsOverVoidSpace)
	{
		float tileScale = Game::GetMapTileScale();

		float x = 0, y = 0, z = 0;
		MapVec3::GetTileWorldCoords(x, y, z, m_hoveredTile, tileScale);
		glm::vec3 worldSpaceTilePos = glm::vec3(x, y, z);

		aie::Gizmos::addAABBFilled(worldSpaceTilePos, glm::vec3(tileScale / 2, 0, tileScale / 2), glm::vec4(1, 0.5f, 1, 0.4f));
	}
}

void ClientSideGameManager::DrawPath() const
{
	if (m_currentPath.size() > 0 && m_selectedCharacter != nullptr)
	{
		// Get the colour of the line
		glm::vec4 colour = glm::vec4(1, 1, 1, 0.8f);

		// Check if the hovered tile is within a 1-point walking range
		auto walkTiles = m_selectedCharacter->Get1PointWalkTiles();
		if (std::find(walkTiles.begin(), walkTiles.end(), m_hoveredTile) != walkTiles.end())
			colour = glm::vec4(0, 0, 1, 0.8f);		// blue
		else
			colour = glm::vec4(0.9f, 0.55f, 0.05f, 0.8f);		// orange

		MapVec3 point1 = *(m_currentPath.begin());
		for (auto& iter = m_currentPath.begin(); iter != m_currentPath.end(); iter++)
		{
			// Ignore first point in list
			if (iter == m_currentPath.begin())
				continue;

			MapVec3 point2 = *iter;

			// Convert path points to world space
			float tileScale = Game::GetMapTileScale();
			float x = 0, y = 0, z = 0;
			MapVec3::GetTileWorldCoords(x, y, z, point1, tileScale);
			glm::vec3 p1World = glm::vec3(x, y, z);

			MapVec3::GetTileWorldCoords(x, y, z, point2, tileScale);
			glm::vec3 p2World = glm::vec3(x, y, z);

			// Draw a gizmo between the two points
			aie::Gizmos::addLine(p1World, p2World, colour);

			// Increment the first point
			point1 = point2;
		}
	}
}

void ClientSideGameManager::UpdateTilePath()
{
	m_currentPath.clear();

	if (m_selectedCharacter != nullptr)
	{
		if (m_selectedCharacter->HasRemainingPoints() && m_selectedCharacter->Alive())
		{
			TileMap* map = Game::GetMap();
			auto obstacles = Game::GetInstance()->GetAllCharacterPositions(true);
			m_currentPath = map->FindPath(m_selectedCharacter->GetPosition(), m_hoveredTile, obstacles);
		}
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
		map->Raycast(camPos.x, camPos.y, camPos.z, rayDir.x, rayDir.y, rayDir.z, 100.0f, tileScale);

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
	int ImGuiID = 0;
	aie::Input* input = aie::Input::getInstance();
	Game* game = Game::GetInstance();
	TileMap* map = Game::GetMap();

	// If not player's turn, only draw notification
	if (!game->IsMyTurn())
	{
		ImGui::SetNextWindowContentSize(ImVec2(150, 40));
		ImGui::Begin("", (bool*)0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
		ImGui::Text("Enemy Activity");
		ImGui::End();
		return;		// No other HUD elements need to be drawn
	}

	// Draw character info
	{
		ImGui::Begin("Current Character", (bool*)0, ImGuiWindowFlags_NoCollapse);

		if (m_selectedCharacter == nullptr)
		{
			ImGui::Text("No character selected.");
		}
		else
		{
			if (!m_selectedCharacter->Alive())
			{
				ImGui::Text("Selected character is dead.");
			}
			else
			{
				int actionPoints = (int)m_selectedCharacter->RemainingActionPoints();
				ImGui::Text("Action Points: %d", actionPoints);

				bool hunkered = m_selectedCharacter->IsHunkeredDown();
				if (hunkered)	ImGui::Text("Hunkered Down: true");
				else			ImGui::Text("Hunkered Down: false");

				// Show amount of enemies visible
				auto visibleEnemies = m_selectedCharacter->GetVisibleEnemies();
				ImGui::Text("Visible Enemies: %d", (int)visibleEnemies.size());

				// TEMP: Get position
				MapVec3 pos = m_selectedCharacter->GetPosition();

				ImGui::Text("Position: %d, %d, %d", pos.m_x, pos.m_y, pos.m_z);

				ImGui::Text("Ammo: %d", m_selectedCharacter->GetRemainingAmmo());

				ImGui::Text("Health: %d", m_selectedCharacter->RemainingHealth());
			}
		}

		ImGui::End();
	}

	// Draw abilities bar
	if (m_selectedCharacter != nullptr)
	{
		ImGui::Begin("Abilities", (bool*)0, ImGuiWindowFlags_NoCollapse);

		/* TODO: GetPosition only retrieves a character's current position.
		 * In case of a currently moving character, this may return a tile which is in cover
		 * even when their destination tile is not. Need to implement a new variable
		 * which is set when a character is affected by a move action.
		 */
		// Hunker is only useable in cover
		if (map->TileIsInCover(m_selectedCharacter->GetPosition()))
		{
			if (ImGui::Button("Hunker Down", ImVec2(40, 40)))
				HunkerDown();
		}

		// Reload is only useable when not at full ammo
		if (m_selectedCharacter->GetRemainingAmmo() != m_selectedCharacter->GetMaxAmmo())
		{
			if (ImGui::Button("Reload", ImVec2(40, 40)))
				Reload();
		}

		ImGui::End();
	}

	// Draw visible enemies selection
	if (m_selectedCharacter != nullptr)
	{
		ImGui::Begin("Enemies", (bool*)0, ImGuiWindowFlags_NoCollapse);

		std::list<Character*> visibleEnemies = m_selectedCharacter->GetVisibleEnemies();

		for (auto& iter = visibleEnemies.begin(); iter != visibleEnemies.end(); iter++)
		{
			Character* current = (*iter);
			bool thisEnemyIsSelected = (m_currentEnemy == current);

			// If selected, hilight the button
			ImGuiStyle& style = ImGui::GetStyle();
			ImVec4 defaultBtnColour = style.Colors[ImGuiCol_Button];

			if (thisEnemyIsSelected)
			{
				ImVec4 selectedColour = ImVec4(1,0,0,1);
				style.Colors[ImGuiCol_Button] = selectedColour;
			}

			ImGui::PushID(ImGuiID++);

			if (ImGui::Button("Enemy", ImVec2(40, 40)))
			{
				if (thisEnemyIsSelected)
				{
					// Target is confirmed
					AttackEnemy();
				}
				else
					m_currentEnemy = current;
			}

			ImGui::PopID();

			// Revert button colour
			style.Colors[ImGuiCol_Button] = defaultBtnColour;
		}

		ImGui::End();
	}

	// Draw enemy attack confirmation
	if (m_currentEnemy != nullptr && m_selectedCharacter != nullptr)
	{
		ImGui::Begin("Attack Confirmation");

		int shotChance = game->GetShotChance(m_selectedCharacter, m_currentEnemy->GetPosition());
		ImGui::Text("Chance to hit: %d", shotChance);

		if (ImGui::Button("Fire", ImVec2(120, 40)) || input->wasKeyPressed(aie::INPUT_KEY_SPACE))
		{
			AttackEnemy();
		}

		ImGui::End();
	}
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
	m_currentEnemy = nullptr;

	m_hoveredTile = MapVec3(0);
	m_mouseIsOverVoidSpace = true;
}

ClientSideGameManager::~ClientSideGameManager()
{
}

void ClientSideGameManager::Update(const float dTime)
{
	/* Check if mouse pointer is over ImGui elements */
	bool mouseIsOverHUD = ImGui::IsMouseHoveringAnyWindow();

	if (!mouseIsOverHUD)
	{
		/* Update mouse-hover-point */
		MapVec3 previouslyHovered = m_hoveredTile;
		m_hoveredTile = GetTileUnderMouse(m_mouseIsOverVoidSpace);

		// Update path
		if (previouslyHovered != m_hoveredTile)
			UpdateTilePath();
	}

	// Get lists of tiles the selected character is able to move to
	std::list<MapVec3> walkTiles;
	std::list<MapVec3> dashTiles;
	if (m_selectedCharacter != nullptr)
	{
		walkTiles = m_selectedCharacter->Get1PointWalkTiles();
		dashTiles = m_selectedCharacter->Get2PointWalkTiles();
	}

	if (!mouseIsOverHUD)
	{
		// Draw gizmos along path
		DrawPath();

		// Draw a translucent gizmo over the hovered tile
		DrawHoverTile();
	}

	// Draw a translucent gizmo over the currently selected enemiy's tile
	DrawEnemyTile();

	/* Handle input */
	aie::Input* input = aie::Input::getInstance();
	Game* game = Game::GetInstance();

	if (input->wasMouseButtonPressed(0) && !mouseIsOverHUD)	// Left click: Select friendly character at clicked tile
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
	else if (input->wasMouseButtonPressed(1) && !mouseIsOverHUD)	// Right click: Move selected character to clicked target
	{
		if (!m_mouseIsOverVoidSpace && game->IsPlayersTurn(m_thisClient->GetID()))
		{
			if (m_selectedCharacter != nullptr)
			{
				if (m_selectedCharacter->Alive())
				{
					bool moveable = false;

					// Check if there is currently another character on this tile
					if (game->FindCharacterAtCoords(m_hoveredTile) == nullptr)
					{
						// Get remaining points
						unsigned int points = m_selectedCharacter->RemainingActionPoints();

						if (points >= 1)
						{
							// Check the walk list for the clicked tile
							for (auto& iter = walkTiles.cbegin(); iter != walkTiles.cend(); iter++)
							{
								if ((*iter) == m_hoveredTile)
								{
									moveable = true;
									break;
								}
							}
						}

						// Check the dash list for the clicked tile
						if (!moveable && points >= 2)
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
					}

					if (moveable)
					{
						// This seems to be a legal move. Send a message to the server
						m_thisClient->sendCharacterMove(m_selectedCharacter->GetID(), m_hoveredTile);
						m_currentPath.clear();
					}
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
	SelectCharacter(c);
}

bool ClientSideGameManager::GetSpectatorMode() const
{
	return m_forceSpectator;
}


#endif