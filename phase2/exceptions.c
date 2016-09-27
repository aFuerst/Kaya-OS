/*
	This module implements the TLB Pg
*/
void tlbManager(){
	
}

/*
	Program Trap
*/
void pgmTrap(){
	
}


/*
 * SYSCALL
 * 
 * */
 void syscallHandler(){
	 
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

/*
 * SYS2
 * Terminate_Process
 * Causes the executing process to cease to exist. All progeny of this
 * process are terminated as well. Execution of this instructions 
 * DOES NOT complete untill ALL progeny are terminated.
 * called by placing 2 in a0 and executing SYSCALL
 */

/*
 * SYS3
 * Verhogen (V)
 * Perform a V operation on a semaphore. Place the value 3 in a0 and 
 * the physical address of the semaphore to be V'ed in a1
 */

/*
 * SYS4
 * Passeren (P)
 * Perform a V operation on a semaphore. Place the value 4 in a0 and 
 * the physical address of the semaphore to be V'ed in a1
 * */

/*
 * SYS5
 * Specify_Exception_State_Vector
 * does complicated tlb / PgmTrap / SYS/Bp things
 * */

/*
 * SYS6
 * Cet_CPU_Time
 * Causes the processor time (in microseconds) used by the requesting
 * process to be placed/returned callers v0. This means that th nucleus
 * must record (in the ProcBlk) the amount of processor time used by 
 * each process. 
 * */
 
/*
 * SYS7
 * Wait_For_Clock
 * Perform a V operation on the nucleus maintained pseudo-clock timer
 * semaphore. This semaphore is V'ed every 100 milliseconds 
 * automatically by the nucleus. 
 * */

/*
 * SYS8
 * Wait_For_IO_Device
 * Perform a V operation on the nucleus maintained pseudo-clock timer
 * semaphore. This semaphore is V'ed every 100 milliseconds 
 * automatically by the nucleus. 
 * */
