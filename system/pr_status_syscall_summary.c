#include <xinu.h>
#include <stdio.h>
#include <math.h>
void pr_status_syscall_summary()
{
    pid32 pid;
    int k;
    uint32 avg_cycles;

    char syscall[8][10]={"create","kill","ready","sleepms","suspend","wait","wakeup","yield"};
    printf("%3s %7s %5s %14s\n",
           "pid","syscall","count","average cycles");

    printf("%3s %7s %5s %14s\n",
           "---","-------","-----","--------------");

    for(pid=0 ; pid<NPROC ; pid++)
    {

	int isActive=0;
	int j;
	for(j=0; j<8; j++) {
		if(summ_table[pid].syssumm[j].count > 0)
			isActive = 1;
	}

	if(isActive) {
           for(k=0 ; k<8 ; k++)
           {
               if(summ_table[pid].syssumm[k].count != 0)
               {
                   avg_cycles = summ_table[pid].syssumm[k].total_cycles/summ_table[pid].syssumm[k].count;
                   printf("%-3d %-7s %-5d %-14d\n", 
                   pid, syscall[k], summ_table[pid].syssumm[k].count, avg_cycles);
               }
           }
           printf("--------------------------------\n");

	}
    }
}
