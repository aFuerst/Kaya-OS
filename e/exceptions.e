/*

*/

#ifndef exceptions
#define exceptions

extern void syscallHandler();
extern void tlbManager();
extern void pgmTrap();

/* 
 * CopyState
 * Copies the processor state pointed to by src to the location pointed
 * to by dest
 */
extern void copyState(state_PTR src, state_PTR dest);


#endif
