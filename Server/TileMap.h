#pragma once

#include <vector>
#include <list>
#include <unordered_map>

enum COVER_VALUE { COVER_NONE = 0, COVER_LOW, COVER_HIGH };
enum MAP_CONNECTION_DIR { LEFT = 0, RIGHT, FRONT, BACK, 
							FRONTLEFT, FRONTRIGHT, BACKRIGHT, BACKLEFT };
enum MAP_CONNECTION_LEVEL { LEVEL = 0, UP, DOWN };

struct MapVec3
{
	short m_x, m_y, m_z;
	MapVec3() { m_x = 0; m_y = 0; m_z = 0; };
	MapVec3(short i) { m_x = i; m_y = i; m_z = i; };
	MapVec3(short x, short y, short z) { m_x = x; m_y = y; m_z = z; };
	const bool operator== (MapVec3& rhs) { return (m_x == rhs.m_x && m_y == rhs.m_y && m_z == rhs.m_z); };
	const bool operator!= (MapVec3& rhs) { return (m_x != rhs.m_x || m_y != rhs.m_y || m_z != rhs.m_z); };
	MapVec3& operator+ (MapVec3& rhs) 
	{
		m_x += rhs.m_x;
		m_y += rhs.m_y;
		m_z += rhs.m_z;
		return *this;
	}
};

class TileMap
{
private:
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

		std::list<MapTile*> m_connectedTiles;

		MapVec3 m_tilePosition;

	public:
		// CONSTRUCTORS
		MapTile() {};
		MapTile(MapVec3 pos) { m_tilePosition = pos; };

		// COVER INFO
		COVER_VALUE GetCoverLeft() { return GetCover(0); };
		COVER_VALUE GetCoverRight() { return GetCover(2); };
		COVER_VALUE GetCoverFront() { return GetCover(4); };
		COVER_VALUE GetCoverBack() { return GetCover(6); };

		void SetCoverLeft(const COVER_VALUE val) { SetCover(val, 0); };
		void SetCoverRight(const COVER_VALUE val) { SetCover(val, 2); };
		void SetCoverFront(const COVER_VALUE val) { SetCover(val, 4); };
		void SetCoverBack(const COVER_VALUE val) { SetCover(val, 6); };

		// PATHFINDING INFO
		void AddConnection(MapTile* connection) { m_connectedTiles.push_back(connection); };
		void RemoveConnection(MapTile* connected) { m_connectedTiles.remove(connected); };
		MapTile* GetConnection(MAP_CONNECTION_DIR dir, MAP_CONNECTION_LEVEL level = LEVEL)
		{
			// enum MAP_CONNECTION_DIR values translate to these offsets
			static MapVec3 m_offsetVecs[] = { MapVec3(-1,0,0),  MapVec3(1,0,0),
				MapVec3(0,0,1), MapVec3(0,0,-1), MapVec3(-1,0,1), MapVec3(1,0,1),
				MapVec3(1,0,-1), MapVec3(-1,0,-1) };
			static MapVec3 m_heightOffsetVecs[] = { MapVec3(0,0,0), MapVec3(0,1,0), MapVec3(0,-1,0) };

			for (auto& connection : m_connectedTiles)
			{
				// Check if connected tile's position is at current's plus offset
				MapVec3 offsetPosition = this->m_tilePosition + 
					m_offsetVecs[(unsigned int)dir] + m_heightOffsetVecs[(unsigned int) level];
				if (connection->m_tilePosition == offsetPosition)
					return connection;
			}
			return nullptr;
		}

		// GENERAL
		MapVec3 GetTilePos() { return m_tilePosition; };

	};

	struct MapPlane
	{
		std::unordered_map<std::pair<short, short>, MapTile*> m_tiles;
	};

	std::unordered_map<short, MapPlane> m_planes;

	TileMap::MapTile* FindTile(MapVec3 pos);

public:
	TileMap();
	~TileMap();

	void AddTile(MapVec3 pos, bool autoConnect = true);
	void AddTile(short x, short y, short z, bool autoConnect = true);

	std::vector<MapVec3> TileMap::FindPath(MapVec3 from, MapVec3 to);
};

