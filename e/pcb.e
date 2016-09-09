#ifndef pcb
#define pcb

/*  
	pcb.e
	External functions for interacting with PBC queues 
	and children lists
*/

#include "../h/const.h"
#include "../h/types.h"

/* 
    Initialize the pcbFree list to contain all the elements
    if the static array or MAXPROC ProcBlk's. This method will
        be called only once during data structure initialization.
*/
extern void initPcbs();

/* 
    Return NULL if the pcbFree list is empty. Otherwise, remove
    an element from the pcbFree list, provide initial values for 
    all fields.
*/
extern pcb_t *allocPcb();

/*  
    Insert the element pointed to by p onto the pcbFree list.
*/
extern void freePcb(pcb_t *p);

/*  
    Initialize a variable to be a tail ptr to a proc queue
*/
extern pcb_PTR mkEmptyProcQ();

/*  
    Return TRUE if queue whose tail is pointed to by tp is empty. 
    Return FALSE otherwise
*/
extern int emptyProcQ(pcb_PTR tp);

/*  
    Insert ProcBlk pointed to by p into process queue whose tail-pointer
    is pointed to by tp.
*/
extern void insertProcQ(pcb_PTR *tp, pcb_PTR p);

/*  
    Remove the first (head) element from the process queue whose tail
    pointer is pointed to by tp. Return NULL if the queue is empty. 
    otherwise return pointer to the removed element. Update queue 
    as needed.
*/
extern pcb_PTR removeProcQ(pcb_PTR * tp);

/*  
    Remove ProcBlk pointed to by p from the process queue whose tail 
    pointer is pointed to by tp. Update teh process queue's tail if 
    necessary. If ProcBlk is not in the queue, return NULL; otherwise
    return p.
*/
extern pcb_PTR outProcQ( pcb_PTR *tp, pcb_PTR p);

/*  
    Return a pointer to the first ProcBlk from the process queue whose
    tail pointer is pointed to by tp. Does not remove the ProcBlk.
    Return NULL if the queue is empty.
*/
extern pcb_PTR headProcQ(pcb_PTR tp);

/*
	Make pcb pointed to p by a child of prnt
	inserts to the front of the child list
*/
extern void insertChild(pcb_PTR prnt, pcb_PTR p);

/*
	Make first child of p no longer a child of p.
	return NULL if p initially has no children
	otherwise return pointer to removed process
*/
extern pcb_PTR removeChild(pcb_PTR p);

/*
	Make pcb pointed to by p no longer a child.
	return NULL if p has no parent, otherwise return p
	p can be in any point of the child list 
*/
extern pcb_PTR outChild(pcb_PTR p);

#endif
