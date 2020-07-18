#pragma once

#include "Engine/Core/Math/Primatives/IntVec2.hpp"
#include "Engine/Core/Math/Primatives/AABB2.hpp"

enum TileType : short
{
    TILE_INVALID = -1,

    TILE_GRASS,
    TILE_DIRT,
    TILE_SAND,
    TILE_MUD,

    TILE_FOUNDATION,
    
    TILE_MOSS_STONE,
    TILE_QUAD_WALL,
    TILE_STEEL_BLOCK,
    TILE_WATER,

    TILE_END,

    TILE_FOG,

    NUM_TILE_TYPES,
};

class Texture;

class TileDefinition;

class Tile
{
public:
    Tile( const IntVec2& position);
    Tile( const IntVec2& position, TileType tileType );

    void Create();
    void Destroy();

    //-------------------------------------------------------------------------
    // Tile Querries
    const IntVec2 GetTilePosition() const;
    TileType GetTileType() const;
    const TileDefinition& GetTileDefinition() const;
    const AABB2 GetTileBoundingBox() const;
    bool IsSolid() const;
    bool DoesBlockProjectiles() const;
    bool DoesBlockRaycast() const;
    bool IsSeen() const;
    bool IsTileCurrentSeen() const;

    //-------------------------------------------------------------------------
    void SetTileType(TileType newType );
    void SetTileIsSeen( bool wasSeen );
    void SetTileCurrentSeen( bool newSeen );

private:
    TileType m_TileType;                    // 2B

    bool m_WasSeen = false;                 // 1B
    bool m_IsTileCurrentSeen = false;       // 1B

    IntVec2 m_TilePosition = IntVec2::ZERO; // 8B
};