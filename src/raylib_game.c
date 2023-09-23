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
static float screenRot = 0;

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

Mesh GenMeshTexQuad(float size, Rectangle Src, Vector2 TexSize) {

    Mesh mesh = { 0 };

    mesh.vertexCount = 4;
    mesh.triangleCount = 2;
    mesh.vertices = (float*)RL_MALLOC(mesh.vertexCount * 3 * sizeof(float));
    mesh.texcoords = (float*)RL_MALLOC(mesh.vertexCount * 2 * sizeof(float));
    mesh.normals = (float*)RL_MALLOC(mesh.vertexCount * 3 * sizeof(float));
    mesh.indices = (unsigned short*)RL_MALLOC(mesh.triangleCount * 3 * sizeof(unsigned short));

    float v[12] = { 0.5f,  0.5f, 0.0f ,
                    0.5f, -0.5f, 0.0f,
                    -0.5f, -0.5f, 0.0f,
                    -0.5f,  0.5f, 0.0f };

    memcpy(mesh.vertices, v, 12 * sizeof(float));

    float t[8] = { 1.0f, 1.0f, 
                   1.0f, 0.0f,
                   0.0f, 0.0f,
                   0.0f, 1.0f };

    memcpy(mesh.texcoords, t, 8 * sizeof(float));

    float n[12] = { 0.0f,  0.0f, 1.0f,
                    0.0f,  0.0f, 1.0f,
                    0.0f,  0.0f, 1.0f,
                    0.0f,  0.0f, 1.0f };

    memcpy(mesh.normals, n, 12 * sizeof(float));

    unsigned short i[6] = {0,3,2,2,1,0};


    memcpy(mesh.indices, i, 6 * sizeof(unsigned short));

    UploadMesh(&mesh, false);

    return mesh;
}

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

    Camera camera = { 0 };
    camera.position = (Vector3){ 0.0f, 0.0f, 10.0f }; // Camera position
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };     // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)  
    camera.fovy = 1;// Camera field-of-view Y
    camera.projection = CAMERA_ORTHOGRAPHIC;                   // Camera mode type
   
    Mesh quad = GenMeshTexQuad(1, (Rectangle) { 0 }, (Vector2) { 0 });

    EndTextureMode();
    BeginDrawing();
    ClearBackground(BLUE);

    BeginMode3D(camera);
    //DrawCube((Vector3) { 0, 0, 0 }, 1, 1, 1, WHITE);
    DrawMesh(quad, LoadMaterialDefault(), MatrixIdentity());
    EndMode3D();
    EndDrawing();
    //----------------------------------------------------------------------------------  
}
