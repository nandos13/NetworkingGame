#pragma once

#ifndef NETWORK_SERVER

#include "Game.h"
#include "Camera.h"

class Client;

/**
 * This class handles player input to the game, such as character selection,
 * action input, etc.
 */
class ClientSideGameManager
{
protected:

	Client*		m_thisClient;

	// Camera variables
	Camera*		m_cam;
	glm::vec3	m_camCurrentLookTarget;
	bool		m_camPosLerping;
	bool		m_camPosFollow;

	float		m_camRotationSpeed;
	float		m_camRotationDestination;
	bool		m_camRotLerping;

	bool		m_forceSpectator;

	// Character selection
	Character* m_selectedCharacter;
	void SelectCharacter(Character* c);
	void SelectNextCharacter(const bool reverse = false);
	void HunkerDown() const;

	// Enemy selection
	Character* m_currentEnemy;
	void DrawEnemyTile();
	void AttackEnemy();

	// Tile selection
	MapVec3				m_hoveredTile;
	bool				m_mouseIsOverVoidSpace;
	std::list<MapVec3>	m_currentPath;

	void DrawHoverTile() const;
	void DrawPath() const;
	void UpdateTilePath();
	MapVec3 GetTileUnderMouse(bool& missedTiles) const;

	// HUD
	void DrawHUD();

public:
	ClientSideGameManager(Client* client, Camera* cam);
	~ClientSideGameManager();

	void Update(const float dTime);

	void SetCameraSpeed(const float speed);
	void SetSpectatorMode(const bool forceSpectator);

	void SetSelectedCharacter(Character* c);

	bool GetSpectatorMode() const;
};


#endif