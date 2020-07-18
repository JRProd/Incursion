#pragma once
#include "Engine/Event/EventSystem.hpp"

class Game;
class Camera;

class App
{
public:
    App();
    ~App();
    void Startup();
    void Shutdown();
    void RunFrame();

    unsigned int CurrentFrameCount() const { return m_FrameCount; }
    bool IsQuitting() const { return m_isQuitting; }
    bool HandleKeyPressed( unsigned char keyCode );
    bool HandleKeyReleased( unsigned char keyCode );
    bool HandleQuitRequested( EventArgs* );

private:
    bool m_isQuitting = false;
    unsigned int m_FrameCount = 0;

    void BeginFrame();
    void Update( float deltaSeconds );
    void Render() const;
    void EndFrame();

    void HandleUserInput();

    void RestartGame();
};