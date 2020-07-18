#pragma once

#include <vector>

#include "Engine/Core/Math/Primatives/IntVec2.hpp"

class Game;
class Map;
class PlayerCharacter;

class World
{
public:
    World( Game* gameInstance );
    ~World() {}

    void Create();
    void Update( float deltaSeconds );
    void Render() const;
    void DebugRender() const;
    void Destory();

    const Map* GetCurrentMap() const { return m_CurrentMap; }

    const PlayerCharacter* GetPlayerCharacter() const;
    const IntVec2 GetCurrentMapSize() const;

    void RequestRespawn( PlayerCharacter* player );
    void MapCompleted();

private:
    Game* m_GameInstance = nullptr;

    std::vector<Map*> m_Maps;
    int m_CurrentMapIndex = 0;
    Map* m_CurrentMap = nullptr;

    PlayerCharacter* m_PlayerCharacter = nullptr;

    void GenerateMaps();
};