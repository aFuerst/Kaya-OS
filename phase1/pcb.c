#include "../h/types.h"
#include "../h/const.h"

/*
 * pcb.c
 * 
 * Contains functions for:
 * 	 Process Free List
 * 	   Allocate and free PCBs
 * 	 Process Block Queues Manager
 *     Make an empty Process queue
 *     Check if a Process queue is empty
 *     Insert PCBs to a Process queue
 *     See the next PCB in a Process queue
 * 	   Remove the first PCB from a Process queue
 *     Remove a specific PCB from it's Process queue
 *   Process Children Tree
 *     Insert children into a PCB
 *     Remove the first child from a PCB
 *     Remove a specific PCB from it's parent
 * 
 * Authors: Alex Fuerst, Aaron Pitman
 * The Structure of the "THE" - Multiprogramming System
 * */
 
 /*
  * Pcb Free list is kept as a singly linked linear stack with the
  *  head as pcbList_h
  * 
  * Process Queue's are doubly linked circular queue with a tail pointer
  * 	the tails next is the queue's head
  * 
  * Child Tree is a double linked linear stack with a head pointer
  * 
  * */

/* head of the pcb free list */
HIDDEN pcb_PTR pcbList_h;

/************************ PCB FREE LIST DEFINITIONS *******************/
/*  
    Insert the element pointed to by p onto the pcbFree list.
*/
void freePcb(pcb_PTR p){
	p -> p_next = pcbList_h;
    pcbList_h = p;
}

/* 
    Return NULL if the pcbFree list is empty. Otherwise, remove
    an element from the pcbFree list, provide initial values for 
    all fields.
*/
pcb_t *allocPcb(){
	pcb_PTR temp;
    if(pcbList_h == NULL){
        return NULL;
    }
    temp = pcbList_h;
    pcbList_h = pcbList_h -> p_next;
    /* set queue values to NULL */
    temp -> p_next = NULL;
    temp -> p_prev = NULL;
        
    /* children tree values to NULL */
    temp -> p_prnt = NULL;
    temp -> p_child = NULL;
    temp -> p_sibNext = NULL;
    temp -> p_sibPrev = NULL;

	/* semaphore values to NULL */
    temp -> p_semAdd = NULL; /* not sure what proper state is */
    
    temp -> cpu_time = 0; /* no CPU time */
    /* sys5 exception vectors set to NULL */
    temp -> pgmTrpNew = temp -> pgmTrpOld = temp -> tlbNew = temp -> 
			tlbOld = temp -> sysNew = temp -> sysOld = NULL;
    return temp;
}

/* 
    Initialize the pcbFree list to contain all the elements
    if the static array or MAXPROC ProcBlk's. This method will
        be called only once during data structure initialization.
*/
void initPcbs(){
    static pcb_t procTable[MAXPROC];
    int i;
    pcbList_h = NULL;
    for(i=0;i < MAXPROC; i++){
        freePcb(&procTable[i]);
    }
}

/**********************************************************************/

/**************************** PROC QUEUE DEFINITIONS ******************/

/*  
    Initialize a variable to be a tail ptr to a proc queue
*/
pcb_PTR mkEmptyProcQ(){
	return NULL;
}

/*  
    Return TRUE if queue whose tail is pointed to by tp is empty. 
    Return FALSE otherwise
*/
int emptyProcQ(pcb_PTR tp){
	return (tp == NULL);
}

/*  
    Insert ProcBlk pointed to by p into process queue 
    whose tail-pointer is pointed to by tp.
*/
void insertProcQ(pcb_PTR *tp, pcb_PTR p){
	if(emptyProcQ(*tp)) /* queue empty */
	{
		*tp = p;
		p -> p_next = p;
		p -> p_prev = p;
		return;
	}
	/* queue len >= 1 */
	p -> p_next = (*tp) -> p_next;
	p -> p_prev = *tp;
	(*tp) -> p_next -> p_prev = p;
	(*tp) -> p_next = p;
	*tp = p;	
	return;	
}

/*  
    Remove the first (head) element from the process queue whose tail
    pointer is pointed to by tp. Return NULL if the queue is empty. 
    otherwise return pointer to the removed element. Update queue 
    as needed.
*/
pcb_PTR removeProcQ(pcb_PTR *tp){
	pcb_PTR ret;
	if(emptyProcQ(*tp)){ /* queue is empty */
		return ret = NULL;
	} else if ((*tp) -> p_next == *tp) { /* only one item in queue */
		ret = *tp;
		(*tp) = mkEmptyProcQ(); /* make tp an empty queue */
		return ret;
	} /* else */
	/* n items in queue */
	ret  = (*tp) -> p_next;
	(*tp) -> p_next -> p_next -> p_prev = *tp;
	(*tp) -> p_next = (*tp) -> p_next -> p_next;
	return ret;
}

/*  
    Remove ProcBlk pointed to by p from the process queue whose tail 
    pointer is pointed to by tp. Update the process queue's tail if 
    necessary. If ProcBlk is not in the queue, return NULL; otherwise
    return p.
*/
pcb_PTR outProcQ(pcb_PTR *tp, pcb_PTR p){
	pcb_PTR temp = (*tp);
	int i = FALSE;
	if(emptyProcQ((*tp))){ /* tp is empty */
		return NULL;
	} else if (p == NULL){ /* p not a valid pcb */
		return NULL;
	} else if ((*tp) == p) {
		/* p is the tail */
		return removeProcQ(tp);
	}
	/* go digging to make sure p is in tp */
	temp = temp -> p_next;
	while(temp != (*tp)){
		if(temp == p){
			i = TRUE; /* p is in the queue */
			break;
		}
		temp = temp -> p_next;
	}
	if(!i){
		return NULL; /* p is not in the queue */
	}
	/* generic case */
	p -> p_prev -> p_next = p -> p_next;
	p -> p_next -> p_prev = p -> p_prev;
	return p;	
}

/*  
    Return a pointer to the first ProcBlk from the process queue whose
    tail pointer is pointed to by tp. Does not remove the ProcBlk. 
    Return NULL if the queue is empty.
*/
pcb_PTR headProcQ(pcb_PTR tp){
	return (tp -> p_next);
}

/********************** END PROC QUEUE DEFINITIONS ********************/

/********************** CHILD TREE DEFINITIONS ************************/
/*
	Return TRUE if pcb_PRT p has no children. Returns FALSE otherwise.
*/ 
int emptyChild(pcb_PTR p){
	return (p -> p_child == NULL);
}

/*
	Make pcb pointed to p by a child of prnt
	inserts to the front of the child list
*/
void insertChild(pcb_PTR prnt, pcb_PTR p){
	if(emptyChild(prnt)){
		/* prnt has no children */
		p -> p_prnt = prnt;
		prnt -> p_child = p;
		return;
	}
	if(prnt -> p_child -> p_sibNext == NULL){
		/* only one child */
		p -> p_prnt = prnt;
		p -> p_sibNext = prnt -> p_child;
		prnt -> p_child -> p_sibPrev = p;
		prnt -> p_child -> p_sibNext = NULL;
		p -> p_sibPrev = NULL;
		prnt -> p_child = p;
		return;
	}
	/* multiple children */
	p -> p_prnt = prnt;
	p -> p_sibNext = prnt -> p_child;
	prnt -> p_child -> p_sibPrev = p;
	p -> p_sibPrev = NULL;
	prnt -> p_child = p;
	return;
}

/*
	Make first child of p no longer a child of p.
	return NULL if p initially has no children
	otherwise return pointer to removed process
*/
pcb_PTR removeChild(pcb_PTR p){
	pcb_PTR ret; /* return value */
	if(emptyChild(p)){ /* p has no children */
		return NULL; /* you're bad and you should feel bad */
	}
	ret = p -> p_child;
	if(p -> p_child -> p_sibNext == NULL){
		/* only one child */
		p -> p_child = NULL;
		p -> p_prnt = NULL;
		return ret;
	} else {
		/* has siblings */
		p -> p_child = p -> p_child -> p_sibNext;
		p -> p_child -> p_sibPrev = NULL;
		p -> p_prnt = NULL;
		return ret;		
	}
}

/*
	Make pcb pointed to by p no longer a child.
	return NULL if p has no parent, otherwise return p
	p can be in any point of the child list 
*/
pcb_PTR outChild(pcb_PTR p){
	if(p == NULL || p -> p_prnt == NULL){
		/* p is not a child */
		return NULL;
	}
	if(p -> p_prnt -> p_child == p){
		/* am newest child */
		return removeChild(p -> p_prnt);
	}
	if (p -> p_sibNext == NULL){
		/* p is at the end of child list */
		p -> p_sibPrev -> p_sibNext = NULL;
		p -> p_prnt = NULL;
		return p;
	}
	if (p -> p_sibPrev != NULL && p -> p_sibNext != NULL){ 
		/* p is is a middle child */
		p -> p_sibNext -> p_sibPrev = p -> p_sibPrev;
		p -> p_sibPrev -> p_sibNext = p -> p_sibNext;
		p -> p_prnt = NULL;
		return p;
	}
	/* should never get to this */
	return NULL;
}

/********************* END CHILD TREE DEFINITIONS *********************/

