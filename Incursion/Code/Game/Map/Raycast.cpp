#include "Raycast.hpp"

RayCastHit::RayCastHit( bool wasHit, const Vec2& hitPos, float finish )
    : didHit( wasHit )
    , hitPosition( hitPos )
    , percentToFinish( finish )
{
}