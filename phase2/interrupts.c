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
extern unsigned int TODStarted;
extern int semD[MAGICNUM];
extern void copyState(state_PTR src, state_PTR dest);

void debugInt(int a, int b, int c, int d){
	int i;
	i=0;
}

/*
	This module implements the device interruption exception handler.
	This module will process all the device interrupts, inculding 
	Interval Timer interrupts, conterting device interrupts into V 
	operations on the appropriate semaphores.
*/

HIDDEN int getDeviceNumber(unsigned int* bitMap);

/*
 * interruptHandler
 * 
 * 
 */
void interruptHandler(){
	unsigned int cause, startTime, endTime;
	int devNum, lineNum;
	device_t *devReg;
	int index, status;
	int* sem;
	pcb_PTR waiter;
	state_PTR oldInt = (state_PTR) INTPOLDAREA;
	debugInt(0xabcdabcd, 0, 0, 0);
	STCK(startTime); /* save time started in interrupt handler */
	
	cause = oldInt -> s_cause; /* which line caused interrupt */
	/* active interrupting lines */
	cause = (cause & IPAREA) >> 8; 
	lineNum = 0;

	if((cause & FIRST) != 0) { /* CPU interrupt */
		/* should never happen */
		PANIC();
	}
	
	else if((cause & SECOND) != 0){ /* local timer */
		
	}
	
	else if((cause & THIRD) != 0){ /* interval timer */
		
	}
	
	else if((cause & FOURTH) != 0){ /* disk device  */
		lineNum = 3;
		/*
		devNum = getDeviceNumber((unsigned int*) INTBITMAP + ((lineNum-3) * WORDLEN));
		devReg = (device_t *) INTDEVREG + ((lineNum-3) * DEVREGSIZE * DEVPERINT) + (devNum * DEVREGSIZE);
		*/
	}
	
	else if((cause & FIFTH) != 0){ /* tape device */
		lineNum = 4;
		/*
		devNum = getDeviceNumber((unsigned int*) INTBITMAP + ((lineNum-3) * WORDLEN));
		devReg = (device_t *) INTDEVREG + ((lineNum-3) * DEVREGSIZE * DEVPERINT) + (devNum * DEVREGSIZE);
		*/
	}
	
	else if((cause & SIXTH) != 0){ /* network device */
		lineNum = 5;
		/*
		devNum = getDeviceNumber((unsigned int*) INTBITMAP + ((lineNum-3) * WORDLEN));
		devReg = (device_t *) INTDEVREG + ((lineNum-3) * DEVREGSIZE * DEVPERINT) + (devNum * DEVREGSIZE);
		*/
	}
	
	else if((cause & SEVENTH) != 0){ /* printer device */
		lineNum = 6;
		/*
		devNum = getDeviceNumber((unsigned int*) INTBITMAP + ((lineNum-3) * WORDLEN));
		devReg = (device_t *) INTDEVREG + ((lineNum-3) * DEVREGSIZE * DEVPERINT) + (devNum * DEVREGSIZE);
		*/
	}
	
	else if((cause & EIGHTH) != 0){ /* terminal device */
		lineNum = 7;
		devNum = getDeviceNumber((unsigned int*) INTBITMAP + 
					((lineNum-3) * WORDLEN));
		devReg = (device_t *) INTDEVREG + ((lineNum-DEVWOSEM)
					* DEVREGSIZE * DEVPERINT) + (devNum * DEVREGSIZE);
		/* TODO: handle terminal interrupt differently! */
		debugInt(0xabcdabcd, 8, 8, 8);
		/* check if a char was transmitted successfully 
		 * (sending has higher priority) */
			index = DEVPERINT * (lineNum - DEVWOSEM) + devNum;
			/* ACK recv_status */
			
		
		/* check if a char was recieved successfully */
			/* semaphore for read is one row after transmit */
			index = DEVPERINT * (lineNum - DEVWOSEM + 1) + devNum; 
			/* do something with returned char in trasnm_status */
			/* ACK transm_status */

		goto V;
	} else {
		/* interrupt caused for unknown reason */
		PANIC();
	}
	/* all devices except terminal can be handled the same way */
	devNum = getDeviceNumber((unsigned int*) INTBITMAP + 
					((lineNum-DEVWOSEM) * WORDLEN));
	devReg = (device_t *) INTDEVREG + ((lineNum-DEVWOSEM)
					* DEVREGSIZE * DEVPERINT) + (devNum * DEVREGSIZE);

	/* store device status */
	status = devReg -> d_status;
	/* ACK device */
	devReg -> d_command = ACK;	
	/* compute which semaphore to V*/
	index = DEVPERINT * (lineNum - DEVWOSEM) + devNum;

V:	/* goto for Ving semaphore and returning to scheduler */
	sem = &(semD[index]);
	(*sem)++;

	if((*sem) < 0) {
		/* V semaphore process was blocked on */
		waiter = removeBlocked(sem);
		waiter -> p_s.s_v0 = status;
		insertProcQ(&readyQueue, waiter);
		--sftBlkCount;		
	} else {
		/* ERROR? */
	}
	if(currProc != NULL){ /* was not in a wait state */
		/* take time used in interrupt handler and add it to TODstarted
		 * so currProc is not billed for time
		 */
		STCK(endTime);
		TODStarted = TODStarted - (endTime-startTime); 
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
