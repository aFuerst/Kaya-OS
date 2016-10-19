#include "../h/const.h"
#include "../h/types.h"
#include "../e/initial.e"
#include "../e/pcb.e"
#include "/usr/local/include/umps2/umps/libumps.e"

/*
 *	This module implements Kaya's process scheduler and deadlock 
 *	detector
 *
 * scheduler maintains the following variables
 *	int procCount;
 *	int sftBlkCount;
 *	pcb_PTR currProc;
 *	pcb_PTR readyQueue;
 */
unsigned int TODStarted;
unsigned int currentTOD;

extern int procCount;
extern int sftBlkCount;
extern pcb_PTR currProc;
extern pcb_PTR readyQueue;
extern int semD[MAGICNUM];

void debugSch(int a, int b, int c, int d){
	int i;
	i=0;
}

/*
 * 
 * 
 * 
 */
void scheduler() {
	
	if(currProc != NULL){
		/* save how much time current process used on CPU */
		/* subtract current time from global start time to get this ^ */
		STCK(currentTOD);
		currProc->cpu_time = currProc->cpu_time + (currentTOD - TODStarted);
	}

	if(!emptyProcQ(readyQueue)) {
	/* start next process in queue */
		currProc = removeProcQ(&readyQueue);
		/* get start time */
		STCK(TODStarted);
		/* load QUANTUM into process timer */
		setTIMER(QUANTUM);
		
		LDST(&(currProc -> p_s));

	} else {
		/*debugSch(0x12345678, procCount, sftBlkCount, 0xffffffff);
		/* finished all processes properly */
		if(procCount == 0) {
			HALT();
		}

		/* deadlock */
		if(procCount > 0 && sftBlkCount == 0) {
			PANIC();
		}

		/* now it's just a waiting game */
		if(procCount > 0 && sftBlkCount > 0) {
			/* stuff here! */
			debugSch(0xbbbbbbbb, procCount, sftBlkCount, 0);
			currProc = NULL; /* no running process */
			/* enable all interrupts and go to wait state */
			setSTATUS((getSTATUS() | ALLOFF | IEON | IECON | IMON));
			WAIT(); /* run silent run deep */
		}

	}

}
