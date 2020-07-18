#include "Explosion.hpp"

#include "Engine/Core/Utils/VectorPcuUtils.hpp"
#include "Engine/Core/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/Math/Primatives/AABB2.hpp"
#include "Engine/Core/Math/Primatives/IntVec2.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/Sprite/SpriteAnimDefinition.hpp"

#include "Game/Game.hpp"


#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/Mesh/MeshUtils.hpp"

Explosion::Explosion( Game* gameInstance,
                      Map* currentMap,
                      const Vec3& startingPositon,
                      float duration,
                      const Vec3& scale )
    : Entity( gameInstance, currentMap, startingPositon )
    , m_Duration( duration )
{
    m_Scale = scale;

    m_EntityType = ENTITY_EXPLOSION;
    m_EntityFaction = FACTION_NEUTRAL;
}

void Explosion::Create()
{
    m_ExplosionAnimDef = new SpriteAnimDefinition( *g_Renderer->CreateOrGetSpriteSheetFromFile( "Data/Sprites/Explosion5x5.png",
                                                                                                IntVec2( 5, 5 ) ),
                                                   0,
                                                   24,
                                                   m_Duration,
                                                   SpriteAnimPlaybackType::ONCE );

    m_AngleDegrees = m_GameInstance->GetRng()->FloatLessThan( 360.f );

    m_ExplosionAnimStartTime = m_Age;

    m_AddativeShader = new Shader( g_Renderer->CreateOrGetShaderProgramFromFile( "DEFAULT" ) );
    m_AddativeShader->blendMode = BlendMode::ADDITIVE;
}

void Explosion::Update( float deltaSeconds )
{
    float animTime = m_Age - m_ExplosionAnimStartTime;

    if ( m_ExplosionAnimDef->IsComplete( animTime ) )
    {
        m_IsDead = true;
        m_IsGarbage = true;
        return;
    }

    m_Sprite = &m_ExplosionAnimDef->GetSpriteDefAtTime( animTime );

    Entity::Update( deltaSeconds );
}

void Explosion::Render() const
{
    if ( m_Sprite == nullptr ) { return; }
    Vec2 minUv = Vec2::ZERO;
    Vec2 maxUv = Vec2::ONE;
    m_Sprite->GetUVs( minUv, maxUv );

    AABB2 box = AABB2::MakeFromAspect( m_Sprite->GetAspectRatio() );

    std::vector<VertexMaster> visual;
    AppendAABB2( visual, box, Rgba8::WHITE, minUv, maxUv );
    TransformVertexArray( visual, static_cast<Vec2>(m_Position), m_AngleDegrees, static_cast<Vec2>(m_Scale) );


    g_Renderer->BindShader( m_AddativeShader );
    g_Renderer->BindTexture( &m_Sprite->GetTexture() );
    g_Renderer->DrawVertexArray( visual );
    g_Renderer->BindShader( nullptr );
}

void Explosion::Die()
{
}

void Explosion::Destroy()
{
    delete m_ExplosionAnimDef;
    m_ExplosionAnimDef = nullptr;

    delete m_AddativeShader;
    m_AddativeShader = nullptr;
}

