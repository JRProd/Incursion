#pragma once

class App;
class Window;
class AudioSystem;
class InputSystem;
class RenderContext;
class BitmapFont;
class RandomNumberGenerator;

struct Vec2;
struct Rgba8;
//-----------------------------------------------------------------------------
// Global advertisement of the App and Engine systems
extern App* g_App;
extern AudioSystem* g_AudioSystem;
extern InputSystem* g_InputSystem;
extern RenderContext* g_Renderer;
extern Window* g_Window;

//-----------------------------------------------------------------------------
// Global advertisement for fonts
extern BitmapFont* g_FontDefault;

//-----------------------------------------------------------------------------
// Constants for the Game
constexpr float CLIENT_ASPECT = 1.777f;                    // We are requesting a 1:1 aspect (square) window area
constexpr float CLIENT_SLOPE = 1.f / CLIENT_ASPECT;
constexpr float MAX_UI_HEIGHT = 100.f;
constexpr float MAX_UI_WIDTH = MAX_UI_HEIGHT * CLIENT_ASPECT;
constexpr float MAX_SCREEN_SHAKE = 10.f;
constexpr float SCREEN_SHAKE_ABLATION_PER_SECOND = 1.f;
constexpr float CONTROLLER_VIBRATION_ABLATION_PER_SECOND = .5f;

//-----------------------------------------------------------------------------
// Game Rules
constexpr float GAME_DIE_TRANSITION_SECONDS = 2.f;
constexpr float GAME_DIE_REVERSAL_CHANGE_PER_SECOND = 2.f;
constexpr float GAME_WIN_TRANSITION_TIME = .5f;
constexpr float GAME_WIN_TO_ATTRACT_TIME = 1.f;

//-----------------------------------------------------------------------------
// Entity Rules
constexpr float HIT_TIME = .2f;

//-----------------------------------------------------------------------------
// Player Rules
constexpr int PLAYER_MAX_NUM_LIVES = 3;

//-----------------------------------------------------------------------------
// Tank Rules
constexpr float TANK_MAX_VELOCITY = 1.f;
constexpr float TANK_MAX_ROTATION_SECONDS = 180.f;
constexpr float TANK_TURRENT_MAX_ROTATION_SECONDS = 20.f;
constexpr int TANK_PLAYER_HEALTH = 15;
constexpr int TANK_NPC_HEALTH = 5;
constexpr float TANK_NPC_MAX_VELOCITY = .65f;
constexpr float TANK_NPC_FOLLOW_APETURE = 45.f;
constexpr float TANK_NPC_ENGAGE_APETURE = 5.f;
constexpr float TANK_NPC_RELOAD_SPEED = 1.7f;
constexpr float TANK_NPC_VIEW_DISTANCE = 6.f;
constexpr float TANK_NPC_RANDOM_GOAL_TIME = 2.f;
constexpr float TANK_NPC_WISKER_ANGLE = 27.5f;
constexpr float TANK_NPC_WISKER_DIST = 0.8f;

//-----------------------------------------------------------------------------
// Turret Rules
constexpr int TURRET_NPC_HEALTH = 8;
constexpr float TURRET_NPC_MAX_ROTATION_SECONDS_ENGAGED = 60.f;
constexpr float TURRET_NPC_MAX_ROTATION_SECONDS_PATROL = 30.f;
constexpr float TURRET_NPC_VIEW_DISTANCE = 10.f;
constexpr float TURRET_NPC_RELOAD_SPEED = 1.3f;
constexpr float TURRET_NPC_WATCH_APETURE_HALF = 45.f;
constexpr float TURRET_NPC_ENGAGE_APETURE = 5.f;

//-----------------------------------------------------------------------------
// Bullet Rules
constexpr float BULLET_MAX_VELOCITY = 6.f;

//-------------------------------------------------------------------------------
// Debris Rules
constexpr float MAX_DEBRIS_LIFESPAN = 2.f;
constexpr float DEBRIS_COSMETIC_RADIUS = 1.f;
constexpr float DEBRIS_MIN_SPEED = 2.f;
constexpr float DEBRIS_MAX_SPEED = 55.f;

//-----------------------------------------------------------------------------
// Map Rules
constexpr int NUM_MAPS = 3;

extern Rgba8 DEBUG_FORWARD_VECTOR_COLOR;
extern Rgba8 DEBUG_POSITOIN_VECTOR_COLOR;
extern Rgba8 DEBUG_COSMETIC_CIRCLE;
extern Rgba8 DEBUG_PHYSICS_CIRCLE;

//-------------------------------------------------------------------------------
// Gameplay Utility Functions
// Vec2 PointJustOffScreen(float furthestBound);
