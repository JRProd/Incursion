#pragma once

#include "Game/Entity/Entity.hpp"

enum class TankAIState
{
    WANDER,
    PURSUE,
    ATTACK,
};

class TankNPC: public Entity
{
public:
    TankNPC( Game* gameInstance,
             Map* currentMap,
             const Vec3& startingPosition,
             EntityType type,
             Faction faction );

    virtual void Create() override;
    virtual void Update( float deltaSeconds ) override;
    virtual void Render() const override;
    virtual void DebugRender() const override;
    virtual void Die() override;
    virtual void Destroy() override;

private:
    float m_TargetOrientation = 0.f;

    float m_LastGoalCheck = 0.f;
    float m_LastShotCheck = 0.f;

    TankAIState m_TankState = TankAIState::WANDER;
    Vec2 m_LastSeenPosition = Vec2::ZERO;

    void TankAI( float deltaSeconds );

    void WanderBehavior( float deltaSeconds );
    void Navigate( float deltaSeconds );
    void PursueBehavior( float deltaSeconds );
    void AttackBehavior( float deltaSeconds );

    void ShootBullet();

    bool IsPlayerVisable();

    float NewTargetOrientation();
};