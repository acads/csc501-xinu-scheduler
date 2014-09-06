/* ready.c - ready */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lab1.h>

/*------------------------------------------------------------------------
 * ready  --  make a process eligible for CPU service
 *------------------------------------------------------------------------
 */
int ready(int pid, int resch)
{
	register struct	pentry	*pptr;

	if (isbadpid(pid))
		return(SYSERR);
	pptr = &proctab[pid];
        if ((LINUXSCHED == sched_algo) && (TRUE == ls_change_pstate)) {
            pptr->pstate = PRNEWPA1;
        } else {
            pptr->pstate = PRREADY;
        }
        switch(sched_algo)
        {
            case RANDOMSCHED:
                enqueue_pid_pa1(pid);
                break;
            case LINUXSCHED:
                if (TRUE == ls_change_pstate) {
                    pptr->pstate = PRNEWPA1;
                }
                ls_enqueue_pid(ls_rq, ls_rhead, ls_rtail, pid, LS_GET_PGOOD(pid));
                break;
            default:
                insert(pid,rdyhead,pptr->pprio);
                break;
        }
	if (resch)
		resched();
	return(OK);
}
