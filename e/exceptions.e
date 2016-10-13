/*

*/

#ifndef exceptions
#define exceptions

extern void syscallHandler();
extern void tlbManager();
extern void pgmTrap();
extern void copyState(state_PTR src, state_PTR dest);


#endif
