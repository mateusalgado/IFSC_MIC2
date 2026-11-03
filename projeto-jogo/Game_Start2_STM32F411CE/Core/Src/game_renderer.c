#include "game_renderer.h"
#include "game_config.h"
#include "maze_map.h"
#include "st7735.h"
#include <stdio.h>
#include <string.h>

#define BG 0x0843
#define PANEL 0x10a5
#define WALL 0x94b2
#define WHITE 0xffff
#define GREEN 0x07e0
#define YELLOW 0xffe0
#define RED 0xf800

static Game previous;
static bool first;
static uint16_t pixels[160 * 8];

static void label(int x, int y, const char *value, uint16_t color)
{
    ST7735_WriteString((uint16_t)x, (uint16_t)y, value, Font_7x10, color, BG);
}
static void center(int y, const char *value, uint16_t color, FontDef font)
{
    int x = (160 - (int)strlen(value) * font.width) / 2;
    if (x >= 0) ST7735_WriteString((uint16_t)x, (uint16_t)y, value, font, color, BG);
}
static void heart(int x, uint16_t color)
{
    static const uint8_t rows[7] = {0x36,0x7f,0x7f,0x7f,0x3e,0x1c,0x08};
    for (int y = 0; y < 7; ++y)
        for (int col = 0; col < 7; ++col)
            if (rows[y] & (1U << (6-col))) ST7735_FillRectangle(x+col, y+4, 1, 1, color);
}

static uint16_t background(const MazeMap *map, int x, int y)
{
    if (Maze_Wall(map, x, y)) return WALL;
    int dx = x - map->goal_x, dy = y - map->goal_y;
    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;
    return dx + dy == 4 ? YELLOW : BG;
}

static void region(const MazeMap *map, int x, int y, int w, int h)
{
    for (int row = 0; row < h; row += 8) {
        int rows = h-row < 8 ? h-row : 8;
        for (int j = 0; j < rows; ++j)
            for (int i = 0; i < w; ++i) {
                uint16_t color = background(map, x+i, y+row+j);
                /* ST7735_DrawImage sends bytes as stored, high byte first. */
                pixels[j*w+i] = (uint16_t)((color << 8) | (color >> 8));
            }
        ST7735_DrawImage(x, 16+y+row, w, rows, pixels);
    }
}

static void hud(const Game *g)
{
    char value[12];
    ST7735_FillRectangle(0, 0, 160, 16, PANEL);
    snprintf(value, sizeof(value), "M%02u", (unsigned)Maze_Get(g->map_index)->id);
    ST7735_WriteString(2, 3, value, Font_7x10, WHITE, PANEL);
    for (unsigned i = 0; i < 3; ++i) heart(26+9*i, i < g->lives ? RED : WALL);
    snprintf(value, sizeof(value), "T%02lu", (unsigned long)((g->remaining_ms+999)/1000));
    ST7735_WriteString(61, 3, value, Font_7x10, YELLOW, PANEL);
    snprintf(value, sizeof(value), "%05lu", (unsigned long)g->score);
    ST7735_WriteString(112, 3, value, Font_7x10, WHITE, PANEL);
}

void Renderer_Init(void) { first = true; }

void Renderer_Draw(const Game *g)
{
    const MazeMap *map = Maze_Get(g->map_index);
    bool scene = first || previous.scene != g->scene || previous.map_index != g->map_index;
    if (!scene && memcmp(&previous, g, sizeof(*g)) == 0) return;
    if (scene) {
        ST7735_FillScreen(BG);
        if (g->state == GAME_PLAYING || g->state == GAME_PAUSED || g->state == GAME_HIT) {
            region(map, 0, 0, 160, 112); hud(g);
            if (g->state == GAME_PAUSED)
                ST7735_FillRectangle(Game_X(g)-2, 16+Game_Y(g)-2, 5, 5, GREEN);
        }
    }
    switch (g->state) {
    case GAME_CALIBRATING:
        if (scene) {
            center(20, "CALIBRANDO", WHITE, Font_11x18);
            center(54, "SOLTE O JOYSTICK", GREEN, Font_7x10);
            center(73, "MANTENHA NO CENTRO", WHITE, Font_7x10);
            center(108, GAME_VERSION, WALL, Font_7x10);
        }
        break;
    case GAME_TITLE:
        if (scene) {
            center(12, "LABIRINTO", GREEN, Font_11x18);
            center(38, "DE PRECISAO", WHITE, Font_7x10);
            center(60, "3 FASES / 3 VIDAS", WHITE, Font_7x10);
            center(82, "> INICIAR <", GREEN, Font_7x10);
            center(99, "APERTE O JOYSTICK", WHITE, Font_7x10);
            center(115, GAME_VERSION, WALL, Font_7x10);
        }
        break;
    case GAME_READY: {
        unsigned count = (g->countdown_ms+999)/1000;
        unsigned old = (previous.countdown_ms+999)/1000;
        if (scene) {
            char title[24];
            snprintf(title, sizeof(title), "FASE %u/%u", (unsigned)g->map_index+1U, MAZE_COUNT);
            center(8, title, WHITE, Font_11x18);
            center(32, map->name, WHITE, Font_7x10);
            snprintf(title, sizeof(title), "%s / %lu s", Maze_Difficulty(map), (unsigned long)(map->limit_ms/1000));
            center(48, title, GREEN, Font_7x10);
        }
        if (scene || count != old || (g->neutral_ms >= 200) != (previous.neutral_ms >= 200)) {
            ST7735_FillRectangle(0, 68, 160, 60, BG);
            if (g->neutral_ms < 200) center(86, "CENTRALIZE", YELLOW, Font_11x18);
            else if (count) {
                char n[2] = {(char)('0'+count), 0};
                center(74, n, YELLOW, Font_16x26);
            } else center(86, "PRONTO", GREEN, Font_11x18);
        }
        break;
    }
    case GAME_PLAYING:
        if (scene || Game_X(g) != Game_X(&previous) || Game_Y(g) != Game_Y(&previous)) {
            if (!scene) region(map, Game_X(&previous)-2, Game_Y(&previous)-2, 5, 5);
            ST7735_FillRectangle(Game_X(g)-2, 16+Game_Y(g)-2, 5, 5, GREEN);
        }
        if (!scene && ((g->remaining_ms+999)/1000 != (previous.remaining_ms+999)/1000 ||
                       g->lives != previous.lives || g->score != previous.score)) hud(g);
        break;
    case GAME_PAUSED:
        if (scene || g->pause_choice != previous.pause_choice) {
            ST7735_FillRectangle(20, 29, 120, 82, BG);
            center(38, "PAUSA", WHITE, Font_11x18);
            label(30, 69, g->pause_choice == 0 ? "> CONTINUAR" : "  CONTINUAR", GREEN);
            label(30, 89, g->pause_choice == 1 ? "> SAIR" : "  SAIR", WHITE);
        }
        break;
    case GAME_HIT:
        if (scene) {
            ST7735_FillRectangle(Game_X(g)-2, 16+Game_Y(g)-2, 5, 5, RED);
            ST7735_FillRectangle(22, 45, 116, 37, BG);
            center(48, "-1 VIDA", RED, Font_11x18);
            center(70, g->reason == GAME_TIMEOUT ? "TEMPO ESGOTADO" : "TOCOU NA PAREDE", WHITE, Font_7x10);
        }
        break;
    case GAME_RESULT:
        if (scene) {
            char value[24];
            center(6, "FASE OK!", GREEN, Font_11x18);
            snprintf(value, sizeof(value), "FASE %u/%u - %s", (unsigned)g->map_index+1U, MAZE_COUNT, Maze_Difficulty(map));
            center(30, value, WHITE, Font_7x10);
            snprintf(value, sizeof(value), "FASE: +%05lu", (unsigned long)g->map_score);
            center(48, value, YELLOW, Font_7x10);
            snprintf(value, sizeof(value), "TOTAL: %05lu", (unsigned long)g->score);
            center(64, value, YELLOW, Font_7x10);
            snprintf(value, sizeof(value), "VIDAS: %u", (unsigned)g->lives);
            center(80, value, WHITE, Font_7x10);
            center(98, g->map_index+1U < MAZE_COUNT ? "> PROXIMA FASE <" : "> VER VITORIA <", GREEN, Font_7x10);
            center(115, "APERTE O JOYSTICK", WHITE, Font_7x10);
        }
        break;
    case GAME_WON:
    case GAME_OVER:
        if (scene) {
            char score[24];
            center(8, g->state == GAME_WON ? "VOCE VENCEU" : "GAME OVER",
                   g->state == GAME_WON ? GREEN : RED, Font_11x18);
            center(36, g->state == GAME_WON ? "3 FASES CONCLUIDAS" :
                   (g->reason == GAME_TIMEOUT ? "TEMPO ESGOTADO" : "TOCOU NA PAREDE"), WHITE, Font_7x10);
            if (g->state == GAME_OVER) {
                snprintf(score, sizeof(score), "FASE %u/%u", (unsigned)g->map_index+1U, MAZE_COUNT);
                center(48, score, WHITE, Font_7x10);
            }
            snprintf(score, sizeof(score), "PONTOS: %05lu", (unsigned long)g->score);
            center(61, score, YELLOW, Font_7x10);
            center(83, "APERTE PARA", WHITE, Font_7x10);
            center(99, "> REINICIAR <", GREEN, Font_7x10);
            center(116, GAME_VERSION, WALL, Font_7x10);
        }
        break;
    case GAME_FAULT:
        if (scene) {
            center(15, "FALHA", RED, Font_11x18);
            const char *reason = g->reason == GAME_ADC_ERROR ? "ADC SEM DADOS" :
                g->reason == GAME_TIMING_ERROR ? "ATRASO DA LOGICA" :
                g->reason == GAME_QUEUE_ERROR ? "FILA CHEIA" : "ERRO NO DISPLAY";
            center(52, reason, WHITE, Font_7x10);
            center(78, "CONFIRA E REINICIE", WHITE, Font_7x10);
        }
        break;
    }
    previous = *g; first = false;
}
