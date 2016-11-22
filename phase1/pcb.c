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
 * 
 */
 
 /*
  * Pcb Free list is kept as a singly linked linear stack with the
  *  head as pcbList_h
  * 
  * Process Queue's are doubly linked circular queue with a tail pointer
  * 	the tails next is the queue's head
  * 
  * Child Tree is a double linked linear stack with a head pointer
  * 
  */


/**************************Global Definitions**************************/
/*The pointer to the head of the PCB Free List*/
HIDDEN pcb_PTR pcbList_h;

/*********************Free PCB List Implementation*********************/

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
    temp -> pgmTrpNew =  NULL;
    temp -> pgmTrpOld =  NULL;
    temp -> tlbNew =  NULL;
    temp -> tlbOld =  NULL;
    temp -> sysNew =  NULL;
    temp -> sysOld = NULL;
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
	
	/*If the queue is empty...*/
	if (emptyProcQ(*tp)){
		p->p_next = p;
		p->p_prev = p;
	} /*The queue is not empty*/
	else{
		/*Merge the new element into the list*/
		p->p_next = (*tp)->p_next;
		(*tp)->p_next->p_prev = p;
		(*tp)->p_next = p;
		p->p_prev = *tp;
	}
	
	/*Set the tail pointer to the new node*/
	*tp = p;
}


/*  
    Return a pointer to the first ProcBlk from the process queue whose
    tail pointer is pointed to by tp. Does not remove the ProcBlk. 
    Return NULL if the queue is empty.
*/
pcb_PTR headProcQ(pcb_PTR tp){
	return (tp -> p_next);
}


/*
    Remove the first (head) element from the process queue whose tail
    pointer is pointed to by tp. Return NULL if the queue is empty. 
    otherwise return pointer to the removed element. Update queue 
    as needed.
*/
pcb_PTR removeProcQ(pcb_PTR *tp){
	pcb_PTR ret;
	if(emptyProcQ(*tp)) { /* queue is empty */
		return NULL;
	} else if ((*tp) -> p_next == (*tp)) { /* only one item in queue */
		ret = (*tp);
		(*tp) = mkEmptyProcQ(); /* make tp an empty queue */
		return ret;
	} /* else */
	/* n items in queue */
	ret  = (*tp) -> p_next;
	(*tp) -> p_next -> p_next -> p_prev = (*tp);
	(*tp) -> p_next = ((*tp) -> p_next -> p_next);
	return ret;
}

/*  
    Remove ProcBlk pointed to by p from the process queue whose tail 
    pointer is pointed to by tp. Update the process queue's tail if 
    necessary. If ProcBlk is not in the queue, return NULL; otherwise
    return p.
*/
pcb_PTR outProcQ(pcb_PTR *tp, pcb_PTR p) {
	pcb_PTR ret, temp;
	if(((*tp) == NULL) || (p == NULL)) {
		return NULL;
	}
	/* only one thing in queue and it is what we want */
	if((*tp) == p){
		
		if ((((*tp) -> p_next) == (*tp))) {
			ret = (*tp);
			(*tp) = mkEmptyProcQ();
			return ret;
		} else {
			(*tp)->p_prev->p_next = (*tp)->p_next;
			(*tp)->p_next->p_prev = (*tp)->p_prev;
			*tp = (*tp)->p_prev;
		}
		return p;
	} else {
	/* node is somewhere else, start at p_next */
	temp = (*tp) -> p_next;
	while(temp != (*tp)) {
		/* found node ? */
		if(temp == p){
			/* unleave node and return it */
			ret = temp;
			ret -> p_prev -> p_next = ret -> p_next;
			ret -> p_next -> p_prev = ret -> p_prev;
			ret -> p_next = NULL;
			ret -> p_prev = NULL;
			return ret;
		}
			temp = temp -> p_next;
		}
		/* node not in list here */
		return NULL;
	}
}

/********************** CHILD TREE DEFINITIONS ************************/
/*
	Return TRUE if pcb_PRT p has no children. Returns FALSE otherwise.
*/ 
int emptyChild(pcb_PTR p){
	return ((p -> p_child) == NULL);
}

/*
	Make pcb pointed to p by a child of prnt
	inserts to the front of the child list
*/
void insertChild(pcb_PTR prnt, pcb_PTR p){
	if(emptyChild(prnt)){
		/* no children */
		prnt -> p_child = p;
		p -> p_prnt = prnt;
		p -> p_sibPrev = NULL;
		p -> p_sibNext = NULL;
		return;
	} else {
		/* multiple children */
		prnt -> p_child -> p_sibPrev = p;
		p -> p_sibNext = prnt -> p_child;
		p -> p_sibPrev = NULL;
		prnt -> p_child = p;
		p -> p_prnt = prnt;
		return;
	}
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
		ret -> p_prnt = NULL;
		ret -> p_sibNext = NULL;
		ret -> p_sibPrev = NULL;
		return ret;
	} else {
		/* has siblings */
		p -> p_child = p -> p_child -> p_sibNext;
		p -> p_child -> p_sibPrev = NULL;
		ret -> p_prnt = NULL;
		ret -> p_sibNext = NULL;
		ret -> p_sibPrev = NULL;
		return ret;		
	}
}

/*
	Make pcb pointed to by p no longer a child.
	return NULL if p has no parent, otherwise return p
	p can be in any point of the child list 
*/
pcb_PTR outChild(pcb_PTR p){
	if((p == NULL) || (p -> p_prnt == NULL)){
		/* p is not a child */
		return NULL;
	}
	if((p -> p_prnt -> p_child) == p){
		/* am newest child */
		return removeChild(p -> p_prnt);
	}
	if ((p -> p_sibNext) == NULL){
		/* p is at the end of child list */
		p -> p_sibPrev -> p_sibNext = NULL;
		p -> p_prnt = NULL;
		return p;
	}
	if (((p -> p_sibPrev) != NULL) && ((p -> p_sibNext) != NULL)){ 
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

