#include "../h/const.h"
#include "../h/types.h"
#include "../e/initial.e"
#include "../e/scheduler.e"
#include "../e/pcb.e"
#include "../e/asl.e"
#include "/usr/local/include/umps2/umps/libumps.e"

/* global variables taken from initial */
extern int procCount;
extern int sftBlkCount;
extern pcb_PTR currProc;
extern pcb_PTR readyQueue;
extern int semD[MAGICNUM];

/* local functions used within this file */
HIDDEN void syscall1(state_t* caller);
HIDDEN void syscall2();
HIDDEN void syscall3(state_t* caller);
HIDDEN void syscall4(state_t* caller);
HIDDEN void syscall5(state_t* caller);
HIDDEN void syscall6(state_t* caller);
HIDDEN void syscall7(state_t* caller);
HIDDEN void syscall8(state_t* caller);
HIDDEN void sys2Helper(pcb_PTR head);
HIDDEN void PassUpOrDie();
HIDDEN void copyState(state_PTR src, state_PTR dest);

void debugSys(int a, int b){
	int i;
	i=0;
}

/*********************** START TLB MANAGER MODULE *********************/

/*
 *	This module implements the TLB Pg Manager
 */
void tlbManager(){
	
}

/************************* END TLB MANAGER MODULE *********************/


/********************** START PROGRAM TRAP MODULE *********************/
/*
 *	Program Trap Handler
 */
void pgmTrap(){
	
}

/************************ END PROGRAM TRAP MODULE *********************/


/************************ START SYSCALL MODULE ************************/

/*
 * SYSCALL Handler
 * 
 */
 void syscallHandler(){
	state_t* caller, *pgm;
	int sysRequest;
	unsigned int callerStatus;
	
	caller = (state_t*) SYSCALLOLDAREA; /* cpu state of caller*/
	sysRequest = caller -> s_a0;
	callerStatus = caller -> s_status;	
	
	if(sysRequest > 0 && sysRequest < 9 && 
		!((callerStatus & KUON) == KUON)){
		/* syscall is 1-8 and in not kernel mode */
		/* cause a program trap */
		copyState(caller, (state_t*) PBGTRAPOLDAREA);
		/*
		pgm = (state_t*) PBGTRAPOLDAREA;
		*/
		/* copy state to program trap old */
		/*
		pgm -> s_asid = caller -> s_asid;
		pgm -> s_status = caller -> s_status;
		pgm -> s_pc = caller -> s_pc;
		pgm -> s_cause = caller -> s_cause;
		*/
		/* copy over registers, reuse sysRequest for loop counter */
		/*
		for(sysRequest=0; sysRequest < STATEREGNUM; sysRequest++){
			pgm -> s_reg[sysRequest] = caller -> s_reg[sysRequest];
		}
		*/
		/* set cause to privlidged instruction exception */
		pgm -> s_cause = 10;
		pgmTrap();
	}
	/* increment caller's PC to next instruction */
	caller -> s_pc = caller -> s_pc + 4;
	/* handle valid syscall */
	switch(sysRequest){
			case CREATEPROCESS:
				syscall1(caller);
			break;
			case TERMINATEPROCESS:
				syscall2();
			break;
			case VERHOGEN:
				syscall3(caller);
			break;
			case PASSEREN:
				syscall4(caller);
			break;
			case SPECTRAPVEC:
				syscall5(caller);
			break;
			case GETCPUTIME:
				syscall6(caller);
			break;
			case WAITCLOCK:
				syscall7(caller);
			break;
			case WAITIO:
				syscall8(caller);
			break;
			default: /* everything else not defined */
				PassUpOrDie();
			break;
	}
	/* should never get here */
	PANIC();
	/* individual syscalls decide who to pass control to */
}
 
/*
 * SYS1
 * Create_Process
 * Creates a new process to be a progeny of the caller
 * a1 contains the processor state area the time this instructions 
 * executed. This processor state shold be used for the initial state of
 * the newly created process Process requesting SYS1 continues to 
 * execute and exist. If new process cannot be created due to lack of 
 * resources (no more free PCBs) place -1 in v0, otherwise place 0 in v0
 * and return
 */
HIDDEN void syscall1(state_t* caller){
	pcb_PTR temp = allocPcb();
	if(temp == NULL){
		caller -> s_v0 = -1;
		LDST(caller); /*return CPU to caller */
	}
	procCount++;
	/* make new process a child of current process */
	temp -> p_prnt = currProc;
	insertChild(currProc, temp);
	
	/* add to the ready queue */
	insertProcQ(&currProc, temp);

	/* copy CPU state to new process */
	copyState(&(temp -> p_s), (state_PTR) caller -> s_a1);
	/*	
	newState = (state_t*) caller -> s_a1;
	temp -> p_s.s_asid = newState -> s_asid;
	temp -> p_s.s_status = newState -> s_status;
	temp -> p_s.s_pc = newState -> s_pc;
	temp -> p_s.s_cause = newState -> s_cause;
	for(i=0; i < STATEREGNUM; i++){
		temp -> p_s.s_reg[i] = newState -> s_reg[i];
	}
	*/
	/* set return value */
	caller -> s_v0 = 0;
	/* return CPU to caller */
	LDST(caller);
}
	
/*
 * SYS2
 * Terminate_Process
 * Causes the executing process to cease to exist. All progeny of this
 * process are terminated as well. Execution of this instructions 
 * DOES NOT complete untill ALL progeny are terminated.
 * called by placing 2 in a0 and executing SYSCALL
 */
HIDDEN void syscall2(){
	if(emptyChild(currProc)){
		/* process has no children */
		outChild(currProc);
		freePcb(currProc);
		procCount--;
	} else {
		sys2Helper(currProc);
	}
	/* call scheduler */
	scheduler();
}

/*
 * Sys3Helper
 * 
 * Recursively removes all the children of head
 * Kills them if they are in a semaphore, readyQueue, or currProc
 * frees the PCB and decrements procCount accordingly
 */
HIDDEN void sys2Helper(pcb_PTR head){
	/* remove all children */
	while(!emptyChild(head)){
		sys2Helper(removeChild(head));
	}

	if(head -> p_semAdd != NULL){
		/* try and remove self from ASL */
		outBlocked(head);
		sftBlkCount--;
	} else if (head == currProc) {
		/* try and remove process from it's parent */
		outChild(currProc);
	} else {
		/* try and remove self from readyQueue */
		outProcQ(&readyQueue, head);
	}
	/* free self after we have no more children */
	freePcb(head);
	procCount--;
}

/*
 * SYS3
 * Verhogen (V)
 * Perform a P operation on a semaphore. Place the value 3 in a0 and 
 * the physical address of the semaphore to be V'ed in a1
 */
HIDDEN void syscall3(state_t* caller){
	int* semV = (int*) caller->s_a1;
	*semV--;
	if(*semV < 0){
		insertBlocked(semV, currProc);
		copyState(caller, &(currProc -> p_s));
		scheduler();
	}
	LDST(caller);
}

/*
 * SYS4
 * Passeren (P)
 * Perform a V operation on a semaphore. Place the value 4 in a0 and 
 * the physical address of the semaphore to be V'ed in a1
 */
HIDDEN void syscall4(state_t* caller){
	pcb_PTR newProc;
	int* semV = (int*) caller->s_a1;
	*semV++;
	if(*semV <= 0) { 
		newProc = removeBlocked(semV);
		insertProcQ(&readyQueue, newProc);
	}
	LDST(caller);
}

/*
 * SYS5
 * Specify_Exception_State_Vector
 * does complicated tlb / PgmTrap / SYS/Bp things
 */
HIDDEN void syscall5(state_t* caller){
	
}

/*
 * SYS6
 * Get_CPU_Time
 * Causes the processor time (in microseconds) used by the requesting
 * process to be placed/returned callers v0. This means that the nucleus
 * must record (in the ProcBlk) the amount of processor time used by 
 * each process. 
 */
HIDDEN void syscall6(state_t* caller){
	caller->s_v0 = currProc->cpu_time;
	LDST(caller);
}

/*
 * SYS7
 * Wait_For_Clock
 * Perform a V operation on the nucleus maintained pseudo-clock timer
 * semaphore. This semaphore is V'ed every 100 milliseconds 
 * automatically by the nucleus. 
 */
HIDDEN void syscall7(state_t* caller){
	
}

/*
 * SYS8
 * Wait_For_IO_Device
 * Perform a V operation on the nucleus maintained pseudo-clock timer
 * semaphore. This semaphore is V'ed every 100 milliseconds 
 * automatically by the nucleus. 
 */
HIDDEN void syscall8(state_t* caller){
	
}

/*
 * PassUpOrDie
 * 
 */
HIDDEN void PassUpOrDie(){
	
}

/* 
 * CopyState
 * Copies the state pointed to by src to the location pointed
 * to be dest
 * 
 */
HIDDEN void copyState(state_PTR src, state_PTR dest){
	int i;
	dest -> s_asid = src -> s_asid;
	dest -> s_status = src -> s_status;
	dest -> s_pc = src -> s_pc;
	dest -> s_cause = src -> s_cause;
	for(i=0; i < STATEREGNUM; i++){
		dest -> s_reg[i] = src -> s_reg[i];
	}
}

/************************* END SYSCALL MODULE *************************/
