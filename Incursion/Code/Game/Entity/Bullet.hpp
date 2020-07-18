#pragma once

#include "Game/Entity/Entity.hpp"

class Bullet: public Entity
{
public:
    Bullet( Game* gameInstance, 
            Map* currentMap, 
            const Vec3& startingPosition,
            EntityType type,
            Faction faction);

    virtual void Create() override;
    virtual void Update( float deltaSeconds ) override;
    virtual void Render() const override;
    virtual void Die() override;
    virtual void Destroy() override;

    void SetBulletDireciton( const Vec2& forward );
};