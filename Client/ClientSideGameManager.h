#pragma once

#include "Game.h"
#include "Camera.h"

/**
 * This class handles player input to the game, such as character selection,
 * action input, etc.
 */
class ClientSideGameManager
{
protected:

	Camera* m_cam;
	float m_camSpeed;
	bool m_forceSpectator;

	Character* m_selectedCharacter;
	std::list<Character*> m_selectableCharacters;

	void RefreshSelectableCharList();
	MapVec3 GetClickedTile(bool& missedTiles) const;

public:
	ClientSideGameManager(Camera* cam, const bool forceSpectator);
	~ClientSideGameManager();

	void Update(float dTime);

	void SetCameraSpeed(const float speed);

	void SetSelectedCharacter(Character* c);
};

