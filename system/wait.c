/* wait.c - wait */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  wait  -  Cause current process to wait on a semaphore
 *------------------------------------------------------------------------
 */
syscall	wait(
	  sid32		sem		/* Semaphore on which to wait  */
	)
{
	intmask mask;			/* Saved interrupt mask		*/
        pid32 pid;
        uint32 cycles_low, cycles_high, cycles_low1, cycles_high1;
        uint64 start, end;
	mask = disable();
        asm volatile(
               "RDTSC\n\t"
               "mov %%edx, %0\n\t"
               "mov %%eax, %1\n\t": "=r" (cycles_high), "=r" (cycles_low):: 
        "%eax", "%edx");     

	struct	procent *prptr;		/* Ptr to process's table entry	*/
	struct	sentry *semptr;		/* Ptr to sempahore table entry	*/
        pid=getpid();
        
	if (isbadsem(sem)) {
		restore(mask);
		return SYSERR;
	}
        

	semptr = &semtab[sem];
	if (semptr->sstate == S_FREE) {
		restore(mask);
		return SYSERR;
	}

        

	if (--(semptr->scount) < 0) {		/* If caller must block	*/
		prptr = &proctab[currpid];
		prptr->prstate = PR_WAIT;	/* Set state to waiting	*/
		prptr->prsem = sem;		/* Record semaphore ID	*/
		enqueue(currpid,semptr->squeue);/* Enqueue on semaphore	*/
		resched();			/*   and reschedule	*/
	}

        asm volatile(
               "RDTSC\n\t"
               "mov %%edx, %0\n\t"
               "mov %%eax, %1\n\t": "=r" (cycles_high1), "=r" (cycles_low1)::
        "%eax", "%edx");

        start = (((uint64)cycles_high<<32)|cycles_low);
        end = (((uint64)cycles_high1<<32)|cycles_low1);
        summ_table[currpid].syssumm[5].total_cycles = summ_table[currpid].syssumm[5].total_cycles + (end - start);
        summ_table[currpid].syssumm[5].count = summ_table[currpid].syssumm[5].count + 1;
	restore(mask);
	return OK;
}
