#include "../h/const.h"
#include "../h/types.h"
#include "/usr/local/include/umps2/umps/libumps.e"
#include "../e/initProc.e"

/* variables taken from initProc */
extern int disk0Sem, disk1Sem;
extern Tproc_t uProcStates[MAXUSERPROC];

/* function from UMPS */
extern unsigned int getENRTYHI();

/***********************************************************************
 * Implements VM I/O support level SYS?Bp and PgmTrp exception handlers
 * 
 * 
***********************************************************************/

/* Modual local functions */ 
HIDDEN void syscall9();
HIDDEN void syscall10();
HIDDEN void syscall11();
HIDDEN void syscall12();
HIDDEN void syscall13();
HIDDEN void syscall14();
HIDDEN void syscall15();
HIDDEN void syscall16();
HIDDEN void syscall17();
HIDDEN void syscall18();
HIDDEN unsigned int getASID();
HIDDEN state_PTR getCaller(unsigned int ASID, int trapType);

void userSyscallHandler() {
	/* determine which uProc is requestor */
	state_PTR caller;
	int sysReq;
	unsigned int asid = getASID();/* get process ID */
	caller = getCaller(asid, SYSTRAP);
	
	/*&((uProcStates[SYSTRAP]).Told_trap[asid]);
	/* get correct sysOld area based on requesting uProc */
	/* get requested service out of oldSys -> s_a0 */
	sysReq = caller -> s_a0;
	switch(sysReq) {
		
		case(READ_FROM_TERMINAL):
			syscall9();
		break;
		case(WRITE_TO_TERMINAL):
			syscall10();
		break;
		case(V_VIRTUAL_SEMAPHORE):
			syscall11();
		break;
		case(P_VIRTUAL_SEMAPHORE):
			syscall12();
		break;
		case(DELAY):
			syscall13();
		break;
		case(DISK_PUT):
			syscall14();
		break;
		case(DISK_GET):
			syscall15();
		break;
		case(WRITE_TO_PRINTER):
			syscall16();
		break;
		case(GET_TOD):
			syscall17();
		break;
		case(TERMINATE):
			syscall18();
		break; 
		default: 
			/* user made unsupported syscall, terminate */
			syscall18();
		break;
	}
	/* should never get here, terminate requestor */
	syscall18();
}

/*
 * Read from Terminal
 * 
 * Causes the uProc to be suspended until a line of output has been
 * read from the terminal device assiciated with that uProc
 * 
 * Virtual address of a string buffer where the data read should be
 * placed is in a1
 * 
 * Once the process resumes the number of successful characters actually
 * transmitted is returned in v0. If operation ends with status other
 * than received, the negative of the device's status is returned in v0
 * 
 * If the buffer is in ksegOS, it will be treated as a sys18
 * */
HIDDEN void syscall9(){
	state_PTR caller;
	int i;
	char recvdVal;
	unsigned int status;
	char* string;
	device_t* terminal; /* terminal device register location */

	/* determine which uProc is requestor */	
	unsigned int asid =  getASID();/* get process ID */
	
	caller = getCaller(asid, SYSTRAP);
	string = (char *) caller -> s_a1; /* get string buffer from caller */
	
	/* get device register for specific terminal */
	terminal = (device_t *) (INTDEVREG + ((TERMINT-DEVWOSEM)
				* DEVREGSIZE * DEVPERINT) + ((asid-1) * DEVREGSIZE));
	
	if(string <= (char*)KSEGOSEND) {
		syscall18();
	}
	i=0;
	do {
		terminal -> t_recv_command = RECVCHR;
		/* make call to waitIO after issuing IO request*/
		status = SYSCALL(WAITIO, TERMINT, asid-1, TRUE);
		if((status & 0xFF) != CHRRECVD) {
			/* error in printing */
			/* negate i for placing in callers v0 */
			i = i * -1;
			/* exit print loop */
			break;
		}
		recvdVal = status & 0xFF00;
		string[i] = (recvdVal >> 8);
		++i;
	} while(recvdVal != '\0'); /* loop until EOF */
	
	/* place i in caller's v0 */
	caller -> s_v0 = i;
	/* ldst on caller */
	LDST(caller);
}

/*
 * 	Write to Terminal
 * Causes the uProc to be suspended until a line of output has been
 * transmitted to terminal device assiciated with that uProc
 * 
 * Virtual address of first character in a1
 * Length of string in a2
 * 
 * If a1 is > 128 or < 0, treat as sys18
 * 
 *	If all characters transmit successfully, the number of characters
 * transmitted is put in v0, otherwise the negative of the device's 
 * status value is returned in v0
 * 
 */
HIDDEN void syscall10(){
	state_PTR caller;
	int i,length;
	unsigned int status, command;
	char* string;
	device_t* terminal; /* terminal device register location */
	
	/* determine which uProc is requestor */
	unsigned int asid =  getASID();/* get process ID */
	
	caller = getCaller(asid, SYSTRAP);
	string = (char *) caller -> s_a1;
	length = (int)caller -> s_a2;
	
	/* get device register for specific terminal */
	terminal = (device_t *) (INTDEVREG + ((TERMINT-DEVWOSEM)
					* DEVREGSIZE * DEVPERINT) + ((asid-1) * DEVREGSIZE));
	
	if(length < 0 || length > 128) {
		syscall18();
	}
	if(string <= (char*)KSEGOSEND) {
		syscall18();
	}
	for(i=0; i < length; ++i) {
		command = string[i];
		command = command << 8;
		command = command | TRANSMITCHR;
		terminal -> t_transm_command = command;
		/* make call to waitIO after issuing IO request*/
		status = SYSCALL(WAITIO, TERMINT, asid-1, FALSE);
		if((status & 0xFF) != CHRTRMTD) {
			/* error in printing */
			/* negate i for placing in callers v0 */
			i = i * -1;
			/* exit print loop */
			break;
		}
	}
	/* place i in caller's v0 */
	caller -> s_v0 = i;
	/* ldst on caller */
	LDST(caller);
}

/*
 * V_Virtual_Sem
 * 
 * Currently not implemented, calls sys18
 * 
 */
HIDDEN void syscall11() {
	syscall18();
}

/*
 * P_Virtual_Sem
 * 
 * Currently not implemented, calls sys18
 */
HIDDEN void syscall12() {
	syscall18();
}

/*
 * Delay
 * 
 * Currently not implemented, calls sys18
 */
HIDDEN void syscall13() {
	syscall18();
}

/*
 * Disk_Put
 * 
 * Causes requesting uProc to be suspended until operation has been 
 * completed. 
 * 
 * Virtual address of 4k block to be written is in a1
 * Disk number to write to in a2
 * Disk Sector to write to in a3
 * 
 * On completion, put device completion status in v0 
 * If ends with status other than "device ready" the negative of 
 * completion status is returned in v0
 */
HIDDEN void syscall14() {
	state_PTR caller;
	device_t* disk;
	int maxCyl, maxSect, maxHead, sectNum, cylinder, surface, track;
	unsigned int command, status;
	
	/* determine which uProc is requestor */	
	unsigned int asid =  getASID();/* get process ID */
	
	caller = getCaller(asid, SYSTRAP);
	/* check if a1 != 0, if so terminate */
	if(caller -> s_a0 == 0){
		syscall18();
	}
	
	/* get proper device register */
	
	/* get device register for specific terminal */
	disk = (device_t *) (INTDEVREG + ((DISKINT-DEVWOSEM)
					* DEVREGSIZE * DEVPERINT) + (1 * DEVREGSIZE));
	
	/* get maxcyl, maxhead, maxsect from device register */
	maxCyl = ((disk -> d_data1) & MAXCYLAREA) >> 16;
	maxHead = ((disk -> d_data1) & MAXHEADAREA) >> 8;
	maxSect = ((disk -> d_data1) & MAXSECTAREA);
	
	/* compute disk cylinder, surface and track */
	/* track */
	track = sectNum % maxSect;
	sectNum = sectNum/maxSect;		
	/* surface */
	surface = sectNum % maxHead;
	sectNum=sectNum/maxHead;
	/* cylinder */
	cylinder = sectNum % maxCyl;

	/* do disk seek based on this info */
	command = cylinder << 8;
	command = command | SEEKCYL;
	
	SYSCALL(PASSEREN,(int)(&disk1Sem),0,0);
	
	disk->d_command = command;
	
	status = SYSCALL(WAITIO, DISKINT, asid-1,0);
	
	command = surface;
	command = (command << 8) | track;
	command = (command << 8) | WRITEBLK;
	
	/* perform actual write */
	disk->d_data0 = (int)caller -> s_a1;
	disk->d_command = command;
	status = SYSCALL(WAITIO, DISKINT, asid-1, 0);
	
	SYSCALL(VERHOGEN,(int)(&disk1Sem),0,0);
	
	/* move data somewhere? */
	/* ldst to caller */
	LDST(caller);
}

/*
 * Disk_Get
 * 
 * Causes requesting uProc to be suspended until operation has been 
 * completed. 
 * 
 * Virtual address of 4k block to be read is in a1
 * Disk number to read to in a2
 * Disk Sector to read to in a3
 * 
 * On completion, put device completion status in v0 
 * If ends with status other than "device ready" the negative of 
 * completion status is returned in v0
 */
HIDDEN void syscall15() {
	state_PTR caller;
	device_t* disk;
	int sectNum, cylinder, surface, track, maxCyl, maxSect, maxHead;
	unsigned int command, status;
	
	/* determine which uProc is requestor */	
	unsigned int asid =  getASID();/* get process ID */
	
	caller = getCaller(asid, SYSTRAP);
	
	/* check if a1 != 0, if so terminate */
	if(caller -> s_a1 == 0) {
		syscall18();
	}
	
	/* get device register for specific terminal */
	disk = (device_t *) (INTDEVREG + ((DISKINT-DEVWOSEM)
				* DEVREGSIZE * DEVPERINT) + ((asid-1) * DEVREGSIZE));
	
	/* get maxcyl, maxhead, maxsect from device register */
	maxCyl = ((disk -> d_data1) & MAXCYLAREA) >> 16;
	maxHead = ((disk -> d_data1) & MAXHEADAREA) >> 8;
	maxSect = ((disk -> d_data1) & MAXSECTAREA);
	
	/* compute disk cylinder, surface and track */
	/* track */
	track = sectNum % maxSect;
	sectNum = sectNum/maxSect;		
	/* surface */
	surface = sectNum % maxHead;
	sectNum=sectNum/maxHead;
	/* cylinder */
	cylinder = sectNum % maxCyl;
		
	/* do disk seek based on this info */
	command = cylinder << 8;
	command = command | SEEKCYL;
	
	SYSCALL(PASSEREN,(int)(&disk1Sem),0,0);
	
	disk->d_command = command;
	
	status = SYSCALL(WAITIO, DISKINT, asid-1,0); 
	
	command = surface;
	command = (command << 8) | track;
	command = (command << 8) | READBLK;
	
	/* perform actual write */
	disk->d_data0 = (int)caller -> s_a1;
	disk->d_command = command;
	status = SYSCALL(WAITIO, DISKINT, asid-1,0);

	SYSCALL(VERHOGEN,(int)(&disk1Sem),0,0);
	/* move data somewhere? */
	/* other paging stuff? */
	/* ldst to caller cc*/
	LDST(caller);
}

/*
 * Write to Printer
 * 
 * Causes requesting uProc to be suspended until a line of output has 
 * been transmitted to the printer device associated with the uProc. 
 * 
 * Virtual address of first character is in a1
 * Length of string is in a2
 * 
 * If address is in kSegOS, length is < 0 or length > 128, 
 * 	treat as sys18
 * 
 * On completion, number of transmitted characters is in v0 
 * If ends with status other than "device ready" the negative of 
 * completion status is returned in v0
 */
HIDDEN void syscall16() {
	int i,length;
	unsigned int status;
	char* string;
	state_PTR caller;
	device_t* printer; /* printer device register location */
	
	/* determine which uProc is requestor */	
	unsigned int asid = getASID();/* get process ID */
	caller = getCaller(asid, SYSTRAP);
	
	/* get device register for specific terminal */
	printer = (device_t *) (INTDEVREG + ((PRNTINT-DEVWOSEM)
				* DEVREGSIZE * DEVPERINT) + ((asid-1) * DEVREGSIZE));

	if(length < 0 || length > 128) {
		syscall18();
	}
	if(string <= (char*)KSEGOSEND) {
		syscall18();
	}
	for(i=0; i < length; ++i) {
		printer -> d_data0 = string[i];
		printer -> d_command = PRINTCHR;
		/* make call to waitIO after issuing IO request*/
		status = SYSCALL(WAITIO, PRNTINT, asid-1,0);
		if(status != READY) {
			/* error in printing */
			/* negate i for placing in callers v0 */
			i = i * -1;
			/* exit print loop */
			break;
		}
	}
	/* place i in caller's v0 */
	caller -> s_v0 = i;
	/* ldst on caller */
	LDST(caller);
}

/*
 * Get ToD
 * 
 * Places the number of microseconds since the system was last rebooted
 * to be placed in uProc's v0 
 */
HIDDEN void syscall17() {
	state_PTR caller;
	cpu_t time;
	/* determine which uProc is requestor */	
	unsigned int asid =  getASID();/* get process ID */
	caller = getCaller(asid, SYSTRAP);
	STCK(time);
	/* store time in caller's v0, and ldst to caller */
	caller -> s_v0 = time;
	LDST(caller);
}

/*
 * Terminate
 * 
 * Cuases the executing uProc to cease to exist
 * 
 * 
 */
HIDDEN void syscall18() {
	state_PTR caller;
	/* determine which uProc is requestor */	
	unsigned int asid =  getASID();/* get process ID */
	
	caller = getCaller(asid, SYSTRAP);
}


HIDDEN unsigned int getASID() {
	unsigned int asid = getENRTYHI();
	asid = (asid & 0x00000FC0) >> 6;/* get process ID */
	return asid;
}

HIDDEN state_PTR getCaller(unsigned int ASID, int trapType){
	return (&((uProcStates[asid-1]).Told_trap[trapType]));
)

/***********************************************************************
 * 	User Level Program Trap Exception Handler
 * 
***********************************************************************/

void userPgmTrpHandler() {
	syscall18();
}

