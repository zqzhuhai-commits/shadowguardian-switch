/* Original integration work, MIT. Experimental: no Switch hardware validation. */
#ifndef SG_API_H
#define SG_API_H
#include <stddef.h>
#include <stdint.h>

typedef uintptr_t (*sg_lookup)(void *module, const char *name);
typedef void (*sg_log)(const char *message);
typedef void *(*sg_make_string)(const char *utf8);
typedef void *(*sg_make_class)(const char *name);

/* Signatures follow MetalSyntax/Shadow-Guardian-vita's main.c. These are
 * native method signatures, not an implementation of Android's Java side. */
typedef struct {
    int (*on_load)(void *, void *);
    void (*get_info)(void *, void *, void *, void *, void *, void *, void *, void *, void *);
    void (*resources_init)(void *, void *);
    void (*device_init)(void *, void *);
    void (*renderer_init)(void *, void *, int, int, int, int);
    void (*game_init)(void *, void *);
    void (*resize)(void *, void *, int, int);
    int (*render)(void);
    void (*done)(void *, void *);
    void (*pause)(void *, void *);
    void (*resume)(void *, void *);
    void (*touch)(void *, void *, int, int, int, int);
    void (*key_down)(void *, void *, int);
    void (*key_up)(void *, void *, int);
    /* Engine-owned gameplay GUI state; item opacity must not change this. */
    const unsigned char *gui_visible;
} SgApi;

/* Returns missing export count. Does not execute any game instructions. */
int sg_bind(SgApi *api, sg_lookup lookup, void *module, sg_log log);

/* Boot-test entry: caller must provide a fully implemented JNI,
 * working GLES context, audio, paths, resource loader and constructor setup.
 * The standalone test runtime calls this function. language is an engine
 * index; its Chinese value has NOT been established. */
int sg_bootstrap(const SgApi *api, void *vm, void *env, sg_make_string make_string, sg_make_class make_class,
                 int width, int height, int language, sg_log log);
#endif
