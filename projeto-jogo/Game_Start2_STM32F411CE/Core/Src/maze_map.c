#include "maze_map.h"
#include <stdlib.h>

const MazeMap *Maze_Get(unsigned index)
{
    return &maze_maps[index < MAZE_COUNT ? index : 0];
}

const char *Maze_Difficulty(const MazeMap *map)
{
    return map->difficulty == 1 ? "FACIL" : map->difficulty == 2 ? "MEDIO" : "DIFICIL";
}

bool Maze_Wall(const MazeMap *map, int x, int y)
{
    if (x < 0 || y < 0 || x >= MAZE_WIDTH || y >= MAZE_HEIGHT) return true;
    unsigned bit = (unsigned)(y * MAZE_WIDTH + x);
    return (map->walls[bit / 8U] & (1U << (bit % 8U))) != 0U;
}

bool Maze_Free(const MazeMap *map, int x, int y)
{
    for (int dy = -2; dy <= 2; ++dy)
        for (int dx = -2; dx <= 2; ++dx)
            if (Maze_Wall(map, x + dx, y + dy)) return false;
    return true;
}

bool Maze_SweepFree(const MazeMap *map, int x0, int y0, int x1, int y1)
{
    /* Pixel supercover: include both side cells when crossing a corner. */
    int nx = abs(x1 - x0), ny = abs(y1 - y0);
    int sx = x1 > x0 ? 1 : -1, sy = y1 > y0 ? 1 : -1;
    int ix = 0, iy = 0, x = x0, y = y0;
    if (!Maze_Free(map, x, y)) return false;
    while (ix < nx || iy < ny) {
        int a = (1 + 2 * ix) * ny, b = (1 + 2 * iy) * nx;
        if (a == b) {
            if (!Maze_Free(map, x + sx, y) || !Maze_Free(map, x, y + sy)) return false;
            x += sx; y += sy; ++ix; ++iy;
        } else if (a < b) { x += sx; ++ix; }
        else { y += sy; ++iy; }
        if (!Maze_Free(map, x, y)) return false;
    }
    return true;
}

bool Maze_Exit(const MazeMap *map, int x, int y)
{
    /* Every goal chamber is 14x14; the entire 5x5 player must fit. */
    return x - 2 >= map->goal_x - 7 && x + 2 < map->goal_x + 7 &&
           y - 2 >= map->goal_y - 7 && y + 2 < map->goal_y + 7 && Maze_Free(map, x, y);
}
