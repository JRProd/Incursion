#pragma once

#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/Rgba8.hpp"

class Camera;
struct Vec3;

#include "Game/GameCommon.hpp"

class World;
class PlayerCharacter;

extern bool g_DebugMode;
extern bool g_FullMapView;
extern bool g_NoClip;
extern bool g_NoFog;

enum class GameState
{
    INVALID,

    LOADING,
    ATTRACT,
    PLAY,
    PAUSE,
    WIN,
};

class Game
{
public:
    Game();
    ~Game();

    void Startup();
    void Update( float deltaSeconds );
    void Render() const;
    void Shutdown();

    void PlayerDied();
    void PlayerRespawn( PlayerCharacter* player);
    void WorldComplete();

    World* GetCurrentWorld() { return m_CurrentWorld; }

    RandomNumberGenerator* GetRng();

private:
    Camera* m_GameCamera = nullptr;
    float m_NumVerticalTilesInView = 9.f;

    Camera* m_UICamera = nullptr;

    RandomNumberGenerator* m_Rng = nullptr;

    World* m_CurrentWorld = nullptr;

    GameState m_CurrentState = GameState::LOADING;
    GameState m_NextState = GameState::LOADING;

    SoundPlaybackID m_AttractMusicPlayback = MISSING_SOUND_ID;
    SoundPlaybackID m_PlayMusicPlayback = MISSING_SOUND_ID;

    float m_GameTime = 0.f;

    float m_WorldCompleteTime = 0.f;
    float m_WinTransition = 0.f;

    bool m_IsPlayerOutOfLives = false;
    float m_PlayerLives = PLAYER_MAX_NUM_LIVES;

    bool m_IsPlayerDead = false;
    float m_DieTime = 0.f;
    float m_DieTransition = 0.f;

    bool m_IsPaused = false;
    bool m_IsSlowMo = false;
    float m_SlowMoPercentage = .1f;
    bool m_IsHyperSpeed = false;
    float m_HyperSpeedPercentage = 4.f;
    float UpdateDeltaSeconds( float deltaSeconds );

    void UpdateCameraToPlayerPosition();

    void HandleUserInputStateExclusive();

    void DebugRender() const;
    void RenderLives() const;
    
    void InitializeGameSystems();
    void InitializeGameAssets();
    //-------------------------------------------------------------------------
    // Game State Updates
    void UpdateLoading( float deltaSeconds );

    Rgba8 m_SelectColor = Rgba8::WHITE;
    void UpdateAttract( float deltaSeconds );
    void UpdatePlay( float deltaSeconds );
    void UpdatePause( float deltaSeconds );
    void UpdateWin( float deltaSeconds );
    
    //-------------------------------------------------------------------------
    // Game State Renders
    void RenderLoading() const;
    void RenderAttract() const;
    void RenderPlay() const;
    void RenderPause() const;
    void RenderWin() const;

    void SetPause( bool newPauseState );
    void ResetState();

    void ErrorRecoverable( const char* message );
};

extern Game* g_GameInstance;
