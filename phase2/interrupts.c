#include "../h/const.h"
#include "../h/types.h"
#include "../e/initial.e"
#include "../e/scheduler.e"
#include "../e/pcb.e"
#include "../e/asl.e"
#include "/usr/local/include/umps2/umps/libumps.e"

extern int procCount;
extern int sftBlkCount;
extern pcb_PTR currProc;
extern pcb_PTR readyQueue;
extern cpu_t TODStarted;
extern int semD[MAGICNUM];
extern void copyState(state_PTR src, state_PTR dest);

HIDDEN void debugInt(int a, int b, int c, int d){
	int i;
	i=0;
}

/*
	This module implements the device interruption exception handler.
	This module will process all the device interrupts, inculding 
	Interval Timer interrupts, conterting device interrupts into V 
	operations on the appropriate semaphores.
*/

/* local functions */
HIDDEN int getDeviceNumber(unsigned int* bitMap);
HIDDEN void finish(cpu_t startTime);

/*
 * interruptHandler
 * 
 * 
 */
void interruptHandler(){
	unsigned int cause;
	cpu_t startTime, endTime;
	int devNum, lineNum;
	device_t *devReg;
	int index, status, tranStatus;
	int* semV;
	pcb_PTR waiter;
	state_PTR oldInt = (state_PTR) INTPOLDAREA;
	STCK(startTime); /* save time started in interrupt handler */

	cause = oldInt -> s_cause; /* which line caused interrupt */
	/* active interrupting lines */
	cause = (cause & IPAREA) >> 8; 
	lineNum = 0;	
	/* An interrupt line will always be on if in handler */

	if((cause & FIRST) != 0) { /* CPU interrupt, line 0 */
		/* should never happen */
		PANIC();
	}

	else if((cause & SECOND) != 0){ /* local timer, line 1 */
		/* someone's clock ran out, call scheduler */
		finish(startTime);
	}

	else if((cause & THIRD) != 0){ /* interval timer, line 2 */
		/* unblock everyone who was blocked on the semaphore */	
		LDIT(INTTIME);/* load 100 ms into interval timer*/
		semV = (int*) &(semD[MAGICNUM-1]);
		while(headBlocked(semV) != NULL) {
			waiter = removeBlocked(semV);
			STCK(endTime);
			if(waiter != NULL){
				insertProcQ(&readyQueue, waiter);
				/* bill process for time in interrupt handler */
				(waiter->cpu_time) = 
							(waiter->cpu_time) + (endTime - startTime);
				--sftBlkCount;
				
			}
		}
		(*semV) = 0;
		finish(startTime);
		/* finish up */
	}

	else if((cause & FOURTH) != 0){ /* disk device  */
		lineNum = 3;
	}

	else if((cause & FIFTH) != 0){ /* tape device */
		lineNum = 4;
	}

	else if((cause & SIXTH) != 0){ /* network device */
		lineNum = 5;
	}

	else if((cause & SEVENTH) != 0){ /* printer device */
		lineNum = 6;
	}

	else if((cause & EIGHTH) != 0){ /* terminal device */
		lineNum = 7;

	} else {
		/* interrupt caused for unknown reason */
		PANIC();
	}

	/* get device number */
	devNum = getDeviceNumber((unsigned int*) INTBITMAP + 
					((lineNum-DEVWOSEM) * WORDLEN));
	
	/* get actual register associated with that device */
	devReg = (device_t *) (INTDEVREG + ((lineNum-DEVWOSEM)
					* DEVREGSIZE * DEVPERINT) + (devNum * DEVREGSIZE));
	
	/* part that will be different for terminal! */
	if(lineNum != 7){ /* not terminal */
		/* store device status */
		status = devReg -> d_status;
		/* ACK device */
		devReg -> d_command = ACK;
		/* compute which semaphore to V*/
		index = DEVPERINT * (lineNum - DEVWOSEM) + devNum;
		
	} else { /* terminal */
		tranStatus = (devReg -> t_transm_status & 0xFF);
		/* write terminal */
		if( tranStatus == 3 || tranStatus == 4 || tranStatus == 5 ) {
			index = (DEVPERINT * (lineNum - DEVWOSEM)) + devNum;
			status = devReg -> t_transm_status;
			devReg -> t_transm_command = ACK;
		}
		else {
			/* if it is a read */
			index = DEVPERINT * (lineNum - DEVWOSEM + 1) + devNum;
			status = devReg -> t_recv_status;
			devReg -> t_recv_command = ACK;
		}
	}
	/* everything after this is same again */

	/* V semaphore associated with that device */
	semV = &(semD[index]);
	++(*semV);

	if((*semV) <= 0) {
		/* V semaphore process was blocked on */
		waiter = removeBlocked(semV);
		if(waiter != NULL){
			waiter -> p_s.s_v0 = status;
			insertProcQ(&readyQueue, waiter);
			--sftBlkCount;
		}
	} else {
		/* ERROR? */
	}
	if(currProc != NULL){ /* was not in a wait state */
		/* take time used in interrupt handler and add it to TODstarted
		 * so currProc is not billed for time
		 */
		finish(startTime);
	}
	finish(startTime);
}

HIDDEN void finish(cpu_t startTime){
	cpu_t endTime;
	state_PTR oldInt = (state_PTR) INTPOLDAREA;
	if(currProc != NULL){ /* was not in a wait state */
		/* take time used in interrupt handler and add it to TODstarted
		 * so currProc is not billed for time
		 */
		STCK(endTime);
		TODStarted = TODStarted + (endTime-startTime);
		/* return running process to ready queue */
		copyState(oldInt, &(currProc ->p_s));
		insertProcQ(&readyQueue, currProc);
	}
	scheduler();
}

/*
 * getDeviceNumber
 * 
 * Takes a pointer to a device bitmap. Returns the position of the
 * highest priority bit in that bitmap, starting with 0
 *  
 */
HIDDEN int getDeviceNumber(unsigned int* bitMap) {
	unsigned int cause = *bitMap;
	if((cause & FIRST) != 0) {
		return 0;
	}

	else if((cause & SECOND) != 0){
		return 1;
	}

	else if((cause & THIRD) != 0){
		return 2;
	}

	else if((cause & FOURTH) != 0){
		return 3;
	}

	else if((cause & FIFTH) != 0){
		return 4;
	}

	else if((cause & SIXTH) != 0){
		return 5;
	}

	else if((cause & SEVENTH) != 0){
		return 6;
	}

	else if((cause & EIGHTH) != 0){
		return 7;
	}

	return 0;
}
