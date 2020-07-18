#include "Worm.hpp"

Worm::Worm( IntVec2 position, TileType type, int length )
    : m_CurrentPosition(position)
    , m_TileType(type)
    , m_MaxLength(length)
{
}

void Worm::SetCurrentPosition( const IntVec2& newPosition )
{
    m_CurrentPosition = newPosition;
}
