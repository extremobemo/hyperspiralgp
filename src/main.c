#include <raylib.h>
#include <stddef.h>
#include <raymath.h>
#include <rlgl.h>
#include <stdlib.h>
#include <float.h>
#include "ship/ship.h"
#include "track/track.h"

#define ATTR_ORBIS_WIDTH 640
#define ATTR_ORBIS_HEIGHT 480

static bool done = false;

static void updateController(void) {
    bool startPressed;

    if(!IsGamepadAvailable(0))
        return;

    startPressed = IsGamepadButtonPressed(0, GAMEPAD_BUTTON_MIDDLE_RIGHT);

    if(startPressed)
        done = true;
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

    Model shipModel = LoadModel("/rd/rship.obj");
    Texture2D shipTexture = LoadTexture("/rd/Finish_Line.png");
    GenTextureMipmaps(&shipTexture);
    SetTextureFilter(shipTexture, TEXTURE_FILTER_BILINEAR);
    SetTextureWrap(shipTexture, TEXTURE_WRAP_CLAMP);
    shipModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = shipTexture;
    shipModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = (Color){ 150, 150, 255, 255 }; // Light blue tint

    Texture2D trackTexture = LoadTexture("/rd/Finish_Line.png");
    InitTrack(&gameTrack, 500.0f, 200.0f, 100, 50.0f, 10.0f, trackTexture);

    InitShip(&playerShip, shipModel, shipTexture);
    
    camera.position = (Vector3){ 0.0f, 10.0f, -25.0f }; // Fixed camera position, moved up and back
    camera.target = (Vector3){ 0.0f, 2.0f, 0.0f };      // Fixed camera target (at ship's height)
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera mode type

    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!done)    // Detect window close button or ESC key
    {
        updateController();

        // Update
        //----------------------------------------------------------------------------------
        UpdateShip(&playerShip, gameTrack.model.meshes[0]);

        // Update camera position and target relative to the ship
        float cameraDistance = 30.0f; // Distance behind the ship
        float cameraHeight = 8.0f;    // Height above the ship

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
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(skyboxTexture);
    UnloadModel(skyboxModel);
    UnloadTrack(&gameTrack);
    UnloadShip(&playerShip);
 
    CloseWindow();     // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
