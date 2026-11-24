#include "maze_game.h"
#include "maze_map.h"
#include "game_config.h"
#include <string.h>

/* Regras puras: recebem entrada/tempo e alteram apenas a estrutura Game. */

static int ToPixel(int32_t milli)
{
    return milli >= 0 ? (milli + 500) / 1000 : (milli - 500) / 1000;
}
int Game_X(const Game *game)
{
    return ToPixel(game->x_milli);
}
int Game_Y(const Game *game)
{
    return ToPixel(game->y_milli);
}

/* scene muda em cada transicao: a tarefa do display confirma esse numero. */
static void EnterState(Game *game, GameState next)
{
    game->state = next;
    ++game->scene;
    game->state_ms = 0;
    game->neutral_ms = 0;
}

void Game_Init(Game *game)
{
    memset(game, 0, sizeof(*game));
    game->state = GAME_CALIBRATING;
    game->scene = 1;
}

static void PlaceAtStart(Game *game)
{
    const MazeMap *map = Maze_Get(game->map_index);
    game->x_milli = map->spawn_x * 1000;
    game->y_milli = map->spawn_y * 1000;
}

static void PrepareMap(Game *game)
{
    game->remaining_ms = Maze_Get(game->map_index)->limit_ms;
    game->active_ms = 0;
    game->collisions = 0;
    game->timeouts = 0;
    game->map_score = 0;
    game->reason = GAME_NO_ERROR;
    game->pause_choice = 0;
    game->menu_latched = false;
    game->countdown_ms = 3000;
    PlaceAtStart(game);
    EnterState(game, GAME_READY);
}

static void StartCampaign(Game *game)
{
    uint32_t scene = game->scene;
    memset(game, 0, sizeof(*game));
    game->scene = scene;
    game->lives = 3;
    PrepareMap(game);
}

void Game_Fail(Game *game, GameReason reason)
{
    if (game->state != GAME_FAULT)
    {
        game->reason = reason;
        EnterState(game, GAME_FAULT);
    }
}

uint32_t Game_Score(const MazeMap *map, uint32_t active_ms, unsigned collisions, unsigned timeouts)
{
    uint32_t seconds = active_ms / 1000U + (active_ms % 1000U != 0U);
    uint32_t limit = map->limit_ms / 1000U;
    int64_t result = 500 + 5 * (int64_t)(seconds < limit ? limit - seconds : 0);
    if (collisions == 0 && timeouts == 0)
    {
        result += 200;
    }
    result -= 100 * (int64_t)collisions + 200 * (int64_t)timeouts;
    return result > 0 ? (uint32_t)(result * map->difficulty) : 0;
}

static void ApplyPenalty(Game *game, GameReason reason)
{
    game->reason = reason;
    if (reason == GAME_WALL)
    {
        ++game->collisions;
    }
    else
    {
        ++game->timeouts;
    }
    if (game->lives > 0)
    {
        --game->lives;
    }
    EnterState(game, game->lives == 0 ? GAME_OVER : GAME_HIT);
}

/* Raiz quadrada inteira: preserva a normalizacao diagonal sem usar ponto flutuante. */
static unsigned IntegerSquareRoot(unsigned value)
{
    unsigned result = 0, bit = 1U << 30;
    while (bit > value)
    {
        bit >>= 2;
    }
    while (bit != 0)
    {
        if (value >= result + bit)
        {
            value -= result + bit;
            result = (result >> 1) + bit;
        }
        else
        {
            result >>= 1;
        }
        bit >>= 2;
    }
    return result;
}

/* Posicao em milesimos de pixel. Entrada -1000..1000; tempo em milissegundos. */
static void CalculateDestination(const Game *game, const GameInput *input, uint32_t elapsed_ms,
                                 int32_t *next_x, int32_t *next_y)
{
    int32_t x = input->x, y = input->y;
    unsigned squared_length = (unsigned)(x * x + y * y);
    if (squared_length > 1000000U)
    {
        unsigned length = IntegerSquareRoot(squared_length) + 1U;
        x = x * 1000 / (int32_t)length;
        y = y * 1000 / (int32_t)length;
    }
    *next_x = game->x_milli + x * GAME_SPEED_PX_S * (int32_t)elapsed_ms / 1000;
    *next_y = game->y_milli + y * GAME_SPEED_PX_S * (int32_t)elapsed_ms / 1000;
}

static void EnterTutorialStep(Game *game, TutorialStep step)
{
    game->tutorial_step = step;
    EnterState(game, GAME_HELP);
}
static void PlaceInTutorial(Game *game)
{
    game->x_milli = maze_tutorial.spawn_x * 1000;
    game->y_milli = maze_tutorial.spawn_y * 1000;
}
static void UpdateTutorial(Game *game, const GameInput *input, uint32_t elapsed_ms)
{
    if (!input->render_ready)
    {
        return;
    }
    if (input->click)
    {
        if (game->tutorial_step == TUTORIAL_PAUSE)
        {
            EnterTutorialStep(game, TUTORIAL_RESUME);
        }
        else if (game->tutorial_step == TUTORIAL_RESUME)
        {
            EnterTutorialStep(game, TUTORIAL_DONE);
        }
        else
        {
            game->title_choice = 0;
            game->menu_latched = true;
            EnterState(game, GAME_TITLE);
        }
        return;
    }
    if (game->tutorial_step >= TUTORIAL_PAUSE)
    {
        return;
    }
    if (game->tutorial_step == TUTORIAL_IMPACT)
    {
        if (game->state_ms >= 700)
        {
            PlaceInTutorial(game);
            EnterTutorialStep(game, TUTORIAL_EXIT);
        }
        return;
    }
    if (game->tutorial_wait_ms)
    {
        if (elapsed_ms >= game->tutorial_wait_ms)
        {
            game->tutorial_wait_ms = 0;
            EnterState(game, GAME_HELP);
        }
        else
        {
            game->tutorial_wait_ms -= elapsed_ms;
        }
        return;
    }
    if (game->neutral_ms < 200)
    {
        if (input->x || input->y)
        {
            game->neutral_ms = 0;
        }
        else
        {
            game->neutral_ms += elapsed_ms;
        }
        return;
    }
    if (game->tutorial_step == TUTORIAL_CENTER)
    {
        EnterTutorialStep(game, TUTORIAL_MOVE);
        return;
    }
    int32_t next_x, next_y;
    CalculateDestination(game, input, elapsed_ms, &next_x, &next_y);
    if (!Maze_SweepFree(&maze_tutorial, Game_X(game), Game_Y(game), ToPixel(next_x),
                        ToPixel(next_y)))
    {
        if (game->tutorial_step == TUTORIAL_WALL)
        {
            EnterTutorialStep(game, TUTORIAL_IMPACT);
        }
        else
        {
            PlaceInTutorial(game);
            game->tutorial_wait_ms = 600;
            EnterState(game, GAME_HELP);
        }
        return; /* Practice never touches campaign lives, points or penalties. */
    }
    game->x_milli = next_x;
    game->y_milli = next_y;
    if (game->tutorial_step == TUTORIAL_MOVE && Game_X(game) >= 65 && Game_X(game) <= 71 &&
        Game_Y(game) >= 55 && Game_Y(game) <= 61)
    {
        EnterTutorialStep(game, TUTORIAL_WALL);
    }
    if (game->tutorial_step == TUTORIAL_EXIT &&
        Maze_Exit(&maze_tutorial, Game_X(game), Game_Y(game)))
    {
        EnterTutorialStep(game, TUTORIAL_PAUSE);
    }
}

static void UpdateTitle(Game *game, const GameInput *input)
{
    if (!input->render_ready)
    {
        return;
    }
    if (input->y == 0)
    {
        game->menu_latched = false;
    }
    if (!game->menu_latched && (input->y > 500 || input->y < -500))
    {
        game->title_choice ^= 1U;
        game->menu_latched = true;
    }
    if (input->click && input->calibrated)
    {
        if (game->title_choice)
        {
            game->tutorial_wait_ms = 0;
            PlaceInTutorial(game);
            EnterTutorialStep(game, TUTORIAL_CENTER);
        }
        else
        {
            StartCampaign(game);
        }
    }
}

static void UpdateResult(Game *game, const GameInput *input)
{
    /* One acknowledged result consumes one click; no input leaks into play. */
    if (input->click && input->render_ready)
    {
        if (game->map_index + 1U < MAZE_COUNT)
        {
            ++game->map_index;
            PrepareMap(game);
        }
        else
        {
            EnterState(game, GAME_WON);
        }
    }
}

static void UpdateCountdown(Game *game, const GameInput *input, uint32_t elapsed_ms)
{
    if (!input->render_ready)
    {
        return;
    }
    if (input->x != 0 || input->y != 0)
    {
        game->neutral_ms = 0;
        return;
    }
    if (game->neutral_ms < 200)
    {
        game->neutral_ms += elapsed_ms;
        return;
    }
    if (game->countdown_ms > elapsed_ms)
    {
        game->countdown_ms -= elapsed_ms;
    }
    else
    {
        game->countdown_ms = 0;
        EnterState(game, GAME_PLAYING);
    }
}

static void UpdatePlaying(Game *game, const GameInput *input, uint32_t elapsed_ms)
{
    /* Do not move until the overlay was removed by the display owner. */
    if (!input->render_ready)
    {
        return;
    }
    game->active_ms += elapsed_ms;
    if (elapsed_ms >= game->remaining_ms)
    {
        game->remaining_ms = 0;
        ApplyPenalty(game, GAME_TIMEOUT);
        return;
    }
    game->remaining_ms -= elapsed_ms;
    if (input->click)
    {
        game->pause_choice = 0;
        game->menu_latched = false;
        EnterState(game, GAME_PAUSED);
        return;
    }
    int32_t next_x, next_y;
    CalculateDestination(game, input, elapsed_ms, &next_x, &next_y);
    if (!Maze_SweepFree(Maze_Get(game->map_index), Game_X(game), Game_Y(game), ToPixel(next_x),
                        ToPixel(next_y)))
    {
        ApplyPenalty(game, GAME_WALL);
        return;
    }
    game->x_milli = next_x;
    game->y_milli = next_y;
    if (Maze_Exit(Maze_Get(game->map_index), Game_X(game), Game_Y(game)))
    {
        game->map_score = Game_Score(Maze_Get(game->map_index), game->active_ms, game->collisions,
                                     game->timeouts);
        game->score += game->map_score;
        EnterState(game, GAME_RESULT);
    }
}

static void UpdatePause(Game *game, const GameInput *input)
{
    if (input->y == 0)
    {
        game->menu_latched = false;
    }
    if (!game->menu_latched && (input->y > 500 || input->y < -500))
    {
        game->pause_choice ^= 1U;
        game->menu_latched = true;
    }
    if (input->click)
    {
        if (game->pause_choice)
        {
            game->title_choice = 0;
            game->menu_latched = true;
            EnterState(game, GAME_TITLE);
        }
        else
        {
            game->countdown_ms = 0;
            EnterState(game, GAME_READY);
        }
    }
}

static void UpdateHit(Game *game)
{
    if (game->state_ms >= 600)
    {
        if (game->reason == GAME_TIMEOUT)
        {
            game->remaining_ms = Maze_Get(game->map_index)->limit_ms;
        }
        PlaceAtStart(game);
        game->countdown_ms = 0;
        EnterState(game, GAME_READY);
    }
}

/* Cada estado tem uma rotina curta; nenhuma delas espera ou acessa o RTOS. */
void Game_Tick(Game *game, const GameInput *input, uint32_t elapsed_ms)
{
    if (game->state == GAME_FAULT)
    {
        return;
    }
    if (game->state != GAME_CALIBRATING && !input->valid)
    {
        Game_Fail(game, GAME_ADC_ERROR);
        return;
    }
    if ((game->state == GAME_PLAYING || game->state == GAME_HELP) && elapsed_ms > 50)
    {
        Game_Fail(game, GAME_TIMING_ERROR);
        return;
    }
    game->state_ms += elapsed_ms;
    switch (game->state)
    {
    case GAME_CALIBRATING:
        if (input->valid && input->calibrated)
        {
            EnterState(game, GAME_TITLE);
        }
        break;
    case GAME_TITLE:
        UpdateTitle(game, input);
        break;
    case GAME_HELP:
        UpdateTutorial(game, input, elapsed_ms);
        break;
    case GAME_WON:
    case GAME_OVER:
        if (input->click && input->calibrated && input->render_ready)
        {
            StartCampaign(game);
        }
        break;
    case GAME_RESULT:
        UpdateResult(game, input);
        break;
    case GAME_READY:
        UpdateCountdown(game, input, elapsed_ms);
        break;
    case GAME_PLAYING:
        UpdatePlaying(game, input, elapsed_ms);
        break;
    case GAME_PAUSED:
        UpdatePause(game, input);
        break;
    case GAME_HIT:
        UpdateHit(game);
        break;
    default:
        break;
    }
}
