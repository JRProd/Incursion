#include "MapGeneration.hpp"

#include <queue>

#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Utils/StringUtils.hpp"

#include "Game/Map/TileDefinition.hpp"
#include "Game/Map/Generation/DrunkenWorm.hpp"

MapGeneration::MapGeneration()
    : m_MapRng()
{
}

const IntVec2& MapGeneration::GetStartLocation() const
{
    return m_MapStart;
}

const IntVec2& MapGeneration::GetExitLocation() const
{
    return m_MapEnd;
}

void MapGeneration::SetSize( IntVec2 size )
{
    m_MapSize = size;
}

void MapGeneration::SetBoundaryTiles( TileType boundaryTileType )
{
    m_BoundaryTileType = boundaryTileType;
}

void MapGeneration::SetDefaultInteriorType( TileType defaultInteriorType )
{
    m_DefaultInteriorType = defaultInteriorType;
}

void MapGeneration::SetTileInaccesableType( TileType tileInaccesableType )
{
    m_TileFillInaccesableType = tileInaccesableType;
}

void MapGeneration::SetStartLocation( const IntVec2& startLocation, int fortSize )
{
    m_MapStart = startLocation;
    m_StartFortSize = fortSize;
}

void MapGeneration::SetEndLocation( const IntVec2& endLocation, int fortSize )
{
    m_MapEnd = endLocation;
    m_EndFortSize = fortSize;
}

void MapGeneration::AddWormOfSize( WormType wormType,
                                   TileType tile,
                                   int size, const
                                   IntVec2& position )
{
    CreateWorm( wormType, tile, size, position );
}

void MapGeneration::AddWormOfSizeRandomPosition( WormType wormType,
                                                 TileType tile,
                                                 int size )
{
    IntVec2 randomPosition = IntVec2( m_MapRng.IntInRange( 1, m_MapSize.x - 1 ),
                                      m_MapRng.IntInRange( 1, m_MapSize.y - 1 ) );
    CreateWorm( wormType, tile, size, randomPosition );
}

void MapGeneration::AddWormsOfSizeRandomPositions( WormType wormType,
                                                   TileType tile,
                                                   int numberOfWorms,
                                                   int size )
{
    for ( int wormNumber = 0; wormNumber < numberOfWorms; ++wormNumber )
    {
        AddWormOfSizeRandomPosition( wormType, tile, size );
    }
}

const std::vector<TileType> MapGeneration::GenerateMap()
{
    int numTiles = m_MapSize.x * m_MapSize.y;
    std::vector<TileType> generatedMap;
    // Size up the vector and initialize to default values

    bool didSucceed = false;
    do
    {
        generatedMap.clear();
        generatedMap.resize( numTiles, m_DefaultInteriorType );

        GenerateMapExteriorTiles( generatedMap );

        GenerateMapWorms( generatedMap );

        GenerateMapStart( generatedMap );
        GenerateMapEnd( generatedMap );

        didSucceed = FillMapAndCheckValid( generatedMap );

    } while ( !didSucceed );

    return generatedMap;
}

void MapGeneration::GenerateMapExteriorTiles( MapTileList& map )
{
    int numTiles = m_MapSize.x * m_MapSize.y;

    // Create Horizontal Walls
    for ( int tileIndex = 0; tileIndex < m_MapSize.x; ++tileIndex )
    {
        map[ tileIndex ] = m_BoundaryTileType;

        int topRowIndex = numTiles - tileIndex - 1;
        map[ topRowIndex ] = m_BoundaryTileType;
    }

    // Create Vertical Walls
    //  Do not need to place the corner again
    for ( int tileIndex = 1; tileIndex < m_MapSize.y - 1; ++tileIndex )
    {
        int leftRowIndex = tileIndex * m_MapSize.x;
        map[ leftRowIndex ] = m_BoundaryTileType;

        int rightRowIndex = leftRowIndex + m_MapSize.x - 1;
        map[ rightRowIndex ] = m_BoundaryTileType;
    }
}

void MapGeneration::GenerateMapWorms( MapTileList& map )
{
    for ( int tileTypes = 0; tileTypes < NUM_TILE_TYPES; ++tileTypes )
    {
        WormList& currentList = m_WormsByTileType[ tileTypes ];
        for ( int wormIndex = 0; wormIndex < currentList.size(); ++wormIndex )
        {
            RunWorm( map, currentList.at( wormIndex ) );
        }
    }
}

void MapGeneration::GenerateMapStart( MapTileList& map )
{
    GenerateMapFort( map,
                     m_MapStart,
                     m_BoundaryTileType,
                     TILE_FOUNDATION,
                     TILE_FOUNDATION,
                     m_StartFortSize );
}

void MapGeneration::GenerateMapEnd( MapTileList& map )
{
    GenerateMapFort( map,
                     m_MapEnd,
                     m_BoundaryTileType,
                     TILE_END,
                     TILE_FOUNDATION,
                     m_EndFortSize );
}

void MapGeneration::GenerateMapFort( MapTileList& map,
                                     const IntVec2& location,
                                     TileType wall,
                                     TileType center,
                                     TileType floor,
                                     int size )
{
    int halfSizeInt = size / 2;

    for ( int beginIndexX = location.x - halfSizeInt;
          beginIndexX <= location.x + halfSizeInt;
          ++beginIndexX )
    {
        for ( int beginIndexY = location.y - halfSizeInt;
              beginIndexY <= location.y + halfSizeInt;
              ++beginIndexY )
        {

            IntVec2 position = IntVec2( beginIndexX, beginIndexY );
            if ( IsValidPosition( position ) )
            {
                int tileIndex = beginIndexX + beginIndexY * m_MapSize.x;
                int zeroBasedX = beginIndexX - (location.x - halfSizeInt);
                int zeroBasedY = beginIndexY - (location.y - halfSizeInt);
                if ( (zeroBasedX == 0 && zeroBasedY <= halfSizeInt) ||
                    (zeroBasedX <= halfSizeInt && zeroBasedY == 0) )
                {
                    map[ tileIndex ] = wall;
                }
                else if ( (zeroBasedX == size - 1 && zeroBasedY >= halfSizeInt) ||
                    (zeroBasedX >= halfSizeInt && zeroBasedY == size - 1) )
                {
                    map[ tileIndex ] = wall;
                }
                else if ( zeroBasedX == halfSizeInt && zeroBasedY == halfSizeInt )
                {
                    map[ tileIndex ] = center;
                }
                else
                {
                    map[ tileIndex ] = floor;
                }
            }
        }
    }
}

void MapGeneration::CreateWorm( WormType wormType,
                                TileType tile,
                                int size, const
                                IntVec2& position )
{
    Worm* createdWorm = CreateNewWorm( wormType, tile, size, position );
    AddWormToList( createdWorm );
}

Worm* MapGeneration::CreateNewWorm( WormType wormType,
                                    TileType tile,
                                    int size,
                                    const IntVec2& position )
{
    switch ( wormType )
    {
        case WORM_DRUNKEN: return new DrunkenWorm( position, tile, size );
        default:
            ERROR_AND_DIE( Stringf( "Cannot create worm of type %i", wormType ) );
    }
}

void MapGeneration::AddWormToList( Worm* wormToAdd )
{
    m_WormsByTileType[ wormToAdd->GetTileType() ].push_back( wormToAdd );
}

void MapGeneration::RunWorm( MapTileList& map, Worm* wormToRun )
{
    for ( int wormSpawns = 0; wormSpawns < wormToRun->GetMaxLength(); ++wormSpawns )
    {
        IntVec2 nextWormPosition = wormToRun->NextPosition();
        if ( IsValidPosition( nextWormPosition ) )
        {
            wormToRun->SetCurrentPosition( nextWormPosition );
            int mapIndex = nextWormPosition.x + nextWormPosition.y * m_MapSize.x;
            map[ mapIndex ] = wormToRun->GetTileType();
        }
    }
}

bool MapGeneration::IsValidPosition( const IntVec2& position ) const
{
    if ( position.x < 1 ) { return false; }
    if ( position.x > m_MapSize.x - 2 ) { return false; }
    if ( position.y < 1 ) { return false; }
    if ( position.y > m_MapSize.y - 2 ) { return false; }

    return true;
}

bool MapGeneration::FillMapAndCheckValid( MapTileList& map )
{
    bool reachedEnd = false;
    std::queue<int> transverseMap;
    std::vector<bool> canTransverseMap;

    size_t mapSize = static_cast<size_t>(m_MapSize.x) * static_cast<size_t>(m_MapSize.y);
    canTransverseMap.resize( mapSize, false );

    int vectorIndex = m_MapStart.x + m_MapStart.y * m_MapSize.x;
    transverseMap.push( vectorIndex );
    canTransverseMap[ vectorIndex ] = true;

    while ( !transverseMap.empty() )
    {
        int currentSpot = transverseMap.front();
        transverseMap.pop();

        if ( map.at( currentSpot ) == TILE_END )
        {
            reachedEnd = true;
        }

        // Push right position
        int rightPosition = currentSpot + 1;
        if ( rightPosition >= 0 && rightPosition < map.size() )
        {
            if ( !canTransverseMap.at( rightPosition ) && !IsPositionSolid( map, rightPosition ) )
            {
                canTransverseMap[ rightPosition ] = true;
                transverseMap.push( rightPosition );
            }
        }

        int topPosition = currentSpot + m_MapSize.x;
        if ( topPosition >= 0 && topPosition < map.size() )
        {
        // Push top position
            if ( !canTransverseMap.at( topPosition ) && !IsPositionSolid( map, topPosition ) )
            {
                canTransverseMap[ topPosition ] = true;
                transverseMap.push( topPosition );
            }
        }

        int leftPosition = currentSpot - 1;
        if ( leftPosition >= 0 && leftPosition < map.size() )
        {
        // Push left position
            if ( !canTransverseMap.at( leftPosition ) && !IsPositionSolid( map, leftPosition ) )
            {
                canTransverseMap[ leftPosition ] = true;
                transverseMap.push( leftPosition );
            }
        }

        int bottomPositon = currentSpot - m_MapSize.x;
        if ( bottomPositon >= 0 && bottomPositon < map.size() )
        {
            //Push bottom position
            if ( !canTransverseMap.at( bottomPositon ) && !IsPositionSolid( map, bottomPositon ) )
            {
                canTransverseMap[ bottomPositon ] = true;
                transverseMap.push( bottomPositon );
            }
        }
    }

    for ( int tileIndex = 0; tileIndex < mapSize; ++tileIndex )
    {
        if ( !canTransverseMap.at( tileIndex ) && !IsPositionSolid( map, tileIndex ) )
        {
            map[ tileIndex ] = m_TileFillInaccesableType;
        }
    }

    return reachedEnd;
}

bool MapGeneration::IsPositionSolid( const MapTileList& map, int index ) const
{
    return TileDefinition::DEFINITIONS.at( map.at( index ) ).IsTileSolid();
}
