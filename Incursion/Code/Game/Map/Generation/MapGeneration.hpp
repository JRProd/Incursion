#pragma once

#include <vector>

#include "Engine/Core/Math/RandomNumberGenerator.hpp"

#include "Game/Map/Tile.hpp"
#include "Game/Map/Generation/Worm.hpp"

typedef std::vector<TileType> MapTileList;

class MapGeneration
{
public:
    MapGeneration();

    const IntVec2& GetStartLocation() const;
    const IntVec2& GetExitLocation() const;

    void SetSize( IntVec2 size );
    void SetBoundaryTiles( TileType boundaryTileType );
    void SetDefaultInteriorType( TileType defaultInteriorType );
    void SetTileInaccesableType( TileType tileInaccesableType );
    void SetStartLocation( const IntVec2& startLocation, int fortSize );
    void SetEndLocation( const IntVec2& endLocation, int fortSize );

    void AddWormOfSize( WormType wormType,
                        TileType tile,
                        int size,
                        const IntVec2& position );

    void AddWormOfSizeRandomPosition( WormType wormType,
                                      TileType tile,
                                      int size );
    void AddWormsOfSizeRandomPositions( WormType wormType,
                                        TileType tile,
                                        int numberOfWorms, 
                                        int size );

    const std::vector<TileType> GenerateMap();

private:
    // Used in the spawning of random position worms
    RandomNumberGenerator m_MapRng;

    // Parameters used in the creation of the map
    IntVec2 m_MapSize = IntVec2::ONE;
    TileType m_BoundaryTileType = TILE_QUAD_WALL;
    TileType m_DefaultInteriorType = TILE_GRASS;
    TileType m_TileFillInaccesableType = TILE_QUAD_WALL;
    IntVec2 m_MapStart = IntVec2::ZERO;
    int m_StartFortSize = 1;
    IntVec2 m_MapEnd = IntVec2::ZERO;
    int m_EndFortSize = 1;

    WormList m_WormsByTileType[ NUM_TILE_TYPES ];

    //-------------------------------------------------------------------------
    // Map Generation Functions
    void GenerateMapExteriorTiles( MapTileList& map);
    void GenerateMapWorms( MapTileList& map);
    void GenerateMapStart( MapTileList& map );
    void GenerateMapEnd( MapTileList& map );
    void GenerateMapFort( MapTileList& map,
                          const IntVec2& location,
                          TileType wall, 
                          TileType center, 
                          TileType floor, 
                          int size = 5);

    void RunWorm( MapTileList& map, Worm* wormToRun );
    bool IsValidPosition( const IntVec2& position ) const;

    bool FillMapAndCheckValid( MapTileList& map );
    bool IsPositionSolid( const MapTileList& map, int index ) const;

    //-------------------------------------------------------------------------
    // Functions to create worms
    void CreateWorm( WormType wormType,
                     TileType tile, 
                     int size, const 
                     IntVec2& position );
    Worm* CreateNewWorm( WormType wormType,
                        TileType tile, 
                        int size, const 
                        IntVec2& position );
    void AddWormToList( Worm* wormToAdd );

};

