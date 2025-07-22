#include <raylib.h>
#include <stddef.h>
#include <raymath.h>
#include <rlgl.h>
#include <stdlib.h>
#include <float.h>
#include <math.h> // Required for sinf, cosf, atan2f, FLT_MAX

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

// Custom function to get track surface info (approximated normal and height)
// This is a simplified approach for fixed-function pipeline
Vector3 GetTrackSurfaceInfo(Vector3 shipPos, Mesh trackMesh, float *outHeight)
{
    Vector3 normal = { 0.0f, 1.0f, 0.0f }; // Default to flat normal
    *outHeight = 0.0f; // Default height

    // Create a ray from above the ship, pointing downwards
    Ray ray = { 0 };
    ray.position = (Vector3){ shipPos.x, 100.0f, shipPos.z }; // Start ray from a safe height above the track
    ray.direction = (Vector3){ 0.0f, -1.0f, 0.0f }; // Pointing straight down

    // Get collision info between the ray and the track mesh
    // Assuming trackModel.transform is identity, otherwise it should be passed here
    RayCollision collision = GetRayCollisionMesh(ray, trackMesh, MatrixIdentity());

    if (collision.hit)
    {
        *outHeight = collision.point.y;
        normal = collision.normal;
    }
    // If no hit, it means the ship is off the track or the ray started too low.
    // For this example, we assume the ship is always over the track.

    return normal;
}

// Custom function to generate a simple non-flat track mesh
Mesh GenMeshTrack(float radius, float width, int segments, float heightVariation, float twistAmount)
{
    Mesh mesh = { 0 };
    int numVertices = segments * 2; // Two vertices per segment (inner and outer edge)
    int numTriangles = segments * 2; // Two triangles per segment

    mesh.vertexCount = numVertices;
    mesh.triangleCount = numTriangles;

    mesh.vertices = (float *)RL_MALLOC(numVertices * 3 * sizeof(float));
    mesh.texcoords = (float *)RL_MALLOC(numVertices * 2 * sizeof(float));
    mesh.normals = (float *)RL_MALLOC(numVertices * 3 * sizeof(float));
    mesh.indices = (unsigned short *)RL_MALLOC(numTriangles * 3 * sizeof(unsigned short));

    float angleStep = 360.0f / segments; // Degrees per segment

    for (int i = 0; i < segments; i++)
    {
        float currentAngle = i * angleStep;
        float radAngle = currentAngle * DEG2RAD;

        // Calculate height variation
        float currentHeight = heightVariation * sinf(radAngle * 2.0f); // Simple sine wave for height

        // Inner vertex
        mesh.vertices[i * 6 + 0] = (radius - width / 2.0f) * cosf(radAngle);
        mesh.vertices[i * 6 + 1] = currentHeight;
        mesh.vertices[i * 6 + 2] = (radius - width / 2.0f) * sinf(radAngle);

        // Outer vertex
        mesh.vertices[i * 6 + 3] = (radius + width / 2.0f) * cosf(radAngle);
        mesh.vertices[i * 6 + 4] = currentHeight;
        mesh.vertices[i * 6 + 5] = (radius + width / 2.0f) * sinf(radAngle);

        // Texture coordinates (simple mapping)
        mesh.texcoords[i * 4 + 0] = (float)i / segments; // U for inner
        mesh.texcoords[i * 4 + 1] = 0.0f;                // V for inner
        mesh.texcoords[i * 4 + 2] = (float)i / segments; // U for outer
        mesh.texcoords[i * 4 + 3] = 1.0f;                // V for outer

        // Normals (simplified for now, will be calculated more accurately later)
        // For now, assume flat normal, will be updated by surface scanning
        mesh.normals[i * 6 + 0] = 0.0f;
        mesh.normals[i * 6 + 1] = 1.0f;
        mesh.normals[i * 6 + 2] = 0.0f;
        mesh.normals[i * 6 + 3] = 0.0f;
        mesh.normals[i * 6 + 4] = 1.0f;
        mesh.normals[i * 6 + 5] = 0.0f;
    }

    // Generate indices for a closed loop track
    int index = 0;
    for (int i = 0; i < segments; i++)
    {
        int i0 = i * 2;
        int i1 = (i * 2 + 1);
        int i2 = ((i + 1) % segments) * 2;
        int i3 = (((i + 1) % segments) * 2 + 1);

        // First triangle of quad
        mesh.indices[index++] = i0;
        mesh.indices[index++] = i2;
        mesh.indices[index++] = i1;

        // Second triangle of quad
        mesh.indices[index++] = i1;
        mesh.indices[index++] = i2;
        mesh.indices[index++] = i3;
    }

    UploadMesh(&mesh, false);

    return mesh;
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

    Vector3 shipPosition = { 0.0f, 2.0f, 0.0f }; // Ship floating above the plane
    float shipSpeed = 0.0f;
    float shipYaw = 0.0f; // Ship's current yaw angle
    Quaternion shipRotation = QuaternionIdentity(); // Ship's current rotation

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second

    // Create a cube model for the skybox
    Mesh skyboxMesh = GenMeshCube(1.0f, 1.0f, 1.0f);
    Model skyboxModel = LoadModelFromMesh(skyboxMesh);
    Texture2D skyboxTexture = LoadTexture("/rd/gradient_skybox.png");
    skyboxModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = skyboxTexture;

    Model trackModel = LoadModelFromMesh(GenMeshTrack(500.0f, 200.0f, 100, 50.0f, 10.0f)); // Radius, width, segments, heightVariation, twistAmount
    Texture2D trackTexture = LoadTexture("/rd/Finish_Line.png"); // Load the same texture as the ship
    trackModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = trackTexture; // Apply the texture to the track

    Model cubeModel = LoadModel("/rd/rship.obj");
    Texture2D cubeTexture = LoadTexture("/rd/Finish_Line.png");
    GenTextureMipmaps(&cubeTexture);
    SetTextureFilter(cubeTexture, TEXTURE_FILTER_BILINEAR);
    SetTextureWrap(cubeTexture, TEXTURE_WRAP_CLAMP);
    cubeModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = cubeTexture;
    cubeModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = (Color){ 150, 150, 255, 255 }; // Light blue tint
    
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
        // Read analog stick input for steering
        float yawInput = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X); // Assuming left stick X-axis

        // Apply deadzone to yawInput
        float deadzone = 0.1f; // Adjust this value as needed
        if (fabsf(yawInput) < deadzone) yawInput = 0.0f;

        // Update ship's yaw (heading)
        float turnSpeed = 2.0f; // Adjust sensitivity
        shipYaw += yawInput * turnSpeed; // Accumulate yaw for 360-degree turns

        // Calculate new forward vector components based on current yaw
        float forwardX = sinf(shipYaw * DEG2RAD);
        float forwardZ = cosf(shipYaw * DEG2RAD);

        // Move ship forward along its current heading
        shipPosition.x += forwardX * shipSpeed;
        shipPosition.z += forwardZ * shipSpeed;

        // Update ship's Y position and orientation to follow the track surface
        float surfaceHeight;
        Vector3 surfaceNormal = GetTrackSurfaceInfo(shipPosition, trackModel.meshes[0], &surfaceHeight);
        shipPosition.y = surfaceHeight + 2.0f; // Offset above the surface

        // Calculate pitch and roll from surface normal
        Vector3 shipForward = { sinf(shipYaw * DEG2RAD), 0.0f, cosf(shipYaw * DEG2RAD) };
        // 1. Define the Up vector (aligned with surface normal)
        Vector3 shipUp = surfaceNormal;

        // 2. Define the initial Forward vector (based on shipYaw, in XZ plane)
        Vector3 initialForward = { sinf(shipYaw * DEG2RAD), 0.0f, cosf(shipYaw * DEG2RAD) };
        initialForward = Vector3Normalize(initialForward);

        // 3. Calculate the Right vector (orthogonal to Up and initialForward)
        Vector3 shipRight = Vector3CrossProduct(shipUp, initialForward);
        shipRight = Vector3Normalize(shipRight);

        // 4. Recalculate the Forward vector (orthogonal to Right and Up)
        // This ensures the forward vector is truly in the plane defined by Right and Up,
        // and thus prevents roll.
        shipForward = Vector3CrossProduct(shipRight, shipUp);
        shipForward = Vector3Normalize(shipForward);

        // Construct the rotation matrix using these orthonormal vectors
        Matrix targetMatrix = MatrixIdentity();
        targetMatrix.m0 = shipRight.x;    targetMatrix.m4 = shipUp.x;    targetMatrix.m8 = shipForward.x;
        targetMatrix.m1 = shipRight.y;    targetMatrix.m5 = shipUp.y;    targetMatrix.m9 = shipForward.y;
        targetMatrix.m2 = shipRight.z;    targetMatrix.m6 = shipUp.z;    targetMatrix.m10 = shipForward.z;

        // Convert the target matrix to a quaternion
        Quaternion targetRotation = QuaternionFromMatrix(targetMatrix);

        // Smoothly interpolate ship's rotation
        shipRotation = QuaternionSlerp(shipRotation, targetRotation, 0.1f); // Adjust interpolation factor (0.1f) for desired smoothness

        // Update camera position and target relative to the ship
        float cameraDistance = 30.0f; // Distance behind the ship
        float cameraHeight = 8.0f;    // Height above the ship

        // Calculate camera position based on ship's position and yaw
        camera.position.x = shipPosition.x - forwardX * cameraDistance;
        camera.position.z = shipPosition.z - forwardZ * cameraDistance;
        camera.position.y = shipPosition.y + cameraHeight;

        camera.target = shipPosition; // Camera always looks at the ship

        // Check for 'A' button press to accelerate
        if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) // Assuming A button is RIGHT_FACE_DOWN
        {
            shipSpeed += 0.05f; // Increase speed
        }
        else
        {
            // Decelerate when A is not pressed
            shipSpeed -= 0.02f; // Deceleration rate
            if (shipSpeed < 0.0f) shipSpeed = 0.0f; // Prevent negative speed
        }

        // Check for 'B' button press to brake
        if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_LEFT)) // Assuming B button is RIGHT_FACE_LEFT
        {
            shipSpeed -= 0.1f; // Brake rate
            if (shipSpeed < 0.0f) shipSpeed = 0.0f; // Prevent negative speed
        }

        // Clamp ship speed to a reasonable maximum
        float maxSpeed = 5.0f;
        if (shipSpeed > maxSpeed) shipSpeed = maxSpeed;
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
                DrawModel(trackModel, (Vector3){ 0.0f, 0.0f, 0.0f }, 1.0f, DARKGRAY); // Track is stationary

                rlPushMatrix();
                    rlTranslatef(shipPosition.x, shipPosition.y, shipPosition.z); // Translate to ship's world position
                    rlMultMatrixf(MatrixToFloatV(QuaternionToMatrix(shipRotation)).v);
                    rlRotatef(90.0f, 0.0f, 1.0f, 0.0f); // Adjust for model's default orientation
                    DrawModel(cubeModel, (Vector3){0.0f, 0.0f, 0.0f}, 1.0f, WHITE); // Draw at local origin
                    
                rlPopMatrix();

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
    UnloadModel(trackModel);
    UnloadTexture(cubeTexture);
    UnloadModel(cubeModel);
 
    CloseWindow();     // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
