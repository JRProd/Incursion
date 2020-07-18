#pragma once


#include <vector>

#include "Game/Entity/Entity.hpp"
#include "Engine/Core/Rgba8.hpp"

struct VertexMaster;

class Debris: public Entity
{
    friend class DebrisType;

public:
    ~Debris();

    virtual void Create() override;
    virtual void Update( float deltaSeconds ) override;
    virtual void Render() const override;
    virtual void Die() override;
    virtual void Destroy() override;

private:
    std::vector<VertexMaster> m_Visual;

    float m_LifeSpan = 1.f;
    bool m_FadeOut = true;

    int m_TriangleCount = 4;
    float m_InnerRadius = .5f;
    float m_OuterRadius = 1.f;

    Rgba8 m_DebrisColor = Rgba8::MAGENTA;

    Debris( Game* gameInstance, 
            Map* currentMap, const Vec3& startingPosition,
            const Vec3& initialVelocity,
            float angularVelocity,
            float scale,
            float lifeSpan,
            const Rgba8& color,
            bool fadeOut,
            Texture* texture,
            int triangleCount,
            float innerRadius,
            float outerRadius) ;

};

class DebrisType
{
public:
    static DebrisType UniformDebrisExplosion( Game* gameInstance, 
                                              Map* currentMap, 
                                              const Vec3& centerPointSpawn, 
                                              float initialSpeed);
    static DebrisType ShotgunWithVelocity( Game* gameInstance,
                                           Map* currentMap,
                                           const Vec3& centerPointSpawn, 
                                           float initialSpeed, 
                                           const Vec3& direction, 
                                           float angularDistance);

    DebrisType( Game* gameInstance, Map* currentMap, const Vec3& centerPointSpawn );

    DebrisType& SetCircularSpawnRadius( float circularRadius );
    DebrisType& SetInitialSpeed( float initialSpeed );
    DebrisType& SetSpeedRangeChange( float lowerRange, float upperRange );
    DebrisType& SetVelocityDirection( float initialDirection );
    DebrisType& SetInitialScale( float initialScale );
    DebrisType& SetScaleRangeChange( float lowerScaleRange, float upperScaleRange );
    DebrisType& SetVelocityDirectionRangeChange( float lowerAngularDisplacement, float upperAngularDisplacement );
    DebrisType& SetInitialAngularVelocity( float angularVelocity );
    DebrisType& SetAngularVelocityRangeChange( float lowerAngularVelocity, float upperAngularVelocity );
    DebrisType& SetLifeSpan( float lifeSpan );
    DebrisType& SetInitialColor( const Rgba8& color );
    DebrisType& SetFadeOut( bool fadeOut );
    DebrisType& SetTexture( Texture* texture );
    DebrisType& SetTriangeCount( int triangleCount );
    DebrisType& SetTriangleRadiusRange( float innerRadius, float outerRadius );
    Debris* BuildDebris();
private:
    Game* m_GameInstance = nullptr;
    Map* m_CurrentMap = nullptr;
    Vec3 m_CenterPointSpawn = Vec3( 0.f, 0.f, 0.f );

    float m_CircleRadius = 0.f;
    float m_InitialSpeed = 0.f;
    Vec3 m_InitialVelocityDirection = Vec3( 1.f, 0.f, 0.f );
    float m_InitialScale = 1.f;
    float m_LowerScaleRange = 0.f;
    float m_UpperScaleRange = 0.f;
    float m_LowerInitialSpeedRange = 0.f;
    float m_UpperInitialSpeedRange = 0.f;
    float m_InitialAngularVelocity = 0.f;
    float m_LowerAngularDisplacement = 0.f;
    float m_UpperAngularDisplacement = 0.f;
    float m_LowerAngularVelocity = -50.f;
    float m_UpperAngularVelocity = 50.f;
    float m_LifeSpan = 0.f;
    Rgba8 m_InitialColor = Rgba8::WHITE;
    bool m_FadeOut = true;
    Texture* m_Texture = nullptr;
    int m_TriangleCount = 5;
    float m_InnerRadius = .5f;
    float m_OuterRadius = 1.f;
};