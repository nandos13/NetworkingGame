#include "Game.h"
#include "TileMap.h"
#include "StandardGun.h"
#include "SniperRifle.h"
#include "Shotgun.h"


// Static variable declaration
Game* Game::m_singleton;

#ifdef NETWORK_SERVER
/** 
 * Set up the Game instance. This handles initialization of squads,
 * tile map, etc.
 */
void Game::Setup()
{
	// TODO: This is only a temporary method for adding characters to each team.
	// Add a better method, like loading from file, etc.
	TempGameSetup();
}
#endif

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

int Game::GetDamage(const Character * shooter, const bool critical)
{
	// TODO: If supporting secondary weapons, etc, this needs to take in which weapon is being used

	std::pair<unsigned int, unsigned int> damageVals = shooter->GetWeaponDamage();
	int difference = damageVals.second - damageVals.first;

	// Choose a damage value between the min and max damage
	int r = rand() % (difference - 1) + (int)damageVals.first;

	// Apply critical damage
	if (critical)
		r = (int)((float)r * 1.5f);

	return r;
}

void Game::ClearGame()
{
	// Delete map & clear dangling pointer
	delete m_map;
	m_map = nullptr;
	
	// Delete characters
	for (auto& iter = m_characters.begin(); iter != m_characters.end(); iter++)
	{
		delete iter->second;
	}
	m_characters.clear();
	m_squads[0].ClearMembers();

	// Delete actions
	for (auto& iter = m_actionQueue.begin(); iter != m_actionQueue.end(); iter++)
	{
		delete *iter;
	}
	m_actionQueue.clear();
	m_currentAction = m_actionQueue.begin();
}

Game::Game()
{
	if (!m_singleton)
		m_singleton = this;

	m_spectating = false;
	m_map = new TileMap();

	m_tileScale = 1;
	m_currentTurn = 0;
	m_currentAction = m_actionQueue.begin();

#ifdef NETWORK_SERVER
	Setup();
#endif
}

Game::~Game()
{
	if (m_singleton == this)
		m_singleton = nullptr;
}

void Game::SafeDelete()
{
	ClearGame();

	m_singleton = nullptr;
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

void Game::SetSpectatorMode(const bool state)
{
	m_spectating = state;
}

#ifndef NETWORK_SERVER
void Game::Draw()
{
	m_squads[0].Draw();
	m_squads[1].Draw();
}

void Game::Read(RakNet::Packet * packet)
{
	ClearGame();

	// Get bitstream from the packet
	RakNet::BitStream bsIn(packet->data, packet->length, false);
	bsIn.IgnoreBytes(sizeof(RakNet::MessageID));

	// Create a new map if it is currently null
	if (m_map == nullptr) m_map = new TileMap();

	// Read Tilemap data
	m_map->Read(bsIn);
	bsIn.Read(m_tileScale);

	// Read Character data
	unsigned int charactersSize = 0;
	bsIn.Read(charactersSize);
	for (unsigned int i = 0; i < charactersSize; i++)
	{
		short squad = 0;
		bsIn.Read(squad);
		short id = 0;
		bsIn.Read(id);

		Character* c = new Character(id, squad);
		c->Read(bsIn);

		m_characters[id] = c;

		// Place the character in the correct squad
		if (squad != 0 && squad != 1)
			printf("Error: Character squad was read as %d. Only 0 or 1 is allowed.\n", (int)squad);
		else
			m_squads[squad].AddMember(c);
	}

	// Read the action queue
	unsigned int queueSize = 0;
	bsIn.Read(queueSize);
	for (unsigned int i = 0; i < queueSize; i++)
	{
		GameAction* gA = new GameAction();
		gA->Read(bsIn);

		// Add the action to the queue
		m_actionQueue.push_back(gA);
	}
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

		bool crit = false;

		if (r < chanceToCrit)
		{
			shotType = CRITICAL;
			crit = true;
		}
		else
			shotType = HIT;

		damage = GetDamage(shooter, crit);
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
		Character* c = cIter->second;
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

/**
 * Temporary way to set up a game. Will implement a better method later,
 * probably reading from a file or allowing each client to choose their squad.
 */
void Game::TempGameSetup()
{
	printf("Creating characters for each squad.\n");
	short id = 0;

	// Loop through both squads
	for (unsigned int sq = 0; sq < 2; sq++)
	{
		printf("Creating squad %i\n", sq);
		// Add a heavy gunner
		{
			Character* c = new Character(id, sq);
			m_characters[id] = c;
			id++;
			m_squads[sq].AddMember(c);
			StandardGun* g = new StandardGun(3, 3, 5);

			c->SetAim(70);
			c->SetHealth(5, true);
			c->SetMobility(9);
			c->SetPrimaryGun(g);
		}

		// Add an assault trooper
		{
			Character* c = new Character(id, sq);
			m_characters[id] = c;
			id++;
			m_squads[sq].AddMember(c);
			StandardGun* g = new StandardGun(4, 2, 4, -2, 10);

			c->SetAim(76);
			c->SetHealth(4, true);
			c->SetMobility(12);
			c->SetPrimaryGun(g);
		}

		// Add a sniper
		{
			Character* c = new Character(id, sq);
			m_characters[id] = c;
			id++;
			m_squads[sq].AddMember(c);
			StandardGun* g = new StandardGun(3, 3, 5, 9, 25);

			c->SetAim(81);
			c->SetHealth(3, true);
			c->SetMobility(10);
			c->SetPrimaryGun(g);
		}

		// Add a support shotgunner
		{
			Character* c = new Character(id, sq);
			m_characters[id] = c;
			id++;
			m_squads[sq].AddMember(c);
			StandardGun* g = new StandardGun(4, 3, 5, -8, 20);

			c->SetAim(76);
			c->SetHealth(4, true);
			c->SetMobility(13);
			c->SetPrimaryGun(g);
		}
	}
}

void Game::Write(RakNet::BitStream & bs)
{
	// TODO: Finish this implementation

	// Write Tilemap data
	m_map->Write(bs);
	bs.Write(m_tileScale);

	// Write Character data
	bs.Write((unsigned int)m_characters.size());
	for (auto& iter = m_characters.begin(); iter != m_characters.end(); iter++)
	{
		// Write character's home squad & ID
		bs.Write(iter->second->GetHomeSquad());
		bs.Write(iter->second->GetID());
		iter->second->Write(bs);
	}

	// Write the action queue
	bs.Write((unsigned int)m_actionQueue.size());
	for (auto& iter = m_actionQueue.begin(); iter != m_actionQueue.end(); iter++)
	{
		(*iter)->Write(bs);
	}

}

#endif