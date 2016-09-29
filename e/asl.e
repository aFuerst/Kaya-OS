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

/* 
    Search the ASL for a descriptor of this semaphore. If none is
    found, return NULL; otherwise, remove the first (head) ProcBlk from 
    the process queue of the found semaphore descriptor and return a
    pointer to it. If the process queue for this semaphore becomes empty
    remove the semaphore descriptor from the ASL and return it to the
    free list.
*/
extern pcb_PTR removeBlocked(int *semAdd);

/* 
    Remove the ProcBlk pointed to by p from the process queue associated
    with p's semaphore on the ASL. If ProcBlk pointed to by p does not
    appear in the process queue associated with p's semaphore, which is
    an error condition, return NULL; otherwise, return p
*/
extern pcb_PTR outBlocked(pcb_PTR p);

/*
	Returns a pointer to procBlok that is at th ehead of process queue
	with semaphore semAdd. return NULL if it doesn't exist or if the 
	process queue with semAdd is empty
*/
extern pcb_PTR headBlocked(int *semAdd);

/*
	Initialize the semdFree list to contain all the elements of the array
    static semd_t semdTable[MAXPROC] 
    This method will only be called once during data structure initialization
*/
extern void initASL();

#endif

