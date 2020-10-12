/* wakeup.c - wakeup */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  wakeup  -  Called by clock interrupt handler to awaken processes
 *------------------------------------------------------------------------
 */
void	wakeup(void)
{

        uint32 cycles_low, cycles_high, cycles_low1, cycles_high1;
        uint64 start, end;

	/* Awaken all processes that have no more time to sleep */
        asm volatile(
               "RDTSC\n\t"
               "mov %%edx, %0\n\t"
               "mov %%eax, %1\n\t": "=r" (cycles_high), "=r" (cycles_low):: 
        "%eax", "%edx");

	resched_cntl(DEFER_START);
	while (nonempty(sleepq) && (firstkey(sleepq) <= 0)) {
		ready(dequeue(sleepq));
	}

        

	resched_cntl(DEFER_STOP);
    
        asm volatile(
               "RDTSC\n\t"
               "mov %%edx, %0\n\t"
               "mov %%eax, %1\n\t": "=r" (cycles_high1), "=r" (cycles_low1)::
        "%eax", "%edx");

        start = (((uint64)cycles_high<<32)|cycles_low);
        end = (((uint64)cycles_high1<<32)|cycles_low1);
        summ_table[currpid].syssumm[6].total_cycles = summ_table[currpid].syssumm[6].total_cycles + (end - start);
        summ_table[currpid].syssumm[6].count = summ_table[currpid].syssumm[6].count + 1;
	return;
}
