#include "PlayerCharacter.hpp"

#include "Engine/Core/Utils/VectorPcuUtils.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/Math/MathUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/Fonts/BitmapFont.hpp"

#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/AssetManagers/TextureManager.hpp"
#include "Game/AssetManagers/AudioManager.hpp"
#include "Game/World.hpp"
#include "Game/Entity/Bullet.hpp"
#include "Game/Map/Map.hpp"

#include "Engine/Renderer/Mesh/MeshUtils.hpp"

PlayerCharacter::PlayerCharacter( Game* gameInstance,
                                  Map* currentMap,
                                  const Vec3& startingPosition )
    : Entity( gameInstance, currentMap, startingPosition )
{
    m_PhysicsRadius = .25f;
    m_CosmeticRadius = .6f;

    m_Health = TANK_PLAYER_HEALTH;

    m_EntityType = ENTITY_PLAYER;
    m_EntityFaction = FACTION_PLAYER;

    m_OverlapsTiles = true;
    m_IsPushedByWalls = true;
    m_OverlapsEntities = true;
    m_IsFixed = false;
    m_IsPushedByEntities = true;
    m_DoesPushEntities = true;
    m_IsHitByBullets = true;

    m_DamageSoundInitiallized = true;
    m_DamageSound = AUDIO_PLAYER_HIT;
}

void PlayerCharacter::Create()
{
    m_Texture = g_Renderer->CreateOrGetTextureFromFile( SPRITE_PLAYER_TANK_BODY );
    m_TurrentTexture = g_Renderer->CreateOrGetTextureFromFile( SPRITE_PLAYER_TANK_TURRENT );
}

void PlayerCharacter::Update( float deltaSeconds )
{
    HandleUserInput();
    if ( m_IsDead || m_IsGarbage ) { return; }


    m_AngleDegrees = GetTurnedTowards( m_AngleDegrees,
                                       m_TankTargetRotation,
                                       TANK_MAX_ROTATION_SECONDS * deltaSeconds );

    m_TurrentCurrentOffset = GetTurnedTowards( m_TurrentCurrentOffset,
                                               m_TurrentTargetOffset,
                                               (TANK_MAX_ROTATION_SECONDS + TANK_TURRENT_MAX_ROTATION_SECONDS) * deltaSeconds );

    Entity::Update( deltaSeconds );

    ClampVelocity();
}

void PlayerCharacter::Render() const
{
    if ( m_IsDead || m_IsGarbage ) { return; }
    RenderTankBody();
    RenderTankTurrent();
}

void PlayerCharacter::Die()
{
    g_AudioSystem->PlaySound( AUDIO_PLAYER_DIED );
    m_CurrentMap->SpawnNewExplosion( m_Position, 1.75f, m_Scale * 1.5f );
    m_GameInstance->PlayerDied();
}

void PlayerCharacter::Destroy()
{
}

void PlayerCharacter::RespawnResetStats()
{
    m_Health = TANK_PLAYER_HEALTH;
    m_IsDead = false;

    m_Velocity = Vec3::ZERO;

    m_AngleDegrees = 0.f;
    m_TankTargetRotation = 0.f;

    m_TurrentCurrentOffset = 0.f;
    m_TurrentTargetOffset = 0.f;
}

void PlayerCharacter::TeleportToNewMap( Map* newMap )
{
    m_CurrentMap = newMap;
}

void PlayerCharacter::HandleUserInput()
{
    Vec2 tankTargetPosition = Vec2::ZERO;
    if ( g_InputSystem->IsKeyPressed( 'W' ) )
    {
        tankTargetPosition.y += 1;
    }
    if ( g_InputSystem->IsKeyPressed( 'A' ) )
    {
        tankTargetPosition.x -= 1;
    }
    if ( g_InputSystem->IsKeyPressed( 'S' ) )
    {
        tankTargetPosition.y -= 1;
    }
    if ( g_InputSystem->IsKeyPressed( 'D' ) )
    {
        tankTargetPosition.x += 1;
    }

    Vec2 turrentTargetPosition = Vec2::ZERO;
    if ( g_InputSystem->IsKeyPressed( 'I' ) )
    {
        turrentTargetPosition.y += 1;
    }
    if ( g_InputSystem->IsKeyPressed( 'J' ) )
    {
        turrentTargetPosition.x -= 1;
    }
    if ( g_InputSystem->IsKeyPressed( 'K' ) )
    {
        turrentTargetPosition.y -= 1;
    }
    if ( g_InputSystem->IsKeyPressed( 'L' ) )
    {
        turrentTargetPosition.x += 1;
    }

    if ( !m_IsDead && g_InputSystem->WasKeyJustPressed( SPACE ) )
    {
        ShootBullet();
    }

    if ( g_InputSystem->WasKeyJustPressed( 'B' ) )
    {
        if ( m_IsDead )
        {
            m_GameInstance->PlayerRespawn( this );
        }
    }

    if ( tankTargetPosition != Vec2::ZERO )
    {
        m_TankTargetRotation = tankTargetPosition.GetAngleDegrees();
        SetVelocity( Vec3::MakeFromPolarDegreesXY( m_AngleDegrees, TANK_MAX_VELOCITY ) );
    }
    else
    {
        m_TankTargetRotation = m_AngleDegrees;
        SetVelocity( Vec3::ZERO );
    }

    if ( turrentTargetPosition != Vec2::ZERO )
    {
        m_TurrentTargetOffset = turrentTargetPosition.GetAngleDegrees() - m_AngleDegrees;
    }
    else
    {
        m_TurrentTargetOffset = m_TurrentCurrentOffset;
    }

    HandleGamepadInput();
}

void PlayerCharacter::HandleGamepadInput()
{
    if ( g_InputSystem->GetXboxController( 0 ).IsConnected() )
    {
        const XboxController& gamepad = g_InputSystem->GetXboxController( 0 );

        if ( gamepad.GetLeftJoystick().GetMagnitude() > 0 )
        {
            const AnalogJoystick& leftJoystick = gamepad.GetLeftJoystick();

            m_TankTargetRotation = leftJoystick.GetAngleDegrees();

            SetVelocity( Vec3::MakeFromPolarDegreesXY( m_AngleDegrees,
                                                       TANK_MAX_VELOCITY * leftJoystick.GetMagnitude() )
            );
        }
        else if ( !g_InputSystem->IsAnyKeyDown() )
        {
            m_TankTargetRotation = m_AngleDegrees;

            SetVelocity( Vec3::ZERO );
        }

        if ( gamepad.GetRightJoystick().GetMagnitude() > 0 )
        {
            const AnalogJoystick& rightJoystick = gamepad.GetRightJoystick();

            m_TurrentTargetOffset = rightJoystick.GetAngleDegrees() - m_AngleDegrees;
        }
        else if ( !g_InputSystem->IsAnyKeyDown() )
        {
            m_TurrentTargetOffset = m_TurrentCurrentOffset;
        }

        if ( !m_IsDead && gamepad.IsButtonJustPressed( XBOX_BUTTON_R_BUMPER ) )
        {
            ShootBullet();
        }

        if ( gamepad.IsButtonJustPressed( XBOX_BUTTON_START ) )
        {
            if ( m_IsDead )
            {
                m_GameInstance->PlayerRespawn( this );
            }
        }
    }
}

void PlayerCharacter::ClampVelocity()
{
    m_Velocity.ClampLength( TANK_MAX_VELOCITY );
}

void PlayerCharacter::ShootBullet()
{
    Vec2 spawnPosition = static_cast<Vec2>(m_Position) +
        Vec2::MakeFromPolarDegrees( m_AngleDegrees + m_TurrentCurrentOffset,
        (m_TurrentBoundingBoxUnits.x * .375f) + m_TurrentVisualOffset.x );

    float bulletDireciton = m_AngleDegrees + m_TurrentCurrentOffset;
    Bullet* bullet = ( Bullet*) m_CurrentMap->SpawnNewEntity( ENTITY_BULLET_ALLIED,
                                                              spawnPosition );
    bullet->SetBulletDireciton( Vec2::MakeFromPolarDegrees( bulletDireciton ) );

    m_CurrentMap->SpawnNewExplosion( static_cast<Vec3>(spawnPosition),
                                     .25f,
                                     Vec3( .25f, .25f, .25f ) );

    g_AudioSystem->PlaySound( AUDIO_PLAYER_SHOOT );
}

void PlayerCharacter::RenderTankBody() const
{
    std::vector<VertexMaster> bodyVisual;
    AABB2 boundingBox = AABB2::UNIT_AROUND_ZERO;
    boundingBox.SetDimensions( m_BoundingBoxUnits );
    AppendAABB2( bodyVisual, boundingBox, Rgba8::WHITE );

    TransformVertexArray( bodyVisual, static_cast<Vec2>(m_Position), m_AngleDegrees, static_cast<Vec2>(m_Scale) );
    g_Renderer->BindTexture( m_Texture );
    g_Renderer->DrawVertexArray( bodyVisual );

    char* life = new char[ m_Health + 1 ];
    for ( int health = 0; health < m_Health; ++health )
    {
        life[ health ] = '|';
    }
    life[ m_Health ] = '\0';

    Rgba8 lifeColor = Rgba8::GREEN;
    if ( m_Health < floori( TANK_PLAYER_HEALTH * .66f ) )
    {
        lifeColor = Rgba8::YELLOW;
    }
    if ( m_Health < floori( TANK_PLAYER_HEALTH * .33f ) )
    {
        lifeColor = Rgba8::RED;
    }

    std::vector<VertexMaster> lifeVisual;
    g_FontDefault->AddVertsForText( lifeVisual,
                                    life,
                                    Vec2(-1.2f, -1.f),
                                    .4f,
                                    lifeColor,
                                    .25f );
    TransformVertexArray( lifeVisual, static_cast<Vec2>(m_Position), 0.f, 1.f );
    g_Renderer->BindTexture( g_FontDefault->GetTexture() );
    g_Renderer->DrawVertexArray( lifeVisual );
}

void PlayerCharacter::RenderTankTurrent() const
{
    std::vector<VertexMaster> turrentVisual;
    AppendAABB2( turrentVisual,
                            AABB2::MakeFromAspect( m_TurrentTexture->GetAspectRatio() ),
                            Rgba8::WHITE );

    Vec3 correctedPosition = m_Position;
    correctedPosition += static_cast<Vec3>(m_TurrentVisualOffset.GetRotatedDegrees( m_AngleDegrees + m_TurrentCurrentOffset ));

    TransformVertexArray( turrentVisual, static_cast<Vec2>(correctedPosition), m_AngleDegrees + m_TurrentCurrentOffset, 1.f );
    g_Renderer->BindTexture( m_TurrentTexture );
    g_Renderer->DrawVertexArray( turrentVisual );
}
