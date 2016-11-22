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

/* globals taken from sceduler */
extern cpu_t TODStarted;
extern cpu_t currentTOD;

/* local functions used within this file */
HIDDEN void syscall1(state_PTR caller);
HIDDEN void syscall2();
HIDDEN void syscall3(state_PTR caller);
HIDDEN void syscall4(state_PTR caller);
HIDDEN void syscall5(state_PTR caller);
HIDDEN void syscall6(state_PTR caller);
HIDDEN void syscall7(state_PTR caller);
HIDDEN void syscall8(state_PTR caller);
HIDDEN void sys2Helper(pcb_PTR head);
HIDDEN void PassUpOrDie(state_PTR caller, int reason);

/* functions defined here and used globally  */
void copyState(state_PTR src, state_PTR dest);


/*********************** START TLB MANAGER MODULE *********************/

/***********************************************************************
 *	This module implements the TLB Pg Manager
 * 		goes to PassUpOrDie to see if a SYS5 exception vector has been
 * 		designated for the offending process
***********************************************************************/
void tlbManager(){
	state_PTR caller = (state_PTR) TBLMGMTOLDAREA;
	PassUpOrDie(caller, TLBTRAP);
}

/************************* END TLB MANAGER MODULE *********************/


/********************** START PROGRAM TRAP MODULE *********************/

/***********************************************************************
 *	Program Trap Handler
 * 		goes to PassUpOrDie to see if a SYS5 exception vector has been
 * 		designated for the offending process
***********************************************************************/
void pgmTrap(){
	state_PTR caller = (state_PTR) PGMTRAPOLDAREA;
	PassUpOrDie(caller, PROGTRAP);
}

/************************ END PROGRAM TRAP MODULE *********************/


/************************ START SYSCALL MODULE ************************/

/***********************************************************************
 * SYSCALL Handler
 * 
***********************************************************************/
 void syscallHandler(){
	state_PTR caller, pgm;
	int sysRequest;
	unsigned int callerStatus, temp;
	
	caller = (state_PTR) SYSCALLOLDAREA; /* cpu state of caller*/
	sysRequest = caller -> s_a0;
	callerStatus = caller -> s_status;
	
	if((sysRequest > 0) && (sysRequest < 9) && 
		((callerStatus & KUON) != ALLOFF)){
		pgm = (state_PTR) PGMTRAPOLDAREA;
		/* syscall is 1-8 and in not kernel mode */
		/* cause a program trap */
		copyState(caller, pgm);
		/* set cause to privlidged instruction exception */
		temp = (pgm -> s_cause) & ~(0xFF);
		(pgm -> s_cause) = (temp | (10 << 2));
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
			default: /* everything else not defined here */
				PassUpOrDie(caller, SYSTRAP);
			break;
	}
	/* should never get here */
	PANIC();
	/* individual syscalls decide who to pass control to */
}
 
/***********************************************************************
 * SYS1
 * Create_Process
 * Creates a new process to be a progeny of the caller
 * a1 contains the processor state area the time this instructions 
 * executed. This processor state shold be used for the initial state of
 * the newly created process Process requesting SYS1 continues to 
 * execute and exist. If new process cannot be created due to lack of 
 * resources (no more free PCBs) place -1 in v0, otherwise place 0 in v0
 * and return
***********************************************************************/
HIDDEN void syscall1(state_t* caller){
	pcb_PTR temp = allocPcb();
	if(temp == NULL){
		caller -> s_v0 = FAILURE;
		LDST(caller); /*return CPU to caller */
	}
	++procCount;
	/* make new process a child of current process */
	/*temp -> p_prnt = currProc;*/
	insertChild(currProc, temp);
	
	/* add to the ready queue */
	insertProcQ(&readyQueue, temp);

	/* copy CPU state to new process */
	copyState((state_PTR) caller -> s_a1, &(temp -> p_s));

	/* set return value */
	caller -> s_v0 = SUCCESS;
	/* return CPU to caller */
	LDST(caller);
}
	
/***********************************************************************
 * SYS2
 * Terminate_Process
 * Causes the executing process to cease to exist. All progeny of this
 * process are terminated as well. Execution of this instructions 
 * DOES NOT complete untill ALL progeny are terminated.
 * called by placing 2 in a0 and executing SYSCALL
***********************************************************************/
HIDDEN void syscall2(){
	if(emptyChild(currProc)){
		/* current process has no children */
		outChild(currProc);
		freePcb(currProc);
		--procCount;
	} else {
		/* recusrive call */
		sys2Helper(currProc);
	}
	/* no current process anymore */
	currProc = NULL;
	/* call scheduler */
	scheduler();
}

/***********************************************************************
 * Sys2Helper
 * 
 * Recursively removes all the children of head
 * Kills them if they are in a semaphore, readyQueue, or currProc
 * frees the PCB and decrements procCount accordingly
***********************************************************************/
HIDDEN void sys2Helper(pcb_PTR head){
	/* remove all children */
	while(!emptyChild(head)){
		/* nuke it till it pukes */
		sys2Helper(removeChild(head));
	}
	
	if(head -> p_semAdd != NULL){
		/* try and remove self from ASL */
		int* sem = head -> p_semAdd;
		
		/* debug */		
		outBlocked(head);
		/* check if blocked on device */
		if(sem >= &(semD[0]) && sem <= &(semD[MAGICNUM-1])){ 
			sftBlkCount--;
		} else {
			++(*(sem)); /* increment semaphore */
		}
	} else if (head == currProc) {
		/* try and remove process from it's parent */
		outChild(currProc);
	} else {
		/* try and remove self from readyQueue */
		outProcQ(&readyQueue, head);
	}
	/* free self after we have no more children */
	freePcb(head);
	--procCount;
}

/***********************************************************************
 * SYS3
 * Verhogen (V)
 * Perform a V operation on a semaphore. Place the value 3 in a0 and 
 * the physical address of the semaphore to be V'ed in a1
 * Returns control to the requesting process
***********************************************************************/
HIDDEN void syscall3(state_PTR caller){
	pcb_PTR newProc = NULL;
	int* semV = (int*) caller->s_a1;
	++(*semV); /* increment semaphore */
	if((*semV) <= 0) {
		/* something is waiting on the semaphore */
		newProc = removeBlocked(semV);
		if(newProc != NULL){
			/* add it to the ready queue */
			insertProcQ(&readyQueue, newProc);
		} else {
			/* nothing was waiting on semaphore */
		}
	}
	/* always return control to caller */
	LDST(caller);
}

/***********************************************************************
 * SYS4
 * Passeren (P)
 * Perform a P operation on a semaphore. Place the value 4 in a0 and 
 * the physical address of the semaphore to be V'ed in a1
 * May or may not return control to requesting process. If the semaphpre
 * value is less than 0 process will be blocked.
***********************************************************************/
HIDDEN void syscall4(state_PTR caller){
	int* semV = (int*) caller->s_a1;
	--(*semV); /* decrement semaphore */
	if((*semV) < 0){
		/* something already has control of the semaphore */
		copyState(caller, &(currProc -> p_s));
		insertBlocked(semV, currProc);
		scheduler();
	}
	/* nothing had control of the sem, return control to caller */
	LDST(caller);
}

/***********************************************************************
 * SYS5
 * Specify_Exception_State_Vector
 * does complicated tlb / PgmTrap / SYS/Bp things
 * Returns control to the requesting process
***********************************************************************/
HIDDEN void syscall5(state_PTR caller){
	switch(caller->s_a1) {
		case TLBTRAP:
			if(currProc->tlbNew != NULL) {
				syscall2(); /* already called for this type */
			}
			/* assign exception vector values */
			currProc->tlbNew = (state_PTR) caller->s_a3;
			currProc->tlbOld = (state_PTR) caller->s_a2;
			break;
			
		case PROGTRAP:
			if(currProc->pgmTrpNew != NULL) {
				syscall2(); /* already called for this type */
			}
			/* assign exception vector values */
			currProc->pgmTrpNew = (state_PTR) caller->s_a3;
			currProc->pgmTrpOld = (state_PTR) caller->s_a2;
			break;
			
		case SYSTRAP:
			if(currProc->sysNew != NULL) {
					syscall2(); /* already called for this type */
			}
			/* assign exception vector values */
			currProc->sysNew = (state_PTR) caller->s_a3;
			currProc->sysOld = (state_PTR) caller->s_a2;
			break;
	}
	LDST(caller);
}

/***********************************************************************
 * SYS6
 * Get_CPU_Time
 * Causes the processor time (in microseconds) used by the requesting
 * process to be placed/returned callers v0. This means that the nucleus
 * must record (in the ProcBlk) the amount of processor time used by 
 * each process. 
***********************************************************************/
HIDDEN void syscall6(state_PTR caller){
	cpu_t temp;
	/* get current time, subtract from global start time */
	STCK(temp);
	/* add that to process used time and give to process */
	(currProc->cpu_time) = (currProc->cpu_time) + (temp - TODStarted);
	(caller -> s_v0) = (currProc->cpu_time);
	/* update start time */
	STCK(TODStarted);
	LDST(caller);
}

/***********************************************************************
 * SYS7
 * Wait_For_Clock
 * Perform a P operation on the nucleus maintained pseudo-clock timer
 * semaphore. This semaphore is V'ed every 100 milliseconds 
 * automatically by the nucleus. 
***********************************************************************/
HIDDEN void syscall7(state_PTR caller){
	/* interval timer semaphore is last in semD list */
	int* semV = (int*) &(semD[MAGICNUM-1]);
	--(*semV); /* decrement semaphore */
	insertBlocked(semV, currProc);
	copyState(caller, &(currProc -> p_s)); /* store state back in curr*/
	++sftBlkCount;
	scheduler();
}

/***********************************************************************
 * SYS8
 * Wait_For_IO_Device
 * 
 * Performs a P operation on the requested device semaphore
 * Line number is in a1 and device number is in a2
 * 
***********************************************************************/
HIDDEN void syscall8(state_PTR caller){
	int lineNum, deviceNum, read, index;
	int *sem;
	lineNum = caller -> s_a1;
	deviceNum = caller -> s_a2;
	read = caller -> s_a3; /* terminal read / write */
	/*index = 0;*/
	
	if(lineNum < 3 || lineNum > 7){
		syscall2(); /* illegal IO wait request */
	}
	
	/* compute which device */
	if(lineNum == 7 && read == TRUE){
		/* terminal read operation */
		index = DEVPERINT * (lineNum - DEVWOSEM + read) + deviceNum;
	} else {
		/* anything else */
		index = DEVPERINT * (lineNum - DEVWOSEM) + deviceNum;
	}
	sem = &(semD[index]);
	--(*sem);
	if((*sem) < 0) {
		insertBlocked(sem, currProc);
		copyState(caller, &(currProc -> p_s));
		++sftBlkCount;
		scheduler();
	}
	/* ERROR? */
	/* EDIT */
	LDST(caller);
}

/***********************************************************************
 * PassUpOrDie
 * 
 * Tests if an exception vector has been set for whatever unhandled 
 * operation has been encountered. If so the error is passed up to 
 * that handler. If none had been set the process is nuked.
 * 
 * parameters:
 * 		state_PTR caller, state pointer to location where offending 
 * 			process is located
 * 		int reason, value representing SYSCALL, TLBTRAP, or PROGTRAP
 * 
***********************************************************************/
HIDDEN void PassUpOrDie(state_PTR caller, int reason){
	/* has a sys 5 been called? */
	switch(reason){
		case SYSTRAP: /* syscall exception */
			if(currProc -> sysNew != NULL){
				/* yes a sys trap created */
				copyState(caller, currProc -> sysOld);
				LDST(currProc -> sysNew);
			}
		break;
		case TLBTRAP: /* TLB trap exception */
			if(currProc -> tlbNew != NULL){
				/* yes a tlb trap created */
				copyState(caller, currProc -> tlbOld);
				LDST(currProc -> tlbNew);
			}
		break;
		case PROGTRAP: /* pgmTrp exception */
			if(currProc -> pgmTrpNew != NULL){
				/* yes a ogm trap created */
				copyState(caller, currProc -> pgmTrpOld);
				LDST(currProc -> pgmTrpNew);
			}
		break;
	}
	syscall2(); /* if no vector defined, kill process */
}

/***********************************************************************
 * CopyState
 * Copies the processor state pointed to by src to the location pointed
 * to by dest
***********************************************************************/
void copyState(state_PTR src, state_PTR dest){
	int i;
	dest -> s_asid = src -> s_asid;
	dest -> s_status = src -> s_status;
	dest -> s_pc = src -> s_pc;
	dest -> s_cause = src -> s_cause;
	for(i=0; i < STATEREGNUM; ++i){
		dest -> s_reg[i] = src -> s_reg[i];
	}
}

/************************* END SYSCALL MODULE *************************/
