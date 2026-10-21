#include "sg_api.h"
#include <string.h>

#define SG_PREFIX_CLASS "com/gameloft/android/ANMP/GloftSGHP/ML/"
#define SG_PREFIX "Java_com_gameloft_android_ANMP_GloftSGHP_ML_"

int sg_bind(SgApi *api, sg_lookup lookup, void *module, sg_log log) {
    int missing = 0;
    if (!api || !lookup) return -1;
    memset(api, 0, sizeof(*api));
    /* memcpy avoids ISO C object/function pointer casts; this binding is
     * intended for ARM EABI where function pointers and uintptr_t agree. */
#define BIND(field, symbol) do { \
    uintptr_t addr = lookup(module, symbol); \
    _Static_assert(sizeof(api->field) == sizeof(addr), "unsupported pointer ABI"); \
    memcpy(&api->field, &addr, sizeof(addr)); \
    if (!addr) { ++missing; if (log) log(symbol); } \
} while (0)
    BIND(on_load, "JNI_OnLoad");
    BIND(get_info, SG_PREFIX "ShadowGuardian_nativeGetInfo");
    BIND(resources_init, SG_PREFIX "GLResLoader_nativeInit");
    BIND(device_init, SG_PREFIX "GLUtils_Device_nativeInit");
    BIND(renderer_init, SG_PREFIX "GameRenderer_nativeInit");
    BIND(game_init, SG_PREFIX "ShadowGuardian_nativeInit");
    BIND(resize, SG_PREFIX "GameRenderer_nativeResize");
    BIND(render, SG_PREFIX "GameRenderer_nativeRender");
    BIND(done, SG_PREFIX "GameRenderer_nativeDone");
    BIND(pause, SG_PREFIX "GameGLSurfaceView_nativePause");
    BIND(resume, SG_PREFIX "GameGLSurfaceView_nativeResume");
    BIND(touch, SG_PREFIX "GameGLSurfaceView_nativeOnTouch");
    BIND(key_down, SG_PREFIX "ShadowGuardian_nativeOnKeyDown");
    BIND(key_up, SG_PREFIX "ShadowGuardian_nativeOnKeyUp");
#undef BIND
    return missing;
}

int sg_bootstrap(const SgApi *a, void *vm, void *env, sg_make_string str, sg_make_class clazz,
                 int width, int height, int language, sg_log log) {
    if (!a || !vm || !env || !str || !clazz || width <= 0 || height <= 0) return -1;
    if (!a->on_load || !a->get_info || !a->resources_init || !a->device_init ||
        !a->renderer_init || !a->game_init || !a->resize) return -2;
    /* These native methods retain their declaring jclass with NewGlobalRef.
     * NULL makes every later GetStaticMethodID return NULL. */
    void *game_class = clazz(SG_PREFIX_CLASS "ShadowGuardian");
    void *resource_class = clazz(SG_PREFIX_CLASS "GLResLoader");
    void *device_class = clazz(SG_PREFIX_CLASS "GLUtils/Device");
    void *renderer_class = clazz(SG_PREFIX_CLASS "GameRenderer");
    if (!game_class || !resource_class || !device_class || !renderer_class) return -5;
    /* Strings are JNI objects. Never pass C string literals as jstring. */
    void *info[7];
    const char *values[7] = { "US", "GL_00", "Nintendo_Switch", "2.3.4",
                              "Nintendo", "Switch", "Switch" };
    for (size_t i = 0; i < 7; ++i) {
        info[i] = str(values[i]);
        if (!info[i]) return -3;
    }
    if (log) log("JNI_OnLoad");
    int version = a->on_load(vm, NULL);
    if (version != 0x00010001 && version != 0x00010002 &&
        version != 0x00010004 && version != 0x00010006) return -4;
    if (log) log("nativeGetInfo");
    a->get_info(env, game_class, info[0], info[1], info[2], info[3], info[4], info[5], info[6]);
    if (log) log("GLResLoader.nativeInit");
    a->resources_init(env, resource_class);
    if (log) log("Device.nativeInit");
    a->device_init(env, device_class);
    /* 4 is the compatibility manufacturer profile used by the Vita port;
     * validity on Switch has not been tested. */
    if (log) log("GameRenderer.nativeInit");
    a->renderer_init(env, renderer_class, 4, width, height, language);
    if (log) log("ShadowGuardian.nativeInit");
    a->game_init(env, game_class);
    if (log) log("GameRenderer.nativeResize");
    a->resize(env, renderer_class, width, height);
    return 0;
}
