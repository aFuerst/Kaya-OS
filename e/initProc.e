#ifndef INITPROC_E
#define INITPROC_E

#include "../h/const.h"
#include "../h/types.h"


/* global variables! */
extern pteOS_t kSegOS; /* OS page table */
extern pte_t kUSeg3;	/* kuSeg3 page table */
extern swapPool_t swapPool[SWAPSIZE];
extern int nextSwap; /* which page in swap pool will be swapped next */
extern int disk0Sem, disk1Sem, swapSem, masterSem; /* global semaphores */
extern Tproc_t uProcs[MAXUSERPROC];


extern unsigned int getASID();
extern void diskIO(int sector, int cylinder, int head, int* semaphore, int disk, int pageLoc, int rw);
extern state_PTR getCaller(unsigned int ASID, int trapType);

#endif

