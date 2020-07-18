#include "TankNPC.hpp"

#include "Engine/Core/Utils/VectorPcuUtils.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Math/MathUtils.hpp"
#include "Engine/Core/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/RenderContext.hpp"

#include "Game/Game.hpp"
#include "Game/World.hpp"
#include "Game/AssetManagers/AudioManager.hpp"
#include "Game/AssetManagers/TextureManager.hpp"
#include "Game/Entity/Bullet.hpp"
#include "Game/Map/Map.hpp"
#include "Game/Map/Raycast.hpp"

#include "Engine/Renderer/Mesh/MeshUtils.hpp"

TankNPC::TankNPC( Game* gameInstance,
                  Map* currentMap,
                  const Vec3& startingPosition,
                  EntityType type,
                  Faction faction )
    : Entity( gameInstance, currentMap, startingPosition, faction )
{
    m_PhysicsRadius = .25f;
    m_CosmeticRadius = .6f;

    m_EntityType = type;

    m_Health = TANK_NPC_HEALTH;

    m_Texture = g_Renderer->CreateOrGetTextureFromFile( SPRITE_ENEMY_TANK );

    m_OverlapsTiles = true;
    m_IsPushedByWalls = true;
    m_OverlapsEntities = true;
    m_IsFixed = false;
    m_IsPushedByEntities = true;
    m_DoesPushEntities = true;
    m_IsHitByBullets = true;

    m_DamageSoundInitiallized = true;
    m_DamageSound = AUDIO_ENEMY_HIT;
}

void TankNPC::Create()
{
}

void TankNPC::Update( float deltaSeconds )
{
    if ( m_IsDead ) { return; }

    TankAI( deltaSeconds );



    Entity::Update( deltaSeconds );
}

void TankNPC::Render() const
{
    if ( m_IsDead ) { return; }

    std::vector<VertexMaster> bodyVisual;
    AABB2 boundingBox = AABB2::UNIT_AROUND_ZERO;
    boundingBox.SetDimensions( m_BoundingBoxUnits );
    AppendAABB2( bodyVisual, boundingBox, Rgba8::WHITE );

    TransformVertexArray( bodyVisual, static_cast<Vec2>(m_Position), m_AngleDegrees, static_cast<Vec2>(m_Scale) );
    g_Renderer->BindTexture( m_Texture );
    g_Renderer->DrawVertexArray( bodyVisual );
}

void TankNPC::DebugRender() const
{
    Vec2 pos2 = static_cast<Vec2>(m_Position);
    if ( m_TankState == TankAIState::PURSUE ||
         m_TankState == TankAIState::ATTACK )
    {
        // g_Renderer->DrawLine( pos2, m_LastSeenPosition, Rgba8::RED, .1f );
    }

    RayCastHit leftWisker = m_CurrentMap->RayCastSolid( pos2,
                                                        m_TargetOrientation + TANK_NPC_WISKER_ANGLE,
                                                        TANK_NPC_WISKER_DIST );
    RayCastHit rightWisker = m_CurrentMap->RayCastSolid( pos2,
                                                         m_TargetOrientation - TANK_NPC_WISKER_ANGLE,
                                                         TANK_NPC_WISKER_DIST );

    // g_Renderer->DrawLine( pos2, leftWisker.hitPosition, Rgba8::YELLOW, .05f );
    // g_Renderer->DrawLine( pos2, rightWisker.hitPosition, Rgba8::YELLOW, .05f );

    Entity::DebugRender();
}

void TankNPC::Die()
{
    g_AudioSystem->PlaySound( AUDIO_ENEMY_DIED );

    m_CurrentMap->SpawnNewExplosion( m_Position, 1.f, m_Scale );
}

void TankNPC::Destroy()
{
}

void TankNPC::TankAI( float deltaSeconds )
{
    switch ( m_TankState )
    {
        case TankAIState::WANDER:
            WanderBehavior( deltaSeconds );
            break;
        case TankAIState::PURSUE:
            PursueBehavior( deltaSeconds );
            break;
        case TankAIState::ATTACK:
            AttackBehavior( deltaSeconds );
            break;
        default:
            break;
    }

    m_AngleDegrees = GetTurnedTowards( m_AngleDegrees,
                                       m_TargetOrientation,
                                       TANK_MAX_ROTATION_SECONDS * deltaSeconds );
}

void TankNPC::WanderBehavior( float deltaSeconds )
{
    UNUSED( deltaSeconds );

    if ( IsPlayerVisable() )
    {
        m_TankState = TankAIState::ATTACK;
        return;
    }

    Navigate( deltaSeconds );

    SetVelocity( Vec3::MakeFromPolarDegreesXY( m_AngleDegrees, TANK_NPC_MAX_VELOCITY ) );
}

void TankNPC::Navigate( float deltaSeconds )
{
    Vec2 pos2 = static_cast<Vec2>(m_Position);
    RayCastHit leftWisker = m_CurrentMap->RayCastSolid( pos2,
                                                        m_TargetOrientation + TANK_NPC_WISKER_ANGLE,
                                                        TANK_NPC_WISKER_DIST );
    RayCastHit rightWisker = m_CurrentMap->RayCastSolid( static_cast<Vec2>(m_Position),
                                                         m_TargetOrientation - TANK_NPC_WISKER_ANGLE,
                                                         TANK_NPC_WISKER_DIST );

    float leftPercent = 1.f - leftWisker.percentToFinish;
    float rightPercent = 1.f - rightWisker.percentToFinish;

    bool deepWisker = false;
    if ( leftPercent > .4f || rightPercent > .4f )
    {
        m_VelocityModifier = .25f;
        deepWisker = true;
    }
    bool doubleDeepWisker = false;
    if ( leftPercent > .4f && rightPercent > .4f )
    {
        m_VelocityModifier = .1f;
        doubleDeepWisker = true;
    }
    
    if ( leftPercent > rightPercent )
    {
        m_TargetOrientation -= leftPercent * TANK_MAX_ROTATION_SECONDS * deltaSeconds;
        if ( !deepWisker )
        {
            m_LastGoalCheck = m_Age;
        }
    }
    else if ( leftPercent < rightPercent )
    {
        m_TargetOrientation += rightPercent * TANK_MAX_ROTATION_SECONDS * deltaSeconds;
        if ( !deepWisker )
        {
            m_LastGoalCheck = m_Age;
        }
    }

    if ( doubleDeepWisker )
    {
        m_TargetOrientation += 180;
    }

    if ( m_LastGoalCheck + TANK_NPC_RANDOM_GOAL_TIME < m_Age )
    {
        m_TargetOrientation = NewTargetOrientation();
        m_LastGoalCheck = m_Age;
    }
}

void TankNPC::PursueBehavior( float deltaSeconds )
{
    UNUSED( deltaSeconds );

    if ( IsPlayerVisable() )
    {
        m_TankState = TankAIState::ATTACK;
        return;
    }

    if ( GetEntityPhysicsDisc().IsPointInside( m_LastSeenPosition ) )
    {
        m_TankState = TankAIState::WANDER;
        return;
    }

    float angleToPoint = (m_LastSeenPosition - static_cast<Vec2>(m_Position)).GetAngleDegrees();
    SetVelocity( Vec3::MakeFromPolarDegreesXY( angleToPoint, TANK_NPC_MAX_VELOCITY ) );
}

void TankNPC::AttackBehavior( float deltaSeconds )
{
    UNUSED( deltaSeconds );

    if ( !IsPlayerVisable() )
    {
        m_TankState = TankAIState::PURSUE;
        return;
    }

    Entity* player = ( Entity*) (g_GameInstance->GetCurrentWorld()->GetPlayerCharacter());

    m_LastSeenPosition = static_cast<Vec2>(player->GetPosition());

    m_TargetOrientation = (player->GetPosition() - m_Position).GetAngleAboutZDegrees();

    if ( abs( GetShortestAngularDisplacement( m_TargetOrientation, m_AngleDegrees ) ) < TANK_NPC_FOLLOW_APETURE )
    {
        SetVelocity( Vec3::MakeFromPolarDegreesXY( m_AngleDegrees, TANK_NPC_MAX_VELOCITY ) );
    }
    else
    {
        SetVelocity( Vec3::ZERO );
    }

    if ( abs( GetShortestAngularDisplacement( m_TargetOrientation, m_AngleDegrees ) ) < TANK_NPC_ENGAGE_APETURE )
    {
        if ( m_LastShotCheck + TANK_NPC_RELOAD_SPEED < m_Age )
        {
            m_LastShotCheck = m_Age;

            ShootBullet();
        }
    }
}

void TankNPC::ShootBullet()
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

bool TankNPC::IsPlayerVisable()
{
    Entity* player = ( Entity*) (m_GameInstance->GetCurrentWorld()->GetPlayerCharacter());
    if ( player == nullptr ) { return false; }
    if ( player->IsDead() ) { return false; }

    return m_CurrentMap->HasLineOfSight( *this, *player, TANK_NPC_VIEW_DISTANCE );
}

float TankNPC::NewTargetOrientation()
{
    return g_GameInstance->GetRng()->FloatLessThan( 360.f );
}
