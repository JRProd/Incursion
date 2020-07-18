#pragma once

#include "Game/Entity/Entity.hpp"

class Bolder: public Entity
{
public:
    Bolder( Game* gameInstance, 
            Map* currentMap, 
            const Vec3& startingPosition );

    virtual void Create() override;
    virtual void Render() const override;
    virtual void Die() override;
    virtual void Destroy() override;
};