#include "Map.hpp"

#include <algorithm>

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Utils/VectorPcuUtils.hpp"
#include "Engine/Core/STL/RapidReplaceVector.hpp"
#include "Engine/Core/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/Math/Primatives/AABB2.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/Sprite/SpriteSheet.hpp"
#include "Engine/Renderer/Sprite/SpriteDefinition.hpp"

#include "Game/Game.hpp"
#include "Game/World.hpp"
#include "Game/AssetManagers/TextureManager.hpp"
#include "Game/Entity/Entity.hpp"
#include "Game/Entity/PlayerCharacter.hpp"
#include "Game/Entity/Bolder.hpp"
#include "Game/Entity/Bullet.hpp"
#include "Game/Entity/TankNPC.hpp"
#include "Game/Entity/TurretNPC.hpp"
#include "Game/Entity/Explosion.hpp"
#include "Game/Map/Tile.hpp"
#include "Game/Map/TileDefinition.hpp"
#include "Game/Map/Generation/MapGeneration.hpp"

#include "Engine/Renderer/Mesh/MeshUtils.hpp"

Map::~Map()
{
}

Map::Map( Game* gameInstance, World* world, int sizeX, int sizeY )
    : m_GameInstance( gameInstance )
  , m_World( world )
  , m_Size( sizeX, sizeY )
  , m_NumTiles( sizeX * sizeY )
{
}

Map::Map( Game* gameInstance, World* world, const IntVec2& size )
    : m_GameInstance( gameInstance )
  , m_World( world )
  , m_Size( size )
  , m_NumTiles( m_Size.x * m_Size.y )
{
}

void Map::Create()
{
    m_Tiles.reserve( m_NumTiles );
}

//-----------------------------------------------------------------------------
void Map::GenerateMap( MapGeneration& generator )
{
    m_StartLocation = generator.GetStartLocation();
    m_ExitLocation = generator.GetExitLocation();

    std::vector<TileType> tiles = generator.GenerateMap();

    for( int tileIndex = 0; tileIndex < tiles.size(); ++tileIndex )
    {
        IntVec2 tilePos = IntVec2( tileIndex % m_Size.x, tileIndex / m_Size.x );
        m_Tiles.push_back( Tile( tilePos, tiles.at( tileIndex ) ) );
    }
}

void Map::Update( float deltaSeconds )
{
    UpdateEntities( deltaSeconds );

    UpdateFogOfWar( ENTITY_PLAYER, 8, 8, CLIENT_ASPECT );

    HandleMapCollisions();

    DeleteGarbageEntities();
}

void Map::Render() const
{
    RenderTiles();
    RenderEntities();
}

void Map::DebugRender() const
{
    DebugRenderEntities();

    std::vector<VertexMaster> collisionVisual;
    for ( int tileIndex = 0; tileIndex < m_Tiles.size(); ++tileIndex )
    {
        const Tile& currentTile = m_Tiles.at( tileIndex );
        if ( currentTile.IsSolid() )
        {
            AppendAABB2( collisionVisual, currentTile.GetTileBoundingBox(), Rgba8::CYAN );
        }
    }

    g_Renderer->BindTexture( nullptr );
    g_Renderer->DrawVertexArray( collisionVisual );
}

void Map::Destroy()
{
    DestroyEntities();
}

const IntVec2 Map::GetMapSize() const
{
    return m_Size;
}

bool Map::IsPointInSolid( const Vec2& point ) const
{
    bool isInSolidTile = IsTileSolid( *GetTileFromPosition( static_cast<IntVec2>(point) ) );
    return isInSolidTile;
}

bool Map::IsPointBlockRaycast( const Vec2& point ) const
{
    IntVec2 tilePos = GetTilePositionFromWorldCoords( point );
    if( IsValidTilePos( tilePos ) )
    {
        bool isInBlockRaycast = DoseTileBlockRaycast( *GetTileFromPosition( tilePos ) );
        return isInBlockRaycast;
    }
    else
    {
        return true;
    }
}

Entity* Map::SpawnNewEntityAtStart( EntityType type )
{
    return SpawnNewEntity( type, static_cast<Vec2>(m_StartLocation) );
}

Entity* Map::SpawnNewEntity( EntityType type, const Vec2& spawnPosition )
{
    Entity* spawnedEntity = SpawnEntityOfType( type, spawnPosition );
    spawnedEntity->Create();
    AddEntityToMap( type, spawnedEntity );
    return spawnedEntity;
}

Explosion* Map::SpawnNewExplosion( const Vec3& position, float duration, const Vec3& scale )
{
    Explosion* spanwedEntity = new Explosion( m_GameInstance,
                                              this,
                                              position,
                                              duration,
                                              scale
                                            );
    spanwedEntity->Create();
    AddEntityToMap( ENTITY_EXPLOSION, spanwedEntity );
    return spanwedEntity;
}

void Map::SpawnNewEntitiesOfTypeInOpenSpace( int number, EntityType type )
{
    for( int entityIndex = 0; entityIndex < number; ++entityIndex )
    {
        Vec2 spawnPosition = FindPointNotInWall( AABB2::UNIT_BOX.GetDimensions() );
        SpawnNewEntity( type, spawnPosition );
    }
}

void Map::RequestRespawn( PlayerCharacter* entityToRespawn )
{
    entityToRespawn->SetPosition( static_cast<Vec3>(static_cast<Vec2>(m_StartLocation)) );
    entityToRespawn->RespawnResetStats();
}

void Map::AddEntityToMapAtStart( Entity* entity )
{
    AddEntityToMap( entity, static_cast<Vec2>(m_StartLocation) );
}

void Map::AddEntityToMap( Entity* entity, const Vec2& spawnPosition )
{
    entity->SetPosition( static_cast<Vec3>(spawnPosition) );
    m_EntityListsByType[ entity->GetEntityType() ].RapidReplace( entity );
}

bool Map::IsValidTilePos( const IntVec2& testPos ) const
{
    if( testPos.x < 0 || testPos.y < 0 ) { return false; }
    if( testPos.x > m_Size.x - 1 || testPos.y > m_Size.y - 1 ) { return false; }
    return true;
}

const IntVec2 Map::GetTilePositionFromWorldCoords( const Vec2& worldCoords ) const
{
    return IntVec2( (int)worldCoords.x, (int)worldCoords.y );
}

const IntVec2 Map::GetTilePositionFromIndex( int index ) const
{
    return m_Tiles.at( index ).GetTilePosition();
}

int Map::GetTileIndexFromPosition( const IntVec2& position ) const
{
    return (position.y * m_Size.x) + position.x;
}

bool Map::IsTileSolid( const Tile& tile ) const
{
    return tile.IsSolid();
}

bool Map::DoseTileBlockRaycast( const Tile& tile ) const
{
    return tile.DoesBlockRaycast();
}

void Map::SetTypeOfTile( const IntVec2& positions, TileType tileType )
{
    int tileIndex = GetTileIndexFromPosition( positions );
    m_Tiles.at( tileIndex ).SetTileType( tileType );
}

void Map::SetTilePositionVisable( const IntVec2& position )
{
    if( IsValidTilePos( position ) )
    {
        Tile& tileToSet = m_Tiles.at( GetTileIndexFromPosition( position ) );
        tileToSet.SetTileIsSeen( false );
        tileToSet.SetTileCurrentSeen( true );
    }
}

const Tile* Map::GetTileFromIndex( int index ) const
{
    return &m_Tiles.at( index );
}

const Tile* Map::GetTileFromPosition( const IntVec2& position ) const
{
    int index = GetTileIndexFromPosition( position );
    if( index < 0 || index >= m_Tiles.size() ) { return nullptr; }

    return &m_Tiles.at( index );
}

PlayerCharacter* Map::GetPlayerEntity() const
{
    return static_cast<PlayerCharacter*>(m_EntityListsByType[ ENTITY_PLAYER ].data.at( 0 ));
}

RayCastHit Map::RayCastSolid( const Vec2& start, float angleDegrees, float maxDist ) const
{
    return RayCastSolid( start, Vec2::MakeFromPolarDegrees( angleDegrees ), maxDist );
}

RayCastHit Map::RayCastSolid( const Vec2& start, const Vec2& direciton, float maxDist ) const
{
    constexpr float RAYCAST_STEP = 1.f / 50.f;

    Vec2 position = start;
    Vec2 previous = Vec2::ZERO;

    bool wasObjectHit = true;
    while( !IsPointInSolid( position ) )
    {
        previous = position;
        position += direciton * RAYCAST_STEP;

        if( (position - start).GetLength() >= maxDist )
        {
            wasObjectHit = false;
            break;
        }
    }

    if( position == start )
    {
        return RayCastHit( true, start, 0.f );
    }
    else
    {
        Vec2 avg( (previous.x + position.x) * .5f, (previous.y + position.y) * .5f );
        Vec2 finishDisp = direciton * maxDist;
        Vec2 reachedDisplace = avg - start;
        float finishPercent = ClampZeroToOne( reachedDisplace.GetLength() / finishDisp.GetLength() );
        return RayCastHit( wasObjectHit, avg, finishPercent );
    }
}

RayCastHit Map::RayCastVisual( const Vec2& start, float angleDegrees, float maxDist ) const
{
    return RayCastVisual( start, Vec2::MakeFromPolarDegrees( angleDegrees ), maxDist );
}

RayCastHit Map::RayCastVisual( const Vec2& start, const Vec2& direciton, float maxDist ) const
{
    constexpr float RAYCAST_STEP = 1.f / 50.f;

    Vec2 position = start;

    bool wasObjectHit = true;
    while( !IsPointBlockRaycast( position ) )
    {
        position += direciton * RAYCAST_STEP;

        float distance = (position - start).GetLength();
        if( distance > maxDist || distance == 0 )
        {
            wasObjectHit = false;
            break;
        }
    }

    if( position == start )
    {
        return RayCastHit( true, start, 0.f );
    }
    else
    {
        Vec2 finishDisp = direciton * maxDist;
        Vec2 reachedDisplace = position - start;
        float finishPercent = ClampZeroToOne( reachedDisplace.GetLength() / finishDisp.GetLength() );
        return RayCastHit( wasObjectHit, position, finishPercent );
    }
}

bool Map::HasLineOfSight( const Entity& entity1, const Entity& entity2, float maxDist ) const
{
    Vec3 entityDisplacement = entity2.GetPosition() - entity1.GetPosition();
    if( entityDisplacement.GetLengthSquared() > (maxDist * maxDist) )
    {
        return false;
    }

    RayCastHit ray = RayCastVisual( static_cast<Vec2>(entity1.GetPosition()),
                                    entityDisplacement.GetAngleAboutZDegrees(),
                                    entityDisplacement.GetLength()
                                  );
    return !ray.didHit;
}

void Map::UpdateEntities( float deltaSeconds )
{
    for( int entityListIndex = 0; entityListIndex < NUM_ENTITY_TYPES; ++entityListIndex )
    {
        EntityList& currentEntityList = m_EntityListsByType[ entityListIndex ];

        for( int entityIndex = 0; entityIndex < currentEntityList.data.size(); ++entityIndex )
        {
            Entity*& currentEntity = currentEntityList.data.at( entityIndex );
            if( currentEntity != nullptr )
            {
                currentEntity->Update( deltaSeconds );
            }
        }
    }
}

void Map::UpdateFogOfWar( EntityType revealForEntityType, int fieldOfView, int viewHeight, float viewAspect )
{
    // The out of the field of view, where tiles start to not be seen
    int outerX = ceili( viewHeight * viewAspect );
    int outerY = viewHeight;

    int tileVectorSize = 2 * fieldOfView * fieldOfView + 2 * fieldOfView;
    const EntityList& fogOfWarList = m_EntityListsByType[ revealForEntityType ];

    // For each entity in the list
    for( int playerIndex = 0; playerIndex < fogOfWarList.data.size(); ++playerIndex )
    {
        const Entity* const& currentPlayerEntity = fogOfWarList.data.at( playerIndex );
        if( currentPlayerEntity == nullptr ) { continue; }
        if( currentPlayerEntity->IsDead() || currentPlayerEntity->IsGarbage() ) { return; }

        // Get the tile position of entity
        IntVec2 tilePosOfEntity = GetTilePositionFromWorldCoords(
                                                                 static_cast<Vec2>(currentPlayerEntity->GetPosition())
                                                                );

        // Create vector of tiles in field of view
        std::vector<Tile*> potentialTiles;
        potentialTiles.reserve( tileVectorSize );

        // For all the tiles in a outerFieldOfView * 2 + 1 box around the entity
        for( int testPosX = tilePosOfEntity.x - outerX;
             testPosX <= tilePosOfEntity.x + outerX;
             ++testPosX )
        {
            for( int testPosY = tilePosOfEntity.y - outerY;
                 testPosY <= tilePosOfEntity.y + outerY;
                 ++testPosY )
            {
                // Get the potential tile position
                IntVec2 testPos = IntVec2( testPosX, testPosY );
                // Stop if the point inst valid
                if( !IsValidTilePos( testPos ) ) { continue; }

                Tile* tileToTest = &m_Tiles.at( GetTileIndexFromPosition( testPos ) );
                // Specific test to check if tile was slated to be added by a entity
                if( !tileToTest->IsSeen() && tileToTest->IsTileCurrentSeen() )
                {
                    tileToTest->SetTileCurrentSeen( true );
                    potentialTiles.push_back( tileToTest );
                    continue;
                }
                else
                {
                    tileToTest->SetTileCurrentSeen( false );
                }

                if( tilePosOfEntity == testPos )
                {
                    tileToTest->SetTileCurrentSeen( true );
                    tileToTest->SetTileIsSeen( true );
                    continue;
                }

                // If the tile can be tested for visible, add it to the list
                if( IntVec2::GetTaxicabDistance( tilePosOfEntity, testPos ) <= fieldOfView )
                {
                    potentialTiles.push_back( tileToTest );
                }
            }
        }
        UpdateFogOfWarOnTilesFrom( currentPlayerEntity, potentialTiles );
    }
}

void Map::UpdateFogOfWarOnTilesFrom( const Entity* const& entity, std::vector<Tile*>& tiles )
{
    for( int tileIndex = 0; tileIndex < tiles.size(); ++tileIndex )
    {
        Tile* currentTile = tiles.at( tileIndex );

        // Get the direction to the tile
        Vec2 tileCenter = currentTile->GetTileBoundingBox().GetCenter();
        Vec2 raycastDisplacement = tileCenter - static_cast<Vec2>(entity->GetPosition());

        RayCastHit tileRaycastHit = RayCastVisual( static_cast<Vec2>(entity->GetPosition()),
                                                   raycastDisplacement.GetNormalized(),
                                                   raycastDisplacement.GetLength()
                                                 );

        IntVec2 tilePosOfHit = GetTilePositionFromWorldCoords( tileRaycastHit.hitPosition );
        if( IsValidTilePos( tilePosOfHit ) )
        {
            Tile& hitTile = m_Tiles[ GetTileIndexFromPosition( tilePosOfHit ) ];

            hitTile.SetTileIsSeen( true );
            hitTile.SetTileCurrentSeen( true );
        }
    }
}

void Map::RenderTiles() const
{
    std::vector<VertexMaster> tileVector;
    for( int tileIndex = 0; tileIndex < m_Tiles.size(); ++tileIndex )
    {
        const Tile& currentTile = m_Tiles.at( tileIndex );
        AABB2 tileBounds = AABB2::MakeFromUnitBoxAround( static_cast<Vec2>(currentTile.GetTilePosition()) );
        if( currentTile.GetTileType() != TILE_INVALID )
        {
            // If fog is off, render tiles
            if( g_NoFog )
            {
                AppendTileToVectorPCU( tileVector, currentTile, false );
            }
                // If tile is seen
            else if( currentTile.IsSeen() )
            {
                AppendTileToVectorPCU( tileVector, currentTile );
            }
            else if( !currentTile.IsSeen() )
            {
                AppendTileDefToVectorPCU( tileVector,
                                          tileBounds,
                                          TileDefinition::DEFINITIONS[ TILE_FOG ]
                                        );
            }
            else
            {
                AppendAABB2( tileVector, tileBounds, Rgba8::MAGENTA );
            }
        }
        else
        {
            AppendAABB2( tileVector, tileBounds, Rgba8::MAGENTA );
        }
    }

    g_Renderer->BindTexture( &m_Tiles.at( 0 ).GetTileDefinition().GetSpriteSheet()->GetTexture() );
    g_Renderer->DrawVertexArray( tileVector );
}

void Map::AppendTileToVectorPCU( std::vector<VertexMaster>& vector,
                                 const Tile& currentTile,
                                 bool shadeUnseenTiles ) const
{
    TileDefinition tileDef = currentTile.GetTileDefinition();
    Vec2 minUV = Vec2::ZERO;
    Vec2 maxUV = Vec2::ZERO;
    tileDef.GetSpriteSheet()->GetSpriteUVs( tileDef.GetTileSpriteIndex(), minUV, maxUV );

    Rgba8 tileColor = tileDef.GetTileTint();
    if( shadeUnseenTiles )
    {
        float notCurrentSeenMod = .6f;
        if( currentTile.IsTileCurrentSeen() )
        {
            notCurrentSeenMod = 1.f;
        }
        tileColor = tileColor.GetScaledColor( notCurrentSeenMod );
    }
    AppendAABB2( vector,
                 currentTile.GetTileBoundingBox(),
                 tileColor,
                 minUV,
                 maxUV
               );
}

void Map::AppendTileDefToVectorPCU( std::vector<VertexMaster>& vector,
                                    const AABB2& bounds,
                                    const TileDefinition& tileDef ) const
{
    Vec2 minUV = Vec2::ZERO;
    Vec2 maxUV = Vec2::ZERO;
    tileDef.GetSpriteSheet()->GetSpriteUVs( tileDef.GetTileSpriteIndex(), minUV, maxUV );

    AppendAABB2( vector,
                 bounds,
                 tileDef.GetTileTint(),
                 minUV,
                 maxUV
               );
}

void Map::RenderEntities() const
{
    for( int entityListIndex = 0; entityListIndex < NUM_ENTITY_TYPES; ++entityListIndex )
    {
        const EntityList& currentEntityList = m_EntityListsByType[ entityListIndex ];

        for( int entityIndex = 0; entityIndex < currentEntityList.data.size(); ++entityIndex )
        {
            const Entity* const& currentEntity = currentEntityList.data.at( entityIndex );
            if( currentEntity != nullptr )
            {
                // Check if entity is in seen tile
                IntVec2 tilePos = GetTilePositionFromWorldCoords( static_cast<const Vec2>(currentEntity->GetPosition())
                                                                );
                if( IsValidTilePos( tilePos ) )
                {
                    const Tile& tile = m_Tiles.at( GetTileIndexFromPosition( tilePos ) );
                    if( g_NoFog || tile.IsSeen() && tile.IsTileCurrentSeen() )
                    {
                        currentEntity->Render();
                    }
                }
            }
        }
    }
}

void Map::DebugRenderEntities() const
{
    for( int entityListIndex = 0; entityListIndex < NUM_ENTITY_TYPES; ++entityListIndex )
    {
        // Render Entity Player last
        if( entityListIndex == ENTITY_PLAYER ) { continue; }
        const EntityList& currentEntityList = m_EntityListsByType[ entityListIndex ];

        for( int entityIndex = 0; entityIndex < currentEntityList.data.size(); ++entityIndex )
        {
            const Entity* const& currentEntity = currentEntityList.data.at( entityIndex );
            if( currentEntity != nullptr )
            {
                currentEntity->DebugRender();
            }
        }
    }

    const EntityList& currentEntityList = m_EntityListsByType[ ENTITY_PLAYER ];

    for( int entityIndex = 0; entityIndex < currentEntityList.data.size(); ++entityIndex )
    {
        const Entity* const& currentEntity = currentEntityList.data.at( entityIndex );
        if( currentEntity != nullptr )
        {
            currentEntity->DebugRender();
        }
    }
}

Entity* Map::SpawnEntityOfType( EntityType type, const Vec2& spawnPosition )
{
    Vec3 spawnLoc = static_cast<Vec3>(spawnPosition);
    switch( type )
    {
        case ENTITY_INVALID: ERROR_AND_DIE( "Attempting to spawn entity of ENTITY_INVALID" );

        case ENTITY_PLAYER: return new PlayerCharacter( m_GameInstance, this, spawnLoc );

        case ENTITY_ALLIED_TANK: return new TankNPC( m_GameInstance, this, spawnLoc, type, FACTION_PLAYER );
        case ENTITY_ENEMY_TANK: return new TankNPC( m_GameInstance, this, spawnLoc, type, FACTION_ENEMY );

        case ENTITY_ALLIED_TURRET: return new TurretNPC( m_GameInstance, this, spawnLoc, type, FACTION_PLAYER );
        case ENTITY_ENEMY_TURRET: return new TurretNPC( m_GameInstance, this, spawnLoc, type, FACTION_ENEMY );

        case ENTITY_BULLET_ALLIED: return new Bullet( m_GameInstance, this, spawnLoc, type, FACTION_PLAYER );
        case ENTITY_BULLET_ENEMY: return new Bullet( m_GameInstance, this, spawnLoc, type, FACTION_ENEMY );

        case ENTITY_BOLDER: return new Bolder( m_GameInstance, this, spawnLoc );

        case ENTITY_EXPLOSION: ERROR_AND_DIE( "Spawn Explosion from specific explosion function" );

        case NUM_ENTITY_TYPES: ERROR_AND_DIE( "Attempting to spawn entity of NUM_ENTITY_TYPES" );
        default: ERROR_AND_DIE( Stringf( "Attempting to spawn entity of %i", type ) );
    }
}

void Map::AddEntityToMap( EntityType type, Entity* entity )
{
    m_EntityListsByType[ type ].RapidReplace( entity );
}

void Map::HandleMapCollisions()
{
    for( int entityListIndex1 = 0; entityListIndex1 < NUM_ENTITY_TYPES; ++entityListIndex1 )
    {
        for( int entityListIndex2 = entityListIndex1; entityListIndex2 < NUM_ENTITY_TYPES; ++entityListIndex2 )
        {
            HandleListVsListOverlaps( entityListIndex1,
                                      entityListIndex2
                                    );
        }

        HandleListVsTileOverlaps( entityListIndex1 );
    }

    Entity* playerCharacter = (Entity*)m_World->GetPlayerCharacter();
    Disc playerDisc = playerCharacter->GetEntityPhysicsDisc();
    Vec2 exitCenter = GetTileFromPosition( m_ExitLocation )->GetTileBoundingBox().GetCenter();

    if( playerDisc.IsPointInside( exitCenter ) )
    {
        m_World->MapCompleted();
    }
}

void Map::HandleListVsListOverlaps( EntityListIndex l1, EntityListIndex l2 )
{
    if( !Entity::DoEntityListsOverlap( l1, l2 ) ) { return; }

    EntityList& list1 = m_EntityListsByType[ l1 ];
    EntityList& list2 = m_EntityListsByType[ l2 ];

    for( int entityIndex1 = 0; entityIndex1 < list1.data.size(); ++entityIndex1 )
    {
        for( int entityIndex2 = 0; entityIndex2 < list2.data.size(); ++entityIndex2 )
        {
            HandleEntityVsEntityOverlaps( list1.data.at( entityIndex1 ),
                                          list2.data.at( entityIndex2 )
                                        );
        }
    }
}

void Map::HandleEntityVsEntityOverlaps( Entity*& entity1, Entity*& entity2 )
{
    if( entity1 == nullptr || entity2 == nullptr ) { return; }
    if( entity1->IsDead() || entity1->IsGarbage() ) { return; }
    if( entity2->IsDead() || entity2->IsGarbage() ) { return; }

    // Check non colliding entities
    switch( Entity::OverlapsWith( entity1, entity2 ) )
    {
        case EntityOverlapType::NONE: return;
        case EntityOverlapType::OVERLAP_ONLY: HandleOverlapOnly( entity1, entity2 );
            break;
        case EntityOverlapType::PUSH_PUSH: HandlePushedVsPushed( entity1, entity2 );
            break;
        case EntityOverlapType::PUSH_FIXED: DeterminePushedVsFixedEntity( entity1, entity2 );
            break;
        case EntityOverlapType::PUSH_NO_PUSH: break;
        case EntityOverlapType::FIXED_NO_PUSH: break;
        default:
            ERROR_AND_DIE( "Collision type not handled" );
            break;
    }
}

void Map::HandleOverlapOnly( Entity*& entity1, Entity*& entity2 )
{
    Entity* entity = nullptr;
    Bullet* bulletEntity = nullptr;

    if( entity1->IsHitByBullets() )
    {
        entity = entity1;
        bulletEntity = static_cast<Bullet*>(entity2);
    }
    else if( entity2->IsHitByBullets() )
    {
        entity = entity2;
        bulletEntity = static_cast<Bullet*>(entity1);
    }
    else
    {
        // Neither entity is hit by bullets
        return;
    }

    HandleEntityVsBulletCollision( entity, bulletEntity );
}

void Map::HandlePushedVsPushed( Entity*& entity1, Entity*& entity2 )
{
    // Dead entities don't collide
    if( entity1->IsDead() || entity2->IsDead() ) { return; }
    if( DoDiscsOverlap( entity1->GetEntityPhysicsDisc(), entity2->GetEntityPhysicsDisc() ) )
    {
        Disc entityDisc1 = entity1->GetEntityPhysicsDisc();
        Disc entityDisc2 = entity2->GetEntityPhysicsDisc();

        Disc::PushDiscMobileOutOfDiscMobile( entityDisc1, entityDisc2 );
        entity2->SetPosition( static_cast<Vec3>(entityDisc2.center) );
        entity1->SetPosition( static_cast<Vec3>(entityDisc1.center) );
    }
}

void Map::DeterminePushedVsFixedEntity( Entity*& entity1, Entity*& entity2 )
{
    Entity* pushed = nullptr;
    Entity* fixed = nullptr;

    if( entity1->IsFixed() && entity2->IsFixed() ) { return; }
    if( entity1->IsFixed() && !entity2->IsFixed() )
    {
        fixed = entity1;
        pushed = entity2;
    }
    else if( !entity1->IsFixed() && entity2->IsFixed() )
    {
        fixed = entity2;
        pushed = entity1;
    }

    HandlePushedVsFixed( pushed, fixed );
}

void Map::HandlePushedVsFixed( Entity*& pushed, const Entity* fixed )
{
    // Dead entities don't collide
    if( pushed->IsDead() || fixed->IsDead() ) { return; }
    if( DoDiscsOverlap( pushed->GetEntityPhysicsDisc(), fixed->GetEntityPhysicsDisc() ) )
    {
        Disc pushedDisc = pushed->GetEntityPhysicsDisc();
        const Disc fixedDisc = fixed->GetEntityPhysicsDisc();

        Disc::PushDiscMobileOutOfDiscFixed( pushedDisc, fixedDisc );
        pushed->SetPosition( static_cast<Vec3>(pushedDisc.center) );
    }
}

void Map::HandleEntityVsBulletCollision( Entity*& entity, Bullet*& bullet )
{
    // Bullets do not collide with the same faction entities
    if( entity->GetEntityFaction() == bullet->GetEntityFaction() ) { return; }

    const Disc& entityDisc = entity->GetEntityPhysicsDisc();
    const Disc& buttleDisc = bullet->GetEntityPhysicsDisc();

    if( DoDiscsOverlap( entityDisc, buttleDisc ) )
    {
        if( entity->GetEntityType() == ENTITY_BOLDER )
        {
            Vec3 reflectedVelocity = bullet->GetVelocity();
            Vec3 hitNormal = entity->GetPhysicsDiscNormalAt( bullet->GetPosition() );

            if( Vec3::Dot( reflectedVelocity, hitNormal ) > 0 )
            {
                reflectedVelocity.ReflectAcrossNormal( hitNormal );
                bullet->SetVelocity( reflectedVelocity );
            }
        }
        else
        {
            entity->DamageEntity( 1 );
            bullet->Die();
        }
    }
}

void Map::HandleListVsTileOverlaps( EntityListIndex l1 )
{
    EntityList& list1 = m_EntityListsByType[ l1 ];

    // Entity Interactions with Tiles
    for( int entityIndex = 0; entityIndex < list1.data.size(); ++entityIndex )
    {
        Entity*& currentEntity = list1.data.at( entityIndex );
        if( currentEntity == nullptr ) { continue; }
        if( currentEntity->IsDead() || currentEntity->IsGarbage() ) { continue; }

        if( Entity::OverlapsWithTiles( currentEntity ) )
        {
            if( currentEntity->GetEntityType() == ENTITY_PLAYER && g_NoClip ) { continue; }

            static IntVec2 bottomCenter = IntVec2( 0, -1 );
            static IntVec2 rightCenter = IntVec2( 1, 0 );
            static IntVec2 topCenter = IntVec2( 0, 1 );
            static IntVec2 leftCenter = IntVec2( -1, 0 );
            static IntVec2 bottomLeft = IntVec2( -1, -1 );
            static IntVec2 bottomRight = IntVec2( -1, 1 );
            static IntVec2 topRight = IntVec2( 1, 1 );
            static IntVec2 topLeft = IntVec2( 1, -1 );
            IntVec2 entityTilePosition = GetTilePositionFromWorldCoords(
                                                                        static_cast<Vec2>(currentEntity->GetPosition())
                                                                       );

            HandleEntityVsTileOverlap( currentEntity, GetTileFromPosition( entityTilePosition ) );
            HandleEntityVsTileOverlap( currentEntity, GetTileFromPosition( entityTilePosition + bottomCenter ) );
            HandleEntityVsTileOverlap( currentEntity, GetTileFromPosition( entityTilePosition + rightCenter ) );
            HandleEntityVsTileOverlap( currentEntity, GetTileFromPosition( entityTilePosition + topCenter ) );
            HandleEntityVsTileOverlap( currentEntity, GetTileFromPosition( entityTilePosition + leftCenter ) );
            HandleEntityVsTileOverlap( currentEntity, GetTileFromPosition( entityTilePosition + bottomLeft ) );
            HandleEntityVsTileOverlap( currentEntity, GetTileFromPosition( entityTilePosition + bottomRight ) );
            HandleEntityVsTileOverlap( currentEntity, GetTileFromPosition( entityTilePosition + topRight ) );
            HandleEntityVsTileOverlap( currentEntity, GetTileFromPosition( entityTilePosition + topLeft ) );
        }
    }
}

void Map::HandleEntityVsTileOverlap( Entity* entity, const Tile* tile )
{
    if( tile == nullptr ) { return; }

    if( tile->IsSolid() )
    {
        HandleEntityVsTileCollision( entity, tile );
    }
    else if( !tile->IsSolid() )
    {
        HandleEntityVsTileOverlapOnly( entity, tile );
    }
}

void Map::HandleEntityVsTileOverlapOnly( Entity* entity, const Tile* tile )
{
    if( tile->GetTileType() != TILE_MUD ) { return; }

    // Handle Entity vs Mud Overlap
    Disc entityPhysicsDisc = entity->GetEntityPhysicsDisc();
    if( DoAABB2OverlapDisc( tile->GetTileBoundingBox(), entityPhysicsDisc ) )
    {
        if( entity->GetEntityType() == ENTITY_PLAYER ||
            entity->GetEntityType() == ENTITY_ALLIED_TANK ||
            entity->GetEntityType() == ENTITY_ENEMY_TANK )
        {
            entity->SetVelocityModifier( TileDefinition::DEFINITIONS[ TILE_MUD ].GetVelocityModifier() );
        }
    }
}

void Map::HandleEntityVsTileCollision( Entity* entity, const Tile* tile )
{
    // Handle Entity vs Wall collision
    Disc entityPhysicsDisc = entity->GetEntityPhysicsDisc();
    if( DoAABB2OverlapDisc( tile->GetTileBoundingBox(), entityPhysicsDisc ) )
    {
        if( entity->IsPushedByWalls() )
        {
            entityPhysicsDisc.PushOutOfAABB2Fixed( tile->GetTileBoundingBox() );
            entity->SetPosition( static_cast<Vec3>(entityPhysicsDisc.center) );
        }
        else if( (entity->GetEntityType() == ENTITY_BULLET_ALLIED ||
                entity->GetEntityType() == ENTITY_BULLET_ENEMY) &&
            tile->DoesBlockProjectiles() )
        {
            entity->Die();
        }
    }
}

Vec2 Map::FindPointNotInWall( const Vec2& bounds )
{
    Vec2 pointNotInWall = Vec2::ZERO;
    bool isInWall = true;
    do
    {
        pointNotInWall.x = g_GameInstance->GetRng()->FloatInRange( 6.f, static_cast<float>(m_Size.x - 6.f) );
        pointNotInWall.y = g_GameInstance->GetRng()->FloatInRange( 6.f, static_cast<float>(m_Size.y - 6.f) );

        AABB2 boundingBox = AABB2::UNIT_AROUND_ZERO;
        boundingBox *= bounds;
        boundingBox.TranslateCenter( pointNotInWall );

        bool foundWallOverlap = false;
        for( int tileIndex = 0; tileIndex < m_Tiles.size(); ++tileIndex )
        {
            Tile& currentTile = m_Tiles.at( tileIndex );
            if( currentTile.IsSolid() )
            {
                if( DoAABB2sOverlap( boundingBox, currentTile.GetTileBoundingBox() ) )
                {
                    foundWallOverlap = true;
                    break;
                }
            }
        }

        isInWall = foundWallOverlap;
    }
    while( isInWall );

    return pointNotInWall;
}

void Map::DeleteGarbageEntities()
{
    for( int entityListIndex = 0; entityListIndex < NUM_ENTITY_TYPES; ++entityListIndex )
    {
        EntityList& currentEntityList = m_EntityListsByType[ entityListIndex ];

        for( int entityIndex = 0; entityIndex < currentEntityList.data.size(); ++entityIndex )
        {
            Entity*& currentEntity = currentEntityList.data.at( entityIndex );
            if( currentEntity != nullptr && currentEntity->IsGarbage() )
            {
                currentEntity->Destroy();
                currentEntityList.RapidRemoval( entityIndex );
            }
        }
    }
}

void Map::DestroyEntities()
{
    for( int entityListIndex = 0; entityListIndex < NUM_ENTITY_TYPES; ++entityListIndex )
    {
        if( entityListIndex == ENTITY_PLAYER ) { continue; }
        EntityList& currentEntityList = m_EntityListsByType[ entityListIndex ];

        for( int entityIndex = 0; entityIndex < currentEntityList.data.size(); ++entityIndex )
        {
            Entity*& currentEntity = currentEntityList.data.at( entityIndex );
            DestroyEntity( currentEntity );
        }

        currentEntityList.data.clear();
    }
}

void Map::DestroyEntity( Entity*& entityToDestroy )
{
    if( entityToDestroy == nullptr ) { return; }

    entityToDestroy->Destroy();
    delete entityToDestroy;
    entityToDestroy = nullptr;
}
