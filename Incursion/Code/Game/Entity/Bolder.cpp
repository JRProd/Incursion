#include "Bolder.hpp"

#include "Engine/Core/Utils/VectorPcuUtils.hpp"
#include "Engine/Core/Math/Primatives/AABB2.hpp"
#include "Engine/Core/Math/Primatives/IntVec2.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/Sprite/SpriteSheet.hpp"

#include "Game/AssetManagers/TextureManager.hpp"

#include "Engine/Renderer/Mesh/MeshUtils.hpp"

Bolder::Bolder( Game* gameInstance, 
                Map* currentMap, 
                const Vec3& startingPosition )
    : Entity(gameInstance, currentMap, startingPosition, FACTION_NEUTRAL)
{
    m_PhysicsRadius = .4f;
    m_CosmeticRadius = .5f;

    m_EntityType = ENTITY_BOLDER;

    m_OverlapsTiles = true;
    m_IsPushedByWalls = true;
    m_OverlapsEntities = true;
    m_IsFixed = false;
    m_IsPushedByEntities = true;
    m_DoesPushEntities = true;
    m_IsHitByBullets = true;

    m_Sprite = &g_Renderer->CreateOrGetSpriteSheetFromFile( SPRITE_SHEET_EXTRAS, IntVec2( 4, 4 ) )->GetSpriteDefinition(3);
}

void Bolder::Create()
{
}

void Bolder::Render() const
{
    std::vector<VertexMaster> bolderVisual;
    Vec2 uvMin = Vec2::ZERO;
    Vec2 uvMax = Vec2::ZERO;
    m_Sprite->GetUVs( uvMin, uvMax );
    
    AppendAABB2( bolderVisual, 
                            AABB2::MakeFromAspect(m_Sprite->GetAspectRatio()), 
                            Rgba8::WHITE, 
                            uvMin, 
                            uvMax );

    TransformVertexArray( bolderVisual, static_cast<Vec2>(m_Position), m_AngleDegrees, 1.f );
    g_Renderer->BindTexture( GetTexture() );
    g_Renderer->DrawVertexArray( bolderVisual );
}

void Bolder::Die()
{
    m_IsDead = false;
}

void Bolder::Destroy()
{
}
