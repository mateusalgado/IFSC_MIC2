#include "game_renderer.h"
#include "st7735.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static uint16_t screen[128][160],saved[128][160];
void ST7735_FillRectangle(uint16_t x,uint16_t y,uint16_t w,uint16_t h,uint16_t color)
{
    assert(w && h && x+w<=160 && y+h<=128);
    for (unsigned row=y;row<y+h;++row)
        for (unsigned col=x;col<x+w;++col) screen[row][col]=color;
}
void ST7735_FillScreen(uint16_t color) { ST7735_FillRectangle(0,0,160,128,color); }
void ST7735_DrawImage(uint16_t x,uint16_t y,uint16_t w,uint16_t h,const uint16_t *data)
{
    assert(w && h && x+w<=160 && y+h<=128);
    for (unsigned row=0;row<h;++row)
        for (unsigned col=0;col<w;++col) {
            uint16_t c=data[row*w+col];screen[y+row][x+col]=(uint16_t)((c<<8)|(c>>8));
        }
}
void ST7735_WriteString(uint16_t x,uint16_t y,const char *str,FontDef font,uint16_t color,uint16_t bg)
{
    assert(x+strlen(str)*font.width<=160 && y+font.height<=128);
    for (;*str;++str,x+=font.width) {
        assert(*str>=32 && *str<=126);
        for (unsigned row=0;row<font.height;++row)
            for (unsigned col=0;col<font.width;++col)
                screen[y+row][x+col]=font.data[(*str-32)*font.height+row]&(0x8000U>>col)?color:bg;
    }
}
static void save(const char *directory,const char *name)
{
    char file[1024];snprintf(file,sizeof(file),"%s/%s.ppm",directory,name);
    FILE *out=fopen(file,"wb");assert(out);
    fprintf(out,"P6\n160 128\n255\n");
    for (unsigned y=0;y<128;++y) for (unsigned x=0;x<160;++x) {
        unsigned c=screen[y][x];unsigned char rgb[3]={(c>>11)*255/31,((c>>5)&63)*255/63,(c&31)*255/31};
        assert(fwrite(rgb,1,3,out)==3);
    }
    fclose(out);
}
static void same_as_full(const Game *g)
{
    memcpy(saved,screen,sizeof(saved));Renderer_Init();Renderer_Draw(g);
    assert(memcmp(saved,screen,sizeof(saved))==0);
}
static void interactive_states(void)
{
    Game g;Game_Init(&g);g.state=GAME_TITLE;g.scene++;
    Renderer_Draw(&g);g.title_choice=1;g.state_ms=500;Renderer_Draw(&g);same_as_full(&g);
    g.title_choice=0;g.state_ms=1000;Renderer_Draw(&g);same_as_full(&g);
    g.state=GAME_READY;g.scene++;g.countdown_ms=3000;Renderer_Draw(&g);
    g.neutral_ms=200;Renderer_Draw(&g);same_as_full(&g);
    for (unsigned count=2;count>0;--count) {
        g.countdown_ms=count*1000;Renderer_Draw(&g);same_as_full(&g);
    }
    g.neutral_ms=0;Renderer_Draw(&g);same_as_full(&g);
    g.neutral_ms=200;g.countdown_ms=0;Renderer_Draw(&g);same_as_full(&g);
    g.state=GAME_PLAYING;g.scene++;g.x_milli=12000;g.y_milli=16000;
    g.lives=3;g.remaining_ms=6000;g.score=16800;Renderer_Draw(&g);
    g.remaining_ms=5000;Renderer_Draw(&g);same_as_full(&g);
    g.state=GAME_PAUSED;g.scene++;Renderer_Draw(&g);
    g.pause_choice=1;Renderer_Draw(&g);same_as_full(&g);
}

int main(int argc,char **argv)
{
    assert(argc==2);
    interactive_states();
    Renderer_Init();
    Game g;Game_Init(&g);g.lives=3;g.remaining_ms=60000;g.x_milli=12000;g.y_milli=16000;
    const char *names[]={"calibracao","inicio","preparar","mapa01","pausa","impacto","resultado","concluido","derrota","falha","ajuda"};
    for (int state=GAME_CALIBRATING;state<=GAME_HELP;++state) {
        g.state=(GameState)state;g.scene++;g.reason=GAME_WALL;
        if (g.state==GAME_HELP) {g.x_milli=24000;g.y_milli=58000;}
        g.lives=state==GAME_OVER?0:state==GAME_HIT?2:3;
        g.score=state==GAME_WON?850:0;
        Renderer_Draw(&g);save(argv[1],names[state]);
    }
    uint32_t preview_total=0;
    for (unsigned stage=0;stage<MAZE_COUNT;++stage) {
        const MazeMap *map=Maze_Get(stage);
        char name[32];
        g.map_index=(uint8_t)stage;g.remaining_ms=map->limit_ms;g.lives=3;g.score=preview_total;
        g.x_milli=map->spawn_x*1000;g.y_milli=map->spawn_y*1000;
        g.state=GAME_READY;g.scene++;
        Renderer_Draw(&g);snprintf(name,sizeof(name),"preparar%02u",map->id);save(argv[1],name);
        g.state=GAME_PLAYING;g.scene++;
        Renderer_Draw(&g);snprintf(name,sizeof(name),"mapa%02u",map->id);save(argv[1],name);
        for (int y=0;y<MAZE_HEIGHT;++y) for (int x=0;x<MAZE_WIDTH;++x) {
            uint16_t color=screen[y+16][x];
            if (Maze_Wall(map,x,y)) assert(color!=0x0843 && color!=0xffe0 && color!=0x07e0);
            else if (abs(x-Game_X(&g))>2 || abs(y-Game_Y(&g))>2)
                assert(color==0x0843 || color==0xffe0); /* decorations never fill corridors */
        }
        for (int x=13;x<=48;++x) { g.x_milli=x*1000;Renderer_Draw(&g); }
        memcpy(saved,screen,sizeof(saved));
        Renderer_Init();Renderer_Draw(&g);
        assert(memcmp(saved,screen,sizeof(saved))==0); /* dirty regions restore exact map */
        /* Draw over the goal and move away, ensuring its diamond is restored. */
        g.x_milli=map->goal_x*1000;g.y_milli=map->goal_y*1000;g.scene++;
        Renderer_Draw(&g);g.x_milli-=3000;Renderer_Draw(&g);
        memcpy(saved,screen,sizeof(saved));Renderer_Init();Renderer_Draw(&g);
        assert(memcmp(saved,screen,sizeof(saved))==0);
        g.state=GAME_RESULT;g.scene++;g.active_ms=30000;g.map_score=Game_Score(map,30000,0,0);
        preview_total+=g.map_score;g.score=preview_total;
        Renderer_Draw(&g);snprintf(name,sizeof(name),"resultado%02u",map->id);save(argv[1],name);
    }
    for (int step=TUTORIAL_CENTER;step<=TUTORIAL_DONE;++step) {
        g.state=GAME_HELP;g.tutorial_step=(TutorialStep)step;g.scene++;
        g.x_milli=step==TUTORIAL_WALL?68000:step==TUTORIAL_IMPACT?91000:
            step>=TUTORIAL_PAUSE?130000:24000;
        g.y_milli=58000;g.neutral_ms=200;
        g.tutorial_wait_ms=0;
        Renderer_Draw(&g);
        char name[32];snprintf(name,sizeof(name),"tutorial%d",step);save(argv[1],name);
        if (step==TUTORIAL_MOVE || step==TUTORIAL_EXIT) {
            for (int x=25;x<44;++x) {g.x_milli=x*1000;Renderer_Draw(&g);}
            same_as_full(&g);
            g.neutral_ms=0;Renderer_Draw(&g);same_as_full(&g);
        }
    }
    g.state=GAME_WON;g.scene++;Renderer_Draw(&g);save(argv[1],"concluido");
    puts("Renderer passed: eleven states, nine maps/results, menu/countdown/pause/time warning, text bounds, wall geometry, dirty/full equality.");
    return 0;
}
