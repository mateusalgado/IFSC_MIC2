#include "maze_game.h"
#include "maze_map.h"
#include "joy_input.h"
#include "game_config.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static GameInput commands = {.valid=true, .calibrated=true, .render_ready=true};
static unsigned checks;
#define CHECK(expr) do { assert(expr); ++checks; } while (0)

static void playing(Game *g)
{
    for (unsigned i=0; i<400 && g->state != GAME_PLAYING; ++i) Game_Tick(g, &commands, 10);
    CHECK(g->state == GAME_PLAYING);
}

static Game fresh(void)
{
    Game g; Game_Init(&g);
    Game_Tick(&g, &commands, 10); CHECK(g.state == GAME_TITLE);
    GameInput press=commands; press.click=true;
    Game_Tick(&g, &press, 10); CHECK(g.state == GAME_READY);
    playing(&g); CHECK(g.lives==3 && g.active_ms==0 && g.remaining_ms==60000);
    return g;
}

static void move(Game *g, int x, int y, unsigned ticks)
{
    GameInput in=commands; in.x=(int16_t)x; in.y=(int16_t)y;
    for (unsigned i=0; i<ticks; ++i) Game_Tick(g, &in, 10);
}

static void geometry(void)
{
    CHECK(Maze_Free(12,16)); CHECK(Maze_Free(144,88)); CHECK(Maze_Exit(144,88));
    CHECK(!Maze_Exit(138,88)); CHECK(!Maze_Exit(149,88));
    CHECK(Maze_Wall(-1,0)); CHECK(Maze_Wall(160,0)); CHECK(Maze_Wall(0,112));
    CHECK(!Maze_Free(1,16)); CHECK(!Maze_Free(12,8));
    CHECK(Maze_SweepFree(12,16,50,16));
    CHECK(Maze_SweepFree(50,16,50,48));
    CHECK(!Maze_SweepFree(12,16,144,88)); /* cannot tunnel through walls */
    CHECK(!Maze_SweepFree(50,16,96,48)); /* diagonal across an inside corner */
    static unsigned char seen[160*112];
    static int pending[160*112];
    unsigned head=0,tail=0;
    pending[tail++]=16*160+12; seen[pending[0]]=1;
    while (head<tail) {
        int value=pending[head++], x=value%160,y=value/160;
        const int dx[4]={-1,1,0,0},dy[4]={0,0,-1,1};
        for (int i=0;i<4;++i) {
            int nx=x+dx[i],ny=y+dy[i];
            if (Maze_Free(nx,ny) && !seen[ny*160+nx]) {
                seen[ny*160+nx]=1;pending[tail++]=ny*160+nx;
            }
        }
    }
    CHECK(seen[88*160+144]);
}

static void route(void)
{
    Game g=fresh();
    const int points[][2]={{50,16},{50,48},{96,48},{96,88},{144,88}};
    for (unsigned p=0;p<5;++p) {
        for (unsigned i=0;i<500 && g.state==GAME_PLAYING; ++i) {
            int dx=points[p][0]-Game_X(&g),dy=points[p][1]-Game_Y(&g);
            if (!dx && !dy) break;
            move(&g,dx>0?1000:dx<0?-1000:0,dy>0?1000:dy<0?-1000:0,1);
        }
        CHECK(g.state==GAME_PLAYING || g.state==GAME_WON);
    }
    CHECK(g.state==GAME_WON && g.lives==3 && g.collisions==0);
    CHECK(g.score==Game_Score(g.active_ms,0,0));
    uint32_t score=g.score,active=g.active_ms;
    move(&g,1000,1000,100); CHECK(g.score==score && g.active_ms==active);
    GameInput press=commands; press.click=true;
    Game_Tick(&g,&press,10);
    CHECK(g.state==GAME_READY && g.score==0 && g.lives==3 && g.active_ms==0);
}

static void lives_and_pause(void)
{
    Game g=fresh();
    for (unsigned life=2; ; --life) {
        move(&g,-1000,0,30);
        CHECK(g.lives==life && g.collisions==3-life && Maze_Free(Game_X(&g),Game_Y(&g)));
        if (!life) { CHECK(g.state==GAME_OVER); break; }
        CHECK(g.state==GAME_HIT);
        uint32_t remaining=g.remaining_ms;
        playing(&g); CHECK(g.remaining_ms==remaining && Game_X(&g)==12);
    }
    GameInput press=commands;press.click=true;
    Game_Tick(&g,&press,10);playing(&g);
    CHECK(g.collisions==0 && g.timeouts==0 && g.lives==3);
    move(&g,1000,0,10);
    Game_Tick(&g,&press,10);CHECK(g.state==GAME_PAUSED);
    int32_t x=g.x_milli;uint32_t remaining=g.remaining_ms,active=g.active_ms;
    move(&g,1000,0,300);
    CHECK(g.x_milli==x && g.remaining_ms==remaining && g.active_ms==active);
    Game_Tick(&g,&press,10);playing(&g);CHECK(g.x_milli==x && g.remaining_ms==remaining);
    Game_Tick(&g,&press,10);move(&g,0,1000,1);CHECK(g.pause_choice==1);
    Game_Tick(&g,&press,10);CHECK(g.state==GAME_TITLE);
}

static void timing(void)
{
    Game g=fresh();
    GameInput in=commands;in.render_ready=false;
    for (unsigned i=0;i<100;++i) Game_Tick(&g,&in,10);
    CHECK(g.remaining_ms==60000 && g.active_ms==0);
    g.remaining_ms=10;g.x_milli=144000;g.y_milli=88000;
    Game_Tick(&g,&commands,10);CHECK(g.reason==GAME_TIMEOUT && g.lives==2 && g.score==0);
    uint32_t active=g.active_ms;playing(&g);
    CHECK(g.remaining_ms==60000 && g.active_ms==active && g.timeouts==1);
    Game_Tick(&g,&commands,51);CHECK(g.state==GAME_FAULT && g.reason==GAME_TIMING_ERROR);
    CHECK(g.lives==2);
    g=fresh();in=commands;in.valid=false;Game_Tick(&g,&in,10);
    CHECK(g.state==GAME_FAULT && g.reason==GAME_ADC_ERROR && g.lives==3);
    g=fresh();g.state=GAME_READY;g.countdown_ms=3000;g.neutral_ms=0;
    move(&g,1000,0,400);CHECK(g.state==GAME_READY && g.countdown_ms==3000);
}

static void input(void)
{
    Joy j;Joy_Init(&j);JoyResult result={0};
    for (unsigned i=0;i<100;++i) result=Joy_Update(&j,2048,2048,false,true);
    CHECK(result.calibrated && result.x==0 && result.y==0);
    for (unsigned i=0;i<30;++i) result=Joy_Update(&j,4095,2048,false,true);
    CHECK(result.x>990 && result.y==0);
    result=Joy_Update(&j,2048,2048,false,true);CHECK(result.x==0);
    unsigned events=0;
    for (unsigned i=0;i<20;++i) events+=Joy_Update(&j,2048,2048,i%2,true).click;
    CHECK(events==0);
    for (unsigned i=0;i<50;++i) events+=Joy_Update(&j,2048,2048,true,true).click;
    CHECK(events==1);
    for (unsigned i=0;i<5;++i) Joy_Update(&j,2048,2048,false,true);
    for (unsigned i=0;i<5;++i) events+=Joy_Update(&j,2048,2048,true,true).click;
    CHECK(events==2);
    Joy_Init(&j);
    for (unsigned i=0;i<200;++i) result=Joy_Update(&j,4095,2048,false,true);
    CHECK(!result.calibrated);
    Joy_Init(&j);
    for (unsigned i=0;i<200;++i) result=Joy_Update(&j,i%2?1500:2500,2048,false,true);
    CHECK(!result.calibrated);
    Joy_Init(&j);
    for (unsigned i=0;i<99;++i) Joy_Update(&j,2048,2048,false,true);
    Joy_Update(&j,2048,2048,false,false);
    result=Joy_Update(&j,2048,2048,false,true);CHECK(!result.calibrated);
}

int main(void)
{
    geometry(); route(); lives_and_pause(); timing(); input();
    CHECK(Game_Score(30000,0,0)==850);
    CHECK(Game_Score(30001,0,0)==845);
    CHECK(Game_Score(60000,0,0)==700);
    CHECK(Game_Score(55000,0,1)==325);
    CHECK(Game_Score(60000,20,20)==0);
    Game diagonal=fresh(),horizontal=fresh();
    move(&diagonal,1000,1000,10);move(&horizontal,1000,0,10);
    int64_t dx=diagonal.x_milli-12000,dy=diagonal.y_milli-16000,hx=horizontal.x_milli-12000;
    CHECK(dx*dx+dy*dy <= hx*hx);
    printf("%u checks passed: geometry, reachable map, complete game, lives, score, timing, input.\n",checks);
    return 0;
}
