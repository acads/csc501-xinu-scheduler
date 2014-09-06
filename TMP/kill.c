/* kill.c - kill */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <q.h>
#include <stdio.h>
#include <lab1.h>

/*------------------------------------------------------------------------
 * kill  --  kill a process and remove it from the system
 *------------------------------------------------------------------------
 */
SYSCALL kill(int pid)
{
	STATWORD ps;    
	struct	pentry	*pptr;		/* points to proc. table for pid*/
	int	dev;

	disable(ps);
	if (isbadpid(pid) || (pptr= &proctab[pid])->pstate==PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	if (--numproc == 0)
		xdone();

	dev = pptr->pdevs[0];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->pdevs[1];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->ppagedev;
	if (! isbaddev(dev) )
		close(dev);
	
	send(pptr->pnxtkin, pid);

	freestk(pptr->pbase, pptr->pstklen);
	switch (pptr->pstate) {

	case PRCURR:	pptr->pstate = PRFREE;	/* suicide */
        DTRACE("DBG_INFO: %d %s: pid %d pname %s killed\n", currpid, __func__, pid, LS_GET_PNAME(pid));
			resched();

	case PRWAIT:	semaph[pptr->psem].semcnt++;

	case PRREADY:   if (LINUXSCHED == sched_algo) {
                            ls_remove_pid(pid);
                        } else if (RANDOMSCHED == sched_algo) {
                            rs_remove_pid(pid);
                        } else {
                            dequeue(pid); 
                        }
        		pptr->pstate = PRFREE;
			break;

	case PRSLEEP:
	case PRTRECV:	unsleep(pid);
						/* fall through	*/
	default:	pptr->pstate = PRFREE;
	}
        DTRACE("DBG_INFO: %d %s: pid %d pname %s killed\n", currpid, __func__, pid, LS_GET_PNAME(pid));
	restore(ps);
	return(OK);
}
