#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

#ifndef __mem_h__
#define __mem_h__

#define P_BESTFIT  (1)
#define P_WORSTFIT (2)
#define P_FIRSTFIT (3)

int Mem_Init(int region_size, int policy);
void *Mem_Alloc(int size);
int Mem_Free(void *ptr);
void Mem_Dump();


#endif // __mem_h__