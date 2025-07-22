#include "track.h"
#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <stdlib.h>
#include <math.h>

#include "track.h"
#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <stdlib.h>
#include <math.h>

// Custom function to get track surface info (approximated normal and height)
// This is a simplified approach for fixed-function pipeline
Vector3 GetTrackSurfaceInfo(Vector3 shipPos, Mesh trackMesh, float *outHeight)
{
    Vector3 normal = { 0.0f, 1.0f, 0.0f }; // Default to flat normal
    *outHeight = shipPos.y; // Default to current ship height if no hit

    // Define ray parameters
    float rayOffset = 50.0f; // Offset for ray start above/below ship

    // Ray 1: From above, pointing downwards
    Ray rayDown = { 0 };
    rayDown.position = (Vector3){ shipPos.x, shipPos.y + rayOffset, shipPos.z };
    rayDown.direction = (Vector3){ 0.0f, -1.0f, 0.0f };

    // Ray 2: From below, pointing upwards
    Ray rayUp = { 0 };
    rayUp.position = (Vector3){ shipPos.x, shipPos.y - rayOffset, shipPos.z };
    rayUp.direction = (Vector3){ 0.0f, 1.0f, 0.0f };

    RayCollision collisionDown = GetRayCollisionMesh(rayDown, trackMesh, MatrixIdentity());
    RayCollision collisionUp = GetRayCollisionMesh(rayUp, trackMesh, MatrixIdentity());

    bool hitDown = collisionDown.hit;
    bool hitUp = collisionUp.hit;

    if (hitDown && hitUp)
    {
        // Both rays hit, choose the one closer to the ship's current Y position
        float distDown = fabsf(shipPos.y - collisionDown.point.y);
        float distUp = fabsf(shipPos.y - collisionUp.point.y);

        if (distDown < distUp)
        {
            *outHeight = collisionDown.point.y;
            normal = collisionDown.normal;
        }
        else
        {
            *outHeight = collisionUp.point.y;
            normal = collisionUp.normal;
        }
    }
    else if (hitDown)
    {
        // Only rayDown hit
        *outHeight = collisionDown.point.y;
        normal = collisionDown.normal;
    }
    else if (hitUp)
    {
        // Only rayUp hit
        *outHeight = collisionUp.point.y;
        normal = collisionUp.normal;
    }
    // If no hit, outHeight remains shipPos.y (default) and normal remains {0,1,0}

    return normal;
}

// Custom function to generate a simple non-flat track mesh
Mesh GenMeshTrack(float radius, float width, int segments, float heightVariation, float twistAmount, Vector3 **outWaypoints, int *outWaypointCount)
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

    *outWaypointCount = segments;
    *outWaypoints = (Vector3 *)RL_MALLOC(segments * sizeof(Vector3));

    float angleStep = 360.0f / segments; // Degrees per segment

    for (int i = 0; i < segments; i++)
    {
        float currentAngle = i * angleStep;
        float radAngle = currentAngle * DEG2RAD;

        // Calculate height variation
        float currentHeight = heightVariation * sinf(radAngle * 2.0f); // Simple sine wave for height

        // Waypoint at the center of the track segment
        (*outWaypoints)[i].x = radius * cosf(radAngle);
        (*outWaypoints)[i].y = currentHeight;
        (*outWaypoints)[i].z = radius * sinf(radAngle);

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

void InitTrack(Track *track, float radius, float width, int segments, float heightVariation, float twistAmount, Texture2D trackTexture)
{
    track->model = LoadModelFromMesh(GenMeshTrack(radius, width, segments, heightVariation, twistAmount, &track->waypoints, &track->waypointCount));
    track->texture = trackTexture;
    track->model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = track->texture;
}

// Custom function to generate a figure 8 track mesh
Mesh GenMeshFigure8Track(float loopRadius, float trackWidth, int segmentsPerLoop, float heightVariation, Vector3 **outWaypoints, int *outWaypointCount)
{
    Mesh mesh = { 0 };
    int totalSegments = segmentsPerLoop * 2; // Two loops for figure 8
    int numVertices = totalSegments * 2; // Two vertices per segment (inner and outer edge)
    int numTriangles = totalSegments * 2; // Two triangles per segment

    mesh.vertexCount = numVertices;
    mesh.triangleCount = numTriangles;

    mesh.vertices = (float *)RL_MALLOC(numVertices * 3 * sizeof(float));
    mesh.texcoords = (float *)RL_MALLOC(numVertices * 2 * sizeof(float));
    mesh.normals = (float *)RL_MALLOC(numVertices * 3 * sizeof(float));
    mesh.indices = (unsigned short *)RL_MALLOC(numTriangles * 3 * sizeof(unsigned short));

    *outWaypointCount = totalSegments;
    *outWaypoints = (Vector3 *)RL_MALLOC(totalSegments * sizeof(Vector3));

    

    for (int i = 0; i < totalSegments; i++)
    {
        float t = (float)i / totalSegments * (2.0f * PI); // Parameter t from 0 to 2PI

        // Parametric equations for a figure 8 (lemniscate-like)
        float x = loopRadius * sinf(t);
        float z = loopRadius * sinf(t) * cosf(t); // This creates a figure 8 shape

        // Calculate height variation
        float currentHeight = heightVariation * cosf(t); // Use cosine for overpass/underpass at intersection

        // Waypoint at the center of the track segment
        (*outWaypoints)[i].x = x;
        (*outWaypoints)[i].y = currentHeight;
        (*outWaypoints)[i].z = z;

        // Calculate tangent for track width
        // Numerical differentiation for tangent
        float dt = 0.001f;
        float x_plus_dt = loopRadius * sinf(t + dt);
        float z_plus_dt = loopRadius * sinf(t + dt) * cosf(t + dt);

        float tangentX = x_plus_dt - x;
        float tangentZ = z_plus_dt - z;

        // Normalize tangent
        float tangentLength = sqrtf(tangentX * tangentX + tangentZ * tangentZ);
        if (tangentLength > 0) {
            tangentX /= tangentLength;
            tangentZ /= tangentLength;
        }

        // Perpendicular vector for track width (rotated 90 degrees from tangent)
        float perpX = -tangentZ;
        float perpZ = tangentX;

        // Inner vertex
        mesh.vertices[i * 6 + 0] = x + (trackWidth / 2.0f) * perpX;
        mesh.vertices[i * 6 + 1] = currentHeight;
        mesh.vertices[i * 6 + 2] = z + (trackWidth / 2.0f) * perpZ;

        // Outer vertex
        mesh.vertices[i * 6 + 3] = x - (trackWidth / 2.0f) * perpX;
        mesh.vertices[i * 6 + 4] = currentHeight;
        mesh.vertices[i * 6 + 5] = z - (trackWidth / 2.0f) * perpZ;

        // Texture coordinates (simple mapping)
        mesh.texcoords[i * 4 + 0] = (float)i / totalSegments; // U for inner
        mesh.texcoords[i * 4 + 1] = 0.0f;                // V for inner
        mesh.texcoords[i * 4 + 2] = (float)i / totalSegments; // U for outer
        mesh.texcoords[i * 4 + 3] = 1.0f;                // V for outer

        // Normals (simplified for now, will be updated by surface scanning)
        mesh.normals[i * 6 + 0] = 0.0f;
        mesh.normals[i * 6 + 1] = 1.0f;
        mesh.normals[i * 6 + 2] = 0.0f;
        mesh.normals[i * 6 + 3] = 0.0f;
        mesh.normals[i * 6 + 4] = 1.0f;
        mesh.normals[i * 6 + 5] = 0.0f;
    }

    // Generate indices for a closed loop track
    int index = 0;
    for (int i = 0; i < totalSegments; i++)
    {
        int i0 = i * 2;
        int i1 = (i * 2 + 1);
        int i2 = ((i + 1) % totalSegments) * 2; // Connect last segment to first
        int i3 = (((i + 1) % totalSegments) * 2 + 1);

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

void InitFigure8Track(Track *track, float loopRadius, float trackWidth, int segmentsPerLoop, float heightVariation, Texture2D trackTexture)
{
    track->model = LoadModelFromMesh(GenMeshFigure8Track(loopRadius, trackWidth, segmentsPerLoop, heightVariation, &track->waypoints, &track->waypointCount));
    track->texture = trackTexture;
    track->model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = track->texture;
}



void DrawTrack(Track *track)
{
    DrawModel(track->model, (Vector3){ 0.0f, 0.0f, 0.0f }, 1.0f, DARKGRAY);
}

void UnloadTrack(Track *track)
{
    UnloadTexture(track->texture);
    UnloadModel(track->model);
    RL_FREE(track->waypoints);
}