/*
	This module implements main() and exports the nucleus's global 
	variables. (E.G. Process Count, device semaphores, etc.)
	Contains the bootstrap code that is run on machine startup
*/

#include "../e/asl.e"
#include "../e/pcb.e"
#include "../e/scheduler.e"
#include "../h/const.h"
#include "../h/types.h"
#include "p2test.h"

void debugA(int i){
	int q;
	q = 0;
}

/* global variables */
int processCount, softBlockCount;
pcb_PTR currentProcess;
pcb_PTR readyQueue;

/* does the things */
void main(){
	int i;
	unsigned int RAMTOP;
	state_PTR newLocation;
	devregarea_t* deviceInfo = (devregarea_t *) 0x10000000;

	/* calcualte RAMTOP */
	RAMTOP = (deviceInfo -> rambase) + (deviceInfo -> ramsize);

	/* Initialize the PCB and ASL lists */
	initPcbs();
	initASL();

	/* initialize global variables */
	readyQueue = mkEmptyProcQ();
	currentProcess = NULL;
	processCount = softBlockCount = 0;

	/* Initialize 4 "new" interrupt vectors */
	/* syscall */
	newLocation =  (state_PTR)SYSCALLNEWAREA;
	newLocation -> s_sp = RAMTOP;
	newLocation -> s_pc = (memaddr) 0; /* set to actual address */
	newLocation -> s_t9 = (memaddr) 0; /* set to actual address */

	/* pgmtrp */
	newLocation =  (state_PTR)PBGTRAPNEWAREA;
	newLocation -> s_sp = RAMTOP;
	newLocation -> s_pc = (memaddr) 0;/* set to actual address */
	newLocation -> s_t9 = (memaddr) 0; /* set to actual address */

	/* tlbmgmt */
	newLocation =  (state_PTR)TBLMGMTNEWAREA;
	newLocation -> s_sp = RAMTOP;
	newLocation -> s_pc = (memaddr) 0;/* set to actual address */
	newLocation -> s_t9 = (memaddr) 0; /* set to actual address */	

	/* interrupt */
	newLocation =  (state_PTR)INTPNEWAREA;
	newLocation -> s_sp = RAMTOP;
	newLocation -> s_pc = (memaddr) 0;/* set to actual address */
	newLocation -> s_t9 = (memaddr) 0; /* set to actual address */

	/* create a semD for each device and set to 0 */
	int semD[DEVINTNUM * DEVPERINT];
	for(i = 0; i < (DEVINTNUM * DEVPERINT); ++i){
		semD[i] = 0;
	}

	/* create an initial process */	
	currentProcess = allocPcb();
	currentProcess -> p_s.s_sp = (RAMTOP - PAGESIZE);
	currentProcess -> p_s.s_pc = (memaddr)test;

	/* insert first process into readyQ */
	insertProcQ(&readyQueue, currentProcess);

	/* send control over to the scheduler */

	return;
}
