#include "../h/const.h"
#include "../h/types.h"
#include "../e/initial.e"
#include "../e/pcb.e"
#include "/usr/local/include/umps2/umps/libumps.e"

/*
 *	This module implements Kaya's process scheduler and deadlock 
 *	detector
 */

/*
 * 
 * scheduler maintains the following variables
 *	int procCount;
 *	int sftBlkCount;
 *	pcb_PTR currProc;
 *	pcb_PTR readyQueue;
 */
void scheduler() {
	
	if(!emptyProcQ(readyQueue)) {
	/* start next process in queue */
		currProc = removeProcQ(&readyQueue);
		LDIT(QUANTUM);
		LDST(&(currProc -> p_s));
		
	} else {
		
		/* finished all processes properly */
		if(procCount == 0) {
			HALT();
		}
		
		/* deadlock */
		if(procCount > 0 && sftBlkCount == 0) {
			PANIC();
		}
		
		if(procCount > 0 && sftBlkCount > 0) {
			/* stuff here! */
			WAIT();
		}

	}

}
