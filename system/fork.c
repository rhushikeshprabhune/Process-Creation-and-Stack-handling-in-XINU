/* fork.c - create child, newpid */

#include <xinu.h>

local	int newpid();
/*------------------------------------------------------------------------
 *  fork  -  Create child process from parent.
 *------------------------------------------------------------------------
 */
pid32	fork()
{
	intmask 	mask;    	/* Interrupt mask		*/
	pid32		pid;		/* Stores new process id	*/
	struct	procent	*prptr;		/* Pointer to proc. table entry */
        struct	procent	*prptr_parent;  /* Pointer to proc. table entry for parent process */
	uint32		*saddr;		/* Stack address		*/
        int32 i;

        /*  Get pointer to the parents PCB  */
        prptr_parent = &proctab[(pid32)getpid()];

        /*  Use the stack length of parent to find enough space for childs stack in memory */
        uint32 ssize = prptr_parent->prstklen;
	mask = disable();
	if (ssize < MINSTK)
		ssize = MINSTK;
	ssize = (uint32) roundmb(ssize);
	if ( (prptr_parent->prprio < 1) || ((pid=newpid()) == SYSERR) ||
	     ((saddr = (uint32 *)getstk(ssize)) == (uint32 *)SYSERR) ) {
		restore(mask);
		return SYSERR;
	}
       
	prcount++;
        /*  Get pointer to childs PCB  */
	prptr = &proctab[pid];

        /* The child has the same priority,stack length and name as parent process  */
	prptr->prprio = prptr_parent->prprio;
        prptr->prstklen = prptr_parent->prstklen;
	prptr->prname[PNMLEN-1] = NULLCH;
	for (i=0 ; i<PNMLEN-1 && (prptr->prname[i]=prptr_parent->prname[i])!=NULLCH; i++)
		;
        
        /* Initialise stack base pointer. This will be used afterwards since we have to copy the  */
        /* contents of the parent stack onto the child stack from the base to EBP of parent.     */
        prptr->prstkbase = (char *)saddr;
        
        /*Initialise other fields*/
	prptr->prsem = -1;
        prptr->prparent = (pid32)getpid();
	prptr->prhasmsg = FALSE;
        
        /* Initialise creation_time to ctr1000 to record the current value of ctr1000*/
        prptr->creation_time=ctr1000;

	/* Set up stdin, stdout, and stderr descriptors for the shell	*/
	prptr->prdesc[0] = CONSOLE;
	prptr->prdesc[1] = CONSOLE;
	prptr->prdesc[2] = CONSOLE;

        unsigned long* parent_FP;           /*  Parent frame pointer  */
        asm("movl %%ebp, %0\n" :"=r"(parent_FP));
        
        unsigned long* parent_base;         /* Parent base pointer  */
        parent_base=prptr_parent->prstkbase;

        unsigned long* child_base;         /* Child base pointer  */
        child_base=prptr->prstkbase;
      
        /*Copy parent stack from base to frame pointer (EBP) to child stack*/
        unsigned long* parent_iterator;
        unsigned long* child_iterator;
        parent_iterator=parent_base;
        child_iterator=child_base; 
        while(parent_iterator>=(parent_FP))
        {
            *child_iterator=*parent_iterator;
            parent_iterator--;
            child_iterator--;
        }
        parent_iterator++;
        child_iterator++;
        
        /*Modify child stack to contain the correct memory addresses of the child stack (NOT the parent stack)*/
        unsigned long* mod_iterator;
        unsigned long base_offset;
        mod_iterator=child_iterator;
        base_offset=(unsigned long)parent_base-(unsigned long)child_base;
        while(mod_iterator<child_base)
        {
           *mod_iterator=*mod_iterator-((unsigned long)base_offset);
           mod_iterator=*mod_iterator;
   
        }
        
	/* Initialize stack as if the process was called		*/
	/* The following entries on the stack must match what ctxsw	*/
	/*   expects a saved process state to contain: flags, registers	*/	                    
	/* Basically, the following emulates an x86 "pushal" and "pushfl" instruction*/

        *--child_iterator = 0x00000200;		/* New process runs with	*/
	*--child_iterator = NPROC;		/* %eax; return NPROC for child process */
	*--child_iterator = 0;			/* %ecx */
	*--child_iterator = 0;			/* %edx */
	*--child_iterator = 0;			/* %ebx */
	*--child_iterator = 0;			/* %esp; value filled in below	*/
	*--child_iterator = 0;		        /* %ebp (while finishing ctxsw)	*/
	*--child_iterator = 0;			/* %esi */
	*--child_iterator = 0;			/* %edi */
	prptr->prstkptr=child_iterator;	        /*   interrupts enabled		*/
	restore(mask);

        /*print memory addresses and data for parent and child*/
/*        unsigned long* piterator;*/
/*        unsigned long* citerator;*/
/*        piterator=parent_base;*/
/*        citerator=child_base;*/
/*        while(piterator!=(parent_FP-))*/
/*        {*/
/*            kprintf("3Parent:: Address: %X Data: %X\n",piterator,*piterator);*/
/*            kprintf("3Child :: Address: %X Data: %X\n",citerator,*citerator);*/
/*            piterator--;*/
/*            citerator--;*/
/*        }*/

        /*  This part deals with making the child ready and inserting it into ready list  */
        prptr->prstate = PR_READY;                /*  Modify the state of the child to denote it is ready for execution  */
        insert(pid,readylist,prptr->prprio);      /*  Insert child process in readylist */
      
	return pid;
}

/*------------------------------------------------------------------------
 *  newpid  -  Obtain a new (free) process ID
 *------------------------------------------------------------------------
 */
local	pid32	newpid(void)
{
	uint32	i;			/* Iterate through all processes*/
	static	pid32 nextpid = 1;	/* Position in table to try or	*/
					/*   one beyond end of table	*/

	/* Check all NPROC slots */

	for (i = 0; i < NPROC; i++) {
		nextpid %= NPROC;	/* Wrap around to beginning */
		if (proctab[nextpid].prstate == PR_FREE) {
			return nextpid++;
		} else {
			nextpid++;
		}
	}
	return (pid32) SYSERR;
}
