/* resume.c - resume */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <lab1.h>

/*------------------------------------------------------------------------
 * resume  --  unsuspend a process, making it ready; return the priority
 *------------------------------------------------------------------------
 */
SYSCALL resume(int pid)
{
	STATWORD ps;    
	struct	pentry	*pptr;		/* pointer to proc. tab. entry	*/
	int	prio;			/* priority to return		*/

	disable(ps);
        DTRACE_START;
	if (isbadpid(pid) || ((pptr= &proctab[pid])->pstate!=PRSUSP &&
                (PRNEWPA1 != pptr->pstate))) {
            DTRACE_END;
	    restore(ps);
	    return(SYSERR);
	}
	prio = pptr->pprio;        
        if (PRNEWPA1 == pptr->pstate) {
            /* New proc and Linux scheduling is being used. Force ready() to use
             * PRNEWPA1 as the new proc's state before enqueing.
             */
            ls_change_pstate = TRUE;
            ready(pid, RESCHYES);
            ls_change_pstate = FALSE;
        } else {
            ready(pid, RESCHYES);
        }
        DTRACE("DBG_INFO: %d %s: resumed pid %d\n", currpid, __func__, pid);
        DTRACE_END;
	restore(ps);        
	return(prio);
}
