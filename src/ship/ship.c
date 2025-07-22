#include "ship.h"
#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <math.h>
#include "../track/track.h"

// Custom function to get track surface info (approximated normal and height)
// This is a simplified approach for fixed-function pipeline


void InitShip(Ship *ship, Model shipModel, Texture2D shipTexture)
{
    ship->position = (Vector3){ 0.0f, 2.0f, 0.0f };
    ship->speed = 0.0f;
    ship->yaw = 0.0f;
    ship->rotation = QuaternionIdentity();
    ship->model = shipModel;
    ship->texture = shipTexture;
}

void UpdateShip(Ship *ship, Mesh trackMesh)
{
    // Read analog stick input for steering
    float yawInput = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X); // Assuming left stick X-axis

    // Apply deadzone to yawInput
    float deadzone = 0.1f; // Adjust this value as needed
    if (fabsf(yawInput) < deadzone) yawInput = 0.0f;

    // Update ship's yaw (heading)
    float turnSpeed = 2.0f; // Adjust sensitivity
    ship->yaw += yawInput * turnSpeed; // Accumulate yaw for 360-degree turns

    // Calculate new forward vector components based on current yaw
    float forwardX = sinf(ship->yaw * DEG2RAD);
    float forwardZ = cosf(ship->yaw * DEG2RAD);

    // Move ship forward along its current heading
    ship->position.x += forwardX * ship->speed;
    ship->position.z += forwardZ * ship->speed;

    // Update ship's Y position and orientation to follow the track surface
    float surfaceHeight;
    Vector3 surfaceNormal = GetTrackSurfaceInfo(ship->position, trackMesh, &surfaceHeight);
    ship->position.y = surfaceHeight + 2.0f; // Offset above the surface

    // Calculate pitch and roll from surface normal
    Vector3 shipForward = { sinf(ship->yaw * DEG2RAD), 0.0f, cosf(ship->yaw * DEG2RAD) };
    // 1. Define the Up vector (aligned with surface normal)
    Vector3 shipUp = surfaceNormal;

    // 2. Define the initial Forward vector (based on shipYaw, in XZ plane)
    Vector3 initialForward = { sinf(ship->yaw * DEG2RAD), 0.0f, cosf(ship->yaw * DEG2RAD) };
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
    ship->rotation = QuaternionSlerp(ship->rotation, targetRotation, 0.1f); // Adjust interpolation factor (0.1f) for desired smoothness

    // Check for 'A' button press to accelerate
    if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) // Assuming A button is RIGHT_FACE_DOWN
    {
        ship->speed += 0.05f; // Increase speed
    }
    else
    {
        // Decelerate when A is not pressed
        ship->speed -= 0.02f; // Deceleration rate
        if (ship->speed < 0.0f) ship->speed = 0.0f; // Prevent negative speed
    }

    // Check for 'B' button press to brake
    if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_LEFT)) // Assuming B button is RIGHT_FACE_LEFT
    {
        ship->speed -= 0.1f; // Brake rate
        if (ship->speed < 0.0f) ship->speed = 0.0f; // Prevent negative speed
    }

    // Clamp ship speed to a reasonable maximum
    float maxSpeed = 5.0f;
    if (ship->speed > maxSpeed) ship->speed = maxSpeed;
}

void DrawShip(Ship *ship)
{
    rlPushMatrix();
        rlTranslatef(ship->position.x, ship->position.y, ship->position.z); // Translate to ship's world position
        rlMultMatrixf(MatrixToFloatV(QuaternionToMatrix(ship->rotation)).v);
        rlRotatef(90.0f, 0.0f, 1.0f, 0.0f); // Adjust for model's default orientation
        DrawModel(ship->model, (Vector3){0.0f, 0.0f, 0.0f}, 1.0f, WHITE); // Draw at local origin
    rlPopMatrix();
}

void UnloadShip(Ship *ship)
{
    UnloadTexture(ship->texture);
    UnloadModel(ship->model);
}
