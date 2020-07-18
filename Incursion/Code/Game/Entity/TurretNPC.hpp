#pragma once

#include "Game/Entity/Entity.hpp"
#include "Game/Map/Raycast.hpp"

enum class TurretAIState
{
    SCAN,
    WATCH,
    ATTACK,
};

class TurretNPC: public Entity
{
public:
    TurretNPC( Game* gameInstance, 
               Map* currentMap, 
               const Vec3& startingPosition,
               EntityType type,
               Faction faction );

    virtual void Create() override;
    virtual void Update( float deltaSeconds ) override;
    virtual void Render() const override;
    virtual void Die() override;
    virtual void Destroy() override;

private:
    float m_TargetOrientation = 0.f;
    float m_LastShotCheck = 0.f;

    Texture* m_TurretTexture = nullptr;

    RayCastHit m_RayTraceResult;

    void RenderBase() const;
    void RenderTurrent() const;

    TurretAIState m_TurretState = TurretAIState::SCAN;
    float m_LastSeenAngle = 0.f;
    int m_TurnDirection = 1;

    void TurretAI( float deltaSeconds );
    void ScanBahavior( float deltaSeconds );
    void WatchBehavior( float deltaSeconds );
    void AttackBehavior( float deltaSeconds );

    void ShootBullet();

    bool IsPlayerVisable();
};