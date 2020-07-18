#define WIN32_LEAN_AND_MEAN		// Always #define this before #including <windows.h>
#include <windows.h>			// #include this (massive, platform-specific) header in very few places


#include "Game/App.hpp"

#include "Engine/Console/Console.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Utils/XmlUtils.hpp"
#include "Engine/Event/EventSystem.hpp"
#include "Engine/OS/Window.hpp"

//-------------------------------------------------------------------------------
// #SD1ToDo: Move each of these items to its proper place, once that place is established
const char* APP_NAME = "Incursion"; 			    // ...becomes ??? (Change this per project!)
const char* g_ConfigFileName = "Data/GameConfig.xml";

//-------------------------------------------------------------------------------
App* g_App = nullptr;
Window* g_Window = nullptr;

//-----------------------------------------------------------------------------
// Handles application startup event like initializing the console, loading
//  configs from GameConfig.xml and creating the OSWindow and RenderContext before
//  the app is created.
void Startup()
{
    //Initialize the event system
    g_EventSystem = new EventSystem();
    g_EventSystem->Startup();

    g_Window = new Window();
    g_Window->Open( APP_NAME, 1.77f );

    // Creates the app and starts it up
    g_App = new App();
    g_App->Startup();
}



//-----------------------------------------------------------------------------
// Closes the app and all remaining engine systems and ensures any final 
//  requirements are satisfied
void Shutdown()
{
    g_App->Shutdown();
    delete g_App;
    g_App = nullptr;

    g_EventSystem->Shutdown();
    delete g_EventSystem;
    g_EventSystem = nullptr;
}

//-------------------------------------------------------------------------------
int WINAPI WinMain( _In_ HINSTANCE applicationInstanceHandle, _In_opt_ HINSTANCE, _In_ LPSTR commandLineString, _In_ int )
{
    UNUSED( commandLineString );
    UNUSED( applicationInstanceHandle );

    Startup();

    // Program main loop; keep running frames until it's time to quit
    while( !g_App->IsQuitting() )
    {
        g_Window->BeginFrame();
        g_App->RunFrame();
    }

    Shutdown();
    return 0;
}


