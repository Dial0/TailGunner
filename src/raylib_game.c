/*******************************************************************************************
*
*   raylib 9years gamejam template
*
*   Template originally created with raylib 4.5-dev, last time updated with raylib 4.5-dev
*
*   Template licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2022 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"
#include "raymath.h"

#if defined(PLATFORM_WEB)
    #define CUSTOM_MODAL_DIALOGS            // Force custom modal dialogs usage
    #include <emscripten/emscripten.h>      // Emscripten library - LLVM to JavaScript compiler
#endif

#include <stdio.h>                          // Required for: printf()
#include <stdlib.h>                         // Required for: 
#include <string.h>                         // Required for: 

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
// Simple log system to avoid printf() calls if required
// NOTE: Avoiding those calls, also avoids const strings memory usage
#define SUPPORT_LOG_INFO
#if defined(SUPPORT_LOG_INFO)
    #define LOG(...) printf(__VA_ARGS__)
#else
    #define LOG(...)
#endif

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef enum { 
    SCREEN_LOGO = 0, 
    SCREEN_TITLE, 
    SCREEN_GAMEPLAY, 
    SCREEN_ENDING
} GameScreen;

// TODO: Define your custom data types here

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------


static Vector2 shipPos;
static Texture2D shipTex;
static Texture2D effects;

// TODO: Define global variables here, recommended to make them static

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
static void UpdateDrawFrame(void);      // Update and Draw one frame

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
#if !defined(_DEBUG)
    SetTraceLogLevel(LOG_NONE);         // Disable raylib trace log messsages
#endif
    static const int screenWidth = 1024;
    static const int screenHeight = 768;
    // Initialization
    //--------------------------------------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "raylib 9yr gamejam");
    shipPos = (Vector2){ screenWidth / 2,screenHeight / 2 };
    shipTex = LoadTexture("resources/ship.png");
    effects = LoadTexture("resources/effects.png");
#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
    SetTargetFPS(60);     // Set our game frames-per-second
    //--------------------------------------------------------------------------------------
    
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button
    {
        UpdateDrawFrame();
    }
#endif

    // De-Initialization
    //--------------------------------------------------------------------------------------
    
    // TODO: Unload all loaded resources at this point

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//--------------------------------------------------------------------------------------------
// Module functions definition
//--------------------------------------------------------------------------------------------
// Update and draw frame

void DrawShip(Vector2 pos, Vector2 tar) {

    int triangleSize = 20;

    Vector2 dir = Vector2Normalize(Vector2Subtract(tar, pos));

    Vector2 front = Vector2Add((Vector2){pos.x,pos.y}, Vector2Scale(dir, (float)triangleSize));
    Vector2 back = Vector2Subtract((Vector2) { pos.x, pos.y }, Vector2Scale(dir, (float)triangleSize));

    Vector2 leftNormal = Vector2Normalize((Vector2) { dir.y,-dir.x });
    Vector2 rightNormal = Vector2Normalize((Vector2) { -dir.y,dir.x });

    Vector2 leftPoint = Vector2Add(back,Vector2Scale(leftNormal, (float)triangleSize));
    Vector2 rightPoint = Vector2Add(back, Vector2Scale(rightNormal, (float)triangleSize));

    DrawTriangle(rightPoint, front, leftPoint, RED);
}

void UpdateDrawFrame(void)
{

    Vector2 dir = Vector2Normalize(Vector2Subtract((Vector2) { GetMouseX(), GetMouseY() }, shipPos));
    Vector2 leftNormal = Vector2Normalize((Vector2) { dir.y, -dir.x });
    Vector2 rightNormal = Vector2Normalize((Vector2) { -dir.y, dir.x });

    float velocity = 4.0f;

    int thrusters = 0b0000;

    if (IsKeyDown(KEY_W)) {
        shipPos = Vector2Add(shipPos, Vector2Scale(dir, velocity));
        thrusters |= 0b1000;
    }
    if (IsKeyDown(KEY_S)) {
        shipPos = Vector2Add(shipPos, Vector2Scale(dir, -velocity));
        thrusters |= 0b0100;
    }
    if (IsKeyDown(KEY_A)) {
        shipPos = Vector2Add(shipPos, Vector2Scale(leftNormal, velocity));
        thrusters |= 0b0010;
    }
    if (IsKeyDown(KEY_D)) {
        shipPos = Vector2Add(shipPos, Vector2Scale(rightNormal, velocity));
        thrusters |= 0b0001;
    }
    
    BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawFPS(0, 0);
        // Draw render texture to screen scaled as required
        // Draw equivalent mouse position on the target render-texture
        DrawCircleLines(GetMouseX(), GetMouseY(), 10, MAROON);

        //DrawShip(shipPos, (Vector2) { GetMouseX(), GetMouseY() });
        float rotShip = Vector2Angle((Vector2) { 0,-1 }, dir);
        float rotShipDeg = -rotShip * (180 / PI);
        float shipScale = 1.0f;
        DrawTexturePro(shipTex, (Rectangle){0,0, shipTex.width,shipTex.height},
            (Rectangle) { shipPos.x, shipPos.y, shipTex.width*shipScale, shipTex.height* shipScale},
            (Vector2) {(float)shipTex.width* shipScale /2, (float)shipTex.height* shipScale /2 }, rotShipDeg, WHITE);
        
        Rectangle thrustSprSrc = { 121, 29, 9, 12 };

        if (thrusters & 0b1000) {
            DrawTexturePro(effects, thrustSprSrc,
                (Rectangle) {shipPos.x, shipPos.y, 9, 12},
                (Vector2) { (float)9 / 2, ((float)12 / 2) - (shipTex.height * shipScale) +6 }, rotShipDeg, WHITE);
        }
        if (thrusters & 0b0100) {
            DrawTexturePro(effects, thrustSprSrc,
                (Rectangle) { shipPos.x, shipPos.y, 9, 12 },
                (Vector2) { (float)9 / 2, ((float)12 / 2) - (shipTex.height * shipScale) +4 }, rotShipDeg +180.0f, WHITE);
        }
        if (thrusters & 0b0010) {
            DrawTexturePro(effects, thrustSprSrc,
                (Rectangle) {
                shipPos.x, shipPos.y, 9, 12
            },
                (Vector2) {
                (float)9 / 2, ((float)12 / 2) - (shipTex.width * shipScale) + 3
            }, rotShipDeg - 90.0f, WHITE);
        }
        if (thrusters & 0b0001) {
            DrawTexturePro(effects, thrustSprSrc,
                (Rectangle) {
                shipPos.x, shipPos.y, 9, 12
            },
                (Vector2) {
                (float)9 / 2, ((float)12 / 2) - (shipTex.width * shipScale) + 3
            }, rotShipDeg + 90.0f, WHITE);
        }


    EndDrawing();
    //----------------------------------------------------------------------------------  
}
