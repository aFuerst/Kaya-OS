
#include "../h/const.h"
#include "../h/types.h"
#include "../e/initProc.e"
#include "../e/sysSupport.e"

#include "/usr/local/include/umps2/umps/libumps.e"

/*
 * Implements VM- I/O level TLB exception handler
 */
HIDDEN int getFrame();

extern swapPool_t swapPool[SWAPSIZE];
extern Tproc_t uProcs[MAXUSERPROC];
extern int nextSwap; /* which page in swap pool will be swapped next */
extern void diskIO(int sector, int cylinder, int head, int* semaphore, int disk, int pageLoc, int rw);

void debugPager(){
	int i;
	i=0;
}
void pager(){
	state_PTR causingProc;
	unsigned int frameNum, pageNum, removingProcID;
	unsigned int missingPage, pageIndex, segment;
	unsigned int pageLocation, status, cause;
	devregarea_t* deviceInfo = (devregarea_t *) RAMBASEADDR;
	unsigned int currAsid = getASID();
	causingProc = getCaller(currAsid, TLBTRAP);
	
	/* gain control of swap semaphore */
	SYSCALL(PASSEREN, (int) &swapSem, 0, 0);
	
	/* calcualte RAMTOP */
	pageLocation = (deviceInfo -> rambase) + (deviceInfo -> ramsize);
	pageLocation = pageLocation - (PAGESIZE * 3);/* start of first of user frames */
	cause = ((causingProc->s_cause) & 0xFC) >> 2;
	
	/* get offending segment number and page number */
	segment = causingProc->s_asid >> 30;
	pageIndex = missingPage = (causingProc->s_asid & 0x3ffff000) >> 12;	
	
	/* get cause, if not tlb invalid: kill */
	if(cause != TLBL && cause != TLBS)
	{
		/* kill causingProc */
		SYSCALL(TERMINATEPROCESS, 0, 0, 0);
	}
	
	frameNum = getFrame();
	pageLocation = pageLocation - (frameNum * PAGESIZE);
	/* was the stack page missing? */
	if(missingPage >= KUSEGPTESIZE) {
		pageIndex = KUSEGPTESIZE -1;
	}
	
	/* if occupied */
	if(swapPool[frameNum].asid != -1) {
		
		/* turn off interrupts here */
		status = getSTATUS();
		setSTATUS(ALLOFF);
		
		removingProcID = swapPool[frameNum].asid;
		pageNum = swapPool[frameNum].pageNo;
		if(pageNum >= KUSEGPTESIZE) {
			pageNum = KUSEGPTESIZE -1;
		}

		/* turn off valid bit */
		/* invalidate that processes pte */
		swapPool[frameNum].pte -> entryLO = ALLOFF | DIRTYON;
		swapPool[frameNum].asid = -1;
		swapPool[frameNum].pageNo = 0;
		swapPool[frameNum].segNo = 0;
		swapPool[frameNum].pte = NULL;	
		TLBCLR();
		
		/* turn interruprts back on */
		setSTATUS(status);
		
		diskIO(removingProcID-1, pageNum, 0, &disk0Sem, 0, pageLocation, WRITEBLK);
		
	}
	
	debugPager(currAsid, pageIndex, pageLocation,frameNum);
	
	diskIO(currAsid-1, pageIndex, 0, &disk0Sem, 0, pageLocation, READBLK);

	/** Change proc page table and dswap pool **/
	swapPool[frameNum].asid = currAsid;
	swapPool[frameNum].pageNo = missingPage;
	swapPool[frameNum].segNo = segment;
	swapPool[frameNum].pte = &(uProcs[currAsid-1].Tp_pte.pteTable[pageIndex]);
	swapPool[frameNum].pte -> entryLO = (pageLocation & 0xFFFFF000) | VALIDON | DIRTYON;
	/*
	uProcs[currAsid-1].Tp_pte.pteTable[missingPage].entryLO = (pageLocation & 0xFFFFF000) | VALIDON | DIRTYON;
	*/
	TLBCLR();
	
	/* return control of swap pool */
	SYSCALL(VERHOGEN, (int) &swapSem, 0, 0);
	
	/* ldst to user proc again */
	LDST(causingProc);
}

HIDDEN int getFrame() {
	static int frame = 0;
	
	frame = (frame + 1); /* % SWAPSIZE;*/
	if(frame >= SWAPSIZE){
		frame = 0;
	}
	return(frame);
}

