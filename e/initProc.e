#include "../h/const.h"
#include "../h/types.h"

/* global variables! */
extern pteOS_t kSegOS;
extern pte_t kUSeg3;
extern swapPool_t swapPool[SWAPSIZE];
extern int disk0Sem, disk1Sem;
extern Tproc_t uProcStates[MAXUSERPROC];
