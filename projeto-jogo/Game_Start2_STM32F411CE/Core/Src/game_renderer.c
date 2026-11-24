#include "game_renderer.h"
#include "game_config.h"
#include "maze_map.h"
#include "st7735.h"
#include <stdio.h>
#include <string.h>

#define BG 0x0843
#define PANEL 0x10a5
#define WALL 0x294a
#define EDGE 0x7bf0
#define WHITE 0xffff
#define GREEN 0x07e0
#define YELLOW 0xffe0
#define RED 0xf800
#define CYAN 0x2e9f
#define GOLD 0xfdc0
#define PINK 0xf9b2

static Game previous;
static bool first_frame;
/* Oito linhas por transferencia: nao precisamos de um framebuffer de 40 KB. */
static uint16_t pixel_buffer[160 * 8];

static uint16_t DifficultyColor(const MazeMap *map)
{
    switch (map->difficulty)
    {
    case 1:
        return CYAN;
    case 2:
        return GOLD;
    default:
        return PINK;
    }
}
static void DrawCenteredTextOn(int y, const char *value, uint16_t color, uint16_t bg, FontDef font)
{
    int x = (160 - (int)strlen(value) * font.width) / 2;
    if (x >= 0)
    {
        ST7735_WriteString((uint16_t)x, (uint16_t)y, value, font, color, bg);
    }
}
static void DrawCenteredText(int y, const char *value, uint16_t color, FontDef font)
{
    DrawCenteredTextOn(y, value, color, BG, font);
}
static void DrawOutline(int x, int y, int w, int h, uint16_t color)
{
    ST7735_FillRectangle(x, y, w, 1, color);
    ST7735_FillRectangle(x, y + h - 1, w, 1, color);
    ST7735_FillRectangle(x, y, 1, h, color);
    ST7735_FillRectangle(x + w - 1, y, 1, h, color);
}
static void DrawIcon(int x, int y, const uint8_t rows[7], uint16_t color)
{
    for (int row = 0; row < 7; ++row)
    {
        for (int col = 0; col < 7; ++col)
        {
            if (rows[row] & (1U << (6 - col)))
            {
                ST7735_FillRectangle(x + col, y + row, 1, 1, color);
            }
        }
    }
}
static void DrawHeart(int x, uint16_t color)
{
    static const uint8_t rows[7] = {0x36, 0x7f, 0x7f, 0x7f, 0x3e, 0x1c, 0x08};
    DrawIcon(x, 4, rows, color);
}
static void DrawButton(int y, const char *value, bool selected, uint16_t color)
{
    ST7735_FillRectangle(8, y, 144, 16, selected ? PANEL : BG);
    DrawOutline(8, y, 144, 16, selected ? color : WALL);
    DrawCenteredTextOn(y + 3, value, selected ? WHITE : EDGE, selected ? PANEL : BG, Font_7x10);
}
static void DrawProgress(int y, unsigned complete, bool current)
{
    for (unsigned i = 0; i < MAZE_COUNT; ++i)
    {
        int x = 10 + 16 * (int)i;
        uint16_t color = DifficultyColor(Maze_Get(i));
        ST7735_FillRectangle(x, y, 12, 5, i < complete ? color : BG);
        uint16_t border_color = WALL;
        if (i == complete && current)
        {
            border_color = WHITE;
        }
        else if (i < complete)
        {
            border_color = color;
        }
        DrawOutline(x, y, 12, 5, border_color);
    }
}
static void DrawPlayer(const Game *game, uint16_t color)
{
    ST7735_FillRectangle(Game_X(game) - 2, 16 + Game_Y(game) - 2, 5, 5, color);
    ST7735_FillRectangle(Game_X(game), 16 + Game_Y(game), 1, 1, WHITE);
}
static uint16_t BackgroundColor(const MazeMap *map, int x, int y)
{
    if (Maze_Wall(map, x, y))
    {
        /* O contorno claro pertence a parede; nao ocupa o corredor livre. */
        bool edge = !Maze_Wall(map, x - 1, y) || !Maze_Wall(map, x + 1, y) ||
                    !Maze_Wall(map, x, y - 1) || !Maze_Wall(map, x, y + 1);
        return edge ? EDGE : WALL;
    }
    int dx = x - map->goal_x, dy = y - map->goal_y;
    if (dx < 0)
    {
        dx = -dx;
    }
    if (dy < 0)
    {
        dy = -dy;
    }
    return dx + dy == 4 ? YELLOW : BG;
}
/* Reconstroi o fundo sob o jogador antigo, usando a mesma mascara da colisao. */
static void RestoreRegion(const MazeMap *map, int x, int y, int w, int h)
{
    for (int row = 0; row < h; row += 8)
    {
        int rows = h - row < 8 ? h - row : 8;
        for (int j = 0; j < rows; ++j)
        {
            for (int i = 0; i < w; ++i)
            {
                uint16_t color = BackgroundColor(map, x + i, y + row + j);
                pixel_buffer[j * w + i] = (uint16_t)((color << 8) | (color >> 8));
            }
        }
        ST7735_DrawImage(x, 16 + y + row, w, rows, pixel_buffer);
    }
}
static void DrawHud(const Game *game)
{
    static const uint8_t clock[7] = {0x1c, 0x22, 0x49, 0x49, 0x45, 0x22, 0x1c};
    const MazeMap *map = Maze_Get(game->map_index);
    uint16_t time_color = game->remaining_ms <= 5000 ? RED : YELLOW;
    char value[12];
    ST7735_FillRectangle(0, 0, 160, 16, PANEL);
    snprintf(value, sizeof(value), "M%02u", (unsigned)map->id);
    ST7735_WriteString(2, 2, value, Font_7x10, DifficultyColor(map), PANEL);
    for (unsigned i = 0; i < 3; ++i)
    {
        DrawHeart(26 + 9 * i, i < game->lives ? RED : WALL);
    }
    DrawIcon(56, 4, clock, time_color);
    snprintf(value, sizeof(value), "%02lu", (unsigned long)((game->remaining_ms + 999) / 1000));
    ST7735_WriteString(65, 2, value, Font_7x10, time_color, PANEL);
    snprintf(value, sizeof(value), "%05lu", (unsigned long)game->score);
    ST7735_WriteString(112, 2, value, Font_7x10, WHITE, PANEL);
    for (unsigned i = 0; i < MAZE_COUNT; ++i)
    {
        ST7735_FillRectangle(4 + 17 * i, 14, 15, 2,
                             i < game->map_index    ? DifficultyColor(Maze_Get(i))
                             : i == game->map_index ? WHITE
                                                    : WALL);
    }
}

static void DrawCalibration(bool scene_changed)
{
    if (scene_changed)
    {
        DrawCenteredText(20, "CALIBRANDO", WHITE, Font_11x18);
        DrawCenteredText(54, "SOLTE O JOYSTICK", CYAN, Font_7x10);
        DrawCenteredText(73, "MANTENHA NO CENTRO", WHITE, Font_7x10);
        DrawCenteredText(110, GAME_VERSION, EDGE, Font_7x10);
    }
}

static void DrawTitle(const Game *game, bool scene_changed)
{
    if (scene_changed)
    {
        DrawCenteredText(6, "LABIRINTO", CYAN, Font_11x18);
        DrawCenteredText(29, "9 MAPAS / 3 NIVEIS", WHITE, Font_7x10);
        /* Emblema feito com retangulos, sem uma imagem adicional na memoria. */
        ST7735_FillRectangle(30, 48, 47, 3, EDGE);
        ST7735_FillRectangle(74, 48, 3, 16, EDGE);
        ST7735_FillRectangle(74, 61, 55, 3, EDGE);
        ST7735_FillRectangle(28, 46, 7, 7, GREEN);
        DrawOutline(125, 58, 9, 9, YELLOW);
        DrawCenteredText(116, GAME_VERSION, EDGE, Font_7x10);
    }
    if (scene_changed || game->title_choice != previous.title_choice ||
        game->state_ms / 500 != previous.state_ms / 500)
    {
        uint16_t color = game->state_ms / 500 % 2 ? CYAN : WHITE;
        DrawButton(76, "JOGAR", game->title_choice == 0, color);
        DrawButton(95, "COMO JOGAR", game->title_choice == 1, color);
    }
}

/* O texto segue a mesma ordem de prioridade das etapas do tutorial. */
static const char *TutorialHint(const Game *game)
{
    switch (game->tutorial_step)
    {
    case TUTORIAL_CENTER:
        return "SOLTE O JOYSTICK";
    case TUTORIAL_IMPACT:
        return "NO JOGO: -1 VIDA";
    case TUTORIAL_PAUSE:
        return "CLIQUE PARA PAUSAR";
    case TUTORIAL_RESUME:
        return "PAUSADO: CLIQUE";
    default:
        break;
    }
    if (game->tutorial_wait_ms != 0)
    {
        return "TENTE DE NOVO";
    }
    if (game->neutral_ms < 200)
    {
        return "CENTRALIZE";
    }
    if (game->tutorial_step == TUTORIAL_MOVE)
    {
        return "LEVE ATE O ALVO";
    }
    if (game->tutorial_step == TUTORIAL_WALL)
    {
        return "TOQUE NA PAREDE";
    }
    return "CONTORNE A PAREDE";
}

static unsigned TutorialLesson(TutorialStep step)
{
    if (step <= TUTORIAL_MOVE)
    {
        return 1;
    }
    if (step <= TUTORIAL_IMPACT)
    {
        return 2;
    }
    if (step == TUTORIAL_EXIT)
    {
        return 3;
    }
    return 4;
}

static const char *FaultMessage(GameReason reason)
{
    switch (reason)
    {
    case GAME_ADC_ERROR:
        return "ADC SEM DADOS";
    case GAME_TIMING_ERROR:
        return "ATRASO DA LOGICA";
    case GAME_QUEUE_ERROR:
        return "FILA CHEIA";
    default:
        return "ERRO NO DISPLAY";
    }
}

static void DrawTutorial(const Game *game, bool scene_changed)
{
    MazeMap arena = maze_tutorial;
    if (game->tutorial_step == TUTORIAL_MOVE)
    {
        arena.goal_x = 68;
    }
    if (game->tutorial_step == TUTORIAL_DONE)
    {
        if (scene_changed)
        {
            DrawCenteredText(12, "TREINO OK!", GREEN, Font_11x18);
            DrawCenteredText(44, "MOVIMENTO E PAREDES", WHITE, Font_7x10);
            DrawCenteredText(62, "SAIDA E PAUSA", WHITE, Font_7x10);
            DrawCenteredText(82, "AGORA E SUA VEZ", CYAN, Font_7x10);
            DrawButton(108, "VOLTAR AO MENU", true, CYAN);
        }
        return;
    }
    if (scene_changed)
    {
        unsigned lesson = TutorialLesson(game->tutorial_step);
        char title[24];
        snprintf(title, sizeof(title), "TREINO %u/4", lesson);
        DrawCenteredText(4, title, CYAN, Font_11x18);
        RestoreRegion(&arena, 8, 20, 144, 64);
        /* O obstaculo visual esta dentro da mascara usada para testar colisao. */
        ST7735_FillRectangle(94, 56, 8, 22, RED);
        DrawPlayer(game, game->tutorial_step == TUTORIAL_IMPACT ? RED : GREEN);
        DrawCenteredText(
            110, game->tutorial_step >= TUTORIAL_PAUSE ? "CLIQUE: CONTINUAR" : "CLIQUE: SAIR", EDGE,
            Font_7x10);
    }
    if (scene_changed || (game->neutral_ms >= 200) != (previous.neutral_ms >= 200))
    {
        const char *hint = TutorialHint(game);
        ST7735_FillRectangle(0, 25, 160, 10, BG);
        DrawCenteredText(25, hint,
                         game->tutorial_step == TUTORIAL_IMPACT || game->tutorial_wait_ms ? RED
                                                                                          : WHITE,
                         Font_7x10);
    }
    if (!scene_changed && (Game_X(game) != Game_X(&previous) || Game_Y(game) != Game_Y(&previous)))
    {
        RestoreRegion(&arena, Game_X(&previous) - 2, Game_Y(&previous) - 2, 5, 5);
        DrawPlayer(game, GREEN);
    }
}

static void DrawCountdown(const Game *game, const MazeMap *map, bool scene_changed)
{
    unsigned count = (game->countdown_ms + 999) / 1000, old = (previous.countdown_ms + 999) / 1000;
    if (scene_changed)
    {
        char title[24];
        snprintf(title, sizeof(title), "FASE %u/%u", (unsigned)game->map_index + 1U, MAZE_COUNT);
        DrawCenteredText(6, title, WHITE, Font_11x18);
        DrawProgress(28, game->map_index, true);
        DrawCenteredText(42, map->name, WHITE, Font_7x10);
        snprintf(title, sizeof(title), "%s / %lu s", Maze_Difficulty(map),
                 (unsigned long)(map->limit_ms / 1000));
        DrawCenteredText(58, title, DifficultyColor(map), Font_7x10);
        DrawCenteredText(116, "SOLTE O JOYSTICK", EDGE, Font_7x10);
    }
    if (scene_changed || count != old || (game->neutral_ms >= 200) != (previous.neutral_ms >= 200))
    {
        ST7735_FillRectangle(0, 76, 160, 35, BG);
        if (game->neutral_ms < 200)
        {
            DrawCenteredText(84, "CENTRALIZE", YELLOW, Font_11x18);
        }
        else if (count)
        {
            char n[2] = {(char)('0' + count), 0};
            DrawCenteredText(78, n, DifficultyColor(map), Font_16x26);
        }
        else
        {
            DrawCenteredText(84, "PRONTO", GREEN, Font_11x18);
        }
    }
}

static void DrawPlaying(const Game *game, const MazeMap *map, bool scene_changed)
{
    if (scene_changed || Game_X(game) != Game_X(&previous) || Game_Y(game) != Game_Y(&previous))
    {
        if (!scene_changed)
        {
            RestoreRegion(map, Game_X(&previous) - 2, Game_Y(&previous) - 2, 5, 5);
        }
        DrawPlayer(game, GREEN);
    }
    if (!scene_changed &&
        ((game->remaining_ms + 999) / 1000 != (previous.remaining_ms + 999) / 1000 ||
         game->lives != previous.lives || game->score != previous.score))
    {
        DrawHud(game);
    }
}

static void DrawPause(const Game *game, const MazeMap *map, bool scene_changed)
{
    if (scene_changed || game->pause_choice != previous.pause_choice)
    {
        ST7735_FillRectangle(6, 28, 148, 82, BG);
        DrawOutline(6, 28, 148, 82, DifficultyColor(map));
        DrawCenteredText(34, "PAUSA", WHITE, Font_11x18);
        DrawButton(61, "CONTINUAR", game->pause_choice == 0, DifficultyColor(map));
        DrawButton(82, "SAIR", game->pause_choice == 1, DifficultyColor(map));
    }
}

static void DrawHit(const Game *game, bool scene_changed)
{
    if (scene_changed)
    {
        DrawPlayer(game, RED);
        ST7735_FillRectangle(18, 42, 124, 44, BG);
        DrawOutline(18, 42, 124, 44, RED);
        DrawCenteredText(47, "-1 VIDA", RED, Font_11x18);
        DrawCenteredText(70, game->reason == GAME_TIMEOUT ? "TEMPO ESGOTADO" : "TOCOU NA PAREDE",
                         WHITE, Font_7x10);
    }
}

static void DrawResult(const Game *game, const MazeMap *map, bool scene_changed)
{
    if (scene_changed)
    {
        char value[24];
        DrawCenteredText(4, "FASE OK!", GREEN, Font_11x18);
        snprintf(value, sizeof(value), "FASE %u/%u - %s", (unsigned)game->map_index + 1U,
                 MAZE_COUNT, Maze_Difficulty(map));
        DrawCenteredText(26, value, DifficultyColor(map), Font_7x10);
        DrawProgress(40, game->map_index + 1U, false);
        snprintf(value, sizeof(value), "FASE: +%05lu", (unsigned long)game->map_score);
        DrawCenteredText(51, value, YELLOW, Font_7x10);
        snprintf(value, sizeof(value), "TOTAL: %05lu", (unsigned long)game->score);
        DrawCenteredText(65, value, WHITE, Font_7x10);
        snprintf(value, sizeof(value), "%lus / %u VIDAS",
                 (unsigned long)((game->active_ms + 999) / 1000), (unsigned)game->lives);
        DrawCenteredText(79, value, EDGE, Font_7x10);
        if (game->map_index + 1U < MAZE_COUNT)
        {
            snprintf(value, sizeof(value), "PROXIMO: %s",
                     Maze_Difficulty(Maze_Get(game->map_index + 1U)));
            DrawCenteredText(95, value, DifficultyColor(Maze_Get(game->map_index + 1U)), Font_7x10);
        }
        else
        {
            DrawCenteredText(95, "CAMPANHA COMPLETA", GREEN, Font_7x10);
        }
        DrawButton(110, game->map_index + 1U < MAZE_COUNT ? "PROXIMA FASE" : "VER VITORIA", true,
                   DifficultyColor(map));
    }
}

static void DrawEnding(const Game *game, const MazeMap *map, bool scene_changed)
{
    if (scene_changed)
    {
        char value[24];
        bool won = game->state == GAME_WON;
        DrawCenteredText(6, won ? "VOCE VENCEU" : "GAME OVER", won ? GREEN : RED, Font_11x18);
        DrawProgress(29, won ? MAZE_COUNT : game->map_index, false);
        const char *message = "TOCOU NA PAREDE";
        if (won)
        {
            message = "9 MAPAS CONCLUIDOS";
        }
        else if (game->reason == GAME_TIMEOUT)
        {
            message = "TEMPO ESGOTADO";
        }
        DrawCenteredText(43, message, WHITE, Font_7x10);
        if (!won)
        {
            snprintf(value, sizeof(value), "FASE %u/%u", (unsigned)game->map_index + 1U,
                     MAZE_COUNT);
            DrawCenteredText(59, value, DifficultyColor(map), Font_7x10);
        }
        else
        {
            DrawCenteredText(59, "PONTUACAO TOTAL", EDGE, Font_7x10);
        }
        snprintf(value, sizeof(value), "%05lu", (unsigned long)game->score);
        DrawCenteredText(77, value, YELLOW, Font_11x18);
        DrawButton(108, "JOGAR NOVAMENTE", true, won ? GREEN : RED);
    }
}

static void DrawFault(const Game *game, bool scene_changed)
{
    if (scene_changed)
    {
        DrawCenteredText(15, "FALHA", RED, Font_11x18);
        const char *reason = FaultMessage(game->reason);
        DrawCenteredText(52, reason, WHITE, Font_7x10);
        DrawCenteredText(78, "CONFIRA E REINICIE", WHITE, Font_7x10);
    }
}

void Renderer_Init(void)
{
    first_frame = true;
}
void Renderer_Draw(const Game *game)
{
    const MazeMap *map = Maze_Get(game->map_index);
    bool scene_changed =
        first_frame || previous.scene != game->scene || previous.map_index != game->map_index;
    if (!scene_changed && memcmp(&previous, game, sizeof(*game)) == 0)
    {
        return;
    }
    if (scene_changed)
    {
        ST7735_FillScreen(BG);
        if (game->state == GAME_PLAYING || game->state == GAME_PAUSED || game->state == GAME_HIT)
        {
            RestoreRegion(map, 0, 0, 160, 112);
            DrawHud(game);
            if (game->state == GAME_PAUSED)
            {
                DrawPlayer(game, GREEN);
            }
        }
    }
    switch (game->state)
    {
    case GAME_CALIBRATING:
        DrawCalibration(scene_changed);
        break;
    case GAME_TITLE:
        DrawTitle(game, scene_changed);
        break;
    case GAME_HELP:
        DrawTutorial(game, scene_changed);
        break;
    case GAME_READY:
        DrawCountdown(game, map, scene_changed);
        break;
    case GAME_PLAYING:
        DrawPlaying(game, map, scene_changed);
        break;
    case GAME_PAUSED:
        DrawPause(game, map, scene_changed);
        break;
    case GAME_HIT:
        DrawHit(game, scene_changed);
        break;
    case GAME_RESULT:
        DrawResult(game, map, scene_changed);
        break;
    case GAME_WON:
    case GAME_OVER:
        DrawEnding(game, map, scene_changed);
        break;
    case GAME_FAULT:
        DrawFault(game, scene_changed);
        break;
    }
    previous = *game;
    first_frame = false;
}
