/* SPDX-License-Identifier: MIT. Feed controller samples to production code. */
#include "sg.h"
#include <switch.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
void *g_jni_env;
SgApi sg_api;
static int game;
static void *game_object=&game;
static int in_game(void *p){assert(p==&game);return game;}
static u64 buttons;
static HidAnalogStickState sticks[2];
static HidTouchScreenState screen;
static struct {int type,x,y,id;} events[4096];
static int count,downs[256],ups[256];
void padConfigureInput(int n,int style){assert(n==1&&style==1);}
void padInitializeDefault(PadState *p){(void)p;}
void hidInitializeTouchScreen(void){}
void padUpdate(PadState *p){(void)p;}
u64 padGetButtons(PadState *p){(void)p;return buttons;}
HidAnalogStickState padGetStickPos(PadState *p,int i){(void)p;return sticks[i];}
int hidGetTouchScreenStates(HidTouchScreenState *s,int n){assert(n==1);*s=screen;return 1;}
static void touch(void *e,void *o,int t,int x,int y,int id){
 (void)e;assert(!o&&count<4096);events[count].type=t;events[count].x=x;events[count].y=y;events[count++].id=id;
}
static void kd(void *e,void *o,int k){(void)e;(void)o;downs[k]++;}
static void ku(void *e,void *o,int k){(void)e;(void)o;ups[k]++;}
static int seen(int type,int id){int n=0;for(int i=0;i<count;i++)n+=events[i].type==type&&events[i].id==id;return n;}
static void tick(u64 keys){buttons=keys;assert(!sg_input_poll());}
static void clear(void){count=0;}
int main(void){
 sg_api=(SgApi){.touch=touch,.key_down=kd,.key_up=ku,.game_instance=&game_object,.is_in_action_phase=in_game};sg_input_init();
 /* Menus: B is Back, direction key edges work, no gameplay touches. */
 tick(HidNpadButton_B|HidNpadButton_Up);assert(downs[4]==1&&downs[19]==0&&!count);
 tick(0);assert(ups[4]==1&&ups[19]==0);
 game=1;tick(0);
 tick(HidNpadButton_B);assert(seen(1,62)==1&&downs[4]==1);
 tick(HidNpadButton_B);assert(seen(1,62)==1);
 tick(0);assert(seen(0,62)==1);
 clear();tick(HidNpadButton_Left|HidNpadButton_Up);assert(seen(1,50)==1);
 assert(events[count-1].x==79&&events[count-1].y==356);
 tick(0);assert(seen(0,50)==1);
 /* Camera moves by deltas without requiring A, never clamps to 800. */
 clear();sticks[1].x=32767;
 for(int i=0;i<200;i++)tick(0);
 assert(seen(1,51)==1&&seen(0,51)==0&&events[count-1].x>800);
 sticks[1].x=0;tick(0);assert(seen(0,51)==1);
 /* X: ordered weapon swipe on three separate render ticks, no repeats. */
 clear();tick(HidNpadButton_X);assert(count==1&&events[0].type==1&&events[0].x==688);
 tick(HidNpadButton_X);assert(count==2&&events[1].type==2&&events[1].x==788);
 tick(HidNpadButton_X);assert(count==3&&events[2].type==0);
 tick(HidNpadButton_X);assert(count==3);tick(0);
 clear();tick(HidNpadButton_ZR|HidNpadButton_R);assert(seen(1,60)==1);
 tick(HidNpadButton_R);assert(!seen(0,60));tick(0);assert(seen(0,60)==1);
 /* Y: stationary tap in the native reload band, once per press. */
 clear();tick(HidNpadButton_Y);assert(count==1&&events[0].type==1&&events[0].x==688);
 tick(HidNpadButton_Y);assert(count==2&&events[1].type==0&&events[1].x==688);
 tick(HidNpadButton_Y);assert(count==2);tick(0);
 /* Pause/context change releases B; held B cannot become menu Back. */
 clear();tick(HidNpadButton_B|HidNpadButton_A|HidNpadButton_ZL);
 game=0;tick(HidNpadButton_B|HidNpadButton_A|HidNpadButton_ZL);
 assert(seen(0,62)==1&&seen(0,63)==1&&seen(0,61)==1&&downs[4]==1);
 tick(0);tick(HidNpadButton_B);assert(downs[4]==2);tick(0);
 /* Focus loss balances native key edges and blocks held buttons. */
 tick(HidNpadButton_Plus);assert(downs[4]==3);sg_input_release();assert(ups[4]==3);
 tick(HidNpadButton_Plus);assert(downs[4]==3);tick(0);
 int x,y;assert(sg_input_cursor(&x,&y));int oldx=x;
 tick(HidNpadButton_Right);assert(sg_input_cursor(&x,&y)&&x>oldx);tick(0);
 /* Real screen touches retain 720p -> 800x480 coordinate conversion. */
 clear();screen.count=1;screen.touches[0]=(HidTouchState){.finger_id=3,.x=1280,.y=720};
 tick(0);assert(events[0].x==800&&events[0].y==480&&events[0].id==3);
 screen.count=0;tick(0);assert(seen(0,3)==1);
 /* Combined quit shortcut is recognized before input dispatch. */
 buttons=HidNpadButton_Plus|HidNpadButton_Minus;assert(sg_input_poll()==1);
 puts("PASS: B context, D-pad movement, continuous camera, framed weapon/reload gestures, trigger aliases, balanced releases and real touch mapping");
}
