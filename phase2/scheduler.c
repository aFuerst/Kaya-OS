#include "../h/const.h"
#include "../h/types.h"
#include "../e/initial.e"
#include "../e/pcb.e"
#include "/usr/local/include/umps2/umps/libumps.e"

/***********************************************************************
 *	This module implements Kaya's process scheduler and deadlock 
 *	detector
 * 
 * It also is the primary location for maintaining processor cpu usage.
 *
 * scheduler maintains the following variables
 *	int procCount;
 *	int sftBlkCount;
 *	pcb_PTR currProc;
 *	pcb_PTR readyQueue;
***********************************************************************/
 
/* global vars for maintaining cpu time usage */
cpu_t TODStarted;
cpu_t currentTOD;

/* external globals scheduler uses */
extern int procCount;
extern int sftBlkCount;
extern pcb_PTR currProc;
extern pcb_PTR readyQueue;
/*extern int semD[MAGICNUM];*/

/***********************************************************************
 * Scheduler function
 * 
 * Implements a simple Round-Robin scheduling algorithm
 * Primary maintainer for cpu time usage
 * 
 * After all processes are terminated the scheduler will halt the system
 * 
 * In the event of deadlock (procCount > 0, sftBlk == 0) the system will
 * 	PANIC()
 * 
 * 
 * 
***********************************************************************/
void scheduler() {

	/* was someone just running? */
	/* this means a process was running and was then blocked 
	 * or returned to readyQ for some reason */
	if(currProc != NULL){
		/* save how much time current process used on CPU */
		/* subtract current time from global start time to get this ^ */
		STCK(currentTOD);
		currProc->cpu_time = (currProc->cpu_time)
							+ (currentTOD - TODStarted);
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
	/* nothing was in readyQueue */
		currProc = NULL; /* no running process */
		
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
			setSTATUS((getSTATUS() | ALLOFF | IEON | IECON | IMON));
			WAIT(); /* run silent run deep */
		}

	}

}
/** end scheduler **/
