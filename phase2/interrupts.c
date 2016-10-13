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
extern int semD[MAGICNUM];
extern void copyState(state_PTR src, state_PTR dest);

/*
	This module implements the device interruption exception handler.
	This module will process all the device interrupts, inculding 
	Interval Timer interrupts, conterting device interrupts into V 
	operations on the appropriate semaphores.
*/
void interruptHandler(){
	unsigned int cause, interruptBits, device;
	int devNum, lineNum;
	devregarea_t* deviceReg = (devregarea_t*) RAMBASEADDR;
	state_PTR oldInt = (state_PTR) INTPOLDAREA;
	cause = oldInt -> s_cause;
	interruptBits = (cause & IPAREA) >> 8;
	lineNum = 0;
	while((interruptBits & 0x1) != TRUE){
		++lineNum; /* determine which line caused the interrupt */
		interruptBits = interruptBits >> 1;
	}
	device = deviceReg -> interrupt_dev[lineNum - 3];
	devNum = 0;
	while((device & 0x1) != TRUE){
		++devNum; /* determine which device caused the interrupt */
		device = device >> 1;
	}
	if(lineNum < 3){
			if(lineNum == 1){
				/* processor local timer */
				/* process has used up quantum */
				copyState(oldInt, &(currProc -> p_s));
				insertProcQ(&readyQueue, currProc);
				scheduler();
			}
			if(lineNum == 2){
				/* interval timer */
			}
	}
}
