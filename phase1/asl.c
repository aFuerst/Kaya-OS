/*
 * ASL.c
 * Active Semaphore List
 * the active list is stored as a singly linked list with two dummy
 * nodes (front and back)
 * Contains functions for:
 *  semaphore free list
 *      allocate and free semd
 *  inserting and removing blocked processes
 *      insertBlocked
 *      removeBlocked
 *      outBlocked
 *  
 * 
 * Authors: Alex Fuerst, Aaron Pitman
 * */

#include "../h/types.h"
#include "../h/const.h"
#include "../e/pcb.e"

/* head of free semaphore list */
HIDDEN semd_t *semdFreeList_h;
/* head of active semaphore list */
HIDDEN semd_t *semdActiveList_h;

/* hidden functions defined later needed within the module */
HIDDEN semd_t *search(int *semAdd);
HIDDEN void freeSEMD(semd_t *s);
HIDDEN semd_t *allocSEMD();

void debugA(int a){
	int i;
	i=0;
}

/*
	insert the pcb pointed to be p at the tail of the process queue
	at semaphore semAdd and set it's semaphore address to semAdd.
	If a new semaphore needs to be allocated and the semdFree list 
	is empty, return TRUE. Otherwise return FALSE.
*/
int insertBlocked(int *semAdd, pcb_PTR p){
    /* find insert location */
    semd_t *temp = search(semAdd);
	if(temp -> s_next -> s_semAdd == semAdd) {
        /* insert location exists */
		p -> p_semAdd = semAdd;
		insertProcQ(&(temp -> s_next -> s_procQ), p);
        /* easy peasy, lemon squeasy */
		return FALSE;
	} 
	else {
        /* not so easy, need to allocate new node */
		semd_t *newSem = allocSEMD();
		if(newSem == NULL) {
            /* too many semaphores already */
			return TRUE;
		}
        /* insert new node into active list */
		newSem -> s_next = temp -> s_next;
		temp -> s_next = newSem;
		newSem -> s_procQ = mkEmptyProcQ();
        /* insert ProcBlk into new semaphore */
		insertProcQ(&(newSem -> s_procQ),p);
		newSem -> s_semAdd = semAdd;
		p -> p_semAdd = semAdd;
		return FALSE;
	}
}

/* 
    Search the ASL for a descriptor of this semaphore. If none is
    found, return NULL; otherwise, remove the first (head) ProcBlk from 
    the process queue of the found semaphore descriptor and return a
    pointer to it. If the process queue for this semaphore becomes empty
    remove the semaphore descriptor from the ASL and return it to the
    free list.
*/
pcb_PTR removeBlocked(int *semAdd){
    /* find remove location */
	semd_t *parent = search(semAdd);
	if(parent -> s_next -> s_semAdd == semAdd) {
        /* next node is where we want to remove from */
		pcb_PTR returnVal = removeProcQ(&(parent -> s_next -> s_procQ));
		if(emptyProcQ(parent -> s_next -> s_procQ)){
            /* semaphore is empty, free it */
			semd_t *temp = parent -> s_next;
			parent -> s_next = parent -> s_next -> s_next; 
			freeSEMD(temp);
		}
		returnVal -> p_semAdd = NULL; /* no longer has a semAdd */
		return returnVal;
	}
    /* node doesn't exist, jerk */
	return NULL;
}

/* 
    Remove the ProcBlk pointed to by p from the process queue associated
    with p's semaphore on the ASL. If ProcBlk pointed to by p does not
    appear in the process queue associated with p's semaphore, which is
    an error condition, return NULL; otherwise, return p
*/
pcb_PTR outBlocked(pcb_PTR p){
	/* find remove location */
	semd_t *parent = search(p -> p_semAdd);
	if(parent -> s_next -> s_semAdd == p -> p_semAdd) {
        /* next node is where we want to remove from */
        pcb_PTR returnVal = outProcQ(&(parent -> s_next -> s_procQ), p);
		if(emptyProcQ(parent -> s_next -> s_procQ)){
            /* semaphore is empty, free it */
			semd_t *temp = parent -> s_next;
			parent -> s_next = parent -> s_next -> s_next; 
			freeSEMD(temp);
		}
		returnVal -> p_semAdd = NULL; /* no longer has a semAdd */
        return returnVal;
	}
    /* node doesn't exist, jerk */
	return NULL;
}

/*
	Returns a pointer to procBlok that is at the head of process queue
	with semaphore semAdd. return NULL if it doesn't exist or if the 
	process queue with semAdd is empty
*/
pcb_PTR headBlocked(int *semAdd){
	semd_t *temp = search(semAdd);
	if(temp -> s_next -> s_semAdd == semAdd){
		return headProcQ(temp -> s_next -> s_procQ);
	}
	return NULL;
}

/*
	Initialize the semdFree list to contain all the elements of the array
    static semd_t semdTable[MAXPROC] 
    This method will only be called once during data structure initialization
*/
void initASL(){
    static semd_t semArr[MAXPROC + 2];
    int i;
    semdActiveList_h = NULL;
    semdFreeList_h = NULL;
    /* insert MAXPROC nodes onto the free list */
	for(i=0;i<MAXPROC;++i){
		freeSEMD(&(semArr[i]));
	}
    /* two extra nodes placed as dummies on the semaphore list */
	/* initialize the active array with 2 dummy nodes */
	semdActiveList_h = &(semArr[MAXPROC + 1]);
	semdActiveList_h -> s_next = NULL;
    /* last node in active list */
	semdActiveList_h -> s_semAdd = (int*)MAX_INT; 
	semdActiveList_h -> s_procQ = NULL;
	
	(semArr[MAXPROC]).s_next = semdActiveList_h;
	semdActiveList_h = &(semArr[MAXPROC]);
	semdActiveList_h -> s_semAdd = 0; /* frist node in active list */
	semdActiveList_h -> s_procQ = NULL;
}

/*
    Removes a semaphore from the free list and sets it's values to the
    default ones. Returns a pointer to this new node.
*/
HIDDEN semd_t *allocSEMD(){
	semd_t *ret = semdFreeList_h;
	if(semdFreeList_h == NULL){
        /* free list is null */
		return NULL;
	}
	semdFreeList_h = semdFreeList_h -> s_next;
	/* set values to start state */
	ret -> s_next = NULL;
	ret -> s_semAdd = 0;
	ret -> s_procQ = mkEmptyProcQ(); 
	return ret;
}

/*
    Takes a pointer to a semaphore and inserts it on the semaphore
    free list.
*/
HIDDEN void freeSEMD(semd_t *s){
	if (semdFreeList_h == NULL){
        /* free list is currently empty */
        semdFreeList_h = s;
        semdFreeList_h -> s_next = NULL;
    } else {
        /* push node onto free list */
        s -> s_next = semdFreeList_h;
        semdFreeList_h = s;
    }	
}

/*
	Searches until it finds the parent of semAdd
	returns a semd_t pointer to that parent. If not found it returns
	pointer to the parent of where it would be.
*/
HIDDEN semd_t *search(int *semAdd){
	semd_t *temp = semdActiveList_h;
	if(semAdd == NULL){
		semAdd = (int*) MAX_INT;
	}
	while(semAdd > temp -> s_next -> s_semAdd){
        /* advance until next node is greater than search value */
		temp = temp -> s_next;
	}
	return temp;
}
