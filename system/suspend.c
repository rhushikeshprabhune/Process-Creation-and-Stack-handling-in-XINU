/* suspend.c - suspend */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  suspend  -  Suspend a process, placing it in hibernation
 *------------------------------------------------------------------------
 */
syscall	suspend(
	  pid32		pid		/* ID of process to suspend	*/
	)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	procent *prptr;		/* Ptr to process's table entry	*/
	pri16	prio;			/* Priority to return		*/

        uint32 cycles_low, cycles_high, cycles_low1, cycles_high1;
        uint64 start, end;

	mask = disable();

        asm volatile(
               "RDTSC\n\t"
               "mov %%edx, %0\n\t"
               "mov %%eax, %1\n\t": "=r" (cycles_high), "=r" (cycles_low):: 
        "%eax", "%edx");

	if (isbadpid(pid) || (pid == NULLPROC)) {
		restore(mask);
		return SYSERR;
	}
   
	/* Only suspend a process that is current or ready */

	prptr = &proctab[pid];

        

	if ((prptr->prstate != PR_CURR) && (prptr->prstate != PR_READY)) {
		restore(mask);
		return SYSERR;
	}
	if (prptr->prstate == PR_READY) {
		getitem(pid);		    /* Remove a ready process	*/
					    /*   from the ready list	*/
		prptr->prstate = PR_SUSP;
	} else {
		prptr->prstate = PR_SUSP;   /* Mark the current process	*/
		resched();		    /*   suspended and resched.	*/
	}
	prio = prptr->prprio;

        summ_table[currpid].syssumm[4].count = summ_table[currpid].syssumm[4].count + 1;

        asm volatile(
               "RDTSC\n\t"
               "mov %%edx, %0\n\t"
               "mov %%eax, %1\n\t": "=r" (cycles_high1), "=r" (cycles_low1)::
        "%eax", "%edx");

        start = (((uint64)cycles_high<<32)|cycles_low);
        end = (((uint64)cycles_high1<<32)|cycles_low1);
        summ_table[currpid].syssumm[4].total_cycles = summ_table[currpid].syssumm[4].total_cycles + (end - start);
        summ_table[currpid].syssumm[4].count = summ_table[currpid].syssumm[4].count + 1;
	restore(mask);
	return prio;
}
