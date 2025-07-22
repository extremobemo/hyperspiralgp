#ifndef SHIP_H
#define SHIP_H

#include <raylib.h>
#include <raymath.h>

// Define the Ship structure
typedef struct Ship {
    Vector3 position;
    float speed;
    float yaw;
    Quaternion rotation;
    Model model;
    Texture2D texture;
} Ship;

// Function declarations
void InitShip(Ship *ship, Model shipModel, Texture2D shipTexture);
void UpdateShip(Ship *ship, Mesh trackMesh);
void DrawShip(Ship *ship);
void UnloadShip(Ship *ship);

#endif // SHIP_H
