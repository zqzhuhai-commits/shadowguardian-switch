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
