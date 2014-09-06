/* lsched.c -- Implements Linux-like scheduling */

#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>
#include <lab1.h>

/* Globals for LS */
int ls_change_pstate = FALSE;   /* to be used by resume() to for ready() to use
                                 * PRNEWPA1 as state for new procs in LSched. */
int ls_rhead = NPROC;           /* head of ls ready queue */
int ls_rtail = NPROC + 1;       /* tail of ls ready queue */
struct qent ls_rq[NPQENT];      /* ready queue for Linux scheduling */
int ls_prio_pid[NPROC] = {EMPTY_PA1};
int ls_prio_pri[NPROC] = {EMPTY_PA1};


/* Sorts the pid's in non-ascending order of priority. This will be used in case
 * of scheduling two different procs having same goodness. The process with the
 * higher priority wins.
 */
void
ls_prio_sort(void)
{
    int i, j, tmp, max, id;
    tmp = max = 0;
    id = -1;

    for (i = 1; i < NPROC; ++i) {
        ls_prio_pri[i] = LS_GET_PPRIO(i);
    }

    for (j = 0; j < NPROC; ++j) {
        for (i = 1; i < NPROC; ++i) {
            if (0 == LS_GET_PPRIO(i)) {
                continue;
            }
            if (PRREADY != LS_GET_PSTATE(i)) {
                continue;
            }
            tmp = ls_prio_pri[i];
            if (tmp > max) {
                max = tmp;
                id = i;
            }
        }
        ls_prio_pid[j] = id;
        ls_prio_pri[id] = EMPTY_PA1;
        max = tmp = 0;
        id = EMPTY_PA1;
    }
#if 0
    DTRACE("\n");
    for (i = 0; i < NPROC; ++i) {
        DTRACE("%d, ", ls_prio_pid[i]);
    }
    DTRACE("\n");
#endif
    return;
}


/* Prints the contents of the proctab. */
void
ls_print_proctab(void)
{
    int pid = 0;
    struct pentry *pptr;

    for (pid = 0; pid < NPROC; ++pid) {
        pptr = LS_GET_PPTR(pid);    
        if (PRFREE == pptr->pstate) {
            continue;
        }
        DTRACE("PID: %d PName: %s PState: %s, ", pptr->pid, pptr->pname, ch_pstates[pptr->pstate - 1]);
        DTRACE("PPrio: %d, PQuant: %d, PCtr: %d, PGood: %d\n", pptr->pprio, pptr->pquant, pptr->pctr, pptr->pgood);
    }
    return;
}


/* Prints the contents of LS queue. */
void
ls_print_queue(struct qent *qptr, int head, int tail)
{
    int pid = 0;
    DTRACE_START;

    pid = qptr[tail].qprev;
    DTRACE("DBG_INFO: %d %s: ---LS rq info start---\n", currpid, __func__);
    DTRACE("DBG_INFO: %d %s: pid pname pstate pquant pctr pgood\n", currpid, __func__);
    while (pid != head) {
        DTRACE("DBG_INFO: %d %s: %d %s %s %d %d %d\n", currpid, __func__, proctab[pid].pid, proctab[pid].pname, ch_pstates[LS_GET_PSTATE(pid) - 1], LS_GET_PQUANT(pid), LS_GET_PCTR(pid), LS_GET_PGOOD(pid));
        pid = qptr[pid].qprev;
    }
    DTRACE("DBG_INFO: %d %s: ---LS rq info end---\n", currpid, __func__);

    DTRACE_END;
    return;
}

/* Initializes the given LS queue. */
void
ls_init_queue(struct qent *qptr, int head, int tail)
{
    int next = 0;
    DTRACE_START;

    qptr[head].qnext = tail;
    qptr[head].qprev = EMPTY_PA1;
    qptr[head].qkey = LS_MIN_PGOOD;
    qptr[tail].qprev = head;
    qptr[tail].qnext = EMPTY_PA1;
    qptr[tail].qkey = LS_MAX_PGOOD;
    next = qptr[head].qnext;
    for (next = 0; next < head; ++next) {
        qptr[next].qprev = qptr[next].qnext = EMPTY_PA1;
    }

    DTRACE_END;
    return;
}


/* Reorders the given queue based on the key. In Linux-like scheduling the key
 * is usually the goodness of a proc. The proc with more goodness will be at
 * the tail of the queue and will be scheduled next.
 *
 * This function will be usually called at the beginning of an epoch.
 */
void
ls_reorder_queue(struct qent *qptr, int head, int tail)
{
    uint32_t i = 0;
    uint32_t pid = 0;
    DTRACE_START;

    ls_prio_sort();
    for (i = 0; i < NPROC; ++i) {
        pid = ls_prio_pid[i];
        if (PRREADY == LS_GET_PSTATE(pid)) {
            ls_enqueue_pid(ls_rq, head, tail, pid, LS_GET_PGOOD(pid));
        }
    }
    ls_print_queue(ls_rq, ls_rhead, ls_rtail);
    DTRACE_END;
    return;
}


/* Inserts the given pid into the given ls queue at the appropriate place 
 * based on the process goodness. More goodness leans towards the tail.
 */
int
ls_enqueue_pid(struct qent *qptr, int head, int tail, int pid, int pgood)
{
    int nnext;
    int rprev;
    int next;
    int prev;
    DTRACE_START;

    DTRACE("DBG_INFO: %d %s: pid %d, pquant %d, pctr %d, pgood %d\n", currpid, __func__, pid, LS_GET_PQUANT(pid), LS_GET_PCTR(pid), pgood);
    if (NULLPROC == pid) {
        /* Never admit the NULLPROC */
        DTRACE("DBG_INFO: %d %s: nullproc not admitted\n", currpid, __func__);
        DTRACE_END;
        return OK;
    }
    
    if (PRREADY == LS_GET_PSTATE(pid)) {
        rprev = qptr[tail].qprev;
        while ((rprev != head) && (PRNEWPA1 != LS_GET_PSTATE(rprev))) {
            if (qptr[rprev].qkey > pgood) {
                rprev = qptr[rprev].qprev;
            }
            else if (qptr[rprev].qkey == pgood) {
                /* The proc with higher priority leans over tail. */
                if (LS_GET_PPRIO(rprev) > LS_GET_PPRIO(pid)) {
                    rprev = qptr[rprev].qprev;
                } else {
                    break;
                }
            } else {
                break;
            }
        }
        qptr[pid].qprev = rprev;
        qptr[pid].qnext = next = qptr[rprev].qnext;
        qptr[pid].qkey  = pgood;
        qptr[rprev].qnext = pid;
        qptr[next].qprev = pid;
        DTRACE("DBG_INFO: %d %s: pid %d enqd in ls_rq after %d & before %d\n", currpid, __func__, pid, ls_rq[pid].qprev, ls_rq[pid].qnext);
    } else if (PRNEWPA1 == LS_GET_PSTATE(pid)) {
        nnext = qptr[head].qnext;
        while ((nnext != tail) && (PRREADY != LS_GET_PSTATE(nnext)) &&
                (qptr[nnext].qkey < pgood)) {
            nnext = qptr[nnext].qnext;
        }
        qptr[pid].qnext = nnext;
        qptr[pid].qprev = prev = qptr[nnext].qprev;
        qptr[pid].qkey  = pgood;
        qptr[prev].qnext = pid;
        qptr[nnext].qprev = pid;
        DTRACE("DBG_INFO: %d %s: pid %d enqd in ls_nq after %d & before %d\n", currpid, __func__, pid, ls_rq[pid].qprev, ls_rq[pid].qnext);
    }
    DTRACE_END;
    return(OK);
}


/* Dequeues a process from the tail of the given ls queue */
int
ls_dequeue_pid(struct qent *qptr, int head, int tail)
{
    int ret_pid = 0;
    struct qent *rptr;
    DTRACE_START;

    if (LS_IS_Q_EMPTY(qptr, head, tail)) {
        DTRACE("DBG_INFO: %d %s: empty ls_rq, returning EMPTY_PA1\n", currpid, __func__);
        return EMPTY_PA1;
    }
    ret_pid = qptr[tail].qprev;
    if (PRREADY != LS_GET_PSTATE(ret_pid)) {
        DTRACE("DBG_INFO: %d %s: no PRREADY procs, returning EMPTY_PA1\n", currpid, __func__);
        return EMPTY_PA1;
    }
    while ((PRREADY == LS_GET_PSTATE(ret_pid)) && (ret_pid != head)) {
        if ((0 != LS_GET_PCTR(ret_pid))) {
            break;
        }
        ret_pid = qptr[ret_pid].qprev;        
    }
    if ((PRREADY == LS_GET_PSTATE(ret_pid)) && (0 != LS_GET_PCTR(ret_pid)) &&
            (ret_pid != head)) {
        rptr = &qptr[ret_pid];
        qptr[rptr->qnext].qprev = rptr->qprev;
        qptr[rptr->qprev].qnext = rptr->qnext;
        DTRACE("DBG_INFO: %d %s: deq'd pid %d\n", currpid, __func__, ret_pid);
        DTRACE_END;
        return ret_pid;
    } else {
        /* Nothing runnable. */
        DTRACE("DBG_INFO: %d %s: nothing runnable\n", currpid, __func__);
        DTRACE_END;
        return EMPTY_PA1;
    }
    DTRACE_END;
    return ret_pid;
}


/* Removes the given pid from the LS ready queue if present. This will be
 * used to remove a proc from ready queue if it's suspended or terminated.
 */
int
ls_remove_pid(int pid)
{
    int head = ls_rhead;
    int tail = ls_rtail;
    int iter_pid = ls_rq[tail].qprev;

    DTRACE_START;
    while (iter_pid != head) {
        if (pid == iter_pid) {
            /* Got our victim. Remove him. */
            ls_rq[ls_rq[iter_pid].qprev].qnext = ls_rq[iter_pid].qnext;
            ls_rq[ls_rq[iter_pid].qnext].qprev = ls_rq[iter_pid].qprev;
            ls_rq[iter_pid].qnext = ls_rq[iter_pid].qprev = EMPTY_PA1;
            DTRACE("DBG_INFO: %d %s: removed pid %d from ls_rq\n", currpid, __func__, pid);
            ls_print_queue(ls_rq, ls_rhead, ls_rtail);
            DTRACE_END;
            return pid;
        }
        iter_pid = ls_rq[iter_pid].qprev;
    }
    /* Given pid is not even present in ls_rq. */
    DTRACE("DBG_INFO: %d %s: pid %d not found in ls_rq\n", currpid, __func__, pid);
    DTRACE_END;
    return EMPTY_PA1;
}


/* Builds a ls ready queue from any of the existing queues - Xinu 
 * queue or random scheduling queue.
 */
void
ls_build_queue(void)
{
    uint32_t i = 0;
    uint32_t pid = 0;
    struct qent *qptr = NULL;;
    struct qent *rq[3] = {qh, qm, ql};
    STATWORD ps;

    disable(ps);
    DTRACE_START;
    switch (prev_sched_algo)
    {
        case RANDOMSCHED:
            for (i = 0; i < 3; ++i) {
                qptr = rq[i];
                for (pid = 0; ((pid < NPROC) && (PID_IN_QUEUE(qptr, pid))); 
                        ++pid) {
                    ls_enqueue_pid(ls_rq, ls_rhead, ls_rtail, 
                            pid, proctab[pid].pgood);
                }
            }
            init_queues_pa1();
            break;

        case XINUSCHED:
            xs_print_queue();
            pid = q[rdytail].qprev;
            while (pid < NPROC) {            
                ls_enqueue_pid(ls_rq, ls_rhead, ls_rtail, 
                        pid, proctab[pid].pgood);
                pid = q[pid].qprev;
            }
            xs_init_queue();
            break;

        default:
            break;
    }
    DTRACE_END;
    restore(ps);
    return;
}


/* Updates the given pid's pquant, pctr and pgood. Usually called when an
 * epoch begins.
 */
void
ls_update_pquant_pgood(int pid)
{
    uint32_t old_quant = 0;
    uint32_t new_quant = 0;
    uint32_t pprio = 0;
    uint32_t pgood = 0;
    uint32_t ctr = 0;

    DTRACE_START;
    pprio = LS_GET_PPRIO(pid);
    old_quant = LS_GET_PQUANT(pid);
    ctr = LS_GET_PCTR(pid);
    if (old_quant == ctr) {
        /* The proc didn't use any of it's quantum. */
        new_quant = pprio;
        pgood = ctr + pprio;
        DTRACE("DBG_INFO: %d %s: pid %d didn't use any, quant %d, pgood %d\n", currpid, __func__, pid, new_quant, pgood);
    } else if (0 == ctr) {
        /* The proc has used its quantum to its fullest. */
        new_quant = pprio;
        pgood = 0;
        DTRACE("DBG_INFO: %d %s: pid %d used all, quant %d, pgood %d\n", currpid, __func__, pid, new_quant, pgood);
    } else if (ctr != 0) {
        /* Some quantum is left. */
        new_quant = pprio + (ctr >> 1);
        pgood = ctr + pprio;
        DTRACE("DBG_INFO: %d %s: pid %d used some, quant %d, pgood %d\n", currpid, __func__, pid, new_quant, pgood);
    }
    proctab[pid].pquant = new_quant;
    proctab[pid].pgood = pgood;
    proctab[pid].pctr = LS_GET_PQUANT(pid);

    DTRACE_END;
    return;
}


/* Recalculates the pquant, pctr & pgood for all procs in the system. Usually
 * called when a new epoch starts.
 */
void
ls_calc_pquant_pgood(struct qent *qptr, int head, int tail)
{
    int iter_pid = 0;
    DTRACE_START;

    iter_pid = qptr[head].qnext;
    while (iter_pid != tail) {
        if (PRFREE != LS_GET_PSTATE(iter_pid)) {
            ls_update_pquant_pgood(iter_pid);
        }
        iter_pid = qptr[iter_pid].qnext;
    }

    for (iter_pid = 1; iter_pid < NPROC; ++iter_pid) {
        switch (LS_GET_PSTATE(iter_pid)) {
            case PRSUSP:
                DTRACE("DBG_INFO: %d %s: suspended proc %d\n", currpid, __func__, iter_pid);            
                ls_update_pquant_pgood(iter_pid);
                break;
            case PRSLEEP:
                DTRACE("DBG_INFO: %d %s: sleeping proc %d\n", currpid, __func__, iter_pid);            
                ls_update_pquant_pgood(iter_pid);
                break;
            default:
                break;
        }
    }

    /* Now, update the currproc's quantum and goodness. */
    if ((NULLPROC != currpid) && ((PRCURR == LS_GET_PSTATE(currpid)) || 
                (PRREADY == LS_GET_PSTATE(currpid)))) {
        ls_update_pquant_pgood(currpid);
    }

    DTRACE_END;
    return;
}

/* Makes the newly created procs (during an ongoing epoch) eligible to be run
 * in the next epoch. This function is usually called once an epoch ends.
 */
void
ls_admit_new_procs(struct qent *qptr, int head, int tail)
{
    int pid = 0;
    DTRACE_START;

    for (pid = head; pid != tail; pid = qptr[pid].qnext) {
        if (PRNEWPA1 == LS_GET_PSTATE(pid)) {            
            proctab[pid].pstate = PRREADY;
            DTRACE("DBG_INFO: %d %s: pid %d's state changed to PRREADY\n", currpid, __func__, pid);
        }
    }
    DTRACE_END;
    return;
}


/* Returns TRUE iff the current epoch is over. An epoch ends when there are no
 * runnable procs in the ready queue (i.e., either ready queue is empty or none
 * of the procs in the ready queue has a non-zero quantum) and the currproc
 * doesn't have any ticks left either.
 */
int
ls_end_of_epoch(void)
{
    int iter_pid = 0;
    int end_flag = TRUE;
    DTRACE_START;

    iter_pid = ls_rq[ls_rtail].qprev;
    while (iter_pid != ls_rhead) {
        /* An epoch is not over until all runnable procs have used up their 
         * allocated quantum.
         */
        if ((PRREADY == LS_GET_PSTATE(iter_pid)) &&
                    (0 != LS_GET_PCTR(iter_pid))) {
            DTRACE("DBG_INFO: %d %s: pid %d has non-0 ctr\n", currpid, __func__, iter_pid);
            end_flag = FALSE;
            break;
        }
        iter_pid = ls_rq[iter_pid].qprev;
    }

    /* Epoch if not over yet if the currproc is still runnable with some
     * ticks left.
     */
    if (((PRCURR == LS_GET_PSTATE(currpid)) || 
                (PRREADY == LS_GET_PSTATE(currpid)))
            && (LS_GET_PCTR(currpid) != 0)) {
        DTRACE("DBG_INFO: %d %s: currpid has ctr left\n", currpid, __func__);
        end_flag = FALSE;
    }

    if (TRUE == end_flag) {
        DTRACE("DBG_INFO: %d %s: true\n", currpid, __func__);
        DTRACE_END;
        return TRUE;
    } else {
        DTRACE("DBG_INFO: %d %s: false\n", currpid, __func__);
        DTRACE_END;
        return FALSE;
    }
}

/* Checks if ls_rq is empty or not. */
int
ls_is_rqueue_empty(void)
{
    DTRACE_START;
    if (LS_IS_Q_EMPTY(ls_rq, ls_rhead, ls_rtail)) {
        DTRACE("DBG_INFO: %d %s: ls_rq empty\n", currpid, __func__);
        DTRACE_END;
        return TRUE;
    }
    DTRACE("DBG_INFO: %d %s: ls_rq not empty\n", currpid, __func__);
    DTRACE_END;
    return FALSE;
}

/* Called whenever a new epoch is about to begin. This function recalculates all
 * the required parameters of procs in the ready queue, wait queue and of that
 * of the current proc. It then re-orders the ready list based on goodness and 
 * then on priority.
 */
void
ls_process_new_epoch(void)
{
    DTRACE_START;
    ls_calc_pquant_pgood(ls_rq, ls_rhead, ls_rtail);
    ls_admit_new_procs(ls_rq, ls_rhead, ls_rtail);
    ls_init_queue(ls_rq, ls_rhead, ls_rtail);
    ls_reorder_queue(ls_rq, ls_rhead, ls_rtail);
    DTRACE_END;
    return;
}

/* Implements the actual Linux-like scheduling algorithm. Called during
 * ctxsw by resched() and returns a pointer to the newly selected proc.
 */
struct pentry*
linux_sched(struct pentry *optr)
{
    int npid = 0;
    struct pentry *nptr = NULL;

    DTRACE_START;
    DTRACE("DBG_INFO: %d %s: currproc state: %s\n", currpid, __func__, ch_pstates[optr->pstate - 1]);
    ls_print_queue(ls_rq, ls_rhead, ls_rtail);

    if (TRUE == ls_is_rqueue_empty()) {
        if (PRCURR == optr->pstate) {
            /* No other runnable procs. Recalculate the quantum and goodness of 
             * the currproc and schedule it again. 
             */
            if (0 == LS_GET_PCTR(currpid)) {
                optr->pticks += (optr->pquant - PA1_TICKS);
                ls_update_pquant_pgood(currpid);
                preempt = optr->pquant;
                DTRACE("DBG_INFO: %d %s: recal quant & preempt for currproc %d\n", currpid, __func__, optr->pid);
            }
                DTRACE("DBG_INFO: %d %s: %d is good to go\n", currpid, __func__, optr->pid);
            DTRACE_END;
            return NULL;
        } else {
            /* The currproc isn't runnable either. Schedule nullproc. */
            DTRACE("DBG_INFO: %d %s: returning nullproc\n", currpid, __func__);
            DTRACE_END;
            nptr = LS_GET_PPTR(NULLPROC);
            return nptr;
        }
    }


    if (TRUE == ls_end_of_epoch()) {
        ls_process_new_epoch();
    }

    /* If we are here, the epoch is not over. Return the next "good and
     * runnable" process from the ls_rq.
     */
    npid = ls_dequeue_pid(ls_rq, ls_rhead, ls_rtail);
    if (EMPTY_PA1 == npid) {
        if (0 != LS_GET_PCTR(currpid)) {
            /* No runnable procs in ls_rq and currproc has some time left.
             * Schedule currproc again.
             */
            DTRACE("DBG_INFO: %d %s: returning currproc\n", currpid, __func__);
            DTRACE_END;
            return NULL;
        } else {
            /* No runnable procs. Schedule nullproc. */
            DTRACE("DBG_INFO: %d %s: returning nullproc\n", currpid, __func__);
            DTRACE_END;
            nptr = LS_GET_PPTR(NULLPROC);
            return nptr;
        }
    }
    nptr = LS_GET_PPTR(npid);
    DTRACE_END;
    return nptr;
}

