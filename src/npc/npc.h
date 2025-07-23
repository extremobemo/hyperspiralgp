#ifndef NPC_H
#define NPC_H

#include "../ship/ship.h"
#include "../track/track.h"

// Define the NPC structure, which includes a ship
typedef struct NPC {
    Ship ship;
    int currentWaypointIndex;
} NPC;

// Function declarations
void InitNPCs(NPC *npcs, int npcCount, Model npcModel, Texture2D npcTexture, Track *track);
void UpdateNPCs(NPC *npcs, int npcCount, Track *track, Mesh trackMesh);
void DrawNPCs(NPC *npcs, int npcCount);
void UnloadNPCs(NPC *npcs, int npcCount);

#endif // NPC_H
