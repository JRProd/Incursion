#include "App.hpp"

#include "Engine/Core/Time.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/RenderContext.hpp"

#include "Game/Game.hpp"

// Globally defined renderer
#if !defined ENGINE_DISABLE_AUDIO
AudioSystem* g_AudioSystem = nullptr;
#endif
InputSystem* g_InputSystem = nullptr;
RenderContext* g_Renderer = nullptr;
Game* g_GameInstance = nullptr;

//-----------------------------------------------------------------------------
App::App()
{
}

//-----------------------------------------------------------------------------
App::~App()
{
}

//-----------------------------------------------------------------------------
void App::Startup()
{
    // Initialize the Engine
#if !defined ENGINE_DISABLE_AUDIO
    g_AudioSystem = new AudioSystem();
#endif

    g_InputSystem = &InputSystem::INSTANCE();
    g_InputSystem->SetWindow( g_Window );

    g_Renderer = new RenderContext();
    g_Renderer->Startup( g_Window );

    // Initialize the Game
    g_GameInstance = new Game();
    g_GameInstance->Startup();

    g_EventSystem->Subscribe( "WM_CLOSE", this, &App::HandleQuitRequested );
    g_EventSystem->Subscribe( "Shutdown", this, &App::HandleQuitRequested );
}

//-----------------------------------------------------------------------------
void App::Shutdown()
{
    g_GameInstance->Shutdown();
    delete g_GameInstance;
    g_GameInstance = nullptr;

    g_Renderer->Shutdown();
    delete g_Renderer;
    g_Renderer = nullptr;

    g_InputSystem = nullptr;

#if !defined ENGINE_DISABLE_AUDIO
    delete g_AudioSystem;
    g_AudioSystem = nullptr;
#endif
}

//-----------------------------------------------------------------------------
void App::RunFrame()
{
    static double timeLastFrameStarted = GetCurrentTimeSeconds();
    double timeThisFrameStarted = GetCurrentTimeSeconds();
    float deltaSeconds = static_cast<float>(timeThisFrameStarted -
                                             timeLastFrameStarted);
    timeLastFrameStarted = timeThisFrameStarted;

    constexpr float MAX_DELTA_SECONDS = 1.f / 10.f;
    if ( deltaSeconds > MAX_DELTA_SECONDS ) { deltaSeconds = MAX_DELTA_SECONDS; }

    BeginFrame(); // For all engine systems, before game updates
    Update( deltaSeconds );
    Render();
    EndFrame(); // For all engine systems, after the game updates
    
    m_FrameCount += 1;
}

//-----------------------------------------------------------------------------
bool App::HandleQuitRequested( EventArgs* )
{
    m_isQuitting = true;
    return true;
}

//-----------------------------------------------------------------------------
void App::BeginFrame()
{
    g_AudioSystem->BeginFrame();
    g_InputSystem->BeginFrame();
    g_Renderer->BeginFrame();
}

//-----------------------------------------------------------------------------
void App::Update( float deltaSeconds )
{
    HandleUserInput();

    g_GameInstance->Update( deltaSeconds );
}

//-----------------------------------------------------------------------------
void App::Render() const
{
    g_GameInstance->Render();
}

//-----------------------------------------------------------------------------
void App::EndFrame()
{
    g_Renderer->EndFrame();
    g_InputSystem->EndFrame();
    g_AudioSystem->EndFrame();
}

//-----------------------------------------------------------------------------
void App::HandleUserInput()
{
    if ( g_InputSystem->WasKeyJustPressed( F8 ) )
    {
        RestartGame();
    }
}

//-----------------------------------------------------------------------------
void App::RestartGame()
{
    // Delete the game
    g_GameInstance->Shutdown();
    delete g_GameInstance;
    g_GameInstance = nullptr;

    // Recreate the game
    g_GameInstance = new Game();
    g_GameInstance->Startup();
}
