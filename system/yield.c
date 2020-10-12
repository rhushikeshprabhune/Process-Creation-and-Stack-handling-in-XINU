/* yield.c - yield */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  yield  -  Voluntarily relinquish the CPU (end a timeslice)
 *------------------------------------------------------------------------
 */
syscall	yield(void)
{
	intmask	mask;			/* Saved interrupt mask		*/
        uint32 cycles_low, cycles_high, cycles_low1, cycles_high1;
        uint64 start, end;

	mask = disable();   
        
        asm volatile(
               "RDTSC\n\t"
               "mov %%edx, %0\n\t"
               "mov %%eax, %1\n\t": "=r" (cycles_high), "=r" (cycles_low):: 
        "%eax", "%edx");
	
        resched();
      
        asm volatile(
               "RDTSC\n\t"
               "mov %%edx, %0\n\t"
               "mov %%eax, %1\n\t": "=r" (cycles_high1), "=r" (cycles_low1)::
        "%eax", "%edx");

        start = (((uint64)cycles_high<<32)|cycles_low);
        end = (((uint64)cycles_high1<<32)|cycles_low1);
        summ_table[currpid].syssumm[7].total_cycles = summ_table[currpid].syssumm[7].total_cycles + (uint32)(end - start);
	summ_table[currpid].syssumm[7].count = summ_table[currpid].syssumm[7].count + 1;
       restore(mask);
	return OK;
}
