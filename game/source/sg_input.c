/* SPDX-License-Identifier: MIT.
 * Touch coordinates and continuous-camera convention follow
 * MetalSyntax/Shadow-Guardian-vita 043e95b (see licenses/).
 * Dispatch through the original engine: never write actor/ammo state.
 */
#include "sg.h"
#include "jni.h"
#include <switch.h>
#include <stdlib.h>
#include <string.h>
static PadState pad;
static HidTouchScreenState previous;
static int joy,joyx,joyy,camera,camx,camy,aim,fire,action,grab;
static float lookx,looky;
static int cx=SG_WIDTH/2,cy=SG_HEIGHT/2,click,back,swipe,weapon_mode,context=-1;
static u64 last_keys,blocked;
static void touch(int type,int x,int y,int id){sg_api.touch(g_jni_env,NULL,type,x,y,id);}
static void key(int down,int code){if(down)sg_api.key_down(g_jni_env,NULL,code);else sg_api.key_up(g_jni_env,NULL,code);}
static void held(u64 keys,u64 mask,int x,int y,int id,int *old){
 int active=(keys&mask)!=0;
 if(active!=*old)touch(active?1:0,x,y,id);
 *old=active;
}
static int axis(int v){
 int a=abs(v);if(a<=6000)return 0;
 if(a>32767)a=32767;
 int out=(a-6000)*50/(32767-6000);return v<0?-out:out;
}
static float curve(int v){
 float a=(float)abs(v)/32767.0f;if(a<=0.15f)return 0;
 if(a>1)a=1;
 float t=(a-0.15f)/0.85f;t=0.35f*t+0.65f*t*t;
 return v<0?-t:t;
}
static void look(HidAnalogStickState r){
 float x=curve(r.x)*5.6f,y=-curve(r.y)*3.6f;
 if(x==0&&y==0){if(camera)touch(0,camx,camy,51);camera=0;return;}
 if(!camera){lookx=583;looky=240;camx=583;camy=240;touch(1,camx,camy,51);camera=1;}
 lookx+=x;looky+=y;
 int nx=(int)lookx,ny=(int)looky;
 /* The engine consumes relative deltas; clamping at the screen edge stops
  * rotation. Keep one drag active, rebase only to bound the accumulator. */
 if(nx!=camx||ny!=camy){camx=nx;camy=ny;touch(2,camx,camy,51);}
 if(abs(camx-583)>50000||abs(camy-240)>50000){touch(0,camx,camy,51);camera=0;}
}
void sg_input_init(void){
 padConfigureInput(1,HidNpadStyleSet_NpadStandard);padInitializeDefault(&pad);hidInitializeTouchScreen();
 memset(&previous,0,sizeof previous);context=-1;last_keys=blocked=0;
}
void sg_input_release(void){
 if(joy)touch(0,joyx,joyy,50);
 joy=0;
 if(camera)touch(0,camx,camy,51);
 camera=0;
 held(0,HidNpadButton_ZL,754,444,61,&aim);
 held(0,HidNpadButton_ZR,625,451,60,&fire);
 held(0,HidNpadButton_B,725,330,62,&action);
 held(0,HidNpadButton_A,725,152,63,&grab);
 if(swipe)touch(0,weapon_mode==1&&swipe==2?788:688,30,64);
 swipe=weapon_mode=0;
 if(click)touch(0,cx,cy,55);
 click=0;
 if(back)key(0,4);
 back=0;
 for(int i=0;i<previous.count;i++)touch(0,previous.touches[i].x*SG_WIDTH/1280,previous.touches[i].y*SG_HEIGHT/720,(int)previous.touches[i].finger_id);
 previous.count=0;
 blocked|=last_keys;
}
int sg_input_poll(void){
 padUpdate(&pad);u64 raw=padGetButtons(&pad);
 if((raw&HidNpadButton_Plus)&&(raw&HidNpadButton_Minus))return 1;
 int game=sg_api.game_instance&&*sg_api.game_instance&&sg_api.is_in_action_phase(*sg_api.game_instance);
 if(context>=0&&game!=context)sg_input_release();
 context=game;blocked&=raw;
 u64 keys=raw&~blocked,down=keys&~last_keys;
 last_keys=raw;
 int want_back=(keys&(HidNpadButton_Plus|(game?0:HidNpadButton_B)))!=0;
 if(want_back!=back)key(want_back,4);
 back=want_back;
 HidAnalogStickState l=padGetStickPos(&pad,0),r=padGetStickPos(&pad,1);
 if(game){
  int dx=axis(l.x),dy=-axis(l.y);
  if(keys&HidNpadButton_Left)dx=-50;
  if(keys&HidNpadButton_Right)dx=50;
  if(keys&HidNpadButton_Up)dy=-50;
  if(keys&HidNpadButton_Down)dy=50;
  if(dx||dy){
   joyx=129+dx;joyy=406+dy;
   if(!joy)touch(1,129,406,50);
   touch(2,joyx,joyy,50);joy=1;
  }else if(joy){touch(0,joyx,joyy,50);joy=0;}
  look(r);
  held(keys,HidNpadButton_ZL|HidNpadButton_L,754,444,61,&aim);
  held(keys,HidNpadButton_ZR|HidNpadButton_R,625,451,60,&fire);
  held(keys,HidNpadButton_B,725,330,62,&action);
  held(keys,HidNpadButton_A,725,152,63,&grab);
  /* The engine normalizes x by 800/480. At x=688 a stationary
   * release is 412.8: inside the native 365..450 reload interval.
   * A +100-pixel swipe exceeds its 30-unit next-weapon threshold.
   * Both pass through PlayerCtrl::Update and Actor::OnReload unchanged. */
  if(swipe==1&&weapon_mode==1){touch(2,788,30,64);swipe=2;}
  else if(swipe){touch(0,weapon_mode==1?788:688,30,64);swipe=weapon_mode=0;}
  else if(down&(HidNpadButton_X|HidNpadButton_Y)){
   weapon_mode=(down&HidNpadButton_Y)?2:1;
   touch(1,688,30,64);swipe=1;
  }
 }else{
  /* Native Android DPAD keys are ignored by Game::OnKeyPressed.
   * Drive a visible touch cursor; A activates the original menu controls. */
  if(abs(l.x)>6000)cx+=l.x/5000;
  if(abs(l.y)>6000)cy-=l.y/5000;
  if(abs(r.x)>6000)cx+=r.x/5000;
  if(abs(r.y)>6000)cy-=r.y/5000;
  if(keys&HidNpadButton_Left)cx-=8;
  if(keys&HidNpadButton_Right)cx+=8;
  if(keys&HidNpadButton_Up)cy-=8;
  if(keys&HidNpadButton_Down)cy+=8;
  if(cx<0)cx=0;
  if(cx>=SG_WIDTH)cx=SG_WIDTH-1;
  if(cy<0)cy=0;
  if(cy>=SG_HEIGHT)cy=SG_HEIGHT-1;
  held(keys,HidNpadButton_A,cx,cy,55,&click);
  if(click)touch(2,cx,cy,55);
 }
 HidTouchScreenState now={0};hidGetTouchScreenStates(&now,1);
 for(int i=0;i<previous.count;i++){
  int found=0;for(int j=0;j<now.count;j++)found|=previous.touches[i].finger_id==now.touches[j].finger_id;
  if(!found)touch(0,previous.touches[i].x*SG_WIDTH/1280,previous.touches[i].y*SG_HEIGHT/720,(int)previous.touches[i].finger_id);
 }
 for(int i=0;i<now.count;i++){
  int found=0;for(int j=0;j<previous.count;j++)found|=now.touches[i].finger_id==previous.touches[j].finger_id;
  touch(found?2:1,now.touches[i].x*SG_WIDTH/1280,now.touches[i].y*SG_HEIGHT/720,(int)now.touches[i].finger_id);
 }
 previous=now;return 0;
}

int sg_input_cursor(int *x,int *y){*x=cx;*y=cy;return context==0;}
