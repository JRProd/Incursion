#pragma once

#include <vector>

#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/Sprite/SpriteDefinition.hpp"

#include "Game/Map/Tile.hpp"

class TileDefinition
{
public:
    static std::vector<TileDefinition> DEFINITIONS;
    static SpriteSheet* s_SpriteSheet;
    static void BuildTileDefinitions();

    const SpriteSheet* GetSpriteSheet() const { return s_SpriteSheet; }
    TileType GetTileType() const { return m_TileType; }
    int GetTileSpriteIndex() const { return m_SpriteIndex; }
    const Rgba8& GetTileTint() const { return m_Tint; }

    float GetVelocityModifier() const { return m_VelocityModifier; }

    bool IsTileSolid() const { return m_IsSolid; }
    bool DoesBlockProjectiles() const { return m_BlockProjectiles; }
    bool DoesBlockRaycast() const { return m_BlocksRaycast; }

    TileDefinition& operator=( const TileDefinition& other );

    TileDefinition() {}

private:
    static void BuildDefinition( TileType tileType,
                                 Rgba8& tint,
                                 int spriteIndex,
                                 float velocityModifier = 1.f,
                                 bool isSolid = false,
                                 bool doesBlockProjectiles = false,
                                 bool blocksRaycast = false);

    TileDefinition( TileType tileType, 
                    Rgba8& tint,
                    int spriteIndex,
                    float velocityModifier,
                    bool isSolid,
                    bool doesBlockProjectiles,
                    bool blocksRaycast );
    
    TileType m_TileType = TILE_INVALID;
    Rgba8 m_Tint = Rgba8::WHITE;
    int m_SpriteIndex = -1;

    float m_VelocityModifier = 1.f;

    bool m_IsSolid = false;
    bool m_BlockProjectiles = false;
    bool m_BlocksRaycast = false;
};