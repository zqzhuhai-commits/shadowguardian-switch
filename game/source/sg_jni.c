/* SPDX-License-Identifier: MIT. SG-specific JNI; reference: Shadow-Guardian-vita.
 * Exact class names are from the supplied native ELF. Unknown calls remain logged.
 */
#include "sg.h"
#include "jni.h"
#include "rt_applet.h"
#include "dcr_path.h"
#include "util.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

const JMethodDef jni_method_defs[]={{NULL,NULL,NULL,NULL}};
const JFieldDef jni_field_defs[]={
 {"android/os/Build","MODEL",NULL,0,"Nintendo Switch"},
 {"android/os/Build","MANUFACTURER",NULL,0,"Nintendo"},
 {"android/os/Build$VERSION","SDK_INT",NULL,10,NULL},
 {NULL,NULL,NULL,0,NULL}
};
const char *const jni_class_supers[][2]={
 {SG_CLASS "ShadowGuardian","android/app/Activity"},
 {"android/app/Activity","android/content/Context"},
 {"android/content/Context","java/lang/Object"},
 {NULL,NULL}
};
const char *const jni_missing_classes[]={NULL};
static unsigned unknown,reported;
static int is_sg(const char *c){return !strncmp(c,SG_CLASS,strlen(SG_CLASS));}
/* Resolve bounded GLResLoader requests only to game-owned resources. */
static FILE *resource(const char *name,long *size){
 if(!name || strstr(name,"..") || strchr(name,'\\'))return NULL;
 const char *base=strrchr(name,'/');base=base?base+1:name;
 const char *prefix[]={"res/raw/","res/drawable/","res/","assets/",""};
 const char *suffix[]={"",".bin",".txt",".png"};
 char p[1024];
 for(unsigned i=0;i<5;i++)for(unsigned j=0;j<4;j++){
  if(snprintf(p,sizeof p,"%s/%s%s%s",dcr_game_root(),prefix[i],base,suffix[j])>=(int)sizeof p)continue;
  FILE *f=fopen(p,"rb");if(!f)continue;
  if(fseek(f,0,SEEK_END) || (*size=ftell(f))<0){fclose(f);continue;}
  rewind(f);return f;
 }
 debugPrintf("[jni/resource] missing %s\n",base);return NULL;
}
static jvalue res_call(const JMethod *m,const jvalue *a){
 if(strncmp(m->sig,"(Ljava/lang/String;",19)){debugPrintf("[jni/resource] unsupported ID-based request %s%s\n",m->name,m->sig);return jv_l(NULL);}
 long size=0;FILE *f=resource(jni_utf(a[0].l),&size);
 if(!strcmp(m->name,"getResourceLength")){if(f)fclose(f);return jv_i(size);}
 if(!f)return jv_l(NULL);
 long offset=0,length=size;
 if(!strcmp(m->name,"getResourceBytes")){offset=a[1].i;length=a[2].i;}
 if(offset<0 || length<0 || offset>size || length>size-offset || length>(16<<20)){fclose(f);return jv_l(NULL);}
 JObj *o=jni_array('B',(int)length);
 if(!o || fseek(f,offset,SEEK_SET) || fread(o->a.data,1,length,f)!=(size_t)length){jni_release(o);o=NULL;}
 fclose(f);return jv_l(o);
}
int port_jni_invoke(JObj *self,JMethod *m,const jvalue *a,jvalue *out){
 const char *c=m->cls->name,*n=m->name;
 if((!strcmp(c,"java/lang/System") && !strcmp(n,"currentTimeMillis")) || (is_sg(c) && !strcmp(n,"GetCurrentTime"))){
  struct timespec ts;clock_gettime(CLOCK_REALTIME,&ts);*out=jv_j((int64_t)ts.tv_sec*1000+ts.tv_nsec/1000000);return 1;
 }
 if(!strcmp(c,"android/media/AudioTrack")){
  if(!strcmp(n,"<init>")){
   if(self){self->v[0]=a[1].i;self->v[1]=(a[2].i==4||a[2].i==2)?1:2;}
   *out=jv_none();return 1;
  }
  if(!strcmp(n,"getMinBufferSize")){*out=jv_i(16384);return 1;}
  if(!strcmp(n,"write") && !strcmp(m->sig,"([BII)I")){
   JObj *o=a[0].l;int off=a[1].i,len=a[2].i;
   if(!o || o->kind!=JK_ARRAY || o->a.elem!='B' || off<0 || len<0 || off>o->a.len || len>o->a.len-off){*out=jv_i(-2);return 1;}
   unsigned rate=self&&self->v[0]?(unsigned)self->v[0]:44100;
   int channels=self&&self->v[1]?(int)self->v[1]:2;
   *out=jv_i(sg_audio_write((unsigned char *)o->a.data+off,len,rate,channels));return 1;
  }
  if(!strcmp(n,"play")){sg_audio_pause(0);*out=jv_none();return 1;}
  if(!strcmp(n,"pause")||!strcmp(n,"stop")||!strcmp(n,"release")){sg_audio_pause(1);*out=jv_none();return 1;}
 }
 if(!is_sg(c) && strncmp(c,"android/content/",16) && strcmp(c,"android/app/Activity") && strcmp(c,"java/io/File"))return 0;
 if(!strcmp(c,SG_CLASS "GLResLoader") && (!strcmp(n,"getResourceLength")||!strcmp(n,"getResourceBytes")||!strcmp(n,"getResourceFull")||!strcmp(n,"getRawResource"))){*out=res_call(m,a);return 1;}
 if(!strcmp(n,"Exit") || !strcmp(n,"sendAppToBackground")){debugPrintf("[sg] original game requested exit\n");log_flush_ring();exit(0);}
 if(!strcmp(n,"getPath")||!strcmp(n,"getAbsolutePath")){*out=jv_l(jni_str("/sdcard/gameloft/games/GloftSGHP/"));return 1;}
 if(!strcmp(n,"getFilesDir")){*out=jv_l(jni_singleton("java/io/File"));return 1;}
 if(!strcmp(n,"getPackageName")){*out=jv_l(jni_str(PORT_PACKAGE));return 1;}
 if(!strcmp(n,"getContext")||!strcmp(n,"getInstance")){*out=jv_l(jni_singleton(SG_CLASS "ShadowGuardian"));return 1;}
 if(!strcmp(n,"getResources")){*out=jv_l(jni_singleton("android/content/res/Resources"));return 1;}
 if(!strcmp(n,"getAssets")){*out=jv_l(jni_singleton("android/content/res/AssetManager"));return 1;}
 if(!strcmp(n,"getCountry")||!strcmp(n,"getNetworkCountryIso")||!strcmp(n,"getSimCountryIso")){*out=jv_l(jni_str("US"));return 1;}
 if(!strcmp(n,"getLanguage")){*out=jv_l(jni_str("en"));return 1;}
 if(!strcmp(n,"getPhoneModel")||!strcmp(n,"getDevice")){*out=jv_l(jni_str("Nintendo_Switch"));return 1;}
 if(!strcmp(n,"getCarrier")){*out=jv_l(jni_str("Nintendo"));return 1;}
 if(!strcmp(n,"getWidth")){*out=jv_i(SG_WIDTH);return 1;}
 if(!strcmp(n,"getHeight")){*out=jv_i(SG_HEIGHT);return 1;}
 if(!strcmp(n,"getManufacture")){*out=jv_i(4);return 1;}
 if(!strcmp(n,"GetPhoneLanguage")){*out=jv_i(0);return 1;}
 if(!strcmp(n,"isRoaming")||!strcmp(n,"getIsRoaming")||!strcmp(n,"IsWifiEnable")){*out=jv_z(0);return 1;}
 if(!strcmp(n,"isWifiEnabled")){*out=jv_i(0);return 1;}
 if(!strcmp(n,"isNeedGcOnFrame")){*out=jv_z(0);return 1;}
 if(!strcmp(n,"getUniqueCode")||!strcmp(n,"createUniqueCode")){*out=jv_i(0);return 1;}
 if(!strcmp(n,"getIMEI")||!strcmp(n,"getHostName")||!strcmp(n,"getDemoCode")){*out=jv_l(jni_str(""));return 1;}
 if(!strcmp(n,"InitDeviceValues")||!strcmp(n,"init")||!strcmp(n,"destroy")||!strcmp(n,"openBrowser")||!strcmp(n,"notifyTrophy")||!strcmp(n,"sendTrackingInfo")){*out=jv_none();return 1;}
 /* Do not fabricate success for unimplemented calls. */
 if(unknown++<128)debugPrintf("[sg/jni] unimplemented %s.%s%s\n",c,n,m->sig);
 return 0;
}
void sg_jni_check(const char *stage){if(unknown!=reported){debugPrintf("[sg/jni] stage=%s unimplemented calls=%u\n",stage,unknown);reported=unknown;}}
