#include "TileMap.h"
#include "GameMessages.h"

#include <list>



/* Deletes all nodes & planes from memory. */
void TileMap::ClearAllData()
{
	std::unordered_map<short, MapPlane>::iterator iter;
	for (iter = m_planes.begin(); iter != m_planes.end(); iter++)
	{
		iter->second.SafeDelete();
	}
	m_planes.clear();
}

TileMap::MapTile * TileMap::FindTile(MapVec3 pos)
{
	MapPlane* p = &( m_planes[pos.m_y] );
	if (p != nullptr)
	{
		return ( p->m_tiles[std::make_pair(pos.m_x, pos.m_z)] );
	}
	return nullptr;
}

TileMap::TileMap()
{
}


TileMap::~TileMap()
{
	ClearAllData();
}

void TileMap::AddTile(MapVec3 pos, unsigned char coverData, bool autoConnect)
{
	// Insert a plane at specified y-level if it does not exist
	m_planes.insert(std::make_pair(pos.m_y, MapPlane()));

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

					float weight = (i >= 4) ? 1.4142 : 1;	// weigh diagonal movements as root 2
					MapTileConnection* c = new MapTileConnection(temp, newTile, weight);

				}
			}
		}
	}
}

void TileMap::AddTile(short x, short y, short z, bool autoConnect)
{
	AddTile(MapVec3(x,y,z), autoConnect);
}

std::vector<MapVec3> TileMap::FindPath(MapVec3 from, MapVec3 to)
{
	MapTile* origin = FindTile(from);
	if (origin)
	{
		MapTile* destination = FindTile(to);
		if (destination)
		{
			// TODO: Code here
		}
	}
	return std::vector<MapVec3>();
}

#ifdef NETWORK_SERVER
/* Write & send the whole tilemap. */
void TileMap::WriteTilemapNew(RakNet::RakPeerInterface * pPeerInterface, RakNet::SystemAddress & address)
{
	RakNet::BitStream bs;
	bs.Write((RakNet::MessageID)GameMessages::ID_SERVER_SEND_TILEMAP);

	// Write number of planes
	unsigned int planesQuantity = m_planes.size();
	bs.Write(planesQuantity);

	// Iterate through planes
	std::unordered_map<short, MapPlane>::iterator planeIter;
	for (planeIter = m_planes.begin(); planeIter != m_planes.end(); planeIter++)
	{
		// Write current plane key (height)
		bs.Write(&planeIter->first);

		// Write number of tiles in this plane
		unsigned int tilesQuantity = (&planeIter->second)->m_tiles.size();
		bs.Write(tilesQuantity);

		// Iterate through tiles
		std::map<std::pair<short, short>, MapTile*>::iterator tileIter;
		for (tileIter = (&planeIter->second)->m_tiles.begin(); tileIter != (&planeIter->second)->m_tiles.end(); tileIter++)
		{
			// Write current tile key (x, z)
			std::pair<short, short> key = tileIter->first;
			bs.Write(key.first);
			bs.Write(key.second);

			// Write tile's cover data
			bs.Write( tileIter->second->GetCoverDataRaw() );

			// Write number of connections
			auto& connections = tileIter->second->GetAllConnections();
			unsigned int connectionsQuantity = connections.size();

			std::unordered_map<MAP_CONNECTION_DIR, MapTileConnection*>::iterator connectionIter;
			for (connectionIter = connections.begin(); connectionIter != connections.end(); connectionIter++)
			{
				// Write current connection's key (x, y, z)
				MapTile* connectedTile = connectionIter->second->GetConnected(tileIter->second);
				MapVec3 connectedPos = connectedTile->GetTilePos();
				bs.Write((char*)&connectedPos, sizeof(MapVec3));
				
				// Write current connection's weight
				bs.Write(connectionIter->second->GetWeight());
			}
		}
	}

	pPeerInterface->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, address, false);
}

/* Write & send data that has changed since last update. */
void TileMap::WriteTilemapDiff(RakNet::RakPeerInterface * pPeerInterface, RakNet::SystemAddress & address)
{
	// TODO: Need some way to track what has changed. maybe keep a copy
	// of the bitstream each time a packet is sent.
	// Or maybe pass in which data to send.
}
#endif

#ifndef NETWORK_SERVER
/* Read a packet as a new tilemap. Wipes all old data if present. */
void TileMap::ReadTilemapNew(RakNet::Packet * packet)
{
	ClearAllData();

	RakNet::BitStream bsIn(packet->data, packet->length, false);
	// Ignore the system message
	bsIn.IgnoreBytes(sizeof(RakNet::MessageID));

	// Declare a map to store connection keys temporarily. See further down where this info is read for more info
	std::unordered_map<const MapVec3, std::list<std::pair<MapVec3, float> > > m_connectionKeys;
	
	// Read number of planes
	unsigned int planesQuantity = 0;
	bsIn.Read(planesQuantity);

	// Iterate through planes
	for (int i = 0; i < planesQuantity; i++)
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
		for (int j = 0; j < tilesQuantity; j++)
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
			 * m_connectionKeys map uses the current tile's position as the key &
			 * the value is a list of all connected tile positions.
			 */
			for (int k = 0; k < connectionsQuantity; k++)
			{
				// Read connection's position
				MapVec3 connectionPos;
				bsIn.Read(connectionPos);

				float connectionWeight;
				bsIn.Read(connectionWeight);

				// Add this to the temp connection map
				std::pair<const MapVec3, float> connectionInfo = std::make_pair(connectionPos, connectionWeight);
				std::list< std::pair < MapVec3, float > >* ptrList;
				const MapVec3 mv(0);
				(m_connectionKeys[mv]);
				//ptrList->push_back(connectionInfo);

				//(m_connectionKeys[currPos]).push_back(connectionInfo);
			}
		}
	}

	// Now all planes & tiles have been created. We need to iterate through all 
	// temp connections and actually connect the tiles they correspond to

	// Iterate through each tile
	//std::map<MapVec3, std::list<std::pair<MapVec3, float> > >::iterator tileKeyIter;
	//for (tileKeyIter = m_connectionKeys.begin(); tileKeyIter != m_connectionKeys.end(); tileKeyIter++)
	//{
	//	MapVec3 currentTilePos = tileKeyIter->first;
	//	MapTile* currentTile = FindTile(currentTilePos);
	//	std::list<std::pair<MapVec3, float> > currentConnList = tileKeyIter->second;
	//	if (currentTile != nullptr)
	//	{
	//		// Iterate through all connections
	//		std::list<std::pair<MapVec3, float> >::iterator connKeyIter;
	//		for (connKeyIter = currentConnList.begin(); connKeyIter != currentConnList.end(); connKeyIter++)
	//		{
	//			MapTile* connectionTile = FindTile(connKeyIter->first);
	//			float cost = connKeyIter->second;
	//			if (connectionTile != nullptr)
	//			{
	//				// Check if the connection already exists
	//				bool alreadyConnected = currentTile->IsConnected(connectionTile);
	//				if (!alreadyConnected)
	//					MapTileConnection* connection = new MapTileConnection(currentTile, connectionTile, cost);
	//			}
	//			else
	//				printf("Something went wrong. Unable to find a connected tile which should have been created.");
	//		}
	//	}
	//	else
	//		printf("Something went wrong. Unable to find a tile which should have been created.");
	//}
}

/** 
 * Read a packet as a diff. Will only write over mapped data
 * that exists in the packet. Everything else will remain untouched.
 */
void TileMap::ReadTilemapDiff(RakNet::Packet * packet)
{
	RakNet::BitStream bsIn(packet->data, packet->length, false);
	bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
}
#endif
