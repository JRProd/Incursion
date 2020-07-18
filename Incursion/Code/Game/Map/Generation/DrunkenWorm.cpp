#include "DrunkenWorm.hpp"

DrunkenWorm::DrunkenWorm( IntVec2 position, TileType type, int length )
    : Worm( position, type, length )
    , m_DirectionPicker()
{
}

IntVec2 DrunkenWorm::NextPosition()
{
    m_CurrentLength += 1;
    // If current length was just zero return starting position
    if ( m_CurrentLength == 1 )
    {
        return m_CurrentPosition;
    }

    int d4Role = m_DirectionPicker.IntLessThan( 4 );

    switch ( d4Role )
    {
        case 0:
            return m_CurrentPosition + IntVec2( 1, 0 );
        case 1:
            return m_CurrentPosition + IntVec2( 0, 1 );
        case 2:
            return m_CurrentPosition + IntVec2( -1, 0 );
        case 3:
            return m_CurrentPosition + IntVec2( 0, -1 );
        default:
            return m_CurrentPosition + IntVec2( 1, 0 );
    }
}
