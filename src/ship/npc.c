#include "npc.h"
#include <raymath.h>

void InitNPCs(NPC *npcs, int npcCount, Model npcModel, Texture2D npcTexture, Track *track)
{
    for (int i = 0; i < npcCount; i++) {
        InitShip(&npcs[i].ship, npcModel, npcTexture);
        // Distribute NPCs along the track by assigning different starting waypoints
        npcs[i].currentWaypointIndex = (i * (track->waypointCount / npcCount)) % track->waypointCount;
        npcs[i].ship.position = track->waypoints[npcs[i].currentWaypointIndex];
        npcs[i].ship.speed = 2.0f; // Set a constant speed for NPCs
    }
}

void UpdateNPCs(NPC *npcs, int npcCount, Track *track, Mesh trackMesh)
{
    for (int i = 0; i < npcCount; i++) {
        // Get the current and next waypoint
        Vector3 currentWaypoint = track->waypoints[npcs[i].currentWaypointIndex];
        int nextWaypointIndex = (npcs[i].currentWaypointIndex + 1) % track->waypointCount;
        Vector3 nextWaypoint = track->waypoints[nextWaypointIndex];

        // Move the NPC ship towards the next waypoint
        Vector3 direction = Vector3Normalize(Vector3Subtract(nextWaypoint, npcs[i].ship.position));
        npcs[i].ship.position = Vector3Add(npcs[i].ship.position, Vector3Scale(direction, npcs[i].ship.speed));

        // Update yaw to look at the next waypoint
        npcs[i].ship.yaw = atan2f(direction.x, direction.z) * RAD2DEG;

        // Check if the NPC has reached the next waypoint
        if (Vector3Distance(npcs[i].ship.position, nextWaypoint) < 5.0f) {
            npcs[i].currentWaypointIndex = nextWaypointIndex;
        }

        // Update ship's Y position and orientation to follow the track surface
        float surfaceHeight;
        Vector3 surfaceNormal = GetTrackSurfaceInfo(npcs[i].ship.position, trackMesh, &surfaceHeight);
        npcs[i].ship.position.y = surfaceHeight + 2.0f; // Offset above the surface

        // Update ship rotation based on surface normal and direction
        Vector3 shipUp = surfaceNormal;
        Vector3 shipForward = direction;
        Vector3 shipRight = Vector3CrossProduct(shipUp, shipForward);
        shipForward = Vector3CrossProduct(shipRight, shipUp);

        Matrix targetMatrix = MatrixIdentity();
        targetMatrix.m0 = shipRight.x;    targetMatrix.m4 = shipUp.x;    targetMatrix.m8 = shipForward.x;
        targetMatrix.m1 = shipRight.y;    targetMatrix.m5 = shipUp.y;    targetMatrix.m9 = shipForward.y;
        targetMatrix.m2 = shipRight.z;    targetMatrix.m6 = shipUp.z;    targetMatrix.m10 = shipForward.z;

        Quaternion targetRotation = QuaternionFromMatrix(targetMatrix);
        npcs[i].ship.rotation = QuaternionSlerp(npcs[i].ship.rotation, targetRotation, 0.1f);
    }
}

void DrawNPCs(NPC *npcs, int npcCount)
{
    for (int i = 0; i < npcCount; i++) {
        DrawShip(&npcs[i].ship);
    }
}

void UnloadNPCs(NPC *npcs, int npcCount)
{
    for (int i = 0; i < npcCount; i++) {
        // Note: Model and Texture are shared, so they should be unloaded separately in main
    }
}
