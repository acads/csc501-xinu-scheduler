/* resched.c  -  resched */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>
#include <lab1.h>

unsigned long currSP;	/* REAL sp of current process */
extern int ctxsw(int, int, int, int);
/*-----------------------------------------------------------------------
 * resched  --  reschedule processor to highest priority ready process
 *
 * Notes:	Upon entry, currpid gives current process id.
 *		Proctab[currpid].pstate gives correct NEXT state for
 *			current process if other than PRREADY.
 *------------------------------------------------------------------------
 */
int resched()
{
    register struct pentry *optr = NULL;    /* pointer to old process entry */
    register struct pentry *nptr = NULL;    /* pointer to new process entry */
    //STATWORD ps;

    //disable(ps);
    DTRACE_START;
    optr = &proctab[currpid];
    switch(sched_algo) {
        case RANDOMSCHED:
            nptr = rand_sched(optr);
            break;

        case LINUXSCHED:
            optr->pctr = preempt;
            if ((int) optr->pctr < 0) {
                optr->pctr = 0;
            }
            if (PRFREE == optr->pstate) {
                /* Reset the pctr for free procs, so that linux_sched can start
                 * a new epoch (if possible) or run nullproc.
                 */
                optr->pctr = optr->pquant = optr->pgood = 0;
                DTRACE("DBG_INFO: %d %s: reset ctrs for free proc\n", currpid, __func__);
            }
            DTRACE("DBG_INFO: %d %s: pctr %d\n", currpid, __func__, optr->pctr);
            nptr = linux_sched(optr);
            break;

        default:
            nptr = xinu_sched(optr);
            break;
    }
    if (NULL == nptr) {
        /* Current process is good enough to run. */
        //restore(ps);
        return OK;
    }
    DTRACE("DBG_INFO: %s opid %d, npid %d\n", __func__, optr->pid, nptr->pid);

    /* Save the time spent by current process in the CPU. 
     * The states of process will be changed appropriately by the caller 
     * (PCURR --> PSLEEP in case of a sleep) before calling resched().
     * So, we need to save the delta spent by the current process for every
     * possible state other than PREADY.
     */
    switch ((int) optr->pstate) {
        case PRCURR:
        case PRFREE:
        case PRRECV:
        case PRSLEEP:
        case PRSUSP:
        case PRWAIT:
        case PRTRECV:
            optr->pouts += 1;
            if (LINUXSCHED == sched_algo) {
                optr->pticks += (optr->pquant - PA1_TICKS);
            } else {
                optr->pticks += (QUANTUM - PA1_TICKS);
            }
            break;
        default:
            break;
    }

    /* Force context switch if the old process is the current process. */
    if (optr->pstate == PRCURR) {
        optr->pstate = PRREADY;
        switch (sched_algo) {
            case RANDOMSCHED:
                enqueue_pid_pa1(currpid);
                break;
            case LINUXSCHED:
                ls_enqueue_pid(ls_rq, ls_rhead, ls_rtail, currpid, 
                        LS_GET_PGOOD(currpid));
                break;
            default:
                insert(currpid, rdyhead, optr->pprio);
                break;
        }
    }

    /* Change the state of the newly elected process to PRCURR and 
     * do ctxsw.
     */
    currpid = nptr->pid;
    nptr->pstate = PRCURR;      /* mark it currently running	*/
    nptr->pins += 1;
#ifdef	RTCLOCK
    if (LINUXSCHED == sched_algo) {
        preempt = nptr->pctr;
        //preempt = nptr->pquant;
    } else {
        preempt = QUANTUM;
    }
    DTRACE("DBG_INFO: %d %s: preempt %d\n", currpid, __func__, preempt);
#endif
    //restore(ps);
	
    ctxsw((int)&optr->pesp, (int)optr->pirmask, 
            (int)&nptr->pesp, (int)nptr->pirmask);
	
    /* The OLD process returns here when resumed. */    
    DTRACE_END;
    return OK;
}
