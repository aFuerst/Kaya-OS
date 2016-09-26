#ifndef asl
#define asl

#include "../h/const.h"
#include "../h/types.h"

/*
	insert the pcb pointed to be p at the tail of the process queue
	at semaphore semAdd and set it's semaphore address to semAdd.
	If a new semaphore needs to be allocated and the semdFree list 
	is empty, return TRUE. Otherwise return FALSE.
*/
extern int insertBlocked(int *semAdd, pcb_PTR p);

extern pcb_PTR removeBlocked(int *semAdd);

extern pcb_PTR outBlocked(pcb_PTR p);

/*
	Returns a pointer to procBlok that is at th ehead of process queue
	with semaphore semAdd. return NULL if it doesn't exist or if the 
	process queue with semAdd is empty
*/
extern pcb_PTR headBlocked(int *semAdd);

extern void initASL();

#endif

