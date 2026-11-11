#include "app.h"
#include <iostream>
// #include "RendererTest.h"
#include "Critter.h"
#include "CritterFactory.h"
#include "Tower.h"
#include "DirectDammageTower.h"
#include "AoeTower.h"
#include "SlowTower.h"
#include "Map.h"

#include <memory>

#ifdef main
#undef main
#endif // main

#ifdef _WIN32
#include "windows.h"
#define _CRTDBG_MAP_ALLOC // to get more details
#include <stdlib.h>
#include <crtdbg.h> //for malloc and free
#endif              // _WIN32

/// Program entry point: constructs the Application and runs its main loop.
int main()
{
#ifdef _WIN32
    _CrtMemState sOld;
    _CrtMemState sNew;
    _CrtMemState sDiff;
    _CrtMemCheckpoint(&sOld); // take a snapshot
#endif                        // _WIN32

    App::Application *app = new App::Application();
    app->mainLoop();
    delete app;

#ifdef _WIN32
    _CrtMemCheckpoint(&sNew);                    // take a snapshot
    if (_CrtMemDifference(&sDiff, &sOld, &sNew)) // if there is a difference
    {
        OutputDebugString("-----------_CrtMemDumpStatistics ---------");
        _CrtMemDumpStatistics(&sDiff);
        OutputDebugString("-----------_CrtMemDumpAllObjectsSince ---------");
        _CrtMemDumpAllObjectsSince(&sOld);
        OutputDebugString("-----------_CrtDumpMemoryLeaks ---------");
        _CrtDumpMemoryLeaks();
    }
#endif // _WIN32

    return 0;
}