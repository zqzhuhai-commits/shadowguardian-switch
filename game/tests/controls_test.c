#include "sg.h"
#include "sg_controls.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
SgApi sg_api;
static unsigned char state[256];
static void *instance=state,*game=(void *)1;
static int phase=1,patches;
static uint32_t entries[4][2]={{0xe92d4ff0,0xed2d8b02},{0xe92d4ff0,0xed2d8b04},{0xe92d41f0,0xe1a04000},{0xe92d4ff0,0xed2d8b08}};
void sg_controls_graph_hook(void){} void sg_controls_rect_hook(void){}
void sg_controls_joystick_hook(void){} void sg_controls_gauge_hook(void){}
static int action(void *g){assert(g==game);return phase;}
static uintptr_t lookup(void *m,const char *s){
 (void)m;
 if(strstr(s,"s_instanceE"))return (uintptr_t)&instance;
 if(strstr(s,"PaintGraphItem"))return (uintptr_t)entries[0];
 if(strstr(s,"PaintRectItem"))return (uintptr_t)entries[1];
 if(strstr(s,"Joystick"))return (uintptr_t)entries[2];
 if(strstr(s,"PaintButtonGauge"))return (uintptr_t)entries[3];
 return 0;
}
static int patch(void *dst,const void *src,size_t len){
 assert(dst==(void *)entries[patches]);assert(len==8);
 assert(((const uint32_t *)src)[0]==0xe51ff004);++patches;return 0;
}
int main(void){
 sg_api.game_instance=&game;sg_api.is_in_action_phase=action;
 /* Failed prologue validation must not leave a partially patched library. */
 entries[3][1]^=1;assert(sg_controls_install(lookup,NULL,patch)==-1);assert(!patches);
 entries[3][1]^=1;assert(!sg_controls_install(lookup,NULL,patch));assert(patches==4);
 memset(state,0xa5,sizeof state);int id=19;void *gui=(void *)2;
 memcpy(state+0x68,&id,sizeof id);memcpy(state+0x6c,&gui,sizeof gui);
 unsigned char before[sizeof state];memcpy(before,state,sizeof state);
 for(unsigned i=0;i<50;i++)assert(sg_controls_hidden(gui,i)==(i<11 || i>32));
 assert(!memcmp(before,state,sizeof state)); /* No visibility/enabled/highlight changes. */
 assert(!sg_controls_hidden((void *)3,0));
 id=20;memcpy(state+0x68,&id,sizeof id);assert(sg_controls_hidden(gui,39));
 id=18;memcpy(state+0x68,&id,sizeof id);assert(!sg_controls_hidden(gui,0));
 phase=0;assert(!sg_controls_active());assert(!sg_controls_hidden(gui,39));
 phase=1;instance=NULL;assert(!sg_controls_active());assert(!sg_controls_hidden(gui,39));
 puts("controls: drawing filters preserve weapon HUD, menus and input state");
}
