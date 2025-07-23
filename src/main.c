#include <raylib.h>
#include <stddef.h>
#include <raymath.h>
#include <rlgl.h>
#include <stdlib.h>
#include <float.h>
#include <kos/dbglog.h>
#include "ship/ship.h"
#include "track/track.h"

#define ATTR_ORBIS_WIDTH 640
#define ATTR_ORBIS_HEIGHT 480

static bool done = false;

typedef enum GameState {
    MENU,
    SHIP_SELECTION,
    PLAYING
} GameState;

static GameState gameState = MENU;
static int currentTrackSelection = 0; // 0 for circular, 1 for figure 8
static int currentShipSelection = 0; // 0 for ship 1, 1 for ship 2

static Model shipModels[2];
static Texture2D shipTextures[2];

static void updateController(void) {
    if(!IsGamepadAvailable(0))
        return;

    if(IsGamepadButtonPressed(0, GAMEPAD_BUTTON_MIDDLE_RIGHT)) {
        if (gameState == PLAYING) {
            gameState = MENU;
        } else if (gameState == MENU) {
            done = true; // Exit game from menu if start is pressed
        } else if (gameState == SHIP_SELECTION) {
            gameState = MENU; // Go back to track selection from ship selection
        }
    }

    if (gameState == SHIP_SELECTION) {
        if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_UP) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_DOWN)) {
            currentShipSelection = (currentShipSelection + 1) % 2; // Toggle between 0 and 1
        }
    }
}



int main(int argc, char** argv) {

    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = ATTR_ORBIS_WIDTH;
    const int screenHeight = ATTR_ORBIS_HEIGHT;

    InitWindow(screenWidth, screenHeight, "raylib [models] example - Spinning Cube");

    // Define the camera to look into our 3d world
    Camera camera = { 0 };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera mode type

    // Shader outline = LoadShader(NULL, "/rd/outline.fs");
    // RenderTexture2D target = LoadRenderTexture(screenWidth, screenHeight);

    Ship playerShip;

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second

    // Create a cube model for the skybox
    Mesh skyboxMesh = GenMeshCube(1.0f, 1.0f, 1.0f);
    Model skyboxModel = LoadModelFromMesh(skyboxMesh);
    Texture2D skyboxTexture = LoadTexture("/rd/gradient_skybox.png");
    skyboxModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = skyboxTexture;

    Track gameTrack;

    Texture2D trackTexture = LoadTexture("/rd/Finish_Line.png");

    // Load ship models and textures
    shipModels[0] = LoadModel("/rd/rship.obj");
    shipTextures[0] = LoadTexture("/rd/Finish_Line.png");
    GenTextureMipmaps(&shipTextures[0]);
    SetTextureFilter(shipTextures[0], TEXTURE_FILTER_BILINEAR);
    SetTextureWrap(shipTextures[0], TEXTURE_WRAP_CLAMP);
    shipModels[0].materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = shipTextures[0];
    shipModels[0].materials[0].maps[MATERIAL_MAP_DIFFUSE].color = (Color){ 150, 150, 255, 255 }; // Light blue tint

    // Placeholder for second ship
    shipModels[1] = LoadModel("/rd/cube.obj");
    shipTextures[1] = LoadTexture("/rd/orange.png");
    GenTextureMipmaps(&shipTextures[1]);
    SetTextureFilter(shipTextures[1], TEXTURE_FILTER_BILINEAR);
    SetTextureWrap(shipTextures[1], TEXTURE_WRAP_CLAMP);
    shipModels[1].materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = shipTextures[1];
    shipModels[1].materials[0].maps[MATERIAL_MAP_DIFFUSE].color = (Color){ 255, 150, 50, 255 }; // Orange tint

    // InitFigure8Track(&gameTrack, 600.0f, 300.0f, 100, 60.0f, trackTexture);

    // InitShip(&playerShip, shipModel, shipTexture);
    
    camera.position = (Vector3){ 0.0f, 100.0f, -250.0f }; // Adjusted camera position for large track
    camera.target = (Vector3){ 0.0f, 2.0f, 0.0f };      // Fixed camera target (at ship's height)
    camera.up = (Vector3){ 0.0f, 2.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 95.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera mode type

    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!done)    // Detect window close button or ESC key
    {
        updateController();

        switch (gameState)
        {
            case MENU:
            {
                // Update menu logic
                if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_UP) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_DOWN))
                {
                    currentTrackSelection = (currentTrackSelection + 1) % 2; // Toggle between 0 and 1
                }

                if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) // 'A' button to select
                {
                    // Initialize track based on selection
                    if (currentTrackSelection == 0)
                    {
                        // Load rainbow_road.obj
                        gameTrack.model = LoadModel("/rd/rainbow_road.obj");
                        gameTrack.texture = LoadTexture("/rd/Finish_Road.jpg"); // Assuming this texture
                        GenTextureMipmaps(&gameTrack.texture);
                        SetTextureFilter(gameTrack.texture, TEXTURE_FILTER_BILINEAR);
                        SetTextureWrap(gameTrack.texture, TEXTURE_WRAP_CLAMP);
                        gameTrack.model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = gameTrack.texture;
                        gameTrack.waypoints = NULL; // No waypoints for loaded model
                        gameTrack.waypointCount = 0; // No waypoints for loaded model
                    }
                    else
                    {
                        // Figure 8 track
                        InitFigure8Track(&gameTrack, 600.0f, 300.0f, 100, 60.0f, trackTexture);
                    }
                    gameState = SHIP_SELECTION;
                }

                // Draw menu
                BeginDrawing();
                    ClearBackground(BLACK);
                    DrawText("Hyper Spiral GP", screenWidth / 2 - MeasureText("Hyper Spiral GP", 30) / 2, screenHeight / 2 - 100, 30, RAYWHITE);
                    DrawText("Select Track:", screenWidth / 2 - MeasureText("Select Track:", 20) / 2, screenHeight / 2 - 50, 20, WHITE);
                    
                    if (currentTrackSelection == 0)
                    {
                        DrawText("> Track 1 (Circular)", screenWidth / 2 - MeasureText("> Track 1 (Circular)", 20) / 2, screenHeight / 2, 20, YELLOW);
                        DrawText("  Track 2 (Figure 8)", screenWidth / 2 - MeasureText("  Track 2 (Figure 8)", 20) / 2, screenHeight / 2 + 30, 20, WHITE);
                    }
                    else
                    {
                        DrawText("  Track 1 (Circular)", screenWidth / 2 - MeasureText("  Track 1 (Circular)", 20) / 2, screenHeight / 2, 20, WHITE);
                        DrawText("> Track 2 (Figure 8)", screenWidth / 2 - MeasureText("> Track 2 (Figure 8)", 20) / 2, screenHeight / 2 + 30, 20, YELLOW);
                    }
                EndDrawing();
            } break;

            case SHIP_SELECTION:
            {
                // Update ship selection logic
                if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) // 'A' button to select
                {
                    InitShip(&playerShip, shipModels[currentShipSelection], shipTextures[currentShipSelection]);
                    gameState = PLAYING;
                }

                // Draw ship selection menu
                BeginDrawing();
                    ClearBackground(BLACK);
                    DrawText("Select Ship:", screenWidth / 2 - MeasureText("Select Ship:", 20) / 2, screenHeight / 2 - 150, 20, WHITE);
                    
                    if (currentShipSelection == 0)
                    {
                        DrawText("> Ship 1", screenWidth / 2 - MeasureText("> Ship 1", 20) / 2, screenHeight / 2 + 100, 20, YELLOW);
                        DrawText("  Ship 2", screenWidth / 2 - MeasureText("  Ship 2", 20) / 2, screenHeight / 2 + 130, 20, WHITE);
                    }
                    else
                    {
                        DrawText("  Ship 1", screenWidth / 2 - MeasureText("  Ship 1", 20) / 2, screenHeight / 2 + 100, 20, WHITE);
                        DrawText("> Ship 2", screenWidth / 2 - MeasureText("> Ship 2", 20) / 2, screenHeight / 2 + 130, 20, YELLOW);
                    }

                    // Draw 3D preview of the selected ship
                    // Set up a temporary camera for the preview
                    Camera previewCamera = camera;
                    previewCamera.position = (Vector3){ 0.0f, 5.0f, 10.0f };
                    previewCamera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
                    BeginMode3D(previewCamera);
                        rlPushMatrix();
                            rlRotatef(GetTime() * 50.0f, 0.0f, 1.0f, 0.0f); // Rotate ship
                            DrawModel(shipModels[currentShipSelection], (Vector3){0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
                        rlPopMatrix();
                    EndMode3D();

                EndDrawing();
            } break;

            case PLAYING:
            {
                // Update
                //----------------------------------------------------------------------------------
                UpdateShip(&playerShip, gameTrack.model.meshes[0]);

                // Update camera position and target relative to the ship
                float cameraDistance = 30.0f; // Distance behind the ship
                float cameraHeight = 10.0f;    // Height above the ship

                // Calculate new forward vector components based on current yaw
                float forwardX = sinf(playerShip.yaw * DEG2RAD);
                float forwardZ = cosf(playerShip.yaw * DEG2RAD);

                // Calculate camera position based on ship's position and yaw
                camera.position.x = playerShip.position.x - forwardX * cameraDistance;
                camera.position.z = playerShip.position.z - forwardZ * cameraDistance;
                camera.position.y = playerShip.position.y + cameraHeight;

                camera.target = playerShip.position; // Camera always looks at the ship
                //----------------------------------------------------------------------------------

                // Draw
                //----------------------------------------------------------------------------------
                BeginDrawing();

                    ClearBackground(BLACK); // Clear background to black

                    BeginMode3D(camera);

                        // Draw skybox
                        rlDisableDepthMask(); // Disable depth writes
                        rlDisableBackfaceCulling(); // Disable backface culling
                        DrawModel(skyboxModel, (Vector3){ 0.0f, 0.0f, 0.0f }, 1000.0f, WHITE); // Draw a large skybox at fixed position
                        rlEnableBackfaceCulling(); // Re-enable backface culling
                        rlEnableDepthMask(); // Re-enable depth writes

                        // Draw track
                        DrawTrack(&gameTrack);

                        DrawShip(&playerShip);

                        DrawGrid(10, 1.0f);

                    EndMode3D();

                    DrawFPS(10, 10);

                EndDrawing();
                //----------------------------------------------------------------------------------
            } break;
        }
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(skyboxTexture);
    UnloadModel(skyboxModel);
    UnloadTrack(&gameTrack);
    // UnloadShip(&playerShip); // Ship is unloaded as part of the loop now
    for (int i = 0; i < 2; i++) {
        UnloadModel(shipModels[i]);
        UnloadTexture(shipTextures[i]);
    }
    CloseWindow();     // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}