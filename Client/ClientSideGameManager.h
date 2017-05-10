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

	Client* m_thisClient;

	// Camera variables
	Camera* m_cam;
	glm::vec3 m_camCurrentLookTarget;
	bool m_camPosLerping;

	float m_camRotationSpeed;
	float m_camRotationDestination;
	bool m_camRotLerping;

	bool m_forceSpectator;

	// Character selection
	Character* m_selectedCharacter;
	std::list<Character*> m_selectableCharacters;
	void SelectNextCharacter(const bool reverse = false);

	// Other
	MapVec3 GetClickedTile(glm::vec2 clickPointSS, bool& missedTiles) const;

public:
	ClientSideGameManager(Client* client, Camera* cam);
	~ClientSideGameManager();

	void Update(const float dTime);

	void SetCameraSpeed(const float speed);
	void SetSpectatorMode(const bool forceSpectator);

	void SetSelectedCharacter(Character* c);
};


#endif