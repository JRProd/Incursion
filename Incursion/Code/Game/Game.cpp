#include "Game.hpp"

#include "Game/App.hpp"
#include "Game/AssetManagers/AudioManager.hpp"
#include "Game/AssetManagers/TextureManager.hpp"
#include "Game/Entity/Entity.hpp"
#include "Game/Entity/PlayerCharacter.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Map/TileDefinition.hpp"
#include "Game/World.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Utils/VectorPcuUtils.hpp"
#include "Engine/Core/Math/MathUtils.hpp"
#include "Engine/Core/Math/Primatives/Disc.hpp"
#include "Engine/Core/Math/Primatives/IntVec2.hpp"
#include "Engine/Core/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/Fonts/BitmapFont.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/Mesh/MeshUtils.hpp"


#include <winuser.rh>

bool g_DebugMode = false;
bool g_FullMapView = false;
bool g_NoClip = false;
bool g_NoFog = false;

BitmapFont* g_FontDefault = nullptr;

//-----------------------------------------------------------------------------
Game::Game()
{
}

//-----------------------------------------------------------------------------
Game::~Game()
{
}

//-----------------------------------------------------------------------------
void Game::Startup()
{
    g_FontDefault = g_Renderer->CreateOrGetBitmapFontFromFile( "Data/Fonts/SquirrelFixedFont" );
}

//-----------------------------------------------------------------------------
void Game::Shutdown()
{
    if ( m_AttractMusicPlayback != MISSING_SOUND_ID )
    {
        g_AudioSystem->StopSound( m_AttractMusicPlayback );
    }

    if ( m_PlayMusicPlayback != MISSING_SOUND_ID )
    {
        g_AudioSystem->StopSound( m_PlayMusicPlayback );
    }

    m_CurrentWorld->Destory();
    delete m_CurrentWorld;
    m_CurrentWorld = nullptr;

    delete m_GameCamera;
    m_GameCamera = nullptr;

    delete m_UICamera;
    m_UICamera = nullptr;

    delete m_Rng;
    m_Rng = nullptr;
}

void Game::PlayerDied()
{
    m_DieTime = m_GameTime;
    if ( m_PlayerLives <= 0 )
    {
        m_IsPlayerOutOfLives = true;
    }
}

void Game::PlayerRespawn( PlayerCharacter* player )
{
    m_PlayerLives -= 1;

    if ( m_PlayerLives >= 0 && m_DieTransition > .5 )
    {
        m_CurrentWorld->RequestRespawn( player );
    }
}

void Game::WorldComplete()
{
    m_WorldCompleteTime = m_GameTime;
    m_NextState = GameState::WIN;
}

//-----------------------------------------------------------------------------

RandomNumberGenerator* Game::GetRng()
{
    return m_Rng;
}

//-----------------------------------------------------------------------------
void Game::Update( float deltaSeconds )
{
    m_CurrentState = m_NextState;

    deltaSeconds = UpdateDeltaSeconds( deltaSeconds );
    m_GameTime += deltaSeconds;

    HandleUserInputStateExclusive();

    switch ( m_CurrentState )
    {
        case GameState::LOADING:
            UpdateLoading( deltaSeconds );
            break;
        case GameState::ATTRACT:
            UpdateAttract( deltaSeconds );
            break;
        case GameState::PLAY:
            UpdatePlay( deltaSeconds );
            break;
        case GameState::PAUSE:
            UpdatePause( deltaSeconds );
            break;
        case GameState::WIN:
            UpdateWin( deltaSeconds );
            break;
        default:
            ERROR_AND_DIE( "Game is in invalid state" );
            break;
    }

}

//-----------------------------------------------------------------------------
void Game::Render() const
{
    switch ( m_CurrentState )
    {
        case GameState::LOADING:
            RenderLoading();
            break;
        case GameState::ATTRACT:
            RenderAttract();
            break;
        case GameState::PLAY:
            RenderPlay();
            break;
        case GameState::PAUSE:
            RenderPause();
            break;
        case GameState::WIN:
            RenderWin();
            break;
        default:
            ERROR_AND_DIE( "Game is in invalid state" );
            break;
    }

}

float Game::UpdateDeltaSeconds( float deltaSeconds )
{
    if ( m_IsSlowMo ) { deltaSeconds *= m_SlowMoPercentage; }
    if ( m_IsHyperSpeed ) { deltaSeconds *= m_HyperSpeedPercentage; }
    if ( m_IsPaused ) { deltaSeconds = 0; }
    return deltaSeconds;
}

void Game::HandleUserInputStateExclusive()
{
    if ( g_InputSystem->WasKeyJustPressed( F1 ) )
    {
        g_DebugMode = !g_DebugMode;
    }
    if ( g_InputSystem->WasKeyJustPressed( F3 ) ||
         g_InputSystem->GetXboxController( 0 ).IsButtonJustPressed( XBOX_BUTTON_D_UP ) )
    {
        g_NoClip = !g_NoClip;
    }
    if ( g_InputSystem->WasKeyJustPressed( F4 ) ||
         g_InputSystem->GetXboxController( 0 ).IsButtonJustPressed( XBOX_BUTTON_D_DOWN ) )
    {
        g_FullMapView = !g_FullMapView;
    }
    if ( g_InputSystem->WasKeyJustPressed( F6 ) || g_InputSystem->GetXboxController( 0 ).IsButtonJustPressed( XBOX_BUTTON_D_LEFT ) )
    {
        g_NoFog = !g_NoFog;
    }

    if ( g_InputSystem->IsKeyPressed( 'T' ) && !g_InputSystem->IsKeyPressed( 'Y' ) )
    {
        m_IsSlowMo = true;
    }
    else
    {
        m_IsSlowMo = false;
    }

    if ( g_InputSystem->IsKeyPressed( 'Y' ) && !g_InputSystem->IsKeyPressed( 'T' ) ||
         g_InputSystem->GetXboxController( 0 ).IsButtonPressed( XBOX_BUTTON_A ) )
    {
        m_IsHyperSpeed = true;
    }
    else
    {
        m_IsHyperSpeed = false;
    }
}

void Game::UpdateCameraToPlayerPosition()
{
    if ( g_FullMapView )
    {
        IntVec2 mapSize = m_CurrentWorld->GetCurrentMapSize();
        Vec2 fullMapCoords;
        if ( mapSize.x > mapSize.y )
        {
            fullMapCoords = Vec2( static_cast<float>(mapSize.x),
                                  static_cast<float>(mapSize.x) * CLIENT_SLOPE );
        }
        else
        {
            fullMapCoords = Vec2( static_cast<float>(mapSize.y) * CLIENT_ASPECT,
                                  static_cast<float>(mapSize.y) );
        }
        AABB2 fullViewOrtho = AABB2( Vec2( 0.f, 0.f ), fullMapCoords );
        m_GameCamera->SetProjectionOrthographic( fullViewOrtho );
        m_GameCamera->SetCameraPosition( Vec3::ZERO );
    }
    else
    {
        Vec2 playerPos = static_cast<Vec2>(m_CurrentWorld->
                                            GetPlayerCharacter()->
                                            GetPosition());
        IntVec2 mapSize = m_CurrentWorld->GetCurrentMapSize();
        AABB2 normalViewOrtho = AABB2( Vec2( 0.f, 0.f ),
                                       Vec2( m_NumVerticalTilesInView * CLIENT_ASPECT,
                                             m_NumVerticalTilesInView ) );

        m_GameCamera->SetProjectionOrthographic( normalViewOrtho );
        m_GameCamera->SetCameraPosition( static_cast<Vec3>(playerPos - normalViewOrtho.GetPointAtUV( Vec2::ALIGN_CENTERED ) ) );
        m_GameCamera->FitCameraInAABB2( AABB2( Vec2( 0.f, 0.f ),
                                               static_cast<Vec2>(mapSize) ) );
    }
}


void Game::RenderLives() const
{
    std::vector<VertexMaster> liveVisual;
    Vec2 liveStartingPoint = Vec2( 7.5f, MAX_UI_HEIGHT - 7.5f );
    Vec2 liveOffset = Vec2( 10.f, 0.f );
    for ( int liveIndex = 0; liveIndex < m_PlayerLives; liveIndex++ )
    {
        AABB2 liveBox = AABB2::MakeFromCenterBoxAround( liveStartingPoint );
        liveBox *= 10.f;
        AppendAABB2( liveVisual, liveBox, Rgba8::WHITE );

        liveStartingPoint += liveOffset;
    }

    if ( m_PlayerLives > 0 )
    {
        g_Renderer->BindTexture( g_Renderer->CreateOrGetTextureFromFile( "Data/Sprites/FriendlyTank4.png" ) );
        g_Renderer->DrawVertexArray( liveVisual );
    }

}

void Game::InitializeGameSystems()
{
    // Initialize Game Systems
    m_Rng = new RandomNumberGenerator();

    m_GameCamera = new Camera(g_Renderer);
    m_GameCamera->SetProjectionOrthographic( AABB2( Vec2( 0.f, 0.f ),
                                       Vec2( m_NumVerticalTilesInView * CLIENT_ASPECT,
                                             m_NumVerticalTilesInView ) ) );
    m_GameCamera->SetClearMode( CLEAR_COLOR_BIT );
    m_GameCamera->SetColorTarget( g_Renderer->GetBackBuffer() );

    m_UICamera = new Camera(g_Renderer);
    m_UICamera->SetProjectionOrthographic( AABB2( Vec2( 0.f, 0.f ),
                                     Vec2( MAX_UI_WIDTH, MAX_UI_HEIGHT ) ) );
    m_UICamera->SetColorTarget( g_Renderer->GetBackBuffer() );
}

void Game::InitializeGameAssets()
{
    // Initialize Game Resources
    AUDIO_ANTICIPATION = g_AudioSystem->CreateOrGetSound( AUDIO_ANTICIPATION_SRC );
    g_AudioSystem->PlaySound( AUDIO_ANTICIPATION );
    AUDIO_ATTRACT_MUSIC = g_AudioSystem->CreateOrGetSound( AUDIO_ATTRACT_MUSIC_SRC );
    m_AttractMusicPlayback = g_AudioSystem->PlaySound( AUDIO_ATTRACT_MUSIC, true );

    AUDIO_PLAY_MUSIC = g_AudioSystem->CreateOrGetSound( AUDIO_PLAY_MUSIC_SRC );

    AUDIO_PAUSE_ACTIVATE = g_AudioSystem->CreateOrGetSound( AUDIO_PAUSE_ACTIVATE_SRC );
    AUDIO_PAUSE_DEACTIVATE = g_AudioSystem->CreateOrGetSound( AUDIO_PAUSE_DEACTIVATE_SRC );

    AUDIO_PLAYER_SHOOT = g_AudioSystem->CreateOrGetSound( AUDIO_PLAYER_SHOOT_SRC );
    AUDIO_PLAYER_HIT = g_AudioSystem->CreateOrGetSound( AUDIO_PLAYER_HIT_SRC );
    AUDIO_PLAYER_DIED = g_AudioSystem->CreateOrGetSound( AUDIO_PLAYER_DIED_SRC );

    AUDIO_ENEMY_SHOOT = g_AudioSystem->CreateOrGetSound( AUDIO_ENEMY_SHOOT_SRC );
    AUDIO_ENEMY_HIT = g_AudioSystem->CreateOrGetSound( AUDIO_ENEMY_HIT_SRC );
    AUDIO_ENEMY_DIED = g_AudioSystem->CreateOrGetSound( AUDIO_ENEMY_DIED_SRC );

    // Load tile definitions
    TileDefinition::BuildTileDefinitions();

    // Load Textures 
    g_Renderer->CreateOrGetTextureFromFile( SPRITE_PLAYER_TANK_BODY );
    g_Renderer->CreateOrGetTextureFromFile( SPRITE_PLAYER_TANK_TURRENT );
    g_Renderer->CreateOrGetTextureFromFile( SPRITE_PLAYER_TANK_LIVES );
    g_Renderer->CreateOrGetTextureFromFile( SPRITE_ENEMY_TANK );
    g_Renderer->CreateOrGetTextureFromFile( SPRITE_ENEMY_TURRENT_BASE );
    g_Renderer->CreateOrGetTextureFromFile( SPRITE_ENEMY_TURRENT_TOP );

    // Load Sprite Sheets
    g_Renderer->CreateOrGetSpriteSheetFromFile( SPRITE_SHEET_EXTRAS, IntVec2( 4, 4 ) );

    // Initialize World
    m_CurrentWorld = new World( this );
    m_CurrentWorld->Create();
}

//-----------------------------------------------------------------------------
void Game::DebugRender() const
{
    m_CurrentWorld->DebugRender();
}

void Game::UpdateLoading( float deltaSeconds )
{
    InitializeGameSystems();

    UNUSED( deltaSeconds );
    if ( g_App->CurrentFrameCount() == 0 ) { return; }

    InitializeGameAssets();

    m_NextState = GameState::ATTRACT;
}

void Game::UpdateAttract( float deltaSeconds )
{
    UNUSED( deltaSeconds );

    if ( g_InputSystem->WasKeyJustPressed( ESC ) )
    {
        g_EventSystem->Publish( "Shutdown" );
    }

    float sine = .5f * sinf( m_GameTime * 5.f ) + .5f;
    m_SelectColor.SetAlphaAsPercent( sine > .5f ? 1.f : 0.f );

    if ( g_InputSystem->WasKeyJustPressed( SPACE ) ||
         g_InputSystem->GetXboxController( 0 ).IsButtonJustPressed( XBOX_BUTTON_START ) ||
         g_InputSystem->GetXboxController( 0 ).IsButtonJustPressed( XBOX_BUTTON_A ) )
    {
        m_NextState = GameState::PLAY;
        g_AudioSystem->StopSound( m_AttractMusicPlayback );
        m_PlayMusicPlayback = g_AudioSystem->PlaySound( AUDIO_PLAY_MUSIC, true );
    }

}

void Game::UpdatePlay( float deltaSeconds )
{
    const PlayerCharacter* player = m_CurrentWorld->GetPlayerCharacter();

    if ( g_InputSystem->WasKeyJustPressed( 'P' ) && player && !player->IsDead() && !player->IsGarbage() )
    {
        SetPause( true );
        return;
    }

    if ( (g_InputSystem->GetXboxController( 0 ).IsButtonJustPressed( XBOX_BUTTON_START ) ||
           g_InputSystem->GetXboxController( 0 ).IsButtonJustPressed( XBOX_BUTTON_BACK )) &&
         player &&
         !player->IsDead() &&
         !player->IsGarbage() )
    {
        SetPause( true );
        return;
    }

    if ( m_CurrentWorld )
    {
        m_IsPlayerDead = m_CurrentWorld->GetPlayerCharacter()->IsDead();

        if ( m_IsPlayerDead )
        {
            m_DieTransition = ClampZeroToOne( (m_GameTime - m_DieTime) / GAME_DIE_TRANSITION_SECONDS );

            if ( g_InputSystem->WasKeyJustPressed( ESC ) ||
                 g_InputSystem->GetXboxController( 0 ).IsButtonJustPressed( XBOX_BUTTON_BACK ) )
            {
                ResetState();
                return;
            }
        }
        else
        {
            m_DieTransition -= GAME_DIE_REVERSAL_CHANGE_PER_SECOND * deltaSeconds;
            m_DieTransition = ClampZeroToOne( m_DieTransition );
        }

        m_CurrentWorld->Update( deltaSeconds );
        UpdateCameraToPlayerPosition();
    }
}

void Game::UpdatePause( float deltaSeconds )
{
    UNUSED( deltaSeconds );

    if ( g_InputSystem->WasKeyJustPressed( 'P' ) ||
         g_InputSystem->GetXboxController( 0 ).IsButtonJustPressed( XBOX_BUTTON_START ) )
    {
        SetPause( false );
        return;
    }

    if ( g_InputSystem->WasKeyJustPressed( ESC ) ||
         g_InputSystem->GetXboxController( 0 ).IsButtonJustPressed( XBOX_BUTTON_BACK ) )
    {
        ResetState();
        return;
    }
}

void Game::UpdateWin( float deltaSeconds )
{
    UNUSED( deltaSeconds );

    m_WinTransition = ClampZeroToOne( (m_GameTime - m_WorldCompleteTime) / GAME_WIN_TRANSITION_TIME );

    if ( m_WorldCompleteTime + GAME_WIN_TO_ATTRACT_TIME < m_GameTime )
    {
        if ( g_InputSystem->WasKeyJustPressed( 'P' ) ||
             g_InputSystem->WasKeyJustPressed( ESC ) ||
             g_InputSystem->WasKeyJustPressed( SPACE ) ||
             g_InputSystem->GetXboxController( 0 ).IsButtonJustPressed( XBOX_BUTTON_START ) ||
             g_InputSystem->GetXboxController( 0 ).IsButtonJustPressed( XBOX_BUTTON_BACK ) ||
             g_InputSystem->GetXboxController( 0 ).IsButtonJustPressed( XBOX_BUTTON_A ) )
        {
            ResetState();
        }
    }

}

void Game::RenderLoading() const
{
    // Render UI
    g_Renderer->BeginCamera( *m_UICamera );
    std::string title = "Loading...";
    g_FontDefault->Render( *g_Renderer,
                           title,
                           Vec2( MAX_UI_WIDTH * .30f, MAX_UI_HEIGHT * .65f ),
                           15.f,
                           Rgba8::YELLOW,
                           .6f );
    g_Renderer->EndCamera( *m_UICamera );
}

void Game::RenderAttract() const
{
    g_Renderer->BeginCamera( *m_GameCamera );
    g_Renderer->EndCamera( *m_GameCamera );

    // Render UI
    g_Renderer->BeginCamera( *m_UICamera );

    std::string title = "INCURSION";
    g_FontDefault->Render( *g_Renderer,
                           title,
                           Vec2( MAX_UI_WIDTH * .30f, MAX_UI_HEIGHT * .65f ),
                           15.f,
                           Rgba8::YELLOW,
                           .6f );

    std::string select = ">";
    g_FontDefault->Render( *g_Renderer,
                           select,
                           Vec2( MAX_UI_WIDTH * .40f, MAX_UI_HEIGHT * .50f ),
                           6.f,
                           m_SelectColor,
                           .6f );

    std::string start = "Press Start";
    g_FontDefault->Render( *g_Renderer,
                           start,
                           Vec2( MAX_UI_WIDTH * .425f, MAX_UI_HEIGHT * .50f ),
                           6.f,
                           Rgba8::WHITE,
                           .6f );

    std::string howToPlayKeyboard = "How To Play:\n\nWASD: Move Tank\nIJKL:Turn Turret\nSPACE: Shoot\nB: Respawn";
    g_FontDefault->Render( *g_Renderer,
                           howToPlayKeyboard,
                           Vec2( MAX_UI_WIDTH * .70f, MAX_UI_HEIGHT * .40f ),
                           4.f,
                           Rgba8::WHITE,
                           .6f );

    std::string howToPlayJoystick = "How To Play:\n\nLeft Analog: Move Tank\nRight Analog: Turn Turret\nRight Bumper: Shoot\nStart:Respawn";
    g_FontDefault->Render( *g_Renderer,
                           howToPlayJoystick,
                           Vec2( MAX_UI_WIDTH * .10f, MAX_UI_HEIGHT * .40f ),
                           4.f,
                           Rgba8::WHITE,
                           .6f );

    g_Renderer->EndCamera( *m_UICamera );

}

void Game::RenderPlay() const
{
    // Render Game
    g_Renderer->BeginCamera( *m_GameCamera );

    m_CurrentWorld->Render();

    if ( g_DebugMode )
    {
        DebugRender();
    }

    g_Renderer->EndCamera( *m_GameCamera );

    // Render UI
    g_Renderer->BeginCamera( *m_UICamera );
    RenderLives();

    std::vector<VertexMaster> overlay;
    AABB2 overlaySize = AABB2( Vec2::ZERO, Vec2( MAX_UI_WIDTH, MAX_UI_HEIGHT ) );
    Rgba8 transitionColor = Rgba8( 0, 0, 0, static_cast<unsigned char>(100 * m_DieTransition) );

    AppendAABB2( overlay, overlaySize, transitionColor );
    g_Renderer->BindTexture( nullptr );
    g_Renderer->DrawVertexArray( overlay );

    if ( m_DieTransition > .5 )
    {
        if ( !m_IsPlayerOutOfLives )
        {
            std::string died = "You Died!";
            float fontTransition = ClampZeroToOne( (m_DieTransition - .5f) / .5f );
            Rgba8 fontColor = Rgba8( 255, 0, 0, static_cast<unsigned char>(255 * fontTransition) );
            g_FontDefault->Render( *g_Renderer,
                                   died,
                                   Vec2( MAX_UI_WIDTH * .4f, MAX_UI_HEIGHT * .50f ),
                                   10.f,
                                   fontColor,
                                   .6f );
            std::string instructions = "Press Start to Respawn\n  Press Back to Exit";
            g_FontDefault->Render( *g_Renderer,
                                   instructions,
                                   Vec2( MAX_UI_WIDTH * .3f, MAX_UI_HEIGHT * .4f ),
                                   6.f,
                                   fontColor,
                                   .6f );
        }
        else
        {
            std::string died = "Game Over";
            float fontTransition = ClampZeroToOne( (m_DieTransition - .5f) / .5f );
            Rgba8 fontColor = Rgba8( 255, 0, 0, static_cast<unsigned char>(255 * fontTransition) );
            g_FontDefault->Render( *g_Renderer,
                                   died,
                                   Vec2( MAX_UI_WIDTH * .4f, MAX_UI_HEIGHT * .50f ),
                                   10.f,
                                   fontColor,
                                   .6f );

            std::string instructions = "   Press Back to Exit";
            g_FontDefault->Render( *g_Renderer,
                                   instructions,
                                   Vec2( MAX_UI_WIDTH * .3f, MAX_UI_HEIGHT * .4f ),
                                   6.f,
                                   fontColor,
                                   .6f );
        }


    }

    g_Renderer->EndCamera( *m_UICamera );
}

void Game::RenderPause() const
{
    // g_Renderer->ClearColor( Rgba8::BLACK );
    // Render Game
    g_Renderer->BeginCamera( *m_GameCamera );

    m_CurrentWorld->Render();

    if ( g_DebugMode )
    {
        DebugRender();
    }

    g_Renderer->EndCamera( *m_GameCamera );

    // Render UI
    g_Renderer->BeginCamera( *m_UICamera );

    std::vector<VertexMaster> overlay;
    AABB2 overlaySize = AABB2( Vec2::ZERO, Vec2( MAX_UI_WIDTH, MAX_UI_HEIGHT ) );
    AppendAABB2( overlay, overlaySize, Rgba8::DARK_GRAY_75 );
    g_Renderer->BindTexture( nullptr );
    g_Renderer->DrawVertexArray( overlay );

    std::string pause = "Paused";
    g_FontDefault->Render( *g_Renderer,
                           pause,
                           Vec2( MAX_UI_WIDTH * .4f, MAX_UI_HEIGHT * .50f ),
                           10.f,
                           Rgba8::RED,
                           .6f );



    g_Renderer->EndCamera( *m_UICamera );
}

void Game::RenderWin() const
{
    // g_Renderer->ClearColor( Rgba8::BLACK );
    // Render Game
    g_Renderer->BeginCamera( *m_GameCamera );

    m_CurrentWorld->Render();

    g_Renderer->EndCamera( *m_GameCamera );

    // Render UI
    g_Renderer->BeginCamera( *m_UICamera );

    std::vector<VertexMaster> overlay;
    AABB2 overlaySize = AABB2( Vec2::ZERO, Vec2( MAX_UI_WIDTH, MAX_UI_HEIGHT ) );
    Rgba8 transitionColor = Rgba8( 0, 0, 0, static_cast<unsigned char>(255 * m_WinTransition) );
    AppendAABB2( overlay, overlaySize, transitionColor );

    g_Renderer->BindTexture( nullptr );
    g_Renderer->DrawVertexArray( overlay );

    if ( m_WinTransition > .5 )
    {
        std::string died = "Victory";
        float fontTransition = ClampZeroToOne( (m_WinTransition - .5f) / .5f );
        Rgba8 fontColor = Rgba8( 0, 255, 0, static_cast<unsigned char>(255 * fontTransition) );
        g_FontDefault->Render( *g_Renderer,
                               died,
                               Vec2( MAX_UI_WIDTH * .4f, MAX_UI_HEIGHT * .50f ),
                               10.f,
                               fontColor,
                               .6f );
    }

    g_Renderer->EndCamera( *m_UICamera );
}

void Game::SetPause( bool newPauseState )
{
    m_IsPaused = newPauseState;
    m_NextState = m_IsPaused ? GameState::PAUSE : GameState::PLAY;

    if ( m_NextState == GameState::PAUSE )
    {
        m_AttractMusicPlayback = g_AudioSystem->PlaySound( AUDIO_PAUSE_ACTIVATE );
        g_AudioSystem->SetSoundPlaybackSpeed( m_PlayMusicPlayback, 0.f );
    }
    else if ( m_NextState == GameState::PLAY )
    {
        g_AudioSystem->PlaySound( g_AudioSystem->CreateOrGetSound( AUDIO_PAUSE_DEACTIVATE_SRC ) );
        g_AudioSystem->SetSoundPlaybackSpeed( m_PlayMusicPlayback, 1.f );
    }
}

void Game::ResetState()
{
    m_AttractMusicPlayback = g_AudioSystem->PlaySound( AUDIO_ATTRACT_MUSIC );
    g_AudioSystem->StopSound( m_PlayMusicPlayback );

    m_NextState = GameState::ATTRACT;

    m_WorldCompleteTime = 0.f;
    m_WinTransition = 0.f;

    m_IsPlayerOutOfLives = false;
    m_PlayerLives = PLAYER_MAX_NUM_LIVES;

    m_IsPlayerDead = false;
    m_DieTime = 0.f;
    m_DieTransition = 0.f;

    m_IsPaused = false;
    m_IsSlowMo = false;
    m_IsHyperSpeed = false;

    m_CurrentWorld->Destory();
    delete m_CurrentWorld;
    m_CurrentWorld = new World( this );
    m_CurrentWorld->Create();
}

//-----------------------------------------------------------------------------
void Game::ErrorRecoverable( const char* message )
{
    if ( m_IsSlowMo )
    {
        m_IsSlowMo = false;
    }
    ERROR_RECOVERABLE( message );
}
