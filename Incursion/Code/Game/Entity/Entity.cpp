#include "Entity.hpp"

#include "Engine/Core/ErrorWarningAssert.hpp"

#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/Math/MathUtils.hpp"
#include "Engine/Core/Math/Primatives/Disc.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/Sprite/SpriteDefinition.hpp"

#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/Map/Map.hpp"

//-------------------------------------------------------------------------------
Entity::Entity( Game* gameInstance,
                Map* currentMap,
                const Vec3& startingPositon )
    : m_GameInstance( gameInstance )
    , m_CurrentMap( currentMap )
    , m_Position( startingPositon )
{
}

Entity::Entity( Game* gameInstance,
                Map* currentMap,
                const Vec3& startingPosition,
                Faction faction )
    : m_GameInstance( gameInstance )
    , m_CurrentMap( currentMap )
    , m_Position( startingPosition )
    , m_EntityFaction( faction )
{
}

//-------------------------------------------------------------------------------
Entity::~Entity()
{
    // DO NOTHING
}

bool Entity::DoEntityListsOverlap( EntityListIndex l1, EntityListIndex l2 )
{
    // Bullets do not overlap
    if ( l1 == ENTITY_BULLET_ALLIED && l2 == ENTITY_BULLET_ENEMY ||
         l2 == ENTITY_BULLET_ALLIED && l1 == ENTITY_BULLET_ENEMY )
    {
        return false;
    }

    // Allied bullets do not overlap allied tanks or turrets
    if ( (l1 == ENTITY_BULLET_ALLIED && (l2 == ENTITY_ALLIED_TANK ||
                                          l2 == ENTITY_ALLIED_TURRET)) ||
                                          (l2 == ENTITY_BULLET_ALLIED && (l1 == ENTITY_ALLIED_TANK ||
                                                                           l1 == ENTITY_ALLIED_TURRET)) )
    {
        return false;
    }

    // Enemy bullets do not overlap enemy tanks or turrets
    if ( (l1 == ENTITY_BULLET_ENEMY && (l2 == ENTITY_ENEMY_TANK ||
                                         l2 == ENTITY_ENEMY_TURRET)) ||
                                         (l2 == ENTITY_BULLET_ENEMY && (l1 == ENTITY_ENEMY_TANK ||
                                                                         l1 == ENTITY_ENEMY_TURRET)) )
    {
        return false;
    }

    return true;
}

EntityOverlapType Entity::OverlapsWith( const Entity* const& entity1, const Entity* const& entity2 )
{
    // Entities cannot overlap with themselves
    if ( entity1 == entity2 ) { return EntityOverlapType::NONE; }
    // Entities cannot overlap with others that do not overlap with entities
    if ( !entity1->m_OverlapsEntities || !entity2->m_OverlapsEntities )
    {
        return EntityOverlapType::NONE;
    }

    EntityType entityType1 = entity1->GetEntityType();
    EntityType entityType2 = entity2->GetEntityType();

    // If entities do not get hit by bullets
    if ( !entity1->m_IsHitByBullets && (entityType2 == ENTITY_BULLET_ALLIED ||
                                         entityType2 == ENTITY_BULLET_ENEMY) )
    {
        return EntityOverlapType::NONE;
    }
    if ( !entity2->m_IsHitByBullets && (entityType1 == ENTITY_BULLET_ALLIED ||
                                         entityType1 == ENTITY_BULLET_ENEMY) )
    {
        return EntityOverlapType::NONE;
    }

    if ( entity1->DoesPushEntities() && entity1->IsPushedByEntities() &&
         entity2->DoesPushEntities() && entity2->IsPushedByEntities() )
    {
        return EntityOverlapType::PUSH_PUSH;
    }

    if ( entity1->DoesPushEntities() && entity2->IsPushedByEntities() && !entity1->IsFixed() ||
         entity2->DoesPushEntities() && entity1->IsPushedByEntities() && !entity2->IsFixed() )
    {
        return EntityOverlapType::PUSH_NO_PUSH;
    }

    if ( entity1->DoesPushEntities() && entity2->IsFixed()  && !entity1->IsFixed() ||
         entity2->DoesPushEntities() && entity1->IsFixed()  && !entity2->IsFixed() )
    {
        return EntityOverlapType::PUSH_FIXED;
    }

    return EntityOverlapType::OVERLAP_ONLY;
}

bool Entity::OverlapsWithTiles( const Entity* const& entity )
{
    return entity->m_OverlapsTiles;
}

void Entity::Create()
{
}

//-------------------------------------------------------------------------------
void Entity::Update( float deltaSeconds )
{
    m_Age += deltaSeconds;

    m_Velocity *= m_VelocityModifier;
    m_Position += m_Velocity * deltaSeconds;
    m_Velocity += m_Acceleration * deltaSeconds;

    m_AngleDegrees += m_AngularVelocity * deltaSeconds;
    m_AngularVelocity += m_AngularAcceleration * deltaSeconds;

    m_VelocityModifier = 1.f;
}

//-------------------------------------------------------------------------------
void Entity::DebugRender() const
{
    g_Renderer->BindTexture( nullptr );
    // Draw debug velocity
//     g_Renderer->DrawLine( Vec2( m_Position.x, m_Position.y ),
//                           Vec2( m_Position.x, m_Position.y ) +
//                           Vec2( m_Velocity.x, m_Velocity.y ),
//                           Rgba8( 255, 255, 0 ), .05f );
//            // Draw Physics circle
//     g_Renderer->DrawDiscPerimeter( Disc( Vec2( m_Position.x, m_Position.y ),
//                                 m_PhysicsRadius * m_Scale.x ),
//                           Rgba8::CYAN,
//                           .025f );
//          // Draw Cosmetic circle
//     g_Renderer->DrawDiscPerimeter( Disc( Vec2( m_Position.x, m_Position.y ),
//                                 m_CosmeticRadius * m_Scale.x ),
//                           Rgba8::MAGENTA,
//                           .025f );
}

void Entity::Destroy()
{
}

//-------------------------------------------------------------------------------
const Vec3 Entity::GetPosition() const
{
    return m_Position;
}

//-------------------------------------------------------------------------------
const Vec3 Entity::GetVelocity() const
{
    return m_Velocity;
}

//-------------------------------------------------------------------------------
const Vec3 Entity::GetAcceleration() const
{
    return m_Acceleration;
}

//-------------------------------------------------------------------------------
const Vec3 Entity::GetForwardVector() const
{
    return Vec3::MakeFromPolarDegreesXY( m_AngleDegrees );
}

const Vec3 Entity::GetScale() const
{
    return m_Scale;
}

//-------------------------------------------------------------------------------
float Entity::GetAngleDegrees() const
{
    return m_AngleDegrees;
}

//-------------------------------------------------------------------------------
float Entity::GetAngularVelocity() const
{
    return m_AngularVelocity;
}

//-------------------------------------------------------------------------------
float Entity::GetAngularAcceleration() const
{
    return m_AngularAcceleration;
}

const Disc Entity::GetEntityPhysicsDisc() const
{
    return Disc( static_cast<Vec2>(m_Position), m_PhysicsRadius * m_Scale.x );
}

const Texture* Entity::GetTexture() const
{
    if ( m_Texture != nullptr )
    {
        return m_Texture;
    }
    else
    {
        if ( m_Sprite != nullptr )
        {
            return &m_Sprite->GetTexture();

        }
    }

    return nullptr;
}

const Rgba8 Entity::GetTint() const
{
    return m_Tint;
}

void Entity::SetTexture( Texture* newTexture )
{
    m_Texture = newTexture;
}

EntityType Entity::GetEntityType() const
{
    return m_EntityType;
}

Faction Entity::GetEntityFaction() const
{
    return m_EntityFaction;
}

float Entity::GetVelocityModifier() const
{
    return m_VelocityModifier;
}

//-------------------------------------------------------------------------------
int Entity::GetHealth() const
{
    return m_Health;
}

float Entity::GetAge() const
{
    return m_Age;
}

//-------------------------------------------------------------------------------
bool Entity::IsDead() const
{
    return m_IsDead;
}

//-------------------------------------------------------------------------------
// bool Entity::IsOffscreen() const
// {
//     if ( m_Position.x < -MAX_SCREEN_SHAKE - m_CosmeticRadius )
//     {
//         return true;
//     }
//     if ( m_Position.x > CAMERA_SIZE_X + MAX_SCREEN_SHAKE + m_CosmeticRadius )
//     {
//         return true;
//     }
//     if ( m_Position.y < -MAX_SCREEN_SHAKE - m_CosmeticRadius )
//     {
//         return true;
//     }
//     if ( m_Position.y > CAMERA_SIZE_Y + MAX_SCREEN_SHAKE + m_CosmeticRadius )
//     {
//         return true;
//     }
//     return false;
// }

//-------------------------------------------------------------------------------
bool Entity::IsGarbage() const
{
    return m_IsGarbage;
}

const Vec2 Entity::GetBoundBoxUnits() const
{
    return m_BoundingBoxUnits;
}

bool Entity::IsFixed() const
{
    return m_IsFixed;
}

bool Entity::IsPushedByWalls() const
{
    return m_IsPushedByWalls;
}

bool Entity::IsPushedByEntities() const
{
    return m_IsPushedByEntities;
}

bool Entity::DoesPushEntities() const
{
    return m_DoesPushEntities;
}

bool Entity::IsHitByBullets() const
{
    return m_IsHitByBullets;
}

const Vec3 Entity::GetPhysicsDiscNormalAt( const Vec3& hitPosition )
{
    float angle = (m_Position - hitPosition).GetAngleAboutZDegrees();
    return Vec3::MakeFromPolarDegreesXY(angle);
}

//-------------------------------------------------------------------------------
void Entity::SetPosition( const Vec3& newPosition )
{
    m_Position = newPosition;
}

//-------------------------------------------------------------------------------
void Entity::AddPosition( const Vec3& deltaPosition )
{
    m_Position += deltaPosition;
}

//-------------------------------------------------------------------------------
void Entity::SetVelocity( const Vec3& newVelocity )
{
    m_Velocity = newVelocity;
}

//-------------------------------------------------------------------------------
void Entity::AddVelocity( const Vec3& deltaVelocity )
{
    m_Velocity += deltaVelocity;
}

//-------------------------------------------------------------------------------
void Entity::SetAcceleration( const Vec3& newAcceleration )
{
    m_Acceleration = newAcceleration;
}

//-------------------------------------------------------------------------------
void Entity::AddAcceleration( const Vec3& deltaAcceleration )
{
    m_Acceleration += deltaAcceleration;
}

void Entity::SetScale( const Vec3& newScale )
{
    m_Scale = newScale;
}

void Entity::AddScale( const Vec3& deltaScale )
{
    m_Scale = deltaScale;
}

void Entity::SetUniformScale( float newScale )
{
    m_Scale = Vec3( newScale, newScale, newScale );
}

void Entity::AddUniformScale( float deltaScale )
{
    m_Scale += Vec3( deltaScale, deltaScale, deltaScale );
}

//-------------------------------------------------------------------------------
void Entity::SetAngleDegrees( float newRotationDegrees )
{
    m_AngleDegrees = newRotationDegrees;
}

//-------------------------------------------------------------------------------
void Entity::AddAngleDegrees( float deltaDegrees )
{
    m_AngleDegrees += deltaDegrees;
}

//-------------------------------------------------------------------------------
void Entity::SetAngularVelocity( float newAngularVelocity )
{
    m_AngularVelocity = newAngularVelocity;
}

//-------------------------------------------------------------------------------
void Entity::AddAngularVelocity( float deltaAngularVelocity )
{
    m_AngularVelocity += deltaAngularVelocity;
}

//-------------------------------------------------------------------------------
void Entity::SetAngularAcceleration( float newAngularAcceleration )
{
    m_AngularAcceleration = newAngularAcceleration;
}

//-------------------------------------------------------------------------------
void Entity::AddAngularAcceleration( float deltaAngularAcceleration )
{
    m_AngularAcceleration += deltaAngularAcceleration;
}



void Entity::SetVelocityModifier( float velocityModifier )
{
    m_VelocityModifier = velocityModifier;
}

void Entity::SetHealth( int newHealth )
{
    m_Health = newHealth;
}

//-------------------------------------------------------------------------------
void Entity::DamageEntity( int damage )
{
    m_Health -= damage;

    if ( m_DamageSoundInitiallized )
    {
        g_AudioSystem->PlaySound( m_DamageSound );
    }

    if ( m_Health <= 0 )
    {
        m_IsDead = true;
        Die();
    }
}

//-------------------------------------------------------------------------------
void Entity::SetDead( bool newDead )
{
    m_IsDead = newDead;
}
