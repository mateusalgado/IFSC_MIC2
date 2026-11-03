#include "maze_game.h"
#include "maze_map.h"
#include "game_config.h"
#include <string.h>

static int pixel(int32_t milli) { return milli >= 0 ? (milli + 500) / 1000 : (milli - 500) / 1000; }
int Game_X(const Game *g) { return pixel(g->x_milli); }
int Game_Y(const Game *g) { return pixel(g->y_milli); }

static void state(Game *g, GameState next)
{
    g->state = next; ++g->scene; g->state_ms = 0; g->neutral_ms = 0;
}

void Game_Init(Game *g)
{
    memset(g, 0, sizeof(*g));
    g->state = GAME_CALIBRATING;
    g->scene = 1;
}

static void spawn(Game *g)
{
    const MazeMap *map = Maze_Get(g->map_index);
    g->x_milli = map->spawn_x * 1000; g->y_milli = map->spawn_y * 1000;
}

static void prepare_map(Game *g)
{
    g->remaining_ms = Maze_Get(g->map_index)->limit_ms;
    g->active_ms = 0; g->collisions = 0; g->timeouts = 0; g->map_score = 0;
    g->reason = GAME_NO_ERROR; g->pause_choice = 0; g->menu_latched = false;
    g->countdown_ms = 3000;
    spawn(g); state(g, GAME_READY);
}

static void start(Game *g)
{
    uint32_t scene = g->scene;
    memset(g, 0, sizeof(*g));
    g->scene = scene;
    g->lives = 3;
    prepare_map(g);
}

void Game_Fail(Game *g, GameReason reason)
{
    if (g->state != GAME_FAULT) { g->reason = reason; state(g, GAME_FAULT); }
}

uint32_t Game_Score(const MazeMap *map, uint32_t active_ms, unsigned collisions, unsigned timeouts)
{
    uint32_t seconds = active_ms / 1000U + (active_ms % 1000U != 0U);
    uint32_t limit = map->limit_ms / 1000U;
    int64_t result = 500 + 5 * (int64_t)(seconds < limit ? limit - seconds : 0);
    if (collisions == 0 && timeouts == 0) result += 200;
    result -= 100 * (int64_t)collisions + 200 * (int64_t)timeouts;
    return result > 0 ? (uint32_t)(result * map->difficulty) : 0;
}

static void hit(Game *g, GameReason reason)
{
    g->reason = reason;
    if (reason == GAME_WALL) ++g->collisions; else ++g->timeouts;
    if (g->lives > 0) --g->lives;
    state(g, g->lives == 0 ? GAME_OVER : GAME_HIT);
}

static unsigned root(unsigned value)
{
    unsigned result = 0, bit = 1U << 30;
    while (bit > value) bit >>= 2;
    while (bit != 0) {
        if (value >= result + bit) { value -= result + bit; result = (result >> 1) + bit; }
        else result >>= 1;
        bit >>= 2;
    }
    return result;
}

static void destination(const Game *g, const GameInput *in, uint32_t dt, int32_t *nx, int32_t *ny)
{
    int32_t x = in->x, y = in->y;
    unsigned norm2 = (unsigned)(x*x + y*y);
    if (norm2 > 1000000U) {
        unsigned length = root(norm2) + 1U;
        x = x * 1000 / (int32_t)length; y = y * 1000 / (int32_t)length;
    }
    *nx = g->x_milli + x * GAME_SPEED_PX_S * (int32_t)dt / 1000;
    *ny = g->y_milli + y * GAME_SPEED_PX_S * (int32_t)dt / 1000;
}

static void tutorial_step(Game *g, TutorialStep step)
{
    g->tutorial_step = step;
    state(g, GAME_HELP);
}
static void tutorial_spawn(Game *g)
{
    g->x_milli = maze_tutorial.spawn_x * 1000;
    g->y_milli = maze_tutorial.spawn_y * 1000;
}
static void tutorial(Game *g, const GameInput *in, uint32_t dt)
{
    if (!in->render_ready) return;
    if (in->click) {
        if (g->tutorial_step == TUTORIAL_PAUSE) tutorial_step(g, TUTORIAL_RESUME);
        else if (g->tutorial_step == TUTORIAL_RESUME) tutorial_step(g, TUTORIAL_DONE);
        else { g->title_choice = 0; g->menu_latched = true; state(g, GAME_TITLE); }
        return;
    }
    if (g->tutorial_step >= TUTORIAL_PAUSE) return;
    if (g->tutorial_step == TUTORIAL_IMPACT) {
        if (g->state_ms >= 700) { tutorial_spawn(g); tutorial_step(g, TUTORIAL_EXIT); }
        return;
    }
    if (g->tutorial_wait_ms) {
        if (dt >= g->tutorial_wait_ms) {
            g->tutorial_wait_ms = 0; state(g, GAME_HELP);
        } else g->tutorial_wait_ms -= dt;
        return;
    }
    if (g->neutral_ms < 200) {
        if (in->x || in->y) g->neutral_ms = 0;
        else g->neutral_ms += dt;
        return;
    }
    if (g->tutorial_step == TUTORIAL_CENTER) {
        tutorial_step(g, TUTORIAL_MOVE); return;
    }
    int32_t nx, ny; destination(g, in, dt, &nx, &ny);
    if (!Maze_SweepFree(&maze_tutorial, Game_X(g), Game_Y(g), pixel(nx), pixel(ny))) {
        if (g->tutorial_step == TUTORIAL_WALL) tutorial_step(g, TUTORIAL_IMPACT);
        else {
            tutorial_spawn(g); g->tutorial_wait_ms = 600; state(g, GAME_HELP);
        }
        return; /* Practice never touches campaign lives, points or penalties. */
    }
    g->x_milli = nx; g->y_milli = ny;
    if (g->tutorial_step == TUTORIAL_MOVE && Game_X(g) >= 65 && Game_X(g) <= 71 &&
        Game_Y(g) >= 55 && Game_Y(g) <= 61) tutorial_step(g, TUTORIAL_WALL);
    if (g->tutorial_step == TUTORIAL_EXIT && Maze_Exit(&maze_tutorial, Game_X(g), Game_Y(g)))
        tutorial_step(g, TUTORIAL_PAUSE);
}

void Game_Tick(Game *g, const GameInput *in, uint32_t dt)
{
    if (g->state == GAME_FAULT) return;
    if (g->state != GAME_CALIBRATING && !in->valid) {
        Game_Fail(g, GAME_ADC_ERROR); return;
    }
    if ((g->state == GAME_PLAYING || g->state == GAME_HELP) && dt > 50) {
        Game_Fail(g, GAME_TIMING_ERROR); return;
    }
    g->state_ms += dt;
    switch (g->state) {
    case GAME_CALIBRATING:
        if (in->valid && in->calibrated) state(g, GAME_TITLE);
        break;
    case GAME_TITLE:
        if (!in->render_ready) break;
        if (in->y == 0) g->menu_latched = false;
        if (!g->menu_latched && (in->y > 500 || in->y < -500)) {
            g->title_choice ^= 1U; g->menu_latched = true;
        }
        if (in->click && in->calibrated) {
            if (g->title_choice) {
                g->tutorial_wait_ms = 0; tutorial_spawn(g); tutorial_step(g, TUTORIAL_CENTER);
            }
            else start(g);
        }
        break;
    case GAME_HELP:
        tutorial(g, in, dt);
        break;
    case GAME_WON:
    case GAME_OVER:
        if (in->click && in->calibrated && in->render_ready) start(g);
        break;
    case GAME_RESULT:
        /* One acknowledged result consumes one click; no input leaks into play. */
        if (in->click && in->render_ready) {
            if (g->map_index + 1U < MAZE_COUNT) { ++g->map_index; prepare_map(g); }
            else state(g, GAME_WON);
        }
        break;
    case GAME_READY:
        if (!in->render_ready) break;
        if (in->x != 0 || in->y != 0) { g->neutral_ms = 0; break; }
        if (g->neutral_ms < 200) { g->neutral_ms += dt; break; }
        if (g->countdown_ms > dt) g->countdown_ms -= dt;
        else { g->countdown_ms = 0; state(g, GAME_PLAYING); }
        break;
    case GAME_PLAYING: {
        /* Do not move until the overlay was removed by the display owner. */
        if (!in->render_ready) break;
        g->active_ms += dt;
        if (dt >= g->remaining_ms) { g->remaining_ms = 0; hit(g, GAME_TIMEOUT); break; }
        g->remaining_ms -= dt;
        if (in->click) { g->pause_choice = 0; g->menu_latched = false; state(g, GAME_PAUSED); break; }
        int32_t nx, ny; destination(g, in, dt, &nx, &ny);
        if (!Maze_SweepFree(Maze_Get(g->map_index), Game_X(g), Game_Y(g), pixel(nx), pixel(ny))) {
            hit(g, GAME_WALL); break;
        }
        g->x_milli = nx; g->y_milli = ny;
        if (Maze_Exit(Maze_Get(g->map_index), Game_X(g), Game_Y(g))) {
            g->map_score = Game_Score(Maze_Get(g->map_index), g->active_ms, g->collisions, g->timeouts);
            g->score += g->map_score;
            state(g, GAME_RESULT);
        }
        break;
    }
    case GAME_PAUSED:
        if (in->y == 0) g->menu_latched = false;
        if (!g->menu_latched && (in->y > 500 || in->y < -500)) {
            g->pause_choice ^= 1U; g->menu_latched = true;
        }
        if (in->click) {
            if (g->pause_choice) {
                g->title_choice = 0; g->menu_latched = true; state(g, GAME_TITLE);
            }
            else { g->countdown_ms = 0; state(g, GAME_READY); }
        }
        break;
    case GAME_HIT:
        if (g->state_ms >= 600) {
            if (g->reason == GAME_TIMEOUT) g->remaining_ms = Maze_Get(g->map_index)->limit_ms;
            spawn(g); g->countdown_ms = 0; state(g, GAME_READY);
        }
        break;
    default: break;
    }
}
