#pragma once

#include "Engine/Core/Math/Primatives/Vec2.hpp"

struct RayCastHit
{
public:
    bool didHit = false;
    Vec2 hitPosition = Vec2::ZERO;
    float percentToFinish = 0.f;

    RayCastHit() {}
    explicit RayCastHit( bool washit, const Vec2& hitPos, float finish );
    //Tile* hitTile;
    //Entity* hitEntity;
};