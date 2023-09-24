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

Vector3 cameraUp = { 0.0f, -1.0f, 0.0f };



typedef struct Phys {
    Vector2 pos;
    Vector2 dir;
    float vel;
} Phys;

typedef struct playerShip {
    Vector2 tarDir;
    float maxAngleVel;
    Phys phys;
};



static Phys ship = {
    .pos = {0,0,},
    .dir = {0,-1,},
    .vel = 0
};

static Phys cam = {
    .pos = {0,0,},
    .dir = {0,-1,},
    .vel = 0
};

static int bulletidx = 0;
#define MAX_BULLETS 100
#define SCREENWIDTH 1024
#define SCREENHEIGHT 768
static Phys bullets[100];

static int ballMove = 0;
static int ballMoveDir = 0;

// TODO: Define global variables here, recommended to make them static
static RenderTexture2D target = { 0 };  // Initialized at init

static Mesh quad;
static Mesh bulQuad;
static Material quadTex;
static Material bulQuadTex;

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
static void UpdateDrawFrame(void);      // Update and Draw one frame

Mesh GenMeshTexQuad(Rectangle src, Vector2 texSize) {

    Vector2 texelSize = { 1 / texSize.x,1 / texSize.y };
    Rectangle quadTexSrc = {src.x* texelSize.x, 1- src.y * texelSize.y,src.width * texelSize.x,src.height * texelSize.y };
    Vector2 quadSize = { src.width / (float)SCREENHEIGHT, src.height / (float)SCREENHEIGHT };

    Mesh mesh = { 0 };

    mesh.vertexCount = 4;
    mesh.triangleCount = 2;
    mesh.vertices = (float*)RL_MALLOC(mesh.vertexCount * 3 * sizeof(float));
    mesh.texcoords = (float*)RL_MALLOC(mesh.vertexCount * 2 * sizeof(float));
    mesh.normals = (float*)RL_MALLOC(mesh.vertexCount * 3 * sizeof(float));
    mesh.indices = (unsigned short*)RL_MALLOC(mesh.triangleCount * 3 * sizeof(unsigned short));

    float v[12] = { 0.5f * quadSize.x,  0.5f * quadSize.y, 0.0f ,
                    0.5f * quadSize.x, -0.5f * quadSize.y, 0.0f,
                    -0.5f * quadSize.x, -0.5f * quadSize.y, 0.0f,
                    -0.5f * quadSize.x,  0.5f * quadSize.y, 0.0f };

    memcpy(mesh.vertices, v, 12 * sizeof(float));

    float t[8] = { quadTexSrc.x + quadTexSrc.width, quadTexSrc.y+ quadTexSrc.width,
                   quadTexSrc.x + quadTexSrc.width,quadTexSrc.y,
                   quadTexSrc.x, quadTexSrc.y,
                  quadTexSrc.x, quadTexSrc.y + quadTexSrc.width };

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

    //SetTraceLogLevel(LOG_NONE);         // Disable raylib trace log messsages

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

    quad = GenMeshTexQuad((Rectangle) { 0,0,19,27 }, (Vector2) { shipTex.width, shipTex.height });
    bulQuad = GenMeshTexQuad((Rectangle) { 4, 42, 4, 11 }, (Vector2) { effects.width, effects.height });

    quadTex = LoadMaterialDefault();
    SetMaterialTexture(&quadTex, MATERIAL_MAP_DIFFUSE, shipTex);
    
    bulQuadTex = LoadMaterialDefault();
    SetMaterialTexture(&bulQuadTex, MATERIAL_MAP_DIFFUSE, effects);

    
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
    float velocity = 0.006f;

    Vector2 leftNormal = Vector2Normalize((Vector2) { ship.dir.y, -ship.dir.x });
    Vector2 rightNormal = Vector2Normalize((Vector2) { -ship.dir.y, ship.dir.x });
  
    float lookAheadMulti = 0;

    if (IsKeyDown(KEY_W)) {
        ship.pos = Vector2Add(ship.pos, Vector2Scale(ship.dir, velocity));
        lookAheadMulti = 1;
        //thrusters |= 0b1000;
    }
    if (IsKeyDown(KEY_S)) {
        ship.pos = Vector2Add(ship.pos, Vector2Scale(ship.dir, -velocity));
        lookAheadMulti = -1;
        //thrusters |= 0b0100;
    }
    if (IsKeyDown(KEY_A)) {
        ship.pos = Vector2Add(ship.pos, Vector2Scale(rightNormal, velocity));
        //lookAheadMulti = 1;
        //thrusters |= 0b0010;
    }
    if (IsKeyDown(KEY_D)) {
        ship.pos = Vector2Add(ship.pos, Vector2Scale(leftNormal, velocity));
        //lookAheadMulti = 1;
        //thrusters |= 0b0001;
    }

    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        bullets[bulletidx].dir = ship.dir;
        bullets[bulletidx].pos = ship.pos;
        bullets[bulletidx].vel = 0.02f;
        bulletidx += 1;
        if (bulletidx >= MAX_BULLETS) {
            bulletidx = 0;
        }
    }

    for (int i = 0; i < bulletidx; i++) {
        bullets[i].pos = Vector2Add(bullets[i].pos, Vector2Scale(bullets[i].dir, bullets[i].vel));
    }

    Vector2 ScreenSpaceCursor = { GetMouseX(), GetMouseY() };
    Vector2 ScreenSpaceOrigin = { SCREENWIDTH/2, SCREENHEIGHT/2 };

    Vector2 RelativeCursor = Vector2Normalize(Vector2Subtract(ScreenSpaceCursor, ScreenSpaceOrigin));
    float CursorAngle = -Vector2Angle((Vector2) { 0, -1 }, RelativeCursor);
    float rotMulti = 1.0f;
    float cameraRot = CursorAngle;// *rotMulti;



    

    float mouseXNDC = (GetMouseX() / (float)SCREENWIDTH) * 2.0f - 1.0f;
    float mouseYNDC = ((SCREENHEIGHT - GetMouseY()) / (float)SCREENHEIGHT) * 2.0f - 1.0f;

    float cameraMaxDistErr = 0.1f;
    float lookAhead = 0.5f * lookAheadMulti;
    Vector2 camTar = Vector2Add(ship.pos, Vector2Scale(ship.dir, lookAhead));

    float DistErr = Vector2Distance(camTar, cam.pos);
    float camVel = (DistErr / cameraMaxDistErr)* velocity;
    Vector2 camTarDir = Vector2Subtract(camTar, cam.pos);
    cam.pos = Vector2Add(cam.pos, Vector2Scale(camTarDir, camVel));

    float cameraMaxAngleErr = 20.0f;
    float cameraAngleErr = Vector2Angle(ship.dir, cam.dir);
    float camAngleVel = fabs(cameraAngleErr / cameraMaxAngleErr);
    //if (camAngleVel > 1.0f) { camAngleVel = 1.0f; }

    Vector2 newCamDir = Vector2Rotate(cam.dir, cameraAngleErr* camAngleVel);
    Vector2 altdir = Vector2Rotate(cam.dir, -cameraAngleErr);


    cam.dir = newCamDir;
    cameraUp = (Vector3){ cam.dir.x, cam.dir.y, 0.0f };

    Camera camera = { 0 };
    camera.position = (Vector3){ cam.pos.x, cam.pos.y, 10.0f }; // Camera position
    camera.target = (Vector3){ cam.pos.x, cam.pos.y, 0.0f };     // Camera looking at point
    camera.up = cameraUp;          // Camera up vector (rotation towards target)  
    camera.fovy = 1;// Camera field-of-view Y
    camera.projection = CAMERA_ORTHOGRAPHIC;                   // Camera mode type

    Matrix invCam = MatrixInvert(GetCameraMatrix(camera));

    Vector3 mouseWorld = Vector3Transform((Vector3) { mouseXNDC, mouseYNDC, 0.0f }, invCam);
    Vector2 mouseWorld2D = { mouseWorld.x,mouseWorld.y };
    Vector2 tarDir = Vector2Normalize(Vector2Subtract(mouseWorld2D, ship.pos));
    float angleErr = -Vector2Angle(ship.dir, tarDir);
    if (angleErr > 0.07f) { angleErr = 0.07f; }
    if (angleErr < -0.07f) { angleErr = -0.07f; }
    Vector2 newDirAngle = Vector2Rotate(ship.dir, angleErr * rotMulti);
    Vector2 newDir = Vector2Lerp(ship.dir, tarDir, rotMulti);
    ship.dir = Vector2Normalize(newDirAngle);


   
    



    EndTextureMode();
    BeginDrawing();
    ClearBackground(BLUE);

    BeginMode3D(camera);
    DrawCube((Vector3) { 0, 0, -10 }, 1, 1, 1, WHITE);

    for (size_t i = 0; i < bulletidx; i++) {
        float bulAngle = -Vector2Angle((Vector2) { 0, -1 }, bullets[i].dir);
        Matrix bulMtx = MatrixIdentity();
        Matrix bulRotMtx = MatrixRotateZ(bulAngle);
        Matrix bulTransMtx = MatrixTranslate(bullets[i].pos.x, bullets[i].pos.y, 0);
        bulMtx = MatrixMultiply(bulRotMtx, bulMtx);
        bulMtx = MatrixMultiply(bulMtx, bulTransMtx);
        DrawMesh(bulQuad, bulQuadTex, bulMtx);
    }

    float shipAngle = -Vector2Angle((Vector2) { 0, -1 }, ship.dir);
    Matrix shipMtx = MatrixIdentity();
    Matrix shipRotMtx = MatrixRotateZ(shipAngle);
    Matrix shipTransMtx = MatrixTranslate(ship.pos.x,ship.pos.y,0);
    shipMtx = MatrixMultiply(shipRotMtx, shipMtx);
    shipMtx = MatrixMultiply(shipMtx, shipTransMtx);
    DrawMesh(quad, quadTex, shipMtx);
    EndMode3D();
    char str[100];
    sprintf(str, "rotAngle: %f", angleErr);
    DrawText(str, 0, 0, 30, RED);

    sprintf(str, "mouse pos scrn: %i,%i", GetMouseX(), GetMouseY());
    DrawText(str, 0, 30, 30, RED);



    sprintf(str, "mouse pos ndc: %f,%f", mouseXNDC, mouseYNDC);
    DrawText(str, 0, 60, 30, RED);



    sprintf(str, "mouse pos wrld: %f,%f", mouseWorld.x, mouseWorld.y);
    DrawText(str, 0, 90, 30, RED);

    DrawCircleLines(GetMouseX(), GetMouseY(), 10, MAROON);



    EndDrawing();

    UnloadMesh(quad);
    UnloadMesh(bulQuad);
    //----------------------------------------------------------------------------------  
}
