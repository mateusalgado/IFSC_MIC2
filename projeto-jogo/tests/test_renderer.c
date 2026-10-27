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
int main(int argc,char **argv)
{
    assert(argc==2);
    Game g;Game_Init(&g);g.lives=3;g.remaining_ms=60000;g.x_milli=12000;g.y_milli=16000;
    const char *names[]={"calibracao","inicio","preparar","mapa01","pausa","impacto","concluido","derrota","falha"};
    for (int state=GAME_CALIBRATING;state<=GAME_FAULT;++state) {
        g.state=(GameState)state;g.scene++;g.reason=GAME_WALL;
        g.lives=state==GAME_OVER?0:state==GAME_HIT?2:3;
        g.score=state==GAME_WON?850:0;
        Renderer_Draw(&g);save(argv[1],names[state]);
    }
    g.state=GAME_PLAYING;g.scene++;g.x_milli=12000;g.y_milli=16000;
    Renderer_Draw(&g);
    for (int x=13;x<=48;++x) { g.x_milli=x*1000;Renderer_Draw(&g); }
    memcpy(saved,screen,sizeof(saved));
    Renderer_Init();Renderer_Draw(&g);
    assert(memcmp(saved,screen,sizeof(saved))==0); /* dirty regions restore exact map */
    puts("Renderer passed: nine states, text bounds, RGB565 and dirty/full frame equality.");
    return 0;
}
