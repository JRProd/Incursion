#include "Tile.hpp"

#include "Engine/Renderer/RenderContext.hpp"

#include "Game/GameCommon.hpp"

#include "Game/Map/TileDefinition.hpp"

Tile::Tile( const IntVec2& position )
    : m_TilePosition( position )
    , m_TileType( TileType::TILE_GRASS )
{
}

Tile::Tile( const IntVec2& position, TileType tileType )
    : m_TilePosition( position )
    , m_TileType( tileType )
{
}

void Tile::Create()
{
}

void Tile::Destroy()
{
}

const IntVec2 Tile::GetTilePosition() const
{
    return m_TilePosition;
}

TileType Tile::GetTileType() const
{
    return m_TileType;
}

const TileDefinition& Tile::GetTileDefinition() const
{
    return TileDefinition::DEFINITIONS[ m_TileType ];
}

const AABB2 Tile::GetTileBoundingBox() const
{
    AABB2 boundingBox = AABB2::UNIT_BOX;
    boundingBox.Translate( static_cast<Vec2>(m_TilePosition ));
    return boundingBox;
}

bool Tile::IsSolid() const
{
    return TileDefinition::DEFINITIONS[ m_TileType ].IsTileSolid();
}

bool Tile::DoesBlockProjectiles() const
{
    return TileDefinition::DEFINITIONS[ m_TileType ].DoesBlockProjectiles();
}

bool Tile::DoesBlockRaycast() const
{
    return TileDefinition::DEFINITIONS[ m_TileType ].DoesBlockRaycast();
}

bool Tile::IsSeen() const
{
    return m_WasSeen;
}

bool Tile::IsTileCurrentSeen() const
{
    return m_IsTileCurrentSeen;
}

void Tile::SetTileType( TileType newType )
{
    m_TileType = newType;
}

void Tile::SetTileIsSeen( bool wasSeen )
{
    m_WasSeen = wasSeen;
}

void Tile::SetTileCurrentSeen( bool newSeen )
{
    m_IsTileCurrentSeen = newSeen;
}
