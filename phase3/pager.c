
#include "../h/const.h"
#include "../h/types.h"
#include "../e/initProc.e"
#include "../e/sysSupport.e"

#include "/usr/local/include/umps2/umps/libumps.e"

/*
 * Implements VM- I/O level TLB exception handler
 * */

void pager(){
	state_PTR causingProc;
	unsigned int page, segment; 
	unsigned int asid = getASID();
	causingProc = getCaller(asid, TLBTRAP);
	
	
}

