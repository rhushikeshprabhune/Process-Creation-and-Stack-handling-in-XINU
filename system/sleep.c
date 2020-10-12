/* sleep.c - sleep sleepms */

#include <xinu.h>

#define	MAXSECONDS	2147483		/* Max seconds per 32-bit msec	*/

/*------------------------------------------------------------------------
 *  sleep  -  Delay the calling process n seconds
 *------------------------------------------------------------------------
 */
syscall	sleep(
	  int32	delay		/* Time to delay in seconds	*/
	)
{
	if ( (delay < 0) || (delay > MAXSECONDS) ) {
		return SYSERR;
	}
	sleepms(1000*delay);
	return OK;
}

/*------------------------------------------------------------------------
 *  sleepms  -  Delay the calling process n milliseconds
 *------------------------------------------------------------------------
 */
syscall	sleepms(
	  int32	delay			/* Time to delay in msec.	*/
	)
{
	intmask	mask;			/* Saved interrupt mask		*/

	if (delay < 0) {
		return SYSERR;
	}
        
        
	if (delay == 0) {
		yield();
		return OK;
	}


        uint32 cycles_low, cycles_high, cycles_low1, cycles_high1;
        uint64 start, end;
	/* Delay calling process */

	mask = disable();
        

        asm volatile(
               "RDTSC\n\t"
               "mov %%edx, %0\n\t"
               "mov %%eax, %1\n\t": "=r" (cycles_high), "=r" (cycles_low):: 
        "%eax", "%edx");

	if (insertd(currpid, sleepq, delay) == SYSERR) {
		restore(mask);
		return SYSERR;
	}

	proctab[currpid].prstate = PR_SLEEP;
	resched();

        asm volatile(
               "RDTSC\n\t"
               "mov %%edx, %0\n\t"
               "mov %%eax, %1\n\t": "=r" (cycles_high1), "=r" (cycles_low1)::
        "%eax", "%edx");

        start = (((uint64)cycles_high<<32)|cycles_low);
        end = (((uint64)cycles_high1<<32)|cycles_low1);
        summ_table[currpid].syssumm[3].total_cycles = summ_table[currpid].syssumm[3].total_cycles + (uint32)(end - start);
        summ_table[currpid].syssumm[3].count = summ_table[currpid].syssumm[3].count + 1;
	restore(mask);
	return OK;
}
