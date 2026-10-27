#ifndef MAZE_MAP_H
#define MAZE_MAP_H
#include <stdbool.h>
#include <stdint.h>

#define MAZE_WIDTH 160
#define MAZE_HEIGHT 112
#define MAZE_BYTES (MAZE_WIDTH * MAZE_HEIGHT / 8)
#define MAZE_SPAWN_X 12
#define MAZE_SPAWN_Y 16
extern const uint8_t maze_map01[MAZE_BYTES];
bool Maze_Wall(int x, int y);
bool Maze_Free(int center_x, int center_y);
bool Maze_SweepFree(int x0, int y0, int x1, int y1);
bool Maze_Exit(int center_x, int center_y);
#endif
