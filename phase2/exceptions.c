#include "../h/const.h"
#include "../h/types.h"
#include "../e/initial.e"
#include "../e/scheduler.e"
#include "../e/pcb.e"
#include "../e/asl.e"
#include "/usr/local/include/umps2/umps/libumps.e"

/* global variables from initial */
extern int procCount;
extern int sftBlkCount;
extern pcb_PTR currProc;
extern pcb_PTR readyQueue;
extern int semD[MAGICNUM];

void syscall1(state_t* caller);
void syscall2();
void syscall3();
void syscall4();
void syscall5();
void syscall6();
void syscall7();
void syscall8();
void sys2Helper(pcb_PTR head);
void PassUpOrDie();

/*
 *	This module implements the TLB Pg Manager
 */
void tlbManager(){
	
}

/*
 *	Program Trap Handler
 */
void pgmTrap(){
	
}


/*
 * SYSCALL Handler
 * 
 */
 void syscallHandler(){
	state_t* caller, *pgm;
	int sysRequest = caller -> s_a0;
	unsigned int callerStatus = caller -> s_status;
	caller = (state_t*)SYSCALLOLDAREA; /* cpu state of caller*/

	if(sysRequest > 0 && sysRequest < 9 && 
		!((callerStatus & KUON) == KUON)){
		/* syscall is 1-8 and in not kernel mode */
		/* cause a program trap */
		pgm = (state_t*) PBGTRAPOLDAREA;
		/* copy state to program trap old */
		pgm -> s_asid = caller -> s_asid;
		pgm -> s_status = caller -> s_status;
		pgm -> s_pc = caller -> s_pc;
		pgm -> s_cause = caller -> s_cause;
		/* copy over registers, reuse sysRequest for loop counter */
		for(sysRequest=0; sysRequest < STATEREGNUM; sysRequest++){
			pgm -> s_reg[sysRequest] = caller -> s_reg[sysRequest];
		}
		/* Ask mikey if there is a better way! */
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
				syscall3();
			break;
			case PASSEREN:
				syscall4();
			break;
			case SPECTRAPVEC:
				syscall5();
			break;
			case GETCPUTIME:
				syscall6();
			break;
			case WAITCLOCK:
				syscall7();
			break;
			case WAITIO:
				syscall8();
			break;
			default: /* everything else not defined */
				PassUpOrDie();
			break;
	}
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
void syscall1(state_t* caller){
	pcb_PTR temp = allocPcb();
	state_t* newState;
	int i;
	if(temp == NULL){
		caller -> s_v0 = -1;
		return;
	}
	procCount++;
	/* make new process a child of current process */
	temp -> p_prnt = currProc;
	insertChild(currProc, temp);
	
	/* add to the ready queue */
	insertProcQ(&currProc, temp);

	newState = (state_t*) caller -> s_a1;
	/* copy state to program trap old */
	temp -> p_s.s_asid = newState -> s_asid;
	temp -> p_s.s_status = newState -> s_status;
	temp -> p_s.s_pc = newState -> s_pc;
	temp -> p_s.s_cause = newState -> s_cause;
	for(i=0; i < STATEREGNUM; i++){
		temp -> p_s.s_reg[i] = newState -> s_reg[i];
	}

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
void syscall2(){
	if(emptyChild(currProc)){
		freePcb(currProc);
	} else {
		sys2Helper(currProc);
	}
	scheduler();
}

void sys2Helper(pcb_PTR head){
	/* remove all children */
	while(!emptyChild(head)){
		sys2Helper(removeChild(head));
	}
	/* try and remove self from readyQueue */
	outProcQ(&readyQueue, head);
	/* try and remove self from ASL */
	outBlocked(head);
	/* free self after we have no more children */
	freePcb(head);
	procCount--;
}

/*
 * SYS3
 * Verhogen (V)
 * Perform a V operation on a semaphore. Place the value 3 in a0 and 
 * the physical address of the semaphore to be V'ed in a1
 */
void syscall3(){
	
}


/*
 * SYS4
 * Passeren (P)
 * Perform a V operation on a semaphore. Place the value 4 in a0 and 
 * the physical address of the semaphore to be V'ed in a1
 */
void syscall4(){
	
}

/*
 * SYS5
 * Specify_Exception_State_Vector
 * does complicated tlb / PgmTrap / SYS/Bp things
 */
 void syscall5(){
	
}
 

/*
 * SYS6
 * Cet_CPU_Time
 * Causes the processor time (in microseconds) used by the requesting
 * process to be placed/returned callers v0. This means that th nucleus
 * must record (in the ProcBlk) the amount of processor time used by 
 * each process. 
 */
 void syscall6(){
	
}

/*
 * SYS7
 * Wait_For_Clock
 * Perform a V operation on the nucleus maintained pseudo-clock timer
 * semaphore. This semaphore is V'ed every 100 milliseconds 
 * automatically by the nucleus. 
 */
void syscall7(){
	
}

/*
 * SYS8
 * Wait_For_IO_Device
 * Perform a V operation on the nucleus maintained pseudo-clock timer
 * semaphore. This semaphore is V'ed every 100 milliseconds 
 * automatically by the nucleus. 
 */
void syscall8(){
	
}

/*
 * PassUpOrDie
 * 
 */
void PassUpOrDie(){
	
}
