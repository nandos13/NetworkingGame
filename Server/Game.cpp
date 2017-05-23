#include "Game.h"
#include "TileMap.h"
#include "StandardGun.h"
#include "SniperRifle.h"
#include "Shotgun.h"

#include "ShootAction.h"
#include "MovementAction.h"
#include "RefreshWalkableTilesAction.h"
#include "SetPointsAction.h"
#include "SetVisibleEnemiesAction.h"
#include "StartNewTurnAction.h"
#include "HunkerAction.h"


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

/**
 * Creates an action to initialize each character's walkable tiles.
 * Should be used when initializing the game.
 */
GameAction * Game::CreateInitialWalkableTilesAction()
{
	GameAction* g = new GameAction();

	for (auto& iter = m_characters.cbegin(); iter != m_characters.cend(); iter++)
	{
		Character* c = iter->second;

		if (c != nullptr)
		{
			auto list1P = m_map->GetWalkableTiles(c->GetPosition(), c->GetMoveDistance());
			auto list2P = m_map->GetWalkableTiles(c->GetPosition(), c->GetDashDistance());

			// Remove any 1-point-walk tiles from the 2P list
			for (auto& iter1P = list1P.begin(); iter1P != list1P.end(); iter1P++)
			{
				for (auto& iter2P = list2P.begin(); iter2P != list2P.end(); iter2P++)
				{
					if ((*iter1P) == (*iter2P))
						iter2P = list2P.erase(iter2P);
				}
			}
			RefreshWalkableTilesAction* rwtA = new RefreshWalkableTilesAction(c, list1P, list2P);
			g->AddToQueue(rwtA);
		}
	}

	return g;
}

GameAction * Game::CreateInitialVisibleEnemiesAction()
{
	GameAction* g = new GameAction();

	for (auto& iter = m_characters.cbegin(); iter != m_characters.cend(); iter++)
	{
		Character* c = iter->second;

		if (c != nullptr)
		{
			auto visibleEnemies = GetVisibleEnemies(c);

			SetVisibleEnemiesAction* sveA = new SetVisibleEnemiesAction(c, visibleEnemies);
			g->AddToQueue(sveA);
		}
	}

	return g;
}

std::list<Character*> Game::GetVisibleEnemies(Character * lookUnit)
{
	Squad* waitingSquad = GetWaitingSquad();
	MapVec3 characterPos = lookUnit->GetPosition();

	auto enemyUnits = waitingSquad->GetAllCharacters();
	std::list<Character*> visibleEnemies;
	unsigned int characterSightRadius = lookUnit->GetSightRadius();

	// Iterate through all enemy-controlled units and add them to the list if they are visible
	for (auto& iter = enemyUnits.begin(); iter != enemyUnits.end(); iter++)
	{
		MapVec3 enemyPos = (*iter)->GetPosition();
		if (enemyPos != characterPos)
		{
			if (m_map->CheckTileSight(characterPos, enemyPos, m_tileScale, characterSightRadius))
				visibleEnemies.push_back((*iter));
		}
	}

	return visibleEnemies;
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

/* Checks if the current player has no remaining points on any characters. If so, ends the turn */
void Game::QueryTurnEnd(GameAction * g)
{
	Squad* currentlyPlaying = GetPlayingSquad();
	if (currentlyPlaying)
	{
		auto characters = currentlyPlaying->GetAllCharacters();

		// Iterate through each character in the squad & check if they have any remaining action points
		for (auto& iter = characters.begin(); iter != characters.end(); iter++)
		{
			if ((*iter)->RemainingActionPoints() > 0)
				return;
		}

		// The function did not return, therefore all points are spent. End the turn
		CreateTurnEndAction(g);
	}
}

void Game::CreateTurnEndAction(GameAction * g)
{
	if (g != nullptr)
	{
		bool playerOneTurn = (m_currentTurn == 0) ? false : true;
		StartNewTurnAction* sntA = new StartNewTurnAction(nullptr, playerOneTurn);
		g->AddToQueue(sntA);
	}
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

	m_forcedSpectator = false;
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
void Game::QueueAction(GameAction* action)
{
	if (action == nullptr) { return; };
	m_actionQueue.push_back(action);
	if (m_actionQueue.size() == 1)
	{
		// This was the first action to be added. Set current action
		m_currentAction = m_actionQueue.begin();
	}

	if (m_currentAction == m_actionQueue.end())
	{
		// Set iterator to first non-complete action
		for (auto& iter = m_actionQueue.begin(); iter != m_actionQueue.end(); iter++)
		{
			if (!(*iter)->IsCompleted())
			{
				m_currentAction = iter;
				break;
			}
		}
	}
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

/* Returns a list of all characters which initially belonged to the specified squad. */
std::list<Character*> Game::GetCharactersByHomeSquad(const unsigned int squad) const
{
	std::list<Character*> returnList;
	for (auto& iter = m_characters.cbegin(); iter != m_characters.cend(); iter++)
	{
		if ((*iter).second->GetHomeSquad() == squad)
			returnList.push_back((*iter).second);
	}
	return returnList;
}

int Game::GetShotChance(const Character * shooter, MapVec3 target)
{
	if (shooter)
	{
		// Get the direction from the target square to shooter
		MAP_CONNECTION_DIR dir = target.GetDirectionTo(shooter->GetPosition());

		// Is the target covered from this direction?
		COVER_VALUE targetCover = m_map->GetCoverInDirection(target, dir);
		unsigned int targetCoverVal = 0;
		if (targetCover == COVER_HIGH) targetCoverVal = 40;
		else if (targetCover == COVER_LOW) targetCoverVal = 20;

		Character* targetCharacter = FindCharacterAtCoords(target);

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
			unsigned int def = targetCharacter->GetCurrentDefense() + targetCoverVal;
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

void Game::SetSpectatorMode(const bool state)
{
	m_forcedSpectator = state;
}

/** 
 * Ends the turn for the current player and passes control to another. 
 * If playerOne == true, player one's turn starts. Else, control is given to player two.
 */
void Game::SetTurn(const bool playerOne)
{
	// Get the squad that is not currently playing
	Squad* currentlyWaiting = GetWaitingSquad();
	if (currentlyWaiting != nullptr)
	{
		// Reset action points, etc for this squad before we pass control
		currentlyWaiting->StartTurn();
	}

	m_currentTurn = (playerOne) ? 0 : 1 ;

	// Client-Side: Notify the player that their turn has started or ended
#ifndef NETWORK_SERVER
	if (m_currentTurn == m_mySquad)
		printf("Your turn has started!\n");
	else
		printf("Your turn has ended!\n");
#endif
}

bool Game::IsSpectator() const
{
	return m_forcedSpectator;
}

bool Game::IsPlayersTurn(const unsigned int playerID) const
{
	if (m_currentTurn == playerID)
		return true;
	return false;
}

#ifndef NETWORK_SERVER
void Game::Draw()
{
	m_squads[0].Draw();
	m_squads[1].Draw();
	m_map->Draw();
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
		Character* c = Character::Read(bsIn);
		short id = c->GetID();
		short squad = c->GetHomeSquad();

		m_characters[id] = c;

		// Place the character in the correct squad
		if (squad != 0 && squad != 1)
			printf("Error: Character squad was read as %d. Only 0 or 1 is allowed.\n", (int)squad);
		else
			m_squads[squad].AddMember(c);
	}

	// Change colour of all controlled units to green
	Squad* mySquad = GetMySquad();
	if (mySquad != nullptr)
	{
		auto characters = mySquad->GetAllCharacters();

		for (auto& iter = characters.begin(); iter != characters.end(); iter++)
		{
			(*iter)->SetGameobjectColour(0.1f, 1.0f, 0.15f, 0.5f);
		}
	}

	// Read the action queue
	unsigned int queueSize = 0;
	bsIn.Read(queueSize);
	for (unsigned int i = 0; i < queueSize; i++)
	{
		GameAction* gA = new GameAction();
		gA->Read(bsIn);

		// Add the action to the queue
		QueueAction(gA);
	}
}

/* Allows a client to take control of a squad. Use 1 or 2 as input parameter */
void Game::TakeControlOfSquad(const int squad)
{
	// TODO: This is inconsistent. Some functions take 0-1, this takes 1-2
	if (squad == 1)
		m_mySquad = 0;
	else if (squad == 2)
		m_mySquad = 1;

	// Note: m_mySquad stores the index of squad in array m_squads.
}

/* Returns a list of the characters which are currently selectable, based on the player's team. */
std::list<Character*> Game::GetSelectableCharacters() const
{
	if (m_mySquad != 0 && m_mySquad != 1)
		return std::list<Character*>();

	return m_squads[m_mySquad].GetSelectableCharacters();
}

bool Game::IsMyTurn() const
{
	return (m_currentTurn == m_mySquad) ? true : false;
}

Squad * Game::GetMySquad()
{
	if (m_mySquad == 0 || m_mySquad == 1)
		return &m_squads[m_mySquad];
	return nullptr;
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
 * Handle character shooting. 
 * Returns null if the shot is not possible.
 */
GameAction * Game::CreateShootAction(const short shooterID, short victimID)
{
	// Find the referenced character
	auto& cIter = m_characters.find(shooterID);
	if (cIter != m_characters.end())
	{
		Character* c = cIter->second;
		if (c != nullptr)
		{
			// Get victim character
			Character* victim = FindCharacterByID(victimID);
			if (victim != nullptr)
			{
				// Get victim's position
				MapVec3 vicPos = victim->GetPosition();

				// Get shot info
				short damage = 0;
				SHOT_STATUS shotType = MISS;

				GetShotVariables(damage, shotType, c, vicPos);

				// Create an action
				GameAction* g = new GameAction();

				ShootAction* sA = new ShootAction(c, vicPos, damage, shotType/*, ammoUse, shred*/);	// TODO: Get ammo use & shred state
				g->AddToQueue(sA);

				// TODO: Some abilities may allow a show to be taken on first turn, etc.
				// Check for this first
				SetPointsAction* spA = new SetPointsAction(c, 0);
				g->AddToQueue(spA);

				QueryTurnEnd(g);

				// Simulate on server
				while (!g->IsCompleted())	g->Execute(0);

				return g;
			}
		}
		else
			printf("Error: CreateShootAction function could not find character with id %i.\n", shooterID);
	}
	else
		printf("Error: Could not find character.\n");

	return nullptr;
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

			// Check if there is already a character at the destination tile
			Character* destCharacter = FindCharacterAtCoords(coords);
			if (destCharacter != nullptr)
			{
				printf("Warning: Tried to move to an already occupied space.\n");
				return nullptr;
			}

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
			unsigned int pointsToMove = c->PointsToMove((short)path.size());
			if (pointsToMove == 0)
			{
				printf("Error: Character (id: %d) cannot legally move %d tiles.\n", characterID, (int)path.size());
				return nullptr;
			}

			// Create an action
			GameAction* g = new GameAction();
			Squad* waitingSquad = GetWaitingSquad();

			// Use up action points
			// TODO: Run & Gun, etc abilities may allow a move to be taken without taking a point
			unsigned int newPointValue = c->RemainingActionPoints() - pointsToMove;
			if (newPointValue < 0)		newPointValue = 0;
			else if (newPointValue > 2)	newPointValue = 2;
			SetPointsAction* spA = new SetPointsAction(c, newPointValue);
			g->AddToQueue(spA);

			// Iterate through tiles in the path
			std::list<MapVec3>::iterator pathIter;
			for (pathIter = path.begin(); pathIter != path.end(); pathIter++)
			{
				// Do not create a move for the first tile, as FindPath returns a list with the origin tile included
				if (pathIter == path.begin())
					continue;

				MapVec3 thisPos = c->GetPosition();
				MapVec3 nextPos = (*pathIter);

				// Check for overwatch triggers. If any are triggered, actions will be added to the list
				waitingSquad->QueryOverwatch(g, c, *m_map);

				// Simulate on the server
				// NOTE: Execute takes a float, but this does not do anything when simulated on the server, 
				// as everything is done instantly
				while (!g->IsCompleted())	g->Execute(0);

				// Create movement to next tile in the path
				if (c->Alive())
				{
					MovementAction* mA = new MovementAction(c, nextPos);
					g->AddToQueue(mA);
				}

				// Simulate move on server
				while (!g->IsCompleted())	g->Execute(0);
			}

			// Check overwatch triggers for final tile move
			waitingSquad->QueryOverwatch(g, c, *m_map);

			// Simulate last overwatch
			while (!g->IsCompleted())	g->Execute(0);

			// Set the moving character's moveable tiles lists & visible enemies list
			if (c->Alive())
			{
				/* Update Walkable Tiles Lists */
				auto list1P = m_map->GetWalkableTiles(c->GetPosition(), c->GetMoveDistance());
				auto list2P = m_map->GetWalkableTiles(c->GetPosition(), c->GetDashDistance());

				// Remove any 1-point-walk tiles from the 2P list
				for (auto& iter1P = list1P.begin(); iter1P != list1P.end(); iter1P++)
				{
					for (auto& iter2P = list2P.begin(); iter2P != list2P.end(); iter2P++)
					{
						if ((*iter1P) == (*iter2P))
							iter2P = list2P.erase(iter2P);
					}
				}

				// Remove any 1-point-walk tiles from the 2P list
				RefreshWalkableTilesAction* rwtA = new RefreshWalkableTilesAction(c, list1P, list2P);
				g->AddToQueue(rwtA);

				/* Update Visible Enemies List */
				auto visibleEnemies = GetVisibleEnemies(c);

				SetVisibleEnemiesAction* sveA = new SetVisibleEnemiesAction(c, visibleEnemies);
				g->AddToQueue(sveA);

				// Simulate last overwatch
				while (!g->IsCompleted())	g->Execute(0);
			}

			QueryTurnEnd(g);

			// Reset the game action. 
			// (Reverses the effects of simulating it on the server, so it is again ready to use on a client)
			// TODO: Does this actually need to be done here? Write function does not necessarily care if the action is already run.
			// I may be able to get rid of this.
			g->Reset();

			return g;
		}
		else
			printf("Error: CreateMoveAction function could not find character with id %i.\n", characterID);
	}
	else
		printf("Error: Could not find character.\n");

	return nullptr;
}

GameAction * Game::CreateHunkerAction(const short characterID)
{
	// Find the referenced character
	auto& cIter = m_characters.find(characterID);
	if (cIter != m_characters.end())
	{
		Character* c = cIter->second;
		if (c != nullptr)
		{
			// TODO: Hunker should only be useable in cover. Check for cover here

			GameAction* g = new GameAction();

			HunkerAction* hA = new HunkerAction(c);
			g->AddToQueue(hA);

			SetPointsAction* spA = new SetPointsAction(c, 0);
			g->AddToQueue(spA);

			QueryTurnEnd(g);

			// Simulate on server
			while (!g->IsCompleted())	g->Execute(0);

			return g;
		}
		else
			printf("Error: CreateHunkerAction function could not find character with id %i.\n", characterID);
	}
	else
		printf("Error: Could not find character.\n");

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

	// Loop through & create both squads
	MapVec3 tempSpawn = MapVec3(0);
	for (unsigned int sq = 0; sq < 2; sq++)
	{
		printf("Creating squad %i\n", sq);
		// Add a heavy gunner
		{
			Character* c = new Character(id, sq, 5, 70, 9);
			c->SetPosition(tempSpawn);
			tempSpawn = tempSpawn + MapVec3(1,0,1);
			m_characters[id] = c;
			id++;
			m_squads[sq].AddMember(c);
			StandardGun* g = new StandardGun(3, 3, 5);
			c->SetPrimaryGun(g);
		}

		// Add an assault trooper
		{
			Character* c = new Character(id, sq, 4, 76, 12);
			c->SetPosition(tempSpawn);
			tempSpawn = tempSpawn + MapVec3(1, 0, 1);
			m_characters[id] = c;
			id++;
			m_squads[sq].AddMember(c);
			StandardGun* g = new StandardGun(4, 2, 4, -2, 10);
			c->SetPrimaryGun(g);
		}

		// Add a sniper
		{
			Character* c = new Character(id, sq, 3, 81, 10);
			c->SetPosition(tempSpawn);
			tempSpawn = tempSpawn + MapVec3(1, 0, 1);
			m_characters[id] = c;
			id++;
			m_squads[sq].AddMember(c);
			StandardGun* g = new StandardGun(3, 3, 5, 9, 25);
			c->SetPrimaryGun(g);
		}

		// Add a support shotgunner
		{
			Character* c = new Character(id, sq, 4, 76, 13);
			c->SetPosition(tempSpawn);
			tempSpawn = tempSpawn + MapVec3(1, 0, 1);
			m_characters[id] = c;
			id++;
			m_squads[sq].AddMember(c);
			StandardGun* g = new StandardGun(4, 3, 5, -8, 20);
			c->SetPrimaryGun(g);
		}
	}

	// Create a basic tile map
	printf("Creating a basic tilemap.\n");
	for (int i = -8; i <= 8; i++)
	{
		for (int j = -8; j <= 8; j++)
		{
			m_map->AddTile(i, 0, j, true);
		}
	}
	// TODO: Spawn points on map

	printf("Initializing characters' walkable tiles lists.\n");
	GameAction* gWalkTiles = CreateInitialWalkableTilesAction();
	QueueAction(gWalkTiles);

	printf("Initializing characters' visible enemies list.\n");
	GameAction* gVisibleEnemies = CreateInitialVisibleEnemiesAction();
	QueueAction(gVisibleEnemies);

	printf("Game generation done.\n");
}

void Game::Write(RakNet::BitStream & bs)
{
	// Write Tilemap data
	m_map->Write(bs);
	bs.Write(m_tileScale);

	// Write Character data
	bs.Write((unsigned int)m_characters.size());
	for (auto& iter = m_characters.begin(); iter != m_characters.end(); iter++)
	{
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