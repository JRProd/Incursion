#pragma once

#include <vector>

#include "Engine/Core/Math/Primatives/IntVec2.hpp"

#include "Game/Entity/Entity.hpp"
#include "Game/Map/Tile.hpp"
#include "Game/Map/Generation/MapGeneration.hpp"
#include "Game/Map/Raycast.hpp"

struct VertexMaster;
class Game;
class World;
class PlayerCharacter;
class Bullet;
class Explosion;
class Gameboy;

class Map
{
public:
    ~Map();
    explicit Map( Game* gameInstance, World* world, int sizeX, int sizeY );
    explicit Map( Game* gameInstnace, World* world, const IntVec2& size );

    //-------------------------------------------------------------------------
    // Map Lifecycle
    void Create();
    void GenerateMap( MapGeneration& generator);
    void Update( float deltaSeconds );
    void Render() const;
    void DebugRender() const;
    void Destroy();

    //-------------------------------------------------------------------------
    // Map queries
    PlayerCharacter* GetPlayerEntity() const;
    const IntVec2 GetMapSize() const;
    bool IsPointInSolid( const Vec2& point ) const;
    bool IsPointBlockRaycast( const Vec2& point ) const;

    //-------------------------------------------------------------------------
    // Tile queries
    bool IsValidTilePos( const IntVec2& testPos ) const;
    const IntVec2 GetTilePositionFromWorldCoords( const Vec2& worldCoords ) const;
    const IntVec2 GetTilePositionFromIndex( int index ) const;
    const Tile* GetTileFromIndex( int index ) const;
    const Tile* GetTileFromPosition( const IntVec2& position ) const;
    int GetTileIndexFromPosition( const IntVec2& position ) const;
    bool IsTileSolid( const Tile& tile ) const;
    bool DoseTileBlockRaycast( const Tile& tile ) const;

    //-------------------------------------------------------------------------
    void SetTypeOfTile( const IntVec2& positions, TileType tileType );
    void SetTilePositionVisable( const IntVec2& position );

    //-------------------------------------------------------------------------
    // Spawning Entities
    Entity* SpawnNewEntityAtStart( EntityType type );
    Entity* SpawnNewEntity( EntityType type, const Vec2& spawnPosition );
    Explosion* SpawnNewExplosion( const Vec3& position,  
                                  float duration, 
                                  const Vec3& scale );
    void SpawnNewEntitiesOfTypeInOpenSpace( int number, EntityType type );
    void RequestRespawn( PlayerCharacter* entityToRespawn );
    void AddEntityToMapAtStart( Entity* entity );
    void AddEntityToMap( Entity* entity, const Vec2& spawnPosition );

    //-------------------------------------------------------------------------
    // Ray cast Functions
    RayCastHit RayCastSolid( const Vec2& start, float angleDegrees, float maxDist ) const;
    RayCastHit RayCastSolid( const Vec2& start, const Vec2& direciton, float maxDist ) const;
    RayCastHit RayCastVisual( const Vec2& start, float angleDegrees, float maxDist ) const;
    RayCastHit RayCastVisual( const Vec2& start, const Vec2& direciton, float maxDist ) const;
    bool HasLineOfSight( const Entity& entity1, const Entity& entity2, float maxDist ) const;

private:
    Game* m_GameInstance = nullptr;
    World* m_World = nullptr;

    IntVec2 m_Size = IntVec2::ZERO;
    int m_NumTiles = 0;

    EntityList m_EntityListsByType[ NUM_ENTITY_TYPES ];
    std::vector<Tile> m_Tiles;

    IntVec2 m_StartLocation = IntVec2::ZERO;
    IntVec2 m_ExitLocation = IntVec2::ZERO;

    void UpdateEntities( float deltaSeconds );
    void UpdateFogOfWar( EntityType revealForEntityType, 
                         int fieldOfView, 
                         int viewHeight,
                         float viewAspect );
    void UpdateFogOfWarOnTilesFrom( const Entity* const& entity, 
                                    std::vector<Tile*>& tiles );
    void RenderTiles() const;
    void AppendTileToVectorPCU( std::vector<VertexMaster>& vector, 
                                const Tile& currentTile,
                                bool shadeUnseenTiles = true) const;
    void AppendTileDefToVectorPCU( std::vector<VertexMaster>& vector,
                                   const AABB2& bounds,
                                   const TileDefinition& tileDef ) const;
    void RenderEntities() const;
    void DebugRenderEntities() const;

    //-------------------------------------------------------------------------
    // Entity Spawner
    Entity* SpawnEntityOfType( EntityType type, const Vec2& spawnPosition );
    void AddEntityToMap( EntityType type, Entity* entity );

    void HandleMapCollisions();
    void HandleListVsListOverlaps( EntityListIndex l1, EntityListIndex l2 );
    void HandleEntityVsEntityOverlaps( Entity*& entity1, Entity*& entity2 );
    void HandleOverlapOnly( Entity*& entity1, Entity*& entity2 );
    void HandlePushedVsPushed( Entity*& entity1, Entity*& entity2 );
    void DeterminePushedVsFixedEntity( Entity*& entity1, Entity*& entity2 );
    void HandlePushedVsFixed( Entity*& pushed, const Entity* fixed );
    void HandleEntityVsBulletCollision( Entity*& entity, Bullet*& bullet );

    void HandleListVsTileOverlaps( EntityListIndex l1 );
    void HandleEntityVsTileOverlap( Entity* entity, const Tile* tile );
    void HandleEntityVsTileOverlapOnly( Entity* entity, const Tile* tile );
    void HandleEntityVsTileCollision( Entity* entity, const Tile* tile );



    Vec2 FindPointNotInWall( const Vec2& bounds );

    void DeleteGarbageEntities();
    void DestroyEntities();
    void DestroyEntity( Entity*& entityToDestroy );
};