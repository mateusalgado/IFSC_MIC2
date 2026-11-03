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
static bool first;
static uint16_t pixels[160 * 8];

static uint16_t accent(const MazeMap *map)
{
    return map->difficulty == 1 ? CYAN : map->difficulty == 2 ? GOLD : PINK;
}
static void center_on(int y, const char *value, uint16_t color, uint16_t bg, FontDef font)
{
    int x = (160 - (int)strlen(value) * font.width) / 2;
    if (x >= 0) ST7735_WriteString((uint16_t)x, (uint16_t)y, value, font, color, bg);
}
static void center(int y, const char *value, uint16_t color, FontDef font)
{
    center_on(y, value, color, BG, font);
}
static void outline(int x, int y, int w, int h, uint16_t color)
{
    ST7735_FillRectangle(x,y,w,1,color); ST7735_FillRectangle(x,y+h-1,w,1,color);
    ST7735_FillRectangle(x,y,1,h,color); ST7735_FillRectangle(x+w-1,y,1,h,color);
}
static void icon(int x, int y, const uint8_t rows[7], uint16_t color)
{
    for (int row=0; row<7; ++row)
        for (int col=0; col<7; ++col)
            if (rows[row] & (1U << (6-col))) ST7735_FillRectangle(x+col,y+row,1,1,color);
}
static void heart(int x, uint16_t color)
{
    static const uint8_t rows[7] = {0x36,0x7f,0x7f,0x7f,0x3e,0x1c,0x08};
    icon(x,4,rows,color);
}
static void button(int y, const char *value, bool selected, uint16_t color)
{
    ST7735_FillRectangle(8,y,144,16,selected ? PANEL : BG);
    outline(8,y,144,16,selected ? color : WALL);
    center_on(y+3,value,selected ? WHITE : EDGE,selected ? PANEL : BG,Font_7x10);
}
static void progress(int y, unsigned complete, bool current)
{
    for (unsigned i=0; i<MAZE_COUNT; ++i) {
        int x=10+16*(int)i;
        uint16_t color=accent(Maze_Get(i));
        ST7735_FillRectangle(x,y,12,5,i<complete ? color : BG);
        outline(x,y,12,5,i==complete && current ? WHITE : i<complete ? color : WALL);
    }
}
static void player(const Game *g, uint16_t color)
{
    ST7735_FillRectangle(Game_X(g)-2,16+Game_Y(g)-2,5,5,color);
    ST7735_FillRectangle(Game_X(g),16+Game_Y(g),1,1,WHITE);
}
static uint16_t background(const MazeMap *map, int x, int y)
{
    if (Maze_Wall(map,x,y)) {
        /* A light edge belongs to the wall, never to the free corridor. */
        bool edge=!Maze_Wall(map,x-1,y) || !Maze_Wall(map,x+1,y) ||
                  !Maze_Wall(map,x,y-1) || !Maze_Wall(map,x,y+1);
        return edge ? EDGE : WALL;
    }
    int dx=x-map->goal_x, dy=y-map->goal_y;
    if (dx<0) dx=-dx;
    if (dy<0) dy=-dy;
    return dx+dy==4 ? YELLOW : BG;
}
static void region(const MazeMap *map, int x, int y, int w, int h)
{
    for (int row=0; row<h; row+=8) {
        int rows=h-row<8 ? h-row : 8;
        for (int j=0; j<rows; ++j)
            for (int i=0; i<w; ++i) {
                uint16_t color=background(map,x+i,y+row+j);
                pixels[j*w+i]=(uint16_t)((color<<8)|(color>>8));
            }
        ST7735_DrawImage(x,16+y+row,w,rows,pixels);
    }
}
static void hud(const Game *g)
{
    static const uint8_t clock[7]={0x1c,0x22,0x49,0x49,0x45,0x22,0x1c};
    const MazeMap *map=Maze_Get(g->map_index);
    uint16_t time_color=g->remaining_ms<=5000 ? RED : YELLOW;
    char value[12];
    ST7735_FillRectangle(0,0,160,16,PANEL);
    snprintf(value,sizeof(value),"M%02u",(unsigned)map->id);
    ST7735_WriteString(2,2,value,Font_7x10,accent(map),PANEL);
    for (unsigned i=0; i<3; ++i) heart(26+9*i,i<g->lives ? RED : WALL);
    icon(56,4,clock,time_color);
    snprintf(value,sizeof(value),"%02lu",(unsigned long)((g->remaining_ms+999)/1000));
    ST7735_WriteString(65,2,value,Font_7x10,time_color,PANEL);
    snprintf(value,sizeof(value),"%05lu",(unsigned long)g->score);
    ST7735_WriteString(112,2,value,Font_7x10,WHITE,PANEL);
    for (unsigned i=0; i<MAZE_COUNT; ++i)
        ST7735_FillRectangle(4+17*i,14,15,2,i<g->map_index ? accent(Maze_Get(i)) : i==g->map_index ? WHITE : WALL);
}

void Renderer_Init(void) { first=true; }
void Renderer_Draw(const Game *g)
{
    const MazeMap *map=Maze_Get(g->map_index);
    bool scene=first || previous.scene!=g->scene || previous.map_index!=g->map_index;
    if (!scene && memcmp(&previous,g,sizeof(*g))==0) return;
    if (scene) {
        ST7735_FillScreen(BG);
        if (g->state==GAME_PLAYING || g->state==GAME_PAUSED || g->state==GAME_HIT) {
            region(map,0,0,160,112); hud(g);
            if (g->state==GAME_PAUSED) player(g,GREEN);
        }
    }
    switch (g->state) {
    case GAME_CALIBRATING:
        if (scene) {
            center(20,"CALIBRANDO",WHITE,Font_11x18);
            center(54,"SOLTE O JOYSTICK",CYAN,Font_7x10);
            center(73,"MANTENHA NO CENTRO",WHITE,Font_7x10);
            center(110,GAME_VERSION,EDGE,Font_7x10);
        }
        break;
    case GAME_TITLE:
        if (scene) {
            center(6,"LABIRINTO",CYAN,Font_11x18);
            center(29,"9 MAPAS / 3 NIVEIS",WHITE,Font_7x10);
            /* Small route emblem uses only geometric primitives in Flash/code. */
            ST7735_FillRectangle(30,48,47,3,EDGE);
            ST7735_FillRectangle(74,48,3,16,EDGE);
            ST7735_FillRectangle(74,61,55,3,EDGE);
            ST7735_FillRectangle(28,46,7,7,GREEN);
            outline(125,58,9,9,YELLOW);
            center(116,GAME_VERSION,EDGE,Font_7x10);
        }
        if (scene || g->title_choice!=previous.title_choice || g->state_ms/500!=previous.state_ms/500) {
            uint16_t color=g->state_ms/500%2 ? CYAN : WHITE;
            button(76,"JOGAR",g->title_choice==0,color);
            button(95,"COMO JOGAR",g->title_choice==1,color);
        }
        break;
    case GAME_HELP: {
        MazeMap arena=maze_tutorial;
        if (g->tutorial_step==TUTORIAL_MOVE) arena.goal_x=68;
        if (g->tutorial_step==TUTORIAL_DONE) {
            if (scene) {
                center(12,"TREINO OK!",GREEN,Font_11x18);
                center(44,"MOVIMENTO E PAREDES",WHITE,Font_7x10);
                center(62,"SAIDA E PAUSA",WHITE,Font_7x10);
                center(82,"AGORA E SUA VEZ",CYAN,Font_7x10);
                button(108,"VOLTAR AO MENU",true,CYAN);
            }
            break;
        }
        if (scene) {
            unsigned lesson=g->tutorial_step<=TUTORIAL_MOVE?1:g->tutorial_step<=TUTORIAL_IMPACT?2:g->tutorial_step==TUTORIAL_EXIT?3:4;
            char title[24];snprintf(title,sizeof(title),"TREINO %u/4",lesson);
            center(4,title,CYAN,Font_11x18);
            region(&arena,8,20,144,64);
            /* Red obstacle is fully inside the same collision mask. */
            ST7735_FillRectangle(94,56,8,22,RED);
            player(g,g->tutorial_step==TUTORIAL_IMPACT?RED:GREEN);
            center(110,g->tutorial_step>=TUTORIAL_PAUSE ? "CLIQUE: CONTINUAR" : "CLIQUE: SAIR",EDGE,Font_7x10);
        }
        if (scene || (g->neutral_ms>=200)!=(previous.neutral_ms>=200)) {
            const char *hint=g->tutorial_step==TUTORIAL_CENTER ? "SOLTE O JOYSTICK" :
                g->tutorial_step==TUTORIAL_IMPACT ? "NO JOGO: -1 VIDA" :
                g->tutorial_step==TUTORIAL_PAUSE ? "CLIQUE PARA PAUSAR" :
                g->tutorial_step==TUTORIAL_RESUME ? "PAUSADO: CLIQUE" :
                g->tutorial_wait_ms ? "TENTE DE NOVO" :
                g->neutral_ms<200 ? "CENTRALIZE" :
                g->tutorial_step==TUTORIAL_MOVE ? "LEVE ATE O ALVO" :
                g->tutorial_step==TUTORIAL_WALL ? "TOQUE NA PAREDE" : "CONTORNE A PAREDE";
            ST7735_FillRectangle(0,25,160,10,BG);
            center(25,hint,g->tutorial_step==TUTORIAL_IMPACT || g->tutorial_wait_ms ? RED : WHITE,Font_7x10);
        }
        if (!scene && (Game_X(g)!=Game_X(&previous) || Game_Y(g)!=Game_Y(&previous))) {
            region(&arena,Game_X(&previous)-2,Game_Y(&previous)-2,5,5);player(g,GREEN);
        }
        break;
    }
    case GAME_READY: {
        unsigned count=(g->countdown_ms+999)/1000, old=(previous.countdown_ms+999)/1000;
        if (scene) {
            char title[24];
            snprintf(title,sizeof(title),"FASE %u/%u",(unsigned)g->map_index+1U,MAZE_COUNT);
            center(6,title,WHITE,Font_11x18); progress(28,g->map_index,true);
            center(42,map->name,WHITE,Font_7x10);
            snprintf(title,sizeof(title),"%s / %lu s",Maze_Difficulty(map),(unsigned long)(map->limit_ms/1000));
            center(58,title,accent(map),Font_7x10);
            center(116,"SOLTE O JOYSTICK",EDGE,Font_7x10);
        }
        if (scene || count!=old || (g->neutral_ms>=200)!=(previous.neutral_ms>=200)) {
            ST7735_FillRectangle(0,76,160,35,BG);
            if (g->neutral_ms<200) center(84,"CENTRALIZE",YELLOW,Font_11x18);
            else if (count) {
                char n[2]={(char)('0'+count),0}; center(78,n,accent(map),Font_16x26);
            } else center(84,"PRONTO",GREEN,Font_11x18);
        }
        break;
    }
    case GAME_PLAYING:
        if (scene || Game_X(g)!=Game_X(&previous) || Game_Y(g)!=Game_Y(&previous)) {
            if (!scene) region(map,Game_X(&previous)-2,Game_Y(&previous)-2,5,5);
            player(g,GREEN);
        }
        if (!scene && ((g->remaining_ms+999)/1000!=(previous.remaining_ms+999)/1000 ||
                       g->lives!=previous.lives || g->score!=previous.score)) hud(g);
        break;
    case GAME_PAUSED:
        if (scene || g->pause_choice!=previous.pause_choice) {
            ST7735_FillRectangle(6,28,148,82,BG); outline(6,28,148,82,accent(map));
            center(34,"PAUSA",WHITE,Font_11x18);
            button(61,"CONTINUAR",g->pause_choice==0,accent(map));
            button(82,"SAIR",g->pause_choice==1,accent(map));
        }
        break;
    case GAME_HIT:
        if (scene) {
            player(g,RED);
            ST7735_FillRectangle(18,42,124,44,BG); outline(18,42,124,44,RED);
            center(47,"-1 VIDA",RED,Font_11x18);
            center(70,g->reason==GAME_TIMEOUT ? "TEMPO ESGOTADO" : "TOCOU NA PAREDE",WHITE,Font_7x10);
        }
        break;
    case GAME_RESULT:
        if (scene) {
            char value[24];
            center(4,"FASE OK!",GREEN,Font_11x18);
            snprintf(value,sizeof(value),"FASE %u/%u - %s",(unsigned)g->map_index+1U,MAZE_COUNT,Maze_Difficulty(map));
            center(26,value,accent(map),Font_7x10); progress(40,g->map_index+1U,false);
            snprintf(value,sizeof(value),"FASE: +%05lu",(unsigned long)g->map_score);
            center(51,value,YELLOW,Font_7x10);
            snprintf(value,sizeof(value),"TOTAL: %05lu",(unsigned long)g->score);
            center(65,value,WHITE,Font_7x10);
            snprintf(value,sizeof(value),"%lus / %u VIDAS",(unsigned long)((g->active_ms+999)/1000),(unsigned)g->lives);
            center(79,value,EDGE,Font_7x10);
            if (g->map_index+1U<MAZE_COUNT) {
                snprintf(value,sizeof(value),"PROXIMO: %s",Maze_Difficulty(Maze_Get(g->map_index+1U)));
                center(95,value,accent(Maze_Get(g->map_index+1U)),Font_7x10);
            } else center(95,"CAMPANHA COMPLETA",GREEN,Font_7x10);
            button(110,g->map_index+1U<MAZE_COUNT ? "PROXIMA FASE" : "VER VITORIA",true,accent(map));
        }
        break;
    case GAME_WON:
    case GAME_OVER:
        if (scene) {
            char value[24]; bool won=g->state==GAME_WON;
            center(6,won ? "VOCE VENCEU" : "GAME OVER",won ? GREEN : RED,Font_11x18);
            progress(29,won ? MAZE_COUNT : g->map_index,false);
            center(43,won ? "9 MAPAS CONCLUIDOS" : g->reason==GAME_TIMEOUT ? "TEMPO ESGOTADO" : "TOCOU NA PAREDE",WHITE,Font_7x10);
            if (!won) {
                snprintf(value,sizeof(value),"FASE %u/%u",(unsigned)g->map_index+1U,MAZE_COUNT);
                center(59,value,accent(map),Font_7x10);
            } else center(59,"PONTUACAO TOTAL",EDGE,Font_7x10);
            snprintf(value,sizeof(value),"%05lu",(unsigned long)g->score);
            center(77,value,YELLOW,Font_11x18);
            button(108,"JOGAR NOVAMENTE",true,won ? GREEN : RED);
        }
        break;
    case GAME_FAULT:
        if (scene) {
            center(15,"FALHA",RED,Font_11x18);
            const char *reason=g->reason==GAME_ADC_ERROR ? "ADC SEM DADOS" :
                g->reason==GAME_TIMING_ERROR ? "ATRASO DA LOGICA" :
                g->reason==GAME_QUEUE_ERROR ? "FILA CHEIA" : "ERRO NO DISPLAY";
            center(52,reason,WHITE,Font_7x10);
            center(78,"CONFIRA E REINICIE",WHITE,Font_7x10);
        }
        break;
    }
    previous=*g; first=false;
}
