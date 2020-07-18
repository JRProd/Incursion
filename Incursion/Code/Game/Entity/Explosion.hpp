#pragma once

#include "Game/Entity/Entity.hpp"

#include "Engine/Renderer/Shaders/Shader.hpp"

class SpriteAnimDefinition;

class Explosion: public Entity
{
public:
    Explosion( Game* gameInstance,
               Map* currentMap,
               const Vec3& startingPositon,
               float duration,
               const Vec3& size);

    virtual void Create() override;
    virtual void Update( float deltaSeconds ) override;
    virtual void Render() const override;
    virtual void Die() override;
    virtual void Destroy() override;

private:
    SpriteAnimDefinition* m_ExplosionAnimDef = nullptr;
    SpriteDefinition* m_SpriteDefinition = nullptr;
    float m_Duration = 1.f;
    float m_ExplosionAnimStartTime = 0.f;

    Shader* m_AddativeShader = nullptr;

};