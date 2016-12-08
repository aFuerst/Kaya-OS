#ifndef SYSSUPPORT_E
#define SYSSUPPORT_E

#include "../h/const.h"
#include "../h/types.h"

extern void userSyscallHandler();
extern void userPgmTrpHandler();

extern state_PTR getCaller(unsigned int ASID, int trapType);

#endif
