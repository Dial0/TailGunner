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
static int shipTargetRadius;
static Texture2D shipTex;
static Texture2D effects;

typedef struct Bullet {
    Vector2 pos;
    Vector2 dir;
    float vel;
} Bullet;

static int bulletidx = 0;
#define MAX_BULLETS 100
#define SCREENWIDTH 1024
#define SCREENHEIGHT 768
static Bullet bullets[100];

static int ballMove = 0;
static int ballMoveDir = 0;

// TODO: Define global variables here, recommended to make them static
static RenderTexture2D target = { 0 };  // Initialized at init
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
    static const int screenWidth = SCREENWIDTH;
    static const int screenHeight = SCREENHEIGHT;
    // Initialization
    //--------------------------------------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "raylib 9yr gamejam");
    target = LoadRenderTexture(screenWidth, screenHeight);
    shipPos = (Vector2){ screenWidth / 2,screenHeight / 2 };
    shipTex = LoadTexture("resources/ship.png");
    effects = LoadTexture("resources/effects.png");
    shipTargetRadius = 50;
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
    float velocity = 4.0f;
    //Vector2 dir = { 0 };
    //float cursDist = Vector2Distance((Vector2) { GetMouseX(), GetMouseY() }, shipPos);
    //if (cursDist > 20.0f) {
    //    dir = Vector2Normalize(Vector2Subtract((Vector2) { GetMouseX(), GetMouseY() }, shipPos));
    //    lastDir = dir;
    //} else {
    //    dir = lastDir;
    //    velocity = 0.0f;
    //}

    Vector2 ScreenSpaceCursor = { GetMouseX(), GetMouseY() };
    Vector2 ShipRelativeCursor = Vector2Subtract(ScreenSpaceCursor, shipPos);
    float rotShip = Vector2Angle((Vector2) { 0,-1 }, Vector2Normalize(ShipRelativeCursor));
    Vector2 WorldSpaceCursor = Vector2Rotate(ShipRelativeCursor,rotShip);

    Vector2 dir = Vector2Normalize(Vector2Subtract((Vector2) { GetMouseX(), GetMouseY() }, shipPos));
    Vector2 leftNormal = Vector2Normalize((Vector2) { dir.y, -dir.x });
    Vector2 rightNormal = Vector2Normalize((Vector2) { -dir.y, dir.x });

    float deadZone = 5.0f;

    int thrusters = 0b0000;
    float cursDist = Vector2Distance((Vector2) { GetMouseX(), GetMouseY() }, shipPos);
    
    if (IsKeyDown(KEY_W) && shipTargetRadius > 20.0f) {
        shipTargetRadius -= velocity;
        //shipPos = Vector2Add(shipPos, Vector2Scale(dir, velocity));
        //thrusters |= 0b1000;
    }
    if (IsKeyDown(KEY_S)) {
        shipTargetRadius += velocity;
        //shipPos = Vector2Add(shipPos, Vector2Scale(dir, -velocity));
        //thrusters |= 0b0100;
    }
    if (IsKeyDown(KEY_A)) {
        shipPos = Vector2Add(shipPos, Vector2Scale(leftNormal, velocity));
        thrusters |= 0b0010;
    }
    if (IsKeyDown(KEY_D)) {
        shipPos = Vector2Add(shipPos, Vector2Scale(rightNormal, velocity));
        thrusters |= 0b0001;
    }

    if(cursDist>(shipTargetRadius + deadZone)){
        shipPos = Vector2Add(shipPos, Vector2Scale(dir, velocity));
        thrusters |= 0b1000;
    }
    if(cursDist<(shipTargetRadius - deadZone)){
        shipPos = Vector2Add(shipPos, Vector2Scale(dir, -velocity));
        thrusters |= 0b0100;
    }

    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        bullets[bulletidx].dir = dir;
        bullets[bulletidx].pos = shipPos;
        bullets[bulletidx].vel = 8.0f;
        bulletidx += 1;
        if (bulletidx >= MAX_BULLETS) {
            bulletidx = 0;
        }
    }

    for (int i = 0; i < bulletidx; i++) {
        bullets[i].pos = Vector2Add(bullets[i].pos, Vector2Scale(bullets[i].dir, bullets[i].vel));
    }

    if (ballMoveDir == 0) {
        ballMove += 2;
        if (ballMove > 300) {
            ballMoveDir = 1;
        }
    } else {
        ballMove -= 2;
        if (ballMove < -300) {
            ballMoveDir = 0;
        }
    }
    
    //BeginDrawing();
    BeginTextureMode(target);
        ClearBackground(RAYWHITE);

        DrawCircle(SCREENWIDTH / 2 + 30, SCREENHEIGHT / 2, 60, BLUE);

        DrawCircle(SCREENWIDTH / 2 + 250, SCREENHEIGHT / 2 + 220, 20, YELLOW);

        DrawCircle(SCREENWIDTH / 2 - 250, SCREENHEIGHT / 2 + ballMove, 20, RED);

        DrawFPS(0, 0);
        // Draw render texture to screen scaled as required
        // Draw equivalent mouse position on the target render-texture


        //DrawShip(shipPos, (Vector2) { GetMouseX(), GetMouseY() });
        float rotShip = Vector2Angle((Vector2) { 0,-1 }, dir);
        float rotShipDeg = -rotShip * (180 / PI);
        float shipScale = 1.0f;

        Rectangle bulletSprSrc = { 4, 42, 4, 12 };
        for (int i = 0; i < bulletidx; i++) {
            float rotBul = Vector2Angle((Vector2) { 0, -1 }, bullets[i].dir);
            float rotBulDeg = -rotBul * (180 / PI);
            Rectangle bulSprDst = { bullets[i].pos.x, bullets[i].pos.y, bulletSprSrc.width,bulletSprSrc.height };
            Vector2 org = { (float)bulletSprSrc.width / 2, (float)bulletSprSrc.height / 2 };
            DrawTexturePro(shipTex, bulletSprSrc, bulSprDst, org, rotBulDeg, WHITE);
        }

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


    //EndDrawing();
    EndTextureMode();
    BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawCircleLines(GetMouseX(), GetMouseY(), 10, MAROON);

        //DrawCircleLines(GetMouseX(), GetMouseY(), shipTargetRadius-(deadZone/2), MAROON);
        Rectangle tarSrc = { 0, 0, (float)target.texture.width, -(float)target.texture.height };
        Rectangle dst = { target.texture.width/2, target.texture.height/2, (float)target.texture.width, (float)target.texture.height };
        Vector2 org = shipPos; //{GetMouseX(), GetMouseY()};
        DrawTexturePro(target.texture, tarSrc, dst, org, rotShipDeg, WHITE);
    EndDrawing();
    //----------------------------------------------------------------------------------  
}
