#include "maze_map.h"
#include <stdlib.h>

bool Maze_Wall(int x, int y)
{
    if (x < 0 || y < 0 || x >= MAZE_WIDTH || y >= MAZE_HEIGHT) return true;
    unsigned bit = (unsigned)(y * MAZE_WIDTH + x);
    return (maze_map01[bit / 8U] & (1U << (bit % 8U))) != 0U;
}

bool Maze_Free(int x, int y)
{
    for (int dy = -2; dy <= 2; ++dy)
        for (int dx = -2; dx <= 2; ++dx)
            if (Maze_Wall(x + dx, y + dy)) return false;
    return true;
}

bool Maze_SweepFree(int x0, int y0, int x1, int y1)
{
    /* Pixel supercover: include both side cells when crossing a corner. */
    int nx = abs(x1 - x0), ny = abs(y1 - y0);
    int sx = x1 > x0 ? 1 : -1, sy = y1 > y0 ? 1 : -1;
    int ix = 0, iy = 0, x = x0, y = y0;
    if (!Maze_Free(x, y)) return false;
    while (ix < nx || iy < ny) {
        int a = (1 + 2 * ix) * ny, b = (1 + 2 * iy) * nx;
        if (a == b) {
            if (!Maze_Free(x + sx, y) || !Maze_Free(x, y + sy)) return false;
            x += sx; y += sy; ++ix; ++iy;
        } else if (a < b) { x += sx; ++ix; }
        else { y += sy; ++iy; }
        if (!Maze_Free(x, y)) return false;
    }
    return true;
}

bool Maze_Exit(int x, int y)
{
    /* Goal chamber is [137,151) x [81,95); entire 5x5 player must fit. */
    return x - 2 >= 137 && x + 2 < 151 && y - 2 >= 81 && y + 2 < 95
        && Maze_Free(x, y);
}
