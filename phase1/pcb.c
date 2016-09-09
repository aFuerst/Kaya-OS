#include "../h/types.h"
#include "../h/const.h"


static pcb_PTR pcbList_h;

void debugA(int a){
	int i;
	i = 0;
		
}


/* DEFINITIONS */
/************************ PCB FREE LIST DEFINITIONS *******************/
/*  
    Insert the element pointed to by p onto the pcbFree list.
*/
void freePcb(pcb_PTR p){
    if (pcbList_h == NULL){
        pcbList_h = p;
        p -> p_next = NULL;
    } else {
        p -> p_next = pcbList_h;
        pcbList_h = p;
    }
}

/* 
    Return NULL if the pcbFree list is empty. Otherwise, remove
    an element from the pcbFree list, provide initial values for 
    all fields.
*/
pcb_t *allocPcb(){
    if(pcbList_h == NULL){
        return NULL;
    } else {
        pcb_PTR temp = pcbList_h;
        pcbList_h = pcbList_h -> p_next;
        /* set queue and tree values to NULL */
        temp -> p_next = NULL;
        temp -> p_prev = NULL;
        temp -> p_prnt = NULL;
        temp -> p_child = NULL;
        temp -> p_sibNext = NULL;
        temp -> p_sibPrev = NULL;
        /* set state values to 0 */
        /*
        temp -> p_s.s_asid = 0;
		temp -> p_s.s_cause = 0;
		temp -> p_s.s_status = 0;
		temp -> p_s.s_pc = 0;
		temp -> p_s.s_reg[STATEREGNUM] = 0;
		*/
        temp -> p_semAdd = NULL; /* not sure what proper state is */
        return temp;
    }
}

/* 
    Initialize the pcbFree list to contain all the elements
    if the static array or MAXPROC ProcBlk's. This method will
        be called only once during data structure initialization.
*/
void initPcbs(){
    static pcb_t procTable[MAXPROC];
    int i;
    pcbList_h = NULL; /* CHANGE */
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
    Insert ProcBlk pointed to by p into process queue whose tail-pointer
    is pointed to by tp.
*/
void insertProcQ(pcb_PTR *tp, pcb_PTR p){
	if(emptyProcQ(*tp)) /* queue empty */
	{
		*tp = p;
		p -> p_next = p;
		p -> p_prev = p;
	}
	else if ((*tp) -> p_next == *tp) /* queue len == 1 */
	{
		p -> p_next = *tp;
		p -> p_prev = *tp;
		(*tp) -> p_next = p;
		(*tp) -> p_prev = p;
		*tp = p;
	} else { /* queue len > 1 */
		p -> p_next = (*tp) -> p_next;
		p -> p_prev = *tp;
		(*tp) -> p_next -> p_prev = p;
		(*tp) -> p_next = p;
		*tp = p;		
	}
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
		*tp = NULL;
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
		debugA(0x00003333);
		return NULL;
	} else if (p == NULL){ /* p not a valid pcb */
		debugA(0x00004444);
		return NULL;
	} else if ( (*tp) -> p_next == (*tp) && (*tp) == p) {
		debugA(0x00001111);
		/* p is the tail */
		(*tp) = mkEmptyProcQ();
		return p;
	}
	/* go digging */
	temp = temp -> p_next;
	while(temp != (*tp)){
		if(temp == p){
			i = TRUE; /* p is in the queue */
			break;
		}
		temp = temp -> p_next;
	}
	if(!i){
		debugA(0x00002222);
		return NULL; /* p is not in the queue */
	}
	/* generic case */
	p -> p_prev -> p_next = p -> p_next;
	p -> p_next -> p_prev = p -> p_prev;
	if( p == (*tp))  /* check if removed node is the tail */
	{
		(*tp) = p -> p_prev;
	}
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
		/*
		p -> p_sibNext = NULL;
		p -> p_sibPrev = NULL;
		*/
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
		/*
		ret -> p_sibNext = NULL;
		ret -> p_sibPrev = NULL;
		*/
		p -> p_prnt = NULL;
		return p;
	} else {
		/* has siblings */
		p -> p_child = p -> p_child -> p_sibNext;
		p -> p_child -> p_sibPrev = NULL;
		/*
		ret -> p_sibNext = NULL;
		ret -> p_sibPrev = NULL;
		*/
		p -> p_prnt = NULL;
		return p;		
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
		p -> p_sibNext = NULL;
		p -> p_sibPrev = NULL;
		return p;
	}
	if (p -> p_sibPrev != NULL && p -> p_sibNext != NULL){ 
		/* p is is a middle child */
		p -> p_sibNext -> p_sibPrev = p -> p_sibPrev;
		p -> p_sibPrev -> p_sibNext = p -> p_sibNext;
		p -> p_prnt = NULL;
		p -> p_sibNext = NULL;
		p -> p_sibPrev = NULL;
		return p;
	}
	/* should never get to this */
	return NULL;
}

/********************* END CHILD TREE DEFINITIONS *********************/

