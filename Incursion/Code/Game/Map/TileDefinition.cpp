#include "TileDefinition.hpp"

#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/Sprite/SpriteSheet.hpp"

#include "Game/GameCommon.hpp"
#include "Game/AssetManagers/TextureManager.hpp"

void TileDefinition::BuildTileDefinitions()
{
    s_SpriteSheet = g_Renderer->CreateOrGetSpriteSheetFromFile( SPRITE_SHEET_TERRAIN,
                                                                IntVec2( 8, 8 ) );

    BuildDefinition( TILE_GRASS, Rgba8::WHITE, 0 );
    BuildDefinition( TILE_DIRT, Rgba8::TAN, 12);
    BuildDefinition( TILE_SAND, Rgba8::WHITE, 15 );
    BuildDefinition( TILE_MUD, Rgba8::WHITE, 21, .5f );

    BuildDefinition( TILE_FOUNDATION, Rgba8::GRAY, 28 );

    BuildDefinition( TILE_MOSS_STONE, Rgba8::WHITE, 45, 1.f, true, true, true );
    BuildDefinition( TILE_QUAD_WALL, Rgba8::WHITE, 51, 1.f, true, true, true );
    BuildDefinition( TILE_STEEL_BLOCK, Rgba8::WHITE, 53, 1.f, true, true, true );
    BuildDefinition( TILE_WATER, Rgba8::WHITE, 61, 1.f, true, false, false );

    BuildDefinition( TILE_END, Rgba8::WHITE, 57 );

    BuildDefinition( TILE_FOG, Rgba8::DARK_GRAY, 16 );
}

TileDefinition& TileDefinition::operator=( const TileDefinition& other )
{
    m_TileType = other.m_TileType;
    m_Tint = other.m_Tint;
    m_SpriteIndex = other.m_SpriteIndex;
    m_VelocityModifier = other.m_VelocityModifier;
    m_IsSolid = other.m_IsSolid;
    return *this;
}

void TileDefinition::BuildDefinition( TileType tileType,
                                      Rgba8& tint,
                                      int spriteIndex,
                                      float velocityModifier,
                                      bool isSolid,
                                      bool doesBlockProjectiles,
                                      bool blocksRaycast )
{
    TileDefinition& def = DEFINITIONS[ tileType ];
    def.m_TileType = tileType;
    def.m_Tint = tint;
    def.m_SpriteIndex = spriteIndex;
    def.m_VelocityModifier = velocityModifier;
    def.m_IsSolid = isSolid;
    def.m_BlockProjectiles = doesBlockProjectiles;
    def.m_BlocksRaycast = blocksRaycast;
}

TileDefinition::TileDefinition( TileType type,
                                Rgba8& tint,
                                int spriteIndex,
                                float velocityModifier,
                                bool isSolid,
                                bool doesBlockProjectiles,
                                bool blocksRaycast )
    : m_TileType( type )
    , m_Tint( tint )
    , m_SpriteIndex( spriteIndex )
    , m_VelocityModifier( velocityModifier )
    , m_IsSolid( isSolid )
    , m_BlockProjectiles( doesBlockProjectiles )
    , m_BlocksRaycast( blocksRaycast )
{
}

std::vector<TileDefinition> TileDefinition::DEFINITIONS( NUM_TILE_TYPES );
SpriteSheet* TileDefinition::s_SpriteSheet = nullptr;