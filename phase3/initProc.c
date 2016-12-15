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
Tproc_t uProcs[MAXUSERPROC];

int swapSem;
int mutexSemArray[49];
int masterSem;

unsigned int getASID();
HIDDEN void setUpProcess();
void diskIO(int sector, int cylinder, int head, int* semaphore, int disk, int pageLoc, int rw);

void test() {
	  
	/* Local Variables */
	int i;
	int j;
	state_t procState;
	int status;
	segTable_t* segTable;
	
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
		swapPool[i].segNo = 0;
		swapPool[i].pageNo = 0;
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
		uProcs[i-1].Tp_pte.header = (PGTBLMAGICNUM << 24) | KUSEGPTESIZE;
														   
		for(j = 0; j < KUSEGPTESIZE; ++j){
			
			uProcs[i-1].Tp_pte.pteTable[j].entryHI = 
					 ((0x80000 + j) << ENTRYHISHIFT) | (i << ASIDSHIFT);
			uProcs[i-1].Tp_pte.pteTable[j].entryLO = ALLOFF | DIRTYON;
		}
		
		/* Init last entry in the table */
		uProcs[i-1].Tp_pte.pteTable[KUSEGPTESIZE-1].entryHI = (0xBFFFF  << ENTRYHISHIFT) | (i << ASIDSHIFT);
		
		/* location of the segment table */
		segTable = (segTable_t *) (SEGSTART + (i * SEGWIDTH));

		/*Point to their page tables*/
		segTable->ksegOS = &kSegOS;
		segTable->kUseg2 = &(uProcs[i-1].Tp_pte);

		/* Set up process's state */
		procState.s_asid = (i << ASIDSHIFT);
		
		/* use processes tlb trap stack space for it's setup */
		procState.s_sp = (TAPEBUFFSTART - (((TRAPTYPES-1) * PAGESIZE * (i-1)) + (PAGESIZE * TLBTRAP)));

		/* set to our code */
		procState.s_t9 = procState.s_pc = (memaddr)setUpProcess;
		procState.s_status = ALLOFF | IEON | IMON | TEON;
		
		uProcs[i-1].Tp_sem = 0;

		/*Create the process */
		status = SYSCALL(CREATEPROCESS, (int)&procState, 0, 0);
		if(status != SUCCESS){
			/* process creation failed for unknown reason */
			SYSCALL(TERMINATEPROCESS,0,0,0);
		}
	}
	
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
	device_PTR tape, disk;
	unsigned int asid = getASID();
	i=0;
	/* this is the tape we will read data from */
	tape = (device_t*) (INTDEVREG + ((TAPEINT-DEVWOSEM) * DEVREGSIZE * DEVPERINT) + ((asid-1) * DEVREGSIZE));
	
	/* backing store disk */
	disk = (device_t*) (INTDEVREG + ((DISKINT-DEVWOSEM) * DEVREGSIZE * DEVPERINT) + (0 * DEVREGSIZE));
	
	/* memory buffer work tape to disk transfer */
	memBuffer = TAPEBUFFSTART + (PAGESIZE * (asid-1));
	
	/* loop until whole file has been read */
	while((tape -> d_data1 != EOF) && (tape -> d_data1 != EOT)) {
		tape -> d_data0 = memBuffer;
		/* issue read command */
		tape -> d_command = READBLK;
		/* wait for data read */
		status = SYSCALL(WAITIO, TAPEINT, asid-1, 0);
		
		if(status != READY){
			SYSCALL(TERMINATE,0,0,0);
		}
		
		diskIO(asid-1, i, 0, &disk0Sem, 0, memBuffer, WRITEBLK);
		
		/* keep track of what page we're on */
		++i;
	}
	
	/* each process gets 2 pages for SYS, PROG, TLB handling */
	for(i=0; i < TRAPTYPES; ++i) {
		/* get new state for processor */
		newState = &(uProcs[asid-1].Tnew_trap[i]);
		newState->s_asid = getENTRYHI();
		/* handlers run in kernel mode with VM on! */
		newState->s_status= ALLOFF | IEON | IMON | INTON  | TEON | VMON;
		switch (i) {
			case (TLBTRAP):
				(newState->s_t9) = (newState->s_pc) = (memaddr)pager;
				/* get memory page for sys5 stack page */
				newState->s_sp = (int)(TAPEBUFFSTART - (((TRAPTYPES-1) * PAGESIZE * (asid-1)) + (PAGESIZE * TLBTRAP)));
			break;
			
			case (SYSTRAP):
				(newState->s_t9) = (newState->s_pc) = (memaddr)userSyscallHandler;
				/* get memory page for sys5 stack page, systrap and program trap use same stack page */
				newState->s_sp = (int)(TAPEBUFFSTART - (((TRAPTYPES-1) * PAGESIZE * (asid-1)) + (PAGESIZE * PROGTRAP)));
			break;
			
			case(PROGTRAP):
				(newState->s_t9) = (newState->s_pc) = (memaddr)userPgmTrpHandler;
				/* get memory page for sys5 stack page */
				newState->s_sp = (int)(TAPEBUFFSTART - (((TRAPTYPES-1) * PAGESIZE * (asid-1)) + (PAGESIZE * PROGTRAP)));
			break;
		}
		/* make 3 sys5 requests */
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

void diskIO(int sector, int cylinder, int head, int* semaphore, int diskNum, int pageLoc, int rw) {
	unsigned int command, status, myStatus;
	device_PTR disk = (device_t*) (INTDEVREG + ((DISKINT-DEVWOSEM) * DEVREGSIZE * DEVPERINT) + (diskNum * DEVREGSIZE));
	status = command = ALLOFF;
	/* gain control of disk 0 */
	SYSCALL(PASSEREN, (int)semaphore, 0, 0);

	/*
	int maxCyl, maxSect, maxHead;
	maxCyl = ((disk -> d_data1) & MAXCYLAREA) >> 16;
	maxHead = ((disk -> d_data1) & MAXHEADAREA) >> 8;
	maxSect = ((disk -> d_data1) & MAXSECTAREA);
	*/
	/* seek cylinder where cylinder is our ASID - 1 */
	command = (cylinder << 8);
	command = command | SEEKCYL;

	myStatus = getSTATUS();
	setSTATUS(ALLOFF);
	
	disk -> d_command = command;
	/* wait for IO to complete on disk 0 */
	status = SYSCALL(WAITIO, DISKINT, diskNum,0);
	
	/* turn interruprts back on */
	setSTATUS(myStatus);
	
	/* TODO: Check status! */
	if(status != READY){
		SYSCALL(TERMINATE,0,0,0);
	}
	
	/* write on surface 0 and sector i */
	disk -> d_data0 = pageLoc;
	command = head;
	command = (command << 8) | sector;
	command = (command << 8) | rw;
	
	/* turn off interrupts */
	myStatus = getSTATUS();
	setSTATUS(ALLOFF);
	
	disk -> d_command = command;
	/* wait for IO to complete on disk 0 */
	status = SYSCALL(WAITIO, DISKINT, diskNum,0);
	
	/* turn interruprts back on */
	setSTATUS(myStatus);
	
	/* TODO: Check status! */
	if(status != READY){
		SYSCALL(TERMINATE,0,0,0);
	}
	
	/* release disk 0 for now */
	SYSCALL(VERHOGEN,  (int)semaphore, 0, 0);
	
	return;
}

state_PTR getCaller(unsigned int ASID, int trapType) {
	return (&((uProcs[ASID-1]).Told_trap[trapType]));
}

