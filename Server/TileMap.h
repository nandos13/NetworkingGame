#pragma once

#include "RakPeerInterface.h"

#include <vector>
#include <unordered_map>
#include <map>
#include <BitStream.h>

enum COVER_VALUE { COVER_NONE = 0, COVER_LOW, COVER_HIGH };
enum MAP_CONNECTION_DIR { LEFT = 0, RIGHT, FRONT, BACK, 
							FRONTLEFT, FRONTRIGHT, BACKRIGHT, BACKLEFT };
enum MAP_CONNECTION_LEVEL { LEVEL = MAP_CONNECTION_DIR::BACKLEFT + 1, UP, DOWN };

struct MapVec3
{
	short m_x, m_y, m_z;
	MapVec3() { m_x = 0; m_y = 0; m_z = 0; };
	MapVec3(short i) { m_x = i; m_y = i; m_z = i; };
	MapVec3(short x, short y, short z) { m_x = x; m_y = y; m_z = z; };
	const bool operator== (const MapVec3& rhs) const { return (m_x == rhs.m_x && m_y == rhs.m_y && m_z == rhs.m_z); };
	const bool operator!= (const MapVec3& rhs) { return (m_x != rhs.m_x || m_y != rhs.m_y || m_z != rhs.m_z); };
	MapVec3& operator+ (const MapVec3& rhs) 
	{
		m_x += rhs.m_x;
		m_y += rhs.m_y;
		m_z += rhs.m_z;
		return *this;
	}
	MapVec3& operator- (const MapVec3& rhs)
	{
		m_x -= rhs.m_x;
		m_y -= rhs.m_y;
		m_z -= rhs.m_z;
		return *this;
	}
};
//#define NETWORK_SERVER	// !!! TODO: REMOVE THIS WHEN DONE WRITING FUNCTIONALITY FOR SERVER !!!
class TileMap
{
private:
	struct MapPlane;
	struct MapTileConnection;
	// Struct storing info about each individual tile
	struct MapTile
	{
	private:
		/**
		* Use a single byte to store info about cover values in all directions.
		* This uses only an 8th of the data when compared to storing 4x short-types,
		* which makes transferring map data over the network much faster.
		*/
		unsigned char m_coverVals = 0;
#ifdef NETWORK_SERVER
		void SetCover(const COVER_VALUE val, short bitOffset = 0)
		{
			/** HOW IT WORKS
			* Each direction from the tile has two bits of info. The first bit (right to left)
			* is the value of the cover (0 = low cover, 1 = high cover).
			* The second bit represents whether or not there is cover in that direction.
			* If there is no cover, this will be set to 0.
			*
			* Example: 00101110 represents a tile (00, 10, 11, 10) where:
			* Left (bitOffset = 0, bits = '10') is low cover. There is cover but it is not high.
			* Right (bitOffset = 2, bits = '11') is high cover. There is cover and it is high.
			* Front (bitOffset = 4, bits = '10') is low cover. There is cover but it is not high.
			* Bottom (bitOffset = 6, bits = '00') is not covered. There is no cover in the second bit (left to right).
			*/

			// Set the bit for low or high cover
			m_coverVals &= ~(1 << bitOffset);								// Clear the bit
			m_coverVals |= ((val == COVER_HIGH) ? 1 : 0) << bitOffset;		// Set the bit to 1 if cover is high

																			// Set the bit for cover existence
			m_coverVals &= ~(1 << bitOffset + 1);							// Clear the bit
			m_coverVals |= ((val == COVER_NONE) ? 0 : 1) << bitOffset + 1;	// Set the bit to 0 if there is no cover in this direction
		}
#endif
		COVER_VALUE GetCover(short bitOffset)
		{
			// Isolate the two bits corresponding to the direction we are checking
			unsigned char isolatedBits = (3 << bitOffset) & (m_coverVals);

			// Shift this back by offset
			if (bitOffset > 0)
				isolatedBits = isolatedBits >> bitOffset;

			switch (isolatedBits)
			{
			case 0:
				return COVER_NONE;
			case 2:
				return COVER_LOW;
			case 3:
				return COVER_HIGH;
			default:
				return COVER_NONE;
			}
		}

		std::unordered_map<MAP_CONNECTION_DIR, MapTileConnection*> m_connectedTiles;

		MapVec3 m_tilePosition;
		MapPlane* m_parentPlane;

	public:
		// CONSTRUCTORS

		MapTile(MapVec3 pos, MapPlane* parentPlane, unsigned char coverData) 
		{ 
			m_parentPlane = parentPlane;
			m_tilePosition = pos;
			m_coverVals = coverData;
		}
		~MapTile() {};

		// COVER INFO

		COVER_VALUE GetCoverLeft() { return GetCover(0); };
		COVER_VALUE GetCoverRight() { return GetCover(2); };
		COVER_VALUE GetCoverFront() { return GetCover(4); };
		COVER_VALUE GetCoverBack() { return GetCover(6); };

#ifdef NETWORK_SERVER
		void SetCoverLeft(const COVER_VALUE val) { SetCover(val, 0); };
		void SetCoverRight(const COVER_VALUE val) { SetCover(val, 2); };
		void SetCoverFront(const COVER_VALUE val) { SetCover(val, 4); };
		void SetCoverBack(const COVER_VALUE val) { SetCover(val, 6); };
		unsigned char GetCoverDataRaw() { return m_coverVals; };
#endif

		// PATHFINDING INFO

		void AddConnection(MapTileConnection* connection) 
		{
			MAP_CONNECTION_DIR dir = connection->GetDirection(this);
			m_connectedTiles[dir] = connection;
			//m_connectedTiles.insert(dir, connection);
		}
		void RemoveConnection(MapTileConnection* connected)
		{
			if (connected != nullptr)
			{
				MAP_CONNECTION_DIR dir = connected->GetDirection(this);
				m_connectedTiles.erase(dir);
			}
		}
		bool IsConnected(MapTile* tile)
		{
			std::unordered_map<MAP_CONNECTION_DIR, MapTileConnection*>::iterator iter;
			for (iter = m_connectedTiles.begin(); iter != m_connectedTiles.end(); iter++)
			{
				if (iter->second->GetConnected(this) == tile)
					return true;
			}
			return false;
		}
#ifdef NETWORK_SERVER
		std::unordered_map<MAP_CONNECTION_DIR, MapTileConnection*> GetAllConnections() { return m_connectedTiles; };
#endif
		MapTile* GetConnectedTile(MAP_CONNECTION_DIR dir, MAP_CONNECTION_LEVEL level = LEVEL)
		{
			// Offsets for tiles connected in same plane. These values correspond to MAP_CONNECTION_DIR values
			const MapVec3 m_offsetVecs[] = { MapVec3(-1,0,0),  MapVec3(1,0,0),
				MapVec3(0,0,1), MapVec3(0,0,-1), MapVec3(-1,0,1), MapVec3(1,0,1),
				MapVec3(1,0,-1), MapVec3(-1,0,-1) };
			
			short offsetHeight = (level == UP) ? 1 : ( (level == DOWN) ? -1 : 0 );
			MapVec3 offsetPos;
			if (dir <= MAP_CONNECTION_DIR::BACKLEFT)
				offsetPos = this->m_tilePosition + m_offsetVecs[(unsigned int)dir];
			else return nullptr;	// Prevent LEVEL enum (which extends DIR) from being passed in
			MapVec3 connectionPos;
			for (auto& connection : m_connectedTiles)
			{
				MapTile* connectedTile = (connection.second)->GetConnected(this);
				connectionPos = connectedTile->m_tilePosition;

				// Check if connected tile's position is at current's plus offset
				if (offsetPos.m_x == connectionPos.m_x && offsetPos.m_z == connectionPos.m_z)
				{
					// Check connected tile's height matches specified level
					bool connected = false;
					switch (level)
					{
					case UP:
						connected = connectionPos.m_y > m_tilePosition.m_y;
						break;
					case DOWN:
						connected = connectionPos.m_y < m_tilePosition.m_y;
						break;
					default:
						connected = connectionPos.m_y == m_tilePosition.m_y;
						break;
					}

					if (connected)
						return connectedTile;
				}
			}
			return nullptr;
		}

		// GENERAL

		MapVec3 GetTilePos() { return m_tilePosition; };
		void SafeDelete() 
		{ 
			// Remove this tile as a connection from all connected
			auto& iter = m_connectedTiles.begin();
			while (iter != m_connectedTiles.end())
			{
				iter->second->SafeDelete();
				iter++;
			}
			// Remove this tile from it's plane
			if (m_parentPlane != nullptr)
				m_parentPlane->m_tiles.erase(std::make_pair(m_tilePosition.m_x, m_tilePosition.m_z));
			// Delete from memory
			delete this;
		};

	};

	// TODO: Implement a way to have mono-direction connections (eg for jumping off a ledge).
	struct MapTileConnection
	{
	private:
		MapTile* nodeA;
		MapTile* nodeB;

		float weight;

	public:

		MapTileConnection(MapTile* a, MapTile* b, float cost)
		{
			if (nodeA && nodeB)
			{
				nodeA = a;
				nodeB = b;
				a->AddConnection(this);
				b->AddConnection(this);
				weight = cost;
			}
			else
				delete this;
		}

		/* Returns the node connected to 'me' */
		MapTile* GetConnected(MapTile* me)
		{
			return ( (nodeA == me) ? nodeB : nodeA );
		}

		/* Returns the direction of connection for tile 'me' */
		MAP_CONNECTION_DIR GetDirection(MapTile* me)
		{
			MapVec3 from, to;
			if (nodeA == me)
			{
				from = nodeA->GetTilePos();
				to = nodeB->GetTilePos();
			}
			else
			{
				from = nodeB->GetTilePos();
				to = nodeA->GetTilePos();
			}

			// Check up-down case
			if (from.m_y != to.m_y)
				return (from.m_y > to.m_y) ? (MAP_CONNECTION_DIR)MAP_CONNECTION_LEVEL::DOWN : (MAP_CONNECTION_DIR)MAP_CONNECTION_LEVEL::UP;
			else
			{ // Same plane. Check all directions
				MapVec3 difference = from - to;

				// Offsets for tiles connected in same plane. These values correspond to MAP_CONNECTION_DIR values
				const MapVec3 m_offsetVecs[] =
				{
					MapVec3(-1,0,0),  MapVec3(1,0,0), MapVec3(0,0,1), MapVec3(0,0,-1),
					MapVec3(-1,0,1), MapVec3(1,0,1), MapVec3(1,0,-1), MapVec3(-1,0,-1)
				};

				for (int i = 0; i < 8; i++) // 9 = number of directions in MAP_CONNECTION_DIR enum
				{
					if (m_offsetVecs[i] == difference)
						return (MAP_CONNECTION_DIR)i;		// Cast i to DIR & return
				}
			}

			printf("ERROR: MapTileConnection::GetDirection found two connected nodes with same position.\n");
			return (MAP_CONNECTION_DIR)MAP_CONNECTION_LEVEL::LEVEL;
		}

		float GetWeight() { return weight; };
		
		void SafeDelete()
		{
			if (nodeA != nullptr)
				nodeA->RemoveConnection(this);
			if (nodeB != nullptr)
				nodeB->RemoveConnection(this);

			delete this;
		}
	};

	struct MapPlane
	{
		~MapPlane() 
		{
			SafeDelete();
		}

		void SafeDelete()
		{
			// Delete all nodes in this plane
			std::map<std::pair<short, short>, MapTile*>::iterator iter;
			for (iter = m_tiles.begin(); iter != m_tiles.end(); iter++)
				delete iter->second;
				//iter->second->SafeDelete();

			delete this;
		}

		std::map<std::pair<short, short>, MapTile*> m_tiles;
	};

	std::unordered_map<short, MapPlane> m_planes;
	void ClearAllData();

	TileMap::MapTile* FindTile(MapVec3 pos);

public:
	TileMap();
	~TileMap();

	void AddTile(MapVec3 pos, unsigned char coverData = 0, bool autoConnect = true);
	void AddTile(short x, short y, short z, bool autoConnect = true);

	std::vector<MapVec3> TileMap::FindPath(MapVec3 from, MapVec3 to);

#ifdef NETWORK_SERVER
	void WriteTilemapNew(RakNet::RakPeerInterface* pPeerInterface, RakNet::SystemAddress & address);
	void WriteTilemapDiff(RakNet::RakPeerInterface* pPeerInterface, RakNet::SystemAddress & address);
#endif

#ifndef NETWORK_SERVER
	void ReadTilemapNew(RakNet::Packet* packet);
	void ReadTilemapDiff(RakNet::Packet* packet);
#endif
};

