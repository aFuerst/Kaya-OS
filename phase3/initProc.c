#include "../h/const.h"
#include "../h/types.h"
#include "../e/pager.e"
#include "../e/sysSupport.e"

#include "/usr/local/include/umps2/umps/libumps.e"

/*
 * Implements test and all U-Proc initialization routines
 * Exports VM-IO support level global variables 
 * (swap-pool, mutual exclusion semaphores)
 * 
 * 
 */

HIDDEN void debugInit(){
	int i;
	i=0;
}

/* included functions */

extern void userPgmTrpHandler();
extern void pager();
extern void userSyscallHandler();

pteOS_t kSegOS; /* OS page table */
pte_t kUSeg3;	/* kuSeg3 page table */
swapPool_t swapPool[SWAPSIZE];
int nextSwap; /* which page in swap pool will be swapped next */
int disk0Sem, disk1Sem, swapSem, masterSem; /* global semaphores */
Tproc_t uProcStates[MAXUSERPROC];
Tproc_t uProcs[MAXUSERPROC];

int swapSem;
int mutexSemArray[49];
int masterSem;

unsigned int getASID();
HIDDEN void setUpProcess();

void test() {
	  
	/* Local Variables */
	int i;
	int j;
	state_t procState;
	int status;
	segTable_t* segTable;
	state_PTR excptState;
	Tproc_PTR myState;
	int exptPageLocations;
	
	nextSwap = 0;
	
	/* Init semaphore to 1 for mutual exclusion */
	swapSem = disk0Sem = disk1Sem = 1;
	
	/* Init master to 0 for */
	masterSem = 0;
	
	/* page table */
	kSegOS.header = (PGTBLMAGICNUM << 24) | KSEGOSPTESIZE;
	for (i = 0; i < KSEGOSPTESIZE; i++){
		/* set up OS pages */
		kSegOS.pteTable[i].entryHI = (0x20000 + i) << 12;
		kSegOS.pteTable[i].entryLO = ((0x20000 + i) << 12)
										| DIRTYON | VALIDON | GLOBALON;
	}
	
	/* swap pool */
	for (i = 0; i < SWAPSIZE; i++){				
		swapPool[i].asid = -1;
		swapPool[i].pte = NULL;
	}
	
	/*Init swap semaphore to 1 for mutual exclusion*/
	swapSem = 1;
	
	/* Init the array of semaphores to 1 */
	for (i = 0; i < 49; ++i){
		mutexSemArray[i] = 1;
	}
	
	/* master sem is 0 for synchronization*/
	masterSem = 0;
		
	/* Init each process */
	for (i = 1; i < MAXUSERPROC + 1; ++i){
		/* these items need editing */
		uProcs[i-1].Tp_pte.header = 1;
														   
		for(j = 0; j < KUSEGPTESIZE; ++j){
			
			uProcs[i-1].Tp_pte.pteTable[j].entryHI = 
					 ((0x80000 + j) << ENTRYHISHIFT) | (i << ASIDSHIFT);
			uProcs[i-1].Tp_pte.pteTable[j].entryLO = ALLOFF | DIRTYON;
		}
		
		/* Init last entry in the table */
		uProcs[i-1].Tp_pte.pteTable[KUSEGPTESIZE-1].entryHI = (0xBFFFF  << ENTRYHISHIFT) | (i << ASIDSHIFT);
		
		/* location of the segment table */
		segTable = (segTable_t *) (SEGSTART + (i * SEGWIDTH));
		debugInit(0xffffffff,0xffffffff,0xffffffff,0xffffffff);
		/*Point to their page tables*/
		segTable->ksegOS = &kSegOS;
		segTable->kUseg2 = &(uProcs[i-1].Tp_pte);
		debugInit(0xaaaaaaaa,0xaaaaaaaa,0xaaaaaaaa,0xaaaaaaaa);
		/* Set up process's state */

		procState.s_asid = (i << ASIDSHIFT);
		
		/* use processes tlb trap stack space for it's setup */
		procState.s_sp = (TAPEBUFFSTART - ((2 * PAGESIZE * (i-1)) + (PAGESIZE * TLBTRAP)));
		debugInit(0xbbbbbbbb,TAPEBUFFSTART,procState.s_sp,0xaaaaaaaa);
		/* set to our code */
		procState.s_pc = (memaddr)setUpProcess;
		procState.s_t9 = (memaddr)setUpProcess;										
		procState.s_status = ALLOFF | IEON | IMON | TEON;
		
		uProcs[i-1].Tp_sem = 0;
		
		uProcs[i-1].Tp_sem = 0;
		
		/*Create the process */
		SYSCALL(CREATEPROCESS, &procState, 0, 0);
	}				
							
	/*
	 go to sleep here somehow 
	 */
	
	/* P on the masterSem for each process created */
	for (i = 0; i < MAXUSERPROC; i++){
		SYSCALL(PASSEREN, (int)&masterSem, 0, 0);
	}
	
	/* Kill the creation process */
	SYSCALL(TERMINATEPROCESS, 0, 0, 0);
}

/***********************************************************************
 * 
 * setUpProcess
 * 
 * Loads a user process from the tape drive associated with that user 
 * process. Copies that data to backing store on disk 0 and then 
 * launches that user process. 
 * 
 **********************************************************************/
HIDDEN void setUpProcess() {
	state_t procState;
	state_PTR newState;
	int i, memBuffer, status;
	unsigned int command;
	device_PTR tape, disk;
	unsigned int asid = getASID();
	i=0;
	/* this is the tape we will read data from */
	tape = (device_t*) (INTDEVREG + ((TAPEINT-DEVWOSEM) * DEVREGSIZE * DEVPERINT) + ((asid-1) * DEVREGSIZE));
	
	/* backing store disk */
	disk = (device_t*) (INTDEVREG + ((DISKINT-DEVWOSEM) * DEVREGSIZE * DEVPERINT) + (0 * DEVREGSIZE));
	
	/* memory buffer work tape to disk transfer */
	memBuffer = TAPEBUFFSTART + (PAGESIZE * (asid-1));
	debugInit(0,0,0,0);
	/* loop until whole file has been read */
	while((tape -> d_data1 != EOF) && (tape -> d_data1 != EOT)) {
		tape -> d_data0 = memBuffer;
		/* issue read command */
		tape -> d_command = READBLK;
		/* wait for data read */
		status = SYSCALL(WAITIO, TAPEINT, asid-1, 0);
		
		/* TODO: Check status! */
		
		/* gain control of disk 0 */
		SYSCALL(PASSEREN, (int)&disk0Sem, 0, 0);
		
		/* seek cylinder where cylinder is our ASID - 1*/
		command = (asid-1) << 8;
		command = command | SEEKCYL;
		disk -> d_command = command;
		/* wait for IO to complete on disk 0 */
		status = SYSCALL(WAITIO, DISKINT, 0,0);
		
		/* TODO: Check status! */
				
		/* write on surface 0 and sector i */
		command = 0;
		command = (command << 8) | i;
		command = (command << 8) | WRITEBLK;
		disk -> d_command = command;
		
		/* wait for IO to complete on disk 0 */
		status = SYSCALL(WAITIO, DISKINT, 0 ,0);
		
		/* TODO: Check status! */
		
		/* release disk 0 for now */
		SYSCALL(VERHOGEN,  (int)&disk0Sem, 0, 0);
		
		/* keep track of what page we're on */
		++i;
	}
	debugInit(i,i,0xaaaa,0xaaaa);
	/* each process gets 2 pages for SYS, PROG, TLB handling */
	for(i=0; i < TRAPTYPES; ++i) {
		 /* get new state for processor */
		newState = &(uProcs[asid-1].Tnew_trap[i]);
		newState->s_asid = getENTRYHI();
		/* handlers run in kernel mode with VM on! */
		newState->s_status= ALLOFF | IEON | IMON | INTON  | TEON | VMON;
		switch (i) {
			case (TLBTRAP):
				newState -> s_t9 = newState->s_pc = (memaddr)pager;
				/* get memory page for sys5 stack page */
				newState->s_sp = (int)(TAPEBUFFSTART - (((TRAPTYPES-1) * PAGESIZE * (asid-1)) + (PAGESIZE * TLBTRAP)));
			break;
			
			case (SYSTRAP):
				newState -> s_t9 = newState->s_pc = (memaddr)userSyscallHandler;
				/* get memory page for sys5 stack page, systrap and program trap use same stack page */
				newState->s_sp = (int)(TAPEBUFFSTART - (((TRAPTYPES-1) * PAGESIZE * (asid-1)) + (PAGESIZE * PROGTRAP)));
			break;
			
			case(PROGTRAP):
				newState -> s_t9 = newState->s_pc = (memaddr)userPgmTrpHandler;
				/* get memory page for sys5 stack page */
				newState->s_sp = (int)(TAPEBUFFSTART - (((TRAPTYPES-1) * PAGESIZE * (asid-1)) + (PAGESIZE * PROGTRAP)));
			break;
		}
		/* make 3 sys5 requests */
		debugInit(SPECTRAPVEC, i, (int)&(uProcs[asid-1].Told_trap[i]), (int)newState);
		SYSCALL(SPECTRAPVEC, i, (int)&(uProcs[asid-1].Told_trap[i]), (int)newState);
	}
	
	/* user code launch state */
	procState.s_t9 = procState.s_pc = (memaddr)WELLKNOWNPCSTART;
	procState.s_sp = WELLKNOWNSTACKSTART;
	procState.s_asid = getENTRYHI();
	procState.s_status = 
			ALLOFF | IEON | IMON | INTON | KUON | TEON | VMON;
	LDST(&procState);
}

/******************* Local functions that are exported ****************/

/*
 * Returns the ASID of the currently running process
 * 
 */
unsigned int getASID() {
	unsigned int asid = getENTRYHI();
	asid = (asid & 0x00000FC0) >> 6;/* get process ID */
	return asid;
}
