/* ready.c - ready */

#include <xinu.h>

qid16	readylist;			/* Index of ready list		*/

/*------------------------------------------------------------------------
 *  ready  -  Make a process eligible for CPU service
 *------------------------------------------------------------------------
 */
status	ready(
	  pid32		pid		/* ID of process to make ready	*/
	)
{

        register struct procent *prptr;
        uint32 cycles_low, cycles_high, cycles_low1, cycles_high1;
        uint64 start, end;

        

	if (isbadpid(pid)) {
		return SYSERR;
	}

        asm volatile(
               "RDTSC\n\t"
               "mov %%edx, %0\n\t"
               "mov %%eax, %1\n\t": "=r" (cycles_high), "=r" (cycles_low):: 
        "%eax", "%edx");
	/* Set process state to indicate ready and add to ready list */

	prptr = &proctab[pid];

	prptr->prstate = PR_READY;
	insert(pid, readylist, prptr->prprio);
	resched();
        asm volatile(
               "RDTSC\n\t"
               "mov %%edx, %0\n\t"
               "mov %%eax, %1\n\t": "=r" (cycles_high), "=r" (cycles_low):: 
        "%eax", "%edx");
        start = (((uint64)cycles_high<<32)|cycles_low);
        end = (((uint64)cycles_high1<<32)|cycles_low1);
        summ_table[currpid].syssumm[2].total_cycles = summ_table[currpid].syssumm[2].total_cycles + (end - start);
        summ_table[currpid].syssumm[2].count = summ_table[currpid].syssumm[2].count + 1;
	return OK;
}
