#pragma once

#include "Game/Entity/Entity.hpp"
#include "Game/Map/Raycast.hpp"

class PlayerCharacter: public Entity
{
public:
    PlayerCharacter( Game* gameInstance, Map* currentMap, const Vec3& startingPosition );

    virtual void Create() override;
    virtual void Update( float deltaSeconds ) override;
    virtual void Render() const override;
    virtual void Die() override;
    virtual void Destroy() override;

    void RespawnResetStats();
    void TeleportToNewMap( Map* newMap );

private:
    float m_TankTargetRotation = 0.f;

    float m_TurrentCurrentOffset = 0.f;
    float m_TurrentTargetOffset = 0.f;

    void HandleUserInput();
    void HandleGamepadInput();

    void ClampVelocity();

    void ShootBullet();

    void RenderTankBody() const;

    Texture* m_TurrentTexture = nullptr;
    Vec2 m_TurrentBoundingBoxUnits = Vec2( 1.25f, 1.f );
    Vec2 m_TurrentVisualOffset = Vec2( .05f, 0.f );
    void RenderTankTurrent() const;
};