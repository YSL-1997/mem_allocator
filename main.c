#include "mem.h"

int main(int argc, char *argv[])
{
  Mem_Init(4095, 1);
  printf("initialized 4096 bytes of mem\n");
  Mem_Dump();
  printf("\n");
  int *a = (int *)Mem_Alloc(30);
  printf("aaa   allocate 30\n");
  Mem_Dump();
  printf("\n");
  int *b = (int *)Mem_Alloc(30);
  printf("bbb   allocate 30\n");
  Mem_Dump();
  printf("\n");
  Mem_Free(a);
  printf("free a:\n");
  Mem_Dump();
  printf("\n");
  printf("Memdump finished\n");
  int *c = (int *)Mem_Alloc(30);
  printf("ccc   allocate 30\n");
  Mem_Dump();
  printf("alloc c succeed\n");

  Mem_Free(b);
  Mem_Dump();
  Mem_Free(c);
  Mem_Dump();

  return 0;
}
/*
int Mem_Init(int region_size, int policy);
void *Mem_Alloc(int size);
int Mem_Free(void *ptr);
void Mem_Dump();
*/