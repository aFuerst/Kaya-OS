#ifndef CONSTS
#define CONSTS

#include "../h/types.h"

/*********************************************************************** 
 *
 * This header file contains utility constants & macro definitions.
 * 
 **********************************************************************/

/* Hardware & software constants */
#define PAGESIZE		4096	/* page size in bytes */
#define WORDLEN			4		/* word size in bytes */
#define PTEMAGICNO		0x2A
#define MAXPROC			20		/* maximum number of processes */

#define ROMPAGESTART	0x20000000	 	/* ROM Reserved Page */
#define QUANTUM 		5000			/* CPU burst time */
#define INTTIME			1000000 		/* interval timer period */
#define MAGICNUM		49		/* num of devices w/sem + 1 for timer*/

/* timer, timescale, TOD-LO and other bus regs */
#define RAMBASEADDR		0x10000000
#define TODLOADDR		0x1000001C
#define INTERVALTMR		0x10000020	
#define TIMESCALEADDR	0x10000024

/* new processor state locations */
#define SYSCALLNEWAREA 0x200003D4
#define PBGTRAPNEWAREA 0x200002BC
#define TBLMGMTNEWAREA 0x200001A4
#define INTPNEWAREA    0x2000008C

/* old processor state locations */
#define SYSCALLOLDAREA 0x20000348
#define PBGTRAPOLDAREA 0x20000230
#define TBLMGMTOLDAREA 0x20000018
#define INTPOLDAREA    0x20000000

#define IPAREA			0x0000FF00

/* On/Off bit manipulations */
#define IEON			0x00000004 /* OR to turn on interrupts */
#define IECON			0x00000001 /* OR to turn interrupt current on */
#define KUON			0x00000008 /* OR to set in user mode */
#define VMON			0x02000000 /* OR to turn on virtual memory */
#define INTON			0x08000000 /* OR to turn on interval timer */
#define IMON			0x0000FF00 /* OR to turn on Interrupt Mask */
#define IEOFF			0xFFFFFFFB /* AND to turn off interrupts */
#define KUOFF			0xFFFFFFF7 /* AND to turn on kernel mode */
#define VMOFF			0xFDFFFFFF /* AND to turn off virtual memory */
#define INTOFF			0xF7FFFFFF /* AND to turn off interval timer */
#define IMOFF			0xFFFF00FF /* AND to turn off Interrupt Mask */
#define ALLOFF 			0x00000000

/* utility constants */
#define	TRUE			1
#define	FALSE			0
#define ON              1
#define OFF             0
#define HIDDEN			static
#define EOS				'\0'
#define SUCCESS			0
#define FAILURE			-1

#ifndef MAX_INT
#define MAX_INT 		0xEFFFFFFF
#endif

#define NULL 	((void *)0xFFFFFFFF)


/* vectors number and type */
#define VECTSNUM		4

#define TLBTRAP			0
#define PROGTRAP		1
#define SYSTRAP			2

#define TRAPTYPES		3


/* device interrupts */
#define DISKINT			3
#define TAPEINT 		4
#define NETWINT 		5
#define PRNTINT 		6
#define TERMINT			7

#define DEVREGLEN		4	/* device register field length in bytes & regs per dev */
#define DEVREGSIZE		16 	/* device register size in bytes */
#define DEVWOSEM		3 	/* first 3 devices do not have 8 semaphores */

/* device register field number for non-terminal devices */
#define STATUS			0
#define COMMAND			1
#define DATA0			2
#define DATA1			3

/* device register field number for terminal devices */
#define RECVSTATUS      0
#define RECVCOMMAND     1
#define TRANSTATUS      2
#define TRANCOMMAND     3

/* device & line number bits on */
#define FIRST			0x1
#define SECOND			0x2
#define THIRD			0x4
#define FOURTH			0x8
#define FIFTH			0x10
#define SIXTH			0x20
#define SEVENTH			0x40
#define EIGHTH			0x80

/* start of interrupt device bitmap and registers */
#define INTBITMAP		0x1000003C
#define INTDEVREG		0x10000050

/* device common STATUS codes */
#define UNINSTALLED		0
#define READY			1
#define BUSY			3

/* device common COMMAND codes */
#define RESET			0
#define ACK				1

/* operations */
#define	MIN(A,B)	((A) < (B) ? A : B)
#define MAX(A,B)	((A) < (B) ? B : A)
#define	ALIGNED(A)	(((unsigned)A & 0x3) == 0)

/* Useful operations */
#define STCK(T) ((T) = ((* ((cpu_t *) TODLOADDR)) / (* ((cpu_t *) TIMESCALEADDR))))
#define LDIT(T)	((* ((cpu_t *) INTERVALTMR)) = (T) * (* ((cpu_t *) TIMESCALEADDR))) 

/* SYSCALL values */
#define CREATEPROCESS 	 1
#define TERMINATEPROCESS 2
#define VERHOGEN 		 3
#define PASSEREN 		 4
#define SPECTRAPVEC 	 5
#define GETCPUTIME 		 6
#define WAITCLOCK 		 7
#define WAITIO 			 8

#endif
