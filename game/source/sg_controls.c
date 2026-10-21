/* SPDX-License-Identifier: MIT. Draw filters; never change game/UI input state. */
#include "sg.h"
#include "sg_controls.h"
#include <string.h>
static void **gameplay_instance;
void *sg_controls_graph_tail, *sg_controls_rect_tail;
void *sg_controls_joystick_tail, *sg_controls_gauge_tail;
extern void sg_controls_graph_hook(void), sg_controls_rect_hook(void);
extern void sg_controls_joystick_hook(void), sg_controls_gauge_hook(void);
int sg_controls_active(void){
 return gameplay_instance && *gameplay_instance && sg_api.game_instance &&
        *sg_api.game_instance && sg_api.is_in_action_phase &&
        sg_api.is_in_action_phase(*sg_api.game_instance);
}
int sg_controls_hidden(void *gui,unsigned index){
 if(!sg_controls_active())return 0;
 unsigned char *gs=*gameplay_instance;int id;void *current;
 memcpy(&id,gs+0x68,sizeof id);memcpy(&current,gs+0x6c,sizeof current);
 if((id!=19 && id!=20)||gui!=current)return 0;
 /* Original gameplay layouts: 0..10 movement/actions, 11..32 weapon HUD,
  * 33 onward the alternate action layout. Pause is drawn separately.
  * Filter drawing only: TouchManager areas, visibility, enabled flags,
  * alpha and highlight timers all remain owned by the original engine. */
 return index<11 || index>32;
}
int sg_controls_install(sg_lookup lookup,void *module,int (*patch)(void *,const void *,size_t)){
 struct Hook {const char *symbol;void (*hook)(void);void **tail;uint32_t words[2];};
 struct Hook hooks[]={
  {"_ZN8GUILevel14PaintGraphItemEjffN3pig5video5ColorE",sg_controls_graph_hook,&sg_controls_graph_tail,{0xe92d4ff0,0xed2d8b02}},
  {"_ZN8GUILevel13PaintRectItemEj",sg_controls_rect_hook,&sg_controls_rect_tail,{0xe92d4ff0,0xed2d8b04}},
  {"_ZN8Joystick6RenderEv",sg_controls_joystick_hook,&sg_controls_joystick_tail,{0xe92d41f0,0xe1a04000}},
  {"_ZN10PlayerCtrl16PaintButtonGaugeEfff",sg_controls_gauge_hook,&sg_controls_gauge_tail,{0xe92d4ff0,0xed2d8b08}},
 };
 uintptr_t singleton=lookup(module,"_ZN9SingletonI11GS_GamePlayE10s_instanceE");
 if(!singleton)return -1;
 memcpy(&gameplay_instance,&singleton,sizeof singleton);
 void *targets[4];
 /* Validate every entry before patching; trampolines replay these exact,
  * position-independent first two ARM instructions from the verified SO. */
 for(unsigned i=0;i<4;i++){
  uintptr_t address=lookup(module,hooks[i].symbol);
  if(!address || memcmp((void *)address,hooks[i].words,8))return -1;
  targets[i]=(void *)address;*hooks[i].tail=(void *)(address+8);
 }
 for(unsigned i=0;i<4;i++){
  uint32_t code[2]={0xe51ff004,0};uintptr_t destination=(uintptr_t)hooks[i].hook;
  code[1]=(uint32_t)destination;
  if(patch(targets[i],code,sizeof code))return -1;
 }
 return 0;
}
