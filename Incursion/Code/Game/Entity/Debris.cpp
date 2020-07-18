#include "Debris.hpp"

#include "Engine/Core/Math/MathUtils.hpp"
#include "Engine/Core/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/RenderContext.hpp"

#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"

Debris::~Debris()
{
}

void Debris::Create()
{
    // Define the outer vertex based on n number of equal degree triangles
    float degreesPerCorner = 360.f / ( float) m_TriangleCount;
    float length = g_GameInstance->GetRng()->FloatInRange( m_InnerRadius,
                                                   m_OuterRadius );
    // First has angle of zero
    Vec2 first = Vec2::MakeFromPolarDegrees( 0.f, length );
    Vec2 prev = first;

    for ( int cornerIndex = 0; cornerIndex < m_TriangleCount; ++cornerIndex )
    {
        // First vertex is always (0, 0)
        m_Visual.emplace_back( Vec2::ZERO, m_DebrisColor, Vec2( .5f, .5f ) );

        float currentDegree = degreesPerCorner * (cornerIndex + 1);
        length = g_GameInstance->GetRng()->FloatInRange( m_InnerRadius, m_OuterRadius );

        Vec2 current = Vec2::ZERO;
        Vec2 uvMap = Vec2::ZERO;
        if ( cornerIndex == m_TriangleCount - 1 )
        {
            current = Vec2::MakeFromPolarDegrees( currentDegree, length );

            uvMap = Vec2( RangeMapFloat( 0.f, m_OuterRadius, .5f, 1.f, current.x ),
                          RangeMapFloat( 0.f, m_OuterRadius, .5f, 1.f, current.y ) );
            m_Visual.emplace_back( current, m_DebrisColor, uvMap );

            uvMap = Vec2( RangeMapFloat( 0.f, m_OuterRadius, .5f, 1.f, first.x ),
                          RangeMapFloat( 0.f, m_OuterRadius, .5f, 1.f, first.y ) );
            m_Visual.emplace_back( first, m_DebrisColor, uvMap );
        }
        else
        {
            uvMap = Vec2( RangeMapFloat( 0.f, m_OuterRadius, .5f, 1.f, prev.x ),
                          RangeMapFloat( 0.f, m_OuterRadius, .5f, 1.f, prev.y ) );
            m_Visual.emplace_back( prev, m_DebrisColor, uvMap );

            current = Vec2::MakeFromPolarDegrees( currentDegree, length );

            uvMap = Vec2( RangeMapFloat( 0.f, m_OuterRadius, .5f, 1.f, current.x ),
                          RangeMapFloat( 0.f, m_OuterRadius, .5f, 1.f, current.y ) );
            m_Visual.emplace_back( current, m_DebrisColor, uvMap );
        }

        prev = current;
    }
}

void Debris::Update( float deltaSeconds )
{
    if ( m_Age > m_LifeSpan )
    {
        Die();
    }

    // Update the color of the debris
    if ( m_FadeOut )
    {
        float alphaModifier = RangeMapFloat( 0.f, m_LifeSpan, .5f, 0.f, m_Age );
        m_DebrisColor.SetAlphaAsPercent( alphaModifier );
    }

    Entity::Update( deltaSeconds );
}

void ChangeVertexArray( std::vector<VertexMaster>& visual, const Rgba8& color )
{
    for( VertexMaster& vertex : visual )
    {
        vertex.color = color;
    }
}

void Debris::Render() const
{
    std::vector<VertexMaster> visualCopy( m_Visual );
    ChangeVertexArray( visualCopy, m_DebrisColor );

    TransformVertexArray( visualCopy, static_cast<Vec2>(m_Position), m_AngleDegrees, static_cast<Vec2>(m_Scale) );

    g_Renderer->BindTexture( m_Texture );
    g_Renderer->DrawVertexArray( visualCopy );
}

void Debris::Die()
{
    m_IsDead = true;
    m_IsGarbage = true;
}

void Debris::Destroy()
{
}

Debris::Debris( Game* gameInstance, 
                Map* currentMap, 
                const Vec3& startingPosition,
                const Vec3& initialVelocity,
                float angularVelocity,
                float scale,
                float lifeSpan,
                const Rgba8& color,
                bool fadeOut,
                Texture* texture,
                int triangeCount,
                float innerRadius,
                float outerRadius )
    :Entity(gameInstance, currentMap, startingPosition )
    , m_LifeSpan( lifeSpan )
    , m_FadeOut( fadeOut )
    , m_DebrisColor( color )
    , m_InnerRadius( innerRadius )
    , m_OuterRadius( outerRadius )
{
    m_Velocity = initialVelocity;
    m_AngularVelocity = angularVelocity;

    SetUniformScale(scale);

    m_Texture = texture;

    m_TriangleCount = triangeCount;

    m_EntityType = ENTITY_DEBRIS;
}

DebrisType DebrisType::UniformDebrisExplosion( Game* gameInstance, 
                                               Map* currentMap, 
                                               const Vec3& centerPointSpawn, 
                                               float initialSpeed )
{
    DebrisType uniformDebris = DebrisType( gameInstance, 
                                           currentMap, 
                                           centerPointSpawn );
    uniformDebris.m_InitialSpeed = initialSpeed;
    uniformDebris.m_LowerAngularDisplacement = 0.f;
    uniformDebris.m_UpperAngularDisplacement = 360.f;

    return uniformDebris;
}

DebrisType DebrisType::ShotgunWithVelocity( Game* gameInstance, 
                                            Map* currentMap, 
                                            const Vec3& centerPointSpawn, 
                                            float initialSpeed, 
                                            const Vec3& direction, 
                                            float angularDistance )
{
    DebrisType shotgunWith = DebrisType( gameInstance, currentMap, centerPointSpawn );
    shotgunWith.m_InitialSpeed = initialSpeed;
    shotgunWith.m_InitialVelocityDirection = direction.GetNormalized();
    shotgunWith.m_LowerAngularDisplacement = -angularDistance;
    shotgunWith.m_UpperAngularDisplacement = angularDistance;

    return shotgunWith;
}

DebrisType::DebrisType( Game* gameInstance, 
                        Map* currentMap, 
                        const Vec3& centerPointSpawn )
    : m_GameInstance(gameInstance)
    , m_CurrentMap(currentMap)
    , m_CenterPointSpawn( centerPointSpawn )
{
}

DebrisType& DebrisType::SetCircularSpawnRadius( float circularRadius )
{
    m_CircleRadius = circularRadius;
    return *this;
}

DebrisType& DebrisType::SetInitialSpeed( float initialSpeed )
{
    m_InitialSpeed = initialSpeed;
    return *this;
}

DebrisType& DebrisType::SetSpeedRangeChange( float lowerRange, float upperRange )
{
    m_LowerInitialSpeedRange = lowerRange;
    m_UpperInitialSpeedRange = upperRange;
    return *this;
}

DebrisType& DebrisType::SetVelocityDirection( float initialDirection )
{
    m_InitialVelocityDirection = Vec3::MakeFromPolarDegreesXY( initialDirection );
    return *this;
}

DebrisType& DebrisType::SetInitialScale( float initialScale )
{
    m_InitialScale = initialScale;
    return *this;
}

DebrisType& DebrisType::SetScaleRangeChange( float lowerScaleRange, float upperScaleRange )
{
    m_LowerScaleRange = lowerScaleRange;
    m_UpperScaleRange = upperScaleRange;
    return *this;
}

DebrisType& DebrisType::SetVelocityDirectionRangeChange( float lowerAngularDisplacement, float upperAngularDisplacement )
{
    m_LowerAngularDisplacement = lowerAngularDisplacement;
    m_UpperAngularDisplacement = upperAngularDisplacement;
    return *this;
}

DebrisType& DebrisType::SetInitialAngularVelocity( float angularVelocity )
{
    m_InitialAngularVelocity = angularVelocity;
    return *this;
}

DebrisType& DebrisType::SetAngularVelocityRangeChange( float lowerAngularVelocity, float upperAngularVelocity )
{
    m_LowerAngularVelocity = lowerAngularVelocity;
    m_UpperAngularVelocity = upperAngularVelocity;
    return *this;
}

DebrisType& DebrisType::SetLifeSpan( float lifeSpan )
{
    m_LifeSpan = lifeSpan;
    return *this;
}

DebrisType& DebrisType::SetInitialColor( const Rgba8& color )
{
    m_InitialColor = color;
    return *this;
}

DebrisType& DebrisType::SetFadeOut( bool fadeOut )
{
    m_FadeOut = fadeOut;
    return *this;
}

DebrisType& DebrisType::SetTexture( Texture* texture )
{
    m_Texture = texture;
    return *this;
}

DebrisType& DebrisType::SetTriangeCount( int triangleCount )
{
    m_TriangleCount = triangleCount;
    return *this;
}

DebrisType& DebrisType::SetTriangleRadiusRange( float innerRadius, float outerRadius )
{
    m_InnerRadius = innerRadius;
    m_OuterRadius = outerRadius;
    return *this;
}

Debris* DebrisType::BuildDebris()
{
    RandomNumberGenerator* rng = m_GameInstance->GetRng();
    Vec3 spawnPosition = m_CenterPointSpawn + Vec3::MakeFromPolarDegreesXY(
        rng->FloatLessThan( 360.f ),
        m_CircleRadius
    );

    Vec3 spawnVelocity = m_InitialVelocityDirection.GetRotatedAboutZDegrees(
        rng->FloatInRange( m_LowerAngularDisplacement, m_UpperAngularDisplacement ) );
    spawnVelocity.SetLength( m_InitialSpeed + rng->FloatInRange( m_LowerInitialSpeedRange,
                                                                 m_UpperInitialSpeedRange )
    );

    float spawnAngularVelocity = m_InitialAngularVelocity + rng->FloatInRange( m_LowerAngularVelocity,
                                                                               m_UpperAngularVelocity
    );

    float spawnScale = m_InitialScale + rng->FloatInRange( m_LowerScaleRange,
                                                           m_UpperScaleRange
    );

    Debris* newDebris = new Debris( m_GameInstance,
                                    m_CurrentMap,
                                    spawnPosition,
                                    spawnVelocity,
                                    spawnAngularVelocity,
                                    spawnScale,
                                    m_LifeSpan,
                                    m_InitialColor,
                                    m_FadeOut,
                                    m_Texture,
                                    m_TriangleCount,
                                    m_InnerRadius,
                                    m_OuterRadius );
    newDebris->Create();

    return newDebris;
}
