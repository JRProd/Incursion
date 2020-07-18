#pragma once

#include "Engine/Core/Math/RandomNumberGenerator.hpp"

#include "Game/Map/Generation/Worm.hpp"

class DrunkenWorm: public Worm
{
public:
    ~DrunkenWorm() {}
    explicit DrunkenWorm( IntVec2 position, TileType type, int length );

    virtual IntVec2 NextPosition() override;

private:
    RandomNumberGenerator m_DirectionPicker;
};