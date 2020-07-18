#include "Bullet.hpp"

#include "Engine/Core/Utils/VectorPcuUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"

#include "Game/Map/Map.hpp"

#include "Engine/Renderer/Mesh/MeshUtils.hpp"

Bullet::Bullet( Game* gameInstance,
                Map* currentMap,
                const Vec3& startingPosition,
                EntityType type,
                Faction faction )
    : Entity( gameInstance, currentMap, startingPosition, faction )
{
    m_PhysicsRadius = .3f;
    m_CosmeticRadius = .55f;

    SetUniformScale( .125f );

    m_EntityType = type;

    m_Texture = g_Renderer->CreateOrGetTextureFromFile( "Data/Sprites/Bullet.png" );

    m_OverlapsTiles = true;
    m_IsPushedByWalls = false;
    m_OverlapsEntities = true;
    m_IsFixed = false;
    m_IsPushedByEntities = false;
    m_DoesPushEntities = false;
    m_IsHitByBullets = false;
}

void Bullet::Create()
{

}

void Bullet::Update( float deltaSeconds )
{
    if ( m_IsDead ) { return; }

    Entity::Update( deltaSeconds );
}

void Bullet::Render() const
{
    if ( m_IsDead ) { return; }

    std::vector<VertexMaster> visual;
    Vec2 minUV = Vec2::ZERO;
    Vec2 maxUV = Vec2::ONE;

    AppendAABB2( visual, AABB2::UNIT_AROUND_ZERO, Rgba8::WHITE );

    TransformVertexArray( visual, static_cast<Vec2>(m_Position), m_AngleDegrees, static_cast<Vec2>(m_Scale) );

    g_Renderer->BindTexture( GetTexture() );
    g_Renderer->DrawVertexArray( visual );
}

void Bullet::Die()
{
    m_IsDead = true;
    m_IsGarbage = true;

    m_CurrentMap->SpawnNewExplosion( m_Position, .5f, Vec3( .25f, .25f, .25f ) );
}

void Bullet::Destroy()
{
}

void Bullet::SetBulletDireciton( const Vec2& forward )
{
    float angle = forward.GetAngleDegrees();
    m_AngleDegrees = angle;

    m_Velocity = static_cast<Vec3>(forward * BULLET_MAX_VELOCITY);
}
