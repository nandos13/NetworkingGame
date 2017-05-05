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

public:
	ClientSideGameManager(Camera* cam);
	~ClientSideGameManager();

	void Update(float dTime);
};

