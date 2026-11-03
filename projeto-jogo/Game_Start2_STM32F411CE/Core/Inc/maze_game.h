#ifndef MAZE_GAME_H
#define MAZE_GAME_H
#include <stdbool.h>
#include <stdint.h>
#include "maze_map.h"

typedef enum {
    GAME_CALIBRATING, GAME_TITLE, GAME_READY, GAME_PLAYING, GAME_PAUSED,
    GAME_HIT, GAME_RESULT, GAME_WON, GAME_OVER, GAME_FAULT, GAME_HELP
} GameState;
typedef enum {
    TUTORIAL_CENTER, TUTORIAL_MOVE, TUTORIAL_WALL, TUTORIAL_IMPACT,
    TUTORIAL_EXIT, TUTORIAL_PAUSE, TUTORIAL_RESUME, TUTORIAL_DONE
} TutorialStep;

typedef enum { GAME_NO_ERROR, GAME_WALL, GAME_TIMEOUT, GAME_ADC_ERROR,
               GAME_DISPLAY_ERROR, GAME_TIMING_ERROR, GAME_QUEUE_ERROR } GameReason;
typedef struct {
    int16_t x, y; /* -1000..1000 */
    bool calibrated, valid, click, render_ready;
} GameInput;
typedef struct {
    GameState state;
    GameReason reason;
    uint32_t scene, state_ms, neutral_ms, countdown_ms;
    uint32_t remaining_ms, active_ms, score, map_score, tutorial_wait_ms;
    uint16_t collisions, timeouts;
    int32_t x_milli, y_milli;
    uint8_t lives, pause_choice, map_index, title_choice;
    bool menu_latched;
    TutorialStep tutorial_step;
} Game;
void Game_Init(Game *game);
void Game_Fail(Game *game, GameReason reason);
void Game_Tick(Game *game, const GameInput *input, uint32_t dt_ms);
int Game_X(const Game *game);
int Game_Y(const Game *game);
uint32_t Game_Score(const MazeMap *map, uint32_t active_ms, unsigned collisions, unsigned timeouts);
#endif
