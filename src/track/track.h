#ifndef TRACK_H
#define TRACK_H

#include <raylib.h>

// Define the Track structure
typedef struct Track {
    Model model;
    Texture2D texture;
    Vector3 *waypoints;
    int waypointCount;
} Track;

// Function declarations
void InitTrack(Track *track, float radius, float width, int segments, float heightVariation, float twistAmount, Texture2D trackTexture);
void DrawTrack(Track *track);
void UnloadTrack(Track *track);

// New function for a figure 8 track
void InitFigure8Track(Track *track, float loopRadius, float trackWidth, int segmentsPerLoop, float heightVariation, Texture2D trackTexture);

// New function for a helix track
void InitHelixTrack(Track *track, float length, float width, int segmentsPerTurn, float heightPerTurn, int numTurns, float twistAmount, Texture2D trackTexture);

// Function to get track surface info
Vector3 GetTrackSurfaceInfo(Vector3 shipPos, Mesh trackMesh, float *outHeight);

#endif // TRACK_H