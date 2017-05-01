#include "Game.h"
#include "TileMap.h"

#include <typeinfo>


// Static variable declaration
Game* Game::m_singleton;

/** 
 * Set up the Game instance. This handles initialization of squads,
 * tile map, etc.
 */
void Game::Setup()
{
	// TODO: create a squad for each player. One squad humans, one aliens
}

Squad * Game::GetPlayingSquad()
{
	return &m_squads[m_currentTurn];
}

Squad * Game::GetWaitingSquad()
{
	unsigned int currentWaiting = (m_currentTurn == 0) ? 1 : 0;
	return &m_squads[currentWaiting];
}

int Game::GetShotChance(const Character * shooter, MapVec3 target)
{
	if (shooter)
	{
		// Get the direction from the target square to shooter
		MAP_CONNECTION_DIR dir = target.GetDirectionTo(shooter->GetPosition());

		// Is the target covered from this direction?
		COVER_VALUE targetCover = m_map->GetCoverInDirection(target, dir);
		Character* targetCharacter = nullptr;
		for each (Squad s in m_squads)
		{
			targetCharacter = s.FindCharacter(target);
			if (targetCharacter != nullptr)
				break;
		}

		if (targetCharacter == nullptr)
		{
			printf("No character found at tile for GetShotChance method\n");
			return 0;	// TODO: Change implementation for explosions, etc
		}
		else
		{
			/* Uses xcom chance-calculation formula as seen on the wiki here: 
			 * http://www.ufopaedia.org/index.php/Chance_to_Hit_(EU2012) 
			 */

			// Calculate base chance based on shooter's aim and target's defense
			unsigned int aim = shooter->GetCurrentAimStat();
			unsigned int def = targetCharacter->GetCurrentDefenseStat() + targetCover;
			int chance = (int)aim - (int)def;
			// Clamp if negative
			if (chance < 1) chance = 1;

			// Calculate range bonus
			float shotDistance = MapVec3::Distance(shooter->GetPosition(), target);
			int rangeBonus = shooter->GetAimBonus(shotDistance);

			chance += rangeBonus;

			// Cap chance
			if (chance > 100) chance = 100;
			if (chance < 1) chance = 1;

			return chance;
		}
	}

	printf("Null reference exception. No shooter specified in GetShotChance method\n");
	return 0;
}

int Game::GetCritChance(const Character * shooter, MapVec3 target)
{
	// TODO:
	// Get crit chance of the shooter.
	// Find if enemy is flanked.
	return 0;
}

int Game::GetDamage(const Character * shooter)
{
	// TODO: If supporting secondary weapons, etc, this needs to take in which weapon is being used

	std::pair<unsigned int, unsigned int> damageVals = shooter->GetWeaponDamage();
	int difference = damageVals.second - damageVals.first;

	int r = rand() % difference + (int)damageVals.first;	// TODO: Verify if this actually uses all values?

	return r;
}

Game::Game()
{
	if (!m_singleton)
		m_singleton = this;
	// TODO: Fix singleton stuff

	m_tileScale = 1;
	m_currentTurn = 0;
	m_currentAction = m_actionQueue.begin();

	Setup();
}

Game::~Game()
{
	if (m_singleton == this)
		m_singleton = nullptr;
}

void Game::SafeDelete()
{
	if (m_map)
		delete m_map;

	delete this;
}

Game * Game::GetInstance()
{
	if (m_singleton == nullptr)
		m_singleton = new Game();
	return m_singleton;
}

TileMap * Game::GetMap()
{
	return GetInstance()->m_map;
}

float Game::GetMapTileScale()
{
	return GetInstance()->m_tileScale;
}

void Game::Update(float dTime)
{
	if (m_currentAction != m_actionQueue.end())
	{
		if ((*m_currentAction)->IsCompleted())
			m_currentAction++;	// Current action is complete. Advance iterator
		else
			(*m_currentAction)->Execute(dTime);	// Play current action
	}
}

/**
 * Add an action to the queue. Actions will be executed one by one
 * by the client, in the order they were sent by the server.
 */
void Game::QueueAction(short uniqueID, GameAction* action)
{
	if (action == nullptr) { return; };
	m_actionQueue.push_back(action);
}

Character * Game::FindCharacterAtCoords(const MapVec3 position) const
{
	Character* c = m_squads[0].FindCharacter(position);
	if (c == nullptr)
		c = m_squads[1].FindCharacter(position);
	return c;
}

Character * Game::FindCharacterByID(const short id) const
{
	Character* c = m_squads[0].FindCharacter(id);
	if (c == nullptr)
		c = m_squads[1].FindCharacter(id);
	return c;
}

#ifndef NETWORK_SERVER
void Game::Draw()
{
	m_squads[0].Draw();
	m_squads[1].Draw();
}

void Game::Read(RakNet::Packet * packet)
{
	// Get bitstream from the packet
	RakNet::BitStream bsIn(packet->data, packet->length, false);
	bsIn.IgnoreBytes(sizeof(RakNet::MessageID));

	// Create a new map if it is currently null
	if (m_map == nullptr) m_map = new TileMap();

	// Read Tilemap data
	m_map->ReadTilemapNew(bsIn);
	bsIn.Read(m_tileScale);

	// TODO: Implement along with Write function
}
#endif

#ifdef NETWORK_SERVER
void Game::GetShotVariables(short & damage, SHOT_STATUS & shotType, const Character * shooter, const MapVec3 target)
{
	int chanceToHit = GetShotChance(shooter, target);
	int r = rand() % 100;

	if (r < chanceToHit)
	{
		// Shot hits

		int chanceToCrit = GetCritChance(shooter, target);
		r = rand() % 100;

		if (r < chanceToCrit)
			shotType = CRITICAL;
		else
			shotType = HIT;

		damage = GetDamage(shooter);
	}
	else
	{
		// Shot misses
		damage = 0;
		shotType = MISS;
	}
}

/**
 * Handle character movement to a specified map coordinate. 
 * Returns null if the character or map coordinate could not be found, or if the
 * move is not valid.
 */
GameAction * Game::CreateMoveAction(short characterID, MapVec3 coords)
{
	// Find the referenced character
	auto& cIter = m_characters.find(characterID);
	if (cIter != m_characters.end())
	{
		Character* c = &(cIter->second);
		if (c != nullptr)
		{
			// Find the tile the character is standing on
			MapVec3 characterPos = c->GetMapTileCoords();
			if (c->RemainingActionPoints() == 0)
			{
				printf("Warning: Tried to move character (id: %d) with no action points.\n", characterID);
				return nullptr;
			}

			// Find a path
			std::list<MapVec3> path = m_map->FindPath(characterPos, coords);
			if (path.size() == 0)
			{
				printf("Error: Found no path between tile (%d, %d, %d) and tile (%d, %d, %d).\n",
					characterPos.m_x, characterPos.m_y, characterPos.m_z, coords.m_x, coords.m_y, coords.m_z);
				return nullptr;
			}

			// Check if the character can legally move this far
			if (c->PointsToMove((short)path.size()) == 0)
				printf("Error: Character (id: %d) cannot legally move %d tiles.\n", characterID, path.size());

			// Create an action
			GameAction* g = new GameAction();

			// Get some basic info
			Squad* waitingSquad = GetWaitingSquad();

			// Iterate through tiles in the path
			std::list<MapVec3>::iterator pathIter;
			for (pathIter = path.begin(); pathIter != path.end(); pathIter++)
			{
				MapVec3 thisPos = c->GetPosition();
				MapVec3 nextPos = (*pathIter);

				// Check for overwatch triggers. If any are triggered, actions will be added to the list
				waitingSquad->QueryOverwatch(g, c, *m_map);

				// Create movement to next tile in the path
				MovementAction* mA = new MovementAction(c, nextPos);
				g->AddToQueue(mA);

				// Simulate on the server
				// NOTE: Execute takes a float, but this does not do anything when simulated on the server, 
				// as everything is done instantly
				while (!g->IsCompleted())
					g->Execute(0);
			}

			// Check overwatch triggers for final tile move
			waitingSquad->QueryOverwatch(g, c, *m_map);

			// Simulate last overwatch
			while (!g->IsCompleted())
				g->Execute(0);

			// Reset the game action. 
			// (Reverses the effects of simulating it on the server, so it is again ready to use on a client)
			g->Reset();

			return g;
		}
		else
			printf("Error: MoveCharacter function could not find character with id %d.\n", characterID);
	}
	else
		printf("Error: Could not find character");

	return nullptr;
}

void Game::Write(RakNet::BitStream & bs)
{
	// TODO: Finish this implementation

	// Write Tilemap data
	m_map->WriteTilemapNew(bs);
	bs.Write(m_tileScale);

	// Write Character data
	bs.Write(m_characters.size());
	for (auto& iter = m_characters.begin(); iter != m_characters.end(); iter++)
	{
		// Write character's home squad
		bs.Write(iter->second.GetHomeSquad());
		iter->second.Write(bs);
	}

	// Write the action queue
	bs.Write(m_actionQueue.size());
	for (auto& iter = m_actionQueue.begin(); iter != m_actionQueue.end(); iter++)
	{
		/* Write action type here.
		 * This is needed when reading, as each action type has it's own Read implementation.
		 * Does not seem to be the best way to achieve this. :( Will have to look into it later.
		 */
		std::string typeName = typeid(&(iter)).name();
		bs.Write(typeName.c_str());

		// Write the action
		(*iter)->Write(bs);
	}

}

#endif