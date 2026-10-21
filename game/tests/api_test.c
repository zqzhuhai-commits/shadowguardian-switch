/* SPDX-License-Identifier: MIT. Test lifecycle ABI/order and failure gates. */
#include "sg_api.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static int step,version=0x10004,strings,fail_string;
static int objects[7];
static void *vm=(void *)(uintptr_t)0x1234,*env=(void *)(uintptr_t)0x5678;
static void *str(const char *s){assert(s&&*s);if(fail_string)return NULL;assert(strings<7);return &objects[strings++];}
static int onload(void *v,void *unused){assert(v==vm&&!unused);assert(++step==1);return version;}
static void info(void *e,void *cls,void *a,void *b,void *c,void *d,void *f,void *g,void *h){
 assert(e==env&&!cls);assert(++step==2);void *v[]={a,b,c,d,f,g,h};for(int i=0;i<7;i++)assert(v[i]==&objects[i]);
}
static void res(void *e,void *cls){assert(e==env&&!cls);assert(++step==3);}
static void device(void *e,void *cls){assert(e==env&&!cls);assert(++step==4);}
static void renderer(void *e,void *cls,int profile,int w,int h,int lang){assert(e==env&&!cls&&profile==4&&w==1280&&h==720&&lang==0);assert(++step==5);}
static void game(void *e,void *cls){assert(e==env&&!cls);assert(++step==6);}
static void resize(void *e,void *cls,int w,int h){assert(e==env&&!cls&&w==1280&&h==720);assert(++step==7);}
static uintptr_t missing(void *module,const char *s){(void)module;assert(!strncmp(s,"Java_com_gameloft_android_ANMP_GloftSGHP_ML_",41)||!strcmp(s,"JNI_OnLoad"));return 0;}
int main(void){
 SgApi a={.on_load=onload,.get_info=info,.resources_init=res,.device_init=device,.renderer_init=renderer,.game_init=game,.resize=resize};
 assert(sg_bootstrap(&a,vm,env,str,1280,720,0,NULL)==0);assert(step==7&&strings==7);
 step=strings=0;version=0x123;assert(sg_bootstrap(&a,vm,env,str,1280,720,0,NULL)==-4&&step==1);
 step=strings=0;version=0x10004;fail_string=1;assert(sg_bootstrap(&a,vm,env,str,1280,720,0,NULL)==-3&&step==0);
 fail_string=0;assert(sg_bootstrap(&a,vm,env,str,0,720,0,NULL)==-1&&step==0);
 a.renderer_init=NULL;assert(sg_bootstrap(&a,vm,env,str,1280,720,0,NULL)==-2&&step==0);
 assert(sg_bind(NULL,missing,NULL,NULL)==-1);assert(sg_bind(&a,missing,NULL,NULL)==14);
 puts("PASS: exact lifecycle order, JNI string identities, dimensions/profile, bad VM version, allocation failure, missing exports");
}
