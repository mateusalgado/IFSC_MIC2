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
    g->x_milli = MAZE_SPAWN_X * 1000; g->y_milli = MAZE_SPAWN_Y * 1000;
}

static void start(Game *g)
{
    uint32_t scene = g->scene;
    memset(g, 0, sizeof(*g));
    g->scene = scene;
    g->lives = 3; g->remaining_ms = GAME_LIMIT_MS; g->countdown_ms = 3000;
    spawn(g); state(g, GAME_READY);
}

void Game_Fail(Game *g, GameReason reason)
{
    if (g->state != GAME_FAULT) { g->reason = reason; state(g, GAME_FAULT); }
}

uint32_t Game_Score(uint32_t active_ms, unsigned collisions, unsigned timeouts)
{
    uint32_t seconds = active_ms / 1000U + (active_ms % 1000U != 0U);
    int64_t result = 500 + 5 * (int64_t)(seconds < 60 ? 60 - seconds : 0);
    if (collisions == 0 && timeouts == 0) result += 200;
    result -= 100 * (int64_t)collisions + 200 * (int64_t)timeouts;
    return result > 0 ? (uint32_t)result : 0;
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

void Game_Tick(Game *g, const GameInput *in, uint32_t dt)
{
    if (g->state == GAME_FAULT) return;
    if (g->state != GAME_CALIBRATING && !in->valid) {
        Game_Fail(g, GAME_ADC_ERROR); return;
    }
    if (g->state == GAME_PLAYING && dt > 50) {
        Game_Fail(g, GAME_TIMING_ERROR); return;
    }
    g->state_ms += dt;
    switch (g->state) {
    case GAME_CALIBRATING:
        if (in->valid && in->calibrated) state(g, GAME_TITLE);
        break;
    case GAME_TITLE:
    case GAME_WON:
    case GAME_OVER:
        if (in->click && in->calibrated) start(g);
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
        int32_t x = in->x, y = in->y;
        unsigned norm2 = (unsigned)(x*x + y*y);
        if (norm2 > 1000000U) {
            unsigned length = root(norm2) + 1U;
            x = x * 1000 / (int32_t)length; y = y * 1000 / (int32_t)length;
        }
        int32_t nx = g->x_milli + x * GAME_SPEED_PX_S * (int32_t)dt / 1000;
        int32_t ny = g->y_milli + y * GAME_SPEED_PX_S * (int32_t)dt / 1000;
        if (!Maze_SweepFree(Game_X(g), Game_Y(g), pixel(nx), pixel(ny))) {
            hit(g, GAME_WALL); break;
        }
        g->x_milli = nx; g->y_milli = ny;
        if (Maze_Exit(Game_X(g), Game_Y(g))) {
            g->score = Game_Score(g->active_ms, g->collisions, g->timeouts);
            state(g, GAME_WON);
        }
        break;
    }
    case GAME_PAUSED:
        if (in->y == 0) g->menu_latched = false;
        if (!g->menu_latched && (in->y > 500 || in->y < -500)) {
            g->pause_choice ^= 1U; g->menu_latched = true;
        }
        if (in->click) {
            if (g->pause_choice) state(g, GAME_TITLE);
            else { g->countdown_ms = 0; state(g, GAME_READY); }
        }
        break;
    case GAME_HIT:
        if (g->state_ms >= 600) {
            if (g->reason == GAME_TIMEOUT) g->remaining_ms = GAME_LIMIT_MS;
            spawn(g); g->countdown_ms = 0; state(g, GAME_READY);
        }
        break;
    default: break;
    }
}
