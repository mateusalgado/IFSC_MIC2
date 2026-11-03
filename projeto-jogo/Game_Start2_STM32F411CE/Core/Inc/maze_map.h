#ifndef MAZE_MAP_H
#define MAZE_MAP_H
#include <stdbool.h>
#include <stdint.h>

#define MAZE_WIDTH 160
#define MAZE_HEIGHT 112
#define MAZE_BYTES (MAZE_WIDTH * MAZE_HEIGHT / 8)
#define MAZE_COUNT 9U
typedef struct {
    const uint8_t *walls;
    const char *name;
    uint32_t limit_ms;
    uint8_t id, difficulty, corridor_width;
    uint8_t spawn_x, spawn_y, goal_x, goal_y;
} MazeMap;
extern const MazeMap maze_maps[MAZE_COUNT];
extern const MazeMap maze_tutorial; /* Practice arena; excluded from campaign. */
/* Index is the campaign position (0..8); displayed ids are 1..9. */
const MazeMap *Maze_Get(unsigned index);
const char *Maze_Difficulty(const MazeMap *map);
bool Maze_Wall(const MazeMap *map, int x, int y);
bool Maze_Free(const MazeMap *map, int center_x, int center_y);
bool Maze_SweepFree(const MazeMap *map, int x0, int y0, int x1, int y1);
bool Maze_Exit(const MazeMap *map, int center_x, int center_y);
#endif
