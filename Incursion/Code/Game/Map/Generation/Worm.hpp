#pragma once

#include <vector>

#include "Game/Map/Tile.hpp"

class Worm;

typedef std::vector<Worm*> WormList;

enum WormType
{
    WORM_INVALID,

    WORM_DRUNKEN,

    NUM_WORM_TYPES,
};

class Worm
{
public:
    virtual ~Worm() {}
    explicit Worm( IntVec2 position, TileType type, int length );

    WormType GetWormType() const { return m_WormType; }
    TileType GetTileType() const { return m_TileType; }
    int GetMaxLength() const { return m_MaxLength; }

    virtual IntVec2 NextPosition() = 0;
    void SetCurrentPosition( const IntVec2& newPosition );

protected:
    const WormType m_WormType = WORM_INVALID;
    const TileType m_TileType = TILE_INVALID;
    const int m_MaxLength = 1;
    
    IntVec2 m_CurrentPosition = IntVec2::ZERO;
    int m_CurrentLength = 0;
};