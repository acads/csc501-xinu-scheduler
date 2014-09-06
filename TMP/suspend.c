/* suspend.c - suspend */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>
#include <lab1.h>

/*------------------------------------------------------------------------
 *  suspend  --  suspend a process, placing it in hibernation
 *------------------------------------------------------------------------
 */
SYSCALL	suspend(int pid)
{
    STATWORD ps;    
    struct	pentry	*pptr;		/* pointer to proc. tab. entry	*/
    int	prio;			/* priority returned		*/

    disable(ps);
    if (isbadpid(pid) || pid==NULLPROC || 
            ((pptr= &proctab[pid])->pstate!=PRCURR && pptr->pstate!=PRREADY)) {
	    restore(ps);
	    return(SYSERR);
    }

    /* If the proc is in ready queue, change its state and dequeue it. 
     * Or if it's the currproc, change its state and call resched() to
     * schedule another proc.
     */
    if (pptr->pstate == PRREADY) {
	pptr->pstate = PRSUSP;
        switch (sched_algo) {
            case RANDOMSCHED:
                rs_remove_pid(pid);
                break;        
            case LINUXSCHED:
                ls_remove_pid(pid);
                break;        
            default:
		dequeue(pid);
                break;        
        }
    }
    else {
	pptr->pstate = PRSUSP;
	resched();
    }
    prio = pptr->pprio;
    restore(ps);
    return(prio);
}
