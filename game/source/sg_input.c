/* SPDX-License-Identifier: MIT. Touch-first experimental controller mapping. */
#include "sg.h"
#include "jni.h"
#include <switch.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
static PadState pad;
static HidTouchScreenState previous;
static int joy,joyx,joyy,aim,fire,action,grab;
static int cx=SG_WIDTH/2,cy=SG_HEIGHT/2,click;
static void touch(int type,int x,int y,int id){sg_api.touch(g_jni_env,NULL,type,x,y,id);}
static int px(int x){return x*SG_WIDTH/800;}
static int py(int y){return y*SG_HEIGHT/480;}
static void held(u64 keys,u64 mask,int x,int y,int id,int *old){
 int active=(keys&mask)!=0;
 if(active!=*old)touch(active?1:0,px(x),py(y),id);
 *old=active;
}
void sg_input_init(void){padConfigureInput(1,HidNpadStyleSet_NpadStandard);padInitializeDefault(&pad);hidInitializeTouchScreen();memset(&previous,0,sizeof previous);}
void sg_input_release(void){
 if(joy)touch(0,joyx,joyy,50);
 joy=0;
 held(0,HidNpadButton_ZL,754,444,51,&aim);
 held(0,HidNpadButton_ZR,625,451,52,&fire);
 held(0,HidNpadButton_X,725,330,53,&action);
 held(0,HidNpadButton_Y,725,152,54,&grab);
 if(click)touch(0,cx,cy,55);
 click=0;
 for(int i=0;i<previous.count;i++)touch(0,previous.touches[i].x*SG_WIDTH/1280,previous.touches[i].y*SG_HEIGHT/720,(int)previous.touches[i].finger_id);
 previous.count=0;
}
int sg_input_poll(void){
 padUpdate(&pad);u64 keys=padGetButtons(&pad),down=padGetButtonsDown(&pad),up=padGetButtonsUp(&pad);
 if((keys&HidNpadButton_Plus)&&(keys&HidNpadButton_Minus))return 1;
 if(down&(HidNpadButton_Plus|HidNpadButton_B))sg_api.key_down(g_jni_env,NULL,4);
 if(up&(HidNpadButton_Plus|HidNpadButton_B))sg_api.key_up(g_jni_env,NULL,4);
 HidAnalogStickState l=padGetStickPos(&pad,0),r=padGetStickPos(&pad,1);
 if(abs(l.x)>6000 || abs(l.y)>6000){
  int x=px(129)+l.x*px(50)/32768,y=py(406)-l.y*py(50)/32768;
  if(!joy)touch(1,px(129),py(406),50);
  touch(2,x,y,50);joy=1;joyx=x;joyy=y;
 }else if(joy){touch(0,joyx,joyy,50);joy=0;}
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
 if(down&HidNpadButton_A){touch(1,cx,cy,55);click=1;}
 if(click&&(keys&HidNpadButton_A))touch(2,cx,cy,55);
 if(up&HidNpadButton_A){touch(0,cx,cy,55);click=0;}
 held(keys,HidNpadButton_ZL,754,444,51,&aim);
 held(keys,HidNpadButton_ZR,625,451,52,&fire);
 held(keys,HidNpadButton_X,725,330,53,&action);
 held(keys,HidNpadButton_Y,725,152,54,&grab);
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
