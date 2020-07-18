#include "TurretNPC.hpp"

#include "Engine/Core/Utils/VectorPcuUtils.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/Math/MathUtils.hpp"
#include "Engine/Core/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/Texture.hpp"

#include "Game/Game.hpp"
#include "Game/World.hpp"
#include "Game/AssetManagers/AudioManager.hpp"
#include "Game/AssetManagers/TextureManager.hpp"
#include "Game/Entity/Bullet.hpp"
#include "Game/Entity/PlayerCharacter.hpp"
#include "Game/Map/Map.hpp"
#include "Game/Map/Raycast.hpp"


#include "Engine/Core/Math/Primatives/LineSeg2D.hpp"
#include "Engine/Renderer/Mesh/MeshUtils.hpp"

TurretNPC::TurretNPC( Game* gameInstance,
                      Map* currentMap,
                      const Vec3& startingPosition,
                      EntityType type,
                      Faction faction )
    : Entity( gameInstance, currentMap, startingPosition, faction )
{
    m_PhysicsRadius = .3f;
    m_CosmeticRadius = .6f;

    m_EntityType = type;

    m_Health = TURRET_NPC_HEALTH;

    m_Texture = g_Renderer->CreateOrGetTextureFromFile( SPRITE_ENEMY_TURRENT_BASE );
    m_TurretTexture = g_Renderer->CreateOrGetTextureFromFile( SPRITE_ENEMY_TURRENT_TOP );

    m_OverlapsTiles = false;
    m_IsPushedByWalls = true;
    m_OverlapsEntities = true;
    m_IsFixed = true;
    m_IsPushedByEntities = false;
    m_DoesPushEntities = true;
    m_IsHitByBullets = true;

    m_DamageSoundInitiallized = true;
    m_DamageSound = AUDIO_ENEMY_HIT;
}

void TurretNPC::Create()
{
}

void TurretNPC::Update( float deltaSeconds )
{
    if ( m_IsDead ) { return; }

    TurretAI( deltaSeconds );

    m_RayTraceResult = g_GameInstance->GetCurrentWorld()->
        GetCurrentMap()->
        RayCastVisual( static_cast<Vec2>(m_Position),
                       m_AngleDegrees,
                       TURRET_NPC_VIEW_DISTANCE );

    Entity::Update( deltaSeconds );
}

void TurretNPC::Render() const
{
    if ( m_IsDead ) { return; }

    RenderBase();
    RenderTurrent();

    std::vector<VertexMaster> lineVisual;
    AppendLineSeg2DToVectorMaster( lineVisual, LineSeg2D( static_cast<Vec2>( m_Position ), m_RayTraceResult.hitPosition ), Rgba8::RED, .01f );

    g_Renderer->BindTexture( nullptr );
    g_Renderer->DrawVertexArray( lineVisual );
}

void TurretNPC::Die()
{
    g_AudioSystem->PlaySound( AUDIO_ENEMY_DIED );

    m_CurrentMap->SpawnNewExplosion( m_Position, 1.25f, m_Scale * 1.25f );
}

void TurretNPC::Destroy()
{
}

void TurretNPC::RenderBase() const
{
    std::vector<VertexMaster> baseVisual;
    AABB2 boundingBox = AABB2::UNIT_AROUND_ZERO;
    boundingBox.SetDimensions( m_BoundingBoxUnits );
    AppendAABB2( baseVisual, boundingBox, Rgba8::WHITE );

    TransformVertexArray( baseVisual, static_cast<Vec2>(m_Position), 0.f, 1.f );
    g_Renderer->BindTexture( m_Texture );
    g_Renderer->DrawVertexArray( baseVisual );
}

void TurretNPC::RenderTurrent() const
{
    std::vector<VertexMaster> bodyVisual;
    AABB2 boundingBox = AABB2::UNIT_AROUND_ZERO;
    boundingBox.SetDimensions( m_BoundingBoxUnits );
    AppendAABB2( bodyVisual, boundingBox, Rgba8::WHITE );

    TransformVertexArray( bodyVisual, static_cast<Vec2>(m_Position), m_AngleDegrees, 1.f );
    g_Renderer->BindTexture( m_TurretTexture );
    g_Renderer->DrawVertexArray( bodyVisual );
}

void TurretNPC::TurretAI( float deltaSeconds )
{

    switch ( m_TurretState )
    {
        case TurretAIState::SCAN:
            ScanBahavior( deltaSeconds );
            break;
        case TurretAIState::WATCH:
            WatchBehavior( deltaSeconds );
            break;
        case TurretAIState::ATTACK:
            AttackBehavior( deltaSeconds );
            break;
        default:
            break;
    }
//     if ( IsPlayerVisable() )
//     {
//         m_CurrentMap->SetTilePositionVisable( static_cast<IntVec2>(static_cast<Vec2>(m_Position) ) );
// 
//         Entity* player = ( Entity*) (g_GameInstance->GetCurrentWorld()->GetPlayerCharacter());
//         m_TargetOrientation = (player->GetPosition() - m_Position).GetAngleAboutZDegrees();
// 
//         if ( abs( GetShortestAngularDisplacement( m_TargetOrientation, m_AngleDegrees ) ) < TURRET_NPC_ENGAGE_APETURE )
//         {
//             if ( m_LastShotCheck + TURRET_NPC_RELOAD_SPEED < m_Age )
//             {
//                 m_LastShotCheck = m_Age;
// 
//                 Vec2 spawnPosition = static_cast<Vec2>(m_Position) +
//                     Vec2::MakeFromPolarDegrees( m_AngleDegrees, .25f );
// 
//                 float bulletDireciton = m_AngleDegrees;
//                 EntityType bulletType = m_EntityFaction == FACTION_PLAYER ? ENTITY_BULLET_ALLIED : ENTITY_BULLET_ENEMY;
//                 Bullet* bullet = ( Bullet*) m_CurrentMap->SpawnNewEntity( bulletType,
//                                                                           spawnPosition );
//                 bullet->SetBulletDireciton( Vec2::MakeFromPolarDegrees( bulletDireciton ) );
//                 g_AudioSystem->PlaySound( AUDIO_ENEMY_SHOOT );
//             }
//         }
// 
//         m_AngleDegrees = GetTurnedTowards( m_AngleDegrees,
//                                            m_TargetOrientation,
//                                            TURRET_NPC_MAX_ROTATION_SECONDS_ENGAGED * deltaSeconds );
//     }
//     else
//     {
//         m_TargetOrientation += TURRET_NPC_MAX_ROTATION_SECONDS_PATROL * deltaSeconds;
//         m_AngleDegrees = GetTurnedTowards( m_AngleDegrees,
//                                            m_TargetOrientation,
//                                            TURRET_NPC_MAX_ROTATION_SECONDS_PATROL * deltaSeconds );
//     }
}

void TurretNPC::ScanBahavior( float deltaSeconds )
{
    if ( IsPlayerVisable() )
    {
        m_TurretState = TurretAIState::ATTACK;
        return;
    }

    m_TargetOrientation += TURRET_NPC_MAX_ROTATION_SECONDS_PATROL * deltaSeconds;
    m_AngleDegrees = GetTurnedTowards( m_AngleDegrees,
                                       m_TargetOrientation,
                                       TURRET_NPC_MAX_ROTATION_SECONDS_PATROL * deltaSeconds );
}

void TurretNPC::WatchBehavior( float deltaSeconds )
{
    if ( IsPlayerVisable() )
    {
        m_TurretState = TurretAIState::ATTACK;
        return;
    }

    if ( m_GameInstance->GetCurrentWorld()->GetPlayerCharacter()->IsDead() )
    {
        m_TurretState = TurretAIState::SCAN;
        return;
    }

    float shortest = GetShortestAngularDisplacement( m_LastSeenAngle, m_AngleDegrees );
    if ( abs( shortest ) >= TURRET_NPC_WATCH_APETURE_HALF )
    {
        m_TurnDirection = GetAngleDirectionTowards( m_AngleDegrees, m_TargetOrientation );
    }

    m_AngleDegrees += m_TurnDirection * TURRET_NPC_MAX_ROTATION_SECONDS_ENGAGED * deltaSeconds;
}

void TurretNPC::AttackBehavior( float deltaSeconds )
{
    if ( !IsPlayerVisable() )
    {
        m_TurretState = TurretAIState::WATCH;
        return;
    }
    // Sets the tile visable if the player is being targeted
    m_CurrentMap->SetTilePositionVisable( static_cast<IntVec2>(static_cast<Vec2>(m_Position)) );

    Entity* player = ( Entity*) (g_GameInstance->GetCurrentWorld()->GetPlayerCharacter());
    m_TargetOrientation = (player->GetPosition() - m_Position).GetAngleAboutZDegrees();

    m_LastSeenAngle = m_TargetOrientation;
    m_TurnDirection = m_TargetOrientation > m_AngleDegrees ? 1 : -1;

    if ( abs( GetShortestAngularDisplacement( m_TargetOrientation, m_AngleDegrees ) ) < TURRET_NPC_ENGAGE_APETURE )
    {
        if ( m_LastShotCheck + TURRET_NPC_RELOAD_SPEED < m_Age )
        {
            m_LastShotCheck = m_Age;

            ShootBullet();
        }
    }

    m_AngleDegrees = GetTurnedTowards( m_AngleDegrees,
                                       m_TargetOrientation,
                                       TURRET_NPC_MAX_ROTATION_SECONDS_ENGAGED * deltaSeconds );
}

void TurretNPC::ShootBullet()
{
    Vec2 spawnPosition = static_cast<Vec2>(m_Position) +
        Vec2::MakeFromPolarDegrees( m_AngleDegrees, .5f );

    float bulletDireciton = m_AngleDegrees;
    EntityType bulletType = m_EntityFaction == FACTION_PLAYER ? ENTITY_BULLET_ALLIED : ENTITY_BULLET_ENEMY;
    Bullet* bullet = ( Bullet*) m_CurrentMap->SpawnNewEntity( bulletType,
                                                              spawnPosition );
    bullet->SetBulletDireciton( Vec2::MakeFromPolarDegrees( bulletDireciton ) );

    m_CurrentMap->SpawnNewExplosion( static_cast<Vec3>(spawnPosition), 
                                     .25f, 
                                     Vec3( .25f, .25f, .25f ) );

    g_AudioSystem->PlaySound( AUDIO_ENEMY_SHOOT );
}

bool TurretNPC::IsPlayerVisable()
{
    Entity* player = ( Entity*) (m_GameInstance->GetCurrentWorld()->GetPlayerCharacter());
    if ( player == nullptr ) { return false; }
    if ( player->IsDead() ) { return false; }

    return m_CurrentMap->HasLineOfSight( *this, *player, TURRET_NPC_VIEW_DISTANCE );
}
