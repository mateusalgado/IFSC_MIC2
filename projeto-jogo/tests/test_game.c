#include "maze_game.h"
#include "maze_map.h"
#include "joy_input.h"
#include "game_config.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    CHECK(Maze_Free(Maze_Get(0), 12,16)); CHECK(Maze_Free(Maze_Get(0), 144,88)); CHECK(Maze_Exit(Maze_Get(0), 144,88));
    CHECK(!Maze_Exit(Maze_Get(0), 138,88)); CHECK(!Maze_Exit(Maze_Get(0), 149,88));
    CHECK(Maze_Wall(Maze_Get(0), -1,0)); CHECK(Maze_Wall(Maze_Get(0), 160,0)); CHECK(Maze_Wall(Maze_Get(0), 0,112));
    CHECK(!Maze_Free(Maze_Get(0), 1,16)); CHECK(!Maze_Free(Maze_Get(0), 12,8));
    CHECK(Maze_SweepFree(Maze_Get(0), 12,16,50,16));
    CHECK(Maze_SweepFree(Maze_Get(0), 50,16,50,48));
    CHECK(!Maze_SweepFree(Maze_Get(0), 12,16,144,88)); /* cannot tunnel through walls */
    CHECK(!Maze_SweepFree(Maze_Get(0), 50,16,96,48)); /* diagonal across an inside corner */
    static unsigned char seen[160*112];
    static int pending[160*112];
    unsigned head=0,tail=0;
    pending[tail++]=16*160+12; seen[pending[0]]=1;
    while (head<tail) {
        int value=pending[head++], x=value%160,y=value/160;
        const int dx[4]={-1,1,0,0},dy[4]={0,0,-1,1};
        for (int i=0;i<4;++i) {
            int nx=x+dx[i],ny=y+dy[i];
            if (Maze_Free(Maze_Get(0), nx,ny) && !seen[ny*160+nx]) {
                seen[ny*160+nx]=1;pending[tail++]=ny*160+nx;
            }
        }
    }
    CHECK(seen[88*160+144]);
}

static void easy_route(void)
{
    Game g=fresh();
    const int points[][2]={{50,16},{50,48},{96,48},{96,88},{144,88}};
    for (unsigned p=0;p<5;++p) {
        for (unsigned i=0;i<500 && g.state==GAME_PLAYING; ++i) {
            int dx=points[p][0]-Game_X(&g),dy=points[p][1]-Game_Y(&g);
            if (!dx && !dy) break;
            move(&g,dx>0?1000:dx<0?-1000:0,dy>0?1000:dy<0?-1000:0,1);
        }
        CHECK(g.state==GAME_PLAYING || g.state==GAME_RESULT);
    }
    CHECK(g.state==GAME_RESULT && g.lives==3 && g.collisions==0);
    CHECK(g.score==Game_Score(Maze_Get(0), g.active_ms,0,0));
    uint32_t score=g.score,active=g.active_ms;
    move(&g,1000,1000,100); CHECK(g.score==score && g.active_ms==active);
    GameInput press=commands; press.click=true;
    Game_Tick(&g,&press,10);
    CHECK(g.state==GAME_READY && g.score==score && g.lives==3 && g.active_ms==0 && g.map_index==1);
}

static void lives_and_pause(void)
{
    Game g=fresh();
    for (unsigned life=2; ; --life) {
        move(&g,-1000,0,30);
        CHECK(g.lives==life && g.collisions==3-life && Maze_Free(Maze_Get(0), Game_X(&g),Game_Y(&g)));
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

/* Independent BFS on the compiled mask, then drive the real FSM at 10 ms.
 * This does not assume that the JSON author's centerline is the only route. */
static void finish_map(Game *g)
{
    const MazeMap *map=Maze_Get(g->map_index);
    static int parent[MAZE_WIDTH*MAZE_HEIGHT], queue[MAZE_WIDTH*MAZE_HEIGHT];
    for (unsigned i=0;i<MAZE_WIDTH*MAZE_HEIGHT;++i) parent[i]=-1;
    int start=map->spawn_y*MAZE_WIDTH+map->spawn_x, goal=map->goal_y*MAZE_WIDTH+map->goal_x;
    unsigned head=0,tail=0;
    parent[start]=start;queue[tail++]=start;
    while (head<tail && parent[goal]<0) {
        int node=queue[head++],x=node%MAZE_WIDTH,y=node/MAZE_WIDTH;
        const int dx[]={-1,1,0,0},dy[]={0,0,-1,1};
        for (int k=0;k<4;++k) {
            int nx=x+dx[k],ny=y+dy[k],next=ny*MAZE_WIDTH+nx;
            if (Maze_Free(map,nx,ny) && parent[next]<0) {
                parent[next]=node;queue[tail++]=next;
            }
        }
    }
    CHECK(parent[goal]>=0);
    unsigned count=0;
    for (int n=goal;n!=start;n=parent[n]) queue[count++]=n;
    while (count && g->state==GAME_PLAYING) {
        int node=queue[--count];
        for (unsigned tick=0;tick<10 && g->state==GAME_PLAYING;++tick) {
            int dx=node%MAZE_WIDTH-Game_X(g),dy=node/MAZE_WIDTH-Game_Y(g);
            if (!dx && !dy) break;
            move(g,dx>0?1000:dx<0?-1000:0,dy>0?1000:dy<0?-1000:0,1);
        }
        CHECK(g->state==GAME_RESULT || (g->state==GAME_PLAYING && Game_X(g)==node%MAZE_WIDTH && Game_Y(g)==node/MAZE_WIDTH));
    }
    CHECK(g->state==GAME_RESULT);
}

static void title_menu(void)
{
    Game g;Game_Init(&g);Game_Tick(&g,&commands,10);
    GameInput press=commands,hidden=commands;
    press.click=true;hidden.click=true;hidden.render_ready=false;
    Game_Tick(&g,&hidden,10);CHECK(g.state==GAME_TITLE);
    move(&g,0,1000,20);CHECK(g.title_choice==1); /* held axis selects only once */
    Game_Tick(&g,&press,10);CHECK(g.state==GAME_HELP);
    Game_Tick(&g,&hidden,10);CHECK(g.state==GAME_HELP);
    Game_Tick(&g,&press,10);CHECK(g.state==GAME_TITLE && g.title_choice==1);
    move(&g,0,1000,20);CHECK(g.title_choice==1); /* no accidental repeat on return */
    move(&g,0,0,1);move(&g,0,-1000,1);CHECK(g.title_choice==0);
    Game_Tick(&g,&press,10);CHECK(g.state==GAME_READY && g.map_index==0 && g.score==0);
}

static void campaign(void)
{
    Game g=fresh();
    GameInput press=commands, hidden=commands;
    press.click=true;hidden.click=true;hidden.render_ready=false;
    uint32_t total=0;
    for (unsigned stage=0;stage<MAZE_COUNT;++stage) {
        const MazeMap *map=Maze_Get(stage);
        CHECK(g.map_index==stage && g.state==GAME_PLAYING);
        CHECK(g.remaining_ms==map->limit_ms && g.active_ms==0 && g.map_score==0);
        CHECK(g.collisions==0 && g.timeouts==0 && g.score==total);
        CHECK(MAZE_COUNT==9 && map->id==stage+1 && map->difficulty==stage/3+1);
        CHECK(map->corridor_width==(stage<3?14:stage<6?10:8));
        CHECK(Maze_Free(map,map->spawn_x,map->spawn_y));
        CHECK(Maze_Exit(map,map->goal_x,map->goal_y));
        CHECK(!Maze_Exit(map,map->goal_x-6,map->goal_y));
        CHECK(!Maze_SweepFree(map,map->spawn_x,map->spawn_y,map->goal_x,map->goal_y));
        /* Boundaries remain solid in every map. */
        for (int x=0;x<MAZE_WIDTH;++x) {
            CHECK(Maze_Wall(map,x,0));CHECK(Maze_Wall(map,x,MAZE_HEIGHT-1));
        }
        for (int y=0;y<MAZE_HEIGHT;++y) {
            CHECK(Maze_Wall(map,0,y));CHECK(Maze_Wall(map,MAZE_WIDTH-1,y));
        }
        if (stage>0) {
            Game collision=g;
            move(&collision,-1000,0,30);
            CHECK(collision.state==GAME_HIT && collision.lives==g.lives-1 && collision.collisions==1);
            uint32_t left=collision.remaining_ms;
            playing(&collision);
            CHECK(collision.map_index==stage && collision.score==total && collision.remaining_ms==left);
            CHECK(Game_X(&collision)==map->spawn_x && Game_Y(&collision)==map->spawn_y);
        }
        if (stage==0) { /* Lose a life; it must not be replenished between maps. */
            move(&g,-1000,0,30);CHECK(g.state==GAME_HIT && g.lives==2);
            playing(&g);
        }
        if (stage>0) { /* Timeout refills this map's limit, without erasing penalties. */
            Game sample=g;sample.remaining_ms=10;
            Game_Tick(&sample,&commands,10);CHECK(sample.reason==GAME_TIMEOUT);
            playing(&sample);CHECK(sample.remaining_ms==map->limit_ms && sample.timeouts==1);
            CHECK(sample.score==total && sample.active_ms==10 && sample.lives==1);
            /* Failure on later map preserves earlier score; restart resets campaign. */
            sample.lives=1;sample.remaining_ms=10;Game_Tick(&sample,&commands,10);
            CHECK(sample.state==GAME_OVER && sample.score==total && sample.map_index==stage);
            Game_Tick(&sample,&press,10);
            CHECK(sample.state==GAME_READY && sample.map_index==0 && sample.score==0 && sample.lives==3);
        }
        if (stage==3 || stage==6) {
            Game sample=g;sample.remaining_ms=10;Game_Tick(&sample,&commands,10);
            playing(&sample);CHECK(sample.remaining_ms==map->limit_ms && sample.timeouts==1 && sample.score==total);
            Game_Tick(&g,&press,10);CHECK(g.state==GAME_PAUSED);
            uint32_t remaining=g.remaining_ms;move(&g,0,0,200);
            CHECK(g.remaining_ms==remaining && g.score==total && g.map_index==stage);
            Game_Tick(&g,&press,10);playing(&g);
            CHECK(g.remaining_ms==remaining && g.map_index==stage);
        }
        finish_map(&g);
        CHECK(g.lives==2 && g.remaining_ms>0);
        CHECK(g.map_score==Game_Score(map,g.active_ms,g.collisions,g.timeouts));
        total+=g.map_score;CHECK(g.score==total);
        uint32_t active=g.active_ms,scene=g.scene;
        move(&g,1000,1000,100);CHECK(g.score==total && g.active_ms==active);
        Game_Tick(&g,&hidden,10);CHECK(g.state==GAME_RESULT && g.scene==scene);
        Game_Tick(&g,&press,10);
        CHECK(g.score==total && g.lives==2 && g.scene>scene);
        if (stage+1<MAZE_COUNT) {
            CHECK(g.state==GAME_READY && g.map_index==stage+1 && g.countdown_ms==3000);
            CHECK(g.reason==GAME_NO_ERROR);
            Game_Tick(&g,&hidden,10);CHECK(g.neutral_ms==0 && g.countdown_ms==3000);
            move(&g,1000,0,400);CHECK(g.state==GAME_READY && g.countdown_ms==3000);
            playing(&g);
        } else CHECK(g.state==GAME_WON && g.map_index==stage);
    }
    move(&g,0,0,100);CHECK(g.score==total && g.state==GAME_WON);
    Game_Tick(&g,&press,10);
    CHECK(g.map_index==0 && g.score==0 && g.map_score==0 && g.lives==3 && g.remaining_ms==60000);
    CHECK(g.collisions==0 && g.timeouts==0 && g.active_ms==0 && g.state==GAME_READY);
    CHECK(Game_Score(Maze_Get(3),30000,1,0)==1000);
    CHECK(Game_Score(Maze_Get(6),55000,0,1)==900);
    CHECK(Game_Score(Maze_Get(6),40000,30,30)==0);
}

int main(void)
{
    geometry(); easy_route(); lives_and_pause(); timing(); input(); title_menu(); campaign();
    CHECK(Game_Score(Maze_Get(0), 30000,0,0)==850);
    CHECK(Game_Score(Maze_Get(0), 30001,0,0)==845);
    CHECK(Game_Score(Maze_Get(0), 60000,0,0)==700);
    CHECK(Game_Score(Maze_Get(0), 55000,0,1)==325);
    CHECK(Game_Score(Maze_Get(0), 60000,20,20)==0);
    Game diagonal=fresh(),horizontal=fresh();
    move(&diagonal,1000,1000,10);move(&horizontal,1000,0,10);
    int64_t dx=diagonal.x_milli-12000,dy=diagonal.y_milli-16000,hx=horizontal.x_milli-12000;
    CHECK(dx*dx+dy*dy <= hx*hx);
    printf("%u checks passed: geometry, reachable map, complete game, lives, score, timing, input.\n",checks);
    return 0;
}
