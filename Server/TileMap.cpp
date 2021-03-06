#include "TileMap.h"
using namespace JakePerry;
#include "GameMessages.h"

#include <list>

#ifndef NETWORK_SERVER
#include <gizmos.h>
#include <glm/ext.hpp>
#include "Game.h"
#endif



/* Deletes all nodes & planes from memory. */
void JakePerry::TileMap::ClearAllData()
{
	std::unordered_map<short, MapPlane>::iterator iter;
	for (iter = m_planes.begin(); iter != m_planes.end(); iter++)
	{
		iter->second.SafeDelete();
	}
	m_planes.clear();

	m_spawnPoints.clear();
}

void JakePerry::TileMap::ResetPathingData()
{
	// Loop through all planes
	for (auto& planeIter = m_planes.begin(); planeIter != m_planes.end(); planeIter++)
	{
		// Loop through all tiles
		for (auto& tileIter = planeIter->second.m_tiles.begin(); tileIter != planeIter->second.m_tiles.end(); tileIter++)
		{
			tileIter->second->ResetPathingData();
		}
	}
}

JakePerry::TileMap::MapTile * TileMap::FindTile(const MapVec3 pos) const
{
	// Find plane
	std::unordered_map<short, MapPlane>::const_iterator it = m_planes.find(pos.m_y);
	if (it != m_planes.end())
	{
		// Find tile
		const MapPlane* p = &( it->second );
		std::map<std::pair<short, short>, MapTile*>::const_iterator it2;
		it2 = p->m_tiles.find(std::make_pair(pos.m_x, pos.m_z));

		if (it2 != p->m_tiles.end())
		{
			// Return found tile
			return it2->second;
		}
	}
	return nullptr;
}

std::list<MapVec3> JakePerry::TileMap::AStarSearch(MapTile * from, MapTile * to, const std::list<MapVec3> obstacles) const
{
	// Check if the destination tile is in the obstacle list
	if (std::find(obstacles.begin(), obstacles.end(), to->GetTilePos()) != obstacles.end())
		return std::list<MapVec3>();

	std::list<MapTile*> openList;
	std::list<MapTile*> closedList;

	MapTile* currentNode = nullptr;

	from->gScore = 0;
	from->fScore = 0;
	from->previousNode = nullptr;
	openList.push_back(from);

	while (openList.size() > 0)
	{
		// Sort openList by fscore
		openList.sort([](const MapTile* a, const MapTile* b) { return a->fScore < b->fScore; });

		currentNode = *(openList.begin());

		if (currentNode == to)
			break;		// Found the end node

		// Remove current node from open, add to close
		openList.remove(currentNode);
		closedList.push_back(currentNode);

		// Loop through all connections
		std::unordered_map<MAP_CONNECTION_DIR, MapTileConnection*>::iterator cIter;
		auto& connections = currentNode->GetAllConnections();
		for (cIter = connections.begin(); cIter != connections.end(); cIter++)
		{
			MapTile* n = cIter->second->GetConnected(currentNode);

			// Check if the connected tile is contained in the obstacle list
			if (std::find(obstacles.begin(), obstacles.end(), n->GetTilePos()) != obstacles.end())
				continue;

			float newGScore = currentNode->gScore + cIter->second->GetWeight();
			float newHScore = MapVec3::Distance(n->GetTilePos(), to->GetTilePos());
			float newFScore = newGScore + newHScore;

			if (n->fScore > newFScore || n->previousNode == nullptr)
			{
				n->gScore = newGScore;
				n->hScore = newHScore;
				n->fScore = newFScore;
				n->previousNode = currentNode;
			}

			// Check if n is already in closedList
			bool traversed = false;
			auto nIter = std::find(closedList.begin(), closedList.end(), n);
			if (nIter != closedList.end())
			{
				if (*nIter != nullptr)
					traversed = true;
			}

			if (!traversed)
				openList.push_back(n);
		}
	}

	// Calculate path
	std::list<MapVec3> path;
	currentNode = to;
	while (currentNode != nullptr)
	{
		path.push_front(currentNode->GetTilePos());
		if (currentNode->previousNode != currentNode && currentNode != from)
		{
			// Retain this node & set current to the previous in path
			MapTile* lastNode = currentNode;
			currentNode = currentNode->previousNode;

			// Reset pathing data on 'lastNode'
			lastNode->ResetPathingData();
		}
		else
			break;	// This is the first node
	}

	// Clear all pathing data from traversed nodes
	ClearPathData(closedList);

	return path;
}

void JakePerry::TileMap::ClearPathData(std::list<MapTile*> list) const
{
	for (auto& iter = list.cbegin(); iter != list.cend(); iter++)
	{
		MapTile* tile = *iter;
		if (tile != nullptr)
			tile->ResetPathingData();
	}
}

#ifdef NETWORK_SERVER
/**
 * Internal use for checking sight between two tiles.
 */
bool JakePerry::TileMap::SightBetweenTiles(const MapVec3 from, const MapVec3 to) const
{
	if (from == to)
		return true;	// Error?

	// Check height cases

	if (from.m_y < to.m_y)
	{
		// Check if there is a tile at 'to' position
		if (FindTile(to) != nullptr)	return false;	// Sight line hits the cieling
	}
	else if (from.m_y > to.m_y)
	{
		// Check if there is a tile at 'from' position
		if (FindTile(from) != nullptr)	return false;	// Sight line hits the floor
	}

	// Check same-level directions with some nasty nested if-statements

	MAP_CONNECTION_DIR dir = from.GetDirectionTo(to);

	bool crossesHighCover = false;
	MapTile* tFrom = FindTile(from);

	if (tFrom == nullptr)
	{
		// Sight-line is currently in a space where no tiles exist.
		MapTile* tTo = FindTile(to);
		if (tTo == nullptr)	return true;	// No cover data could block this vision

		// Sight-line is entering a tile. Check the inverse of the 'dir' direction for cover values
		switch (dir)
		{
		case LEFT:
			crossesHighCover = (tTo->GetCoverRight() == COVER_HIGH);
			break;
		case RIGHT:
			crossesHighCover = (tTo->GetCoverLeft() == COVER_HIGH);
			break;
		case FRONT:
			crossesHighCover = (tTo->GetCoverBack() == COVER_HIGH);
			break;
		case BACK:
			crossesHighCover = (tTo->GetCoverFront() == COVER_HIGH);
			break;
		case FRONTLEFT:
			crossesHighCover = (tTo->GetCoverBack() == COVER_HIGH || tTo->GetCoverRight() == COVER_HIGH);
			break;
		case FRONTRIGHT:
			crossesHighCover = (tTo->GetCoverBack() == COVER_HIGH || tTo->GetCoverLeft() == COVER_HIGH);
			break;
		case BACKRIGHT:
			crossesHighCover = (tTo->GetCoverFront() == COVER_HIGH || tTo->GetCoverLeft() == COVER_HIGH);
			break;
		case BACKLEFT:
			crossesHighCover = (tTo->GetCoverFront() == COVER_HIGH || tTo->GetCoverRight() == COVER_HIGH);
			break;
		default:
			crossesHighCover = false;
			break;
		}
	}
	else
	{
		// Check 'dir' direction for cover values
		switch (dir)
		{
		case LEFT:
			crossesHighCover = (tFrom->GetCoverLeft() == COVER_HIGH);
			break;
		case RIGHT:
			crossesHighCover = (tFrom->GetCoverRight() == COVER_HIGH);
			break;
		case FRONT:
			crossesHighCover = (tFrom->GetCoverFront() == COVER_HIGH);
			break;
		case BACK:
			crossesHighCover = (tFrom->GetCoverBack() == COVER_HIGH);
			break;
		case FRONTLEFT:
			crossesHighCover = (tFrom->GetCoverFront() == COVER_HIGH || tFrom->GetCoverLeft() == COVER_HIGH);
			break;
		case FRONTRIGHT:
			crossesHighCover = (tFrom->GetCoverFront() == COVER_HIGH || tFrom->GetCoverRight() == COVER_HIGH);
			break;
		case BACKRIGHT:
			crossesHighCover = (tFrom->GetCoverBack() == COVER_HIGH || tFrom->GetCoverRight() == COVER_HIGH);
			break;
		case BACKLEFT:
			crossesHighCover = (tFrom->GetCoverBack() == COVER_HIGH || tFrom->GetCoverLeft() == COVER_HIGH);
			break;
		default:
			crossesHighCover = false;
			break;
		}
	}

	if (crossesHighCover)
		return false;	// Sight-line hits high cover
	
	return true;
}
#endif

JakePerry::TileMap::TileMap()
{
}

JakePerry::TileMap::~TileMap()
{
	ClearAllData();
}

void JakePerry::TileMap::AddTile(MapVec3 pos, unsigned char coverData, bool autoConnect)
{
	// Insert a plane at specified y-level if it does not exist
	MapPlane plane;
	m_planes.insert(std::make_pair(pos.m_y, plane));

	// Insert a tile at (x, z) in the plane
	MapTile* newTile = new MapTile(pos, &m_planes[pos.m_y], coverData);

	typedef std::map<std::pair<short, short>, MapTile*> xyTile;
	std::pair<xyTile::iterator, bool> tileCreated;

	tileCreated = m_planes[pos.m_y].m_tiles.insert
				(std::make_pair(std::make_pair(pos.m_x, pos.m_z), newTile));

	// If the tile already existed
	if (tileCreated.second == false)
	{
		delete newTile;
		return;
	}
	else
	{
		// Auto-connect functionality
		if (autoConnect)
		{
			// Offsets for tiles connected in same plane. These values correspond to MAP_CONNECTION_DIR values
			const MapVec3 m_offsetVecs[] = 
			{ 
				MapVec3(-1,0,0),  MapVec3(1,0,0), MapVec3(0,0,1), MapVec3(0,0,-1), 
				MapVec3(-1,0,1), MapVec3(1,0,1), MapVec3(1,0,-1), MapVec3(-1,0,-1) 
			};

			// Loop through adjacent tiles & add connections
			MapTile* temp = nullptr;
			for (int i = 0; i < 8; i++)		// 8 = number of directions in MAP_CONNECTION_DIR enum
			{
				MapVec3 offset = m_offsetVecs[i];
				temp = FindTile(offset + newTile->GetTilePos());
				if (temp != nullptr)
				{
					// Link the new tile and the adjacent tile

					float weight = ((float)i >= 4) ? 1.4142f : (float)1;	// weigh diagonal movements as root 2
					MapTileConnection* c = new MapTileConnection(temp, newTile, weight);

				}
			}
		}
	}
}

void JakePerry::TileMap::AddTile(short x, short y, short z, bool autoConnect)
{
	AddTile(MapVec3(x,y,z), autoConnect);
}

void JakePerry::TileMap::ClearSpawnPoints()
{
	m_spawnPoints.clear();
}

void JakePerry::TileMap::AddSpawnPoint(const MapVec3 pos)
{
	m_spawnPoints.push_back(pos);
}

void JakePerry::TileMap::RemoveSpawnPoint(const MapVec3 pos)
{
	for (auto& iter = m_spawnPoints.begin(); iter != m_spawnPoints.end(); iter++)
	{
		if (*iter == pos)
			m_spawnPoints.erase(iter, iter + 1);
	}
}

COVER_VALUE JakePerry::TileMap::GetCoverInDirection(const MapVec3 position, MAP_CONNECTION_DIR dir)
{
	// Find tile at this position
	MapTile* tile = FindTile(position);

	if (tile)
	{
		return tile->GetCoverDir(dir);
	}
	else
		return COVER_NONE;
}

bool JakePerry::TileMap::TileAt(const MapVec3 position) const
{
	return ( FindTile(position) != nullptr );
}

bool JakePerry::TileMap::TileIsInCover(const MapVec3 position) const
{
	MapTile* tile = FindTile(position);
	if (tile != nullptr)
	{
		return
			(tile->GetCoverBack() != COVER_NONE
				&&	tile->GetCoverFront() != COVER_NONE
				&&	tile->GetCoverLeft() != COVER_NONE
				&&	tile->GetCoverRight() != COVER_NONE);
	}
	return false;
}

std::list<MapVec3> JakePerry::TileMap::FindPath(const MapVec3 from, const MapVec3 to, const std::list<MapVec3> obstacles) const
{
	MapTile* origin = FindTile(from);
	if (origin)
	{
		MapTile* destination = FindTile(to);
		if (destination)
		{
			return AStarSearch(origin, destination, obstacles);
		}
		printf("Error: Specified destination could not be found in FindPath method.\n");
	}
	printf("Error: Specified origin could not be found in FindPath method.\n");
	return std::list<MapVec3>();
}

std::list<MapVec3> JakePerry::TileMap::GetWalkableTiles(const MapVec3 start, const int maxTravelDist) const
{
	std::list<MapVec3> firstList;
	std::list<MapVec3> secondList;
	float travelled = 0;

	firstList.push_back(start);

	while (travelled <= maxTravelDist)
	{
		// Loop through all tiles in first list
		std::list<MapVec3>::iterator iter;
		for (iter = firstList.begin(); iter != firstList.end(); iter++)
		{
			// Get reference to tile at this location
			MapTile* tile = FindTile(*iter);

			// Loop through each connection on this tile
			auto& connectedTiles = tile->GetAllConnections();
			for each (auto& ct in connectedTiles)
			{
				// Get the connected tile
				MapTileConnection* c = ct.second;
				MapTile* connected = c->GetConnected(tile);
				MapVec3 connectedPos = connected->GetTilePos();

				// Check if this tile is already in the second list
				bool contained = false;
				for each (MapVec3 mv in secondList)
				{
					if (mv == connectedPos)
					{
						contained = true;
						break;
					}
				}

				if (!contained)
					secondList.push_back(connectedPos);
			}
		}

		// Check if no new connections were found
		if (firstList == secondList)
			break;

		// Set first list to second & increment travelled distance
		firstList = secondList;
		travelled++;
	}

	return firstList;
}

/* Cast a ray through map voxels. NOTE: dir values must be of a normalized vector to work correctly */
std::list<MapVec3> JakePerry::TileMap::Raycast(const float x, const float y, const float z, const float dirX, const float dirY, const float dirZ, const float rayLength, const float tileScale) const
{
	// Function was based on write-up found at: http://www.cs.yorku.ca/~amana/research/grid.pdf

	// Initially found via: http://www.saltgames.com/article/lineOfSight/,
	// and their source-code at: http://www.saltgames.com/articles/lineOfSight/lineOfSightDemo.js

	if (dirX == 0 && dirY == 0 && dirZ == 0)
		return std::list<MapVec3>();

	// Get bounds of map. Raycast will stop when the ray exits the map bounds
	MapVec3 mapExtentsMin = MapVec3(0);
	MapVec3 mapExtentsMax = MapVec3(0);
	GetMapBounds(mapExtentsMin, mapExtentsMax);

	// Check if the ray is starting inside the map bounds
	bool isInBounds = false;
	MapVec3 startPos = MapVec3::FindTileAtWorldCoords(x, y, z, (tileScale > 0) ? tileScale : -tileScale);
	if (startPos.IsWithinBounds(mapExtentsMin, mapExtentsMax))
		isInBounds = true;
	
	std::list<MapVec3> resultList;

	// These values vary from step to step
	float tValue, xGrid, yGrid, zGrid, tNextBorderX, tNextBorderY, tNextBorderZ;

	// Constants
	const float tileScalePositive = (tileScale >= 0) ? tileScale : -tileScale;
	// TODO: Apparently abs is quite slow? look into this some time
	const float tForOneX = fabs(1.0f / dirX);
	const float tForOneY = fabs(1.0f / dirY);
	const float tForOneZ = fabs(1.0f / dirZ);
	const int xStep = (dirX >= 0) ? (int)tileScalePositive : (int)-tileScalePositive;
	const int yStep = (dirY >= 0) ? (int)tileScalePositive : (int)-tileScalePositive;
	const int zStep = (dirZ >= 0) ? (int)tileScalePositive : (int)-tileScalePositive;

	MapVec3 currentPos = MapVec3::FindTileAtWorldCoords(x, y, z, tileScalePositive);
	resultList.push_back(currentPos);

	// Start implementation
	tValue = 0;
	xGrid = floorf(x);
	yGrid = floorf(y);
	zGrid = floorf(z);

	float fracStartPosX = x - floor(x);
	if (dirX > 0)	tNextBorderX = (1 - fracStartPosX) * tForOneX;
	else			tNextBorderX = fracStartPosX * tForOneX;

	float fracStartPosY = y - floor(y);
	if (dirY > 0)	tNextBorderY = (1 - fracStartPosY) * tForOneY;
	else			tNextBorderY = fracStartPosY * tForOneY;

	float fracStartPosZ = z - floor(z);
	if (dirZ > 0)	tNextBorderZ = (1 - fracStartPosZ) * tForOneZ;
	else			tNextBorderZ = fracStartPosZ * tForOneZ;

	while (tValue <= rayLength)
	{
		// Find the smallest t-value until next map space
		float minNextBorder = (tNextBorderX < tNextBorderY) ? tNextBorderX : tNextBorderY;
		if (tNextBorderZ < minNextBorder)	minNextBorder = tNextBorderZ;

		/* NOTE: Check if difference is less than 0.000001 in order to allow diagonal movement when the ray is
		 * very close to the corner of it's containing map grid cube. */

		if (fabs(tNextBorderX - minNextBorder) < 0.000001)
		{
			tValue = tNextBorderX;
			tNextBorderX += tForOneX;
			xGrid += xStep;
		}
		if (fabs(tNextBorderY - minNextBorder) < 0.000001)
		{
			tValue = tNextBorderY;
			tNextBorderY += tForOneY;
			yGrid += yStep;
		}
		if (fabs(tNextBorderZ - minNextBorder) < 0.000001)
		{
			tValue = tNextBorderZ;
			tNextBorderZ += tForOneZ;
			zGrid += zStep;
		}

		currentPos = MapVec3::FindTileAtWorldCoords(xGrid, yGrid, zGrid, tileScalePositive);
		resultList.push_back(currentPos);

		// Has the ray been in the map bounds previously?
		if (isInBounds)
		{
			// Check if the ray is still within these bounds
			bool stillInsideBounds = currentPos.IsWithinBounds(mapExtentsMin, mapExtentsMax);

			if (!stillInsideBounds)		// The ray has exited the map bounds. Return
				return resultList;
		}
		else isInBounds = currentPos.IsWithinBounds(mapExtentsMin, mapExtentsMax);
	}

	return resultList;
}

void JakePerry::TileMap::GetMapBounds(MapVec3 & minBounds, MapVec3 & maxBounds) const
{
	if (m_planes.size() > 0)
	{
		short minX = (std::numeric_limits<short>::max)();
		short minY = (std::numeric_limits<short>::max)();
		short minZ = (std::numeric_limits<short>::max)();
		
		short maxX = (std::numeric_limits<short>::min)();
		short maxY = (std::numeric_limits<short>::min)();
		short maxZ = (std::numeric_limits<short>::min)();

		const MapPlane* minPlane = nullptr;
		const MapPlane* maxPlane = nullptr;

		// Find min & max planes
		for (auto& iter = m_planes.cbegin(); iter != m_planes.cend(); iter++)
		{
			short y = iter->first;
			if (y < minY)
			{
				minPlane = &(iter->second);
				minY = y;
			}
			if (y > maxY)
			{
				maxPlane = &(iter->second);
				maxY = y;
			}
		}

		// Get min coordinates
		if (minPlane != nullptr)
		{
			auto minPlaneTiles = minPlane->m_tiles;
			for (auto& iter = minPlaneTiles.cbegin(); iter != minPlaneTiles.cend(); iter++)
			{
				short x = iter->first.first;
				short z = iter->first.second;

				if (x < minX)	minX = x;
				if (z < minZ)	minZ = z;
			}

			minBounds = MapVec3(minX, minY, minZ);
		}
		else
			minBounds = MapVec3(0);

		// Get max coordinates
		if (maxPlane != nullptr)
		{
			auto maxPlaneTiles = maxPlane->m_tiles;
			for (auto& iter = maxPlaneTiles.cbegin(); iter != maxPlaneTiles.cend(); iter++)
			{
				short x = iter->first.first;
				short z = iter->first.second;

				if (x < maxX)	maxX = x;
				if (z < maxZ)	maxZ = z;
			}

			maxBounds = MapVec3(maxX, maxY, maxZ);
		}
		else
			maxBounds = MapVec3(0);
	}
	else
	{
		minBounds = MapVec3(0);
		maxBounds = MapVec3(0);
	}
}

#ifdef NETWORK_SERVER
/* Checks tiles over a sight-line to check whether or not sight is blocked. */
bool JakePerry::TileMap::CheckTileSight(const MapVec3 from, const MapVec3 to, const float tileScale, int maxSightRange) const
{
	if (MapVec3::Distance(from, to) <= maxSightRange)
	{
		// Get coordinates for center of 'from' tile
		float x1 = 0, y1 = 0, z1 = 0;
		MapVec3::GetTileWorldCoordsCenter(x1, y1, z1, from, tileScale);

		// Get coordinates for center of 'to' tile
		float x2 = 0, y2 = 0, z2 = 0;
		MapVec3::GetTileWorldCoordsCenter(x2, y2, z2, to, tileScale);

		// Find direction between the two points
		float dirX = x2 - x1;
		float dirY = y2 - y1;
		float dirZ = z2 - z1;

		float dist = MapVec3::Distance(from, to) + 0.1f;

		std::list<MapVec3> sightPath = Raycast(x1, y1, z1, dirX, dirY, dirZ, dist, tileScale);

		// Iterate through each tile in the path, checking sight between each adjacent tile
		for (auto& iter = sightPath.cbegin(); iter != sightPath.cend(); iter++)
		{
			// Get next tile along path
			auto& iterNext = std::next(iter, 1);
			if (iterNext != sightPath.cend())
			{
				MapVec3 thisTile = *iter;
				MapVec3 nextTile = *iterNext;

				if (!SightBetweenTiles(thisTile, nextTile))
					return false;
			}
		}

		return true;	// Sight through all tiles in path
	}

	return false;		// Destination tile out of sight range
}

#ifdef TILEMAP_INCLUDE_RAKNET_FUNCTIONS
/* Write & send the whole tilemap. */
void JakePerry::TileMap::Write(RakNet::BitStream& bs)
{
	// List of sent connections. Keeps track of which connections have been sent
	// to avoid sending twice
	std::list<ConnectionData> m_sentConnections;

	// Write number of planes
	unsigned int planesQuantity = (unsigned int)m_planes.size();
	bs.Write(planesQuantity);

	// Iterate through planes
	std::unordered_map<short, MapPlane>::iterator planeIter;
	for (planeIter = m_planes.begin(); planeIter != m_planes.end(); planeIter++)
	{
		// Write current plane key (height)
		short planeKey = planeIter->first;
		bs.Write(planeKey);

		// Write number of tiles in this plane
		unsigned int tilesQuantity = (unsigned int)((&planeIter->second)->m_tiles.size());
		bs.Write(tilesQuantity);

		// Iterate through tiles
		std::map<std::pair<short, short>, MapTile*>::iterator tileIter;
		for (tileIter = (&planeIter->second)->m_tiles.begin(); tileIter != (&planeIter->second)->m_tiles.end(); tileIter++)
		{
			// Write current tile key (x, z)
			std::pair<short, short> key = tileIter->first;
			MapVec3 currTilePos = tileIter->second->GetTilePos();
			bs.Write(key.first);
			bs.Write(key.second);

			// Write tile's cover data
			bs.Write( tileIter->second->GetCoverDataRaw() );

			auto& connections = tileIter->second->GetAllConnections();

			// Convert all connections to temp structs
			std::unordered_map<MAP_CONNECTION_DIR, MapTileConnection*>::iterator connectionIter;
			for (connectionIter = connections.begin(); connectionIter != connections.end(); connectionIter++)
			{
				// Get connection info
				MapTileConnection* connection = connectionIter->second;
				MapTile* connectedTile = connection->GetConnected(tileIter->second);
				MapVec3 connectedPos = connectedTile->GetTilePos();
				float cost = connectionIter->second->GetWeight();
				bool biDir = connection->IsBiDirectional();

				// Create a ConnectionData struct instance & add to list of sent connections
				ConnectionData c = ConnectionData(currTilePos, connectedPos, cost, biDir);

				// Find if the list already contains this connection
				bool alreadySent = std::find(m_sentConnections.begin(), m_sentConnections.end(), c) != m_sentConnections.end();
				if (!alreadySent)
					m_sentConnections.push_back(c);
			}

			// Write number of connections
			unsigned int connectionsQuantity = m_sentConnections.size();
			bs.Write(connectionsQuantity);

			// Send all connections data
			std::list<ConnectionData>::iterator sendIter;
			for (sendIter = m_sentConnections.begin(); sendIter != m_sentConnections.end(); sendIter++)
			{
				(sendIter->GetPos1()).Write(bs);
				(sendIter->GetPos2()).Write(bs);
				bs.Write(sendIter->GetWeight());
				bs.Write(sendIter->IsBiDirectional());
			}
		}
	}
}
#endif

#endif

#ifndef NETWORK_SERVER

#ifdef TILEMAP_INCLUDE_RAKNET_FUNCTIONS
/* Read a packet as a new tilemap. Wipes all old data if present. */
void JakePerry::TileMap::Read(RakNet::BitStream& bsIn)
{
	ClearAllData();

	// Declare a map to store connection keys temporarily. See further down where this info is read for more info
	std::list<ConnectionData*> m_connectKeys;
	
	// Read number of planes
	unsigned int planesQuantity = 0;
	bsIn.Read(planesQuantity);

	// Iterate through planes
	for (unsigned int i = 0; i < planesQuantity; i++)
	{
		// Read current plane's key & create a new plane
		short planeKey;
		bsIn.Read(planeKey);
		m_planes[planeKey] = MapPlane();
		MapPlane* currPlane = &(m_planes[planeKey]);

		// Read number of tiles in this plane
		unsigned int tilesQuantity = 0;
		bsIn.Read(tilesQuantity);

		// Iterate through tiles in current plane
		for (unsigned int j = 0; j < tilesQuantity; j++)
		{
			// Read current tile's key (x, z)
			std::pair<short, short> tileKey = std::make_pair(0, 0);
			bsIn.Read(tileKey.first);
			bsIn.Read(tileKey.second);

			// Read tile's cover data & create a new tile
			unsigned char tileCoverData = 0;
			bsIn.Read(tileCoverData);
			MapTile* currTile = new MapTile(MapVec3(tileKey.first, planeKey, tileKey.second), currPlane, tileCoverData);
			MapVec3 currPos = currTile->GetTilePos();

			// Add tile to current plane
			currPlane->m_tiles[tileKey] = currTile;

			// Read number of connected tiles
			unsigned int connectionsQuantity = 0;
			bsIn.Read(connectionsQuantity);

			// Read current tile's connection keys
			/* NOTE: The MapTile pointers for some of these tiles won't exist yet. 
			 * For now we will store these keys separately and then iterate through and
			 * add connections properly at the end.
			 */
			for (unsigned int k = 0; k < connectionsQuantity; k++)
			{
				// Read connection data
				ConnectionData* c = nullptr;
				MapVec3 pos1, pos2;
				float weight = 0.0f;
				bool biDir = false;

				//bsIn.Read(c);
				pos1.Read(bsIn);
				pos2.Read(bsIn);
				bsIn.Read(weight);
				bsIn.Read(biDir);

				c = new ConnectionData(pos1, pos2, weight, biDir);

				// Add this to the temp list of connections
				m_connectKeys.push_back(c);
			}
		}
	}

	// Now all planes & tiles have been created. We need to iterate through all 
	// temp connections and actually connect the tiles they correspond to

	// Iterate through each connection
	std::list<ConnectionData*>::iterator tileKeyIter;
	for (tileKeyIter = m_connectKeys.begin(); tileKeyIter != m_connectKeys.end(); tileKeyIter++)
	{
		// Get data
		ConnectionData* c = *tileKeyIter;
		MapVec3 from = c->GetPos1();
		MapTile* fromTile = FindTile(from);
		MapVec3 to = c->GetPos2();
		MapTile* toTile = FindTile(to);
		float weight = c->GetWeight();
		bool biDir = c->IsBiDirectional();

		// Create a connection
		MapTileConnection* con = new MapTileConnection(fromTile, toTile, weight, biDir);
	}

	// All connections have been created. Clean up temp data
	for each (ConnectionData* c in m_connectKeys)
		delete c;
}
#endif

/* Basic draw function, draws each tile using gizmos */
void JakePerry::TileMap::Draw() const
{
	float tileScale = Game::GetMapTileScale();

	for (auto& planeIter = m_planes.begin(); planeIter != m_planes.end(); planeIter++)
	{
		auto tiles = planeIter->second.m_tiles;
		for (auto& tileIter = tiles.begin(); tileIter != tiles.end(); tileIter++)
		{
			MapVec3 tilePos = tileIter->second->GetTilePos();
			glm::vec4 colour = glm::vec4(1, 1, 1, 0.6f);

			glm::vec3 backLeft = glm::vec3(tilePos.m_x * tileScale, tilePos.m_y * tileScale, tilePos.m_z * tileScale);
			glm::vec3 frontLeft = backLeft + glm::vec3(tileScale, 0, 0);
			glm::vec3 backRight = backLeft + glm::vec3(0, 0, tileScale);
			glm::vec3 frontRight = backLeft + glm::vec3(tileScale, 0, tileScale);
			
			if (FindTile(tilePos + MapVec3(-1, 0, 0)) != nullptr)
			{
				aie::Gizmos::addLine(backLeft, frontLeft, colour);	// Draw left border
			}
			if (FindTile(tilePos + MapVec3(0, 0, -1)) != nullptr)
			{
				aie::Gizmos::addLine(backLeft, backRight, colour);	// Draw back border
			}

			aie::Gizmos::addLine(backRight, frontRight, colour);
			aie::Gizmos::addLine(frontLeft, frontRight, colour);
		}
	}
}
#endif
