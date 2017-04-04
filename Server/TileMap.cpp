#include "TileMap.h"
#include "GameMessages.h"



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

	typedef std::unordered_map<std::pair<short, short>, MapTile*> xyTile;
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
		std::unordered_map<std::pair<short, short>, MapTile*>::iterator tileIter;
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
	bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
	// TODO
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
