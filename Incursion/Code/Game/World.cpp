#include "World.hpp"

#include "Game/Game.hpp"
#include "Game/Map/Map.hpp"
#include "Game/Map/Generation/MapGeneration.hpp"
#include "Game/Entity/PlayerCharacter.hpp"

World::World( Game* gameInstance )
    : m_GameInstance( gameInstance )
{
}

void World::Create()
{
    GenerateMaps();

    m_PlayerCharacter = static_cast<PlayerCharacter*>(
        m_CurrentMap->SpawnNewEntityAtStart( ENTITY_PLAYER));
}

void World::Update( float deltaSeconds )
{
    m_CurrentMap->Update( deltaSeconds );
}

void World::Render() const
{
    m_CurrentMap->Render();
}

void World::DebugRender() const
{
    m_CurrentMap->DebugRender();
}

void World::Destory()
{
    for ( int mapIndex = 0; mapIndex < m_Maps.size(); ++mapIndex )
    {
        Map*& currentMap = m_Maps.at( mapIndex );
        if ( currentMap != nullptr )
        {
            currentMap->Destroy();
            delete currentMap;
            currentMap = nullptr;
        }
    }

    m_PlayerCharacter->Destroy();
    delete m_PlayerCharacter;
    m_PlayerCharacter = nullptr;

    m_Maps.clear();
}

const PlayerCharacter* World::GetPlayerCharacter() const
{
    return m_PlayerCharacter;
}

const IntVec2 World::GetCurrentMapSize() const
{
    return m_CurrentMap->GetMapSize();
}

void World::RequestRespawn( PlayerCharacter* player )
{
    m_CurrentMap->RequestRespawn( player );
}

void World::MapCompleted()
{
    m_CurrentMapIndex += 1;

    if ( m_CurrentMapIndex >= NUM_MAPS )
    {
        m_GameInstance->WorldComplete();
    }
    else
    {
        m_CurrentMap = m_Maps[ m_CurrentMapIndex ];
        m_PlayerCharacter->TeleportToNewMap( m_CurrentMap );
        m_CurrentMap->AddEntityToMapAtStart( m_PlayerCharacter );
    }

}

void World::GenerateMaps()
{
    static IntVec2 mapSizes[ NUM_MAPS ] = { IntVec2( 20,20 ), IntVec2( 40,20 ), IntVec2( 25,40 ) };
    static IntVec2 startLocations[ NUM_MAPS ] = { IntVec2( 2,2 ), IntVec2( 2,2 ), IntVec2( 10,2 ) };
    static IntVec2 endLocations[ NUM_MAPS ] = { IntVec2( 17,17 ), IntVec2( 37,17 ), IntVec2( 22,37 ) };
    static int tanksOnMaps[ NUM_MAPS ] = { 5, 8, 10 };
    static int turretsOnMaps[ NUM_MAPS ] = { 1, 5, 9 };
    static int boldersOnMaps[ NUM_MAPS ] = { 3, 5, 5 };
    static int fortSize = 5;
    MapGeneration mapGenerators[ NUM_MAPS ];

    // MapGenerator for map one
    MapGeneration& map1 = mapGenerators[ 0 ];
    map1.SetSize( mapSizes[ 0 ] );
    map1.SetBoundaryTiles( TILE_MOSS_STONE );
    map1.SetDefaultInteriorType( TILE_GRASS );
    map1.SetTileInaccesableType( TILE_MOSS_STONE );
    map1.SetStartLocation( startLocations[ 0 ], fortSize);
    map1.SetEndLocation( endLocations[ 0 ], fortSize);
    map1.AddWormsOfSizeRandomPositions( WORM_DRUNKEN, TILE_MOSS_STONE, 4, 13 );
    map1.AddWormsOfSizeRandomPositions( WORM_DRUNKEN, TILE_MOSS_STONE, 10, 8 );
    map1.AddWormsOfSizeRandomPositions( WORM_DRUNKEN, TILE_DIRT, 10, 8);
    map1.AddWormsOfSizeRandomPositions( WORM_DRUNKEN, TILE_MUD, 10, 5 );
    map1.AddWormsOfSizeRandomPositions( WORM_DRUNKEN, TILE_WATER, 2, 12 );
    map1.AddWormsOfSizeRandomPositions( WORM_DRUNKEN, TILE_WATER, 8, 4 );

    // MapGenerator for map two
    MapGeneration& map2 = mapGenerators[ 1 ];
    map2.SetSize( mapSizes[ 1 ] );
    map2.SetBoundaryTiles( TILE_QUAD_WALL );
    map2.SetDefaultInteriorType( TILE_SAND );
    map2.SetTileInaccesableType( TILE_QUAD_WALL );
    map2.SetStartLocation( startLocations[ 1 ], fortSize );
    map2.SetEndLocation( endLocations[ 1 ], fortSize );
    map2.AddWormsOfSizeRandomPositions( WORM_DRUNKEN, TILE_QUAD_WALL, 10, 15 );
    map2.AddWormsOfSizeRandomPositions( WORM_DRUNKEN, TILE_QUAD_WALL, 15, 8 );
    map2.AddWormsOfSizeRandomPositions( WORM_DRUNKEN, TILE_QUAD_WALL, 20, 3 );
    map2.AddWormsOfSizeRandomPositions( WORM_DRUNKEN, TILE_WATER, 2, 4 );

    // MapGenerator for map three
    MapGeneration& map3 = mapGenerators[ 2 ];
    map3.SetSize( mapSizes[ 2 ] );
    map3.SetBoundaryTiles( TILE_STEEL_BLOCK );
    map3.SetDefaultInteriorType( TILE_MUD );
    map3.SetTileInaccesableType( TILE_STEEL_BLOCK );
    map3.SetStartLocation( startLocations[ 2 ], fortSize );
    map3.SetEndLocation( endLocations[ 2 ], fortSize );
    map3.AddWormsOfSizeRandomPositions( WORM_DRUNKEN, TILE_STEEL_BLOCK, 8, 20 );
    map3.AddWormsOfSizeRandomPositions( WORM_DRUNKEN, TILE_STEEL_BLOCK, 15, 8 );
    map3.AddWormsOfSizeRandomPositions( WORM_DRUNKEN, TILE_STEEL_BLOCK, 20, 3 );
    map3.AddWormsOfSizeRandomPositions( WORM_DRUNKEN, TILE_GRASS, 50, 5 );

    for ( int mapIndex = 0; mapIndex < NUM_MAPS; ++mapIndex )
    {
        // Create and generate map
        Map* newMap = new Map( m_GameInstance, this, mapSizes[ mapIndex ] );
        newMap->Create();
        newMap->GenerateMap( mapGenerators[ mapIndex ] );

        // Spawn entities in the map
        newMap->SpawnNewEntitiesOfTypeInOpenSpace( tanksOnMaps[ mapIndex ], ENTITY_ENEMY_TANK );
        newMap->SpawnNewEntitiesOfTypeInOpenSpace( turretsOnMaps[ mapIndex ], ENTITY_ENEMY_TURRET );
        newMap->SpawnNewEntitiesOfTypeInOpenSpace( boldersOnMaps[ mapIndex ], ENTITY_BOLDER );

        m_Maps.push_back( newMap );
    }

    m_CurrentMap = m_Maps[ 0 ];
}
