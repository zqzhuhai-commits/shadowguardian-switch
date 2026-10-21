/* SPDX-License-Identifier: MIT. Standalone AArch32 SG runtime bring-up. */
#include "config.h"
#include "sg.h"
#include "jni.h"
#include "imports.h"
#include "dcr_path.h"
#include "dcr_time.h"
#include "dcr_sched.h"
#include "rt_applet.h"
#include "rt_boot.h"
#include "rt_window.h"
#include "rt_audout.h"
#include "selfproc.h"
#include "error.h"
#include "util.h"
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <switch.h>
SgApi sg_api;
static so_module module;
static int running;
static EGLDisplay display;
static EGLSurface surface;
static EGLContext context;
extern EGLDisplay b_eglGetDisplay(EGLNativeDisplayType);
extern EGLBoolean b_eglInitialize(EGLDisplay,EGLint *,EGLint *);
extern EGLBoolean b_eglChooseConfig(EGLDisplay,const EGLint *,EGLConfig *,EGLint,EGLint *);
extern EGLContext b_eglCreateContext(EGLDisplay,EGLConfig,EGLContext,const EGLint *);
extern EGLSurface b_eglCreateWindowSurface(EGLDisplay,EGLConfig,void *,const EGLint *);
extern EGLBoolean b_eglMakeCurrent(EGLDisplay,EGLSurface,EGLSurface,EGLContext);
extern EGLBoolean b_eglSwapBuffers(EGLDisplay,EGLSurface);
void dcr_config_load(void);
static void stage(const char *s){debugPrintf("[sg/stage] %s\n",s);sg_jni_check(s);log_flush_ring();}
static uintptr_t lookup(void *m,const char *s){return so_try_find_addr_rx(m,s);}
static void note(const char *s){stage(s);}
static void *make_string(const char *s){return jni_str(s);}
static void *make_class(const char *s){JClass *c=jni_class(s);return c?c->obj:NULL;}
static int verified(const char *path){
 FILE *f=fopen(path,"rb");if(!f)return 0;
 Sha256Context c;sha256ContextCreate(&c);unsigned char b[16384],h[32];size_t n,total=0;
 while((n=fread(b,1,sizeof b,f))){sha256ContextUpdate(&c,b,n);total+=n;}
 int error=ferror(f);fclose(f);sha256ContextGetHash(&c,h);
 char hex[65];for(int i=0;i<32;i++)snprintf(hex+2*i,3,"%02x",h[i]);
 debugPrintf("[sg/library] %u bytes SHA256 %s\n",(unsigned)total,hex);
 return !error && total==3821784 && !strcmp(hex,"ad17a127ae438e829a23e372ddd8100f82fe6a311b0ad295b8cead69d5805ef9");
}
const char *port_path_fixup(const char *real,char *out,size_t cap){
 /* Preserve either source directory case without moving the user's data. */
 const char *needle="/gameloft/games/GloftSGHP";const char *p=strstr(real,needle);
 if(p){char dir[768];snprintf(dir,sizeof dir,"%s/gameloft/games/GloftSGHP",dcr_game_root());struct stat st;
  if(stat(dir,&st)){
   size_t lead=p-real;if(lead+strlen("/Gameloft/Games/GloftSGHP")+strlen(p+strlen(needle))+1<=cap){
    memcpy(out,real,lead);snprintf(out+lead,cap-lead,"/Gameloft/Games/GloftSGHP%s",p+strlen(needle));return out;
   }
  }
 }
 return real;
}
static int required_assets(void){
 const char *dirs[]={"gameloft/games/GloftSGHP","Gameloft/Games/GloftSGHP"};
 const char *names[]={"sprites_1_6","gui_1_6"};
 for(unsigned i=0;i<2;i++){
  char path[768];FILE *f=NULL;
  for(unsigned j=0;j<2 && !f;j++){
   snprintf(path,sizeof path,"%s/%s/%s",dcr_game_root(),dirs[j],names[i]);f=fopen(path,"rb");
  }
  if(!f){debugPrintf("[sg/assets] missing %s; restore original data before starting\n",names[i]);return -1;}
  unsigned char header[2],offset[4],magic[2];
  if(fread(header,1,2,f)!=2){fclose(f);return -1;}
  unsigned count=header[0]|(unsigned)header[1]<<8;
  int valid=count==(i?32u:136u);
  if(!i && valid){
   valid=!fseek(f,2+27*4,SEEK_SET)&&fread(offset,1,4,f)==4;
   if(valid){
    uint32_t start=offset[0]|(uint32_t)offset[1]<<8|(uint32_t)offset[2]<<16|(uint32_t)offset[3]<<24;
    valid=!fseek(f,(long)(2+count*4+start),SEEK_SET)&&fread(magic,1,2,f)==2&&magic[0]==0xdf&&magic[1]==5;
   }
  }
  fclose(f);debugPrintf("[sg/assets] %s chunks=%u menu signature=%s\n",names[i],count,valid?"ok":"INVALID");
  if(!valid)return -1;
 }
 return 0;
}
static int load_game(void){
 stage("validate original menu resource packs");if(required_assets())return -1;
 char path[768];snprintf(path,sizeof path,"%s/libshadowguardian.so",dcr_game_root());
 if(!verified(path)){snprintf(path,sizeof path,"%s/lib/armeabi-v7a/libshadowguardian.so",dcr_game_root());
  if(!verified(path)){stage("unsupported or missing original library");return -1;}}
 stage("load ELF");if(dcr_self_process()==INVALID_HANDLE){stage("process handle unavailable");return -1;}
 int rc=so_load(&module,path,NULL,PORT_SO_REGION_BYTES);if(rc){debugPrintf("[sg] so_load=%d\n",rc);return -1;}
 if(so_relocate(&module)){stage("relocation failed");return -1;}
 int missing=so_resolve(&module,dcr_imports,dcr_imports_count,1);
 debugPrintf("[sg] unresolved imports=%d\n",missing);if(missing)return -1;
 if(so_fix_kuser_helpers(&module)<0)return -1;
 so_finalize(&module);so_flush_caches(&module);
 for(int i=0;i<module.phnum;i++)if(module.phdr[i].p_type==PT_LOAD && (module.phdr[i].p_flags&PF_X)){
  uint32_t begin=(uint32_t)(uintptr_t)module.load_virtbase+module.phdr[i].p_vaddr;
  sg_swp_region(begin,begin+module.phdr[i].p_memsz);break;
 }
 int n=sg_bind(&sg_api,lookup,&module,note);if(n){debugPrintf("[sg] missing native exports=%d\n",n);return -1;}
 stage("ELF mapped, imports resolved, native exports bound");return 0;
}
static int graphics(void){
 stage("create EGL/GLES2 window");dcr_window_set_size(SG_OUTPUT_WIDTH,SG_OUTPUT_HEIGHT);
 display=b_eglGetDisplay(EGL_DEFAULT_DISPLAY);EGLint maj=0,min=0;
 if(!b_eglInitialize(display,&maj,&min))return -1;
 EGLint attrs[]={EGL_SURFACE_TYPE,EGL_WINDOW_BIT,EGL_RENDERABLE_TYPE,EGL_OPENGL_ES2_BIT,
 EGL_RED_SIZE,8,EGL_GREEN_SIZE,8,EGL_BLUE_SIZE,8,EGL_ALPHA_SIZE,8,EGL_DEPTH_SIZE,24,EGL_STENCIL_SIZE,8,EGL_NONE};
 EGLConfig config;EGLint count=0;
 if(!b_eglChooseConfig(display,attrs,&config,1,&count))return -1;
 if(!count){attrs[13]=16;attrs[15]=0;if(!b_eglChooseConfig(display,attrs,&config,1,&count)||!count)return -1;}
 const EGLint ca[]={EGL_CONTEXT_CLIENT_VERSION,2,EGL_NONE};
 context=b_eglCreateContext(display,config,EGL_NO_CONTEXT,ca);
 surface=b_eglCreateWindowSurface(display,config,NULL,NULL);
 if(!context||!surface||!b_eglMakeCurrent(display,surface,surface,context))return -1;
 debugPrintf("[sg/gl] vendor=%s renderer=%s GLES=%s\n",glGetString(GL_VENDOR),glGetString(GL_RENDERER),glGetString(GL_VERSION));
 eglSwapInterval(display,1);glClearColor(0.015f,0.025f,0.04f,1);glClear(GL_COLOR_BUFFER_BIT);
 if(!b_eglSwapBuffers(display,surface))return -1;
 stage("EGL surface presented; calling original game next");return 0;
}
void port_focus_lost(void){if(running){sg_input_release();sg_audio_pause(1);sg_api.pause(g_jni_env,NULL);stage("native paused");}}
void port_focus_gained(void){if(running){sg_api.resume(g_jni_env,NULL);sg_audio_pause(0);stage("native resumed");}}
static int run_game(void){
 stage("initialize JNI object model");jni_init();
 extern int g_so_trace_ctors;g_so_trace_ctors=0;
 stage("execute native constructors");so_execute_init_array(&module);so_free_temp(&module);
 if(graphics()){stage("EGL creation/presentation failed");return -1;}
 debugPrintf("[sg/display] output=%dx%d fullscreen; native asset coordinates=%dx%d\n",SG_OUTPUT_WIDTH,SG_OUTPUT_HEIGHT,SG_WIDTH,SG_HEIGHT);
 stage("JNI/native bootstrap begins");
 int rc=sg_bootstrap(&sg_api,g_jni_vm,g_jni_env,make_string,make_class,SG_WIDTH,SG_HEIGHT,0,note);
 debugPrintf("[sg] bootstrap result=%d\n",rc);if(rc)return -1;
 debugPrintf("[sg/swp] atomic exchanges emulated during init=%u\n",sg_swp_count());
 stage("native bootstrap returned; entering frame loop");sg_input_init();running=1;
 unsigned frames=0;
 while(appletMainLoop()&&!rt_exit_requested()){
  rt_applet_poll();if(!rt_focused()){svcSleepThread(50000000);continue;}
  if(sg_input_poll())break;
  int result=sg_api.render();
  if(frames<3)debugPrintf("[sg/frame] nativeRender returned %d frame=%u\n",result,frames);
  if(!b_eglSwapBuffers(display,surface)){stage("EGL swap failed");break;}
  if(++frames%300==0){debugPrintf("[sg/frame] presented=%u JNI_objects=%d SWP=%u\n",frames,jni_live_objects(),sg_swp_count());log_flush_ring();}
 }
 stage("native shutdown");sg_input_release();sg_audio_close();sg_api.done(g_jni_env,NULL);running=0;
 rt_applet_stop();eglMakeCurrent(display,EGL_NO_SURFACE,EGL_NO_SURFACE,EGL_NO_CONTEXT);
 eglDestroySurface(display,surface);eglDestroyContext(display,context);eglTerminate(display);
 return 0;
}
int main(int argc,char **argv){
 (void)argc;(void)argv;const char *root=dcr_game_root();mkdir(root,0777);
 log_init(root);log_console_open();
 if(chdir(root))return 1;
 dcr_config_load();dcr_time_init();dcr_path_prepare_dirs();
 if(load_game()==0){dcr_sched_init();rt_applet_start();if(run_game()==0){log_flush_ring();return 0;}}
 /* No interactive boot gate or diagnostic screen in normal launches. */
 log_flush_ring();return 1;
}
