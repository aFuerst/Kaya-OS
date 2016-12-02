#include "../h/const.h"
#include "../h/types.h"
#include "/usr/local/include/umps2/umps/libumps.e"

/*
 * Implements test and all U-Proc initialization routines
 * Exports VM-IO support level global variables 
 * (swap-pool, mutual exclusion semaphores)
 * 
 * 
 */
 
 
/* global variables! */
pteOS_t kSegOS;
pte_t kUSeg3;
swapPool_t swapPool[SWAPSIZE];
int disk0Sem, disk1Sem;
Tproc_t uProcStates[MAXUSERPROC];

void test() {
	 	 
	/* Local Variables */
	int i;
	int j;
	state_t procState;
	segTable_t* segTable;
	
	disk0Sem = disk1Sem = 0;
		
	/* page table */
	kSegOS.header = (PGTBLMAGICNUM << 32) | KSEGOSPTESIZE;
	for (i = 0; i < KSEGOSPTESIZE; i++){
		/* stuff */
	}
	
	
	kUSeg3.header = ? ;
	for (i = 0; i < KUSEGPTESIZE; ++i){
		
		kUSeg3.pteTable[i].pte_entryHI = ? ;
		kUSeg3.pteTable[i].pte_entryLO = ? ;
	}
	
	/* swap pool */
	for (i = 0; i < SWAPSIZE; i++){
				
		swapPool[i].asid = -1;
		swapPool[i].pte = NULL;
	}
	
	/*Init swap semaphore to 1 for mutual exclusion*/
	swapSem = 1;
	
	/* Init the array of semaphores to 1 */
	for (i = 0; i < MAXSEM; ++i){
		mutexSemArray[i] = 1;
	}
	
	/* master sem is 0 for synchronization*/
	masterSem = 0;
		
	/* Init each process */
	for (i = 1; i < MAXUSERPROC + 1; ++i){
		/* these items need editing */
		uProcs[i-1].Tp_pte.header = 1;
														   
		for(j = 0; j < KUSEGPTESIZE; ++j){
			
			uProcs[i-1].Tp_pte.pteTable[j].pte_entryHI = 
			uProcs[i-1].Tp_pte.pteTable[j].pte_entryLO = 
		}
		
		/* Init last entry in the table */
		uProcs[i-1].Tp_pte.pteTable[KUSEGPTESIZE-1].pte_entryHI = 
													
		/* location of the segment table */
		segTable = (segTable_t *) ? 
		
		/*Point to their page tables*/
		segTable->ksegOS = &kSegOS;
		segTable->kUseg2 = &(uProcs[i-1].Tp_pte);
		segTable->kUseg3 = &kUSeg3;
		
		/* Set up process's state */

		procState.s_CP15_EntryHi = 
		procState.s_sp = EXECTOP - ((i - 1) * something);
		procState.s_pc = /* set to our code */;									
		procState.s_cpsr = ;
										
		uProcs[i-1].Tp_sem = 0;
										
		/*Create the process */
		SYSCALL(CREATEPROCESS, , , )
	}

	/* more below ..?*/
										
	/*

	 go to sleep here somehow 

	 */
		
	/* P on the masterSem for each process created */
	for (i = 0; i < MAXUSERPROC; i++){
		
		SYSCALL(PASSEREN, (int)&masterSem, 0, 0);
	}
	
	/* Kill the creation process */
	SYSCALL(TERMINATEPROCESS, , , );
}

