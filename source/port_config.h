/* Shadow Guardian wrapper configuration. */
#ifndef PORT_CONFIG_H
#define PORT_CONFIG_H

#define PORT_TITLE "Shadow Guardian"
#define PORT_NAME "shadowguardian_nx"
#define PORT_PACKAGE "com.gameloft.android.GloftSGHP"
#define PORT_APK_DEFAULT_NAME "Shadow-Guardian.apk"
#define PORT_APK_DESC "Shadow Guardian 1.0.1 (armeabi-v7a)"
#define PORT_ABI_DIR "lib/armeabi-v7a/"
#define PORT_SO_REGION_BYTES (32u * 1024u * 1024u)

#define PORT_APK_ROLES                                                                             \
{                                                                                                \
.what = "Shadow Guardian 1.0.1", .name = PORT_APK_DEFAULT_NAME, .package = PORT_PACKAGE,       \
.version_code = 101, .need = (const char *const[]) {                                            \
"lib/armeabi-v7a/libshadowguardian.so", NULL                                                 \
}                                                                                              \
}

#define RT_PACKAGE_MISMATCH_FATAL 1
#define RT_OPENSLES 1
#define RT_PAD_MAX_PLAYERS 1
#define RT_SCREEN_W 1280
#define RT_SCREEN_H 720
#define RT_GL_CHECK 1
#define RT_GL_SWAP_ENDS_FRAME 0
#define RT_BOOST_WATCH_THREAD 1
#define RT_PATH_TRACE_EXTRA ""
#define RT_PATH_SD_SUBDIRS "gameloft"
#define RT_PATH_EXTRA_DIRS                                                                         \
"gameloft", "gameloft/games", "gameloft/games/GloftSGHP", "data/shared_prefs", "obb"

#define PORT_LAUNCHER_BYLINE "Shadow Guardian NX port"
#define PORT_LAUNCHER_START_NOTE "Bring your own Shadow Guardian 1.0.1 APK and gameloft data folder."

#endif
