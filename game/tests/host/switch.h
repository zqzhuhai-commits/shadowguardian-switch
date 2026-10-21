#pragma once
#include <pthread.h>
typedef pthread_mutex_t Mutex;
static inline void mutexLock(Mutex *m){pthread_mutex_lock(m);}
static inline void mutexUnlock(Mutex *m){pthread_mutex_unlock(m);}

#include <stdint.h>
typedef uint32_t u32,Result;
typedef uint64_t u64;
typedef struct {uint64_t addr,size;uint32_t type,attr,perm;} MemoryInfo;
#define R_FAILED(r) ((r)!=0)
Result svcQueryMemory(MemoryInfo *,uint32_t *,uint32_t);

/* Controller/touch API surface used by the production input module. */
typedef struct { int unused; } PadState;
typedef struct { int x,y; } HidAnalogStickState;
typedef struct { unsigned finger_id,x,y; } HidTouchState;
typedef struct { int count; HidTouchState touches[16]; } HidTouchScreenState;
#define HidNpadStyleSet_NpadStandard 1
#define HidNpadButton_A (1ull<<0)
#define HidNpadButton_B (1ull<<1)
#define HidNpadButton_X (1ull<<2)
#define HidNpadButton_Y (1ull<<3)
#define HidNpadButton_L (1ull<<6)
#define HidNpadButton_R (1ull<<7)
#define HidNpadButton_ZL (1ull<<8)
#define HidNpadButton_ZR (1ull<<9)
#define HidNpadButton_Plus (1ull<<10)
#define HidNpadButton_Minus (1ull<<11)
#define HidNpadButton_Left (1ull<<12)
#define HidNpadButton_Up (1ull<<13)
#define HidNpadButton_Right (1ull<<14)
#define HidNpadButton_Down (1ull<<15)
void padConfigureInput(int,int);
void padInitializeDefault(PadState *);
void hidInitializeTouchScreen(void);
void padUpdate(PadState *);
u64 padGetButtons(PadState *);
HidAnalogStickState padGetStickPos(PadState *,int);
int hidGetTouchScreenStates(HidTouchScreenState *,int);
